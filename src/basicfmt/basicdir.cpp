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
#include "basicdiritem_dos80.h"
#include "basicdiritem_frost.h"
#include "basicdiritem_magical.h"
#include "basicdiritem_sdos.h"
#include "basicdiritem_mdos.h"
#include "basicdiritem_fp.h"
#include "basicdiritem_xdos.h"
#include "basicdiritem_cdos.h"
#include "basicdiritem_mz_fdos.h"
#include "basicdiritem_hu68k.h"
#include "basicdiritem_losa.h"
#include "basicdiritem_falcom.h"
#include "basicdiritem_apledos.h"
#include "basicdiritem_prodos.h"
#include "basicdiritem_c1541.h"
#include "basicdiritem_amiga.h"
#include "basicdiritem_m68fdos.h"
#include "basicdiritem_trsdos.h"
#include "basicdiritem_hfs.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../diskimg/diskimage.h"
#include "../charcodes.h"


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

	int num = FORMAT_TYPE_UNKNOWN;
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
		item = new DiskBasicDirItemVFAT(basic);
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
	case FORMAT_TYPE_PA:
		item = new DiskBasicDirItemN88(basic);
		break;
	case FORMAT_TYPE_SMC:
		item = new DiskBasicDirItemCPM(basic);
		break;
	case FORMAT_TYPE_FP:
		item = new DiskBasicDirItemFP(basic);
		break;
	case FORMAT_TYPE_MACHFS:
		item = new DiskBasicDirItemHFS(basic);
		break;
	case FORMAT_TYPE_DOS80:
		item = new DiskBasicDirItemDOS80(basic);
		break;
	case FORMAT_TYPE_FROST:
		item = new DiskBasicDirItemFROST(basic);
		break;
	case FORMAT_TYPE_MAGICAL:
		item = new DiskBasicDirItemMAGICAL(basic);
		break;
	case FORMAT_TYPE_SDOS:
		item = new DiskBasicDirItemSDOS(basic);
		break;
	case FORMAT_TYPE_MDOS:
		item = new DiskBasicDirItemMDOS(basic);
		break;
	case FORMAT_TYPE_XDOS:
		item = new DiskBasicDirItemXDOS(basic);
		break;
	case FORMAT_TYPE_TFDOS:
		item = new DiskBasicDirItemTFDOS(basic);
		break;
	case FORMAT_TYPE_CDOS:
		item = new DiskBasicDirItemCDOS(basic);
		break;
	case FORMAT_TYPE_MZ_FDOS:
		item = new DiskBasicDirItemMZFDOS(basic);
		break;
	case FORMAT_TYPE_HU68K:
		item = new DiskBasicDirItemHU68K(basic);
		break;
	case FORMAT_TYPE_LOSA:
		item = new DiskBasicDirItemLOSA(basic);
		break;
	case FORMAT_TYPE_CDOS2:
		item = new DiskBasicDirItemMSDOS(basic);
		break;
	case FORMAT_TYPE_FALCOM:
		item = new DiskBasicDirItemFalcom(basic);
		break;
	case FORMAT_TYPE_APLEDOS:
		item = new DiskBasicDirItemAppleDOS(basic);
		break;
	case FORMAT_TYPE_PRODOS:
		item = new DiskBasicDirItemProDOS(basic);
		break;
	case FORMAT_TYPE_C1541:
		item = new DiskBasicDirItemC1541(basic);
		break;
	case FORMAT_TYPE_AMIGA:
		item = new DiskBasicDirItemAmiga(basic);
		break;
	case FORMAT_TYPE_M68FDOS:
		item = new DiskBasicDirItemM68FDOS(basic);
		break;
	case FORMAT_TYPE_TRSD23:
		item = new DiskBasicDirItemTRSD23(basic);
		break;
	case FORMAT_TYPE_TRSD13:
		item = new DiskBasicDirItemTRSD13(basic);
		break;
	default:
		wxFAIL_MSG(wxT("Unknown type is defined in basic_type.xml."));
