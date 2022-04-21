﻿/// @file basictype_fm.h
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

DiskBasicParam 固有パラメータ
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
	/// ディスクから各パラメータを取得＆必要なパラメータを計算
	double	ParseParamOnDisk(bool is_formatting);
	/// FATエリアをチェック
	double 	CheckFat(bool is_formatting);
	//@}

	/// @name file chain
	//@{
	/// データ領域の開始セクタを計算
	int		CalcDataStartSectorPos();
	/// スキップするトラック番号
	int		CalcSkippedTrack();
	//@}

	/// @name format
	//@{
	/// セクタデータを埋めた後の個別処理
	bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_FM_H_ */
