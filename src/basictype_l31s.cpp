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

/// FATエリアをチェック
bool DiskBasicTypeL31S::CheckFat()
{
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		// 管理トラック分を引く
		end_group -= basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		end_group /= basic->GetSectorsPerGroup();
		basic->SetFatEndGroup(end_group - 1);
	}

	bool valid = DiskBasicType::CheckFat();
	if (valid) {
		// FAT先頭エリアのチェック
		DiskD88Sector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
		if (!sector) {
			valid = false;
		} else if (!((sector->Get(0) == 0 || sector->Get(0) == 0xff) && sector->Get(1) == 0xff)) {
			valid = false;
		}
	}
	return valid;
}
