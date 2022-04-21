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
#include "config.h"
#include "utils.h"


//
//
//

/// X1 Hu-BASIC
const char *gTypeNameX1HU_1[] = {
	"Bin",
	"Bas",
	wxTRANSLATE("Asc(Hu)"),
	wxTRANSLATE("Asc(S-OS)"),
	wxTRANSLATE("Asc(Random Access)"),
	"<DIR>",
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
	m_external_attr = basic->GetVariousIntegerParam(wxT("DefaultAsciiType"));
}
DiskBasicDirItemX1HU::DiskBasicDirItemX1HU(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, secpos, data)
{
	m_external_attr = basic->GetVariousIntegerParam(wxT("DefaultAsciiType"));
}
DiskBasicDirItemX1HU::DiskBasicDirItemX1HU(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	m_external_attr = basic->GetVariousIntegerParam(wxT("DefaultAsciiType"));

	Used(CheckUsed(unuse));

	// グループ数を計算
	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemX1HU::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// X1 Hu
	if (num == 0) {
		size = len = sizeof(m_data->x1hu.name);
		return m_data->x1hu.name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemX1HU::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data->x1hu.ext);
	return m_data->x1hu.ext;
}

/// 属性１を返す
int	DiskBasicDirItemX1HU::GetFileType1() const
{
	return basic->InvertUint8(m_data->x1hu.type);
}

/// 属性２を返す
int	DiskBasicDirItemX1HU::GetFileType2() const
{
	return basic->InvertUint8(m_data->x1hu.password);
}

/// 属性１を設定
void DiskBasicDirItemX1HU::SetFileType1(int val)
{
	m_data->x1hu.type = basic->InvertUint8(val);
}

/// 属性２を設定
void DiskBasicDirItemX1HU::SetFileType2(int val)
{
	m_data->x1hu.password = basic->InvertUint8(val);
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
	if (!m_data) return false;

	int type1 = GetFileType1();
	bool valid = true;
	// 属性が不正
	if (type1 != 0xff && (type1 & 0x08) != 0) {
		valid = false;
	}
	return valid;
}

/// 属性を設定
void DiskBasicDirItemX1HU::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	int passwd = DATATYPE_X1HU_PASSWORD_NONE;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		t1 = file_type.GetOrigin();
		passwd = ((t1 >> 8) & 0xff);
	} else {
		t1 = ConvToNativeType(ftype, GetFileType1());

		t1 &= ~DATATYPE_X1HU_MASK;
		t1 |= (ftype & FILE_TYPE_HIDDEN_MASK ? DATATYPE_X1HU_HIDDEN : 0);
		t1 |= (ftype & FILE_TYPE_READWRITE_MASK ? DATATYPE_X1HU_READ_WRITE : 0);
		t1 |= (ftype & FILE_TYPE_READONLY_MASK ? DATATYPE_X1HU_READ_ONLY : 0);

		// password
		if (ftype & FILE_TYPE_ENCRYPTED_MASK) {
			passwd = file_type.GetOrigin();
		}
	}
	m_external_attr = (t1 >> 16);
	t1 &= 0xff;

	SetFileType1(t1);
	SetFileType2(passwd);
}

