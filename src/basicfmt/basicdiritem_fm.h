/// @file basicdiritem_fm.h
///
/// @brief disk basic directory item for F-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_FM_H
#define BASICDIRITEM_FM_H

#include "basicdiritem_fat8.h"


/// ディレクトリ１アイテム F-BASIC
class DiskBasicDirItemFM : public DiskBasicDirItemFAT8F
{
private:
	DiskBasicDirItemFM() : DiskBasicDirItemFAT8F() {}
	DiskBasicDirItemFM(const DiskBasicDirItemFM &src) : DiskBasicDirItemFAT8F(src) {}

public:
	DiskBasicDirItemFM(DiskBasic *basic);
	DiskBasicDirItemFM(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemFM(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);
};

#endif /* BASICDIRITEM_FM_H */