//		item = new DiskBasicDirItem(basic);
		break;
	}
	if (item) {
		item->ClearData();
	}
	return item;
}
/// ディレクトリアイテムを新規に作成してアサインする
/// @param [in]  n_sector  セクタ
/// @param [in]  n_pos     セクタ内の位置
/// @param [in]  n_data    セクタ内のバッファ
DiskBasicDirItem *DiskBasicDir::NewItem(DiskImageSector *n_sector, int n_pos, wxUint8 *n_data)
{
	DiskBasicDirItem *item = NULL;

	int num = FORMAT_TYPE_UNKNOWN;
	if (format_type) num = format_type->GetTypeNumber();

	switch(num) {
	case FORMAT_TYPE_L3_1S:
		item = new DiskBasicDirItemL31S(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_L3S1_2D:
		item = new DiskBasicDirItemL32D(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_FM:
		item = new DiskBasicDirItemFM(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_MSDOS:
		item = new DiskBasicDirItemVFAT(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_MSX:
		item = new DiskBasicDirItemMSX(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_N88:
		item = new DiskBasicDirItemN88(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_X1HU:
		item = new DiskBasicDirItemX1HU(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_MZ:
		item = new DiskBasicDirItemMZ(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_FLEX:
		item = new DiskBasicDirItemFLEX(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_OS9:
		item = new DiskBasicDirItemOS9(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_CPM:
		item = new DiskBasicDirItemCPM(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_PA:
		item = new DiskBasicDirItemN88(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_SMC:
		item = new DiskBasicDirItemCPM(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_FP:
		item = new DiskBasicDirItemFP(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_MACHFS:
		item = new DiskBasicDirItemHFS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_DOS80:
		item = new DiskBasicDirItemDOS80(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_FROST:
		item = new DiskBasicDirItemFROST(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_MAGICAL:
		item = new DiskBasicDirItemMAGICAL(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_SDOS:
		item = new DiskBasicDirItemSDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_MDOS:
		item = new DiskBasicDirItemMDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_XDOS:
		item = new DiskBasicDirItemXDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_TFDOS:
		item = new DiskBasicDirItemTFDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_CDOS:
		item = new DiskBasicDirItemCDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_MZ_FDOS:
		item = new DiskBasicDirItemMZFDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_HU68K:
		item = new DiskBasicDirItemHU68K(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_LOSA:
		item = new DiskBasicDirItemLOSA(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_CDOS2:
		item = new DiskBasicDirItemMSDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_FALCOM:
		item = new DiskBasicDirItemFalcom(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_APLEDOS:
		item = new DiskBasicDirItemAppleDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_PRODOS:
		item = new DiskBasicDirItemProDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_C1541:
		item = new DiskBasicDirItemC1541(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_AMIGA:
		item = new DiskBasicDirItemAmiga(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_M68FDOS:
		item = new DiskBasicDirItemM68FDOS(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_TRSD23:
		item = new DiskBasicDirItemTRSD23(basic, n_sector, n_pos, n_data);
		break;
	case FORMAT_TYPE_TRSD13:
		item = new DiskBasicDirItemTRSD13(basic, n_sector, n_pos, n_data);
		break;
	default:
		wxFAIL_MSG(wxT("Unknown type is defined in basic_type.xml."));
//		item = new DiskBasicDirItem(basic, n_sector, n_pos, n_data);
		break;
	}
	return item;
}
/// ディレクトリアイテムを新規に作成してアサインする
/// @param [in]  n_num     通し番号
/// @param [in]  n_gitem   トラック番号などのデータ
/// @param [in]  n_sector  セクタ
/// @param [in]  n_pos     セクタ内での位置
/// @param [in]  n_data    セクタ内のバッファ
/// @param [in]  n_next    次のセクタ
/// @param [out] n_unuse   未使用か
DiskBasicDirItem *DiskBasicDir::NewItem(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_pos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
{
	DiskBasicDirItem *item = NULL;

	int num = FORMAT_TYPE_UNKNOWN;
	if (format_type) num = format_type->GetTypeNumber();

	switch(num) {
	case FORMAT_TYPE_L3_1S:
		item = new DiskBasicDirItemL31S(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_L3S1_2D:
		item = new DiskBasicDirItemL32D(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_FM:
		item = new DiskBasicDirItemFM(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_MSDOS:
		item = new DiskBasicDirItemVFAT(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_MSX:
		item = new DiskBasicDirItemMSX(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_N88:
		item = new DiskBasicDirItemN88(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_X1HU:
		item = new DiskBasicDirItemX1HU(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_MZ:
		item = new DiskBasicDirItemMZ(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_FLEX:
		item = new DiskBasicDirItemFLEX(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_OS9:
		item = new DiskBasicDirItemOS9(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_CPM:
		item = new DiskBasicDirItemCPM(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_PA:
		item = new DiskBasicDirItemN88(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_SMC:
		item = new DiskBasicDirItemCPM(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_FP:
		item = new DiskBasicDirItemFP(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_MACHFS:
		item = new DiskBasicDirItemHFS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_DOS80:
		item = new DiskBasicDirItemDOS80(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_FROST:
		item = new DiskBasicDirItemFROST(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_MAGICAL:
		item = new DiskBasicDirItemMAGICAL(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_SDOS:
		item = new DiskBasicDirItemSDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_MDOS:
		item = new DiskBasicDirItemMDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_XDOS:
		item = new DiskBasicDirItemXDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_TFDOS:
		item = new DiskBasicDirItemTFDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_CDOS:
		item = new DiskBasicDirItemCDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_MZ_FDOS:
		item = new DiskBasicDirItemMZFDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_HU68K:
		item = new DiskBasicDirItemHU68K(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_LOSA:
		item = new DiskBasicDirItemLOSA(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_CDOS2:
		item = new DiskBasicDirItemMSDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_FALCOM:
		item = new DiskBasicDirItemFalcom(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_APLEDOS:
		item = new DiskBasicDirItemAppleDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_PRODOS:
		item = new DiskBasicDirItemProDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_C1541:
		item = new DiskBasicDirItemC1541(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_AMIGA:
		item = new DiskBasicDirItemAmiga(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_M68FDOS:
		item = new DiskBasicDirItemM68FDOS(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_TRSD23:
		item = new DiskBasicDirItemTRSD23(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	case FORMAT_TYPE_TRSD13:
		item = new DiskBasicDirItemTRSD13(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	default:
		wxFAIL_MSG(wxT("Unknown type is defined in basic_type.xml."));
//		item = new DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_pos, n_data, n_next, n_unuse);
		break;
	}
	return item;
}
/// ルートディレクトリのアイテムを返す
DiskBasicDirItem *DiskBasicDir::GetRootItem() const
{
	return root;
}
/// ルートディレクトリの一覧を返す
DiskBasicDirItems *DiskBasicDir::GetRootItems(DiskBasicDirItem **dir_item)
{
	DiskBasicDirItem *item = GetRootItem();
	if (dir_item) *dir_item = item;
	return GetChildren(item);
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
	if (dir_item) *dir_item = item;
	return GetChildren(item);
}
/// ディレクトリ内の子供一覧を返す
DiskBasicDirItems *DiskBasicDir::GetChildren(DiskBasicDirItem *dir_item)
{
	if (!dir_item) return NULL;
	return dir_item->GetChildren();
}

/// カレントディレクトリの全ディレクトリアイテムをクリア
void DiskBasicDir::EmptyChildrenInCurrent()
{
	EmptyChildren(GetCurrentItem());
}
/// ディレクトリの全ディレクトリアイテムをクリア
void DiskBasicDir::EmptyChildren(DiskBasicDirItem *dir_item)
{
	if (dir_item) dir_item->EmptyChildren();
}

/// ルートをカレントにする
void DiskBasicDir::SetCurrentAsRoot()
{
	current_item = root;
}

/// 親ディレクトリのアイテムを返す
DiskBasicDirItem *DiskBasicDir::GetParentItemOnCurrent() const
{
	return GetParentItem(GetCurrentItem());
}
/// 親ディレクトリのアイテムを返す
DiskBasicDirItem *DiskBasicDir::GetParentItem(const DiskBasicDirItem *dir_item) const
{
	DiskBasicDirItem *item = NULL;
	if (dir_item) {
		item = dir_item->GetParent();
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
/// カレントディレクトリ内で未使用のディレクトリアイテムを返す
/// @param [in,out] pitem     ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out]    next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicDir::GetEmptyItemOnCurrent(DiskBasicDirItem *pitem, DiskBasicDirItem **next_item)
{
	return GetEmptyItem(GetCurrentItem(), GetCurrentItems(), pitem, next_item);
}

/// ルートディレクトリ内で未使用のディレクトリアイテムを返す
/// @param [in,out] pitem     ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out]    next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicDir::GetEmptyItemOnRoot(DiskBasicDirItem *pitem, DiskBasicDirItem **next_item)
{
	return GetEmptyItem(GetRootItem(), GetRootItems(), pitem, next_item);
}

/// 未使用のディレクトリアイテムを返す
/// @param [in,out] dir_item  ディレクトリ
/// @param [in,out] pitem     ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out]    next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicDir::GetEmptyItem(DiskBasicDirItem *dir_item, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item)
{
	return GetEmptyItem(dir_item, GetChildren(dir_item), pitem, next_item);
}

/// 未使用のディレクトリアイテムを返す
/// @param [in,out] dir_item  ディレクトリ
/// @param [in,out] children  dir_item内のディレクトリアイテム一覧
/// @param [in,out] pitem     ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out]    next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicDir::GetEmptyItem(DiskBasicDirItem *dir_item, DiskBasicDirItems *children, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item)
{
	DiskBasicType *type = basic->GetType();
	return type->GetEmptyDirectoryItem(dir_item, children, pitem, next_item);
}

/// 現在のディレクトリ内に同じファイル名が既に存在するか
/// @param [in]  filename     ファイル名
/// @param [in]  icase        大文字小文字を区別しないか(case insensitive)
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFileOnCurrent(const DiskBasicFileName &filename, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	return FindFile(GetCurrentItem(), filename, icase, exclude_item, next_item);
}

/// 指定したディレクトリ内に同じファイル名が既に存在するか
/// @param [in]  dir_item     検索対象のディレクトリアイテム
/// @param [in]  filename     ファイル名
/// @param [in]  icase        大文字小文字を区別しないか(case insensitive)
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFile(const DiskBasicDirItem *dir_item, const DiskBasicFileName &filename, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	const DiskBasicDirItems *items = dir_item->GetChildren();
	if (items) {
		for(size_t pos = 0; pos <items->Count(); pos++) {
			DiskBasicDirItem *item = items->Item(pos);
			if (item != exclude_item && item->IsSameFileName(filename, icase)) {
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
/// @param [in]  icase        大文字小文字を区別しないか(case insensitive)
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFileOnCurrent(const DiskBasicDirItem *target_item, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	return FindFile(GetCurrentItem(), target_item, icase, exclude_item, next_item);
}

/// 指定したディレクトリ内に同じファイル名が既に存在するか
/// @param [in]  dir_item     検索対象のディレクトリアイテム
/// @param [in]  target_item  検索対象アイテム
/// @param [in]  icase        大文字小文字を区別しないか(case insensitive)
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindFile(const DiskBasicDirItem *dir_item, const DiskBasicDirItem *target_item, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	const DiskBasicDirItems *items = dir_item->GetChildren();
	if (items) {
		for(size_t pos = 0; pos <items->Count(); pos++) {
			DiskBasicDirItem *item = items->Item(pos);
			if (item != exclude_item && item->IsSameFileName(target_item, icase)) {
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
/// @param [in]  icase        大文字小文字を区別しないか(case insensitive)
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindNameOnCurrent(const wxString &name, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	return FindName(GetCurrentItem(), name, icase, exclude_item, next_item);
}

/// 指定したディレクトリ内に同じファイル名(拡張子除く)が既に存在するか
/// @param [in]  dir_item     検索対象のディレクトリアイテム
/// @param [in]  name         ファイル名
/// @param [in]  icase        大文字小文字を区別しないか(case insensitive)
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasicDir::FindName(const DiskBasicDirItem *dir_item, const wxString &name, bool icase, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	const DiskBasicDirItems *items = dir_item->GetChildren();
	if (items) {
		for(size_t pos = 0; pos <items->Count(); pos++) {
			DiskBasicDirItem *item = items->Item(pos);
			if (item != exclude_item && item->IsSameName(name, icase)) {
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
DiskBasicDirItem *DiskBasicDir::FindFileByAttrOnCurrent(int file_type, int mask, DiskBasicDirItem *prev_item)
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
	if (!dir_item) return NULL;

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
double DiskBasicDir::CheckRoot(DiskBasicType *type, int start_sector, int end_sector, bool is_formatting)
{
	DiskBasicGroups root_groups;
	return type->CheckRootDirectory(start_sector, end_sector, root_groups, is_formatting);
}

/// ルートディレクトリをアサイン
/// @param [in] type         DISK BASIC 種類
/// @param [in] start_sector 開始セクタ番号
/// @param [in] end_sector   終了セクタ番号
bool DiskBasicDir::AssignRoot(DiskBasicType *type, int start_sector, int end_sector)
{
	DiskBasicGroups root_groups;
	delete root;
	root = NewItem();
	bool valid = type->AssignRootDirectory(start_sector, end_sector, root_groups, root);
	if (valid) {
		// グループを設定
		root->SetGroups(root_groups);
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
	DiskBasicGroups root_groups;
	delete root;
	root = NewItem();
	bool valid = type->AssignDirectory(true, root_groups, root);
	if (valid) {
		// グループを設定
		root->SetGroups(root_groups);
		// ディレクトリツリー確定
		root->ValidDirectory(true);
		// グループを保持
		current_item = root;
	}
	return valid;
}

/// ルートディレクトリをリリース
/// @param [in] type        DISK BASIC 種類
bool DiskBasicDir::ReleaseRoot(DiskBasicType *type)
{
	delete root;
	root = NULL;
	return true;
}

/// ディレクトリのチェック
/// @param [in] type        DISK BASIC 種類
/// @param [in] group_items グループのリスト
/// @return <0.0 エラー
double DiskBasicDir::Check(DiskBasicType *type, DiskBasicGroups &group_items)
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

	if (!type->IsRootDirectory(dir_item->GetStartGroup(0))) {
		// サブディレクトリのアイテムをアサイン
		if (dir_item->GetChildren() == NULL) {
			DiskBasicGroups groups;
			dir_item->GetAllGroups(groups);

			if (Check(type, groups) < 0.0) {
				return false;
			}
			valid = Assign(type, groups, dir_item);
		}
	}
	return valid;
}

/// ディレクトリエリアを読み直す
bool DiskBasicDir::Reassign(DiskBasicType *type, DiskBasicDirItem *dir_item)
{
	bool valid = true;

	// 子ディレクトリアイテムを削除
	EmptyChildren(dir_item);
	// 再アサイン
	if (GetParentItem(dir_item) != NULL) {
		valid = Assign(type, dir_item);
	} else {
		valid = AssignRoot(type, basic->GetDirStartSector(), basic->GetDirEndSector());
	}
	return valid;
}

/// ディレクトリエリアを読み直す
bool DiskBasicDir::Reassign(DiskBasicDirItem *dir_item)
{
	DiskBasicType *type = basic->GetType();
	return Reassign(type, dir_item);
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
		DiskImageSector *sector = basic->GetManagedSector(sec_pos - 1);
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

	if (type->IsRootDirectory(dst_item->GetStartGroup(0))) {
		// ルートディレクトリに移動
		dst_item = root;
		current_item = root;

	} else {
		// サブディレクトリに移動
		if (dst_item->GetChildren() == NULL) {
			DiskBasicGroups groups;
			dst_item->GetAllGroups(groups);

			if (Check(type, groups) < 0.0) {
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

			if (dst_item->GetStartGroup(0) == pitem->GetStartGroup(0)) {
				dst_item = pitem;
				break;
			} 
		}
		current_item = dst_item;
	}
	return true;
}

/// ディレクトリの拡張ができるか
/// @param [in] dir_item ディレクトリ
bool DiskBasicDir::CanExpand(const DiskBasicDirItem *dir_item)
{
	DiskBasicType *type = basic->GetType();
	return GetParentItem(dir_item) != NULL ? type->CanExpandDirectory() : type->CanExpandRootDirectory();
}

/// ディレクトリを拡張する
/// @param [in,out] dir_item ディレクトリ
bool DiskBasicDir::Expand(DiskBasicDirItem *dir_item)
{
	bool valid = false;
	if (dir_item != NULL) {
		valid = basic->ExpandDirectory(dir_item);
		if (valid) {
			// ディレクトリエリアを読み直す
			valid = Reassign(dir_item);
		}
	}
	return valid;
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
