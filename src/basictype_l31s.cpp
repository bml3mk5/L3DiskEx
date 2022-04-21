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
