/// @file basicdiritem_m68fdos.cpp
///
/// @brief disk basic directory item for Sord M68 FDOS (KDOS)
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_m68fdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include "../config.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// M68 FDOS属性名
const name_value_t gTypeNameM68FDOS[] = {
	{ wxTRANSLATE("A - Attribute Protected"), FILETYPE_M68_FDOS_A },
	{ wxTRANSLATE("P - Permanent"), FILETYPE_M68_FDOS_P },
	{ wxTRANSLATE("W - Write Protected"), FILETYPE_M68_FDOS_W },
	{ wxTRANSLATE("R - Read Protected"), FILETYPE_M68_FDOS_R },
	{ wxTRANSLATE("X - Xfer Protected"), FILETYPE_M68_FDOS_X },
	{ wxTRANSLATE("S - Saved Memory Image"), FILETYPE_M68_FDOS_S },
	{ wxTRANSLATE("C - Continuous"), FILETYPE_M68_FDOS_C },
	{ wxTRANSLATE("D - Device"), FILETYPE_M68_FDOS_D },
	{ NULL, -1 }
};

/// M68 FDOS属性名(リスト用)
const name_value_t gTypeNameShortM68FDOS[] = {
	{ wxTRANSLATE("A"), FILETYPE_M68_FDOS_A },
	{ wxTRANSLATE("P"), FILETYPE_M68_FDOS_P },
	{ wxTRANSLATE("W"), FILETYPE_M68_FDOS_W },
	{ wxTRANSLATE("R"), FILETYPE_M68_FDOS_R },
	{ wxTRANSLATE("X"), FILETYPE_M68_FDOS_X },
	{ wxTRANSLATE("S"), FILETYPE_M68_FDOS_S },
	{ wxTRANSLATE("C"), FILETYPE_M68_FDOS_C },
	{ wxTRANSLATE("D"), FILETYPE_M68_FDOS_D },
	{ NULL, -1 }
};

/// M68 FDOS ファイル名マッピングテーブル
static const char sM68FDOS_CharMap[] = {
	' ','?','0','1','2','3','4','5','6','7',
	'8','9','A','B','C','D','E','F','G','H',
	'I','J','K','L','M','N','O','P','Q','R',
	'S','T','U','V','W','X','Y','Z','+','-',
};

/// M68 FDOS拡張子から属性を設定
const name_value_t gExtNameM68FDOS[] = {
	{ "SAV", FILETYPE_M68_FDOS_C },
	{ NULL, -1 }
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemM68FDOS::DiskBasicDirItemM68FDOS(DiskBasic *basic)
	: DiskBasicDirItemMZBase(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemM68FDOS::DiskBasicDirItemM68FDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemMZBase(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
}
DiskBasicDirItemM68FDOS::DiskBasicDirItemM68FDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemMZBase(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);

	Used(CheckUsed(n_unuse));

	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemM68FDOS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemMZBase::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemM68FDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = 6;
		return m_data.Data()->name.b;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemM68FDOS::GetFileExtPos(size_t &len) const
{
	len = 3;
	return m_data.Data()->ext.b;
}

/// ファイル名を設定
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
/// @note filename はデータビットが反転している場合あり
void DiskBasicDirItemM68FDOS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		m_data.Data()->name.w[0] = basic->OrderUint16(EncodeName(&filename[0], size));
		m_data.Data()->name.w[1] = basic->OrderUint16(EncodeName(&filename[3], size - 3));
	}
}

/// 拡張子を設定
/// @param [in,out] fileext  拡張子
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
/// @note fileext はデータビットが反転している場合あり
void DiskBasicDirItemM68FDOS::SetNativeExt(wxUint8 *fileext, size_t size, size_t length)
{
	wxUint8 *e;
	size_t el = 0;
	e = GetFileExtPos(el);
	if (e != NULL && el > 0) {
		m_data.Data()->ext.w = basic->OrderUint16(EncodeName(&fileext[0], size));
	}
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
/// @note データビットは反転させたまま
void DiskBasicDirItemM68FDOS::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t s = 0;
	size_t l = 0;

	n = GetFileNamePos(0, s, l);
	if (n != NULL && s > 0) {
		if (s > size) s = size;
		DecodeName(basic->OrderUint16(m_data.Data()->name.w[0]), &filename[0], size);
		DecodeName(basic->OrderUint16(m_data.Data()->name.w[1]), &filename[3], size - 3);
	}

	length = l;
}

