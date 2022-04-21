/// @file basicdiritem_fm.cpp
///
/// @brief disk basic directory item for F-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_fm.h"
#include "basicfmt.h"
#include "charcodes.h"


///
///
///
DiskBasicDirItemFM::DiskBasicDirItemFM(DiskBasic *basic)
	: DiskBasicDirItemFAT8F(basic)
{
}
DiskBasicDirItemFM::DiskBasicDirItemFM(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItemFAT8F(basic, sector, data)
{
}
DiskBasicDirItemFM::DiskBasicDirItemFM(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemFAT8F(basic, num, track, side, sector, secpos, data, unuse)
{
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemFM::InvalidateChars() const
{
	return wxT("\":()");
}
