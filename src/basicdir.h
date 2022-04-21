/// @file basicdir.h
///
/// @brief disk basic directory
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIR_H_
#define _BASICDIR_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"
#include "basicdiritem.h"
#include "basicfat.h"
#include "diskd88.h"


class DiskBasic;
class DiskBasicType;
class DiskBasicFormat;
class DiskBasicFat;
class DiskBasicDirItems;
class DiskBasicFileName;
class DiskBasicGroups;

/// ディレクトリ
class DiskBasicDir
{
private:
	DiskBasic		*basic;
	DiskBasicFat	*fat;

	const DiskBasicFormat *format_type;	///< フォーマットタイプ

	DiskBasicDirItem	*root;			///< ルートディレクトリの仮想的なアイテム

	DiskBasicDirItem	*current_item;	///< 現ディレクトリのアイテム

	DiskBasicGroups		root_groups;	///< ルートディレクトリのあるセクタリスト

	DiskBasicDir();
public:
	DiskBasicDir(DiskBasic *basic);
	~DiskBasicDir();

	/// ディレクトリアイテムを新規に作成
	DiskBasicDirItem *NewItem();
	/// ディレクトリアイテムを新規に作成してアサインする
	DiskBasicDirItem *NewItem(DiskD88Sector *newsec, int newpos, wxUint8 *newdata);
	/// ディレクトリアイテムを新規に作成してアサインする
	DiskBasicDirItem *NewItem(int newnum, int newtrack, int newside, DiskD88Sector *newsec, int newpos, wxUint8 *newdata, bool &unuse);

	/// ルートディレクトリのアイテムを返す
	DiskBasicDirItem		*GetRootItem() const;
	/// ルートディレクトリの一覧を返す
	DiskBasicDirItems		*GetRootItems(DiskBasicDirItem **dir_item = NULL);
	/// カレントディレクトリのアイテムを返す
	DiskBasicDirItem		*GetCurrentItem() const;
	/// カレントディレクトリの一覧を返す
	DiskBasicDirItems		*GetCurrentItems(DiskBasicDirItem **dir_item = NULL);
	/// カレントディレクトリの全ディレクトアイテムをクリア
	void					EmptyCurrent();

	/// ルートをカレントにする
	void					SetCurrentAsRoot();

	/// 親ディレクトリのアイテムを返す
	DiskBasicDirItem		*GetParentItem() const;

	/// ディレクトリアイテムのポインタを返す
	DiskBasicDirItem *ItemPtr(size_t idx);
	/// カレントディレクトリ内で未使用のディレクトリアイテムを返す
	DiskBasicDirItem *GetEmptyItemOnCurrent(DiskBasicDirItem **next_item);
	/// ルートディレクトリ内で未使用のディレクトリアイテムを返す
	DiskBasicDirItem *GetEmptyItemOnRoot(DiskBasicDirItem **next_item);
	/// 未使用のディレクトリアイテムを返す
	DiskBasicDirItem *GetEmptyItem(const DiskBasicDirItems *items, DiskBasicDirItem **next_item);

	/// 現在のディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicFileName &filename, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// 指定したディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicDirItem *dir_item, const DiskBasicFileName &filename, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// 現在のディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicDirItem *target_item, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// 指定したディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicDirItem *dir_item, const DiskBasicDirItem *target_item, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// 現在のディレクトリ内に同じファイル名(拡張子除く)が既に存在するか
	DiskBasicDirItem *FindName(const wxString &name, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// 指定したディレクトリ内に同じファイル名(拡張子除く)が既に存在するか
	DiskBasicDirItem *FindName(const DiskBasicDirItem *dir_item, const wxString &name, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// 現在のディレクトリ内の属性に一致するファイルを検索
	DiskBasicDirItem *FindFileByAttr(int file_type, int mask, DiskBasicDirItem *prev_item = NULL);
	/// ルートディレクトリ内の属性に一致するファイルを検索
	DiskBasicDirItem *FindFileByAttrOnRoot(int file_type, int mask, DiskBasicDirItem *prev_item = NULL);
	/// 指定したディレクトリ内の属性に一致するファイルを検索
	DiskBasicDirItem *FindFileByAttr(const DiskBasicDirItem *dir_item, int file_type, int mask, DiskBasicDirItem *prev_item = NULL);
	/// ルートディレクトリのチェック
	double		CheckRoot(DiskBasicType *type, int start_sector, int end_sector, bool is_formatting);
	/// ルートディレクトリをアサイン
	bool		AssignRoot(DiskBasicType *type, int start_sector, int end_sector);
	/// ルートディレクトリをアサイン
	bool		AssignRoot(DiskBasicType *type);
	/// ディレクトリのチェック
	double		Check(DiskBasicType *type, DiskBasicGroups &group_items);
	/// ディレクトリをアサイン
	bool		Assign(DiskBasicType *type, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// ディレクトリをアサイン
	bool		Assign(DiskBasicType *type, DiskBasicDirItem *dir_item);
	/// ディレクトリをアサイン
	bool		Assign(DiskBasicDirItem *dir_item);

	/// ルートディレクトリを初期化
	void        ClearRoot();
	/// ルートディレクトリ領域を指定コードで埋める
	void        Fill(int start_sector, int end_sector, wxUint8 code);

	/// ディレクトリを移動する
	bool		Change(DiskBasicDirItem * &dst_item);

	/// ディレクトリの拡張ができるか
	bool		CanExpand();
	/// ディレクトリを拡張する
	bool		Expand();

	/// フォーマット種類を設定
	void		SetFormatType(const DiskBasicFormat *val) { format_type = val; }
	/// フォーマット種類を得る
	const DiskBasicFormat *GetFormatType() const { return format_type; }

	/// ディレクトリの占有サイズを計算する
	int			CalcSize();
};

#endif /* _BASICDIR_H_ */
