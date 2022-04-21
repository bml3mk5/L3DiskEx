/// @file diskd88writer.cpp
///
/// @brief D88ディスクライター
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskwriter.h"
#include "diskd88writer.h"
#include <wx/stream.h>
#include "diskd88.h"
#include "diskd88creator.h"
#include "diskresult.h"


//
// D88形式で保存
//
DiskD88Writer::DiskD88Writer(DiskWriter *dw, DiskResult *result)
{
	this->dw = dw;
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
			SaveDisk(disk, -1, ostream); 
		}
	} else {
		// 指定したディスクを保存
		DiskD88Disk *disk = file->GetDisk(disk_number);

		SaveDisk(disk, side_number, ostream); 
	}

	return result->GetValid();
}

/// ディスク1つを保存
/// @param [in]  disk        ディスク
/// @param [in]  side_number サイド 両面なら -1
/// @param [out] ostream     出力先
int DiskD88Writer::SaveDisk(DiskD88Disk *disk, int side_number, wxOutputStream *ostream)
{
	if (!disk) {
		result->SetError(DiskResult::ERR_NO_DISK);
		return result->GetValid();
	}

	// オフセットを再計算する
	size_t new_size = 0;
	disk->SetOffsetStart(sizeof(d88_header_t));
	if (side_number < 0) {
		new_size = disk->ShrinkTracks(dw->IsTrimUnusedData());
		disk->SetSizeWithoutHeader((wxUint32)new_size);
	}

	// ディスクヘッダ
	d88_header_t newheader;
	memset(&newheader, 0, sizeof(d88_header_t));
	memcpy(&newheader, disk->GetHeader(), (size_t)disk->GetOffsetStart());

	// write disk header
	ostream->Write(&newheader, sizeof(d88_header_t));	

	DiskD88Tracks *tracks = disk->GetTracks();
	if (!tracks) {
		result->SetError(DiskResult::ERR_NO_DATA);
		return result->GetValid();
	}

	// オフセットクリア
	memset(newheader.offsets, 0, sizeof(newheader.offsets));

	size_t track_start = (side_number < 0 ? 0 : side_number);
	size_t track_count = tracks->Count();
	size_t track_step  = (side_number < 0 ? 1 : 2);

	size_t track_offpos = 0;
	size_t track_offset = (size_t)disk->GetOffsetStart();
	for(size_t track_num = track_start; track_num < track_count; track_num += track_step) {
		DiskD88Track *track = tracks->Item(track_num);
		if (!track) continue;
		size_t track_size = 0;
		DiskD88Sectors *sectors = track->GetSectors();
		size_t count = sectors ? sectors->Count() : 0;
		for(size_t sector_num = 0; sector_num < count; sector_num++) {
			DiskD88Sector *sector = sectors->Item(sector_num);
			if (!sector) continue;

			// セクタヘッダ
			d88_sector_header_t secheader;
			memcpy(&secheader, sector->GetHeader(), sizeof(d88_sector_header_t));

			if (side_number >= 0) {
				// 片面だけ保存のときはID Hを0にする
				secheader.id.h = 0;
			}

			// write sector header
			ostream->Write(&secheader, sizeof(d88_sector_header_t));
			track_size += sizeof(d88_sector_header_t);

			// write sector body
			wxUint8 *buffer = sector->GetSectorBuffer();
			size_t buffer_size = sector->GetSectorBufferSize();
			if (buffer && buffer_size) {
				ostream->Write((void *)buffer, buffer_size);	
				track_size += buffer_size;
			}
//			sector->ClearModify();
		}
		//
		if (!dw->IsTrimUnusedData()) {
			// 余分なデータ
			wxUint8 *extra_data = track->GetExtraData();
			size_t   extra_size = track->GetExtraDataSize();
			if (extra_data && extra_size > 0) {
				ostream->Write(extra_data, extra_size);
				track_size += extra_size;
			}
		}
		if (track_size > 0) {
			// オフセットをセット
			newheader.offsets[track_offpos] = wxUINT32_SWAP_ON_BE((wxUint32)track_offset);
			track_offpos++;
			track_offset += track_size;
			newheader.disk_size = wxUINT32_SWAP_ON_BE((wxUint32)track_offset);
		}
	}
	if (side_number >= 0) {
		// 片面だけ保存のときはディスクヘッダを更新
		ostream->SeekO(0);
		ostream->Write(&newheader, sizeof(d88_header_t));
	}

	if (result->GetValid() >= 0) {
		disk->ClearModify();
	}
	return result->GetValid();
}
