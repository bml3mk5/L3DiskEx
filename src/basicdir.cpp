/// @file basicdir.cpp
///
/// @brief disk basic directory
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdir.h"
#include "basicdiritem_l31s.h"
#include "basicdiritem_l32d.h"
#include "basicdiritem_fm.h"
#include "basicdiritem_msdos.h"
#include "basicdiritem_msx.h"
#include "basicdiritem_n88.h"
#include "basicdiritem_x1hu.h"
#include "basicdiritem_mz.h"
#include "basicdiritem_flex.h"
#include "basicdiritem_os9.h"
#include "basicdiritem_cpm.h"
#include "basicdiritem_tfdos.h"
#include "basicdiritem_cdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"


//
//
//
DiskBasicDir::DiskBasicDir(DiskBasic *basic)
{
	this->basic = basic;
	this->fat = basic->GetFat();

	this->root = NULL;

	this->format_type = NULL;
	this->current_item = NULL;
}
DiskBasicDir::~DiskBasicDir()
{
	delete root;
}
/// ディレクトリアイテムを新規に作成
DiskBasicDirItem *DiskBasicDir::NewItem()
{
	DiskBasicDirItem *item = NULL;

	int num = FORMAT_TYPE_NONE;
	if (format_type) num = format_type->GetTypeNumber();

	switch(num) {
	case FORMAT_TYPE_L3_1S:
		item = new DiskBasicDirItemL31S(basic);
		break;
	case FORMAT_TYPE_L3S1_2D:
		item = new DiskBasicDirItemL32D(basic);
		break;
	case FORMAT_TYPE_FM:
		item = new DiskBasicDirItemFM(basic);
		break;
	case FORMAT_TYPE_MSDOS:
		item = new DiskBasicDirItemMSDOS(basic);
		break;
	case FORMAT_TYPE_MSX:
		item = new DiskBasicDirItemMSX(basic);
		break;
	case FORMAT_TYPE_N88:
		item = new DiskBasicDirItemN88(basic);
		break;
	case FORMAT_TYPE_X1HU:
		item = new DiskBasicDirItemX1HU(basic);
		break;
	case FORMAT_TYPE_MZ:
		item = new DiskBasicDirItemMZ(basic);
		break;
	case FORMAT_TYPE_FLEX:
		item = new DiskBasicDirItemFLEX(basic);
		break;
	case FORMAT_TYPE_OS9:
		item = new DiskBasicDirItemOS9(basic);
		break;
	case FORMAT_TYPE_CPM:
		item = new DiskBasicDirItemCPM(basic);
		break;
	case FORMAT_TYPE_TFDOS:
		item = new DiskBasicDirItemTFDOS(basic);
		break;
	case FORMAT_TYPE_CDOS:
		item = new DiskBasicDirItemCDOS(basic);
		break;
//	default:
//		item = new DiskBasicDirItem(basic);
//		break;
	}
	return item;
}
/// ディレクトリアイテムを新規に作成してアサインする
/// @param [in] newsec  セクタ
/// @param [in] newdata セクタ内のバッファ
DiskBasicDirItem *DiskBasicDir::NewItem(DiskD88Sector *newsec, wxUint8 *newdata)
{
	DiskBasicDirItem *item = NULL;

	int num = FORMAT_TYPE_NONE;
	if (format_type) num = format_type->GetTypeNumber();

	switch(num) {
	case FORMAT_TYPE_L3_1S:
		item = new DiskBasicDirItemL31S(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_L3S1_2D:
		item = new DiskBasicDirItemL32D(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_FM:
		item = new DiskBasicDirItemFM(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_MSDOS:
		item = new DiskBasicDirItemMSDOS(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_MSX:
		item = new DiskBasicDirItemMSX(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_N88:
		item = new DiskBasicDirItemN88(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_X1HU:
		item = new DiskBasicDirItemX1HU(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_MZ:
		item = new DiskBasicDirItemMZ(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_FLEX:
		item = new DiskBasicDirItemFLEX(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_OS9:
		item = new DiskBasicDirItemOS9(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_CPM:
		item = new DiskBasicDirItemCPM(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_TFDOS:
		item = new DiskBasicDirItemTFDOS(basic, newsec, newdata);
		break;
	case FORMAT_TYPE_CDOS:
		item = new DiskBasicDirItemCDOS(basic, newsec, newdata);
		break;
//	default:
//		item = new DiskBasicDirItem(basic, newsec, newdata);
//		break;
	}
	return item;
}
/// ディレクトリアイテムを新規に作成してアサインする
/// @param [in] newnum   通し番号
/// @param [in] newtrack トラック番号
/// @param [in] newside  サイド番号
/// @param [in] newsec   セクタ
/// @param [in] newpos   セクタ内での位置
/// @param [in] newdata  セクタ内のバッファ
/// @param [out] unuse   未使用か   
DiskBasicDirItem *DiskBasicDir::NewItem(int newnum, int newtrack, int newside, DiskD88Sector *newsec, int newpos, wxUint8 *newdata, bool &unuse)
{
	DiskBasicDirItem *item = NULL;

	int num = FORMAT_TYPE_NONE;
	if (format_type) num = format_type->GetTypeNumber();

	switch(num) {
	case FORMAT_TYPE_L3_1S:
		item = new DiskBasicDirItemL31S(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_L3S1_2D:
		item = new DiskBasicDirItemL32D(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_FM:
		item = new DiskBasicDirItemFM(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_MSDOS:
		item = new DiskBasicDirItemMSDOS(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_MSX:
		item = new DiskBasicDirItemMSX(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_N88:
		item = new DiskBasicDirItemN88(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_X1HU:
		item = new DiskBasicDirItemX1HU(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_MZ:
		item = new DiskBasicDirItemMZ(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_FLEX:
		item = new DiskBasicDirItemFLEX(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_OS9:
		item = new DiskBasicDirItemOS9(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_CPM:
		item = new DiskBasicDirItemCPM(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_TFDOS:
		item = new DiskBasicDirItemTFDOS(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	case FORMAT_TYPE_CDOS:
		item = new DiskBasicDirItemCDOS(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
//	default:
//		item = new DiskBasicDirItem(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
//		break;
	}
	return item;
}
#if 0
/// ディレクトリアイテムを新規に作成してアサインする
DiskBasicDirItem *DiskBasicDir::AssignItem(int newnum, int newtrack, int newside, DiskD88Sector *newsec, int newpos, wxUint8 *newdata, bool &unuse)
{
	DiskBasicDirItem *item = NewItem(newnum, newtrack, newside, newsec, newpos, newdata, unuse);
	items.Add(item);
	return item;
}
#endif
/// ルートディレクトリのアイテムを返す
DiskBasicDirItem *DiskBasicDir::GetRootItem() const
{
	return root;
}
/// ルートディレクトリの一覧を返す
DiskBasicDirItems *DiskBasicDir::GetRootItems(DiskBasicDirItem **dir_item)
{
	DiskBasicDirItem *item = GetRootItem();
	if (!item) return NULL;
	if (dir_item) *dir_item = item;
	return item->GetChildren();
}
/// カレントディレクトリのアイテムを返す
DiskBasicDirItem *DiskBasicDir::GetCurrentItem() const
{
	return current_item;
}
/// カレントディレクトリの一覧を返す
DiskBasicDirItems *DiskBasicDir::GetCurrentItems(DiskBasicDirItem **dir_item)
{
	DiskBasicDirItem *item = GetCurrentItem();
	if (!item) return NULL;
	if (dir_item) *dir_item = item;
	return item->GetChildren();
}
/// カレントディレクトリの全ディレクトリアイテムをクリア
void DiskBasicDir::EmptyCurrent()
{
	DiskBasicDirItem *item = GetCurrentItem();
	if (item) item->EmptyChildren();
}
/// ルートをカレントにする
void DiskBasicDir::SetCurrentAsRoot()
{
	current_item = root;
}
/// 親ディレクトリのアイテムを返す
DiskBasicDirItem *DiskBasicDir::GetParentItem() const
{
	DiskBasicDirItem *item = NULL;
	if (current_item) {
		item = current_item->GetParent();
	}
	return item;
}

/// ディレクトリアイテムのポインタを返す
/// @param [in] idx インデックス
DiskBasicDirItem *DiskBasicDir::ItemPtr(size_t idx)
{
	DiskBasicDirItems *items = GetCurrentItems();
	if (!items || idx >= items->Count()) return NULL;
	return items->Item(idx);
}
/// 未使用のディレクトリアイテムを返す
/// @param [out] next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicDir::GetEmptyItemPtr(DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	DiskBasicDirItems *items = GetCurrentItems();
	if (items) {
		for(size_t i=0; i < items->Count(); i++) {
			DiskBasicDirItem *item = items->Item(i);
			if (!item->IsUsed()) {
				match_item = item;
				if (next_item) {
					i++;
					if (i < items->Count()) {
						*next_item = items->Item(i);
					} else {
						*next_item = NULL;
					}
				}
				break;
			}
		}
	}
	return match_item;
}

#if 0
/// ディレクトリアイテムの数
size_t DiskBasicDir::Count()
{
	return items.Count();
}
/// 全ディレクトアイテムをクリア
void DiskBasicDir::Clear()
{
	for(size_t i=0; i<items.Count(); i++) {
		DiskBasicDirItem *p = items.Item(i);
		delete p;
	}
	items.Clear();

	unique_number++;
}
/// 全ディレクトアイテムをクリア
void DiskBasicDir::Empty()
{
	for(size_t i=0; i<items.Count(); i++) {
		DiskBasicDirItem *p = items.Item(i);
		delete p;
	}
	items.Empty();

	unique_number++;
}
#endif

/// 現在のディレクトリ内に同じファイル名が既に存在するか
/// @param [in]  filename     ファイル名
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFile(const DiskBasicFileName &filename, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	return FindFile(GetCurrentItem(), filename, exclude_item, next_item);
}

/// 指定したディレクトリ内に同じファイル名が既に存在するか
/// @param [in]  dir_item     検索対象のディレクトリアイテム
/// @param [in]  filename     ファイル名
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFile(const DiskBasicDirItem *dir_item, const DiskBasicFileName &filename, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	const DiskBasicDirItems *items = dir_item->GetChildren();
	if (items) {
		for(size_t pos = 0; pos <items->Count(); pos++) {
			DiskBasicDirItem *item = items->Item(pos);
			if (item != exclude_item && item->IsSameFileName(filename)) {
				match_item = item;
				if (next_item) {
					pos++;
					if (pos < items->Count()) {
						*next_item = items->Item(pos);
					} else {
						*next_item = NULL;
					}
				}
				break;
			}
		}
	}
	return match_item;
}

/// 現在のディレクトリ内に同じファイル名が既に存在するか
/// @param [in]  target_item  検索対象アイテム
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFile(const DiskBasicDirItem &target_item, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	return FindFile(GetCurrentItem(), target_item, exclude_item, next_item);
}

/// 指定したディレクトリ内に同じファイル名が既に存在するか
/// @param [in]  dir_item     検索対象のディレクトリアイテム
/// @param [in]  target_item  検索対象アイテム
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFile(const DiskBasicDirItem *dir_item, const DiskBasicDirItem &target_item, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	const DiskBasicDirItems *items = dir_item->GetChildren();
	if (items) {
		for(size_t pos = 0; pos <items->Count(); pos++) {
			DiskBasicDirItem *item = items->Item(pos);
			if (item != exclude_item && item->IsSameFileName(target_item)) {
				match_item = item;
				if (next_item) {
					pos++;
					if (pos < items->Count()) {
						*next_item = items->Item(pos);
					} else {
						*next_item = NULL;
					}
				}
				break;
			}
		}
	}
	return match_item;
}

/// 現在のディレクトリ内に同じファイル名(拡張子除く)が既に存在するか
/// @param [in]  name         ファイル名
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindName(const wxString &name, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	return FindName(GetCurrentItem(), name, exclude_item, next_item);
}

/// 指定したディレクトリ内に同じファイル名(拡張子除く)が既に存在するか
/// @param [in]  dir_item     検索対象のディレクトリアイテム
/// @param [in]  name         ファイル名
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindName(const DiskBasicDirItem *dir_item, const wxString &name, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	const DiskBasicDirItems *items = dir_item->GetChildren();
	if (items) {
		for(size_t pos = 0; pos <items->Count(); pos++) {
			DiskBasicDirItem *item = items->Item(pos);
			if (item != exclude_item && item->IsSameName(name)) {
				match_item = item;
				if (next_item) {
					pos++;
					if (pos < items->Count()) {
						*next_item = items->Item(pos);
					} else {
						*next_item = NULL;
					}
				}
				break;
			}
		}
	}
	return match_item;
}

/// 現在のディレクトリ内の属性に一致するファイルを検索
/// @param [in]  file_type 検索対象の属性
/// @param [in]  mask      検索対象外にするビットマスク
/// @param [in]  prev_item 前回一致したアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFileByAttr(int file_type, int mask, DiskBasicDirItem *prev_item)
{
	return FindFileByAttr(GetCurrentItem(), file_type, mask, prev_item);
}

/// ルートディレクトリ内の属性に一致するファイルを検索
/// @param [in]  file_type 検索対象の属性
/// @param [in]  mask      検索対象外にするビットマスク
/// @param [in]  prev_item 前回一致したアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFileByAttrOnRoot(int file_type, int mask, DiskBasicDirItem *prev_item)
{
	return FindFileByAttr(GetRootItem(), file_type, mask, prev_item);
}

/// 指定したディレクトリ内の属性に一致するファイルを検索
/// @param [in]  dir_item  検索対象のディレクトリアイテム
/// @param [in]  file_type 検索対象の属性
/// @param [in]  mask      検索対象外にするビットマスク
/// @param [in]  prev_item 前回一致したアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFileByAttr(const DiskBasicDirItem *dir_item, int file_type, int mask, DiskBasicDirItem *prev_item)
{
	DiskBasicDirItem *match_item = NULL;
	size_t start = 0;
	const DiskBasicDirItems *items = dir_item->GetChildren();
	if (items) {
		if (prev_item) {
			start = (size_t)items->Index(prev_item);
			start++;
		}
		for(size_t pos = start; pos <items->Count(); pos++) {
			DiskBasicDirItem *item = items->Item(pos);
			if (item->GetFileAttr().MatchType(mask, file_type & mask)) {
				match_item = item;
				break;
			}
		}
	}
	return match_item;
}

/// ルートディレクトリのチェック
/// @param [in] type          DISK BASIC 種類
/// @param [in] start_sector  開始セクタ番号
/// @param [in] end_sector    終了セクタ番号
/// @param [in] is_formatting フォーマット中か
bool DiskBasicDir::CheckRoot(DiskBasicType *type, int start_sector, int end_sector, bool is_formatting)
{
	return type->CheckRootDirectory(start_sector, end_sector, root_groups, is_formatting);
}

/// ルートディレクトリをアサイン
/// @param [in] type         DISK BASIC 種類
/// @param [in] start_sector 開始セクタ番号
/// @param [in] end_sector   終了セクタ番号
bool DiskBasicDir::AssignRoot(DiskBasicType *type, int start_sector, int end_sector)
{
	delete root;
	root = NewItem();
	bool valid = type->AssignRootDirectory(start_sector, end_sector, root_groups, root);
	if (valid) {
		// ディレクトリツリー確定
		root->ValidDirectory(true);
		// グループを保持
		current_item = root;
	}
	return valid;
}

/// ルートディレクトリをアサイン
/// @param [in] type        DISK BASIC 種類
bool DiskBasicDir::AssignRoot(DiskBasicType *type)
{
//	return AssignRoot(type, start_sector_pos, end_sector_pos);
	delete root;
	root = NewItem();
	bool valid = type->AssignDirectory(true, root_groups, root);
	if (valid) {
		// ディレクトリツリー確定
		root->ValidDirectory(true);
		// グループを保持
		current_item = root;
	}
	return valid;
}

/// ディレクトリのチェック
/// @param [in] type        DISK BASIC 種類
/// @param [in] group_items グループのリスト
bool DiskBasicDir::Check(DiskBasicType *type, DiskBasicGroups &group_items)
{
	return type->CheckDirectory(false, group_items);
}

/// ディレクトリをアサイン
/// @param [in] type        DISK BASIC 種類
/// @param [in] group_items グループのリスト
/// @param [in] dir_item    ディレクトリのアイテム
bool DiskBasicDir::Assign(DiskBasicType *type, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool valid = type->AssignDirectory(false, group_items, dir_item);
	if (valid) {
		// ディレクトリツリー確定
		dir_item->ValidDirectory(true);
	}
	return valid;
}

/// ディレクトリをアサイン
bool DiskBasicDir::Assign(DiskBasicType *type, DiskBasicDirItem *dir_item)
{
	DiskBasicGroups group_items;
	dir_item->GetAllGroups(group_items);
	return Assign(type, group_items, dir_item);
}

/// ディレクトリをアサイン
bool DiskBasicDir::Assign(DiskBasicDirItem *dir_item)
{
	bool valid = true;
	DiskBasicType *type = basic->GetType();

	if (!type->IsRootDirectory(dir_item->GetStartGroup())) {
		// サブディレクトリのアイテムをアサイン
		if (dir_item->GetChildren() == NULL) {
			DiskBasicGroups groups;
			dir_item->GetAllGroups(groups);

			if (!Check(type, groups)) {
				return false;
			}
			valid = Assign(type, groups, dir_item);
		}
	}
	return valid;
}

/// ルートディレクトリを初期化
void DiskBasicDir::ClearRoot()
{
	Fill(basic->GetDirStartSector(), basic->GetDirEndSector(), basic->GetFillCodeOnDir());
}

/// ルートディレクトリ領域を指定コードで埋める
/// @param [in] start_sector 開始セクタ番号
/// @param [in] end_sector   終了セクタ番号
/// @param [in] code         埋めるコード
void DiskBasicDir::Fill(int start_sector, int end_sector, wxUint8 code)
{
	for(int sec_pos = start_sector; sec_pos <= end_sector; sec_pos++) {
		DiskD88Sector *sector = basic->GetManagedSector(sec_pos - 1);
		if (!sector) {
			break;
		}
		sector->Fill(code);
	}
}

/// ディレクトリを移動する
/// @param [in,out] dst_item 移動先のディレクトリのアイテム
bool DiskBasicDir::Change(DiskBasicDirItem * &dst_item)
{
	DiskBasicType *type = basic->GetType();

	if (type->IsRootDirectory(dst_item->GetStartGroup())) {
		// ルートディレクトリに移動
		dst_item = root;
		current_item = root;

	} else {
		// サブディレクトリに移動
		if (dst_item->GetChildren() == NULL) {
			DiskBasicGroups groups;
			dst_item->GetAllGroups(groups);

			if (!Check(type, groups)) {
				return false;
			}
			Assign(type, groups, dst_item);
		}
		// ".",".."を実際のディレクトリアイテムにするため
		// 親ディレクトリと同じ場合はそのアイテムにする
		DiskBasicDirItem *pitem = dst_item;
		for(int i=0; i<2; i++) {
			pitem = pitem->GetParent();
			if (!pitem) break;

			if (dst_item->GetStartGroup() == pitem->GetStartGroup()) {
				dst_item = pitem;
				break;
			} 
		}
		current_item = dst_item;
	}
	return true;
}

/// ディレクトリの占有サイズを計算する
int DiskBasicDir::CalcSize()
{
	int size = 0;
	DiskBasicDirItems *items = GetCurrentItems();
	if (items) {
		int count = (int)items->Count();
		if (count == 0) {
			return 0;
		}
		int data_size = (int)items->Item(0)->GetDataSize();
		size = data_size * count;
		for(int i = (count-1); i >= 0; i--) {
			DiskBasicDirItem *item = items->Item(i);
			if (item->IsUsed()) {
				break;
			}
			size -= data_size;
		}
	}
	return size;
}
