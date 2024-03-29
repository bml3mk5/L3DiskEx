/// @file basicdiritem_magical.cpp
///
/// @brief disk basic directory item for Magical DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_magical.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../config.h"
#include "../charcodes.h"
#include <wx/msgdlg.h>


//////////////////////////////////////////////////////////////////////
//
//
//

/// Magical DOS 属性名
const name_value_t gTypeNameMAGICAL_1[] = {
	{ "SYS", FILETYPE_MAGICAL_SYS }, // 0x01
	{ "BAS", FILETYPE_MAGICAL_BAS }, // 0x22
	{ "OBJ", FILETYPE_MAGICAL_OBJ }, // 0x03
	{ "ASC", FILETYPE_MAGICAL_ASC }, // 0x44
	{ "DIR", FILETYPE_MAGICAL_DIR }, // 0x05
	{ "CDT", FILETYPE_MAGICAL_CDT }, // 0x06
	{ "PDT", FILETYPE_MAGICAL_PDT }, // 0x07
	{ "GRA", FILETYPE_MAGICAL_GRA }, // 0x08
	{ "GAK", FILETYPE_MAGICAL_GAK }, // 0x49
	{ "SBA", FILETYPE_MAGICAL_SBA }, // 0x2a
	{ "SOB", FILETYPE_MAGICAL_SOB }, // 0x0b
	{ "REP", FILETYPE_MAGICAL_REP }, // 0x4c
	{ "MDT", FILETYPE_MAGICAL_MDT }, // 0x0d
	{ "ARC", FILETYPE_MAGICAL_ARC }, // 0x4e
	{ "KTY", FILETYPE_MAGICAL_KTY }, // 0x4f
	{ "CGP", FILETYPE_MAGICAL_CGP }, // 0x50
	{ "BGM", FILETYPE_MAGICAL_BGM }, // 0x51
	{ "???", FILETYPE_MAGICAL_UNKNOWN }, // 0x80
	{ NULL, -1 }
};

const wxUint8 gTypeNameMAGICALMap[] = {
	FILETYPE_MAGICAL_SYS,
	FILETYPE_MAGICAL_BAS,
	FILETYPE_MAGICAL_OBJ,
	FILETYPE_MAGICAL_ASC,
	FILETYPE_MAGICAL_DIR,
	FILETYPE_MAGICAL_CDT,
	FILETYPE_MAGICAL_PDT,
	FILETYPE_MAGICAL_GRA,
	FILETYPE_MAGICAL_GAK,
	FILETYPE_MAGICAL_SBA,
	FILETYPE_MAGICAL_SOB,
	FILETYPE_MAGICAL_REP,
	FILETYPE_MAGICAL_MDT,
	FILETYPE_MAGICAL_ARC,
	FILETYPE_MAGICAL_KTY,
	FILETYPE_MAGICAL_CGP,
	FILETYPE_MAGICAL_BGM,
	FILETYPE_MAGICAL_UNKNOWN,
	0
};

const char *gTypeNameMAGICAL_2[] = {
	wxTRANSLATE("Write Protected"),
	wxTRANSLATE("Hidden"),
	wxTRANSLATE("System"),
	wxTRANSLATE("Super User"),
	NULL
};

const char gTypeNameMAGICAL_3s[] = "brgmABCDEFGHIJKL";

