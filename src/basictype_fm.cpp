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

/// FATエリアをチェック
bool DiskBasicTypeFM::CheckFat()
{
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		// トラック０と管理トラック分を引く
		end_group -= basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic() * (basic->GetManagedTrackNumber() == 0 ? 1 : 2);
		end_group /= basic->GetSectorsPerGroup();
		basic->SetFatEndGroup(end_group - 1);
	}

	bool valid = DiskBasicType::CheckFat();
	if (valid) {
		// IDのチェック
		wxUint8 id = basic->GetIDString()[0];
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(basic->GetIdSectorPos());
		if (!(sector && id == sector->Get(0))) {
			valid = false;
		}
		// FAT先頭エリアのチェック
		sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
		if (!sector) {
			valid = false;
		} else if (sector->Get(0) != 0 || sector->Get(1) != 0xff) {
			valid = false;
		}
	}
	return valid;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT,IDのセット
bool DiskBasicTypeFM::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// FATエリア先頭に0を入れる
	fat->Set(0, 0);

	// IDエリアを初期化
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(basic->GetIdSectorPos());
	if (sector) {
		sector->Fill(0);
		sector->Copy(basic->GetIDString().To8BitData(), basic->GetIDString().Length());
	}

	return true;
}

/// データ領域の開始セクタを計算
int DiskBasicTypeFM::CalcDataStartSectorPos()
{
	// トラック0を除く
	return basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
}
