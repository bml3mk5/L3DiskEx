/// @file basictype_l31s.h
///
/// @brief disk basic type for L3 BASIC 1S
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_L31S_H_
#define _BASICTYPE_L31S_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype_fat8.h"


/** @class DiskBasicTypeL31S

@brief LEVEL-3 BASIC 1S(片面・単密度)の処理

*/
class DiskBasicTypeL31S : public DiskBasicTypeFAT8F
{
private:
	DiskBasicTypeL31S() : DiskBasicTypeFAT8F() {}
	DiskBasicTypeL31S(const DiskBasicType &src) : DiskBasicTypeFAT8F(src) {}
public:
	DiskBasicTypeL31S(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name check / assign FAT area
	//@{
	/// ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	/// FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	//@}
};

#endif /* _BASICTYPE_L31S_H_ */
