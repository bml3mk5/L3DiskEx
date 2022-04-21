/// @file basicdiritem_hu68k.cpp
///
/// @brief disk basic directory item for Human68k
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_hu68k.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "utils.h"


///
///
///
DiskBasicDirItemHU68K::DiskBasicDirItemHU68K(DiskBasic *basic)
	: DiskBasicDirItemMSDOS(basic)
{
}
DiskBasicDirItemHU68K::DiskBasicDirItemHU68K(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItemMSDOS(basic, sector, secpos, data)
{
}
DiskBasicDirItemHU68K::DiskBasicDirItemHU68K(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemMSDOS(basic, num, track, side, sector, secpos, data, unuse)
{
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemHU68K::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	switch(num) {
	case 0:
		size = len = sizeof(m_data->hu68k.name);
		return m_data->hu68k.name;
	case 1:
		size = len = sizeof(m_data->hu68k.name2);
		return m_data->hu68k.name2;
	default:
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemHU68K::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data->hu68k.ext);
	return m_data->hu68k.ext;
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
#include "intnamebox.h"

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
