/// @file basictype_msx.cpp
///
/// @brief disk basic type for MSX
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_msx.h"
#include "basicfmt.h"


//
//
//
DiskBasicTypeMSX::DiskBasicTypeMSX(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeMSDOS(basic, fat, dir)
{
}

static const char *c_exclude_keywords[] = {
	"IO      SYS",
	"IBMDOS",
	"MSDOS",
	"IBM",
	NULL
};


/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMSX::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 0;

	double valid_ratio = ParseMSDOSParamOnDisk(basic->GetDisk(), is_formatting);
	if (valid_ratio >= 0.0) {
		DiskImageSector *sector = basic->GetSector(0, 0, 1);
		if (!sector) return -1.0;
		wxUint8 *datas = sector->GetSectorBuffer();
		if (!datas) return -1.0;
//		fat_bpb_t *bpb = (fat_bpb_t *)datas;
		// MSXDOSの名前があれば確実
		if (sector->Find("MSXDOS", 6) >= 0) {
			valid_ratio += 0.8;
		} else if (sector->Find("MSX", 3) >= 0) {
			valid_ratio += 0.4;
		}
		// 除外するキーワード
		for(int i=0; c_exclude_keywords[i]; i++) {
			if (sector->Find(c_exclude_keywords[i], strlen(c_exclude_keywords[i])) >= 0) {
				valid_ratio -= 0.5;
				break;
			}
		}
	}
	if (valid_ratio > 1.0) valid_ratio = 1.0;
	else if (valid_ratio < -1.0) valid_ratio = -1.0;

	return valid_ratio;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット IPLの書き込み
bool DiskBasicTypeMSX::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
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
