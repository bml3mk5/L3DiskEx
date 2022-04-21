/// @file basicdiritem_mz_fdos.cpp
///
/// @brief disk basic directory item for MZ Floppy DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_mz_fdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "config.h"
#include "utils.h"


//
//
//

/// MZ FDOS属性名
const char *gTypeNameMZFDOS[] = {
	"???",
	"OBJ",
	"BTX",
	"DAT",
	"ASC",
	"RB",
	"FTN",
	"LIB",
	"PAS",
	"TEM",
	"SYS",
	"GR",
	"GRH",
	NULL
};

static const int gTypeNamePosMZFDOS[] = {
	0,
	FILETYPE_MZ_FDOS_OBJ,
	FILETYPE_MZ_FDOS_BTX,
	FILETYPE_MZ_FDOS_DAT,
	FILETYPE_MZ_FDOS_ASC,
	FILETYPE_MZ_FDOS_RB,
	FILETYPE_MZ_FDOS_FTN,
	FILETYPE_MZ_FDOS_LIB,
	FILETYPE_MZ_FDOS_PAS,
	FILETYPE_MZ_FDOS_TEM,
	FILETYPE_MZ_FDOS_SYS,
	FILETYPE_MZ_FDOS_GR,
	FILETYPE_MZ_FDOS_GRH,
	-1
};

#define MZ_FDOS_NO_PROTECT	0x3053	// "0S"

//
//
//
DiskBasicDirItemMZFDOSChain::DiskBasicDirItemMZFDOSChain()
{
	basic = NULL;
	secs_per_track = 1;
	sector = NULL;
	chain = NULL;
	chain_ownmake = false;
	map_size = 0;
}
DiskBasicDirItemMZFDOSChain::~DiskBasicDirItemMZFDOSChain()
{
	if (chain_ownmake) {
		delete chain;
	}
}
#ifdef COPYABLE_DIRITEM
/// 代入
DiskBasicDirItemMZFDOSChain &DiskBasicDirItemMZFDOSChain::operator=(const DiskBasicDirItemMZFDOSChain &src)
{
	this->Dup(src);
	return *this;
}
/// 複製
void DiskBasicDirItemMZFDOSChain::Dup(const DiskBasicDirItemMZFDOSChain &src)
{
	secs_per_track = src.secs_per_track;
	sector = src.sector;
	if (src.chain_ownmake) {
		chain = new mz_fdos_chain_t;
		memcpy(&chain, src.chain, sizeof(mz_fdos_chain_t));
	} else {
		chain = src.chain;
	}
	chain_ownmake = src.chain_ownmake;
	map_size = src.map_size;
}
#endif
/// ポインタをセット
void DiskBasicDirItemMZFDOSChain::Set(DiskBasic *n_basic, DiskD88Sector *n_sector, mz_fdos_chain_t *n_chain)
{
	basic = n_basic;
	sector = n_sector;
	if (chain_ownmake) delete chain;
	chain = n_chain;
	chain_ownmake = false;
}
/// メモリ確保
void DiskBasicDirItemMZFDOSChain::Alloc()
{
	if (chain_ownmake) delete chain;
	chain = new mz_fdos_chain_t;
	chain_ownmake = true;
	memset(chain, 0, sizeof(mz_fdos_chain_t));
}
/// クリア
void DiskBasicDirItemMZFDOSChain::Clear()
{
	if (sector) sector->Fill(0);
	else if (chain) memset(chain, 0, sizeof(mz_fdos_chain_t));
}
/// セクタ位置の使用状態を返す
bool DiskBasicDirItemMZFDOSChain::IsUsedSector(int sector_pos) const
{
	if (!chain || !basic || sector_pos >= (int)map_size) return true;

	int mask = 1 << (sector_pos & 7);
	int idx = (sector_pos >> 3);

	wxUint8 bits = basic->InvertUint8(chain->map[idx]);
	return (bits & mask) != 0;
}
/// セクタ数を返す
wxUint16 DiskBasicDirItemMZFDOSChain::GetSectors() const
{
	return chain ? (basic ? basic->InvertAndOrderUint16(chain->sectors) : chain->sectors) : 0;
}
/// セクタ位置の使用状態を設定
void DiskBasicDirItemMZFDOSChain::UsedSector(int sector_pos, bool val)
{
	if (!chain || !basic || sector_pos >= (int)map_size) return;

	int mask = 1 << (sector_pos & 7);
	int idx = (sector_pos >> 3);

	wxUint8 bits = basic->InvertUint8(chain->map[idx]);
	bits = (val ? bits | mask : bits & ~mask);
	chain->map[idx] = basic->InvertUint8(bits);
}
/// セクタ数を設定
void DiskBasicDirItemMZFDOSChain::SetSectors(wxUint16 val)
{
	if (chain) {
		chain->sectors = basic ? basic->InvertAndOrderUint16(val) : val;
	}
}

