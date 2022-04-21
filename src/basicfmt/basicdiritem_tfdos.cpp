/// @file basicdiritem_tfdos.cpp
///
/// @brief disk basic directory item for TF-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_tfdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include "../config.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// TF-DOS属性名
const name_value_t gTypeNameTFDOS[] = {
	{ "???", 0 },
	{ "OBJ", FILETYPE_TFDOS_OBJ },
	{ "TEX", FILETYPE_TFDOS_TEX },
	{ "CMD", FILETYPE_TFDOS_CMD },
	{ "SYS", FILETYPE_TFDOS_SYS },
	{ "DAT", FILETYPE_TFDOS_DAT },
	{ "GRA", FILETYPE_TFDOS_GRA },
	{ "DBB", FILETYPE_TFDOS_DBB },
	{ wxTRANSLATE("Write Protected"), 0 },
	{ wxTRANSLATE("Hidden"), 0 },
	{ NULL, -1 }
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemTFDOS::DiskBasicDirItemTFDOS(DiskBasic *basic)
	: DiskBasicDirItemMZBase(basic)
{
	m_data.Alloc();
	m_external_attr = 2;	// TXTの時、BASE互換かを自動判定
}
DiskBasicDirItemTFDOS::DiskBasicDirItemTFDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemMZBase(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	m_external_attr = 2;	// TXTの時、BASE互換かを自動判定
}
DiskBasicDirItemTFDOS::DiskBasicDirItemTFDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemMZBase(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);
	m_external_attr = 2;	// TXTの時、BASE互換かを自動判定

	Used(CheckUsed(n_unuse));

	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemTFDOS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemMZBase::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemTFDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 属性１を返す
int	DiskBasicDirItemTFDOS::GetFileType1() const
{
	return basic->InvertUint8(m_data.Data()->type);	// invert;
}

/// 属性１を設定
void DiskBasicDirItemTFDOS::SetFileType1(int val)
{
	m_data.Data()->type = basic->InvertUint8(val);	// invert
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
	if (!m_data.IsValid()) return false;

	bool valid = true;
	wxUint8 t = GetFileType1();
	if ((t & 0x3f) >= 16) {
		valid = false;
	}
	return valid;
}

/// 削除
bool DiskBasicDirItemTFDOS::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data.Fill(basic->InvertUint8(basic->GetDeleteCode()), 1);
	Used(false);
	return true;
}

/// 属性を設定
void DiskBasicDirItemTFDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = file_type.GetOrigin();
	int val = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		// 同じOSの場合は元の属性をそのままセット
		val = t1;
	} else {
		// 別OSからの場合、近い属性をセット
		if (ftype & FILE_TYPE_BINARY_MASK) {
			if (ftype & FILE_TYPE_BASIC_MASK) val = FILETYPE_TFDOS_CMD;
			else if (ftype & FILE_TYPE_MACHINE_MASK) val = FILETYPE_TFDOS_SYS;
			else val = FILETYPE_TFDOS_OBJ;
		} else if (ftype & FILE_TYPE_DATA_MASK) {
			val = FILETYPE_TFDOS_DAT;
		} else if (ftype & FILE_TYPE_ASCII_MASK) {
			val = FILETYPE_TFDOS_TEX;
		}

		if (ftype & FILE_TYPE_READONLY_MASK) {
			val |= DATATYPE_TFDOS_READ_ONLY;
		}
		if (ftype & FILE_TYPE_HIDDEN_MASK) {
			val |= DATATYPE_TFDOS_HIDDEN;
		}
	}
	m_external_attr = (val >> 16);
	SetFileType1(val);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemTFDOS::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	switch(t1 & 0x3f) {
	case FILETYPE_TFDOS_OBJ:
		val = FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_TFDOS_TEX:
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
	int t1 = GetFileType1();
	wxString attr = wxGetTranslation(gTypeNameTFDOS[ConvFileType1Pos(t1)].name);

	if (t1 & DATATYPE_TFDOS_READ_ONLY) {
		// write protect
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameTFDOS[TYPE_NAME_TFDOS_READ_ONLY].name);
	}
	if (t1 & DATATYPE_TFDOS_HIDDEN) {
		// hidden
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameTFDOS[TYPE_NAME_TFDOS_HIDDEN].name);
	}
	return attr;
}

