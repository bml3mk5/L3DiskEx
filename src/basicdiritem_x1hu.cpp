/// @file basicdiritem_x1hu.cpp
///
/// @brief disk basic directory item for X1 Hu-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_x1hu.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include <wx/datetime.h>
#include <wx/msgdlg.h>
#include "utils.h"


//
//
//

/// X1 Hu-BASIC
const char *gTypeNameX1HU_1[] = {
	wxTRANSLATE("Bin"),
	wxTRANSLATE("Bas"),
	wxTRANSLATE("Asc"),
	wxTRANSLATE("<DIR>"),
	wxTRANSLATE("Asc(Random Access)"),
	NULL
};
const char *gTypeNameX1HU_2[] = {
	wxTRANSLATE("Hidden"),
	wxTRANSLATE("Read After Write"),
	wxTRANSLATE("Write Protected"),
	wxTRANSLATE("Password"),
	NULL
};

//
//
//
DiskBasicDirItemX1HU::DiskBasicDirItemX1HU(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemX1HU::DiskBasicDirItemX1HU(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, data)
{
}
DiskBasicDirItemX1HU::DiskBasicDirItemX1HU(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	// x1 hu
	Used(CheckUsed(unuse));

	// ファイルサイズをセット
	file_size = wxUINT16_SWAP_ON_BE(this->data->x1hu.file_size);

	// グループ数を計算
	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemX1HU::GetFileNamePos(size_t &len, bool *invert) const
{
	// X1 Hu
	len = sizeof(data->x1hu.name);
	if (invert) *invert = basic->IsDataInverted();
	return data->x1hu.name; 
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemX1HU::GetFileExtPos(size_t &len) const
{
	len = sizeof(data->x1hu.ext);
	return data->x1hu.ext;
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemX1HU::GetFileNameSize(bool *invert) const
{
	if (invert) *invert = basic->IsDataInverted();
	return (int)sizeof(data->x1hu.name);
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemX1HU::GetFileExtSize(bool *invert) const
{
	if (invert) *invert = basic->IsDataInverted();
	return (int)sizeof(data->x1hu.ext);
}

/// 属性１を返す
int	DiskBasicDirItemX1HU::GetFileType1() const
{
	return basic->InvertUint8(data->x1hu.type);
}

/// 属性２を返す
int	DiskBasicDirItemX1HU::GetFileType2() const
{
	return basic->InvertUint8(data->x1hu.password);
}

/// 属性１を設定
void DiskBasicDirItemX1HU::SetFileType1(int val)
{
	data->x1hu.type = basic->InvertUint8(val);
}

/// 属性２を設定
void DiskBasicDirItemX1HU::SetFileType2(int val)
{
	data->x1hu.password = basic->InvertUint8(val);
}

/// 使用しているアイテムか
bool DiskBasicDirItemX1HU::CheckUsed(bool unuse)
{
	int type1 = GetFileType1();
	return (type1 != 0 && type1 != 0xff);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemX1HU::Check(bool &last)
{
	if (!data) return false;

	int type1 = GetFileType1();
	bool valid = true;
	// 属性が不正
	if (type1 != 0xff && (type1 & 0x08) != 0) {
		valid = false;
	}
	return valid;
}

void DiskBasicDirItemX1HU::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	// X1 Hu
	int t = GetFileType1();
	t &= ~FILETYPE_X1HU_MASK;
	if (ftype & FILE_TYPE_BINARY_MASK) {
		// bin
		t |= FILETYPE_X1HU_BINARY;
	} else if (ftype & FILE_TYPE_BASIC_MASK) {
		// bas
		t |= FILETYPE_X1HU_BASIC;
	} else if (ftype & FILE_TYPE_ASCII_MASK) {
		// asc
		t |= FILETYPE_X1HU_ASCII;
	} else if (ftype & FILE_TYPE_DIRECTORY_MASK) {
		// sub directory
		t |= FILETYPE_X1HU_DIRECTORY;
	}
	t &= ~DATATYPE_X1HU_MASK;
	t |= (ftype & FILE_TYPE_HIDDEN_MASK ? DATATYPE_X1HU_HIDDEN : 0);
	t |= (ftype & FILE_TYPE_READWRITE_MASK ? DATATYPE_X1HU_READ_WRITE : 0);
	t |= (ftype & FILE_TYPE_READONLY_MASK ? DATATYPE_X1HU_READ_ONLY : 0);
	SetFileType1(t);

	// password
	int passwd = DATATYPE_X1HU_PASSWORD_NONE;
	if (ftype & FILE_TYPE_ENCRYPTED_MASK) {
		passwd = file_type.GetOrigin();
	}
	SetFileType2(passwd);
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemX1HU::InitialData()
{
	if (!data) return;
	int c = basic->GetFillCodeOnDir();
	size_t l;
	l = GetDataSize();
	memset(data, c, l);
	if (basic->IsDataInverted()) mem_invert(data, l);
}

DiskBasicFileType DiskBasicDirItemX1HU::GetFileAttr() const
{
	int t = GetFileType1();
	int val = 0;
	if (t & FILETYPE_X1HU_BINARY) {
		val = FILE_TYPE_MACHINE_MASK;	// bin
		val |= FILE_TYPE_BINARY_MASK;
	} else if (t & FILETYPE_X1HU_BASIC) {
		val = FILE_TYPE_BASIC_MASK;		// bas
	} else if (t & FILETYPE_X1HU_ASCII) {
		val = FILE_TYPE_ASCII_MASK;		// asc
	} else if (t & FILETYPE_X1HU_DIRECTORY) {
		val = FILE_TYPE_DIRECTORY_MASK;	// sub directory
	}

	int passwd = GetFileType2();
	if (passwd != DATATYPE_X1HU_PASSWORD_NONE) {
		val |= FILE_TYPE_ENCRYPTED_MASK;
	}
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, passwd);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemX1HU::GetFileAttrStr() const
{
	wxString attr = wxGetTranslation(gTypeNameX1HU_1[GetFileType1Pos()]);

	int t = GetFileType1();
	if (t & DATATYPE_X1HU_HIDDEN) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_HIDDEN]);	// hidden
	}
	if (t & DATATYPE_X1HU_READ_WRITE) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_READ_WRITE]);	// read after write
	}
	if (t & DATATYPE_X1HU_READ_ONLY) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_READ_ONLY]);	// read only
	}
	if (GetFileType2() != DATATYPE_X1HU_PASSWORD_NONE) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_PASSWORD]);	// password
	}
	return attr;
}