/// 拡張子を得る
/// @param [in,out] fileext  拡張子
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
/// @note データビットは反転させたまま
void DiskBasicDirItemM68FDOS::GetNativeExt(wxUint8 *fileext, size_t size, size_t &length) const
{
	wxUint8 *e = NULL;
	size_t l = 0;

	e = GetFileExtPos(l);
	if (e != NULL && l > 0) {
		DecodeName(basic->OrderUint16(m_data.Data()->ext.w), &fileext[0], size);
	}

	length = l;
}

/// 属性１を返す
int	DiskBasicDirItemM68FDOS::GetFileType1() const
{
	return basic->OrderUint16(m_data.Data()->attr1);
}

/// 属性２を返す
int	DiskBasicDirItemM68FDOS::GetFileType2() const
{
	return basic->OrderUint16(m_data.Data()->attr2);
}

/// 属性３を返す
int	DiskBasicDirItemM68FDOS::GetFileType3() const
{
	return m_data.Data()->attr3;
}

/// 属性１を設定
void DiskBasicDirItemM68FDOS::SetFileType1(int val)
{
	m_data.Data()->attr1 = basic->OrderUint16(val);
}

/// 属性２を設定
void DiskBasicDirItemM68FDOS::SetFileType2(int val)
{
	m_data.Data()->attr2 = basic->OrderUint16(val);
}

/// 属性３を設定
void DiskBasicDirItemM68FDOS::SetFileType3(int val)
{
	m_data.Data()->attr3 = (val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemM68FDOS::CheckUsed(bool unuse)
{
	return !(m_data.Data()->name.w[0] == 0 && m_data.Data()->name.w[1] == 0 && m_data.Data()->ext.w == 0);
}

/// リビジョンを返す
wxString DiskBasicDirItemM68FDOS::GetRevisionStr() const
{
	wxString str;
	wxUint16 rev = GetRevision();
	if (rev != 0) {
		wxUint8 revstr[4];
		DecodeName(rev, revstr, 3);
		revstr[3] = 0;
		str = wxString((char *)revstr);
	}
	return str;
}

/// リビジョンを返す
wxUint16 DiskBasicDirItemM68FDOS::GetRevision() const
{
	return basic->OrderUint16(m_data.Data()->rev.w);
}

/// リビジョンをセット
void DiskBasicDirItemM68FDOS::SetRevision(wxUint16 val)
{
	m_data.Data()->rev.w = basic->OrderUint16(val);
}

/// 削除
bool DiskBasicDirItemM68FDOS::Delete()
{
	// ファイル名をクリア
	m_data.Data()->name.w[0] = 0;
	m_data.Data()->name.w[1] = 0;
	m_data.Data()->ext.w = 0;
	Used(false);
	// 開始グループを未使用にする
	type->SetGroupNumber(GetStartGroup(0), 0);
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemM68FDOS::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	wxUint16 val = basic->OrderUint16(m_data.Data()->name.w[0]);
	if (val >= 0xed80) {
		return false;
	}
	return true;
}

/// 属性を設定
void DiskBasicDirItemM68FDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	int t2 = 0;
	int t3 = 0;
	wxUint16 rev = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		t1 = file_type.GetOrigin(0);
		t2 = file_type.GetOrigin(1);
		t3 = file_type.GetOrigin(2);
		rev = t3 & 0xffff;
		t3 >>= 16;
	} else {
		t1 = ConvToNativeType(ftype);
	}
	SetFileType1(t1);
	SetFileType2(t2);
	SetFileType3(t3);
	SetRevision(rev);
}