//
//
//
DiskBasicDirItemMZFDOS::DiskBasicDirItemMZFDOS(DiskBasic *basic)
	: DiskBasicDirItemMZBase(basic)
{
	chain.SetSectorsPerTrack(basic->GetSectorsPerTrackOnBasic());
	chain.SetMapSize(basic->GetFatEndGroup());
	chain.Alloc();

//	if (data) {
//		mem_invert(data, sizeof(directory_mz_fdos_t));	// invert
//	}
}
DiskBasicDirItemMZFDOS::DiskBasicDirItemMZFDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItemMZBase(basic, sector, secpos, data)
{
	chain.SetSectorsPerTrack(basic->GetSectorsPerTrackOnBasic());
	chain.SetMapSize(basic->GetFatEndGroup());
}
DiskBasicDirItemMZFDOS::DiskBasicDirItemMZFDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemMZBase(basic, num, track, side, sector, secpos, data, unuse)
{
	chain.SetSectorsPerTrack(basic->GetSectorsPerTrackOnBasic());
	chain.SetMapSize(basic->GetFatEndGroup());

	Used(CheckUsed(unuse));

	// チェインセクタへのポインタをセット
	if (IsUsed()) {
		wxUint32 grp = GetStartGroup(0);
		if (grp != 0) {
			DiskD88Sector *sector = basic->GetSectorFromGroup(grp);
			if (sector) {
				chain.Set(basic, sector, (mz_fdos_chain_t *)sector->GetSectorBuffer());
			}
		}
	}

	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemMZFDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = sizeof(m_data->fdos.name);
		len = size - 1;
		return m_data->fdos.name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 属性１を返す
int	DiskBasicDirItemMZFDOS::GetFileType1() const
{
	return basic->InvertUint8(m_data->fdos.type);	// invert;
}

/// 属性２を返す
int	DiskBasicDirItemMZFDOS::GetFileType2() const
{
	int attr = basic->InvertUint8(m_data->fdos.attr[0]);	// invert;
	attr <<= 8;
	attr |= basic->InvertUint8(m_data->fdos.attr[1]);	// invert;
	return attr;
}

/// 属性１を設定
void DiskBasicDirItemMZFDOS::SetFileType1(int val)
{
	m_data->fdos.type = basic->InvertUint8(val);	// invert
}

/// 属性２を設定
void DiskBasicDirItemMZFDOS::SetFileType2(int val)
{
	m_data->fdos.attr[0] = basic->InvertUint8((val >> 8) & 0xff);	// invert
	m_data->fdos.attr[1] = basic->InvertUint8(val & 0xff);	// invert
}

/// 使用しているアイテムか
bool DiskBasicDirItemMZFDOS::CheckUsed(bool unuse)
{
	int typ1 = GetFileType1();
	return (typ1 != 0 && typ1 != 0xfe);
}

/// 削除
/// @param [in] code : 削除コード(デフォルト0)
bool DiskBasicDirItemMZFDOS::Delete(wxUint8 code)
{
	// エントリの先頭にコードを入れる
	SetFileType1(code);
	Used(false);
	// 開始グループを未使用にする
	type->SetGroupNumber(GetStartGroup(0), 0);
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemMZFDOS::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;
	wxUint8 t = GetFileType1();
	if (t != 0xfe && (t & 0x60) != 0) {
		valid = false;
	}
	if (valid && !last) {
		valid = DiskBasicDirItem::Check(last);
	}
	return valid;
}

/// 属性を設定
void DiskBasicDirItemMZFDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	int t2 = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		t1 = (file_type.GetOrigin() & 0xff);
		t2 = (file_type.GetOrigin() >> 8);
	} else {
		t1 = ConvToNativeType(ftype);
		t2 = MZ_FDOS_NO_PROTECT;
	}
	SetFileType1(t1);
	SetFileType2(t2);
}