/// ファイルサイズをセット
void DiskBasicDirItemX1HU::SetFileSize(int val)
{
	if (GetFileType1() & FILETYPE_X1HU_ASCII) {
		// Ascファイルの場合
		// ファイルサイズはセクタサイズで丸める
		file_size = (((val - 1) / basic->GetSectorSize()) + 1) * basic->GetSectorSize();
		// ディレクトリ内のファイルサイズは0
		data->x1hu.file_size = 0;
	} else {
		file_size = val;
		data->x1hu.file_size = wxUINT16_SWAP_ON_BE(basic->InvertUint16(val));
	}
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemX1HU::CalcFileSize()
{
	if (!IsUsed()) return;

	bool rc = true;
	int calc_file_size = 0;
	int calc_groups = 0; 
	wxUint32 last_group = GetStartGroup();
//	int last_sector = 0;

	// 8bit FAT
	wxUint32 group_num = last_group;
	bool working = true;
	int limit = basic->GetFatEndGroup() + 1;
	while(working) {
		wxUint32 next_group = type->GetGroupNumber(group_num);
		if (next_group == group_num) {
			// 同じポジションならエラー
			rc = false;
		} else if (next_group >= basic->GetGroupFinalCode() && next_group <= basic->GetGroupSystemCode()) {
			// 最終グループ(0x80 - 0xff)
//			last_sector = next_group;
			calc_file_size += (basic->GetSectorSize() * (next_group - basic->GetGroupFinalCode() + 1));
			calc_groups++;
			working = false;
		} else if (next_group <= (wxUint32)basic->GetFatEndGroup()) {
			// 次グループ
			calc_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			calc_groups++;
			last_group = group_num;
			group_num = next_group;
			limit--;
		} else {
			// グループ番号がおかしい
			rc = false;
		}
		working = working && rc && (limit >= 0);
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}

	if (rc) {
		groups = calc_groups;
		// ファイルサイズが0のとき(Ascなど)
		if (file_size == 0
		|| ((file_size + basic->GetSectorSize() - 1) / basic->GetSectorSize()) != (calc_file_size / basic->GetSectorSize())) {
			file_size = calc_file_size;
		}
	}
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemX1HU::GetAllGroups(DiskBasicGroups &group_items)
{
	bool rc = true;
//	file_size = 0;
//	groups = 0; 

	// 8bit FAT
	wxUint32 group_num = GetStartGroup();
	bool working = true;
	int limit = basic->GetFatEndGroup() + 1;
	while(working) {
		wxUint32 next_group = type->GetGroupNumber(group_num);
		if (next_group == group_num) {
			// 同じポジションならエラー
			rc = false;
		} else if (next_group >= basic->GetGroupFinalCode() && next_group <= basic->GetGroupSystemCode()) {
			// 最終グループ(0x80 - 0xff)
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), 0, group_items);
//			file_size += (sector_size * (next_group - group_final_code + 1));
//			groups++;
			working = false;
		} else if (next_group <= (wxUint32)basic->GetFatEndGroup()) {
			// 次グループ
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), 0, group_items);
//			file_size += (sector_size * secs_per_group);
//			groups++;
			group_num = next_group;
			limit--;
		} else {
			// グループ番号がおかしい
			rc = false;
		}
		working = working && rc && (limit >= 0);
	}

	group_items.SetSize(file_size);

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
}

