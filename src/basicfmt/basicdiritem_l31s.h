/// @file basicdiritem_l31s.h
///
/// @brief disk basic directory item for L3 BASIC 1S
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_L31S_H
#define BASICDIRITEM_L31S_H

#include "basicdiritem_fat8.h"


/// ディレクトリ１アイテム L3 BASIC 単密度 1S
class DiskBasicDirItemL31S : public DiskBasicDirItemFAT8F
{
private:
	DiskBasicDirItemL31S() : DiskBasicDirItemFAT8F() {}
	DiskBasicDirItemL31S(const DiskBasicDirItemL31S &src) : DiskBasicDirItemFAT8F(src) {}

public:
	DiskBasicDirItemL31S(DiskBasic *basic);
	DiskBasicDirItemL31S(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemL31S(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);
};

#endif /* BASICDIRITEM_L31S_H */