/// 属性を変換
int DiskBasicDirItemX1HU::ConvToNativeType(int file_type, int val) const
{
	// X1 Hu
	val &= ~FILETYPE_X1HU_MASK;
	if ((file_type & (FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK)) == (FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK)) {
		// bin
		val |= FILETYPE_X1HU_BINARY;
	} else if (file_type & FILE_TYPE_BASIC_MASK) {
		// bas
		val |= FILETYPE_X1HU_BASIC;
	} else if (file_type & FILE_TYPE_ASCII_MASK) {
		// asc
		val |= FILETYPE_X1HU_ASCII;
	} else if (file_type & FILE_TYPE_RANDOM_MASK) {
		// random
		val |= (FILETYPE_X1HU_ASCII | (EXTERNAL_X1_RANDOM << 16));
	} else if (file_type & FILE_TYPE_DIRECTORY_MASK) {
		// sub directory
		val |= FILETYPE_X1HU_DIRECTORY;
	}
	return val;
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemX1HU::InitialData()
{
	if (!m_data) return;
	int c = basic->GetFillCodeOnDir();
	size_t l;
	l = GetDataSize();
	memset(m_data, c, l);
	if (basic->IsDataInverted()) mem_invert(m_data, l);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemX1HU::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	if (t1 & FILETYPE_X1HU_BINARY) {
		val = FILE_TYPE_MACHINE_MASK;	// bin
		val |= FILE_TYPE_BINARY_MASK;
	} else if (t1 & FILETYPE_X1HU_BASIC) {
		val = FILE_TYPE_BASIC_MASK;		// bas
		val |= FILE_TYPE_BINARY_MASK;
	} else if (t1 & FILETYPE_X1HU_ASCII) {
		val = FILE_TYPE_ASCII_MASK;		// asc
	} else if (t1 & FILETYPE_X1HU_DIRECTORY) {
		val = FILE_TYPE_DIRECTORY_MASK;	// sub directory
	}

	int passwd = GetFileType2();
	if (passwd != DATATYPE_X1HU_PASSWORD_NONE) {
		val |= FILE_TYPE_ENCRYPTED_MASK;
	}
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, m_external_attr << 16 | passwd << 8 | t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemX1HU::GetFileAttrStr() const
{
	int t = (GetFileType1() | (m_external_attr << 16));
	wxString attr = wxGetTranslation(gTypeNameX1HU_1[GetFileType1Pos(t)]);

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
	m_groups.SetSize(val);

	if (GetFileType1() & FILETYPE_X1HU_ASCII) {
		// Ascファイルの場合
//		// ファイルサイズはセクタサイズで丸める
//		m_groups.SetSize((((val - 1) / basic->GetSectorSize()) + 1) * basic->GetSectorSize());
		// ディレクトリ内のファイルサイズは0
		m_data->x1hu.file_size = 0;
	} else {
		m_data->x1hu.file_size = basic->InvertAndOrderUint16(val);
	}
}

/// ファイルサイズを返す
int	DiskBasicDirItemX1HU::GetFileSize() const
{
	if (GetFileType1() & FILETYPE_X1HU_ASCII) {
		// Ascファイルの場合
		return (int)m_groups.GetSize();
	} else {
		return basic->InvertAndOrderUint16(m_data->x1hu.file_size);
	}
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemX1HU::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemX1HU::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	bool rc = true;
	int calc_file_size = 0;
	int calc_groups = 0; 

	// 8bit FAT
	wxUint32 group_num = GetStartGroup(fileunit_num);
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
			calc_file_size += (basic->GetSectorSize() * (next_group - basic->GetGroupFinalCode() + 1));
			calc_groups++;
			calc_file_size = RecalcFileSize(group_items, calc_file_size);
			working = false;
		} else if (next_group <= (wxUint32)basic->GetFatEndGroup()) {
			// 次グループ
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), 0, group_items);
			calc_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			calc_groups++;
			group_num = next_group;
			limit--;
		} else {
			// グループ番号がおかしい
			rc = false;
		}
		working = working && rc && (limit >= 0);
	}

	group_items.SetNums(calc_groups);
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有しているファイルサイズ
/// @return 計算後のファイルサイズ
int	DiskBasicDirItemX1HU::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
	if (group_items.Count() == 0) return occupied_size;

	DiskBasicGroupItem *litem = &group_items.Last();
	DiskD88Sector *sector = basic->GetSector(litem->track, litem->side, litem->sector_end);
	if (!sector) return occupied_size;

	int sector_size = sector->GetSectorSize();
	int remain_size = ((occupied_size + sector_size - 1) % sector_size) + 1;
	remain_size = type->CalcDataSizeOnLastSector(this, NULL, NULL, sector->GetSectorBuffer(), sector_size, remain_size);

	occupied_size = occupied_size - sector_size + remain_size;
	return occupied_size;
}