void DiskBasicDirItemX1HU::GetFileDate(struct tm *tm) const
{
	wxUint8 date[sizeof(data->x1hu.date) + 1];
	basic->InvertMem(data->x1hu.date, sizeof(data->x1hu.date), date);
	tm->tm_year = ((date[0] & 0xf0) >> 4) * 10 + (date[0] & 0x0f);	// BCD
	tm->tm_mon = ((date[1] & 0xf0) >> 4) - 1;
	tm->tm_mday = ((date[2] & 0xf0) >> 4) * 10 + (date[2] & 0x0f);	// BCD
	if (tm->tm_year < 80) tm->tm_year += 100;	// 2000 - 2079
}

void DiskBasicDirItemX1HU::GetFileTime(struct tm *tm) const
{
	wxUint8 time[sizeof(data->x1hu.time) + 1];
	basic->InvertMem(data->x1hu.time, sizeof(data->x1hu.time), time);
	tm->tm_hour = ((time[0] & 0xf0) >> 4) * 10 + (time[0] & 0x0f);	// BCD
	tm->tm_min = ((time[1] & 0xf0) >> 4) * 10 + (time[1] & 0x0f);	// BCD
	tm->tm_sec = 0;
}

wxString DiskBasicDirItemX1HU::GetFileDateStr() const
{
	struct tm tm;
	GetFileDate(&tm);
	return Utils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemX1HU::GetFileTimeStr() const
{
	struct tm tm;
	GetFileTime(&tm);
	return Utils::FormatHMStr(&tm);
}

void DiskBasicDirItemX1HU::SetFileDate(const struct tm *tm)
{
	if (tm->tm_year < 0 || tm->tm_mon < -1) return;

	data->x1hu.date[0] = (((tm->tm_year / 10) % 10) << 4) | (tm->tm_year % 10);	// year BCD
	data->x1hu.date[1] = (((tm->tm_mon + 1) & 0xf) << 4);	// month
	data->x1hu.date[2] = ((tm->tm_mday / 10) << 4) | (tm->tm_mday % 10);	// day BCD

	// 日付から曜日を計算
	int wk = 0;
	wxDateTime dt;
	wxString sdate = wxString::Format(wxT("%04d-%02d-%02d")
		, (int)tm->tm_year + 1900
		, (int)tm->tm_mon + 1
		, (int)tm->tm_mday
	);
	dt.ParseISODate(sdate);
	if (dt.IsValid()) {
		wk = (int)dt.GetWeekDay();
	}
	data->x1hu.date[1] |= (wk & 0xf);	// day of week

	if (basic->IsDataInverted()) mem_invert(data->x1hu.date, sizeof(data->x1hu.date));
}

void DiskBasicDirItemX1HU::SetFileTime(const struct tm *tm)
{
	if (tm->tm_hour < 0 || tm->tm_min < 0) return;

	data->x1hu.time[0] = ((tm->tm_hour / 10) << 4) | (tm->tm_hour % 10);	// hour BCD
	data->x1hu.time[1] = ((tm->tm_min / 10) << 4) | (tm->tm_min % 10);	// minute BCD

	if (basic->IsDataInverted()) mem_invert(data->x1hu.time, sizeof(data->x1hu.time));
}

/// 開始アドレスを返す
int DiskBasicDirItemX1HU::GetStartAddress() const
{
	return basic->InvertUint16(wxUINT16_SWAP_ON_BE(data->x1hu.load_addr));
}

/// 実行アドレスを返す
int DiskBasicDirItemX1HU::GetExecuteAddress() const
{
	return basic->InvertUint16(wxUINT16_SWAP_ON_BE(data->x1hu.exec_addr));
}

/// 開始アドレスをセット
void DiskBasicDirItemX1HU::SetStartAddress(int val)
{
	data->x1hu.load_addr = (wxUint16)wxUINT16_SWAP_ON_BE(basic->InvertUint16(val));
}

/// 実行アドレスをセット
void DiskBasicDirItemX1HU::SetExecuteAddress(int val)
{
	data->x1hu.exec_addr = (wxUint16)wxUINT16_SWAP_ON_BE(basic->InvertUint16(val));
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemX1HU::GetDataSize() const
{
	return sizeof(directory_x1_hu_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemX1HU::SetStartGroup(wxUint32 val)
{
	// X1 Hu-BASIC
	data->x1hu.start_group_h = basic->InvertUint8((val & 0xff0000) >> 16);
	data->x1hu.start_group_l = wxUINT16_SWAP_ON_BE(basic->InvertUint16(val & 0xffff));
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemX1HU::GetStartGroup() const
{
	// X1 Hu-BASIC
	return (wxUint32)basic->InvertUint8(data->x1hu.start_group_h) << 16 | basic->InvertUint16(wxUINT16_SWAP_ON_BE(data->x1hu.start_group_l));
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemX1HU::NeedCheckEofCode()
{
	// Asc形式のときはEOFコードが必要
	return ((GetFileType1() & FILETYPE_X1HU_ASCII) != 0 && (external_attr == 0));
}

#if 0
/// データをエクスポートする前に必要な処理
/// アスキーファイルをランダムアクセスファイルにするかダイアログ表示
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemX1HU::PreExportDataFile(wxString &filename)
{
	if ((GetFileType1() & FILETYPE_X1HU_ASCII) != 0 && (external_attr == 0)) {
		int sts = wxMessageBox(wxString::Format(_("Is '%s' a random access file?"), filename)
			, _("Select file type."), wxYES_NO);
		external_attr = (sts == wxYES ? 1 : 0);
	}
	return true;
}
#endif

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemX1HU::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		file_size = CheckEofCode(istream, file_size);
	}
	return file_size;
}

/// 同じファイル名か
bool DiskBasicDirItemX1HU::IsSameFileName(const DiskBasicFileName &filename) const
{
	// 属性が0以外
	if (GetFileType1() == 0) return false;

	return DiskBasicDirItem::IsSameFileName(filename);
}

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_RADIO_BINARY	51
#define IDC_RADIO_BASIC		52
#define IDC_RADIO_ASCII		53
#define IDC_RADIO_DIR		54
#define IDC_RADIO_RANDOM	55

#define IDC_CHECK_HIDDEN	56
#define IDC_CHECK_READWRITE 57
#define IDC_CHECK_READONLY	58
#define IDC_CHECK_ENCRYPT	59

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemX1HU::GetFileType1Pos() const
{
	int t = GetFileType1();
	int val = 0;
	if (t & FILETYPE_X1HU_BINARY) {
		val = TYPE_NAME_X1HU_BINARY;	// bin
	} else if (t & FILETYPE_X1HU_BASIC) {
		val = TYPE_NAME_X1HU_BASIC;		// bas
	} else if (t & FILETYPE_X1HU_ASCII) {
		val = (external_attr == 0 ? TYPE_NAME_X1HU_ASCII : TYPE_NAME_X1HU_RANDOM);		// asc
	} else if (t & FILETYPE_X1HU_DIRECTORY) {
		val = TYPE_NAME_X1HU_DIRECTORY;	// sub directory
	}
	return val;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemX1HU::GetFileType2Pos() const
{
	int t = GetFileType1();
	int val = 0;
	val |= (t & DATATYPE_X1HU_HIDDEN ? FILE_TYPE_HIDDEN_MASK : 0);
	val |= (t & DATATYPE_X1HU_READ_WRITE ? FILE_TYPE_READWRITE_MASK : 0);
	val |= (t & DATATYPE_X1HU_READ_ONLY ? FILE_TYPE_READONLY_MASK : 0);

	return val;
}

/// ダイアログ用に属性を設定する
/// インポート時ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemX1HU::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_INVALID_FILE_TYPE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		wxString ext = name.Right(4).Upper();
		if (ext == wxT(".BAS")) {
			file_type_1 = TYPE_NAME_X1HU_BASIC;
		} else if (ext == wxT(".DAT") || ext == wxT(".TXT")) {
			file_type_1 = TYPE_NAME_X1HU_ASCII;
		} else if (ext == wxT(".BIN")) {
			file_type_1 = TYPE_NAME_X1HU_BINARY;
		}
		// パスワードなし
		file_type_2 &= ~FILE_TYPE_ENCRYPTED_MASK;
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemX1HU::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();

	wxRadioButton *radType1[TYPE_NAME_X1HU_RANDOM + 1];
	wxCheckBox *chkHidden;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkReadWrite;
	wxCheckBox *chkEncrypt;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);
	wxBoxSizer *hbox;
	hbox = new wxBoxSizer(wxHORIZONTAL);
	radType1[TYPE_NAME_X1HU_BINARY] = new wxRadioButton(parent, IDC_RADIO_BINARY, wxGetTranslation(gTypeNameX1HU_1[TYPE_NAME_X1HU_BINARY]));
	hbox->Add(radType1[TYPE_NAME_X1HU_BINARY], flags);
	radType1[TYPE_NAME_X1HU_BASIC] = new wxRadioButton(parent, IDC_RADIO_BASIC,   wxGetTranslation(gTypeNameX1HU_1[TYPE_NAME_X1HU_BASIC]));
	hbox->Add(radType1[TYPE_NAME_X1HU_BASIC], flags);
	radType1[TYPE_NAME_X1HU_ASCII] = new wxRadioButton(parent, IDC_RADIO_ASCII,   wxGetTranslation(gTypeNameX1HU_1[TYPE_NAME_X1HU_ASCII]));
	hbox->Add(radType1[TYPE_NAME_X1HU_ASCII], flags);
	radType1[TYPE_NAME_X1HU_DIRECTORY] = new wxRadioButton(parent, IDC_RADIO_DIR, wxGetTranslation(gTypeNameX1HU_1[TYPE_NAME_X1HU_DIRECTORY]));
	hbox->Add(radType1[TYPE_NAME_X1HU_DIRECTORY], flags);
	staType1->Add(hbox);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	radType1[TYPE_NAME_X1HU_RANDOM] = new wxRadioButton(parent, IDC_RADIO_RANDOM, wxGetTranslation(gTypeNameX1HU_1[TYPE_NAME_X1HU_RANDOM]));
	hbox->Add(radType1[TYPE_NAME_X1HU_RANDOM], flags);
	staType1->Add(hbox);
	if (file_type_1 <= TYPE_NAME_X1HU_RANDOM) {
		radType1[file_type_1]->SetValue(true);
	}
	sizer->Add(staType1, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkHidden = new wxCheckBox(parent, IDC_CHECK_HIDDEN, wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_HIDDEN]));
	chkHidden->SetValue((file_type_2 & FILE_TYPE_HIDDEN_MASK) != 0);
	staType4->Add(chkHidden, flags);
	chkReadWrite = new wxCheckBox(parent, IDC_CHECK_READWRITE, wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_READ_WRITE]));
	chkReadWrite->SetValue((file_type_2 & FILE_TYPE_READWRITE_MASK) != 0);
	staType4->Add(chkReadWrite, flags);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_READ_ONLY]));
	chkReadOnly->SetValue((file_type_2 & FILE_TYPE_READONLY_MASK) != 0);
	staType4->Add(chkReadOnly, flags);
	chkEncrypt = new wxCheckBox(parent, IDC_CHECK_ENCRYPT, wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_PASSWORD]));
	// ユーザ定義データ X1ではファイルパスワード
	int passwd;
	if (show_flags & INTNAME_NEW_FILE) {
		passwd = DATATYPE_X1HU_PASSWORD_NONE;
	} else {
		passwd = GetFileType2();
	}
	parent->SetUserData(passwd);
	chkEncrypt->SetValue(passwd != DATATYPE_X1HU_PASSWORD_NONE);
	chkEncrypt->Enable(false);
	staType4->Add(chkEncrypt, flags);
	sizer->Add(staType4, flags);

	// event handler
	parent->Bind(wxEVT_RADIOBUTTON, &IntNameBox::OnChangeType1, parent, IDC_RADIO_BINARY);
	parent->Bind(wxEVT_RADIOBUTTON, &IntNameBox::OnChangeType1, parent, IDC_RADIO_BASIC);
	parent->Bind(wxEVT_RADIOBUTTON, &IntNameBox::OnChangeType1, parent, IDC_RADIO_ASCII);
	parent->Bind(wxEVT_RADIOBUTTON, &IntNameBox::OnChangeType1, parent, IDC_RADIO_DIR);
	parent->Bind(wxEVT_RADIOBUTTON, &IntNameBox::OnChangeType1, parent, IDC_RADIO_RANDOM);
}

