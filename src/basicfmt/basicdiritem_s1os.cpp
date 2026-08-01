/// @file basicdiritem_s1os.cpp
///
/// @brief disk basic directory item for S1-OS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_s1os.h"
#include "basicfmt.h"
#include "basictype_s1os.h"
#include "../charcodes.h"
#include "../config.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// S1-OS属性名 種類
const name_value_t gTypeNameS1OS_1[] = {
	{ wxTRANSLATE("File"), FILETYPE_S1OS_N },
	{ wxTRANSLATE("Directory"), FILETYPE_S1OS_D },
	{ wxTRANSLATE("Record"), FILETYPE_S1OS_R },
	{ NULL, -1 }
};

/// S1-OS属性名 種類(リスト用)
const name_value_t gTypeNameShortS1OS_1[] = {
	{ wxTRANSLATE("File"), FILETYPE_S1OS_N },
	{ wxTRANSLATE("<DIR>"), FILETYPE_S1OS_D },
	{ wxTRANSLATE("Rec"), FILETYPE_S1OS_R },
	{ NULL, -1 }
};

/// S1-OS属性名 状態
const name_value_t gTypeNameS1OS_2[] = {
	{ wxTRANSLATE("Normal"), FILETYPE_S1OS_N },
	{ wxTRANSLATE("Destroyed"), FILETYPE_S1OS_D },
	{ wxTRANSLATE("Erased"), FILETYPE_S1OS_E },
	{ NULL, -1 }
};

/// S1-OS属性名 状態(リスト用)
const name_value_t gTypeNameShortS1OS_2[] = {
	{ "", FILETYPE_S1OS_N },
	{ wxTRANSLATE("Destroyed"), FILETYPE_S1OS_D },
	{ wxTRANSLATE("Erased"), FILETYPE_S1OS_E },
	{ NULL, -1 }
};

/// S1-OS属性名 権限？
const name_value_t gTypeNameS1OS_3[] = {
	{ "", FILETYPE_S1OS_L },
	{ NULL, -1 }
};

/// S1-OS属性名 権限？(リスト用)
const name_value_t gTypeNameShortS1OS_3[] = {
	{ "", FILETYPE_S1OS_L },
	{ NULL, -1 }
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemS1OS::DiskBasicDirItemS1OS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
	AllocateItem(NULL);
}
DiskBasicDirItemS1OS::DiskBasicDirItemS1OS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	AllocateItem(NULL);
}
DiskBasicDirItemS1OS::DiskBasicDirItemS1OS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);
	AllocateItem(n_next);

	Used(CheckUsed(n_unuse));

	CalcFileSize();
}

/// 初期状態に戻す
void DiskBasicDirItemS1OS::Reset()
{
	m_data.Delete();

	DiskBasicDirItem::Reset();

	m_data.Alloc();
	AllocateItem(NULL);
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemS1OS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
	AllocateItem(n_next);
}

/// ディレクトリエントリを確保
/// data  は内部で確保したメモリ
/// sdata がセクタ内部へのポインタとなる
bool DiskBasicDirItemS1OS::AllocateItem(const SectorParam *next)
{
	m_sdata.Clear();
	bool bound = m_sdata.Set(basic, m_sector, m_position, (directory_t *)m_data.Data(), GetDataSize(), next);
	
	if (!m_data.IsSelf() && bound) {
		// セクタをまたぐ場合、dataは内部で確保する
		m_data.Alloc();
		m_data.Fill(0, sizeof(directory_s1os_t));
	}

	// コピー
	if (m_data.IsSelf()) {
		m_sdata.CopyTo((directory_t *)m_data.Data());
	}

	return true;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemS1OS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// ファイル名を設定
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
void DiskBasicDirItemS1OS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		m_data.Data()->name_len = (wxUint8)(ns < length ? ns : length);
		memcpy(m_data.Data()->name, filename, m_data.Data()->name_len); 
	}
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemS1OS::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t s = 0;
	size_t l = 0;

	n = GetFileNamePos(0, s, l);
	if (n != NULL && s > 0) {
		if (s < size) size = s;
		memcpy(&filename[0], m_data.Data()->name, size);
		l = m_data.Data()->name_len;
	}

	length = l;
}

/// 属性１を返す
int	DiskBasicDirItemS1OS::GetFileType1() const
{
	return m_data.Data()->attr.k;
}

/// 属性２を返す
int	DiskBasicDirItemS1OS::GetFileType2() const
{
	return m_data.Data()->attr.s;
}

