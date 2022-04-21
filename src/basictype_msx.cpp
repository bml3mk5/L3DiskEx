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

/// ディスク上のパラメータを読む
int DiskBasicTypeMSX::ParseParamOnDisk(DiskD88Disk *disk)
{
	int valid = DiskBasicTypeFAT12::ParseParamOnDisk(disk);
	if (valid == 0) {
		DiskD88Sector *sector = disk->GetSector(0, 0, 1);
		wxUint8 *datas = sector->GetSectorBuffer();
		fat_bpb_t *bpb = (fat_bpb_t *)datas;
		char oem_name[10];
		memset(oem_name, 0, sizeof(oem_name));
		memcpy(oem_name, bpb->BS_OEMName, sizeof(bpb->BS_OEMName));
		if (strstr(oem_name, "MSX") == NULL) {
			valid = -1;
		}
	}
	return valid;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット IPLの書き込み
bool DiskBasicTypeMSX::AdditionalProcessOnFormatted()
{
	wxUint8 *buf = NULL;
	if (!CreateBiosParameterBlock("\xeb\xfe\x90", "MSX", &buf)) {
		return false;
	}

	// 起動時の実行コード
	if (buf) {
		buf[0x1e] = 0xd0;	// RET NC
	}

	return true;
}