/// ダイアログ内の値を設定
void DiskBasicDirItemX1HU::InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data)
{
	// 日付が０なら日付を無視するにチェック
	if ((show_flags & INTNAME_NEW_FILE) == 0) {
		struct tm tm;
		GetFileDateTime(&tm);
		parent->IgnoreDateTime(
			tm.tm_mon == -1 && ((tm.tm_mday == 0 && tm.tm_hour == 0 && tm.tm_min == 0)
			|| (tm.tm_mday == 0 || tm.tm_mday > 31 || tm.tm_hour > 24 || tm.tm_min > 61))
		);
	}
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemX1HU::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxRadioButton *radTypeBinary = (wxRadioButton *)parent->FindWindow(IDC_RADIO_BINARY);
	wxTextCtrl *txtStartAddr = (wxTextCtrl *)parent->FindWindow(IntNameBox::IDC_TEXT_START_ADDR);
	wxTextCtrl *txtExecAddr = (wxTextCtrl *)parent->FindWindow(IntNameBox::IDC_TEXT_EXEC_ADDR);

	if (!radTypeBinary) return;

	bool enable = (radTypeBinary->GetValue());
	if (txtStartAddr) {
		txtStartAddr->Enable(enable);
	}
	if (txtExecAddr) {
		txtExecAddr->Enable(enable);
	}
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemX1HU::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxRadioButton *radType1[TYPE_NAME_X1HU_RANDOM + 1];
	radType1[TYPE_NAME_X1HU_BINARY] = (wxRadioButton *)parent->FindWindow(IDC_RADIO_BINARY);
	radType1[TYPE_NAME_X1HU_BASIC] = (wxRadioButton *)parent->FindWindow(IDC_RADIO_BASIC);
	radType1[TYPE_NAME_X1HU_ASCII] = (wxRadioButton *)parent->FindWindow(IDC_RADIO_ASCII);
	radType1[TYPE_NAME_X1HU_DIRECTORY] = (wxRadioButton *)parent->FindWindow(IDC_RADIO_DIR);
	radType1[TYPE_NAME_X1HU_RANDOM] = (wxRadioButton *)parent->FindWindow(IDC_RADIO_RANDOM);

	int sel = 0;
	for(int i=TYPE_NAME_X1HU_BINARY; i<=TYPE_NAME_X1HU_RANDOM; i++) {
		if (radType1[i]->GetValue()) {
			sel = i;
			break;
		}
	}

	return sel;
}

