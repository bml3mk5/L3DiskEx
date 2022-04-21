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
	DiskBasicDirItemFM(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemFM(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

//	/// @brief ファイル名に設定できない文字を文字列にして返す
//	wxString	GetDefaultInvalidateChars() const;
};

#endif /* _BASICDIRITEM_FM_H_ */
