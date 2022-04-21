/// @file basictype_magical.h
///
/// @brief disk basic type for Magical DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_MAGICAL_H_
#define _BASICTYPE_MAGICAL_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_xdos.h"


/** @class DiskBasicTypeMAGICAL

@brief Magical DOSの処理

DiskBasicParam
@li DirStartPositionOnRoot : ルートディレクトリ開始セクタのエントリの開始位置
@li DirStartPosition       : サブディレクトリ開始セクタのエントリの開始位置
@li SubDirGroupSize        : サブディレクトリの初期グループ数

*/
class DiskBasicTypeMAGICAL : public DiskBasicTypeXDOS
{
private:
	DiskBasicTypeMAGICAL() : DiskBasicTypeXDOS() {}
	DiskBasicTypeMAGICAL(const DiskBasicType &src) : DiskBasicTypeXDOS(src) {}
public:
	DiskBasicTypeMAGICAL(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief 未使用が連続している位置をさがす
	wxUint32 GetContinuousArea(wxUint32 group_size);
	//@}
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	double 	CheckFat(bool is_formatting);
	//@}

	/// @name disk size
	//@{
	//@}

	/// @name file chain
	//@{
	//@}

	/// @name directory
	//@{
	/// @brief サブディレクトリを作成する前の準備を行う
	bool	PrepareToMakeDirectory(DiskBasicDirItem *item);
	/// @brief サブディレクトリを作成した後の個別処理
	void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	//@}

	/// @name save / write
	//@{
	//@}
};

#endif /* _BASICTYPE_MAGICAL_H_ */
