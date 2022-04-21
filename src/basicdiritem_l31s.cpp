/// @file basicdiritem_l31s.cpp
///
/// @brief disk basic directory item for L3 BASIC 1S
///
#include "basicdiritem_l31s.h"
#include "basicfmt.h"
#include "charcodes.h"

///
///
///
DiskBasicDirItemL31S::DiskBasicDirItemL31S(DiskBasic *basic)
	: DiskBasicDirItemFAT8F(basic)
{
}
DiskBasicDirItemL31S::DiskBasicDirItemL31S(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItemFAT8F(basic, sector, data)
{
}
DiskBasicDirItemL31S::DiskBasicDirItemL31S(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemFAT8F(basic, num, track, side, sector, secpos, data, unuse)
{
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemL31S::InvalidateChars()
{
	return wxT("\":");
}

/// ダイアログ入力前のファイル名を変換 大文字にする
void DiskBasicDirItemL31S::ConvertToFileNameStr(wxString &filename)
{
	filename = filename.Upper();
}
