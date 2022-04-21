/// @file diskplainwriter.cpp
///
/// @brief べたディスクライター
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskplainwriter.h"
#include <wx/stream.h>
#include "diskd88.h"
#include "diskd88creator.h"
#include "diskresult.h"


//
// べた形式で保存
//
DiskPlainWriter::DiskPlainWriter(DiskWriter *dw_, DiskResult *result_)
	: DiskInhWriterBase(dw_, result_)
{
}

/// べたイメージでファイルに保存
/// @param [in,out] image ディスクイメージ
/// @param [in]     disk_number ディスク番号(0-) / -1のときは全体 
/// @param [in]     side_number サイド番号(0-) / -1のときは両面 
/// @param [out]    ostream     出力先
/// @retval  0 正常
/// @retval -1 エラー
int DiskPlainWriter::SaveDisk(DiskD88 *image, int disk_number, int side_number, wxOutputStream *ostream)
{
	result->Clear();

	DiskD88File *file = image->GetFile();
	if (!file) {
		result->SetError(DiskResult::ERR_NO_DATA);
		return result->GetValid();
	}

	if (disk_number < 0) {
		// 最初のディスクだけを保存
		DiskD88Disks *disks = file->GetDisks();
		if (!disks || disks->Count() <= 0) {
			result->SetError(DiskResult::ERR_NO_DISK);
			return result->GetValid();
		}
		for(size_t disk_num = 0; disk_num < 1; disk_num++) {
			DiskD88Disk *disk = disks->Item(disk_num);
			SaveDisk(disk, -1, ostream); 
		}
	} else {
		// 指定したディスクを保存
		DiskD88Disk *disk = file->GetDisk(disk_number);

		SaveDisk(disk, side_number, ostream); 
	}

	return result->GetValid();
}

/// べたイメージでディスク1つを保存
/// @param [in,out] disk        ディスク1つのイメージ
/// @param [in]     side_number サイド番号(0-) / -1のときは両面 
/// @param [out]    ostream     出力先
/// @retval  0 正常
/// @retval -1 エラー
int DiskPlainWriter::SaveDisk(DiskD88Disk *disk, int side_number, wxOutputStream *ostream)
{
	if (!disk) {
		result->SetError(DiskResult::ERR_NO_DISK);
		return result->GetValid();
	}

	DiskD88Tracks *tracks = disk->GetTracks();
	if (!tracks) {
		result->SetError(DiskResult::ERR_NO_DATA);
		return result->GetValid();
	}

	size_t track_start = (side_number < 0 ? 0 : side_number);
	size_t track_count = tracks->Count();
	size_t track_step  = (side_number < 0 ? 1 : 2);

	for(size_t track_num = track_start; track_num < track_count; track_num += track_step) {
		DiskD88Track *track = tracks->Item(track_num);
		if (!track) continue;
		DiskD88Sectors *sectors = track->GetSectors();
		if (!sectors) continue;
		// セクタ番号順に出力する
		DiskD88Sectors sorted_sectors = *sectors;
		sorted_sectors.Sort(&DiskD88Sector::CompareIDR);
		for(size_t idx = 0; idx < sorted_sectors.Count(); idx++) {
			DiskD88Sector *sector = sorted_sectors.Item(idx);
			if (!sector) continue;

			// write sector body
			wxUint8 *buffer = sector->GetSectorBuffer();
			size_t buffer_size = sector->GetSectorBufferSize();
			if (buffer && buffer_size) {
				ostream->Write((void *)buffer, buffer_size);	
			}
//			sector->ClearModify();
		}
	}
//	disk->ClearModify();

	return result->GetValid();
}
