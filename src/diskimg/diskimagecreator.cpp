/// @file diskimagecreator.cpp
///
/// @brief ディスクイメージ作成
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskimagecreator.h"
#include "diskparam.h"
#include "diskimage.h"
#include "diskresult.h"


//
//
//
DiskImageCreator::DiskImageCreator(const wxString &diskname, const DiskParam &param, bool write_protect, DiskImageFile *file, DiskResult &result)
{
	m_diskname = diskname;
	p_param = &param;
	m_write_protect = write_protect;
	p_file = file;
	p_result = &result;
}

DiskImageCreator::~DiskImageCreator()
{
}

/// セクタデータの作成
/// @param[in] track_number      トラック番号
/// @param[in] side_number       サイド番号
/// @param[in] sector_number     セクタ番号
/// @param[in] sector_size       セクタサイズ
/// @param[in] sectors_per_track セクタ数
/// @param[in,out] track         トラック
/// @return 作成したセクタのサイズ（ヘッダ含む）
wxUint32 DiskImageCreator::CreateSector(int track_number, int side_number, int sector_number, int sector_size, int sectors_per_track, DiskImageTrack *track)
{
	// 特殊なセクタにするか
	const wxUint8 *sector_id = NULL;
	if (p_param->FindParticularSector(track_number, side_number, sector_number, sector_size, &sector_id)) {
		if (sector_id[2] > 0) {
			sector_number = sector_id[2];
		}
	}
	// 単密度にするか
	bool single_density = p_param->FindSingleDensity(track_number, side_number, sector_number, sector_size);

	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, sectors_per_track, single_density);
	track->Add(sector);

	// このセクタデータのサイズを返す
	return (wxUint32)sector->GetSize();
}

/// トラックデータの作成
/// @param[in] track_number トラック番号
/// @param[in] side_number  サイド番号
/// @param[in] offset_pos   オフセット番号
/// @param[in] offset       トラックのあるオフセット位置
/// @param[in,out] disk     ディスク
/// @return 作成したトラックサイズ
wxUint32 DiskImageCreator::CreateTrack(int track_number, int side_number, int offset_pos, wxUint32 offset, DiskImageDisk *disk)
{
	// トラック作成
	DiskImageTrack *track = disk->NewImageTrack(track_number, side_number, offset_pos, p_param->GetInterleave());

	int sector_max = p_param->GetSectorsPerTrack();
	int sector_size = p_param->GetSectorSize();

	// 特殊なトラックにするか
	p_param->FindParticularTrack(track_number, side_number, sector_max, sector_size);
	// トラック全体が単密度の場合セクタ数とサイズを得る
	p_param->FindSingleDensity(track_number, side_number, &sector_max, &sector_size);

	// interleave の並び順を計算
//	int *sector_nums = new int[sector_max + 1];
	wxArrayInt sector_nums;
	if (!DiskImageTrack::CalcSectorNumbersForInterleave(p_param->GetInterleave(), sector_max, sector_nums, p_param->GetSectorNumberBaseOnDisk())) {
		p_result->SetError(DiskResult::ERR_INTERLEAVE);
	}

	// create sectors
	wxUint32 track_size = 0;
	for(int sector_pos = 0; sector_pos < sector_max && p_result->GetValid() >= 0; sector_pos++) {
		int sector_offset = 0;
		if (p_param->IsReversible()) {
			// 裏返しできる(AB面あり)場合
			side_number = 0;
		}
		if (p_param->GetNumberingSector() == 1) {
			// 連番にする場合
			sector_offset = side_number * sector_max;
		}
		track_size += CreateSector(track_number, side_number, sector_nums[sector_pos] + sector_offset, sector_size, sector_max, track);
	}

	if (p_result->GetValid() >= 0) {
		// トラックを追加
		track->SetSize(track_size);
		disk->Add(track);
	} else {
		delete track;
	}

	return track_size;
}

/// ディスクデータの作成
/// @param[in] disk_number ディスク番号
/// @param[in] mod_flags   新規 or 追加？(DiskImageFile::MODIFY_NONE/MODIFY_ADD)
/// @return 作成したディスクサイズ
wxUint32 DiskImageCreator::CreateDisk(int disk_number, short mod_flags)
{
	DiskImageDisk *disk = p_file->NewImageDisk(disk_number, *p_param, m_diskname, m_write_protect);

	// create tracks
	size_t create_size = 0;
	int track_num = p_param->GetTrackNumberBaseOnDisk();
	int side_num = 0;
	int tracks_per_side = p_param->GetTracksPerSide() + track_num;

	for(int pos = 0; p_result->GetValid() >= 0; pos++) {
		disk->SetOffsetWithoutHeader(pos, (wxUint32)create_size);
		disk->SetMaxTrackNumber(pos);

		create_size += CreateTrack(track_num, side_num, pos, disk->GetOffset(pos), disk);

		side_num++;
		if (side_num >= p_param->GetSidesPerDisk()) {
			track_num++;
			side_num = 0;
		}
		if (track_num >= tracks_per_side) {
			break;
		}
	}

	if (p_result->GetValid() >= 0) {
		// ディスクを追加
		if (p_param->GetBasicTypes().IsEmpty()) {
			// パラメータが手動設定のときはそれらしいテンプレートをさがす
			disk->CalcMajorNumber();
		} else {
			// テンプレートから設定
			disk->SetDiskParam(*p_param);
			// DISKBASICの準備
			disk->AllocDiskBasics();
		}
		disk->SetSizeWithoutHeader((wxUint32)create_size);
		p_file->Add(disk, mod_flags);
	} else {
		delete disk;
	}

	return (wxUint32)create_size;
}
/// ディスクイメージの新規作成
int DiskImageCreator::Create()
{
	CreateDisk(0, DiskImageFile::MODIFY_NONE);
	return p_result->GetValid();
}
/// 新規作成して既存のイメージに追加
int DiskImageCreator::Add()
{
	int disk_number = 0;
	DiskImageDisks *disks = p_file->GetDisks();
	if (disks) {
		disk_number = (int)disks->Count();
	}

	CreateDisk(disk_number, DiskImageFile::MODIFY_ADD);

	return p_result->GetValid();
}