/// 属性を変換
int DiskBasicDirItemM68FDOS::ConvToNativeType(int file_type) const
{
	int val = 0;
	if (file_type & FILE_TYPE_SYSTEM_MASK) {
		val = FILETYPE_M68_FDOS_S;
	}
	if (file_type & FILE_TYPE_HIDDEN_MASK) {
		val = FILETYPE_M68_FDOS_P;
	}
	if (file_type & FILE_TYPE_READONLY_MASK) {
		val = FILETYPE_M68_FDOS_W;
	}
	if (file_type & FILE_TYPE_WRITEONLY_MASK) {
		val = FILETYPE_M68_FDOS_R;
	}
	return val;
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemM68FDOS::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	if (t1 & FILETYPE_M68_FDOS_S) {
		val = FILE_TYPE_SYSTEM_MASK;
	}
	if (t1 & FILETYPE_M68_FDOS_P) {
		val = FILE_TYPE_HIDDEN_MASK;
	}
	if (t1 & FILETYPE_M68_FDOS_R) {
		val = FILE_TYPE_WRITEONLY_MASK;
	}
	if (t1 & FILETYPE_M68_FDOS_W) {
		val = FILE_TYPE_READONLY_MASK;
	}
	int t2 = GetFileType2();
	int t3 = GetFileType3();
	int rev = GetRevision();

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1, t2, ((t3 << 16) | rev));
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemM68FDOS::GetFileAttrStr() const
{
	wxString attr;
	int ftype = GetFileType1();
	for(int i=0; i<TYPE_NAME_M68_FDOS_END; i++) {
		if (ftype & gTypeNameShortM68FDOS[i].value) {
			if (!attr.IsEmpty()) attr += wxT(", ");
			attr += wxGetTranslation(gTypeNameShortM68FDOS[i].name);
		}
	}

	return attr;
}

/// データ内にファイルサイズをセット
void DiskBasicDirItemM68FDOS::SetFileSizeBase(int val)
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
	m_data.Data()->block_size = basic->OrderUint16(sec); 
	m_data.Data()->eof_in_sector = (eof & 0xff);
}

/// データ内のファイルサイズを返す
int DiskBasicDirItemM68FDOS::GetFileSizeBase() const
{
	int sec_size = basic->GetSectorSize();

	int val = basic->OrderUint16(m_data.Data()->block_size);
	int eof = m_data.Data()->eof_in_sector;
	val *= sec_size;
	if (eof > 0) {
		val += eof;
		val -= sec_size;
	}
	return val;
}

/// ファイルサイズを返す
int DiskBasicDirItemM68FDOS::GetFileSize() const
{
	int sec_size = basic->GetSectorSize();
	if (NeedChainInData()) {
		sec_size -= 2;
	}
	int val = basic->OrderUint16(m_data.Data()->block_size);
	int eof = m_data.Data()->eof_in_sector;
	val *= sec_size;
	if (eof > 0) {
		val += eof;
		val -= sec_size;
	}
	return val;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemM68FDOS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// グループ取得計算中処理
/// @note "C"属性ではないとき、セクタ末尾にある次のセクタ番号を得る。
void DiskBasicDirItemM68FDOS::CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data)
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

/// ファイル名をデコード
void DiskBasicDirItemM68FDOS::DecodeName(wxUint16 code, wxUint8 *name, size_t size)
{
	int sta = size > 2 ? 2 : (int)size;
	for(int i=sta; i>=0; i--) {
		int c = (code % 40);
		name[i] = sM68FDOS_CharMap[c];
		code /= 40;
	}
}

/// ファイル名をエンコード
wxUint16 DiskBasicDirItemM68FDOS::EncodeName(const wxUint8 *name, size_t size)
{
	wxUint16 code = 0;
	int fin = size < 3 ? (int)size : 3;
	for(int i=0; i<fin; i++) {
		code *= 40;

		int match = -1;
		for(int c=0; c<40; c++) {
			if (name[i] == sM68FDOS_CharMap[c]) {
				match = c;
				break;
			}
		}
		if (match >= 0) {
			code += (match % 40);
		}
	}
	return code;
}