const char *gTypeNameMAGICAL_3[] = {
	"GRAM blue",
	"GRAM red",
	"GRAM green",
	"main",
	"ERAM A",
	"ERAM B",
	"ERAM C",
	"ERAM D",
	wxTRANSLATE("Unknown"),
	NULL
};
enum en_type_name_magical_3 {
	TYPE_NAME_MAGICAL_BANK_b,
	TYPE_NAME_MAGICAL_BANK_r,
	TYPE_NAME_MAGICAL_BANK_g,
	TYPE_NAME_MAGICAL_BANK_m,
	TYPE_NAME_MAGICAL_BANK_A,
	TYPE_NAME_MAGICAL_BANK_B,
	TYPE_NAME_MAGICAL_BANK_C,
	TYPE_NAME_MAGICAL_BANK_D,
	TYPE_NAME_MAGICAL_BANK_Unknown
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemMAGICAL::DiskBasicDirItemMAGICAL(DiskBasic *basic)
	: DiskBasicDirItemXDOSBase(basic)
{
	m_data.Alloc();
	AllocateItem(NULL);
}
DiskBasicDirItemMAGICAL::DiskBasicDirItemMAGICAL(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemXDOSBase(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	AllocateItem(NULL);
}
DiskBasicDirItemMAGICAL::DiskBasicDirItemMAGICAL(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemXDOSBase(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse, true)
{
	m_data.Attach(n_data);
	AllocateItem(n_next);

	Used(CheckUsed(n_unuse));

	// チェインセクタへのポインタをセット
	if (IsUsed()) {
		AttachChain(GetStartGroup(0));
	}

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
void DiskBasicDirItemMAGICAL::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemXDOSBase::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);
	m_data.Attach(n_data);
	AllocateItem(n_next);
}

/// ディレクトリエントリを確保
/// data  は内部で確保したメモリ
/// sdata がセクタ内部へのポインタとなる
bool DiskBasicDirItemMAGICAL::AllocateItem(const SectorParam *next)
{
	m_sdata.Clear();
	bool bound = m_sdata.Set(basic, m_sector, m_position, (directory_t *)m_data.Data(), GetDataSize(), next);
	
	if (!m_data.IsSelf() && bound) {
		// セクタをまたぐ場合、dataは内部で確保する
		m_data.Alloc();
		m_data.Fill(0);
	}

	// コピー
	if (m_data.IsSelf()) {
		m_sdata.CopyTo((directory_t *)m_data.Data());
	}

	return true;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemMAGICAL::GetFileNamePos(int num, size_t &size, size_t &len) const
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
int	DiskBasicDirItemMAGICAL::GetFileType1() const
{
	return m_data.Data()->type;
}

/// 属性１の文字列
wxString DiskBasicDirItemMAGICAL::ConvFileType1Str(int t1) const
{
	return wxGetTranslation(gTypeNameMAGICAL_1[ConvFileType1Pos(t1)].name);
}

/// 属性１を設定
void DiskBasicDirItemMAGICAL::SetFileType1(int val)
{
	m_data.Data()->type = val & 0xff;
}

/// 属性２を返す
int DiskBasicDirItemMAGICAL::GetFileType2() const
{
	return m_data.Data()->type2;
}

/// 属性２を設定
void DiskBasicDirItemMAGICAL::SetFileType2(int val)
{
	m_data.Data()->type2 = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemMAGICAL::CheckUsed(bool unuse)
{
	int t1 = GetFileType1();
	return (!unuse && t1 != 0 && t1 != 0xff);
}

/// ファイル名を設定
void DiskBasicDirItemMAGICAL::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	DiskBasicDirItem::SetNativeName(filename, size, length);
}

/// ファイル名と拡張子を得る
void DiskBasicDirItemMAGICAL::GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	DiskBasicDirItem::GetNativeFileName(name, nlen, ext, elen);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemMAGICAL::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;
	if (m_data.Data()->type == 0xff) {
		last = true;
		return valid;
	}
	return valid;
}

/// アイテムを削除できるか
bool DiskBasicDirItemMAGICAL::IsDeletable() const
{
	return true;
}

/// 削除
/// @return true:OK
bool DiskBasicDirItemMAGICAL::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data.Fill(basic->InvertUint8(basic->GetDeleteCode()), 1);
	Used(false);
	return true;
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemMAGICAL::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
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

/// 属性を設定
/// @param [in] file_type
void DiskBasicDirItemMAGICAL::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	int t2 = DATATYPE_MAGICAL_MASK_m;
	if (file_type.IsDirectory()) {
		// ディレクトリの場合
		t1 = FILETYPE_MAGICAL_DIR;
	} else if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		// 同じOSの場合
		t1 = file_type.GetOrigin();
		t2 = t1 >> 8;
		t1 &= 0xff;
	} else {
		// 違うOSの場合
		if (ftype & FILE_TYPE_BINARY_MASK) {
			if (ftype & FILE_TYPE_SYSTEM_MASK) {
				t1 = FILETYPE_MAGICAL_SYS;
			} else if (ftype & FILE_TYPE_BASIC_MASK) {
				t1 = FILETYPE_MAGICAL_BAS;
			} else {
				t1 = FILETYPE_MAGICAL_OBJ;
			}
		} else {
			t1 = FILETYPE_MAGICAL_ASC;
		}
	}
	SetFileType1(t1);
	SetFileType2(t2);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemMAGICAL::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	switch (t1) {
	case FILETYPE_MAGICAL_SYS:
		val = FILE_TYPE_SYSTEM_MASK;		// system
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	case FILETYPE_MAGICAL_OBJ:
		val = FILE_TYPE_MACHINE_MASK;		// machine
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	case FILETYPE_MAGICAL_BAS:
		val = FILE_TYPE_BASIC_MASK;			// basic
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	case FILETYPE_MAGICAL_ASC:
		val = FILE_TYPE_DATA_MASK;			// data
		val |= FILE_TYPE_ASCII_MASK;		// ascii
		break;
	case FILETYPE_MAGICAL_DIR:
		val = FILE_TYPE_DIRECTORY_MASK;		// directory
		break;
	default:
		val = FILE_TYPE_DATA_MASK;			// data
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	}

