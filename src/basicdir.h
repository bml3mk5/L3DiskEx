/// @file basicdir.h
///
/// @brief disk basic directory
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
class DiskBasicFat;
class DiskBasicDirItems;
class DiskBasicGroups;

/// ディレクトリ
class DiskBasicDir
{
private:
	DiskBasic		*basic;
	DiskBasicFat	*fat;

	int start_sector_pos;	///< ルートディレクトリの開始セクタ位置
	int end_sector_pos;		///< ルートディレクトリの終了セクタ位置
	int unique_number;		///< Assignするごとに+1 

	DiskBasicFormatType format_type;	///< フォーマットタイプ

	DiskBasicDirItem	*parent_item;	///< 親ディレクトリのアイテム（ルートの場合NULL）
	DiskBasicGroups		groups;			///< カレントディレクトリのあるグループ番号
	DiskBasicDirItems	items;			///< ディレクトリ内の各アイテム

	DiskBasicDir();
public:
	DiskBasicDir(DiskBasic *basic);
	~DiskBasicDir();

	/// ディレクトリアイテムを新規に作成
	DiskBasicDirItem *NewItem();
	/// ディレクトリアイテムを新規に作成してアサインする
	DiskBasicDirItem *NewItem(DiskD88Sector *newsec, wxUint8 *newdata);
	/// ディレクトリアイテムを新規に作成してアサインする
	DiskBasicDirItem *NewItem(int newnum, int newtrack, int newside, DiskD88Sector *newsec, int newpos, wxUint8 *newdata, bool &unuse);
	/// ディレクトリアイテムを新規に作成してアサインする
	DiskBasicDirItem *AssignItem(int newnum, int newtrack, int newside, DiskD88Sector *newsec, int newpos, wxUint8 *newdata, bool &unuse);

	/// ディレクトリアイテムのポインタを返す
	DiskBasicDirItem *ItemPtr(size_t idx);
	/// 未使用のディレクトリアイテムを返す
	DiskBasicDirItem *GetEmptyItemPtr(DiskBasicDirItem **next_item);

	/// ディレクトリアイテムの数
	size_t		Count();
	/// 全ディレクトアイテムをクリア
	void		Clear();
	/// 全ディレクトアイテムをクリア
	void		Empty();

	/// 現在のディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const wxString &filename, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// 現在のディレクトリ内に同じファイル名が既に存在するか
	DiskBasicDirItem *FindFile(const DiskBasicDirItem &target_item, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// 現在のディレクトリ内に同じファイル名(拡張子除く)が既に存在するか
	DiskBasicDirItem *FindName(const wxString &name, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);
	/// ルートディレクトリのチェック
	bool		CheckRoot(DiskBasicType *type, int start_sector, int end_sector);
	/// ルートディレクトリをアサイン
	bool		AssignRoot(DiskBasicType *type, int start_sector, int end_sector);
	/// ルートディレクトリをアサイン
	bool		AssignRoot(DiskBasicType *type);
	/// ディレクトリのチェック
	bool		Check(DiskBasicType *type, DiskBasicGroups &group_items);
	/// ディレクトリをアサイン
	bool		Assign(DiskBasicType *type, DiskBasicGroups &group_items);

	/// ルートディレクトリを初期化
	void        ClearRoot();
	/// ルートディレクトリ領域を指定コードで埋める
	void        Fill(int start_sector, int end_sector, wxUint8 code);

	/// 親ディレクトリのアイテムを返す
	const DiskBasicDirItem *GetParentItem() const { return parent_item; }
	/// 親ディレクトリのアイテムを設定
	void		SetParentItem(const DiskBasicDirItem *val);

	/// ユニーク番号を得る
	int			GetUniqueNumber() const { return unique_number; }

	/// フォーマット種類を設定
	void		SetFormatType(DiskBasicFormatType val) { format_type = val; }
	/// フォーマット種類を得る
	DiskBasicFormatType GetFormatType() const { return format_type; }

	/// アイテムリストを返す
	const DiskBasicDirItems &GetItems() const { return items; }
};

#endif /* _BASICDIR_H_ */
