/// @file basicdiritem_msx.cpp
///
/// @brief disk basic directory item for MSX DISK BASIC / MSX-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_msx.h"
#include "basicfmt.h"
#include "charcodes.h"


///
///
///
DiskBasicDirItemMSX::DiskBasicDirItemMSX(DiskBasic *basic)
	: DiskBasicDirItemMSDOS(basic)
{
}
DiskBasicDirItemMSX::DiskBasicDirItemMSX(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItemMSDOS(basic, sector, secpos, data)
{
}
DiskBasicDirItemMSX::DiskBasicDirItemMSX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemMSDOS(basic, num, track, side, sector, secpos, data, unuse)
{
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMSX::GetFileAttrStr() const
{
	wxString attr;
	// MSX-DOS
	if (GetFileAttr().MatchType(FILE_TYPE_HIDDEN_MASK, FILE_TYPE_HIDDEN_MASK)) {
		attr = wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_HIDDEN]);	// hidden
	}
	if (attr.IsEmpty()) {
		attr = wxT("---");
	}
	return attr;
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItemMSX::GetFileDateTimeTitle() const
{
	return _("Created Date:");
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_CHECK_HIDDEN 52

/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemMSX::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemMSX::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileAttr().GetType();
	int file_type_2 = 0;
	wxCheckBox *chkHidden;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkHidden = new wxCheckBox(parent, IDC_CHECK_HIDDEN, wxGetTranslation(gTypeNameMS_l[TYPE_NAME_MS_HIDDEN]));
	chkHidden->SetValue((file_type_1 & FILE_TYPE_HIDDEN_MASK) != 0);
	staType4->Add(chkHidden, flags);
	sizer->Add(staType4, flags);

	// event handler
//	parent->Bind(wxEVT_RADIOBOX, &IntNameBox::OnChangeType1, parent, IDC_RADIO_TYPE1);

//	controls.Add(chkHidden);
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemMSX::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxCheckBox *chkHidden = (wxCheckBox *)parent->FindWindow(IDC_CHECK_HIDDEN);

	int val = chkHidden->GetValue() ? FILE_TYPE_HIDDEN_MASK : 0;

	attr.SetFileAttr(FORMAT_TYPE_UNKNOWN, val);

	return true;
}