/// 属性を変換
int DiskBasicDirItemMZFDOS::ConvToNativeType(int file_type) const
{
	// MZ
	int val = 0;
	if (file_type & FILE_TYPE_BINARY_MASK) {
		if (file_type & FILE_TYPE_MACHINE_MASK) {
			val = FILETYPE_MZ_FDOS_OBJ;
		} else {
			val = FILETYPE_MZ_FDOS_RB;
		}
	} else if (file_type & FILE_TYPE_SYSTEM_MASK) {
		val = FILETYPE_MZ_FDOS_SYS;
	} else if (file_type & FILE_TYPE_LIBRARY_MASK) {
		val = FILETYPE_MZ_FDOS_LIB;
	} else {
		val = FILETYPE_MZ_FDOS_ASC;
	}
	return val;
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemMZFDOS::ClearData()
{
	if (!m_data) return;
	wxUint8 sp = basic->GetDirSpaceCode();
	memset(m_data, 0, sizeof(directory_mz_fdos_t));
	// 名前は初期値
	memset(m_data->fdos.name, sp, sizeof(m_data->fdos.name));
	// 日付は初期値
	memset(m_data->fdos.mmddyy, '?', sizeof(m_data->fdos.mmddyy));
	m_data->fdos.mmddyy[sizeof(m_data->fdos.mmddyy) - 1] = sp;
	// 反転
	basic->InvertMem(m_data, sizeof(directory_mz_fdos_t));	// invert
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemMZFDOS::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	switch(t1) {
	case FILETYPE_MZ_FDOS_OBJ:
		val = FILE_TYPE_BINARY_MASK;	// binary
		val |= FILE_TYPE_MACHINE_MASK;	// machine
		break;
	case FILETYPE_MZ_FDOS_ASC:
		val = FILE_TYPE_ASCII_MASK;		// ascii
		break;
	case FILETYPE_MZ_FDOS_RB:
		val = FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_MZ_FDOS_SYS:
		val = FILE_TYPE_SYSTEM_MASK;	// system
		break;
	case FILETYPE_MZ_FDOS_LIB:
		val = FILE_TYPE_LIBRARY_MASK;	// library
		break;
	default:
		val = basic->GetSpecialAttributes().GetTypeByValue(t1);
		break;
	}
	int t2 = GetFileType2();

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t2 << 8 | t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMZFDOS::GetFileAttrStr() const
{
	wxString attr;
	GetFileAttrName(ConvFileType1Pos(GetFileType1()), gTypeNameMZFDOS, TYPE_NAME_MZ_FDOS_UNKNOWN, attr);

	return attr;
}

/// データ内にファイルサイズをセット
void DiskBasicDirItemMZFDOS::SetFileSizeBase(int val)
{
	m_data->fdos.file_size = basic->InvertAndOrderUint16(val);	// invert
}

/// データ内のファイルサイズを返す
int DiskBasicDirItemMZFDOS::GetFileSizeBase() const
{
	return basic->InvertAndOrderUint16(m_data->fdos.file_size);	// invert
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemMZFDOS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

//	// ファイルサイズ
//	m_file_size = basic->InvertAndOrderUint16(m_data->fdos.file_size);	// invert

	GetUnitGroups(fileunit_num, m_groups);
//	m_groups.SetNums(GetGroupSize());
}

/// グループ数をセット
/// @param [in] val 数
void DiskBasicDirItemMZFDOS::SetGroupSize(int val)
{
	m_groups.SetNums(val);
	m_data->fdos.groups = basic->InvertAndOrderUint16(val);
	chain.SetSectors(val);
}

/// グループ数を返す
/// @return 数
int DiskBasicDirItemMZFDOS::GetGroupSize() const
{
	return basic->InvertAndOrderUint16(m_data->fdos.groups);
}

/// グループ取得計算前処理
void DiskBasicDirItemMZFDOS::PreCalcAllGroups(int &calc_flags, wxUint32 &group_num, int &remain, int &sec_size, void **user_data)
{
	sec_size -= 2;

	group_num = GetDataGroup();
}

/// グループ取得計算中処理
void DiskBasicDirItemMZFDOS::CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data)
{
	group_num = type->GetNextGroupNumber(group_num, end_sec);
}

void DiskBasicDirItemMZFDOS::GetFileDate(struct tm *tm) const
{
	wxUint8 mmddyy[sizeof(m_data->fdos.mmddyy)];
	basic->InvertMem(m_data->fdos.mmddyy, sizeof(m_data->fdos.mmddyy), mmddyy);

	tm->tm_year = ('0' <= mmddyy[4] && mmddyy[4] <= '9' && '0' <= mmddyy[5] && mmddyy[5] <= '9') ? ((mmddyy[4] & 0xf) * 10 + (mmddyy[5] & 0xf)) : -1;
	tm->tm_mon = ('0' <= mmddyy[0] && mmddyy[0] <= '9' && '0' <= mmddyy[1] && mmddyy[1] <= '9') ? ((mmddyy[0] & 0xf) * 10 + (mmddyy[1] & 0xf)) - 1 : -2;
	tm->tm_mday = ('0' <= mmddyy[2] && mmddyy[2] <= '9' && '0' <= mmddyy[3] && mmddyy[3] <= '9') ? ((mmddyy[2] & 0xf) * 10 + (mmddyy[3] & 0xf)) : -1;
	if (0 <= tm->tm_year && tm->tm_year < 80) tm->tm_year += 100;	// 2000 - 2079
}

wxString DiskBasicDirItemMZFDOS::GetFileDateStr() const
{
	struct tm tm;
	GetFileDate(&tm);
	return Utils::FormatYMDStr(&tm);
}

void DiskBasicDirItemMZFDOS::SetFileDate(const struct tm *tm)
{
	wxUint8 mmddyy[sizeof(m_data->fdos.mmddyy)];
	int year = tm->tm_year % 100;
	mmddyy[4] = (tm->tm_year >= 0 ? (year / 10) + 0x30 : '?');
	mmddyy[5] = (tm->tm_year >= 0 ? (year % 10) + 0x30 : '?');
	int mon = tm->tm_mon + 1;
	mmddyy[0] = (tm->tm_mon >= 0 ? (mon / 10) + 0x30 : '?');
	mmddyy[1] = (tm->tm_mon >= 0 ? (mon % 10) + 0x30 : '?');
	mmddyy[2] = (tm->tm_mday >= 0 ? (tm->tm_mday / 10) + 0x30 : '?');
	mmddyy[3] = (tm->tm_mday >= 0 ? (tm->tm_mday % 10) + 0x30 : '?');
	mmddyy[6] = 0x0d;
	basic->InvertMem(mmddyy, sizeof(m_data->fdos.mmddyy), m_data->fdos.mmddyy);
}

// 開始アドレスを返す
int DiskBasicDirItemMZFDOS::GetStartAddress() const
{
	return basic->InvertAndOrderUint16(m_data->fdos.load_addr);	// invert
}

// 実行アドレスを返す
int DiskBasicDirItemMZFDOS::GetExecuteAddress() const
{
	return basic->InvertAndOrderUint16(m_data->fdos.exec_addr);	// invert
}

/// 開始アドレスをセット
void DiskBasicDirItemMZFDOS::SetStartAddress(int val)
{
	m_data->fdos.load_addr = (wxUint16)basic->InvertAndOrderUint16(val);	// invert
}

/// 実行アドレスをセット
void DiskBasicDirItemMZFDOS::SetExecuteAddress(int val)
{
	m_data->fdos.exec_addr = (wxUint16)basic->InvertUint16(val);	// invert
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemMZFDOS::GetDataSize() const
{
	return sizeof(directory_mz_fdos_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemMZFDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	int trk = 0;
	int sec = 1;
	basic->CalcNumFromSectorPosTForGroup(val * basic->GetSectorsPerGroup(), trk, sec);
	m_data->fdos.track = basic->InvertUint8(trk);	// invert
	m_data->fdos.sector = basic->InvertUint8(sec);	// invert
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemMZFDOS::GetStartGroup(int fileunit_num) const
{
	int trk = basic->InvertUint8(m_data->fdos.track);	// invert
	int sec = basic->InvertUint8(m_data->fdos.sector);	// invert
	return basic->CalcSectorPosFromNumTForGroup(trk, sec);
}

/// 追加のグループ番号をセット
void DiskBasicDirItemMZFDOS::SetExtraGroup(wxUint32 val)
{
	int trk = 0;
	int sec = 1;
	basic->CalcNumFromSectorPosTForGroup(val * basic->GetSectorsPerGroup(), trk, sec);
	m_data->fdos.track = basic->InvertUint8(trk);	// invert
	m_data->fdos.sector = basic->InvertUint8(sec);	// invert
}

/// 追加のグループ番号を返す
wxUint32 DiskBasicDirItemMZFDOS::GetExtraGroup() const
{
	int trk = basic->InvertUint8(m_data->fdos.track);	// invert
	int sec = basic->InvertUint8(m_data->fdos.sector);	// invert
	return basic->CalcSectorPosFromNumTForGroup(trk, sec);
}

/// 追加のグループ番号を得る
void DiskBasicDirItemMZFDOS::GetExtraGroups(wxArrayInt &arr) const
{
	arr.Add((int)GetExtraGroup());
}

/// データのあるグループ番号をセット
void DiskBasicDirItemMZFDOS::SetDataGroup(wxUint32 val)
{
	int trk = 0;
	int sec = 1;
	basic->CalcNumFromSectorPosTForGroup(val * basic->GetSectorsPerGroup(), trk, sec);
	m_data->fdos.data_track = basic->InvertUint8(trk);	// invert
	m_data->fdos.data_sector = basic->InvertUint8(sec);	// invert
}

/// データのあるグループ番号を返す
wxUint32 DiskBasicDirItemMZFDOS::GetDataGroup() const
{
	int trk = basic->InvertUint8(m_data->fdos.data_track);	// invert
	int sec = basic->InvertUint8(m_data->fdos.data_sector);	// invert
	return basic->CalcSectorPosFromNumTForGroup(trk, sec);
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemMZFDOS::InvalidateChars() const
{
	return wxT("\"\\:*?");
}

/// ファイル名に付随する拡張属性を設定
void DiskBasicDirItemMZFDOS::SetOptionalName(int val)
{
	val &= 0xf;
	val |= (GetFileType1() & 0xf0);
	SetFileType1(val);
}

/// ファイル名に付随する拡張属性を返す
int DiskBasicDirItemMZFDOS::GetOptionalName() const
{
	return GetFileType1() & 0xf;
}

/// エントリデータの不明部分を設定
void DiskBasicDirItemMZFDOS::SetUnknownData()
{
	m_data->fdos.unknown1 = basic->InvertUint8(0x9f);
	m_data->fdos.unknown2 = basic->InvertUint8(0x15);
}

/// データ内部にチェインデータが必要か
bool DiskBasicDirItemMZFDOS::NeedChainInData()
{
	return true;
}

/// データをエクスポートする前に必要な処理
/// 属性に応じて拡張子".xxx"を付加する
///
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemMZFDOS::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	/// 属性から拡張子を付加する
	wxString ext;
	if (GetFileAttrName(ConvFileType1Pos(GetFileType1()), gTypeNameMZFDOS, TYPE_NAME_MZ_FDOS_UNKNOWN, ext)) {
		filename += wxT(".");
		if (Utils::IsUpperString(filename)) {
			filename += ext.Upper();
		} else {
			filename += ext.Lower();
		}
	}
	return true;
}

/// シーケンス番号
void DiskBasicDirItemMZFDOS::AssignSeqNumber()
{
	m_data->fdos.seq_num = basic->InvertUint8(m_num); 
}

/// チェイン情報にセクタをセット
void DiskBasicDirItemMZFDOS::SetChainSector(DiskD88Sector *sector, wxUint8 *data, const DiskBasicDirItem *pitem)
{
	chain.Set(basic, sector, (mz_fdos_chain_t *)data);
}

/// チェイン情報にセクタをセット
void DiskBasicDirItemMZFDOS::SetChainUsedSector(int sector_pos, bool val)
{
	chain.UsedSector(sector_pos, val);
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
bool DiskBasicDirItemMZFDOS::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		IsContainAttrByExtension(filename, gTypeNameMZFDOS, TYPE_NAME_MZ_FDOS_OBJ, TYPE_NAME_MZ_FDOS_GRH, &filename, NULL);
	}
	// 拡張子を消す
	filename = RemakeFileNameOnlyStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemMZFDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = TYPE_NAME_MZ_FDOS_ASC;
	// 拡張子で属性を設定する
	IsContainAttrByExtension(filename, gTypeNameMZFDOS, TYPE_NAME_MZ_FDOS_OBJ, TYPE_NAME_MZ_FDOS_GRH, NULL, &t1);

	// プロテクト
	t1 |= (MZ_FDOS_NO_PROTECT << 8);

	return t1;
}

/// ファイル名から拡張属性を決定する
int DiskBasicDirItemMZFDOS::ConvOptionalNameFromFileName(const wxString &filename) const
{
	return (ConvOriginalTypeFromFileName(filename) & 0xff);
}

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_COMBO_TYPE1 51
#define IDC_TEXT_ATTR1	52

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemMZFDOS::ConvFileType1Pos(int native_type) const
{
	int val = -1;
	for(int i=0; gTypeNamePosMZFDOS[i] >= 0; i++) {
		if (native_type == gTypeNamePosMZFDOS[i]) {
			val = i;
			break;
		}
	}
	if (val < 0) {
		val = -native_type;
	}
	return val;
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemMZFDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = ConvOriginalTypeFromFileName(name);
		file_type_2 = file_type_1 >> 8;
		file_type_1 &= 0xff;
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemMZFDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int type1 = GetFileType1();
	int type2 = GetFileType2();

	parent->SetUserData(type2 << 8 | type1);

	int file_type_1 = ConvFileType1Pos(type1);
	int file_type_2 = type2;

	wxChoice *comType1;
	wxTextCtrl *txtAttr1;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);

	wxArrayString types1;
	CreateChoiceForAttrDialog(basic, gTypeNameMZFDOS, TYPE_NAME_MZ_FDOS_END, types1);

	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	file_type_1 = SelectChoiceForAttrDialog(basic, comType1, file_type_1, TYPE_NAME_MZ_FDOS_END, TYPE_NAME_MZ_FDOS_UNKNOWN);
	staType1->Add(comType1, flags);
	sizer->Add(staType1, flags);

	wxStaticBoxSizer *staType2 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);

	wxUint8 attr1[4];
	attr1[0] = (file_type_2 >> 8) & 0xff;
	attr1[1] = (file_type_2) & 0xff;
	attr1[2] = 0;
	txtAttr1 = new wxTextCtrl(parent, IDC_TEXT_ATTR1, attr1, wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
	staType2->Add(txtAttr1, flags);
	sizer->Add(staType2, flags);

	// event handler
	parent->Bind(wxEVT_CHOICE, &IntNameBox::OnChangeType1, parent, IDC_COMBO_TYPE1);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemMZFDOS::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);

	int selected_idx = 0;
	if (comType1) {
		selected_idx = comType1->GetSelection();
	}

	bool enable = (selected_idx == TYPE_NAME_MZ_FDOS_OBJ || selected_idx == TYPE_NAME_MZ_FDOS_SYS);
	if (selected_idx >= TYPE_NAME_MZ_FDOS_END) {
		int t = CalcSpecialFileTypeFromPos(basic, selected_idx, TYPE_NAME_MZ_FDOS_END);
		enable = (((t & FILE_TYPE_MACHINE_MASK) != 0) || ((t & FILE_TYPE_SYSTEM_MASK) != 0));
	}
	parent->EnableStartAddress(enable);
	parent->EnableExecuteAddress(enable);
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemMZFDOS::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);

	return comType1->GetSelection();
}