/// 属性2を得る
int DiskBasicDirItemX1HU::GetFileType2InAttrDialog(const IntNameBox *parent) const
{
	wxCheckBox *chkHidden = (wxCheckBox *)parent->FindWindow(IDC_CHECK_HIDDEN);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkReadWrite = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READWRITE);

	int val = chkHidden->GetValue() ? FILE_TYPE_HIDDEN_MASK : 0;
	val |= chkReadWrite->GetValue() ? FILE_TYPE_READWRITE_MASK : 0;
	val |= chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	return val;
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int	DiskBasicDirItemX1HU::CalcFileTypeFromPos(int pos)
{
	int val = 0;
	switch(pos) {
	case TYPE_NAME_X1HU_BINARY:
		val = FILE_TYPE_BINARY_MASK;
		break;
	case TYPE_NAME_X1HU_BASIC:
		val = FILE_TYPE_BASIC_MASK;
		break;
	case TYPE_NAME_X1HU_ASCII:
		external_attr = 0;
		val = FILE_TYPE_ASCII_MASK;
		break;
	case TYPE_NAME_X1HU_DIRECTORY:
		val = FILE_TYPE_DIRECTORY_MASK;
		break;
	case TYPE_NAME_X1HU_RANDOM:
		external_attr = 1;
		val = FILE_TYPE_ASCII_MASK;
		break;
	}
	return val;
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemX1HU::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo)
{
	int val = CalcFileTypeFromPos(GetFileType1InAttrDialog(parent));
	val |= GetFileType2InAttrDialog(parent);

	// ユーザ定義データ X1ではファイルパスワード
	wxCheckBox *chkEncrypt = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ENCRYPT);
	int passwd = DATATYPE_X1HU_PASSWORD_NONE;
	if (chkEncrypt->GetValue()) {
		val |= FILE_TYPE_ENCRYPTED_MASK;
		passwd = parent->GetUserData();
	}

	DiskBasicDirItem::SetFileAttr(val, passwd);

	return true;
}

/// ファイルサイズが適正か
bool DiskBasicDirItemX1HU::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	int limit_size = 0xffff;
	if (limit) *limit = limit_size;

	int file_type1 = GetFileType1InAttrDialog(parent);

	if (file_type1 == TYPE_NAME_X1HU_BINARY || file_type1 == TYPE_NAME_X1HU_BASIC) {
		return (size <= limit_size);
	} else {
		return true;
	}
}
