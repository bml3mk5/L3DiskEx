/// @file basictype_pa.h
///
/// @brief disk basic type for PASOPIA T-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_PA_H
#define BASICTYPE_PA_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype_n88.h"


/** @class DiskBasicTypePA

@brief PASOPIA T-BASICの処理

DiskBasicParam
@li ReservedGroups : Group 予約済みにするグループ（クラスタ）番号

*/
class DiskBasicTypePA : public DiskBasicTypeN88
{
protected:
	DiskBasicTypePA() : DiskBasicTypeN88() {}
	DiskBasicTypePA(const DiskBasicType &src) : DiskBasicTypeN88(src) {}
public:
	DiskBasicTypePA(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	//@}

	/// @name check / assign FAT area
	//@{
	//@}

	/// @name file chain
	//@{
	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	//@}

	/// @name format
	//@{
	//@}

	/// @name data access (read / verify)
	//@{
	//@}

	/// @name save / write
	//@{
	//@}
};

#endif /* BASICTYPE_PA_H */
