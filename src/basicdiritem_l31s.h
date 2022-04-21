/// @file basicdiritem_l31s.h
///
/// @brief disk basic directory item for L3 BASIC 1S
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_L31S_H_
#define _BASICDIRITEM_L31S_H_

#include "basicdiritem_fat8.h"


/// ディレクトリ１アイテム L3 BASIC 単密度 1S
class DiskBasicDirItemL31S : public DiskBasicDirItemFAT8F
{
private:
	DiskBasicDirItemL31S() : DiskBasicDirItemFAT8F() {}
	DiskBasicDirItemL31S(const DiskBasicDirItemL31S &src) : DiskBasicDirItemFAT8F(src) {}

public:
	DiskBasicDirItemL31S(DiskBasic *basic);
	DiskBasicDirItemL31S(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemL31S(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);
};

#endif /* _BASICDIRITEM_L31S_H_ */
