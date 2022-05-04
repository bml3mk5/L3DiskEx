/// @file basictype_fm.cpp
///
/// @brief disk basic type for F-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_fm.h"
#include "basicfmt.h"


//
//
//
DiskBasicTypeFM::DiskBasicTypeFM(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT8F(basic, fat, dir)
{
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFM::ParseParamOnDisk(bool is_formatting)
{
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		// トラック０と管理トラック分を引く
		end_group -= basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic() * (basic->GetManagedTrackNumber() == 0 ? 1 : 2);
		end_group /= basic->GetSectorsPerGroup();
		basic->SetFatEndGroup(end_group - 1);
	}
	return 1.0;
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFM::CheckFat(bool is_formatting)
{
	double valid_ratio = DiskBasicTypeFAT8::CheckFat(is_formatting);
	if (valid_ratio >= 0.0) {
		// IDのチェック
		wxUint8 id = basic->GetVariousStringParam(wxT("IDString"))[0];
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(basic->GetVariousIntegerParam(wxT("IDSectorPosition")));
		if (!(sector && id == sector->Get(0))) {
			valid_ratio = -1.0;
		}
		// FAT先頭エリアのチェック
		sector = basic->GetManagedSector(basic->GetFatSideNumber() * basic->GetSectorsPerTrackOnBasic() + basic->GetFatStartSector() - 1);
		if (!sector) {
			valid_ratio = -1.0;
		} else if (sector->Get(0) != 0 || sector->Get(1) != 0xff) {
			valid_ratio = -1.0;
		}
	}
	return valid_ratio;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT,IDのセット
bool DiskBasicTypeFM::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// FATエリア先頭に0を入れる
	fat->Set(0, 0);

	// IDエリアを初期化
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(basic->GetVariousIntegerParam(wxT("IDSectorPosition")));
	if (sector) {
		sector->Fill(0);
		wxCharBuffer id = basic->GetVariousStringParam(wxT("IDString")).To8BitData();
		if (id.length() > 0) {
			sector->Copy(id.data(), (int)id.length());
		}
	}

	return true;
}

/// データ領域の開始セクタを計算
int DiskBasicTypeFM::CalcDataStartSectorPos()
{
	// トラック0を除く
	return basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
}

/// スキップするトラック番号
int DiskBasicTypeFM::CalcSkippedTrack()
{
	int val = basic->GetManagedTrackNumber();
	return val > 0 ? val : 0x7fff;
}
