/// @file diskd88creator.cpp
///
/// @brief D88ディスクイメージ作成
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskd88creator.h"
#include "../diskparam.h"
#include "../diskd88.h"
#include "diskresult.h"


//
//
//
DiskD88Creator::DiskD88Creator(const wxString &diskname, const DiskParam &param, bool write_protect, DiskD88File *file, DiskResult &result)
{
	this->diskname = diskname;
	this->param = &param;
	this->write_protect = write_protect;
	this->file = file;
	this->result = &result;
}

DiskD88Creator::~DiskD88Creator()
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
wxUint32 DiskD88Creator::CreateSector(int track_number, int side_number, int sector_number, int sector_size, int sectors_per_track, DiskD88Track *track)
{
	// 特殊なセクタにするか
	const wxUint8 *sector_id = NULL;
	if (param->FindParticularSector(track_number, side_number, sector_number, sector_size, &sector_id)) {
		if (sector_id[2] > 0) {
			sector_number = sector_id[2];
		}
	}
	// 単密度にするか
	bool single_density = param->FindSingleDensity(track_number, side_number, sector_number, sector_size);

	DiskD88Sector *sector = new DiskD88Sector(track_number, side_number, sector_number, sector_size, sectors_per_track, single_density);
	track->Add(sector);

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + sector->GetSectorBufferSize();
}

/// トラックデータの作成
/// @param[in] track_number トラック番号
/// @param[in] side_number  サイド番号
/// @param[in] offset_pos   オフセット番号
/// @param[in] offset       トラックのあるオフセット位置
/// @param[in,out] disk     ディスク
/// @return 作成したトラックサイズ
wxUint32 DiskD88Creator::CreateTrack(int track_number, int side_number, int offset_pos, wxUint32 offset, DiskD88Disk *disk)
{
	// トラック作成
	DiskD88Track *track = new DiskD88Track(disk, track_number, side_number, offset_pos, param->GetInterleave());

	int sector_max = param->GetSectorsPerTrack();
	int sector_size = param->GetSectorSize();

	// 特殊なトラックにするか
	param->FindParticularTrack(track_number, side_number, sector_max, sector_size);
	// トラック全体が単密度の場合セクタ数とサイズを得る
	param->FindSingleDensity(track_number, side_number, &sector_max, &sector_size);

	// interleave の並び順を計算
//	int *sector_nums = new int[sector_max + 1];
	wxArrayInt sector_nums;
	if (!DiskD88Track::CalcSectorNumbersForInterleave(param->GetInterleave(), sector_max, sector_nums, param->GetSectorNumberBaseOnDisk())) {
		result->SetError(DiskResult::ERR_INTERLEAVE);
	}

	// create sectors
	wxUint32 track_size = 0;
	for(int sector_pos = 0; sector_pos < sector_max && result->GetValid() >= 0; sector_pos++) {
		int sector_offset = 0;
		if (param->IsReversible()) {
			// 裏返しできる(AB面あり)場合
			side_number = 0;
		}
		if (param->GetNumberingSector() == 1) {
			// 連番にする場合
			sector_offset = side_number * sector_max;
		}
		track_size += CreateSector(track_number, side_number, sector_nums[sector_pos] + sector_offset, sector_size, sector_max, track);
	}

	if (result->GetValid() >= 0) {
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
/// @param[in] mod_flags   新規 or 追加？(DiskD88File::MODIFY_NONE/MODIFY_ADD)
/// @return 作成したディスクサイズ
wxUint32 DiskD88Creator::CreateDisk(int disk_number, short mod_flags)
{
	DiskD88Disk *disk = new DiskD88Disk(file, diskname, disk_number, *param, write_protect);

	// create tracks
	size_t create_size = 0;
	int track_num = param->GetTrackNumberBaseOnDisk();
	int side_num = 0;
	int tracks_per_side = param->GetTracksPerSide() + track_num;

	for(int pos = 0; /* pos < DISKD88_MAX_TRACKS && */ result->GetValid() >= 0; pos++) {
		disk->SetOffsetWithoutHeader(pos, (wxUint32)create_size);
		disk->SetMaxTrackNumber(pos);

		create_size += CreateTrack(track_num, side_num, pos, disk->GetOffset(pos), disk);

		side_num++;
		if (side_num >= param->GetSidesPerDisk()) {
			track_num++;
			side_num = 0;
		}
		if (track_num >= tracks_per_side) {
			break;
		}
	}

	if (result->GetValid() >= 0) {
		// ディスクを追加
		if (param->GetBasicTypes().IsEmpty()) {
			// パラメータが手動設定のときはそれらしいテンプレートをさがす
			disk->CalcMajorNumber();
		} else {
			// テンプレートから設定
			disk->SetDiskParam(*param);
			disk->AllocDiskBasics();
		}
		disk->SetSizeWithoutHeader((wxUint32)create_size);
		file->Add(disk, mod_flags);
	} else {
		delete disk;
	}

	return (wxUint32)create_size;
}
/// ディスクイメージの新規作成
int DiskD88Creator::Create()
{
	CreateDisk(0, DiskD88File::MODIFY_NONE);
	return result->GetValid();
}
/// 新規作成して既存のイメージに追加
int DiskD88Creator::Add()
{
	int disk_number = 0;
	DiskD88Disks *disks = file->GetDisks();
	if (disks) {
		disk_number = (int)disks->Count();
	}

	CreateDisk(disk_number, DiskD88File::MODIFY_ADD);

	return result->GetValid();
}