	int t2 = GetFileType2();
	if (t2 & DATATYPE_MAGICAL_MASK_READONLY) {
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (t2 & DATATYPE_MAGICAL_MASK_HIDDEN) {
		val |= FILE_TYPE_HIDDEN_MASK;
	}
	if (t2 & DATATYPE_MAGICAL_MASK_SYSTEM) {
		val |= FILE_TYPE_SYSTEM_MASK;
	}
	if (t2 & DATATYPE_MAGICAL_MASK_SUPER) {
		val |= FILE_TYPE_SYSTEM_MASK;
	}

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t2 << 8 | t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMAGICAL::GetFileAttrStr() const
{
	wxString attr = ConvFileType1Str(GetFileType1());
	//
	int t2 = GetFileType2();

	attr += wxT(", ");
	attr += gTypeNameMAGICAL_3s[t2 & 0xf];

	if (t2 & DATATYPE_MAGICAL_MASK_READONLY) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMAGICAL_2[TYPE_NAME_MAGICAL_READONLY]);
	}
	if (t2 & DATATYPE_MAGICAL_MASK_HIDDEN) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMAGICAL_2[TYPE_NAME_MAGICAL_HIDDEN]);
	}
	if (t2 & DATATYPE_MAGICAL_MASK_SYSTEM) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMAGICAL_2[TYPE_NAME_MAGICAL_SYSTEM]);
	}
	if (t2 & DATATYPE_MAGICAL_MASK_SUPER) {
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMAGICAL_2[TYPE_NAME_MAGICAL_SUPER]);
	}
	return attr;
}

/// ファイルサイズをセット
void DiskBasicDirItemMAGICAL::SetFileSize(int val)
{
	wxUint16 size = basic->OrderUint16(val);

	m_data.Data()->file_size = size;
}