/// 属性2を得る
int DiskBasicDirItemMZFDOS::GetFileType2InAttrDialog(const IntNameBox *parent) const
{
	wxTextCtrl *txtAttr1 = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_ATTR1);

	wxCharBuffer buf = txtAttr1->GetValue().To8BitData();
	int attr1 = 0x3053;	// "0S"
	if (buf.length() == 2) {
		attr1 = buf.data()[0];
		attr1 <<= 8;
		attr1 |= buf.data()[1];
	}
	return attr1;
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int	DiskBasicDirItemMZFDOS::CalcFileTypeFromPos(int pos) const
{
	int val = 0;
	if (TYPE_NAME_MZ_FDOS_OBJ <= pos && pos <= TYPE_NAME_MZ_FDOS_GRH) {
		val = pos;
	} else {
		val = CalcSpecialOriginalTypeFromPos(basic, pos, TYPE_NAME_MZ_FDOS_END);
	}
	return val;
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemMZFDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int val = GetFileType1InAttrDialog(parent);

	int t1 = CalcFileTypeFromPos(val);
	if (t1 < 0) {
		t1 = parent->GetUserData() & 0xff;
	}

	t1 |= (GetFileType2InAttrDialog(parent) << 8);

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t1);

	return true;
}

/// 属性値を加工する
bool DiskBasicDirItemMZFDOS::ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int t1 = (attr.GetFileOriginAttr() & 0xff);
	if (t1 != FILETYPE_MZ_FDOS_OBJ && t1 != FILETYPE_MZ_FDOS_SYS) {
		// バイナリ以外はアドレス固定
		attr.SetStartAddress(0);
		attr.SetExecuteAddress(0xffff);
	}
	return true;
}

/// ファイルサイズが適正か
/// @param [in]  parent     ダイアログ
/// @param [in]  size       ファイルサイズ
/// @param [out] limit      制限サイズ
/// @return true 適正
bool DiskBasicDirItemMZFDOS::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	return true;
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemMZFDOS::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	// 空白はNG
	if (filename.empty()) {
		errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILENAME_EMPTY]);
		return false;
	}
	return true;
}

/// ファイル名に付随する拡張属性をセットする
/// 属性が異なれば別ファイルとして扱う
int DiskBasicDirItemMZFDOS::GetOptionalNameInAttrDialog(const IntNameBox *parent)
{
	int val = GetFileType1InAttrDialog(parent);
	if (val >= 0) {
		val = gTypeNamePosMZFDOS[val];
	} else {
		val = 0;
	}
	return val;
}