/// 属性３を返す
int	DiskBasicDirItemS1OS::GetFileType3() const
{
	return m_data.Data()->attr.p;
}

/// 属性１を設定
void DiskBasicDirItemS1OS::SetFileType1(int val)
{
	m_data.Data()->attr.k = (wxUint8)val;
}

/// 属性２を設定
void DiskBasicDirItemS1OS::SetFileType2(int val)
{
	m_data.Data()->attr.s = (wxUint8)val;
}

/// 属性３を設定
void DiskBasicDirItemS1OS::SetFileType3(int val)
{
	m_data.Data()->attr.p = (wxUint8)val;
}

/// 使用しているアイテムか
bool DiskBasicDirItemS1OS::CheckUsed(bool unuse)
{
	wxUint16 val = basic->OrderUint16(m_data.Data()->record_length);
	if (val != (wxUint16)(sizeof(directory_s1os_t) - sizeof(wxUint16))) {
		return false;
	}
	if (m_data.Data()->name_len == 0 || m_data.Data()->name[0] == 0) {
		return false;
	}
	int t2 = GetFileType2();
	return (t2 == FILETYPE_S1OS_N || t2 == FILETYPE_S1OS_E);
}

/// 削除
bool DiskBasicDirItemS1OS::Delete()
{
	// ステータス属性を'd'にする。
	SetFileType2(FILETYPE_S1OS_D);
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemS1OS::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

//	wxUint16 val = basic->OrderUint16(m_data.Data()->record_length);
//	if (val != (wxUint16)(sizeof(directory_s1os_t) - sizeof(wxUint16))) {
//		last = true;
//	}
	return true;
}

/// ファイル名に付随する拡張属性を返す
///
/// ファイル検索で使用。削除属性が付いたファイル名は一致させないようにする。
int DiskBasicDirItemS1OS::GetOptionalName() const
{
	int t2 = GetFileType2();
	int val = 0;
	// Erased, Destroyed属性のファイルなら重複可能
	if (t2 != FILETYPE_S1OS_N) {
		if (m_sector) {
			val |= m_sector->GetIDC();
			val <<= 8;
			val |= m_sector->GetIDH();
			val <<= 8;
			val |= m_sector->GetIDR();
			val <<= 8;
		}
		val |= (m_num & 0xff);
	}
	return val;
}

/// 属性を設定
void DiskBasicDirItemS1OS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = FILETYPE_S1OS_N;
	int t2 = FILETYPE_S1OS_N;
	int t3 = FILETYPE_S1OS_L;
//	wxUint16 rev = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		t1 = file_type.GetOrigin(0);
		t2 = file_type.GetOrigin(1);
		t3 = file_type.GetOrigin(2);
	} else {
		t1 = ConvToNativeType1(ftype);
		t2 = ConvToNativeType2(ftype);
	}
	int n = -1;
	n = name_value_t::IndexOf(gTypeNameShortS1OS_1, t1);
	if (n < 0) t1 = FILETYPE_S1OS_N;
	n = name_value_t::IndexOf(gTypeNameShortS1OS_2, t2);
	if (n < 0) t2 = FILETYPE_S1OS_N;
	n = name_value_t::IndexOf(gTypeNameShortS1OS_3, t3);
	if (n < 0) t3 = FILETYPE_S1OS_L;

	SetFileType1(t1);
	SetFileType2(t2);
	SetFileType3(t3);
}

/// 属性1を変換
int DiskBasicDirItemS1OS::ConvToNativeType1(int file_type) const
{
	int val = FILETYPE_S1OS_N;
	if (file_type & FILE_TYPE_DIRECTORY_MASK) {
		val = FILETYPE_S1OS_D;
	}
	return val;
}

