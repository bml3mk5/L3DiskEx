/// @file basicdiritem_mz_base.h
///
/// @brief disk basic directory item for MZ Base
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_MZ_BASE_H_
#define _BASICDIRITEM_MZ_BASE_H_

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

	/// @brief 属性の選択肢を作成する（プロパティダイアログ用）
	static void		CreateChoiceForAttrDialog(DiskBasic *basic, const char *list[], int end_pos, wxArrayString &types1);
	/// @brief 属性の選択肢を選ぶ（プロパティダイアログ用）
	static int		SelectChoiceForAttrDialog(DiskBasic *basic, wxChoice *choice, int file_type_1, int end_pos, int unknown_pos);
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	static int		CalcSpecialOriginalTypeFromPos(DiskBasic *basic, int pos, int end_pos);
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	static int		CalcSpecialFileTypeFromPos(DiskBasic *basic, int pos, int end_pos);

public:
	DiskBasicDirItemMZBase(DiskBasic *basic);
	DiskBasicDirItemMZBase(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemMZBase(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief 削除
	virtual bool	Delete(wxUint8 code);

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

#endif /* _BASICDIRITEM_MZ_BASE_H_ */
