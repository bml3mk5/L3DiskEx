/// @file basicdiritem_x1hu.cpp
///
/// @brief disk basic directory item for X1 Hu-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_x1hu.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include <wx/datetime.h>
#include <wx/msgdlg.h>
#include "../config.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// X1 Hu-BASIC
const name_value_t gTypeNameX1HU_1[] = {
	{ "Bin", FILETYPE_X1HU_BINARY },
	{ "Bas", FILETYPE_X1HU_BASIC },
	{ wxTRANSLATE("Asc(Hu)"), FILETYPE_X1HU_ASCII },
	{ wxTRANSLATE("Asc(S-OS)"), FILETYPE_X1HU_ASCII },
	{ wxTRANSLATE("Asc(Random Access)"), FILETYPE_X1HU_ASCII },
	{ "<DIR>", FILETYPE_X1HU_DIRECTORY },
	{ NULL, -1 }
};
const name_value_t gTypeNameX1HU_2[] = {
	{ wxTRANSLATE("Hidden"), DATATYPE_X1HU_HIDDEN },
	{ wxTRANSLATE("Read After Write"), DATATYPE_X1HU_READ_WRITE },
	{ wxTRANSLATE("Write Protected"), DATATYPE_X1HU_READ_ONLY },
	{ wxTRANSLATE("Password"), DATATYPE_X1HU_RESERVED },
	{ NULL, -1 }
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemX1HU::DiskBasicDirItemX1HU(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
	m_external_attr = basic->GetVariousIntegerParam(wxT("DefaultAsciiType"));
}
DiskBasicDirItemX1HU::DiskBasicDirItemX1HU(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	m_external_attr = basic->GetVariousIntegerParam(wxT("DefaultAsciiType"));
}
DiskBasicDirItemX1HU::DiskBasicDirItemX1HU(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);
	m_external_attr = basic->GetVariousIntegerParam(wxT("DefaultAsciiType"));

	Used(CheckUsed(n_unuse));

	// グループ数を計算
	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemX1HU::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemX1HU::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// X1 Hu
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemX1HU::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data.Data()->ext);
	return m_data.Data()->ext;
}

/// 属性１を返す
int	DiskBasicDirItemX1HU::GetFileType1() const
{
	return basic->InvertUint8(m_data.Data()->type);
}

/// 属性２を返す
int	DiskBasicDirItemX1HU::GetFileType2() const
{
	return basic->InvertUint8(m_data.Data()->password);
}

/// 属性１を設定
void DiskBasicDirItemX1HU::SetFileType1(int val)
{
	m_data.Data()->type = basic->InvertUint8(val);
}

/// 属性２を設定
void DiskBasicDirItemX1HU::SetFileType2(int val)
{
	m_data.Data()->password = basic->InvertUint8(val);
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
	if (!m_data.IsValid()) return false;

	int type1 = GetFileType1();
	bool valid = true;
	// 属性が不正
	if (type1 != 0xff && (type1 & 0x08) != 0) {
		valid = false;
	}
	return valid;
}

/// 削除
bool DiskBasicDirItemX1HU::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data.Fill(basic->InvertUint8(basic->GetDeleteCode()), 1);
	Used(false);
	return true;
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
	wxString attr = wxGetTranslation(gTypeNameX1HU_1[GetFileType1Pos(t)].name);

	for(int i=0; i<=TYPE_NAME_X1HU_READ_ONLY; i++) {
		if (t & gTypeNameX1HU_2[i].value) {
			attr += wxT(", ");
			attr += wxGetTranslation(gTypeNameX1HU_2[i].name);
		}
	}
	if (GetFileType2() != DATATYPE_X1HU_PASSWORD_NONE) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameX1HU_2[TYPE_NAME_X1HU_PASSWORD].name);	// password
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
		m_data.Data()->file_size = 0;
	} else {
		m_data.Data()->file_size = basic->InvertAndOrderUint16(val);
	}
}

