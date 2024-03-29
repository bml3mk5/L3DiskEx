/// @file basictype_l32d.h
///
/// @brief disk basic type for L3 BASIC 2D
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_L32D_H
#define BASICTYPE_L32D_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype_fat8.h"


/** @class DiskBasicTypeL32D

@brief LEVEL-3 BASIC 2D(両面・倍密度)の処理

*/
class DiskBasicTypeL32D : public DiskBasicTypeFAT8
{
private:
	DiskBasicTypeL32D() : DiskBasicTypeFAT8() {}
	DiskBasicTypeL32D(const DiskBasicType &src) : DiskBasicTypeFAT8(src) {}
public:
	DiskBasicTypeL32D(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	/// FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// 空きFAT位置を返す
	virtual wxUint32 GetEmptyGroupNumber();
	//@}

	/// @name check / assign FAT area
	//@{
	/// 管理エリアのトラック番号からグループ番号を計算
	virtual wxUint32 CalcManagedStartGroup();
	//@}

	/// @name file chain
	//@{
	/// スキップするトラック番号
	virtual int		CalcSkippedTrack();
	/// データ領域の開始セクタを計算
	virtual int		CalcDataStartSectorPos();
	//@}

	/// @name save / write
	//@{
	/// @brief グループ確保時に最後のグループ番号を計算する
	virtual wxUint32 CalcLastGroupNumber(wxUint32 group_num, int &size_remain);
	//@}
};

#endif /* BASICTYPE_L32D_H */
