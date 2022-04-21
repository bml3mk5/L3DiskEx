/// @file basictype_msdos.cpp
///
/// @brief disk basic type for Human68k
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_hu68k.h"
#include "basicfmt.h"
#include "basicdir.h"


//
//
//
DiskBasicTypeHU68K::DiskBasicTypeHU68K(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeMSDOS(basic, fat, dir)
{
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0  正常
/// @retval <1.0 警告あり
/// @retval <0.0 エラーあり
double DiskBasicTypeHU68K::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 0;

	double valid_ratio = 1.0;
	if (!basic->GetVariousBoolParam(wxT("IgnoreParameter"))) {
		valid_ratio = ParseMSDOSParamOnDisk(basic->GetDisk(), is_formatting);
	}
	if (valid_ratio >= 0.0) {
		// セクタ０
		DiskD88Sector *sector = basic->GetSector(0, 0, 1);
		if (!sector) {
			return -1.0;
		}

		// IPLに"X68IPL"が含まれるか
		// "Human"の文字列が含まれるか
		int found = -1;
		wxCharBuffer istr;
		for(int i=0; i<3; i++) {
			found = -1;
			switch(i) {
			case 0:
				istr = basic->GetVariousStringParam(wxT("IPLString")).To8BitData();
				break;
			case 1:
				istr = basic->GetVariousStringParam(wxT("IPLCompareString")).To8BitData();
				break;
			case 2:
				istr = wxCharBuffer("Human");
				break;
			}
			if (istr.length() > 0) {
				found = sector->Find(istr.data(), istr.length());
			}
			if (found >= 0) {
				valid_ratio = 1.0;
				break;
			}
		}
		if (found < 0) {
			valid_ratio = 0.1;
		}
	}

	return valid_ratio;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット IPLの書き込み
bool DiskBasicTypeHU68K::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	if (!CreateBiosParameterBlock("\x60\x3c\x90", "X68IPL30")) {
		return false;
	}

	// ボリュームラベルを設定
	int dir_start = basic->GetReservedSectors() + basic->GetNumberOfFats() * basic->GetSectorsPerFat();
	DiskD88Sector *sec = basic->GetSectorFromSectorPos(dir_start);
	DiskBasicDirItem *ditem = dir->NewItem(sec, 0, sec->GetSectorBuffer());

	ditem->SetFileNameStr(data.GetVolumeName());
	ditem->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_VOLUME_MASK);
	struct tm tm;
	wxDateTime::GetTmNow(&tm);
	ditem->SetFileDateTime(&tm);

	delete ditem;

	return true;
}
