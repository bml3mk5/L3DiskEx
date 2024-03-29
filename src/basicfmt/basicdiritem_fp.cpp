/// @file basicdiritem_fp.cpp
///
/// @brief disk basic directory item for C82-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_fp.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include "../config.h"
#include <wx/msgdlg.h>


//////////////////////////////////////////////////////////////////////
//
//
//

/// C82-BASIC属性名
const char *gTypeNameFP_2[] = {
	wxTRANSLATE("Write Protected"),
	wxTRANSLATE("Read After Write"),
	NULL
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemFP::DiskBasicDirItemFP(DiskBasic *basic)
	: DiskBasicDirItemFAT8(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemFP::DiskBasicDirItemFP(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemFAT8(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
}
DiskBasicDirItemFP::DiskBasicDirItemFP(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemFAT8(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);

	Used(CheckUsed(n_unuse));
	n_unuse = (n_unuse || (m_data.Data()->name[0] == 0xff));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemFP::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemFAT8::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemFP::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemFP::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data.Data()->ext);
	return m_data.Data()->ext;
}

/// 属性１を返す
int	DiskBasicDirItemFP::GetFileType1() const
{
	return m_data.Data()->type;
}

/// 属性１を設定
void DiskBasicDirItemFP::SetFileType1(int val)
{
	m_data.Data()->type = val & 0xff;
}

/// 属性２を返す
int DiskBasicDirItemFP::GetFileType2() const
{
	return m_data.Data()->attr;
}

/// 属性２のセット
void DiskBasicDirItemFP::SetFileType2(int val)
{
	m_data.Data()->attr = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemFP::CheckUsed(bool unuse)
{
	return (!unuse && this->m_data.Data()->type != 0);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemFP::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;

	// ディレクトリが不正
	if (((m_data.Data()->type & 2) != 0) || (m_data.Data()->term != 0 && m_data.Data()->term != 0xff)) {
		valid = false;
	}
	return valid;
}

/// 削除
bool DiskBasicDirItemFP::Delete()
{
	ClearData();
	Used(false);
	return true;
}

/// 属性を設定
/// @param [in] file_type
void DiskBasicDirItemFP::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	int t2 = 0;
	if (ftype & FILE_TYPE_MACHINE_MASK) {
		t1 = FILETYPE_FP_MACHINE;
	} else if (ftype & FILE_TYPE_BASIC_MASK) {
		t1 = FILETYPE_FP_BASIC;
	} else if (ftype & FILE_TYPE_DATA_MASK) {
		t1 = FILETYPE_FP_DATA;
	}
	if (ftype & FILE_TYPE_RANDOM_MASK) {
		t1 |= FILETYPE_FP_RANDOM;
	} else if (ftype & FILE_TYPE_ASCII_MASK) {
		t1 |= FILETYPE_FP_ASCII;
	}

	if (ftype & FILE_TYPE_READONLY_MASK) {
		t2 |= DATATYPE_MASK_FP_READ_ONLY;
	}
	if (ftype & FILE_TYPE_READWRITE_MASK) {
		t2 |= DATATYPE_MASK_FP_READ_WRITE;
	}

	SetFileType1(t1);
	SetFileType2(t2);

	SetTerminate(t1);
}

/// エントリデータの未使用部分を設定
void DiskBasicDirItemFP::SetTerminate(int val)
{
	if (!m_data.IsValid()) return;

	int c;
	c = (val != 0 ? 0xff : 0);
	m_data.Data()->term = (c & 0xff);

	c = ((val & FILETYPE_FP_BASIC) != 0 ? 0xff : 0);
	memset(m_data.Data()->unknown, c, sizeof(m_data.Data()->unknown));
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemFP::GetFileAttr() const
{
	int t1 = GetFileType1();
	int t2 = GetFileType2();
	int val = 0;
	if ((t1 & FILETYPE_FP_MACHINE) == FILETYPE_FP_MACHINE) {
		val = FILE_TYPE_MACHINE_MASK;
	} else if ((t1 & FILETYPE_FP_BASIC) == FILETYPE_FP_BASIC) {
		val = FILE_TYPE_BASIC_MASK;
	} else if ((t1 & FILETYPE_FP_DATA) == FILETYPE_FP_DATA) {
		val = FILE_TYPE_DATA_MASK;
	}

	if (t1 & FILETYPE_FP_RANDOM) {
		val |= FILE_TYPE_RANDOM_MASK;
	} else if (t1 & FILETYPE_FP_ASCII) {
		val |= FILE_TYPE_ASCII_MASK;
	} else {
		val |= FILE_TYPE_BINARY_MASK;
	}

	if (t2 & DATATYPE_MASK_FP_READ_ONLY) {
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (t2 & DATATYPE_MASK_FP_READ_WRITE) {
		val |= FILE_TYPE_READWRITE_MASK;
	}

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t2 << 8 | t1);
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFP::GetFileType1Pos() const
{
	int t1 = GetFileType1();
	int val = TYPE_NAME_1_UNKNOWN;
	if ((t1 & FILETYPE_FP_MACHINE) == FILETYPE_FP_MACHINE) {
		val = TYPE_NAME_1_MACHINE;
	} else if ((t1 & FILETYPE_FP_BASIC) == FILETYPE_FP_BASIC) {
		val = TYPE_NAME_1_BASIC;
	} else if ((t1 & FILETYPE_FP_DATA) == FILETYPE_FP_DATA) {
		val = TYPE_NAME_1_DATA;
	}
	return val;
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFP::GetFileType2Pos() const
{
	int t1 = GetFileType1();
	int val = 0;
	if (t1 & FILETYPE_FP_RANDOM) {
		val = TYPE_NAME_2_RANDOM;
	} else if (t1 & FILETYPE_FP_ASCII) {
		val = TYPE_NAME_2_ASCII;
	} else {
		val = TYPE_NAME_2_BINARY;
	}
	return val;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemFP::GetFileAttrStr() const
{
	int p1 = GetFileType1Pos();
	wxString attr = wxGetTranslation(gTypeName1[p1]);
	//
	int p2 = GetFileType2Pos();
	attr += wxT(" - ");
	attr += wxGetTranslation(gTypeName2[p2]);
	//
	int t2 = GetFileType2();
	if (t2 & DATATYPE_MASK_FP_READ_ONLY) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameFP_2[TYPE_NAME_FP_READ_ONLY]);
	}
	if (t2 & DATATYPE_MASK_FP_READ_WRITE) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameFP_2[TYPE_NAME_FP_READ_WRITE]);
	}
	return attr;
}

/// ファイルサイズをセット
void DiskBasicDirItemFP::SetFileSize(int val)
{
	m_groups.SetSize(val);
	int t1 = GetFileType1();
	if (t1 == FILETYPE_FP_BASIC) {
		// BASIC - binary
		m_data.Data()->file_size = wxUINT16_SWAP_ON_BE(val);
		m_data.Data()->end_addr = wxUINT16_SWAP_ON_BE(val);
	} else if (t1 == FILETYPE_FP_MACHINE) {
		// Machine
		m_data.Data()->file_size = wxUINT16_SWAP_ON_BE(val);
		val = wxUINT16_SWAP_ON_BE(m_data.Data()->load_addr) + val - 1;
		m_data.Data()->end_addr = wxUINT16_SWAP_ON_BE(val);
	}
	// アスキーファイルはファイルサイズをセットしない
}

/// ファイルサイズを返す
int DiskBasicDirItemFP::GetFileSize() const
{
	int val = wxUINT16_SWAP_ON_BE(m_data.Data()->file_size);
	if (val == 0) {
		val = (int)m_groups.GetSize();
	}
	return val;
}

/// 開始アドレスを返す
int DiskBasicDirItemFP::GetStartAddress() const
{
	int val = m_data.Data()->load_addr;
	return wxUINT16_SWAP_ON_BE(val);
}

/// 終了アドレスを返す
int DiskBasicDirItemFP::GetEndAddress() const
{
	int val = m_data.Data()->end_addr;
	return wxUINT16_SWAP_ON_BE(val);
}

/// 実行アドレスを返す
int DiskBasicDirItemFP::GetExecuteAddress() const
{
	int val = m_data.Data()->exec_addr;
	return wxUINT16_SWAP_ON_BE(val);
}

/// 開始アドレスをセット
void DiskBasicDirItemFP::SetStartAddress(int val)
{
	m_data.Data()->load_addr = wxUINT16_SWAP_ON_BE(val);
}

/// 終了アドレスをセット
void DiskBasicDirItemFP::SetEndAddress(int val)
{
	m_data.Data()->end_addr = wxUINT16_SWAP_ON_BE(val);
}

/// 実行アドレスをセット
void DiskBasicDirItemFP::SetExecuteAddress(int val)
{
	m_data.Data()->exec_addr = wxUINT16_SWAP_ON_BE(val);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemFP::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// fp
	m_data.Data()->start_group = (val & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemFP::GetStartGroup(int fileunit_num) const
{
	// fp
	return m_data.Data()->start_group;
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemFP::NeedCheckEofCode()
{
	// アスキー形式のときはEOFコードでサイズを計算
	return ((GetFileType1() & (FILETYPE_FP_RANDOM | FILETYPE_FP_ASCII)) == FILETYPE_FP_ASCII);
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemFP::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	// ファイルの最終が終端記号で終わっているかを調べる
	// ただし、ファイルサイズがクラスタサイズと合うなら終端記号は不要
	if ((file_size % (basic->GetSectorSize() * basic->GetSectorsPerGroup())) != 0) {
		file_size = CheckEofCode(istream, file_size) - 1;
	}
	return file_size;
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemFP::GetDataSize() const
{
	return sizeof(directory_fp_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemFP::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemFP::CopyData(const directory_t *val)
{
	return m_data.Copy(val);
}

/// ディレクトリをクリア
void DiskBasicDirItemFP::ClearData()
{
	m_data.Fill(basic->GetFillCodeOnDir());
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemFP::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimExtensionByExtensionAttr(filename);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_RADIO_TYPE1 51
#define IDC_RADIO_TYPE2 52
#define IDC_CHECK_READONLY 53
#define IDC_CHECK_READWRITE 54

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemFP::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();
	wxRadioBox *radType1;
	wxRadioBox *radType2;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkReadWrite;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString types1;
	for(size_t i=0; i<=TYPE_NAME_1_MACHINE; i++) {
		types1.Add(wxGetTranslation(gTypeName1[i]));
	}
	radType1 = new wxRadioBox(parent, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 0, wxRA_SPECIFY_COLS);
	radType1->SetSelection(file_type_1);
	sizer->Add(radType1, flags);

	wxGridSizer *gszr = new wxGridSizer(2, 1, 1);

	wxArrayString types2;
	for(size_t i=0; i<=TYPE_NAME_2_RANDOM; i++) {
		types2.Add(wxGetTranslation(gTypeName2[i]));
	}
	radType2 = new wxRadioBox(parent, IDC_RADIO_TYPE2, _("Data Type"), wxDefaultPosition, wxDefaultSize, types2, 0, wxRA_SPECIFY_ROWS);
	radType2->SetSelection(file_type_2);
	gszr->Add(radType2, flags);

	int file_attr = GetFileType2();
	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameFP_2[TYPE_NAME_FP_READ_ONLY]));
	chkReadOnly->SetValue((file_attr & DATATYPE_MASK_FP_READ_ONLY) != 0);
	staType4->Add(chkReadOnly, flags);
	chkReadWrite = new wxCheckBox(parent, IDC_CHECK_READWRITE, wxGetTranslation(gTypeNameFP_2[TYPE_NAME_FP_READ_WRITE]));
	chkReadWrite->SetValue((file_attr & DATATYPE_MASK_FP_READ_WRITE) != 0);
	staType4->Add(chkReadWrite, flags);
	gszr->Add(staType4, flags);

	sizer->Add(gszr);

	// event handler
	parent->Bind(wxEVT_RADIOBOX, &IntNameBox::OnChangeType1, parent, IDC_RADIO_TYPE1);
}

/// controls 0:txtIntName 1...: dependent
void DiskBasicDirItemFP::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);
	wxRadioBox *radType2 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE2);

	int selected_idx = 0;
	if (radType1) {
		selected_idx = radType1->GetSelection();
	}

	switch(selected_idx) {
	case TYPE_NAME_1_BASIC:
		// BASIC
		if (radType2->GetSelection() == TYPE_NAME_2_RANDOM) {
			radType2->SetSelection(TYPE_NAME_2_ASCII);
		}
		radType2->Enable(TYPE_NAME_2_BINARY, true);
		radType2->Enable(TYPE_NAME_2_ASCII, true);
		radType2->Enable(TYPE_NAME_2_RANDOM, false);
		parent->SetEditableStartAddress(false);
		parent->SetEditableEndAddress(false);
		parent->SetEditableExecuteAddress(false);
		break;
	case TYPE_NAME_1_DATA:
		// DATA
		if (radType2->GetSelection() == TYPE_NAME_2_BINARY) {
			radType2->SetSelection(TYPE_NAME_2_ASCII);
		}
		radType2->Enable(TYPE_NAME_2_BINARY, false);
		radType2->Enable(TYPE_NAME_2_ASCII, true);
		radType2->Enable(TYPE_NAME_2_RANDOM, true);
		parent->SetEditableStartAddress(false);
		parent->SetEditableEndAddress(false);
		parent->SetEditableExecuteAddress(false);
		break;
	case TYPE_NAME_1_MACHINE:
		// machine
		radType2->SetSelection(TYPE_NAME_2_BINARY);
		radType2->Enable(TYPE_NAME_2_BINARY, true);
		radType2->Enable(TYPE_NAME_2_ASCII, false);
		radType2->Enable(TYPE_NAME_2_RANDOM, false);
		parent->SetEditableStartAddress(true);
		parent->SetEditableEndAddress(false);
		parent->SetEditableExecuteAddress(true);
		break;
	}

	// 終了アドレスを計算
	parent->CalcEndAddress();
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemFP::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);
	wxRadioBox *radType2 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE2);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkReadWrite = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READWRITE);

	int ftype = 0;

	int sel = radType1->GetSelection();
	switch(sel) {
	case TYPE_NAME_1_BASIC:
		// BASIC
		ftype = FILE_TYPE_BASIC_MASK;
		break;
	case TYPE_NAME_1_MACHINE:
		// Machine
		ftype = FILE_TYPE_MACHINE_MASK;
		break;
	default:
		// DATA
		ftype = FILE_TYPE_DATA_MASK;
		break;
	}

	sel = radType2->GetSelection();
	switch(sel) {
	case TYPE_NAME_2_RANDOM:
		ftype |= FILE_TYPE_RANDOM_MASK;
		break;
	case TYPE_NAME_2_ASCII:
		ftype |= FILE_TYPE_ASCII_MASK;
		break;
	default:
		ftype |= FILE_TYPE_BINARY_MASK;
		break;
	}

	if (chkReadOnly->IsChecked()) {
		ftype |= FILE_TYPE_READONLY_MASK;
	}
	if (chkReadWrite->IsChecked()) {
		ftype |= FILE_TYPE_READWRITE_MASK;
	}

	attr.SetFileAttr(FORMAT_TYPE_UNKNOWN, ftype, 0);

	return true;
}

