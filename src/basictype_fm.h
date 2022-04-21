/// @file basictype_fm.h
///
/// @brief disk basic type for F-BASIC 
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_FM_H_
#define _BASICTYPE_FM_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat8.h"


/** @class DiskBasicTypeFM

@brief F-BASICの処理

DiskBasicParam
@li IDSectorPosition : IDセクタの論理セクタ番号
@li IDString         : IDセクタの最初の文字列

*/
class DiskBasicTypeFM : public DiskBasicTypeFAT8F
{
private:
	DiskBasicTypeFM() : DiskBasicTypeFAT8F() {}
	DiskBasicTypeFM(const DiskBasicType &src) : DiskBasicTypeFAT8F(src) {}
public:
	DiskBasicTypeFM(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name check / assign FAT area
	//@{
	/// FATエリアをチェック
	bool	CheckFat();
	//@}

	/// @name file chain
	//@{
	/// データ領域の開始セクタを計算
	int		CalcDataStartSectorPos();
	//@}

	/// @name format
	//@{
	/// セクタデータを埋めた後の個別処理
	bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_FM_H_ */