/// 日付を得る
void DiskBasicDirItemX1HU::GetFileDate(struct tm *tm) const
{
	wxUint8 date[sizeof(m_data->x1hu.date) + 1];
	basic->InvertMem(m_data->x1hu.date, sizeof(m_data->x1hu.date), date);
	tm->tm_year = (date[0] <= 0x99 ? ((date[0] & 0xf0) >> 4) * 10 + (date[0] & 0x0f) : -1);	// BCD
	tm->tm_mon = ((date[1] & 0xf0) >> 4) - 1;
	tm->tm_mday = (date[2] <= 0x99 ? ((date[2] & 0xf0) >> 4) * 10 + (date[2] & 0x0f) : -1);	// BCD
	if (tm->tm_year >= 0 && tm->tm_year < 80) tm->tm_year += 100;	// 2000 - 2079
}

/// 時間を得る
void DiskBasicDirItemX1HU::GetFileTime(struct tm *tm) const
{
	wxUint8 time[sizeof(m_data->x1hu.time) + 1];
	basic->InvertMem(m_data->x1hu.time, sizeof(m_data->x1hu.time), time);
	tm->tm_hour = (time[0] <= 0x99 ? ((time[0] & 0xf0) >> 4) * 10 + (time[0] & 0x0f) : -1);	// BCD
	tm->tm_min = (time[1] <= 0x99 ? ((time[1] & 0xf0) >> 4) * 10 + (time[1] & 0x0f) : -1);	// BCD
	tm->tm_sec = 0;
}

/// 日付を文字列で返す
wxString DiskBasicDirItemX1HU::GetFileDateStr() const
{
	struct tm tm;
	GetFileDate(&tm);
	return Utils::FormatYMDStr(&tm);
}

/// 時間を文字列で返す
wxString DiskBasicDirItemX1HU::GetFileTimeStr() const
{
	struct tm tm;
	GetFileTime(&tm);
	return Utils::FormatHMStr(&tm);
}

/// 日付を設定
void DiskBasicDirItemX1HU::SetFileDate(const struct tm *tm)
{
	if (tm->tm_year < 0 || tm->tm_mon < -1 || tm->tm_mday < 0) return;

	m_data->x1hu.date[0] = (((tm->tm_year / 10) % 10) << 4) | (tm->tm_year % 10);	// year BCD
	m_data->x1hu.date[1] = (((tm->tm_mon + 1) & 0xf) << 4);	// month
	m_data->x1hu.date[2] = ((tm->tm_mday / 10) << 4) | (tm->tm_mday % 10);	// day BCD

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
	m_data->x1hu.date[1] |= (wk & 0xf);	// day of week

	if (basic->IsDataInverted()) mem_invert(m_data->x1hu.date, sizeof(m_data->x1hu.date));
}

/// 時間を設定
void DiskBasicDirItemX1HU::SetFileTime(const struct tm *tm)
{
	if (tm->tm_hour < 0 || tm->tm_min < 0) return;

	m_data->x1hu.time[0] = ((tm->tm_hour / 10) << 4) | (tm->tm_hour % 10);	// hour BCD
	m_data->x1hu.time[1] = ((tm->tm_min / 10) << 4) | (tm->tm_min % 10);	// minute BCD

	if (basic->IsDataInverted()) mem_invert(m_data->x1hu.time, sizeof(m_data->x1hu.time));
}

/// 開始アドレスを返す
int DiskBasicDirItemX1HU::GetStartAddress() const
{
	return basic->InvertAndOrderUint16(m_data->x1hu.load_addr);
}

/// 実行アドレスを返す
int DiskBasicDirItemX1HU::GetExecuteAddress() const
{
	return basic->InvertAndOrderUint16(m_data->x1hu.exec_addr);
}

/// 開始アドレスをセット
void DiskBasicDirItemX1HU::SetStartAddress(int val)
{
	m_data->x1hu.load_addr = basic->InvertAndOrderUint16(val);
}

