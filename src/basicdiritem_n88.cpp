/// @file basicdiritem_n88.cpp
///
/// @brief disk basic directory item for N88-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_n88.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include <wx/msgdlg.h>


//
//
//

/// n88-BASIC属性名
const char *gTypeNameN88_1[] = {
	wxTRANSLATE("Ascii"),
	wxTRANSLATE("Binary"),
	wxTRANSLATE("Machine"),
	wxTRANSLATE("Ascii(Random Access)"),
	NULL
};
/// n88-BASIC属性名2
const char *gTypeNameN88_2[] = {
	wxTRANSLATE("Write Protected"),
	wxTRANSLATE("Read After Write"),
	wxTRANSLATE("Encrypted"),
	NULL
};

//
//
//
DiskBasicDirItemN88::DiskBasicDirItemN88(DiskBasic *basic)
	: DiskBasicDirItemFAT8(basic)
{
}
DiskBasicDirItemN88::DiskBasicDirItemN88(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItemFAT8(basic, sector, secpos, data)
{
}
DiskBasicDirItemN88::DiskBasicDirItemN88(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemFAT8(basic, num, track, side, sector, secpos, data, unuse)
{
	// n88
	Used(CheckUsed(unuse));
	unuse = (unuse || (m_data->n88.name[0] == 0xff));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemN88::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// N88
	if (num == 0) {
		size = len = sizeof(m_data->n88.name);
		return m_data->n88.name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemN88::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data->n88.ext);
	return m_data->n88.ext;
}

/// 属性１を返す
int	DiskBasicDirItemN88::GetFileType1() const
{
	return m_data->n88.type;
}

/// 属性１を設定
void DiskBasicDirItemN88::SetFileType1(int val)
{
	m_data->n88.type = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemN88::CheckUsed(bool unuse)
{
	return (!unuse && this->m_data->n88.name[0] != 0 && this->m_data->n88.name[0] != 0xff);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemN88::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;
	if (m_data->n88.name[0] == 0xff) {
		last = true;
		return valid;
	}
	// 属性に不正な値がある
	if (GetFileType1() & 0x0c) {
		valid = false;
	}
	return valid;
}

/// 属性を設定
/// @param [in] file_type
void DiskBasicDirItemN88::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	// n88
	int t1 = ConvFileType1(ftype);

	m_external_attr = ((ftype & FILE_TYPE_RANDOM_MASK) ? 1 : 0);

	if (ftype & FILE_TYPE_READONLY_MASK) {
		t1 |= DATATYPE_MASK_N88_READ_ONLY;
	}
	if (ftype & FILE_TYPE_ENCRYPTED_MASK) {
		t1 |= DATATYPE_MASK_N88_ENCRYPTED;
	}
	if (ftype & FILE_TYPE_READWRITE_MASK) {
		t1 |= DATATYPE_MASK_N88_READ_WRITE;
	}
	SetFileType1(t1);
}

/// 属性を変換
int DiskBasicDirItemN88::ConvFileType1(int file_type) const
{
	int t1 = 0;
	if (file_type & FILE_TYPE_MACHINE_MASK) {
		t1 = FILETYPE_N88_MACHINE;
	} else if (file_type & FILE_TYPE_BINARY_MASK) {
		t1 = FILETYPE_N88_BINARY;
	} else {
		t1 = FILETYPE_N88_ASCII;
	}
	return t1;
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemN88::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	if ((t1 & FILETYPE_N88_MACHINE) != 0) {
		val = FILE_TYPE_MACHINE_MASK;		// machine
		val |= FILE_TYPE_BINARY_MASK;		// binary
	} else {
		val = FILE_TYPE_BASIC_MASK;			// basic
		if (t1 & FILETYPE_N88_BINARY) {
			val |= FILE_TYPE_BINARY_MASK;	// binary
		} else {
			val |= FILE_TYPE_ASCII_MASK;	// ascii
		}
	}
	if (t1 & DATATYPE_MASK_N88_READ_ONLY) {
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (t1 & DATATYPE_MASK_N88_ENCRYPTED) {
		val |= FILE_TYPE_ENCRYPTED_MASK;
	}
	if (t1 & DATATYPE_MASK_N88_READ_WRITE) {
		val |= FILE_TYPE_READWRITE_MASK;
	}
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemN88::GetFileAttrStr() const
{
	// n88
	wxString attr = wxGetTranslation(gTypeNameN88_1[ConvFileType1Pos(GetFileType1())]);
	//
	int t = GetFileType1();
	if (t & DATATYPE_MASK_N88_READ_ONLY) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameN88_2[TYPE_NAME_N88_READ_ONLY]);
	}
	if (t & DATATYPE_MASK_N88_READ_WRITE) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameN88_2[TYPE_NAME_N88_READ_WRITE]);
	}
	if (t &DATATYPE_MASK_N88_ENCRYPTED) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameN88_2[TYPE_NAME_N88_ENCRYPTED]);
	}
	return attr;
}