/// 日付を得る
void DiskBasicDirItemM68FDOS::GetFileCreateDate(TM &tm) const
{
	wxUint16 date = basic->OrderUint16(m_data.Data()->date);

	int yy = (date >> 9);
	if (yy < 70) yy += 100;
	tm.SetYear(yy);
	tm.SetMonth(((date & 0x01e0) >> 5) - 1);
	tm.SetDay(date & 0x001f);
}

/// 日付文字列を得る
wxString DiskBasicDirItemM68FDOS::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 日付をセット
void DiskBasicDirItemM68FDOS::SetFileCreateDate(const TM &tm)
{
	int yy = (tm.GetYear() & 0x007f);
	int mm = ((tm.GetMonth() + 1) & 0x000f);
	int dd = (tm.GetDay() & 0x001f);

	wxUint16 date = ((yy << 9) | (mm << 5) | dd);
	m_data.Data()->date = basic->OrderUint16(date);
}

// 開始アドレスを返す
int DiskBasicDirItemM68FDOS::GetStartAddress() const
{
	return basic->OrderUint16(m_data.Data()->load_addr);
}

/// 終了アドレスを返す
int DiskBasicDirItemM68FDOS::GetEndAddress() const
{
	return GetStartAddress() + GetFileSize() - 1;
}

// 実行アドレスを返す
int DiskBasicDirItemM68FDOS::GetExecuteAddress() const
{
	return basic->OrderUint16(m_data.Data()->exec_addr);
}

/// 開始アドレスをセット
void DiskBasicDirItemM68FDOS::SetStartAddress(int val)
{
	m_data.Data()->load_addr = basic->OrderUint16(val);
}