/// 実行アドレスをセット
void DiskBasicDirItemX1HU::SetExecuteAddress(int val)
{
	m_data->x1hu.exec_addr = basic->InvertAndOrderUint16(val);
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemX1HU::GetDataSize() const
{
	return sizeof(directory_x1_hu_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemX1HU::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// X1 Hu-BASIC
	m_data->x1hu.start_group_h = basic->InvertUint8((val & 0xff0000) >> 16);
	m_data->x1hu.start_group_l = basic->InvertAndOrderUint16(val & 0xffff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemX1HU::GetStartGroup(int fileunit_num) const
{
	// X1 Hu-BASIC
	return (wxUint32)basic->InvertUint8(m_data->x1hu.start_group_h) << 16 | basic->InvertAndOrderUint16(m_data->x1hu.start_group_l);
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemX1HU::NeedCheckEofCode()
{
	// Asc形式のときはEOFコードが必要
	return ((GetFileType1() & FILETYPE_X1HU_ASCII) != 0 && (m_external_attr != EXTERNAL_X1_RANDOM));
}

/// ファイルの終端コードを返す
wxUint8	DiskBasicDirItemX1HU::GetEofCode() const
{
	return m_external_attr != EXTERNAL_X1_SWORD ? basic->GetTextTerminateCode() : 0;
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemX1HU::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		file_size = CheckEofCode(istream, file_size);
	}
	return file_size;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemX1HU::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;
	// 拡張子で属性を設定する
	wxFileName fn(filename);
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
	if (sa) {
		t1 = ConvToNativeType(sa->GetType(), t1);
		t1 |= (m_external_attr << 16);
	} else {
		t1 = FILETYPE_X1HU_ASCII;
	}

	// パスワードなし
	t1 |= (DATATYPE_X1HU_PASSWORD_NONE << 8);
	return t1;
}

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_COMBO_TYPE1		51

#define IDC_CHECK_HIDDEN	56
#define IDC_CHECK_READWRITE 57
#define IDC_CHECK_READONLY	58
#define IDC_CHECK_ENCRYPT	59

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemX1HU::GetFileType1Pos(int native_type) const
{
	int val = 0;
	if (native_type & FILETYPE_X1HU_BINARY) {
		val = TYPE_NAME_X1HU_BINARY;	// bin
	} else if (native_type & FILETYPE_X1HU_BASIC) {
		val = TYPE_NAME_X1HU_BASIC;		// bas
	} else if (native_type & FILETYPE_X1HU_ASCII) {
		switch(native_type >> 16) {
		case EXTERNAL_X1_RANDOM:
			val = TYPE_NAME_X1HU_RANDOM;	// asc
			break;
		case EXTERNAL_X1_SWORD:
			val = TYPE_NAME_X1HU_SWORD;		// asc
			break;
		default:
			val = TYPE_NAME_X1HU_ASCII;		// asc
			break;
		}
	} else if (native_type & FILETYPE_X1HU_DIRECTORY) {
		val = TYPE_NAME_X1HU_DIRECTORY;	// sub directory
	}
	return val;
}

#if 0
// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemX1HU::GetFileType1Pos() const
{
	return ConvFileType1Pos(GetFileType1());
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemX1HU::GetFileType2Pos() const
{
	return GetFileType1();
//	int val = 0;
//	val |= (t & DATATYPE_X1HU_HIDDEN ? FILE_TYPE_HIDDEN_MASK : 0);
//	val |= (t & DATATYPE_X1HU_READ_WRITE ? FILE_TYPE_READWRITE_MASK : 0);
//	val |= (t & DATATYPE_X1HU_READ_ONLY ? FILE_TYPE_READONLY_MASK : 0);
//
//	return val;
}
#endif

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemX1HU::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int t1 = (GetFileType1() | (m_external_attr << 16));

	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		t1 = ConvOriginalTypeFromFileName(file_path);
	}

	int file_type_1 = GetFileType1Pos(t1);
	int file_type_2 = (t1 & 0xff);

	wxChoice   *comType1;
	wxCheckBox *chkHidden;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkReadWrite;
	wxCheckBox *chkEncrypt;

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);

	wxArrayString types1;
	for(size_t i=TYPE_NAME_X1HU_BINARY; i<TYPE_NAME_X1HU_END; i++) {
		types1.Add(wxGetTranslation(gTypeNameX1HU_1[i]));
	}

	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	if (file_type_1 >= 0) {
		comType1->SetSelection(file_type_1);
	}
	staType1->Add(comType1, flags);
	sizer->Add(staType1, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxGridSizer *szrG = new wxGridSizer(2, 2, 4);
	chkHidden = new wxCheckBox(parent, IDC_CHECK_HIDDEN, wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_HIDDEN]));
	chkHidden->SetValue((file_type_2 & DATATYPE_X1HU_HIDDEN) != 0);
	szrG->Add(chkHidden);
	chkReadWrite = new wxCheckBox(parent, IDC_CHECK_READWRITE, wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_READ_WRITE]));
	chkReadWrite->SetValue((file_type_2 & DATATYPE_X1HU_READ_WRITE) != 0);
	szrG->Add(chkReadWrite);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_READ_ONLY]));
	chkReadOnly->SetValue((file_type_2 & DATATYPE_X1HU_READ_ONLY) != 0);
	szrG->Add(chkReadOnly);
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
	szrG->Add(chkEncrypt);
	staType4->Add(szrG, flags);
	sizer->Add(staType4, flags);

	// event handler
	parent->Bind(wxEVT_CHOICE, &IntNameBox::OnChangeType1, parent, IDC_COMBO_TYPE1);
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
	wxChoice   *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	wxTextCtrl *txtStartAddr = (wxTextCtrl *)parent->FindWindow(IntNameBox::IDC_TEXT_START_ADDR);
	wxTextCtrl *txtExecAddr = (wxTextCtrl *)parent->FindWindow(IntNameBox::IDC_TEXT_EXEC_ADDR);

	if (!comType1) return;

	bool enable = (comType1->GetSelection() == TYPE_NAME_X1HU_BINARY);
	if (txtStartAddr) {
		txtStartAddr->Enable(enable);
	}
	if (txtExecAddr) {
		txtExecAddr->Enable(enable);
	}
}

