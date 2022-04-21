/// @file diskd88writer.cpp
///
/// @brief D88ディスクライター
///
#include "diskd88writer.h"
#include <wx/stream.h>
#include "diskd88.h"
#include "diskd88creator.h"
#include "diskresult.h"

//
// D88形式で保存
//
DiskD88Writer::DiskD88Writer(DiskResult *result)
{
	this->result = result;
}

DiskD88Writer::~DiskD88Writer()
{
}

/// ストリームの内容をファイルに保存
/// @param [in,out] image ディスクイメージ
/// @param [in]     disk_number ディスク番号(0-) / -1のときは全体 
/// @param [in]     side_number サイド番号(0-) / -1のときは両面 
/// @param [out]    ostream     出力先
/// @retval  0 正常
/// @retval -1 エラー
int DiskD88Writer::SaveDisk(DiskD88 *image, int disk_number, int side_number, wxOutputStream *ostream)
{
	result->Clear();

	DiskD88File *file = image->GetFile();
	if (!file) {
		result->SetError(DiskResult::ERR_NO_DATA);
		return result->GetValid();
	}

	if (disk_number < 0) {
		// 全体を保存
		DiskD88Disks *disks = file->GetDisks();
		if (!disks || disks->Count() <= 0) {
			result->SetError(DiskResult::ERR_NO_DISK);
			return result->GetValid();
		}
		for(size_t disk_num = 0; disk_num < disks->Count(); disk_num++) {
			DiskD88Disk *disk = disks->Item(disk_num);
			SaveDisk(disk, ostream); 
		}
	} else {
		// 指定したディスクを保存
		DiskD88Disk *disk = file->GetDisk(disk_number);

		if (side_number < 0) {
			SaveDisk(disk, ostream);
		} else {
			SaveDisk(disk, side_number, ostream); 
		}
	}

	return result->GetValid();
}

/// ディスク1つを保存
int DiskD88Writer::SaveDisk(DiskD88Disk *disk, wxOutputStream *ostream)
{
	if (!disk) {
		result->SetError(DiskResult::ERR_NO_DISK);
		return result->GetValid();
	}

	// ヘッダオフセットが古い場合や、ディスクサイズが一致しない場合、再計算する
	size_t new_size = disk->CalcSizeWithoutHeader();
	if (new_size != disk->GetSizeWithoutHeader() || (size_t)disk->GetOffsetStart() < sizeof(d88_header_t)) {
		disk->SetOffsetStart(sizeof(d88_header_t));
		new_size = disk->Shrink();
		disk->SetSizeWithoutHeader((wxUint32)new_size);
	}

	// write disk header
	ostream->Write((void *)disk->GetHeader(), (size_t)disk->GetOffsetStart());	

	DiskD88Tracks *tracks = disk->GetTracks();
	if (!tracks) {
		result->SetError(DiskResult::ERR_NO_DATA);
		return result->GetValid();
	}

	for(size_t track_num = 0; track_num < tracks->Count(); track_num++) {
		DiskD88Track *track = tracks->Item(track_num);
		if (!track) continue;
		DiskD88Sectors *sectors = track->GetSectors();
		if (!sectors) continue;
		for(size_t sector_num = 0; sector_num < sectors->Count(); sector_num++) {
			DiskD88Sector *sector = sectors->Item(sector_num);
			if (!sector) continue;

			// write sector header
			ostream->Write((void *)sector->GetHeader(), sizeof(d88_sector_header_t));	
			// write sector body
			wxUint8 *buffer = sector->GetSectorBuffer();
			size_t buffer_size = sector->GetSectorBufferSize();
			if (buffer && buffer_size) {
				ostream->Write((void *)buffer, buffer_size);	
			}
//			sector->ClearModify();
		}
	}
	if (result->GetValid() >= 0) {
		disk->ClearModify();
	}
	return result->GetValid();
}

/// ディスクの1つサイドだけを保存(1S用)
int DiskD88Writer::SaveDisk(DiskD88Disk *disk, int side_number, wxOutputStream *ostream)
{
	if (!disk) {
		result->SetError(DiskResult::ERR_NO_DISK);
		return result->GetValid();
	}

	// 1S用のディスクを作成
	SingleDensities singles;
	singles.Add(new SingleDensity(-1, -1, disk->GetSectorsPerTrack(), 128));
	wxArrayString basic_types;
	DiskParam param(wxT("1S"), 0, basic_types, 1, 40, 16, 128, 0, 0, disk->GetInterleave(), singles, wxT(""));

	DiskD88File tmpfile;
	DiskD88Creator cr("", param, false, &tmpfile, *result);
	int valid_disk = cr.Create();
	if (valid_disk != 0) {
		return valid_disk;
	}

	DiskD88Disk *tmpdisk = tmpfile.GetDisk(0);
	if (!tmpdisk) {
		result->SetError(DiskResult::ERR_NO_DATA);
		return result->GetValid();
	}

	// 新しいディスクにコピーする
	valid_disk = tmpdisk->Replace(0, disk, side_number);
	if (valid_disk != 0) {
		result->SetError(DiskResult::ERR_REPLACE);
	}

	return SaveDisk(tmpdisk, ostream);
}