/// ファイルサイズをセット
void DiskBasicDirItemN88::SetFileSize(int val)
{
	// ファイルサイズはセクタサイズ境界で丸める
	int sector_size = basic->GetSectorSize();
	m_groups.SetSize((((val - 1) / sector_size) + 1) * sector_size);
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemN88::GetDataSize() const
{
	return sizeof(directory_n88_t);
}

/// ファイル内部のアドレスを取り出す
void DiskBasicDirItemN88::TakeAddressesInFile()
{
	if (m_groups.Count() == 0 || (GetFileType1() & FILETYPE_N88_MACHINE) == 0) {
		m_start_address = -1;
		m_end_address = -1;
		m_exec_address = -1;
		return;
	}

	DiskBasicGroupItem *item = &m_groups.Item(0);
	DiskD88Sector *sector = basic->GetSector(item->track, item->side, item->sector_start);
	if (!sector) return;

	bool is_bigendian = basic->IsBigEndian();

	// 開始アドレス
	m_start_address = (int)sector->Get16(0, is_bigendian);
	// 終了アドレス
	m_end_address = (int)sector->Get16(2, is_bigendian);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemN88::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// n88
	m_data->n88.start_group = (val & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemN88::GetStartGroup(int fileunit_num) const
{
	// n88
	return m_data->n88.start_group;
}

/// ENDマークがあるか(一度も使用していないか)
bool DiskBasicDirItemN88::HasEndMark()
{
	bool val = false;
	val = ((wxUint32)m_data->n88.name[0] == basic->GetGroupUnusedCode());
	return val;
}

/// 次のアイテムにENDマークを入れる
void DiskBasicDirItemN88::SetEndMark(DiskBasicDirItem *next_item)
{
	if (!next_item) return;

	if (HasEndMark()) next_item->Delete(basic->GetGroupUnusedCode());
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemN88::NeedCheckEofCode()
{
	// Asc形式のときはEOFコードが必要
	return (((GetFileType1() & (FILETYPE_N88_MACHINE | FILETYPE_N88_BINARY)) == 0) && (m_external_attr == 0));
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemN88::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		file_size = CheckEofCode(istream, file_size);
		// ただし、ファイルサイズがセクタサイズと合うなら終端記号は不要
		if ((file_size % basic->GetSectorSize()) == 1) {
			// 残り１バイトは終端コードのみなので不要
			file_size--;
		}
	}
	return file_size;
}

/// ディレクトリをクリア
void DiskBasicDirItemN88::ClearData()
{
	if (!m_data) return;
	int c = basic->GetFillCodeOnDir();
	size_t l;
	l = GetDataSize();
	memset(m_data, c, l);

	m_data->n88.type = 0;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemN88::ConvFileTypeFromFileName(const wxString &filename) const
{
	int ftype = 0;
	// 拡張子で属性を設定する
	wxFileName fn(filename);
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
	if (sa) {
		ftype = sa->GetType();
	} else {
		ftype = FILE_TYPE_ASCII_MASK;
	}
	return ftype;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemN88::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;
	// 拡張子で属性を設定する
	wxFileName fn(filename);
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
	if (sa) {
		t1 = ConvFileType1(sa->GetType());
	} else {
		t1 = TYPE_NAME_N88_ASCII;
	}
	return t1;
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_RADIO_TYPE1 51
#define IDC_CHECK_READONLY 52
#define IDC_CHECK_READWRITE 53
#define IDC_CHECK_ENCRYPT 54
#define IDC_RADIO_TYPE2 55

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemN88::ConvFileType1Pos(int t1) const
{
	int val = 0;
	if ((t1 & FILETYPE_N88_MACHINE) != 0) {
		val = TYPE_NAME_N88_MACHINE;
	} else {
		if (t1 & FILETYPE_N88_BINARY) {
			val = TYPE_NAME_N88_BINARY;
		} else if (m_external_attr) {
			val = TYPE_NAME_N88_RANDOM;
		} else {
			val = TYPE_NAME_N88_ASCII;
		}
	}
	return val;
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemN88::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = ConvOriginalTypeFromFileName(name);
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemN88::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	wxRadioBox *radType1;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkReadWrite;
	wxCheckBox *chkEncrypt;

	int file_type_1 = GetFileType1();
	int file_type_2 = GetFileAttr().GetType();
	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);
	file_type_1 = ConvFileType1Pos(file_type_1);

	wxBoxSizer *gszr = new wxBoxSizer(wxHORIZONTAL);

	wxArrayString types1;
	for(size_t i=0; gTypeNameN88_1[i] != NULL; i++) {
		types1.Add(wxGetTranslation(gTypeNameN88_1[i]));
	}
	radType1 = new wxRadioBox(parent, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 0, wxRA_SPECIFY_ROWS);
	radType1->SetSelection(file_type_1);
	gszr->Add(radType1, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameN88_2[TYPE_NAME_N88_READ_ONLY]));
	chkReadOnly->SetValue((file_type_2 & FILE_TYPE_READONLY_MASK) != 0);
	staType4->Add(chkReadOnly, flags);
	chkReadWrite = new wxCheckBox(parent, IDC_CHECK_READWRITE, wxGetTranslation(gTypeNameN88_2[TYPE_NAME_N88_READ_WRITE]));
	chkReadWrite->SetValue((file_type_2 & FILE_TYPE_READWRITE_MASK) != 0);
	staType4->Add(chkReadWrite, flags);
	chkEncrypt = new wxCheckBox(parent, IDC_CHECK_ENCRYPT, wxGetTranslation(gTypeNameN88_2[TYPE_NAME_N88_ENCRYPTED]));
	chkEncrypt->SetValue((file_type_2 &FILE_TYPE_ENCRYPTED_MASK) != 0);
	staType4->Add(chkEncrypt, flags);
	gszr->Add(staType4, flags);

	sizer->Add(gszr);

	// event handler
	parent->Bind(wxEVT_RADIOBOX, &IntNameBox::OnChangeType1, parent, IDC_RADIO_TYPE1);
}

/// controls 0:txtIntName 1...: dependent
void DiskBasicDirItemN88::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkReadWrite = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READWRITE);
	wxCheckBox *chkEncrypt = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ENCRYPT);

	int selected_idx = 0;
	if (radType1) {
		selected_idx = radType1->GetSelection();
	}

	switch(selected_idx) {
	case TYPE_NAME_N88_MACHINE:
		// machine
		chkReadOnly->Enable(false);
		chkReadWrite->Enable(false);
		chkEncrypt->Enable(false);
		break;
	case TYPE_NAME_N88_ASCII:
	case TYPE_NAME_N88_RANDOM:
		// ascii
		chkReadOnly->Enable(true);
		chkReadWrite->Enable(true);
		chkEncrypt->Enable(false);
		break;
	case TYPE_NAME_N88_BINARY:
		// binary
		chkReadOnly->Enable(true);
		chkReadWrite->Enable(true);
		chkEncrypt->Enable(true);
		break;
	}
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int	DiskBasicDirItemN88::CalcFileTypeFromPos(int pos) const
{
	int val = 0;
	if (pos == TYPE_NAME_N88_MACHINE) {
		val = FILE_TYPE_MACHINE_MASK;
	} else if (pos == TYPE_NAME_N88_BINARY) {
		val = FILE_TYPE_BINARY_MASK;
	} else {
		val = FILE_TYPE_ASCII_MASK;
		if (pos == TYPE_NAME_N88_RANDOM) {
			val |= FILE_TYPE_RANDOM_MASK;
		}
	}
	return val;
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemN88::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkReadWrite = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READWRITE);
	wxCheckBox *chkEncrypt = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ENCRYPT);

	int val = CalcFileTypeFromPos(radType1->GetSelection());
	val |= chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkEncrypt->GetValue() ? FILE_TYPE_ENCRYPTED_MASK : 0;
	val |= chkReadWrite->GetValue() ? FILE_TYPE_READWRITE_MASK : 0;

	attr.SetFileAttr(FORMAT_TYPE_UNKNOWN, val);

	return true;
}