/// 実行アドレスをセット
void DiskBasicDirItemM68FDOS::SetExecuteAddress(int val)
{
	m_data.Data()->exec_addr = basic->OrderUint16(val);
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemM68FDOS::GetDataSize() const
{
	return sizeof(directory_m68fdos_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemM68FDOS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemM68FDOS::CopyData(const directory_t *val)
{
	return m_data.Copy(val, GetDataSize());
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemM68FDOS::ClearData()
{
	if (!m_data.IsValid()) return;

	m_data.Fill(0);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemM68FDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	wxUint16 start_sector = (val & 0xffff);
	m_data.Data()->start_sector = basic->OrderUint16(start_sector);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemM68FDOS::GetStartGroup(int fileunit_num) const
{
	return basic->OrderUint16(m_data.Data()->start_sector);
}

/// 追加のグループ番号をセット
void DiskBasicDirItemM68FDOS::SetExtraGroup(wxUint32 val)
{
	wxUint16 end_sector = (val & 0xffff);
	m_data.Data()->end_sector = basic->OrderUint16(end_sector);
}

/// 追加のグループ番号を返す
wxUint32 DiskBasicDirItemM68FDOS::GetExtraGroup() const
{
	return basic->OrderUint16(m_data.Data()->end_sector);
}

/// 追加のグループ番号を得る
void DiskBasicDirItemM68FDOS::GetExtraGroups(wxArrayInt &arr) const
{
}

/// データ内部にチェインデータが必要か
bool DiskBasicDirItemM68FDOS::NeedChainInData() const
{
	return ((GetFileType1() & FILETYPE_M68_FDOS_C) == 0);
}

/// データをエクスポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemM68FDOS::PreExportDataFile(wxString &filename)
{
	return true;
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
bool DiskBasicDirItemM68FDOS::PreImportDataFile(wxString &filename)
{
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemM68FDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;

	// 拡張子で属性を設定する
	IsContainAttrByExtension(filename, gExtNameM68FDOS, 0, EXT_NAME_M68_FDOS_END - 1, NULL, &t1, NULL);

	return t1;
}

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

#define IDC_TEXT_REV 51
#define IDC_CHECK_ATTR 53

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemM68FDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
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
void DiskBasicDirItemM68FDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int type1 = GetFileType1();
	int type2 = GetFileType2();

	int file_type_1 = type1;
	int file_type_2 = type2;

	wxCheckBox *chkAttr;
	wxTextCtrl *txtRev;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxSizerFlags expand = wxSizerFlags().Expand();

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxBoxSizer *hbox = NULL;
	for(int i=0; i<TYPE_NAME_M68_FDOS_END; i++) {
		if ((i % 2) == 0) {
			hbox = new wxBoxSizer(wxHORIZONTAL);
			staType1->Add(hbox);
		}
		chkAttr = new wxCheckBox(parent, IDC_CHECK_ATTR + i, wxGetTranslation(gTypeNameM68FDOS[i].name));
		chkAttr->SetValue((file_type_1 & gTypeNameM68FDOS[i].value) != 0);
		hbox->Add(chkAttr, flags);
	}

	sizer->Add(staType1, flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxStaticText(parent, wxID_ANY, _("Revision")), flags);
	txtRev = new wxTextCtrl(parent, IDC_TEXT_REV);
	txtRev->SetValue(GetRevisionStr());
	txtRev->Enable(false);
	hbox->Add(txtRev, expand);
	
	sizer->Add(hbox, flags);
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemM68FDOS::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxCheckBox *chkAttr = NULL;
	int val = 0;
	for(int i=0; i<TYPE_NAME_M68_FDOS_END; i++) {
		chkAttr = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ATTR + i);
		if (!chkAttr) continue;
		if (chkAttr->GetValue()) {
			val |= gTypeNameM68FDOS[i].value;
		}
	}
	return val;
}

/// 属性2を得る
int DiskBasicDirItemM68FDOS::GetFileType2InAttrDialog(const IntNameBox *parent) const
{
	return 0;
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemM68FDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int t1 = GetFileType1InAttrDialog(parent);
	int t2 = GetFileType2();
	int t3 = GetFileType3();
	wxUint16 rev = GetRevision();

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t1, t2, (t3 << 16 | rev));

	return true;
}

/// 属性値を加工する
bool DiskBasicDirItemM68FDOS::ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	return true;
}

/// ファイルサイズが適正か
/// @param [in]  parent     ダイアログ
/// @param [in]  size       ファイルサイズ
/// @param [out] limit      制限サイズ
/// @return true 適正
bool DiskBasicDirItemM68FDOS::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	return true;
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemM68FDOS::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	// 空白はNG
	if (filename.empty()) {
		errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILENAME_EMPTY]);
		return false;
	}
	return true;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemM68FDOS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("NAME"), m_data.Data()->name.b, sizeof(m_data.Data()->name));
	vals.Add(wxT("EXT"), m_data.Data()->ext.b, sizeof(m_data.Data()->ext));
	vals.Add(wxT("ATTR1"), m_data.Data()->attr1, basic->IsBigEndian());
	vals.Add(wxT("ATTR2"), m_data.Data()->attr2, basic->IsBigEndian());
	vals.Add(wxT("BLOCK_SIZE"), m_data.Data()->block_size, basic->IsBigEndian());
	vals.Add(wxT("EOF_IN_SEC"), m_data.Data()->eof_in_sector);
	vals.Add(wxT("DATE"), m_data.Data()->date, basic->IsBigEndian());
	vals.Add(wxT("TIME?"), m_data.Data()->time, basic->IsBigEndian());
	vals.Add(wxT("REV"), m_data.Data()->rev.b,  sizeof(m_data.Data()->rev));
	vals.Add(wxT("START_SECTOR"), m_data.Data()->start_sector, basic->IsBigEndian());
	vals.Add(wxT("ATTR3"), m_data.Data()->attr3);
	vals.Add(wxT("END_SECTOR?"), m_data.Data()->end_sector, basic->IsBigEndian());
	vals.Add(wxT("LOAD_ADDR"), m_data.Data()->load_addr, basic->IsBigEndian());
	vals.Add(wxT("EXEC_ADDR"), m_data.Data()->exec_addr, basic->IsBigEndian());
	vals.Add(wxT("UNKNOWN2"), m_data.Data()->unknown2, sizeof(m_data.Data()->unknown2));
}
