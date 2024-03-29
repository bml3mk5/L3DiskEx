/// @file basicdiritem_mz_base.h
///
/// @brief disk basic directory item for MZ Base
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_MZ_BASE_H
#define BASICDIRITEM_MZ_BASE_H

#include "basicdiritem.h"


class wxChoice;

/// ディレクトリ１アイテム MZ Base
class DiskBasicDirItemMZBase : public DiskBasicDirItem
{
protected:
	DiskBasicDirItemMZBase() : DiskBasicDirItem() {}
	DiskBasicDirItemMZBase(const DiskBasicDirItemMZBase &src) : DiskBasicDirItem(src) {}

	/// @brief データ内にファイルサイズをセット
	virtual void	SetFileSizeBase(int val) {};
	/// @brief データ内のファイルサイズを返す
	virtual int		GetFileSizeBase() const { return 0; }
	/// @brief ファイルサイズとグループ数を計算する前処理
	virtual void	PreCalcFileSize();
	/// @brief グループ取得計算前処理
	virtual void	PreCalcAllGroups(int &calc_flags, wxUint32 &group_num, int &remain, int &sec_size, void **user_data);
	/// @brief グループ取得計算中処理
	virtual void	CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data);
	/// @brief グループ取得計算後処理
	virtual void	PostCalcAllGroups(void *user_data);

public:
	DiskBasicDirItemMZBase(DiskBasic *basic);
	DiskBasicDirItemMZBase(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemMZBase(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief 削除
	virtual bool	Delete();

	/// @brief ディレクトリを初期化 未使用にする
	virtual void	InitialData();

	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);

	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief ファイル名を編集できるか
	virtual bool	IsFileNameEditable() const;

};

#endif /* BASICDIRITEM_MZ_BASE_H */
