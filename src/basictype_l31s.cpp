/// @file basictype_l31s.cpp
///
/// @brief disk basic type for L3 BASIC 1S
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_l31s.h"
#include "basicfmt.h"


//
//
//
DiskBasicTypeL31S::DiskBasicTypeL31S(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT8F(basic, fat, dir)
{
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
double DiskBasicTypeL31S::ParseParamOnDisk(DiskD88Disk *disk, bool is_formatting)
{
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		// 管理トラック分を引く
		end_group -= basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
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
double DiskBasicTypeL31S::CheckFat(bool is_formatting)
{
	double valid_ratio = DiskBasicTypeFAT8::CheckFat(is_formatting);
	if (valid_ratio >= 0.0) {
		// FAT先頭エリアのチェック
		DiskD88Sector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
		if (!sector) {
			valid_ratio = -1.0;
		} else if (!((sector->Get(0) == 0 || sector->Get(0) == 0xff) && sector->Get(1) == 0xff)) {
			valid_ratio = -1.0;
		}
	}
	return valid_ratio;
}
