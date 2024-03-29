/// @file basictype_magical.h
///
/// @brief disk basic type for Magical DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_MAGICAL_H
#define BASICTYPE_MAGICAL_H

#include "../common.h"
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
	virtual wxUint32 GetContinuousArea(wxUint32 group_size);
	//@}
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
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
	virtual bool	PrepareToMakeDirectory(DiskBasicDirItem *item);
	/// @brief サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	//@}

	/// @name save / write
	//@{
	//@}
};

#endif /* BASICTYPE_MAGICAL_H */
