/// @file basicdiritem_fm.h
///
/// @brief disk basic directory item for F-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FM_H_
#define _BASICDIRITEM_FM_H_

#include "basicdiritem_fat8.h"


/// ディレクトリ１アイテム F-BASIC
class DiskBasicDirItemFM : public DiskBasicDirItemFAT8F
{
private:
	DiskBasicDirItemFM() : DiskBasicDirItemFAT8F() {}
	DiskBasicDirItemFM(const DiskBasicDirItemFM &src) : DiskBasicDirItemFAT8F(src) {}

public:
	DiskBasicDirItemFM(DiskBasic *basic);
	DiskBasicDirItemFM(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemFM(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);
};

#endif /* _BASICDIRITEM_FM_H_ */
