/// @file basicdiritem_msx.cpp
///
/// @brief disk basic directory item for MSX DISK BASIC / MSX-DOS
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
DiskBasicDirItemMSX::DiskBasicDirItemMSX(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItemMSDOS(basic, sector, data)
{
}
DiskBasicDirItemMSX::DiskBasicDirItemMSX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemMSDOS(basic, num, track, side, sector, secpos, data, unuse)
{
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemMSX::InvalidateChars()
{
	return wxT(" \"\\/:;*?+,=[]");
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMSX::GetFileAttrStr()
{
	wxString attr;
	// MSX-DOS
	if (GetFileType() & FILE_TYPE_HIDDEN_MASK) {
		attr = wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_HIDDEN]);	// hidden
	}
	if (attr.IsEmpty()) {
		attr = wxT("---");
	}
	return attr;
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItemMSX::GetFileDateTimeTitle()
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

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] file_type_1    ファイル属性1 GetFileType1Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] file_type_2    ファイル属性2 GetFileType2Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] sizer
/// @param [in] flags
/// @param [in,out] controls   [0]: wxTextCtrl::txtIntNameで予約済み [1]からユーザ設定
/// @param [in,out] user_data  ユーザ定義データ
void DiskBasicDirItemMSX::CreateControlsForAttrDialog(IntNameBox *parent, int file_type_1, int file_type_2, wxBoxSizer *sizer, wxSizerFlags &flags, AttrControls &controls, int *user_data)
{
	wxCheckBox *chkHidden;

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkHidden = new wxCheckBox(parent, IDC_CHECK_HIDDEN, wxGetTranslation(gTypeNameMS_l[TYPE_NAME_MS_HIDDEN]));
	chkHidden->SetValue((file_type_1 & FILE_TYPE_HIDDEN_MASK) != 0);
	staType4->Add(chkHidden, flags);
	sizer->Add(staType4, flags);

	// event handler
//	parent->Bind(wxEVT_RADIOBOX, &IntNameBox::OnChangeType1, parent, IDC_RADIO_TYPE1);

	controls.Add(chkHidden);
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemMSX::GetFileType1InAttrDialog(const AttrControls &controls) const
{
	wxCheckBox *chkHidden = (wxCheckBox *)controls.Item(1);

	return chkHidden->GetValue() ? FILE_TYPE_HIDDEN_MASK : 0;
}

/// 機種依存の属性を設定する
bool DiskBasicDirItemMSX::SetAttrInAttrDialog(const AttrControls &controls, DiskBasicError &errinfo)
{
	// 何もしない
	return true;
}
