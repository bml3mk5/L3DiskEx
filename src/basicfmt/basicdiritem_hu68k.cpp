/// @file basicdiritem_hu68k.cpp
///
/// @brief disk basic directory item for Human68k
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_hu68k.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemHU68K::DiskBasicDirItemHU68K(DiskBasic *basic)
	: DiskBasicDirItemMSDOS(basic)
{
}
DiskBasicDirItemHU68K::DiskBasicDirItemHU68K(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemMSDOS(basic, n_sector, n_secpos, n_data)
{
}
DiskBasicDirItemHU68K::DiskBasicDirItemHU68K(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemMSDOS(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemHU68K::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemMSDOS::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemHU68K::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	switch(num) {
	case 0:
		size = len = sizeof(m_data.Data()->hu68k.name);
		return m_data.Data()->hu68k.name;
	case 1:
		size = len = sizeof(m_data.Data()->hu68k.name2);
		return m_data.Data()->hu68k.name2;
	default:
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemHU68K::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data.Data()->hu68k.ext);
	return m_data.Data()->hu68k.ext;
}

size_t DiskBasicDirItemHU68K::GetDataSize() const
{
	return sizeof(directory_hu68k_t);
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_CHECK_READONLY	51
#define IDC_CHECK_HIDDEN	52
#define IDC_CHECK_SYSTEM	53
#define IDC_CHECK_VOLUME	54
#define IDC_CHECK_DIRECTORY	55
#define IDC_CHECK_ARCHIVE	56

/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemHU68K::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = FILE_TYPE_ARCHIVE_MASK;
	}
	if (show_flags & INTNAME_IMPORT_INTERNAL) {
		// 内部からインポート時
		file_type_1 |= FILE_TYPE_ARCHIVE_MASK;
	}
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemHU68K::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("NAME"), m_data.Data()->hu68k.name, sizeof(m_data.Data()->hu68k.name));
	vals.Add(wxT("EXT"), m_data.Data()->hu68k.ext, sizeof(m_data.Data()->hu68k.ext));
	vals.Add(wxT("TYPE"), m_data.Data()->hu68k.type);
	vals.Add(wxT("NAME2"), m_data.Data()->hu68k.name2, sizeof(m_data.Data()->hu68k.name2));
	vals.Add(wxT("WTIME"), m_data.Data()->hu68k.wtime);
	vals.Add(wxT("WDATE"), m_data.Data()->hu68k.wdate);
	vals.Add(wxT("START_GROUP"), m_data.Data()->hu68k.start_group);
	vals.Add(wxT("FILE_SIZE"), m_data.Data()->hu68k.file_size);
}
