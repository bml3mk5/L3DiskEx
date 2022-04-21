/// @file basicdiritem_n88.cpp
///
/// @brief disk basic directory item for N88-BASIC
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
	used = CheckUsed(unuse);
	unuse = (unuse || (this->data->n88.name[0] == 0xff));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemN88::GetFileNamePos(size_t &len, bool *invert) const
{
	// N88
	len = sizeof(data->n88.name);
	return data->n88.name;
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemN88::GetFileExtPos(size_t &len) const
{
	len = sizeof(data->n88.ext);
	return data->n88.ext;
}

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
void DiskBasicDirItemN88::SetFileAttr(int file_type)
{
	if (file_type == -1) return;

	// n88
	int t = 0;
	if (file_type & FILE_TYPE_MACHINE_MASK) {
		t = FILETYPE_N88_MACHINE;
	} else if (file_type & FILE_TYPE_BINARY_MASK) {
		t = FILETYPE_N88_BINARY;
	} else {
		t = FILETYPE_N88_ASCII;
	}
	if (file_type & FILE_TYPE_READONLY_MASK) {
		t |= DATATYPE_MASK_N88_READ_ONLY;
	}
	if (file_type & FILE_TYPE_ENCRYPTED_MASK) {
		t |= DATATYPE_MASK_N88_ENCRYPTED;
	}
	if (file_type & FILE_TYPE_READWRITE_MASK) {
		t |= DATATYPE_MASK_N88_READ_WRITE;
	}
	SetFileType1(t);
}

int DiskBasicDirItemN88::GetFileType()
{
	int t = GetFileType1();
	int val = 0;
	if ((t & FILETYPE_N88_MACHINE) != 0) {
		val = FILE_TYPE_MACHINE_MASK;		// machine
		val |= FILE_TYPE_BINARY_MASK;		// binary
	} else {
		val = FILE_TYPE_BASIC_MASK;			// basic
		if (t & FILETYPE_N88_BINARY) {
			val |= FILE_TYPE_BINARY_MASK;	// binary
		} else {
			val |= FILE_TYPE_ASCII_MASK;	// ascii
		}
	}
	if (t & DATATYPE_MASK_N88_READ_ONLY) {
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (t & DATATYPE_MASK_N88_ENCRYPTED) {
		val |= FILE_TYPE_ENCRYPTED_MASK;
	}
	if (t & DATATYPE_MASK_N88_READ_WRITE) {
		val |= FILE_TYPE_READWRITE_MASK;
	}
	return val;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemN88::GetFileType1Pos()
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

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemN88::GetFileType2Pos()
{
	return GetFileType();
}

int	DiskBasicDirItemN88::CalcFileTypeFromPos(int pos1, int pos2)
{
	int val = 0;
	val = (pos1 == TYPE_NAME_N88_MACHINE ? FILE_TYPE_MACHINE_MASK : (
		(pos1 == TYPE_NAME_N88_BINARY ? FILE_TYPE_BINARY_MASK : (
		FILE_TYPE_ASCII_MASK
	))));
	val |= pos2;

	external_attr = (pos1 == TYPE_NAME_N88_RANDOM ? 1 : 0);

	return val;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemN88::GetFileAttrStr()
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
size_t DiskBasicDirItemN88::GetDataSize()
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

/// 書き込み/上書き禁止か
bool DiskBasicDirItemN88::IsWriteProtected()
{
	return (data->n88.type & 0x10) != 0;
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemN88::NeedCheckEofCode()
{
	// Asc形式のときはEOFコードが必要
	return (((GetFileType1() & (FILETYPE_N88_MACHINE | FILETYPE_N88_BINARY)) == 0) && (external_attr == 0));
}

/// データをエクスポートする前に必要な処理
/// @return false エクスポート中断
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

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemN88::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		// ただし、ファイルサイズがクラスタサイズと合うなら終端記号は不要
		if ((file_size % basic->GetSectorsPerGroup()) != 0) {
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

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] file_type_1    ファイル属性1 GetFileType1Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] file_type_2    ファイル属性2 GetFileType2Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] sizer
/// @param [in] flags
/// @param [in,out] controls   [0]: wxTextCtrl::txtIntNameで予約済み [1]からユーザ設定
/// @param [in,out] user_data  ユーザ定義データ
void DiskBasicDirItemN88::CreateControlsForAttrDialog(IntNameBox *parent, int file_type_1, int file_type_2, wxBoxSizer *sizer, wxSizerFlags &flags, AttrControls &controls, int *user_data)
{
	wxRadioBox *radType1;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkReadWrite;
	wxCheckBox *chkEncrypt;

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

	controls.Add(radType1);
	controls.Add(chkReadOnly);
	controls.Add(chkReadWrite);
	controls.Add(chkEncrypt);
}

/// controls 0:txtIntName 1...: dependent
void DiskBasicDirItemN88::ChangeTypeInAttrDialog(AttrControls &controls)
{
	wxRadioBox *radType1 = (wxRadioBox *)controls.Item(1);
	wxCheckBox *chkReadOnly = (wxCheckBox *)controls.Item(2);
	wxCheckBox *chkReadWrite = (wxCheckBox *)controls.Item(3);
	wxCheckBox *chkEncrypt = (wxCheckBox *)controls.Item(4);

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

/// ダイアログ用に属性を設定する
/// インポート時ダイアログ表示前にファイルの属性を設定
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemN88::SetFileTypeForAttrDialog(const wxString &name, int &file_type_1, int &file_type_2)
{
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

/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemN88::GetFileType1InAttrDialog(const AttrControls &controls) const
{
	wxRadioBox *radType1 = (wxRadioBox *)controls.Item(1);

	return radType1->GetSelection();
}

/// @return CalcFileTypeFromPos()のpos2に渡す値
int DiskBasicDirItemN88::GetFileType2InAttrDialog(const AttrControls &controls, const int *user_data) const
{
	wxCheckBox *chkReadOnly = (wxCheckBox *)controls.Item(2);
	wxCheckBox *chkReadWrite = (wxCheckBox *)controls.Item(3);
	wxCheckBox *chkEncrypt = (wxCheckBox *)controls.Item(4);

	int val = chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkEncrypt->GetValue() ? FILE_TYPE_ENCRYPTED_MASK : 0;
	val |= chkReadWrite->GetValue() ? FILE_TYPE_READWRITE_MASK : 0;

	return val;
}