/// データ内にファイルサイズをセット
void DiskBasicDirItemTFDOS::SetFileSizeBase(int val)
{
	m_data.Data()->file_size = basic->InvertAndOrderUint16(val);	// invert
}

/// データ内のファイルサイズを返す
int DiskBasicDirItemTFDOS::GetFileSizeBase() const
{
	return basic->InvertAndOrderUint16(m_data.Data()->file_size);	// invert
}

// 開始アドレスを返す
int DiskBasicDirItemTFDOS::GetStartAddress() const
{
	return basic->InvertAndOrderUint16(m_data.Data()->load_addr);	// invert
}

// 実行アドレスを返す
int DiskBasicDirItemTFDOS::GetExecuteAddress() const
{
	return basic->InvertAndOrderUint16(m_data.Data()->exec_addr);	// invert
}

/// 開始アドレスをセット
void DiskBasicDirItemTFDOS::SetStartAddress(int val)
{
	m_data.Data()->load_addr = (wxUint16)basic->InvertAndOrderUint16(val);	// invert
}

/// 実行アドレスをセット
void DiskBasicDirItemTFDOS::SetExecuteAddress(int val)
{
	m_data.Data()->exec_addr = (wxUint16)basic->InvertAndOrderUint16(val);	// invert
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemTFDOS::GetDataSize() const
{
	return sizeof(directory_tfdos_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemTFDOS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemTFDOS::CopyData(const directory_t *val)
{
	return m_data.Copy(val, GetDataSize());
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemTFDOS::ClearData()
{
	m_data.Fill(0, sizeof(directory_tfdos_t));
	memset(m_data.Data()->name, 0x0d, sizeof(m_data.Data()->name));
	basic->InvertMem(m_data.Data(), sizeof(directory_tfdos_t));	// invert
}

/// 最初のグループ番号を設定
void DiskBasicDirItemTFDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	m_data.Data()->track = basic->InvertUint8(val & 0xff);	// invert
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemTFDOS::GetStartGroup(int fileunit_num) const
{
	return basic->InvertUint8(m_data.Data()->track);	// invert
}

/// データをエクスポートする前に必要な処理
bool DiskBasicDirItemTFDOS::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	wxString ext;
	if (GetFileAttrName(ConvFileType1Pos(GetFileType1()), gTypeNameTFDOS, ext)) {
		filename += wxT(".");
		if (Utils::IsUpperString(filename)) {
			filename += ext.Upper();
		} else {
			filename += ext.Lower();
		}
	}
	return true;
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
bool DiskBasicDirItemTFDOS::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		IsContainAttrByExtension(filename, gTypeNameTFDOS, TYPE_NAME_TFDOS_OBJ, TYPE_NAME_TFDOS_DBB, &filename, NULL, NULL);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemTFDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	// 拡張子で属性を設定する
	int t1 = 0;
	if (!IsContainAttrByExtension(filename, gTypeNameTFDOS, TYPE_NAME_TFDOS_OBJ, TYPE_NAME_TFDOS_DBB, NULL, &t1, NULL)) {
		t1 = FILETYPE_TFDOS_TEX;
	}
	return t1;
}

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_COMBO_TYPE1    51
#define IDC_CHECK_BASECOMP 52
#define IDC_CHECK_READONLY 53
#define IDC_CHECK_HIDDEN   54

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemTFDOS::ConvFileType1Pos(int t1) const
{
	int val = 0;
	t1 = (t1 & 0x3f);
	if (FILETYPE_TFDOS_OBJ <= t1 && t1 <= FILETYPE_TFDOS_DBB) {
		val = t1;
	}
	return val;
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemTFDOS::ConvFileType2Pos(int t1) const
{
	int val = 0;
	if (t1 & DATATYPE_TFDOS_READ_ONLY) {
		// write protect
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (t1 & DATATYPE_TFDOS_HIDDEN) {
		// hidden
		val |= FILE_TYPE_HIDDEN_MASK;
	}
	return val;
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemTFDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int t1 = GetFileType1();
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		t1 = ConvOriginalTypeFromFileName(file_path);
	}

	int file_type_1 = ConvFileType1Pos(t1);
	int file_type_2 = ConvFileType2Pos(t1);
	wxChoice   *comType1;
	wxCheckBox *chkBaseComp;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkHidden;

	m_show_flags = show_flags;

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);
	
	wxArrayString types1;
	CreateChoiceForAttrDialog(basic, gTypeNameTFDOS, TYPE_NAME_TFDOS_READ_ONLY, types1);
	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	comType1->SetSelection(file_type_1);
	staType1->Add(comType1, flags);

	int chk_style = wxCHK_3STATE;
	if (show_flags & INTNAME_IMPORT_INTERNAL) {
		// 内部コピー
		m_external_attr = 0;
	} else if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート
		m_external_attr = 1;
	} else {
		// プロパティ
		chk_style |= wxCHK_ALLOW_3RD_STATE_FOR_USER;
	}
	chkBaseComp = new wxCheckBox(parent, IDC_CHECK_BASECOMP, _("Treat as BASE compatible text."), wxDefaultPosition, wxDefaultSize, chk_style);
	chkBaseComp->Enable(file_type_1 == TYPE_NAME_TFDOS_TEX);
	chkBaseComp->Set3StateValue((wxCheckBoxState)m_external_attr);
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
		chkBaseComp->Enable(sel == TYPE_NAME_TFDOS_TEX && (m_show_flags & INTNAME_IMPORT_INTERNAL) == 0);
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
int	DiskBasicDirItemTFDOS::CalcFileTypeFromPos(int pos) const
{
	int val = pos;
	if (val < TYPE_NAME_TFDOS_UNKNOWN || val > TYPE_NAME_TFDOS_DBB) {
		val = TYPE_NAME_TFDOS_TEX;
	}
	return val;
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemTFDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxCheckBox *chkBaseComp = (wxCheckBox *)parent->FindWindow(IDC_CHECK_BASECOMP);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkHidden   = (wxCheckBox *)parent->FindWindow(IDC_CHECK_HIDDEN);

	int sel = GetFileType1InAttrDialog(parent);
	int ext = 0;
	if (sel == TYPE_NAME_TFDOS_TEX) {
		ext = (int)chkBaseComp->Get3StateValue();
	}

	int origin = CalcFileTypeFromPos(sel);
	origin |= chkReadOnly->GetValue() ? DATATYPE_TFDOS_READ_ONLY : 0;
	origin |= chkHidden->GetValue() ? DATATYPE_TFDOS_HIDDEN : 0;

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, ext << 16 | origin);

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

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemTFDOS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("inverted"), basic->IsDataInverted());

	vals.Add(wxT("TYPE"), m_data.Data()->type, basic->IsDataInverted());
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name), basic->IsDataInverted());
	vals.Add(wxT("FILE_SIZE"), m_data.Data()->file_size, basic->IsBigEndian(), basic->IsDataInverted());
	vals.Add(wxT("LOAD_ADDR"), m_data.Data()->load_addr, basic->IsBigEndian(), basic->IsDataInverted());
	vals.Add(wxT("EXEC_ADDR"), m_data.Data()->exec_addr, basic->IsBigEndian(), basic->IsDataInverted());
	vals.Add(wxT("TRACK"), m_data.Data()->track, basic->IsDataInverted());
}