/// ファイルサイズを返す
int	DiskBasicDirItemX1HU::GetFileSize() const
{
	if (GetFileType1() & FILETYPE_X1HU_ASCII) {
		// Ascファイルの場合
		return (int)m_groups.GetSize();
	} else {
		return basic->InvertAndOrderUint16(m_data.Data()->file_size);
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
void DiskBasicDirItemX1HU::GetFileCreateDate(TM &tm) const
{
	wxUint8 date[sizeof(m_data.Data()->date) + 1];
	basic->InvertMem(m_data.Data()->date, sizeof(m_data.Data()->date), date);
	tm.SetYear(date[0] <= 0x99 ? ((date[0] & 0xf0) >> 4) * 10 + (date[0] & 0x0f) : -1);	// BCD
	tm.SetMonth(((date[1] & 0xf0) >> 4) - 1);
	tm.SetDay(date[2] <= 0x99 ? ((date[2] & 0xf0) >> 4) * 10 + (date[2] & 0x0f) : -1);	// BCD
	if (tm.GetYear() >= 0 && tm.GetYear() < 80) tm.AddYear(100);	// 2000 - 2079
}

/// 時間を得る
void DiskBasicDirItemX1HU::GetFileCreateTime(TM &tm) const
{
	wxUint8 time[sizeof(m_data.Data()->time) + 1];
	basic->InvertMem(m_data.Data()->time, sizeof(m_data.Data()->time), time);
	tm.SetHour(time[0] <= 0x99 ? ((time[0] & 0xf0) >> 4) * 10 + (time[0] & 0x0f) : -1);	// BCD
	tm.SetMinute(time[1] <= 0x99 ? ((time[1] & 0xf0) >> 4) * 10 + (time[1] & 0x0f) : -1);	// BCD
	tm.SetSecond(0);
}

/// 日付を文字列で返す
wxString DiskBasicDirItemX1HU::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 時間を文字列で返す
wxString DiskBasicDirItemX1HU::GetFileCreateTimeStr() const
{
	TM tm;
	GetFileCreateTime(tm);
	return Utils::FormatHMStr(tm);
}

/// 日付を設定
void DiskBasicDirItemX1HU::SetFileCreateDate(const TM &tm)
{
	if (tm.GetYear() < 0 || tm.GetMonth() < -1 || tm.GetDay() < 0) return;

	m_data.Data()->date[0] = (((tm.GetYear() / 10) % 10) << 4) | (tm.GetYear() % 10);	// year BCD
	m_data.Data()->date[1] = (((tm.GetMonth() + 1) & 0xf) << 4);	// month
	m_data.Data()->date[2] = ((tm.GetDay() / 10) << 4) | (tm.GetDay() % 10);	// day BCD

	// 日付から曜日を計算
	int wk = 0;
	wxDateTime dt;
	wxString sdate = wxString::Format(wxT("%04d-%02d-%02d")
		, tm.GetYear() + 1900
		, tm.GetMonth() + 1
		, tm.GetDay()
	);
	dt.ParseISODate(sdate);
	if (dt.IsValid()) {
		wk = (int)dt.GetWeekDay();
	}
	m_data.Data()->date[1] |= (wk & 0xf);	// day of week

	if (basic->IsDataInverted()) mem_invert(m_data.Data()->date, sizeof(m_data.Data()->date));
}

/// 時間を設定
void DiskBasicDirItemX1HU::SetFileCreateTime(const TM &tm)
{
	if (tm.GetHour() < 0 || tm.GetMinute() < 0) return;

	m_data.Data()->time[0] = ((tm.GetHour() / 10) << 4) | (tm.GetHour() % 10);	// hour BCD
	m_data.Data()->time[1] = ((tm.GetMinute() / 10) << 4) | (tm.GetMinute() % 10);	// minute BCD

	if (basic->IsDataInverted()) mem_invert(m_data.Data()->time, sizeof(m_data.Data()->time));
}

/// 開始アドレスを返す
int DiskBasicDirItemX1HU::GetStartAddress() const
{
	return basic->InvertAndOrderUint16(m_data.Data()->load_addr);
}

/// 実行アドレスを返す
int DiskBasicDirItemX1HU::GetExecuteAddress() const
{
	return basic->InvertAndOrderUint16(m_data.Data()->exec_addr);
}

/// 開始アドレスをセット
void DiskBasicDirItemX1HU::SetStartAddress(int val)
{
	m_data.Data()->load_addr = basic->InvertAndOrderUint16(val);
}

/// 実行アドレスをセット
void DiskBasicDirItemX1HU::SetExecuteAddress(int val)
{
	m_data.Data()->exec_addr = basic->InvertAndOrderUint16(val);
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemX1HU::GetDataSize() const
{
	return sizeof(directory_x1_hu_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemX1HU::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemX1HU::CopyData(const directory_t *val)
{
	return m_data.Copy(val, GetDataSize());
}

/// ディレクトリをクリア
void DiskBasicDirItemX1HU::ClearData()
{
	m_data.Fill(basic->GetDeleteCode(), GetDataSize(), basic->IsDataInverted());
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemX1HU::InitialData()
{
	if (!m_data.IsValid()) return;
	m_data.Fill(basic->GetFillCodeOnDir(), GetDataSize(), basic->IsDataInverted());
}

/// 最初のグループ番号を設定
void DiskBasicDirItemX1HU::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// X1 Hu-BASIC
	m_data.Data()->start_group_h = basic->InvertUint8((val & 0xff0000) >> 16);
	m_data.Data()->start_group_l = basic->InvertAndOrderUint16(val & 0xffff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemX1HU::GetStartGroup(int fileunit_num) const
{
	// X1 Hu-BASIC
	return (wxUint32)basic->InvertUint8(m_data.Data()->start_group_h) << 16 | basic->InvertAndOrderUint16(m_data.Data()->start_group_l);
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
#include "../ui/intnamebox.h"

#define IDC_COMBO_TYPE1		51

#define IDC_CHECK_ATTR1		56

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
	wxCheckBox *chkAttr1;
	wxCheckBox *chkEncrypt = NULL;

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);

	wxArrayString types1;
	for(size_t i=TYPE_NAME_X1HU_BINARY; i<TYPE_NAME_X1HU_END; i++) {
		types1.Add(wxGetTranslation(gTypeNameX1HU_1[i].name));
	}

	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	if (file_type_1 >= 0) {
		comType1->SetSelection(file_type_1);
	}
	staType1->Add(comType1, flags);
	sizer->Add(staType1, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxGridSizer *szrG = new wxGridSizer(2, 2, 4);
	for(int i=0; gTypeNameX1HU_2[i].name != NULL; i++) {
		chkAttr1 = new wxCheckBox(parent, IDC_CHECK_ATTR1 + i, wxGetTranslation(gTypeNameX1HU_2[i].name));
		chkAttr1->SetValue((file_type_2 & gTypeNameX1HU_2[i].value) != 0);
		szrG->Add(chkAttr1);
		if (i == TYPE_NAME_X1HU_PASSWORD) {
			chkEncrypt = chkAttr1;
		}
	}

	// ユーザ定義データ X1ではファイルパスワード
	int passwd;
	if (show_flags & INTNAME_NEW_FILE) {
		passwd = DATATYPE_X1HU_PASSWORD_NONE;
	} else {
		passwd = GetFileType2();
	}
	parent->SetUserData(passwd);
	if (chkEncrypt) {
		chkEncrypt->SetValue(passwd != DATATYPE_X1HU_PASSWORD_NONE);
		chkEncrypt->Enable(false);
	}

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
		TM tm;
		GetFileCreateDateTime(tm);
		parent->IgnoreDateTime(gConfig.DoesIgnoreDateTime()	|| tm.Ignorable());
	}
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemX1HU::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxChoice   *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	if (!comType1) return;

	bool enable = (comType1->GetSelection() == TYPE_NAME_X1HU_BINARY);
	parent->EnableStartAddress(enable);
	parent->EnableExecuteAddress(enable);
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
	int val = 0;

	for(int i=0; i<=TYPE_NAME_X1HU_READ_ONLY; i++) {
		wxCheckBox *chkAttr1 = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ATTR1 + i);
		if (chkAttr1->GetValue()) {
			val |= gTypeNameX1HU_2[i].value;
		}
	}
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
	wxCheckBox *chkEncrypt = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ATTR1 + TYPE_NAME_X1HU_PASSWORD);
	int passwd = DATATYPE_X1HU_PASSWORD_NONE;
	if (chkEncrypt->GetValue()) {
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

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemX1HU::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("inverted"), basic->IsDataInverted());

	vals.Add(wxT("TYPE"), m_data.Data()->type, basic->IsDataInverted());
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name), basic->IsDataInverted());
	vals.Add(wxT("EXT"), m_data.Data()->ext, sizeof(m_data.Data()->ext), basic->IsDataInverted());
	vals.Add(wxT("PASSWORD"), m_data.Data()->password, basic->IsDataInverted());
	vals.Add(wxT("FILE_SIZE"), m_data.Data()->file_size, basic->IsBigEndian(), basic->IsDataInverted());
	vals.Add(wxT("LOAD_ADDR"), m_data.Data()->load_addr, basic->IsBigEndian(), basic->IsDataInverted());
	vals.Add(wxT("EXEC_ADDR"), m_data.Data()->exec_addr, basic->IsBigEndian(), basic->IsDataInverted());
	vals.Add(wxT("DATE"), m_data.Data()->date, sizeof(m_data.Data()->date), basic->IsDataInverted());
	vals.Add(wxT("TIME"), m_data.Data()->time, sizeof(m_data.Data()->time), basic->IsDataInverted());
	vals.Add(wxT("START_GROUP_H"), m_data.Data()->start_group_h, basic->IsDataInverted());
	vals.Add(wxT("START_GROUP_L"), m_data.Data()->start_group_l, basic->IsBigEndian(), basic->IsDataInverted());
}