/// 属性2を変換
int DiskBasicDirItemS1OS::ConvToNativeType2(int file_type) const
{
	int val = FILETYPE_S1OS_N;
	if (file_type & FILE_TYPE_HIDDEN_MASK) {
		val = FILETYPE_S1OS_E;
	}
	return val;
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemS1OS::GetFileAttr() const
{
	int val = 0;
	int t1 = GetFileType1();
	switch(t1) {
	case FILETYPE_S1OS_D:
		val |= FILE_TYPE_DIRECTORY_MASK;
		break;
	default:
		break;
	}
	int t2 = GetFileType2();
	switch(t1) {
	case FILETYPE_S1OS_E:
		val |= FILE_TYPE_HIDDEN_MASK;
		break;
	default:
		break;
	}
	int t3 = GetFileType3();

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1, t2, t3);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemS1OS::GetFileAttrStr() const
{
	wxString attr;
	int n;

	n = name_value_t::IndexOf(gTypeNameShortS1OS_1, GetFileType1());
	if (n >= 0) {
		attr += wxGetTranslation(gTypeNameShortS1OS_1[n].name);
	}
	n = name_value_t::IndexOf(gTypeNameShortS1OS_2, GetFileType2());
	if (n >= 0) {
		wxString str = wxGetTranslation(gTypeNameShortS1OS_2[n].name);
		if (!attr.IsEmpty() && !str.IsEmpty()) attr += wxT(",");
		attr += str;
	}
	return attr;
}

#if 0
/// データ内にファイルサイズをセット
void DiskBasicDirItemS1OS::SetFileSizeBase(int val)
{
	int sec_size = basic->GetSectorSize();
	if (NeedChainInData()) {
		sec_size -= 2;
	}
	wxUint16 sec = val / sec_size;
	int eof = val % sec_size;
	if (eof > 0) {
		sec++;
	}
//	m_data.Data()->block_size = basic->OrderUint16(sec); 
//	m_data.Data()->eof_in_sector = (eof & 0xff);
}

/// データ内のファイルサイズを返す
int DiskBasicDirItemS1OS::GetFileSizeBase() const
{
}
#endif

/// ファイルサイズをセット
void DiskBasicDirItemS1OS::SetFileSize(int val)
{
	m_groups.SetSize(val);
	ConvFromFileSize(basic, val, &m_data.Data()->size);
}

/// ファイルサイズを返す
int DiskBasicDirItemS1OS::GetFileSize() const
{
	int val = ConvToFileSize(basic, &m_data.Data()->size);
	if (val == 0) {
		val = (int)m_groups.GetSize();
	}
	return val;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemS1OS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// ファイルサイズを内部形式に変換
void DiskBasicDirItemS1OS::ConvFromFileSize(DiskBasic *basic, int file_size, s1os_size_t *s1os_size)
{
	// セクタ先頭のチェイン部分は除外
	int sec_size = basic->GetSectorSize() - (int)sizeof(s1os_chain_t);
	int secs = file_size / sec_size;
	int bytes = file_size % sec_size;
	if (file_size > 0) secs++;
	s1os_size->secs = basic->OrderUint32(secs);
	s1os_size->bytes = basic->OrderUint16(bytes);
}

/// 内部形式のファイルサイズを変換
int DiskBasicDirItemS1OS::ConvToFileSize(DiskBasic *basic, const s1os_size_t *s1os_size)
{
	// セクタ先頭のチェイン部分は除外
	int sec_size = basic->GetSectorSize() - (int)sizeof(s1os_chain_t);
	int secs = (int)basic->OrderUint32(s1os_size->secs);
	int bytes = (int)basic->OrderUint16(s1os_size->bytes);
	if (secs > 0) secs--;
	return secs * sec_size + bytes;
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemS1OS::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	((DiskBasicTypeS1OS *)type)->CountGroups(&m_data.Data()->size, &m_data.Data()->start, group_items);
}

#if 0
/// グループ取得計算中処理
void DiskBasicDirItemS1OS::CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data)
{
	if (NeedChainInData()) {
		// セクタ末尾にある次のセクタ番号を得る
		DiskImageSector *sector = basic->GetSectorFromGroup(group_num);
		if (!sector) {
			// Why?
			group_num = INVALID_GROUP_NUMBER;
			return;
		}
		group_num = sector->Get16(-2, basic->IsBigEndian());
		if (group_num == 0 || group_num > basic->GetFatEndGroup()) {
			group_num = INVALID_GROUP_NUMBER;
		}
	} else {
		// 連続している
		group_num++;
	}
}
#endif

/// 日付を得る
void DiskBasicDirItemS1OS::GetFileCreateDate(TM &tm) const
{
	ConvFromFileDateTime(basic, m_data.Data()->date_time, tm);
}

/// 時間を得る
void DiskBasicDirItemS1OS::GetFileCreateTime(TM &tm) const
{
	ConvFromFileDateTime(basic, m_data.Data()->date_time, tm);
}

/// 日付文字列を得る
wxString DiskBasicDirItemS1OS::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 時間文字列を得る
wxString DiskBasicDirItemS1OS::GetFileCreateTimeStr() const
{
	TM tm;
	GetFileCreateTime(tm);
	return Utils::FormatHMSStr(tm);
}

/// 日付時間をセット
void DiskBasicDirItemS1OS::SetFileCreateDateTime(const TM &tm)
{
	m_data.Data()->date_time = ConvToFileDateTime(basic, tm);
}

/// 内部形式の日付時間を変換
void DiskBasicDirItemS1OS::ConvFromFileDateTime(DiskBasic *basic, wxUint32 date_time, TM &tm)
{
	wxDateTime dt(1, wxDateTime::Jan, 1980); 

	time_t t = dt.GetTicks();
	t += (time_t)basic->OrderUint32(date_time);
	dt.Set(t);

	tm.Set(dt);
}

/// 日付時間を内部形式に変換
wxUint32 DiskBasicDirItemS1OS::ConvToFileDateTime(DiskBasic *basic, const TM &tm)
{
	wxDateTime dt(1, wxDateTime::Jan, 1980); 
	wxDateTime ndt;

	tm.Get(ndt);
	time_t t = ndt.GetTicks();
	t -= dt.GetTicks();

	return basic->OrderUint32((wxUint32)t);
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemS1OS::GetDataSize() const
{
	return sizeof(directory_s1os_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemS1OS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemS1OS::CopyData(const directory_t *val)
{
	m_data.Copy(val, GetDataSize());
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
	return true;
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemS1OS::ClearData()
{
	if (!m_data.IsValid()) return;

	m_data.Fill(0);

	directory_s1os_t *data = m_data.Data();
	if (!data) return;

	data->record_length = basic->OrderUint16(sizeof(directory_s1os_t) - sizeof(wxUint16));
}

/// 最初のグループ番号を設定
void DiskBasicDirItemS1OS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;
	type->GetNumFromSectorPos(val, trk_num, sid_num, sec_num);
	SetCHR(basic, trk_num, sid_num, sec_num, &m_data.Data()->start); 
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemS1OS::GetStartGroup(int fileunit_num) const
{
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;
	GetCHR(basic, &m_data.Data()->start, trk_num, sid_num, sec_num); 
	return (wxUint32)type->GetSectorPosFromNum(trk_num, sid_num, sec_num);
}

/// トラック、サイド、セクタ番号を取得
/// @param [in] basic : DiskBasic
/// @param [in] ptr : ポインタ
/// @param [out] trk_num : トラック番号
/// @param [out] sid_num : サイド番号
/// @param [out] sec_num : セクタ番号
void DiskBasicDirItemS1OS::GetCHR(DiskBasic *basic, const s1os_ptr_t *ptr, int &trk_num, int &sid_num, int &sec_num)
{
	trk_num = basic->OrderUint16(ptr->track);
	sid_num = basic->OrderUint16(ptr->side);
	sec_num = basic->OrderUint16(ptr->sector);
}

/// トラック、サイド、セクタ番号をセット
/// @param [in] basic : DiskBasic
/// @param [in] trk_num : トラック番号
/// @param [in] sid_num : サイド番号
/// @param [in] sec_num : セクタ番号
/// @param [out] ptr : ポインタ
void DiskBasicDirItemS1OS::SetCHR(DiskBasic *basic, int trk_num, int sid_num, int sec_num, s1os_ptr_t *ptr)
{
	ptr->track = basic->OrderUint16(trk_num);
	ptr->side = basic->OrderUint16(sid_num);
	ptr->sector = basic->OrderUint16(sec_num);
}

/// データ内部にチェインデータが必要か
bool DiskBasicDirItemS1OS::NeedChainInData() const
{
	return true;
}

/// データをエクスポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemS1OS::PreExportDataFile(wxString &filename)
{
	return true;
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
bool DiskBasicDirItemS1OS::PreImportDataFile(wxString &filename)
{
	return true;
}

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemS1OS::SetModify()
{
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
}

/// ファイル名から属性を決定する
int DiskBasicDirItemS1OS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	return FILETYPE_S1OS_N;
}

/// 属性値を加工する
bool DiskBasicDirItemS1OS::ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	return true;
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemS1OS::ValidateFileName(const wxString &filename, wxString &errormsg)
{
	// 空白はNG
	if (filename.empty()) {
		errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILENAME_EMPTY]);
		return false;
	}
	return true;
}

#ifndef USE_CONSOLE

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_COMBO_TYPE1 51
#define IDC_COMBO_TYPE2 52

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemS1OS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = ConvOriginalTypeFromFileName(name);
		file_type_2 = 0;
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemS1OS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int type1 = GetFileType1();
	int type2 = GetFileType2();

	int file_type_1 = type1;
	int file_type_2 = type2;

	wxChoice *comType1;
	wxChoice *comType2;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

//	wxSizerFlags expand = wxSizerFlags().Expand();

	wxArrayString types1;
	for(size_t i=0; gTypeNameS1OS_1[i].name != NULL; i++) {
		types1.Add(wxGetTranslation(gTypeNameS1OS_1[i].name));
	}
	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Kind")), wxVERTICAL);

	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	for(size_t i=0; gTypeNameS1OS_1[i].name != NULL; i++) {
		if (gTypeNameS1OS_1[i].value == file_type_1) {
			comType1->SetSelection(i);
			break;
		}
	}
	staType1->Add(comType1, flags);
	sizer->Add(staType1, flags);

	wxArrayString types2;
	for(size_t i=0; gTypeNameS1OS_2[i].name != NULL; i++) {
		types2.Add(wxGetTranslation(gTypeNameS1OS_2[i].name));
	}
	wxStaticBoxSizer *staType2 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Status")), wxVERTICAL);
	comType2 = new wxChoice(parent, IDC_COMBO_TYPE2, wxDefaultPosition, wxDefaultSize, types2);
	for(size_t i=0; gTypeNameS1OS_2[i].name != NULL; i++) {
		if (gTypeNameS1OS_2[i].value == file_type_2) {
			comType2->SetSelection(i);
			break;
		}
	}
	staType2->Add(comType2, flags);
	sizer->Add(staType2, flags);

	// 新規作成時は選択不可
	if (show_flags & INTNAME_NEW_FILE) {
		comType1->SetSelection(TYPE_NAME_S1OS_1_N);
		comType2->SetSelection(TYPE_NAME_S1OS_2_N);
		comType1->Enable(false);
		comType2->Enable(false);

	}

//	parent->Bind(wxEVT_CHOICE, &IntNameBox::OnChangeType1, parent, IDC_COMBO_TYPE1);
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemS1OS::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);

	int sel = comType1->GetSelection();
	int val = FILETYPE_S1OS_N;
	if(sel >= 0 && sel < TYPE_NAME_S1OS_1_END) {
		val = gTypeNameS1OS_1[sel].value;
	}
	return val;
}

/// 属性2を得る
int DiskBasicDirItemS1OS::GetFileType2InAttrDialog(const IntNameBox *parent) const
{
	wxChoice *comType2 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE2);

	int sel = comType2->GetSelection();
	int val = FILETYPE_S1OS_N;
	if(sel >= 0 && sel < TYPE_NAME_S1OS_2_END) {
		val = gTypeNameS1OS_2[sel].value;
	}
	return val;
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemS1OS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int t1 = GetFileType1InAttrDialog(parent);
	int t2 = GetFileType2InAttrDialog(parent);
	int t3 = GetFileType3();
	if (t3 < 0x20 || t3 >= 0x80) {
		t3 = FILETYPE_S1OS_L;
	} 

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t1, t2, t3);

	return true;
}

