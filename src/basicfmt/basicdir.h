/// @file basicdir.h
///
/// @brief disk basic directory
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIR_H_
#define _BASICDIR_H_

#include "../common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"
#include "basicdiritem.h"
#include "basicfat.h"
#include "../diskd88.h"


class DiskBasic;
class DiskBasicType;
class DiskBasicFormat;
class DiskBasicFat;
class DiskBasicDirItems;
class DiskBasicFileName;
class DiskBasicGroups;

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリアクセス
class DiskBasicDir
{
private:
	DiskBasic		*basic;
	DiskBasicFat	*fat;

	const DiskBasicFormat *format_type;	///< フォーマットタイプ

	DiskBasicDirItem	*root;			///< ルートディレクトリの仮想的なアイテム

	DiskBasicDirItem	*current_item;	///< 現ディレクトリのアイテム

//	DiskBasicGroups		root_groups;	///< ルートディレクトリのあるセクタリスト

	DiskBasicDir();
public:
	DiskBasicDir(DiskBasic *basic);
	~DiskBasicDir();

	/// @brief ディレクトリアイテムを新規に作成
	DiskBasicDirItem *NewItem();
	/// @brief ディレクトリアイテムを新規に作成してアサインする
	DiskBasicDirItem *NewItem(DiskD88Sector *n_sector, int n_pos, wxUint8 *n_data);
	/// @brief ディレクトリアイテムを新規に作成してアサインする
	DiskBasicDirItem *NewItem(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_pos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief ルートディレクトリのアイテムを返す
	DiskBasicDirItem		*GetRootItem() const;
	/// @brief ルートディレクトリの一覧を返す
	DiskBasicDirItems		*GetRootItems(DiskBasicDirItem **dir_item = NULL);
	/// @brief カレントディレクトリのアイテムを返す
	DiskBasicDirItem		*GetCurrentItem() const;
	/// @brief カレントディレクトリの一覧を返す
	DiskBasicDirItems		*GetCurrentItems(DiskBasicDirItem **dir_item = NULL);
	/// @brief カレントディレクトリの全ディレクトアイテムをクリア
	void					EmptyCurrent();

	/// @brief ルートをカレントにする
	void					SetCurrentAsRoot();

	/// @brief 親ディレクトリのアイテムを返す
	DiskBasicDirItem		*GetParentItem() const;

	/// @brief ディレクトリアイテムのポインタを返す
	DiskBasicDirItem *ItemPtr(size_t idx);
	/// @brief カレントディレクトリ内で未使用のディレクトリアイテムを返す
	DiskBasicDirItem *GetEmptyItemOnCurrent(DiskBasicDirItem *pitem, DiskBasicDirItem **next_item);
	/// @brief ルートディレクトリ内で未使用のディレクトリアイテムを返す
	DiskBasicDirItem *GetEmptyItemOnRoot(DiskBasicDirItem *pitem, DiskBasicDirItem **next_item);
	/// @brief 未使用のディレクトリアイテムを返す
	DiskBasicDirItem *GetEmptyItem(DiskBasicDirItem *parent, DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item);

	/// @brief 現在のディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicFileName &filename, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// @brief 指定したディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicDirItem *dir_item, const DiskBasicFileName &filename, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// @brief 現在のディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicDirItem *target_item, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// @brief 指定したディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicDirItem *dir_item, const DiskBasicDirItem *target_item, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// @brief 現在のディレクトリ内に同じファイル名(拡張子除く)が既に存在するか
	DiskBasicDirItem *FindName(const wxString &name, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// @brief 指定したディレクトリ内に同じファイル名(拡張子除く)が既に存在するか
	DiskBasicDirItem *FindName(const DiskBasicDirItem *dir_item, const wxString &name, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// @brief 現在のディレクトリ内の属性に一致するファイルを検索
	DiskBasicDirItem *FindFileByAttr(int file_type, int mask, DiskBasicDirItem *prev_item = NULL);
	/// @brief ルートディレクトリ内の属性に一致するファイルを検索
	DiskBasicDirItem *FindFileByAttrOnRoot(int file_type, int mask, DiskBasicDirItem *prev_item = NULL);
	/// @brief 指定したディレクトリ内の属性に一致するファイルを検索
	DiskBasicDirItem *FindFileByAttr(const DiskBasicDirItem *dir_item, int file_type, int mask, DiskBasicDirItem *prev_item = NULL);
	/// @brief ルートディレクトリのチェック
	double		CheckRoot(DiskBasicType *type, int start_sector, int end_sector, bool is_formatting);
	/// @brief ルートディレクトリをアサイン
	bool		AssignRoot(DiskBasicType *type, int start_sector, int end_sector);
	/// @brief ルートディレクトリをアサイン
	bool		AssignRoot(DiskBasicType *type);
	/// @brief ルートディレクトリをリリース
	bool		ReleaseRoot(DiskBasicType *type);
	/// @brief ディレクトリのチェック
	double		Check(DiskBasicType *type, DiskBasicGroups &group_items);
	/// @brief ディレクトリをアサイン
	bool		Assign(DiskBasicType *type, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief ディレクトリをアサイン
	bool		Assign(DiskBasicType *type, DiskBasicDirItem *dir_item);
	/// @brief ディレクトリをアサイン
	bool		Assign(DiskBasicDirItem *dir_item);

	/// @brief ルートディレクトリを初期化
	void        ClearRoot();
	/// @brief ルートディレクトリ領域を指定コードで埋める
	void        Fill(int start_sector, int end_sector, wxUint8 code);

	/// @brief ディレクトリを移動する
	bool		Change(DiskBasicDirItem * &dst_item);

	/// @brief ディレクトリの拡張ができるか
	bool		CanExpand();
	/// @brief ディレクトリを拡張する
	bool		Expand();

	/// @brief フォーマット種類を設定
	void		SetFormatType(const DiskBasicFormat *val) { format_type = val; }
	/// @brief フォーマット種類を得る
	const DiskBasicFormat *GetFormatType() const { return format_type; }

	/// @brief ディレクトリの占有サイズを計算する
	int			CalcSize();
};

#endif /* _BASICDIR_H_ */
