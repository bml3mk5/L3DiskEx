/// @file diskplainwriter.cpp
///
/// @brief べたディスクライター
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskplainwriter.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "diskimagecreator.h"
#include "diskresult.h"


//
// べた形式で保存
//
DiskPlainWriter::DiskPlainWriter(DiskWriter *dw_, DiskResult *result_)
	: DiskImageWriter(dw_, result_)
{
}

/// べたイメージでファイルに保存
/// @param [in,out] image ディスクイメージ
/// @param [in]     disk_number ディスク番号(0-) / -1のときは全体 
/// @param [in]     side_number サイド番号(0-) / -1のときは両面 
/// @param [out]    ostream     出力先
/// @retval  0 正常
/// @retval -1 エラー
int DiskPlainWriter::SaveDisk(DiskImage *image, int disk_number, int side_number, wxOutputStream *ostream)
{
	p_result->Clear();

	DiskImageFile *file = image->GetFile();
	if (!file) {
		p_result->SetError(DiskResult::ERR_NO_DATA);
		return p_result->GetValid();
	}

	if (disk_number < 0) {
		// 最初のディスクだけを保存
		DiskImageDisks *disks = file->GetDisks();
		if (!disks || disks->Count() <= 0) {
			p_result->SetError(DiskResult::ERR_NO_DISK);
			return p_result->GetValid();
		}
		for(size_t disk_num = 0; disk_num < 1; disk_num++) {
			DiskImageDisk *disk = disks->Item(disk_num);
			SaveDisk(disk, -1, ostream); 
		}
	} else {
		// 指定したディスクを保存
		DiskImageDisk *disk = file->GetDisk(disk_number);

		SaveDisk(disk, side_number, ostream); 
	}

	return p_result->GetValid();
}

/// べたイメージでディスク1つを保存
/// @param [in,out] disk        ディスク1つのイメージ
/// @param [in]     side_number サイド番号(0-) / -1のときは両面 
/// @param [out]    ostream     出力先
/// @retval  0 正常
/// @retval -1 エラー
int DiskPlainWriter::SaveDisk(DiskImageDisk *disk, int side_number, wxOutputStream *ostream)
{
	if (!disk) {
		p_result->SetError(DiskResult::ERR_NO_DISK);
		return p_result->GetValid();
	}

	DiskImageTracks *tracks = disk->GetTracks();
	if (!tracks) {
		p_result->SetError(DiskResult::ERR_NO_DATA);
		return p_result->GetValid();
	}

	size_t track_start = (side_number < 0 ? 0 : side_number);
	size_t track_count = tracks->Count();
	size_t track_step  = (side_number < 0 ? 1 : 2);

	for(size_t track_num = track_start; track_num < track_count; track_num += track_step) {
		DiskImageTrack *track = tracks->Item(track_num);
		if (!track) continue;
		DiskImageSectors *sectors = track->GetSectors();
		if (!sectors) continue;
		// セクタ番号順に出力する
		DiskImageSectors sorted_sectors = *sectors;
		sorted_sectors.Sort(&DiskImageSector::CompareIDR);
		for(size_t idx = 0; idx < sorted_sectors.Count(); idx++) {
			DiskImageSector *sector = sorted_sectors.Item(idx);
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

	return p_result->GetValid();
}
