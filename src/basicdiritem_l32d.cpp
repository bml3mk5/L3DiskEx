/// @file basicdiritem_l32d.cpp
///
/// @brief disk basic directory item for L3/S1 BASIC 2D/2HD
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_l32d.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"


///
///
///
DiskBasicDirItemL32D::DiskBasicDirItemL32D(DiskBasic *basic)
	: DiskBasicDirItemFAT8(basic)
{
}
DiskBasicDirItemL32D::DiskBasicDirItemL32D(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItemFAT8(basic, sector, secpos, data)
{
}
DiskBasicDirItemL32D::DiskBasicDirItemL32D(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemFAT8(basic, num, track, side, sector, secpos, data, unuse)
{
	// L3 2D

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemL32D::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;
	directory_l3_2d_t *p = (directory_l3_2d_t *)m_data;
	if (p->name[0] == 0xff) {
		last = true;
		return valid;
	}
	// 属性に想定外の値がある場合はエラー
	if (p->type2 != 0 && p->type2 != 0xff) {
		valid = false;
	}
	if (p->name[0] == 0xff) {
		last = true;
	}
	return valid;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemL32D::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// L3 2D
	if (num == 0) {
		size = len = sizeof(m_data->l3_2d.name);
		return m_data->l3_2d.name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemL32D::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data->l3_2d.ext);
	return m_data->l3_2d.ext;
}

/// 属性１を返す
int	DiskBasicDirItemL32D::GetFileType1() const
{
	return m_data->l3_2d.type;
}

/// 属性２を返す
int	DiskBasicDirItemL32D::GetFileType2() const
{
	return m_data->l3_2d.type2;
}

/// 属性１を設定
void DiskBasicDirItemL32D::SetFileType1(int val)
{
	m_data->l3_2d.type = val & 0xff;
}

/// 属性２を設定
void DiskBasicDirItemL32D::SetFileType2(int val)
{
	m_data->l3_2d.type2 = val & 0xff;
}

/// ディレクトリのサイズ
size_t DiskBasicDirItemL32D::GetDataSize() const
{
	return sizeof(directory_l3_2d_t);
}

/// ファイルサイズをセット
/// @param [in] val サイズ
void DiskBasicDirItemL32D::SetFileSize(int val)
{
	DiskBasicDirItemFAT8::SetFileSize(val);
	// 最終セクタのサイズをセット
	SetDataSizeOnLastSecotr(val % basic->GetSectorSize());
}

/// 最初のグループ番号を設定
void DiskBasicDirItemL32D::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// L3/S1 2D/2HD
	m_data->l3_2d.start_group = (val & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemL32D::GetStartGroup(int fileunit_num) const
{
	// L3/S1 2D/2HD
	return m_data->l3_2d.start_group;
}

/// 最終セクタのサイズ(2Dのときのみ有効)
void DiskBasicDirItemL32D::SetDataSizeOnLastSecotr(int val)
{
	// L3/S1 2D/2HD
	m_data->l3_2d.end_bytes.h = ((val & 0xff00) >> 8);
	m_data->l3_2d.end_bytes.l = (val & 0x00ff);
}

/// 最終セクタのサイズ(2Dのときのみ有効)
int DiskBasicDirItemL32D::GetDataSizeOnLastSector() const
{
	// L3/S1 2D/2HD
	int val = (int)m_data->l3_2d.end_bytes.h * 256 + m_data->l3_2d.end_bytes.l;
	return val;
}

/// 最終セクタのサイズを計算してファイルサイズを返す
int DiskBasicDirItemL32D::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
	if (IsUsed() && occupied_size >= 0) {
		occupied_size = occupied_size - basic->GetSectorSize() + GetDataSizeOnLastSector();
	}
	return occupied_size;
}

/// ダイアログ入力後のファイル名文字列を変換
void DiskBasicDirItemL32D::ConvertFileNameAfterRenamed(wxString &filename) const
{
	int file_type_1 = GetFileType1();
	filename = AddExtension(file_type_1, filename);
}

#include <wx/textctrl.h>
#include <wx/radiobox.h>
#include "intnamebox.h"


/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemL32D::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (file_type_2 == TYPE_NAME_2_RANDOM) {
		// 1Sから2Dへのコピーでランダムアクセスのデータはアスキーとする。
		file_type_2 = TYPE_NAME_2_ASCII;
	}
	// 拡張子から属性を設定
	DiskBasicDirItemFAT8::SetFileTypeForAttrDialog(show_flags, name, file_type_1, file_type_2);
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemL32D::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	DiskBasicDirItemFAT8::CreateControlsForAttrDialog(parent, show_flags, file_path, sizer, flags);

	// ランダムアクセスの選択肢を隠す
	wxRadioBox *radType2 = (wxRadioBox *)parent->FindWindow(ATTR_DIALOG_IDC_RADIO_TYPE2);
	radType2->Show(TYPE_NAME_2_RANDOM, false);
}

/// 拡張子を追加
wxString DiskBasicDirItemL32D::AddExtension(int file_type_1, const wxString &name) const
{
	// L3/S1 BASIC
	// 拡張子を自動で付加する
	wxString newname;

	int len = (int)name.Length();
	wxString ext = name.Right(4);
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(ext.Right(3));
	if (sa && ext.Left(1) == wxT(".")) {
		len -= 4;
		if (len >= 0) newname = name.Left(len);
		else newname.Empty();
	} else {
		len = name.Find('.', false);
		if (len >= 0) {
			return name;
		} else {
			newname = name;
		}
	}

	int val = (file_type_1 >= TYPE_NAME_1_BASIC && file_type_1 <= TYPE_NAME_1_MACHINE ? 1 << file_type_1 : 0);
	sa = basic->GetAttributesByExtension().FindType(val, 0x7);
	if (sa) {
		newname += wxT(".");
		newname += sa->GetName();
	}

	return newname;
}
