/// @file basicdiritem_mz_base.h
///
/// @brief disk basic directory item for MZ Base
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_MZ_BASE_H_
#define _BASICDIRITEM_MZ_BASE_H_

#include "basicdiritem.h"


/// ディレクトリ１アイテム MZ Base
class DiskBasicDirItemMZBase : public DiskBasicDirItem
{
protected:
	DiskBasicDirItemMZBase() : DiskBasicDirItem() {}
	DiskBasicDirItemMZBase(const DiskBasicDirItemMZBase &src) : DiskBasicDirItem(src) {}

	/// データ内にファイルサイズをセット
	virtual void	SetFileSizeBase(int val) {};
	/// データ内のファイルサイズを返す
	virtual int		GetFileSizeBase() const { return 0; }
	/// ファイルサイズとグループ数を計算する前処理
	virtual void	PreCalcFileSize();
	/// グループ取得計算前処理
	virtual void	PreCalcAllGroups(int &calc_flags, wxUint32 &group_num, int &remain, int &sec_size, void **user_data);
	/// グループ取得計算中処理
	virtual void	CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data);
	/// グループ取得計算後処理
	virtual void	PostCalcAllGroups(void *user_data);

public:
	DiskBasicDirItemMZBase(DiskBasic *basic);
	DiskBasicDirItemMZBase(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemMZBase(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// 削除
	virtual bool	Delete(wxUint8 code);
	/// ファイル名が一致するか
	virtual bool	IsSameFileName(const DiskBasicFileName &filename) const;
	/// ファイルパスから内部ファイル名を生成する
	virtual wxString RemakeFileNameStr(const wxString &filepath) const;
	/// ファイル名に設定できない文字を文字列にして返す
	virtual wxString InvalidateChars() const;

	/// ディレクトリを初期化 未使用にする
	virtual void	InitialData();

	/// ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// ファイルサイズとグループ数を計算する
	virtual void	CalcFileSize();

	/// 指定ディレクトリのすべてのグループを取得
	virtual void	GetAllGroups(DiskBasicGroups &group_items);

	/// アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// ファイル名を編集できるか
	virtual bool	IsFileNameEditable() const;

};

#endif /* _BASICDIRITEM_MZ_BASE_H_ */
