/// @file basicdiritem_tfdos.cpp
///
/// @brief disk basic directory item for TF-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_tfdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "utils.h"


//
//
//

/// TF-DOS属性名
const char *gTypeNameTFDOS[] = {
	wxTRANSLATE("OBJ"),
	wxTRANSLATE("TEX"),
	wxTRANSLATE("CMD"),
	wxTRANSLATE("SYS"),
	wxTRANSLATE("DAT"),
	wxTRANSLATE("GRA"),
	wxTRANSLATE("DBB"),
	wxTRANSLATE("Write Protected"),
	wxTRANSLATE("Hidden"),
	NULL
};

///
///
///
DiskBasicDirItemTFDOS::DiskBasicDirItemTFDOS(DiskBasic *basic)
	: DiskBasicDirItemMZBase(basic)
{
	external_attr = 2;	// TXTの時、BASE互換かを自動判定
}
DiskBasicDirItemTFDOS::DiskBasicDirItemTFDOS(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItemMZBase(basic, sector, data)
{
	external_attr = 2;	// TXTの時、BASE互換かを自動判定
}
DiskBasicDirItemTFDOS::DiskBasicDirItemTFDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemMZBase(basic, num, track, side, sector, secpos, data, unuse)
{
	external_attr = 2;	// TXTの時、BASE互換かを自動判定

	Used(CheckUsed(unuse));

	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemTFDOS::GetFileNamePos(size_t &len, bool *invert) const
{
	len = sizeof(data->tfdos.name);
	if (invert) *invert = basic->IsDataInverted();
	return data->tfdos.name; 
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemTFDOS::GetFileNameSize(bool *invert) const
{
	if (invert) *invert = basic->IsDataInverted();
	return (int)sizeof(data->tfdos.name);
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemTFDOS::GetFileExtSize(bool *invert) const
{
	if (invert) *invert = basic->IsDataInverted();
	return 0;
}

/// 属性１を返す
int	DiskBasicDirItemTFDOS::GetFileType1() const
{
	return basic->InvertUint8(data->tfdos.type);	// invert;
}

/// 属性１を設定
void DiskBasicDirItemTFDOS::SetFileType1(int val)
{
	data->tfdos.type = basic->InvertUint8(val);	// invert
}

/// 使用しているアイテムか
bool DiskBasicDirItemTFDOS::CheckUsed(bool unuse)
{
	return (GetFileType1() != 0);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemTFDOS::Check(bool &last)
{
	if (!data) return false;

	bool valid = true;
	wxUint8 t = GetFileType1();
	if ((t & 0x3f) >= 16) {
		valid = false;
	}
	return valid;
}

/// 属性を設定
void DiskBasicDirItemTFDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int val = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		// 同じOSの場合は元の属性をそのままセット
		val = file_type.GetOrigin();
	} else {
		// 別OSからの場合、近い属性をセット
		if (ftype & FILE_TYPE_BINARY_MASK) {
			if (ftype & FILE_TYPE_BASIC_MASK) val = FILETYPE_TFDOS_CMD;
			else if (ftype & FILE_TYPE_MACHINE_MASK) val = FILETYPE_TFDOS_SYS;
			else val = FILETYPE_TFDOS_OBJ;
		} else if (ftype & FILE_TYPE_DATA_MASK) {
			val = FILETYPE_TFDOS_DAT;
		} else if (ftype & FILE_TYPE_ASCII_MASK) {
			val = FILETYPE_TFDOS_TXT;
		}

		if (ftype & FILE_TYPE_READONLY_MASK) {
			val |= DATATYPE_TFDOS_READ_ONLY;
		}
		if (ftype & FILE_TYPE_HIDDEN_MASK) {
			val |= DATATYPE_TFDOS_HIDDEN;
		}
	}
	SetFileType1(val);
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemTFDOS::ClearData()
{
	if (!data) return;
	memset(data, 0, sizeof(directory_tfdos_t));
	memset(data->tfdos.name, 0x0d, sizeof(data->tfdos.name));
	basic->InvertMem(data, sizeof(directory_tfdos_t));	// invert
}

// 属性を返す
DiskBasicFileType DiskBasicDirItemTFDOS::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	switch(t1 & 0x3f) {
	case FILETYPE_TFDOS_OBJ:
		val = FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_TFDOS_TXT:
		val = FILE_TYPE_ASCII_MASK;		// ascii
		break;
	case FILETYPE_TFDOS_CMD:
		val = FILE_TYPE_BASIC_MASK;		// BASIC
		val |= FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_TFDOS_SYS:
		val = FILE_TYPE_MACHINE_MASK;	// machine
		val |= FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_TFDOS_DAT:
		val = FILE_TYPE_DATA_MASK;		// data
		break;
	}
	if (t1 & DATATYPE_TFDOS_READ_ONLY) {
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (t1 & DATATYPE_TFDOS_HIDDEN) {
		val |= FILE_TYPE_HIDDEN_MASK;
	}
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemTFDOS::GetFileAttrStr() const
{
	wxString attr = wxGetTranslation(gTypeNameTFDOS[GetFileType1Pos()]);

	int t1 = GetFileType1();
	if (t1 & DATATYPE_TFDOS_READ_ONLY) {
		// write protect
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameTFDOS[TYPE_NAME_TFDOS_READ_ONLY]);
	}
	if (t1 & DATATYPE_TFDOS_HIDDEN) {
		// hidden
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameTFDOS[TYPE_NAME_TFDOS_HIDDEN]);
	}
	return attr;
}

/// データ内にファイルサイズをセット
void DiskBasicDirItemTFDOS::SetFileSizeBase(int val)
{
	data->tfdos.file_size = basic->InvertUint16(val);	// invert
}

/// データ内のファイルサイズを返す
int DiskBasicDirItemTFDOS::GetFileSizeBase() const
{
	return basic->InvertUint16(data->tfdos.file_size);	// invert
}

// 開始アドレスを返す
int DiskBasicDirItemTFDOS::GetStartAddress() const
{
	return basic->InvertUint16(wxUINT16_SWAP_ON_BE(data->tfdos.load_addr));	// invert
}

// 実行アドレスを返す
int DiskBasicDirItemTFDOS::GetExecuteAddress() const
{
	return basic->InvertUint16(wxUINT16_SWAP_ON_BE(data->tfdos.exec_addr));	// invert
}

/// 開始アドレスをセット
void DiskBasicDirItemTFDOS::SetStartAddress(int val)
{
	data->tfdos.load_addr = (wxUint16)basic->InvertUint16(wxUINT16_SWAP_ON_BE(val));	// invert
}

/// 実行アドレスをセット
void DiskBasicDirItemTFDOS::SetExecuteAddress(int val)
{
	data->tfdos.exec_addr = (wxUint16)basic->InvertUint16(wxUINT16_SWAP_ON_BE(val));	// invert
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemTFDOS::GetDataSize() const
{
	return sizeof(directory_tfdos_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemTFDOS::SetStartGroup(wxUint32 val)
{
	data->tfdos.track = basic->InvertUint8(val & 0xff);	// invert
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemTFDOS::GetStartGroup() const
{
	return basic->InvertUint8(data->tfdos.track);	// invert
}

/// ファイルパスから内部ファイル名を生成する
wxString DiskBasicDirItemTFDOS::RemakeFileNameStr(const wxString &filepath) const
{
	wxString newname;
	wxFileName fn(filepath);
	newname = fn.GetFullName().Left(GetFileNameSize());
	ConvertToFileNameStr(newname);
	return newname;
}

/// ダイアログ入力前のファイル名を変換 大文字にするなど
void DiskBasicDirItemTFDOS::ConvertToFileNameStr(wxString &filename) const
{
	filename = filename.Upper();
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemTFDOS::GetDefaultInvalidateChars() const
{
	return wxT("\"\\/:*?");
}

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_COMBO_TYPE1    51
#define IDC_CHECK_BASECOMP 52
#define IDC_CHECK_READONLY 53
#define IDC_CHECK_HIDDEN   54

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemTFDOS::GetFileType1Pos() const
{
	int ftype = GetFileType1();
	int val = 0;
	int t1 = (ftype & 0x3f);
	if (FILETYPE_TFDOS_OBJ <= t1 && t1 <= FILETYPE_TFDOS_DBB) {
		val = t1 - FILETYPE_TFDOS_OBJ;
	}
	return val;
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemTFDOS::GetFileType2Pos() const
{
	int ftype = GetFileType1();
	int val = 0;
	if (ftype & DATATYPE_TFDOS_READ_ONLY) {
		// write protect
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (ftype & DATATYPE_TFDOS_HIDDEN) {
		// hidden
		val |= FILE_TYPE_HIDDEN_MASK;
	}
	return val;
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemTFDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_INVALID_FILE_TYPE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		wxString ext = name.Right(4).Upper();
		if (ext == wxT(".OBJ") || ext == wxT(".BIN")) {
			file_type_1 = TYPE_NAME_TFDOS_OBJ;
		} else if (ext == wxT(".TXT") || ext == wxT(".DOC")) {
			file_type_1 = TYPE_NAME_TFDOS_TXT;
		} else if (ext == wxT(".CMD") || ext == wxT(".COM") || ext == wxT(".EXE")) {
			file_type_1 = TYPE_NAME_TFDOS_CMD;
		} else if (ext == wxT(".SYS")) {
			file_type_1 = TYPE_NAME_TFDOS_SYS;
		} else if (ext == wxT(".DAT")) {
			file_type_1 = TYPE_NAME_TFDOS_DAT;
		} else if (ext == wxT(".GRA")) {
			file_type_1 = TYPE_NAME_TFDOS_GRA;
		} else if (ext == wxT(".DBB")) {
			file_type_1 = TYPE_NAME_TFDOS_DBB;
		}
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemTFDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();
	wxChoice   *comType1;
	wxCheckBox *chkBaseComp;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkHidden;

	m_show_flags = show_flags;
	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);
	
	wxArrayString types1;
	for(int i=TYPE_NAME_TFDOS_OBJ; i<=TYPE_NAME_TFDOS_DBB; i++) {
		types1.Add(wxGetTranslation(gTypeNameTFDOS[i]));
	}
	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	comType1->SetSelection(file_type_1);
	staType1->Add(comType1, flags);

	int chk_style = wxCHK_3STATE;
	if (show_flags & INTNAME_IMPORT_INTERNAL) {
		// 内部コピー
		external_attr = 0;
	} else if (show_flags & INTNAME_INVALID_FILE_TYPE) {
		// 外部からインポート
		external_attr = 0;
	} else {
		// プロパティ
		chk_style |= wxCHK_ALLOW_3RD_STATE_FOR_USER;
	}
	chkBaseComp = new wxCheckBox(parent, IDC_CHECK_BASECOMP, _("Treat as BASE compatible text."), wxDefaultPosition, wxDefaultSize, chk_style);
	chkBaseComp->Enable(file_type_1 == TYPE_NAME_TFDOS_TXT);
	chkBaseComp->Set3StateValue((wxCheckBoxState)external_attr);
	staType1->Add(chkBaseComp, flags);
	
	sizer->Add(staType1, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, _("Write Protect"));
	chkReadOnly->SetValue((file_type_2 & FILE_TYPE_READONLY_MASK) != 0);
	staType4->Add(chkReadOnly, flags);
	chkHidden = new wxCheckBox(parent, IDC_CHECK_HIDDEN, _("Hidden"));
	chkHidden->SetValue((file_type_2 & FILE_TYPE_HIDDEN_MASK) != 0);
	staType4->Add(chkHidden, flags);
	sizer->Add(staType4, flags);

	// bind
	parent->Bind(wxEVT_CHOICE, &IntNameBox::OnChangeType1, parent, IDC_COMBO_TYPE1);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemTFDOS::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	wxCheckBox *chkBaseComp = (wxCheckBox *)parent->FindWindow(IDC_CHECK_BASECOMP);
	if (comType1 && chkBaseComp) {
		int sel = comType1->GetSelection();
		chkBaseComp->Enable(sel == TYPE_NAME_TFDOS_TXT && (m_show_flags & INTNAME_IMPORT_INTERNAL) == 0);
	}
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemTFDOS::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);

	return comType1->GetSelection();
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int	DiskBasicDirItemTFDOS::CalcFileTypeFromPos(int pos)
{
	int val = pos;
	if (val < 0 || val > TYPE_NAME_TFDOS_DBB) {
		val = TYPE_NAME_TFDOS_TXT;
	} else {
		val++;
	}
	return val;
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemTFDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo)
{
	wxCheckBox *chkBaseComp = (wxCheckBox *)parent->FindWindow(IDC_CHECK_BASECOMP);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkHidden   = (wxCheckBox *)parent->FindWindow(IDC_CHECK_HIDDEN);

	int sel = GetFileType1InAttrDialog(parent);
	if (sel == TYPE_NAME_TFDOS_TXT) {
		external_attr = (int)chkBaseComp->Get3StateValue();
	}

	int origin = CalcFileTypeFromPos(sel);
	origin |= chkReadOnly->GetValue() ? DATATYPE_TFDOS_READ_ONLY : 0;
	origin |= chkHidden->GetValue() ? DATATYPE_TFDOS_HIDDEN : 0;

	DiskBasicDirItem::SetFileAttr(0, origin);

	return true;
}

/// ファイルサイズが適正か
/// @param [in]  parent     ダイアログ
/// @param [in]  size       ファイルサイズ
/// @param [out] limit      制限サイズ
/// @return true 適正
bool DiskBasicDirItemTFDOS::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	int limit_size = 0xffff;
	if (limit) *limit = limit_size;
	return (size <= limit_size);
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemTFDOS::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	return true;
}