/// ファイルサイズを返す
int DiskBasicDirItemMAGICAL::GetFileSize() const
{
	wxUint16 size;
	size = m_data.Data()->file_size;
	return basic->OrderUint16(size);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemMAGICAL::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	wxUint8 track = (val /  basic->GetSectorsPerTrackOnBasic()) & 0xff;
	wxUint8 sector = ((val %  basic->GetSectorsPerTrackOnBasic()) + 1) & 0xff;
	m_data.Data()->start.track = track;
	m_data.Data()->start.sector = sector;
	m_data.Data()->start.size = (wxUint8)size;
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemMAGICAL::GetStartGroup(int fileunit_num) const
{
	wxUint32 track = 0;
	wxUint32 sector = 0;
	track = m_data.Data()->start.track;
	sector = m_data.Data()->start.sector;
	return track * basic->GetSectorsPerTrackOnBasic() + sector - 1;
}

/// 追加のグループ番号をセット(機種依存)
void DiskBasicDirItemMAGICAL::SetExtraGroup(wxUint32 val)
{
	wxUint8 track = (val /  basic->GetSectorsPerTrackOnBasic()) & 0xff;
	wxUint8 sector = ((val %  basic->GetSectorsPerTrackOnBasic()) + 1) & 0xff;
	m_data.Data()->start.track = track;
	m_data.Data()->start.sector = sector;
	m_data.Data()->start.size = 1;
}

/// 追加のグループ番号を返す(機種依存)
wxUint32 DiskBasicDirItemMAGICAL::GetExtraGroup() const
{
	wxUint32 track = 0;
	wxUint32 sector = 0;
	track = m_data.Data()->start.track;
	sector = m_data.Data()->start.sector;
	return track * basic->GetSectorsPerTrackOnBasic() + sector - 1;
}

/// 追加のグループ番号を得る(機種依存)
void DiskBasicDirItemMAGICAL::GetExtraGroups(wxArrayInt &arr) const
{
	arr.Add((int)GetExtraGroup());
}

/// 日付を得る
void DiskBasicDirItemMAGICAL::GetFileCreateDate(TM &tm) const
{
	wxUint8 d[2];
	d[0] = m_data.Data()->date[0];
	d[1] = m_data.Data()->date[1];

	wxUint16 date = (d[0] << 8) | d[1];
	ConvDateToTm(date, tm);
}

/// 時間を得る
void DiskBasicDirItemMAGICAL::GetFileCreateTime(TM &tm) const
{
	wxUint8 t[2];
	t[0] = m_data.Data()->time[0];
	t[1] = m_data.Data()->time[1];

	wxUint16 time = (t[0] << 8) | t[1];
	ConvTimeToTm(time, tm);
}

void DiskBasicDirItemMAGICAL::SetFileCreateDate(const TM &tm)
{
	if (tm.GetYear() >= 0 && tm.GetMonth() >= -1) {
		wxUint16 date = ConvTmToDate(tm);
		m_data.Data()->date[0] = (date >> 8);
		m_data.Data()->date[1] = (date & 0xff);
	}
}

void DiskBasicDirItemMAGICAL::SetFileCreateTime(const TM &tm)
{
	if (tm.GetHour() >= 0 && tm.GetMinute() >= 0) {
		wxUint16 time = ConvTmToTime(tm);
		m_data.Data()->time[0] = (time >> 8);
		m_data.Data()->time[1] = (time & 0xff);
	}
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItemMAGICAL::GetFileCreateDateTimeTitle() const
{
	return _("Created Date");
}

/// 日付を変換
void DiskBasicDirItemMAGICAL::ConvDateToTm(wxUint16 date, TM &tm)
{
	int yy = ((date & 0xfe00) >> 9) + 80;
	if (yy >= 128) yy -= 28; 
	tm.SetYear(yy);
	tm.SetMonth(((date & 0x01e0) >> 5) - 1);
	tm.SetDay(date & 0x001f);
}

/// 時間を変換
void DiskBasicDirItemMAGICAL::ConvTimeToTm(wxUint16 time, TM &tm)
{
	tm.SetHour((time & 0xf800) >> 11);
	tm.SetMinute((time & 0x07e0) >> 5);
	tm.SetSecond((time & 0x001f) << 1);
}

/// 日付に変換
wxUint16 DiskBasicDirItemMAGICAL::ConvTmToDate(const TM &tm)
{
	int yy = tm.GetYear();
	if (yy >= 100) yy += 28;
	return (wxUint16)
		(((yy - 80) & 0x7f) << 9)
		| (((tm.GetMonth() + 1) & 0xf) << 5)
		| (tm.GetDay() & 0x1f);
}

/// 時間に変換
wxUint16 DiskBasicDirItemMAGICAL::ConvTmToTime(const TM &tm)
{
	return (wxUint16)
		((tm.GetHour() & 0x1f) << 11)
		| ((tm.GetMinute() & 0x3f) << 5)
		| ((tm.GetSecond() & 0x3f) >> 1);
}

/// 開始アドレスを返す
int DiskBasicDirItemMAGICAL::GetStartAddress() const
{
	int addr;
	addr = m_data.Data()->load_addr;

	return basic->OrderUint16(addr);
}

// 実行アドレスを返す
int DiskBasicDirItemMAGICAL::GetExecuteAddress() const
{
	int addr;
	addr = m_data.Data()->exec_addr;

	return basic->OrderUint16(addr);
}

/// 開始アドレスをセット
void DiskBasicDirItemMAGICAL::SetStartAddress(int val)
{
	m_data.Data()->load_addr = basic->OrderUint16(val);
}

/// 実行アドレスをセット
void DiskBasicDirItemMAGICAL::SetExecuteAddress(int val)
{
	m_data.Data()->exec_addr = basic->OrderUint16(val);
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemMAGICAL::GetDataSize() const
{
	return sizeof(directory_magical_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemMAGICAL::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemMAGICAL::CopyData(const directory_t *val)
{
	bool sts = m_data.Copy(val);
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
	return sts;
}

/// ディレクトリをクリア
void DiskBasicDirItemMAGICAL::ClearData()
{
	m_data.Fill(basic->GetDeleteCode());
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemMAGICAL::InitialData()
{
	m_data.Fill(basic->GetFillCodeOnDir());
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
}

/// データをエクスポートする前に必要な処理
/// 拡張子を付加する
bool DiskBasicDirItemMAGICAL::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	// 拡張子を付加する
	if (!IsDirectory()) {
		wxString ext = ConvFileType1Str(GetFileType1());
		filename += wxT(".");
		if (Utils::IsUpperString(filename)) {
			ext.MakeUpper();
		} else {
			ext.MakeLower();
		}
		filename += ext;
	}
	return true;
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
bool DiskBasicDirItemMAGICAL::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		IsContainAttrByExtension(filename, gTypeNameMAGICAL_1, TYPE_NAME_MAGICAL_SYS, TYPE_NAME_MAGICAL_BGM, &filename, NULL, NULL);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemMAGICAL::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;
	// 拡張子で属性を設定する
	if (!IsContainAttrByExtension(filename, gTypeNameMAGICAL_1, TYPE_NAME_MAGICAL_SYS, TYPE_NAME_MAGICAL_BGM, NULL, &t1, NULL)) {
		// 不明の拡張子
		t1 = FILETYPE_MAGICAL_ASC;
	}
//	if (TYPE_NAME_MAGICAL_SYS <= t1 && t1 <= TYPE_NAME_MAGICAL_BGM) {
//		t1 = gTypeNameMAGICALMap[t1];
//	} else {
//		// 不明の拡張子
//		t1 = FILETYPE_MAGICAL_ASC;
//	}
	t1 |= (DATATYPE_MAGICAL_MASK_m << 8);
	return t1;
}

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemMAGICAL::SetModify()
{
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
}

//
// ダイアログ用
//

#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_COMBO_TYPE1		51
#define IDC_COMBO_MEMBANK	52
#define IDC_CHECK_READONLY	53
#define IDC_CHECK_HIDDEN	54
#define IDC_CHECK_SYSTEM	55
#define IDC_CHECK_SUPER		56

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemMAGICAL::ConvFileType1Pos(int t1) const
{
	int pos = TYPE_NAME_MAGICAL_UNKNOWN;
	for(int i = 0; ; i++) {
		int val = gTypeNameMAGICALMap[i];
		if (val == 0) {
			break;
		}
		if (t1 == val) {
			pos = i;
			break;
		}
	}
	return pos;
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemMAGICAL::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int t1 = GetFileType1();
	int t2 = GetFileType2();

	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		t1 = ConvOriginalTypeFromFileName(file_path);
		t2 = t1 >> 8;
		t1 &= 0xff;
	}

	int file_type_1 = ConvFileType1Pos(t1);
	int file_type_2 = t2;
	wxChoice *comType1;
	wxChoice *comMemBank;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkHidden;
	wxCheckBox *chkSystem;
	wxCheckBox *chkSuper;

	wxGridSizer *gszr = new wxGridSizer(2, 1, 1);

	wxArrayString types1;
	for(size_t i=0; gTypeNameMAGICAL_1[i].name != NULL; i++) {
		types1.Add(wxGetTranslation(gTypeNameMAGICAL_1[i].name));
	}
	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);
	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	if (file_type_1 >= 0) {
		comType1->SetSelection(file_type_1);
	}
	staType1->Add(comType1, flags);
	gszr->Add(staType1, flags);

	wxArrayString types2;
	for(size_t i=0; gTypeNameMAGICAL_3[i] != NULL; i++) {
		types2.Add(wxGetTranslation(gTypeNameMAGICAL_3[i]));
	}
	wxStaticBoxSizer *staType2 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("Memory Bank")), wxVERTICAL);
	comMemBank = new wxChoice(parent, IDC_COMBO_MEMBANK, wxDefaultPosition, wxDefaultSize, types2);
	int mem_bank = (file_type_2 & 0xf);
	if (mem_bank >= 8) mem_bank = TYPE_NAME_MAGICAL_BANK_Unknown;
	comMemBank->SetSelection(mem_bank);
	staType2->Add(comMemBank, flags);
	gszr->Add(staType2, flags);

	sizer->Add(gszr, wxSizerFlags().Expand());

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);

	wxBoxSizer *hszr = new wxBoxSizer(wxHORIZONTAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameMAGICAL_2[TYPE_NAME_MAGICAL_READONLY]));
	chkReadOnly->SetValue((file_type_2 & DATATYPE_MAGICAL_MASK_READONLY) != 0);
	hszr->Add(chkReadOnly, flags);
	chkHidden = new wxCheckBox(parent, IDC_CHECK_HIDDEN, wxGetTranslation(gTypeNameMAGICAL_2[TYPE_NAME_MAGICAL_HIDDEN]));
	chkHidden->SetValue((file_type_2 & DATATYPE_MAGICAL_MASK_HIDDEN) != 0);
	hszr->Add(chkHidden, flags);
	staType4->Add(hszr);

	hszr = new wxBoxSizer(wxHORIZONTAL);
	chkSystem = new wxCheckBox(parent, IDC_CHECK_SYSTEM, wxGetTranslation(gTypeNameMAGICAL_2[TYPE_NAME_MAGICAL_SYSTEM]));
	chkSystem->SetValue((file_type_2 & DATATYPE_MAGICAL_MASK_SYSTEM) != 0);
	hszr->Add(chkSystem, flags);
	chkSuper = new wxCheckBox(parent, IDC_CHECK_SUPER, wxGetTranslation(gTypeNameMAGICAL_2[TYPE_NAME_MAGICAL_SUPER]));
	chkSuper->SetValue((file_type_2 & DATATYPE_MAGICAL_MASK_SUPER) != 0);
	hszr->Add(chkSuper, flags);
	staType4->Add(hszr);

	sizer->Add(staType4, flags);

	parent->SetUserData(file_type_2);
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemMAGICAL::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	wxChoice *comMemBank = (wxChoice *)parent->FindWindow(IDC_COMBO_MEMBANK);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkHidden = (wxCheckBox *)parent->FindWindow(IDC_CHECK_HIDDEN);
	wxCheckBox *chkSystem = (wxCheckBox *)parent->FindWindow(IDC_CHECK_SYSTEM);
	wxCheckBox *chkSuper = (wxCheckBox *)parent->FindWindow(IDC_CHECK_SUPER);

	int t1 = comType1->GetSelection();
	if (t1 >= TYPE_NAME_MAGICAL_SYS && t1 < TYPE_NAME_MAGICAL_UNKNOWN) {
		t1 = gTypeNameMAGICALMap[t1];
	} else {
		t1 = FILETYPE_MAGICAL_UNKNOWN;
	}
	int t2 = comMemBank->GetSelection();
	if (t2 == TYPE_NAME_MAGICAL_BANK_Unknown) {
		t2 = parent->GetUserData();
	}
	t2 &= 0x0f;
	t2 |= chkReadOnly->GetValue() ? DATATYPE_MAGICAL_MASK_READONLY : 0;
	t2 |= chkHidden->GetValue() ? DATATYPE_MAGICAL_MASK_HIDDEN : 0;
	t2 |= chkSystem->GetValue() ? DATATYPE_MAGICAL_MASK_SYSTEM : 0;
	t2 |= chkSuper->GetValue() ? DATATYPE_MAGICAL_MASK_SUPER : 0;

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t2 << 8 | t1);

	return true;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemMAGICAL::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("TYPE"), m_data.Data()->type);
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("TYPE2"), m_data.Data()->type2);
	vals.Add(wxT("LOAD_ADDR"), m_data.Data()->load_addr, basic->IsBigEndian());
	vals.Add(wxT("FILE_SIZE"), m_data.Data()->file_size, basic->IsBigEndian());
	vals.Add(wxT("EXEC_ADDR"), m_data.Data()->exec_addr, basic->IsBigEndian());
	vals.Add(wxT("DATE"), m_data.Data()->date, sizeof(m_data.Data()->date));
	vals.Add(wxT("TIME"), m_data.Data()->time, sizeof(m_data.Data()->time));
	vals.Add(wxT("RESERVED"), m_data.Data()->reserved, sizeof(m_data.Data()->reserved));
	vals.Add(wxT("START.TRACK"), m_data.Data()->start.track);
	vals.Add(wxT("START.SECTOR"), m_data.Data()->start.sector);
	vals.Add(wxT("START.SIZE"), m_data.Data()->start.size);
}
