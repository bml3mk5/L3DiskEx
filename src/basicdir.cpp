/// @file basicdir.cpp
///
/// @brief disk basic directory
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
	this->start_sector_pos = 0;
	this->end_sector_pos = 0;
	this->unique_number = 0;
	this->format_type = FORMAT_TYPE_NONE;
	this->parent_item = NULL;
}
DiskBasicDir::~DiskBasicDir()
{
	Clear();

	delete parent_item;
}
/// ディレクトリアイテムを新規に作成
DiskBasicDirItem *DiskBasicDir::NewItem()
{
	DiskBasicDirItem *item;
	switch(format_type) {
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
	default:
		item = new DiskBasicDirItem(basic);
		break;
	}
	return item;
}
/// ディレクトリアイテムを新規に作成してアサインする
/// @param [in] newsec  セクタ
/// @param [in] newdata セクタ内のバッファ
DiskBasicDirItem *DiskBasicDir::NewItem(DiskD88Sector *newsec, wxUint8 *newdata)
{
	DiskBasicDirItem *item;
	switch(format_type) {
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
	default:
		item = new DiskBasicDirItem(basic, newsec, newdata);
		break;
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
	DiskBasicDirItem *item;
	switch(format_type) {
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
	default:
		item = new DiskBasicDirItem(basic, newnum, newtrack, newside, newsec, newpos, newdata, unuse);
		break;
	}
	return item;
}
/// ディレクトリアイテムを新規に作成してアサインする
DiskBasicDirItem *DiskBasicDir::AssignItem(int newnum, int newtrack, int newside, DiskD88Sector *newsec, int newpos, wxUint8 *newdata, bool &unuse)
{
	DiskBasicDirItem *item = NewItem(newnum, newtrack, newside, newsec, newpos, newdata, unuse);
	items.Add(item);
	return item;
}
/// ディレクトリアイテムのポインタを返す
/// @param [in] idx インデックス
DiskBasicDirItem *DiskBasicDir::ItemPtr(size_t idx)
{
	if (idx >= items.Count()) return NULL;
	return items.Item(idx);
}
/// 未使用のディレクトリアイテムを返す
/// @param [out] next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicDir::GetEmptyItemPtr(DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	for(size_t i=0; i < items.Count(); i++) {
		DiskBasicDirItem *item = items.Item(i);
		if (!item->IsUsed()) {
			match_item = item;
			if (next_item) {
				i++;
				if (i < items.Count()) {
					*next_item = items.Item(i);
				} else {
					*next_item = NULL;
				}
			}
			break;
		}
	}
	return match_item;
}
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
/// 現在のディレクトリ内に同じファイル名が既に存在するか
/// @param [in]  filename     ファイル名
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFile(const wxString &filename, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	for(size_t pos = 0; pos <items.Count(); pos++) {
		DiskBasicDirItem *item = items[pos];
		if (item != exclude_item && item->IsSameFileName(filename)) {
			match_item = item;
			if (next_item) {
				pos++;
				if (pos < items.Count()) {
					*next_item = items[pos];
				} else {
					*next_item = NULL;
				}
			}
			break;
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
	DiskBasicDirItem *match_item = NULL;
	for(size_t pos = 0; pos <items.Count(); pos++) {
		DiskBasicDirItem *item = items.Item(pos);
		if (item != exclude_item && item->IsSameFileName(target_item)) {
			match_item = item;
			if (next_item) {
				pos++;
				if (pos < items.Count()) {
					*next_item = items.Item(pos);
				} else {
					*next_item = NULL;
				}
			}
			break;
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
	DiskBasicDirItem *match_item = NULL;
	for(size_t pos = 0; pos <items.Count(); pos++) {
		DiskBasicDirItem *item = items.Item(pos);
		if (item != exclude_item && item->IsSameName(name)) {
			match_item = item;
			if (next_item) {
				pos++;
				if (pos < items.Count()) {
					*next_item = items.Item(pos);
				} else {
					*next_item = NULL;
				}
			}
			break;
		}
	}
	return match_item;
}

/// ルートディレクトリのチェック
/// @param [in] type          DISK BASIC 種類
/// @param [in] start_sector  開始セクタ番号
/// @param [in] end_sector    終了セクタ番号
bool DiskBasicDir::CheckRoot(DiskBasicType *type, int start_sector, int end_sector)
{
	return type->CheckRootDirectory(start_sector, end_sector);
}

/// ルートディレクトリをアサイン
/// @param [in] type         DISK BASIC 種類
/// @param [in] start_sector 開始セクタ番号
/// @param [in] end_sector   終了セクタ番号
bool DiskBasicDir::AssignRoot(DiskBasicType *type, int start_sector, int end_sector)
{
	bool valid = type->AssignRootDirectory(start_sector, end_sector);
	if (valid) {
		groups.Empty();
		start_sector_pos = start_sector;
		end_sector_pos = end_sector;
		unique_number++;
	}
	return valid;
}

/// ルートディレクトリをアサイン
/// @param [in] type        DISK BASIC 種類
bool DiskBasicDir::AssignRoot(DiskBasicType *type)
{
	return AssignRoot(type, start_sector_pos, end_sector_pos);
}

/// ディレクトリのチェック
/// @param [in] type        DISK BASIC 種類
/// @param [in] group_items グループのリスト
bool DiskBasicDir::Check(DiskBasicType *type, DiskBasicGroups &group_items)
{
	return type->CheckDirectory(group_items);
}

/// ディレクトリをアサイン
/// @param [in] type        DISK BASIC 種類
/// @param [in] group_items グループのリスト
bool DiskBasicDir::Assign(DiskBasicType *type, DiskBasicGroups &group_items)
{
	bool valid = type->AssignDirectory(group_items);
	if (valid) {
		// グループを保持
		groups = group_items;
		unique_number++;
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

/// 親ディレクトリのアイテムを設定
void DiskBasicDir::SetParentItem(const DiskBasicDirItem *val)
{
	delete parent_item;
	if (val) {
		parent_item = NewItem(NULL, NULL);
		parent_item->Dup(*val);
	} else {
		parent_item = NULL;
	}
}