/// 属性1を得る
int DiskBasicDirItemX1HU::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	return comType1->GetSelection();
}

/// 属性2を得る
int DiskBasicDirItemX1HU::GetFileType2InAttrDialog(const IntNameBox *parent) const
{
	wxCheckBox *chkHidden = (wxCheckBox *)parent->FindWindow(IDC_CHECK_HIDDEN);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkReadWrite = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READWRITE);

	int val = chkHidden->GetValue() ? DATATYPE_X1HU_HIDDEN : 0;
	val |= chkReadWrite->GetValue() ? DATATYPE_X1HU_READ_WRITE : 0;
	val |= chkReadOnly->GetValue() ? DATATYPE_X1HU_READ_ONLY : 0;
	return val;
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int	DiskBasicDirItemX1HU::CalcFileTypeFromPos(int pos) const
{
	int val = 0;
	int ext = 0;
	switch(pos) {
	case TYPE_NAME_X1HU_BINARY:
		val = FILETYPE_X1HU_BINARY;
		break;
	case TYPE_NAME_X1HU_BASIC:
		val = FILETYPE_X1HU_BASIC;
		break;
	case TYPE_NAME_X1HU_ASCII:
		ext = EXTERNAL_X1_DEFAULT;
		val = FILETYPE_X1HU_ASCII;
		break;
	case TYPE_NAME_X1HU_DIRECTORY:
		val = FILETYPE_X1HU_DIRECTORY;
		break;
	case TYPE_NAME_X1HU_RANDOM:
		ext = EXTERNAL_X1_RANDOM;
		val = FILETYPE_X1HU_ASCII;
		break;
	case TYPE_NAME_X1HU_SWORD:
		ext = EXTERNAL_X1_SWORD;
		val = FILETYPE_X1HU_ASCII;
		break;
	}
	return (ext << 16 | val);
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemX1HU::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int t1 = CalcFileTypeFromPos(GetFileType1InAttrDialog(parent));
	t1 |= GetFileType2InAttrDialog(parent);

	// ユーザ定義データ X1ではファイルパスワード
	wxCheckBox *chkEncrypt = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ENCRYPT);
	int passwd = DATATYPE_X1HU_PASSWORD_NONE;
	if (chkEncrypt->GetValue()) {
//		val |= FILE_TYPE_ENCRYPTED_MASK;
		passwd = parent->GetUserData();
	}

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, passwd << 8 | t1);

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
