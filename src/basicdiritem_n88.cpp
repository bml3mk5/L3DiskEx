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
DiskBasicDirItemN88::DiskBasicDirItemN88(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItemFAT8(basic, sector, data)
{
}
DiskBasicDirItemN88::DiskBasicDirItemN88(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemFAT8(basic, num, track, side, sector, secpos, data, unuse)
{
	// n88
	Used(CheckUsed(unuse));
	unuse = (unuse || (this->data->n88.name[0] == 0xff));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemN88::GetFileNamePos(size_t &size, size_t &len) const
{
	// N88
	size = len = sizeof(data->n88.name);
	return data->n88.name;
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemN88::GetFileExtPos(size_t &len) const
{
	len = sizeof(data->n88.ext);
	return data->n88.ext;
}

#if 0
/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemN88::GetFileNameSize(bool *invert) const
{
	return (int)sizeof(data->n88.name);
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemN88::GetFileExtSize(bool *invert) const
{
	return (int)sizeof(data->n88.ext);
}
#endif

/// 属性１を返す
int	DiskBasicDirItemN88::GetFileType1() const
{
	return data->n88.type;
}

/// 属性１を設定
void DiskBasicDirItemN88::SetFileType1(int val)
{
	data->n88.type = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemN88::CheckUsed(bool unuse)
{
	return (!unuse && this->data->n88.name[0] != 0 && this->data->n88.name[0] != 0xff);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemN88::Check(bool &last)
{
	if (!data) return false;

	bool valid = true;
	if (data->n88.name[0] == 0xff) {
		last = true;
		return valid;
	}
	// 属性に不正な値がある
	if (GetFileType1() & 0x0c) {
		valid = false;
	}
	return valid;
}

/// @param [in] file_type
void DiskBasicDirItemN88::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	// n88
	int t = 0;
	if (ftype & FILE_TYPE_MACHINE_MASK) {
		t = FILETYPE_N88_MACHINE;
	} else if (ftype & FILE_TYPE_BINARY_MASK) {
		t = FILETYPE_N88_BINARY;
	} else {
		t = FILETYPE_N88_ASCII;
	}
	if (ftype & FILE_TYPE_READONLY_MASK) {
		t |= DATATYPE_MASK_N88_READ_ONLY;
	}
	if (ftype & FILE_TYPE_ENCRYPTED_MASK) {
		t |= DATATYPE_MASK_N88_ENCRYPTED;
	}
	if (ftype & FILE_TYPE_READWRITE_MASK) {
		t |= DATATYPE_MASK_N88_READ_WRITE;
	}
	SetFileType1(t);
}

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
	wxString attr = wxGetTranslation(gTypeNameN88_1[GetFileType1Pos()]);
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
	file_size = (((val - 1) / sector_size) + 1) * sector_size;
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemN88::GetDataSize() const
{
	return sizeof(directory_n88_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemN88::SetStartGroup(wxUint32 val)
{
	// n88
	data->n88.start_group = (val & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemN88::GetStartGroup() const
{
	// n88
	return data->n88.start_group;
}

/// ENDマークがあるか(一度も使用していないか)
bool DiskBasicDirItemN88::HasEndMark()
{
	bool val = false;
	val = ((wxUint32)data->n88.name[0] == basic->GetGroupUnusedCode());
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
	return (((GetFileType1() & (FILETYPE_N88_MACHINE | FILETYPE_N88_BINARY)) == 0) && (external_attr == 0));
}

#if 0
/// データをエクスポートする前に必要な処理
/// アスキーファイルをランダムアクセスファイルにするかダイアログ表示
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemN88::PreExportDataFile(wxString &filename)
{
	if (((GetFileType1() & (FILETYPE_N88_MACHINE | FILETYPE_N88_BINARY)) == 0) && (external_attr == 0)) {
		int sts = wxMessageBox(wxString::Format(_("Is '%s' a random access file?"), filename)
			, _("Select file type."), wxYES_NO);
		if (sts == wxYES) {
			external_attr = 1;
		}
	}
	return true;
}
#endif

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemN88::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		// ただし、ファイルサイズがクラスタサイズと合うなら終端記号は不要
		if ((file_size % (basic->GetSectorSize() * basic->GetSectorsPerGroup())) != 0) {
			file_size = CheckEofCode(istream, file_size);
		}
	}
	return file_size;
}

/// ディレクトリをクリア
void DiskBasicDirItemN88::ClearData()
{
	if (!data) return;
	int c = basic->GetFillCodeOnDir();
	size_t l;
	l = GetDataSize();
	memset(data, c, l);
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
int DiskBasicDirItemN88::GetFileType1Pos() const
{
	int t = GetFileType1();
	int val = 0;
	if ((t & FILETYPE_N88_MACHINE) != 0) {
		val = TYPE_NAME_N88_MACHINE;
	} else {
		if (t & FILETYPE_N88_BINARY) {
			val = TYPE_NAME_N88_BINARY;
		} else if (external_attr) {
			val = TYPE_NAME_N88_RANDOM;
		} else {
			val = TYPE_NAME_N88_ASCII;
		}
	}
	return val;
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemN88::GetFileType2Pos() const
{
	return GetFileAttr().GetType();
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemN88::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_INVALID_FILE_TYPE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		wxString ext = name.Right(4).Upper();
		if (ext == wxT(".BAS")) {
			file_type_1 = TYPE_NAME_N88_BINARY;
		} else if (ext == wxT(".DAT") || ext == wxT(".TXT")) {
			file_type_1 = TYPE_NAME_N88_ASCII;
		} else if (ext == wxT(".BIN")) {
			file_type_1 = TYPE_NAME_N88_MACHINE;
		}
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
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();
	wxRadioBox *radType1;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkReadWrite;
	wxCheckBox *chkEncrypt;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString types1;
	for(size_t i=0; gTypeNameN88_1[i] != NULL; i++) {
		types1.Add(wxGetTranslation(gTypeNameN88_1[i]));
	}
	radType1 = new wxRadioBox(parent, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 0, wxRA_SPECIFY_ROWS);
	radType1->SetSelection(file_type_1);
	sizer->Add(radType1, flags);
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
	sizer->Add(staType4, flags);

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
int	DiskBasicDirItemN88::CalcFileTypeFromPos(int pos)
{
	int val = 0;
	val = (pos == TYPE_NAME_N88_MACHINE ? FILE_TYPE_MACHINE_MASK : (
		(pos == TYPE_NAME_N88_BINARY ? FILE_TYPE_BINARY_MASK : (
		FILE_TYPE_ASCII_MASK
	))));

	external_attr = (pos == TYPE_NAME_N88_RANDOM ? 1 : 0);

	return val;
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemN88::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo)
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkReadWrite = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READWRITE);
	wxCheckBox *chkEncrypt = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ENCRYPT);

	int val = CalcFileTypeFromPos(radType1->GetSelection());
	val |= chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkEncrypt->GetValue() ? FILE_TYPE_ENCRYPTED_MASK : 0;
	val |= chkReadWrite->GetValue() ? FILE_TYPE_READWRITE_MASK : 0;

	DiskBasicDirItem::SetFileAttr(val);

	return true;
}