/// 属性値を加工する
bool DiskBasicDirItemFP::ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int ftype = attr.GetFileType();
	if (ftype & FILE_TYPE_BINARY_MASK) {
		// バイナリ
		if (ftype & FILE_TYPE_BASIC_MASK) {
			// BASICならアドレスは固定
			attr.SetStartAddress(0);
			attr.SetEndAddress(GetFileSize());
			attr.SetExecuteAddress(0);
		}
	} else {
		// アスキー、ランダムアクセス
		// アドレスは固定
		attr.SetStartAddress(0);
		attr.SetEndAddress(0);
		attr.SetExecuteAddress(0);
	}
	return true;
}

/// ダイアログ用に終了アドレスを返す
int DiskBasicDirItemFP::GetEndAddressInAttrDialog(IntNameBox *parent)
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);
	wxRadioBox *radType2 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE2);
	if (!radType1 || !radType2) return -1;

	int val = 0;
	int sel1 = radType1->GetSelection();
	int sel2 = radType2->GetSelection();
	switch(sel2) {
	case TYPE_NAME_2_BINARY:
		// バイナリは自動計算、アスキーやランダムアクセスは０
		if (sel1 == TYPE_NAME_1_BASIC) {
			// BASIC
			val = GetFileSize();
		} else {
			val = -1;
		}
		break;
	}
	return val;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemFP::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("TYPE"), m_data.Data()->type);
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("EXT"), m_data.Data()->ext, sizeof(m_data.Data()->ext));
	vals.Add(wxT("TERM"), m_data.Data()->term);
	vals.Add(wxT("UNKNOWN"), m_data.Data()->unknown, sizeof(m_data.Data()->unknown));
	vals.Add(wxT("LOAD_ADDR"), m_data.Data()->load_addr);
	vals.Add(wxT("END_ADDR"), m_data.Data()->end_addr);
	vals.Add(wxT("EXEC_ADDR"), m_data.Data()->exec_addr);
	vals.Add(wxT("FILE_SIZE"), m_data.Data()->file_size);
	vals.Add(wxT("START_GROUP"), m_data.Data()->start_group);
	vals.Add(wxT("ATTR"), m_data.Data()->attr);
	vals.Add(wxT("RESERVED"), m_data.Data()->reserved, sizeof(m_data.Data()->reserved));
}