/// ファイルサイズが適正か
/// @param [in]  parent     ダイアログ
/// @param [in]  size       ファイルサイズ
/// @param [out] limit      制限サイズ
/// @return true 適正
bool DiskBasicDirItemS1OS::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	return true;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemS1OS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("SIZE_SECS"), m_data.Data()->size.secs, basic->IsBigEndian());
	vals.Add(wxT("SIZE_BYTES"), m_data.Data()->size.bytes, basic->IsBigEndian());
	vals.Add(wxT("START_TRACK"), m_data.Data()->start.track, basic->IsBigEndian());
	vals.Add(wxT("START_SIDE"), m_data.Data()->start.side, basic->IsBigEndian());
	vals.Add(wxT("START_SECTOR"), m_data.Data()->start.sector, basic->IsBigEndian());
	vals.Add(wxT("DATE_TIME"), m_data.Data()->date_time, basic->IsBigEndian());
	vals.Add(wxT("ATTR_S"), m_data.Data()->attr.s);
	vals.Add(wxT("ATTR_K"), m_data.Data()->attr.k);
	vals.Add(wxT("ATTR_P"), m_data.Data()->attr.p);
	size_t name_len = m_data.Data()->name_len;
	vals.Add(wxT("NAME_LEN"), (wxUint8)name_len);
	vals.Add(wxT("NAME"), m_data.Data()->name, name_len);
}

#endif /* USE_CONSOLE */
