/// @file basictype_msx.cpp
///
/// @brief disk basic fat type for MSX
///
#include "basictype_msx.h"
#include "basicfmt.h"

//
//
//
DiskBasicTypeMSX::DiskBasicTypeMSX(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT12(basic, fat, dir)
{
}

/// セクタデータを埋めた後の個別処理
/// フォーマット IPLの書き込み
void DiskBasicTypeMSX::AdditionalProcessOnFormatted()
{
	fat_bpb_t hed;
	memset(&hed, 0, sizeof(hed));
	memcpy(hed.BS_JmpBoot, "\xeb\xfe\x90", 3);
	memcpy(hed.BS_OEMName, "MSX     ", 8);
	hed.BPB_BytsPerSec = wxUINT16_SWAP_ON_BE(basic->GetSectorSize());
	hed.BPB_SecPerClus = basic->GetSectorsPerGroup();
	hed.BPB_RsvdSecCnt = wxUINT16_SWAP_ON_BE(basic->GetReservedSectors());
	hed.BPB_NumFATs = basic->GetNumberOfFats();
	hed.BPB_RootEntCnt = wxUINT16_SWAP_ON_BE(basic->GetDirEntryCount());
	hed.BPB_TotSec16 = basic->GetTracksPerSide() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic();
	hed.BPB_TotSec16 = wxUINT16_SWAP_ON_BE(hed.BPB_TotSec16);
	hed.BPB_Media = basic->GetMediaId();
	hed.BPB_FATSz16 = wxUINT16_SWAP_ON_BE(basic->GetSectorsPerFat());
	hed.BPB_SecPerTrk =  wxUINT16_SWAP_ON_BE(basic->GetSectorsPerTrackOnBasic());
	hed.BPB_NumHeads = wxUINT16_SWAP_ON_BE(basic->GetSidesOnBasic());

	DiskD88Sector *sec = basic->GetDisk()->GetSector(0, 0, 1);
	if (!sec) return;
	wxUint8 *buf = sec->GetSectorBuffer();
	if (!buf) return;
	memcpy(buf, &hed, sizeof(hed));
	// 起動時の実行コード
	buf[0x1e] = 0xd0;	// RET NC

	// FATの先頭にメディアIDをセット
	SetGroupNumber(0, 0xffffff00 | basic->GetMediaId());
	SetGroupNumber(1, 0xffffffff);
}
