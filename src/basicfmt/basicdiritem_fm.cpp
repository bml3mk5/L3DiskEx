/// @file basicdiritem_fm.cpp
///
/// @brief disk basic directory item for F-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_fm.h"
#include "basicfmt.h"
#include "../charcodes.h"


///
///
///
DiskBasicDirItemFM::DiskBasicDirItemFM(DiskBasic *basic)
	: DiskBasicDirItemFAT8F(basic)
{
}
DiskBasicDirItemFM::DiskBasicDirItemFM(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemFAT8F(basic, n_sector, n_secpos, n_data)
{
}
DiskBasicDirItemFM::DiskBasicDirItemFM(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemFAT8F(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
}
