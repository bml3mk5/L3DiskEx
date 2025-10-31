/// @file basicdiritem_trsdos.cpp
///
/// @brief disk basic directory item for Tandy TRSDOS 2.x / 1.3
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_trsdos.h"
//#include <wx/regex.h>
#include "basicfmt.h"
#include "basictype.h"
#include "../config.h"
#include "../charcodes.h"
#include "../utils.h"
#include "basictype_trsdos.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// TRSDOS属性名
const name_value_t gTypeNameTRSDOS[] = {
	{ wxTRANSLATE("Invisible"), FILETYPE_MASK_TRSDOS_INVISIBLE },
	{ wxTRANSLATE("System"), FILETYPE_MASK_TRSDOS_SYSTEM },
	{ wxTRANSLATE("Overflow"), FILETYPE_MASK_TRSDOS_OVERFLOW },
	{ NULL, -1 }
};

/// TRSDOS属性名
const name_value_t gTypeNameTRSDOS2[] = {
	{ "SYS", FILETYPE_MASK_TRSDOS_SYSTEM },
	{ NULL, -1 }
};

//////////////////////////////////////////////////////////////////////
//
// ディレクトリ１アイテム TRSDOS Base
//
DiskBasicDirItemTRSDOS::DiskBasicDirItemTRSDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_position_in_hit = -1;
	next_item = NULL;
}
DiskBasicDirItemTRSDOS::DiskBasicDirItemTRSDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_position_in_hit = -1;
	next_item = NULL;
}
DiskBasicDirItemTRSDOS::DiskBasicDirItemTRSDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_position_in_hit = -1;
	next_item = NULL;
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemTRSDOS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);
	m_position_in_hit = -1;
}

#if 0
/// ディレクトリエントリを確保
/// data  は内部で確保したメモリ
/// sdata がセクタ内部へのポインタとなる
bool DiskBasicDirItemTRSDOS::AllocateItem(const SectorParam *next)
{
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
#endif

#if 0
/// ファイル名を設定
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
void DiskBasicDirItemTRSDOS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		memcpy(n, filename, ns);
	}

	nl = rtrim(n, length, basic->GetDirTerminateCode());
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemTRSDOS::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t s = 0;
	size_t l = 0;

	n = GetFileNamePos(0, s, l);
	if (n != NULL && s > 0) {
		if (s > size) s = size;
		memcpy(filename, n, s);
	}

	length = l;
}

/// ファイル名＋拡張子のサイズ
int DiskBasicDirItemTRSDOS::GetFileNameStrSize() const
{
	size_t s = 0;
	size_t l = 0;
	GetFileNamePos(0, s, l);

	return (int)s;
}
#endif

#if 0
/// 属性２を返す
int	DiskBasicDirItemTRSDOS::GetFileType2() const
{
	return m_data.Data()->overflow;
}

/// 属性２を設定
void DiskBasicDirItemTRSDOS::SetFileType2(int val)
{
	m_data.Data()->overflow = (val & 0xff);
}

/// 属性３を返す ACCESS
int DiskBasicDirItemTRSDOS::GetFileType3() const
{
	return m_data.Data()->access;
}

/// 属性３のセット ACCESS
void DiskBasicDirItemTRSDOS::SetFileType3(int val)
{
	m_data.Data()->access = (val & 0xff);
}

/// AUX_TYPEを返す
int DiskBasicDirItemTRSDOS::GetAuxType() const
{
	return wxUINT16_SWAP_ON_BE(m_data.Data()->f.aux_type);
}

/// AUX_TYPEのセット
void DiskBasicDirItemTRSDOS::SetAuxType(int val)
{
	m_data.Data()->f.aux_type = wxUINT16_SWAP_ON_BE(val & 0xffff);
}
#endif

/// 使用しているアイテムか
bool DiskBasicDirItemTRSDOS::CheckUsed(bool unuse)
{
	bool used = false;
	if (m_position_in_hit >= 0) {
		used = (((DiskBasicTypeTRSDOS *)type)->GetHI(m_position_in_hit) != 0);
	}
	used &= ((GetFileType1() & FILETYPE_MASK_TRSDOS_INUSE) != 0);
	return used;
}

#if 0
/// バージョンを返す(VERSION,MIN_VERSION)
int DiskBasicDirItemTRSDOS::GetVersion() const
{
	return (int)m_data.Data()->version << 8 | m_data.Data()->min_version;
}

/// バージョンをセット(VERSION,MIN_VERSION)
void DiskBasicDirItemTRSDOS::SetVersion(int val)
{
	m_data.Data()->min_version = val & 0xff;
	val >>= 8;
	m_data.Data()->version = val & 0xff;
}

/// 使用ブロック数を返す
int DiskBasicDirItemTRSDOS::GetBlocksUsed() const
{
	return wxUINT16_SWAP_ON_BE(m_data.Data()->blocks_used);
}

/// 使用ブロック数をセット
void DiskBasicDirItemTRSDOS::SetBlocksUsed(int val)
{
	m_data.Data()->blocks_used = wxUINT16_SWAP_ON_BE(val & 0xffff);
}
#endif

/// 削除
bool DiskBasicDirItemTRSDOS::Delete()
{
	// 削除
	Used(false);
	SetFileType1(0);
	// GATのエントリを削除
	type->DeleteGroups(m_groups);
	// HITのエントリも削除
	if (m_position_in_hit >= 0) {
		((DiskBasicTypeTRSDOS *)type)->DeleteHI(m_position_in_hit);
	}
	// Overflowがあるとき
	if (next_item) {
		next_item->Delete();
		next_item = NULL;
	}
	return true;
}

/// 属性を設定
/// @note 固有属性の意味:
void DiskBasicDirItemTRSDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	if (file_type.GetFormat()== basic->GetFormatTypeNumber()) {
		// 同じOSから
		int t1 = file_type.GetOrigin(0);

		SetFileType1(t1);
	} else {
		// 違うOSから
		int t1 = FILETYPE_MASK_TRSDOS_INUSE;

		if (ftype & FILE_TYPE_HIDDEN_MASK) {
			t1 |= FILETYPE_MASK_TRSDOS_INVISIBLE;
		}
		if (ftype & FILE_TYPE_SYSTEM_MASK) {
			t1 |= FILETYPE_MASK_TRSDOS_SYSTEM;
		}

		SetFileType1(t1);
	}
}

/// 属性を返す
/// @note 固有属性の意味:
DiskBasicFileType DiskBasicDirItemTRSDOS::GetFileAttr() const
{
	int val = 0;
	int t1 = GetFileType1();

	if (t1 & FILETYPE_MASK_TRSDOS_INVISIBLE) {
		val |= FILE_TYPE_HIDDEN_MASK;
	}
	if (t1 & FILETYPE_MASK_TRSDOS_SYSTEM) {
		val |= FILE_TYPE_SYSTEM_MASK;
	}

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemTRSDOS::GetFileAttrStr() const
{
	wxString str;
	int val = GetFileType1();
	for(int i=0; gTypeNameTRSDOS[i].name != NULL; i++) {
		if (val & gTypeNameTRSDOS[i].value) {
			if (!str.IsEmpty()) str += wxT(", ");
			str += wxGetTranslation(gTypeNameTRSDOS[i].name);
		}
	}
	return str;
}

/// 属性からリストの位置を返す
int DiskBasicDirItemTRSDOS::ConvFileType1Pos(int type1) const
{
	return 0;
}

#if 0
/// 属性からリストの位置を返す
int DiskBasicDirItemTRSDOS::ConvFileType2Pos(int type2) const
{
	return gTypeNameTRSDOS2[0].IndexOf(gTypeNameTRSDOS2, type2 & 0xff);
}
#endif

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有サイズ
/// @return 計算後のファイルサイズ
int DiskBasicDirItemTRSDOS::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
	return occupied_size;
}

/// ファイル内部のアドレスを取り出す
void DiskBasicDirItemTRSDOS::TakeAddressesInFile(DiskBasicGroups &group_items)
{
	if (group_items.Count() == 0) {
		return;
	}
//	DiskBasicGroupItem *item = &group_items.Item(0);
//	DiskImageSector *sector = basic->GetSector(item->track, item->side, item->sector_start);
//	if (!sector) return;

	// 開始アドレス
//	m_start_address = (int)sector->Get16(0);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemTRSDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	SetGranulesOnGap(0, val, size);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemTRSDOS::GetStartGroup(int fileunit_num) const
{
	return GetGranulesOnGap(0);
}

#if 0
/// グループリストをセット
void DiskBasicDirItemTRSDOS::SetExtraGroups(const DiskBasicGroups &grps)
{
}

/// ディレクトリヘッダのあるグループ番号をセット(機種依存)(HEADER_POINTER)
/// @param [in] val 番号
void DiskBasicDirItemTRSDOS::SetParentGroup(wxUint32 val)
{
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		break;
	default:
		m_data.Data()->f.header_pointer = wxUINT16_SWAP_ON_BE(val);
		break;
	}
}

/// ディレクトリヘッダのあるグループ番号を返す(機種依存)(HEADER_POINTER)
/// @return 番号
wxUint32 DiskBasicDirItemTRSDOS::GetParentGroup() const
{
	wxUint32 val;
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		val = INVALID_GROUP_NUMBER;
		break;
	default:
		val = wxUINT16_SWAP_ON_BE(m_data.Data()->f.header_pointer);
		break;
	}
	return val;
}

/// 追加のグループ番号を返す(機種依存)
wxUint32 DiskBasicDirItemTRSDOS::GetExtraGroup() const
{
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SAPLING:
	case FILETYPE_MASK_PRODOS_TREE:
		return GetStartGroup(0);
		break;
	}
	return INVALID_GROUP_NUMBER;
}

/// 追加のグループ番号を得る(機種依存)
void DiskBasicDirItemTRSDOS::GetExtraGroups(wxArrayInt &arr) const
{
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SAPLING:
	case FILETYPE_MASK_PRODOS_TREE:
		arr.Add((int)GetStartGroup(0));
		break;
	}
}

/// チェイン用のセクタをクリア(機種依存)
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemTRSDOS::ClearChainSector(const DiskBasicDirItem *pitem)
{
	for(size_t i=0; i<m_index.Count(); i++) {
		TRSDOSOneIndex *item = &m_index.Item(i);
		wxUint32 gnum = item->GetMyGroupNumber();
		type->DeleteGroupNumber(gnum);
	}
	m_index.Clear();
	m_index.SetBasic(basic);
}

/// チェイン用のセクタをセット
/// @param [in] num    グループ番号
/// @param [in] pos    セクタ位置
/// @param [in] data   未使用
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemTRSDOS::SetChainSector(wxUint32 num, int pos, wxUint8 *WXUNUSED(data), const DiskBasicDirItem *pitem)
{
	TRSDOSOneIndex item;
	item.AttachBuffer(basic, num, pos);
	m_index.Add(item);
}

/// チェイン用のセクタにグループ番号をセット(機種依存)
void DiskBasicDirItemTRSDOS::AddChainGroupNumber(int idx, wxUint32 val)
{
	m_index.SetGroupNumber(idx, val);
}
#endif

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemTRSDOS::NeedCheckEofCode()
{
	return false;
}

#if 0
/// セーブ時にファイルサイズを再計算する
int DiskBasicDirItemTRSDOS::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	int stype;
	if (file_size < 0x200) {
		// 512バイト未満はインデックスなし
		stype = FILETYPE_MASK_PRODOS_SEEDING;
	} else if (file_size < 0x20000) {
		// 131Kバイト未満はインデックス１つ
		stype = FILETYPE_MASK_PRODOS_SAPLING;
	} else {
		// 131Kバイト以上 ツリー
		stype = FILETYPE_MASK_PRODOS_TREE;
	}
	SetFileType1(stype);

	return file_size;
}
#endif

#if 0
/// データをエクスポートする前に必要な処理
/// 属性に応じて拡張子を付加する
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemTRSDOS::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	return true;
}
#endif

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemTRSDOS::PreImportDataFile(wxString &filename)
{
	// 拡張子前の'.'を'/'に置き換える
	int pos = filename.Find('.', true);
	if (pos != wxNOT_FOUND) {
		filename.SetChar(pos, basic->GetExtensionPreCode());
	}
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemTRSDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;
	// 拡張子で属性を設定する
	IsContainAttrByExtension(filename, gTypeNameTRSDOS2, 0, TYPE_NAME_2_TRSDOS_SYS, NULL, &t1, NULL);

	return t1;
}

/// アイテムを削除できるか
bool DiskBasicDirItemTRSDOS::IsDeletable() const
{
	int stype = GetFileType1();
	return (stype != FILETYPE_MASK_TRSDOS_SYSTEM);
}

#if 0
/// ファイル数を＋１
void DiskBasicDirItemTRSDOS::IncreaseFileCount()
{
	wxUint16 val = 0;
	int stype = GetFileType1();
}

/// ファイル数を－１
void DiskBasicDirItemTRSDOS::DecreaseFileCount()
{
	wxUint16 val = 0;
	int stype = GetFileType1();
}
#endif

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemTRSDOS::SetModify()
{
//	if (m_data.IsSelf()) {
//		m_sdata.CopyFrom((const directory_t *)m_data.Data());
//	}
}

//
// ダイアログ用
//

#include <wx/radiobox.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"
#include "../ui/intnamevalid.h"

#define IDC_CHECK_INVISIBLE	51
#define IDC_CHECK_SYSTEM	52
#define IDC_CHECK_INUSE		53
#define IDC_CHECK_OVERFLOW	54
#define IDC_SPIN_ACCESS		55

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemTRSDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
//		file_type_1 = FILETYPE_MASK_PRODOS_SEEDING << 8	| FILETYPE_MASK_PRODOS_ACCESS_ALL;
//		file_type_2 = ConvOriginalTypeFromFileName(name);
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in,out] sizer      レイアウト
/// @param [in] flags          レイアウトフラグ
void DiskBasicDirItemTRSDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1();
	int file_type_2 = 0;
	wxCheckBox *chkInvisible;
	wxCheckBox *chkSystem;
	wxCheckBox *chkInUse;
	wxCheckBox *chkOverflow;
	wxSpinCtrl *spnAccess;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);
	parent->SetUserData(file_type_1);

	wxSizerFlags expand = wxSizerFlags().Expand();

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	chkInvisible = new wxCheckBox(parent, IDC_CHECK_INVISIBLE, _("Invisible"));
	chkInvisible->SetValue((file_type_1 & FILETYPE_MASK_TRSDOS_INVISIBLE) != 0);
	chkSystem = new wxCheckBox(parent, IDC_CHECK_SYSTEM, _("System"));
	chkSystem->SetValue((file_type_1 & FILETYPE_MASK_TRSDOS_SYSTEM) != 0);
	chkInUse = new wxCheckBox(parent, IDC_CHECK_INUSE, _("In Use"));
	chkInUse->SetValue((file_type_1 & FILETYPE_MASK_TRSDOS_INUSE) != 0);
	chkInUse->Enable(false);
	chkOverflow = new wxCheckBox(parent, IDC_CHECK_OVERFLOW, _("Overflow"));
	chkOverflow->SetValue((file_type_1 & FILETYPE_MASK_TRSDOS_OVERFLOW) != 0);
	chkOverflow->Enable(false);

	hbox->Add(chkInvisible, expand);
	hbox->Add(chkSystem, expand);
	hbox->Add(chkInUse, expand);
	hbox->Add(chkOverflow, expand);
	staType1->Add(hbox, flags);
	sizer->Add(staType1, flags);

	wxSize sz(64, -1);
	wxStaticBoxSizer *staType2 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("Protection Level")), wxVERTICAL);
	hbox = new wxBoxSizer(wxHORIZONTAL);
	spnAccess = new wxSpinCtrl(parent, IDC_SPIN_ACCESS, wxEmptyString, wxDefaultPosition, sz);
	spnAccess->SetRange(0, 7);
	spnAccess->SetValue(file_type_1 & FILETYPE_MASK_TRSDOS_ACCESS);

	hbox->Add(spnAccess, expand);
	staType2->Add(hbox, flags);
	sizer->Add(staType2, flags);
}

/// ダイアログ内の値を設定
void DiskBasicDirItemTRSDOS::InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data)
{
}

#if 0
/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemTRSDOS::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxChoice   *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	if (!comType1) return;
	wxComboBox *comType2 = (wxComboBox *)parent->FindWindow(IDC_COMBO_TYPE2);
	if (!comType2) return;

	int type1pos = comType1->GetSelection();
	int type2pos = comType2->GetSelection();
#if 0
	if (type1pos != TYPE_NAME_PRODOS_VOLUME) {
		if (type2pos == TYPE_NAME_PRODOS_DIR) {
			comType1->SetSelection(TYPE_NAME_PRODOS_SUBDIR);
		} else {
			comType1->SetSelection(TYPE_NAME_PRODOS_FILE);
		}
	}
#endif
}
#endif

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemTRSDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxCheckBox *chkInvisible = (wxCheckBox *)parent->FindWindow(IDC_CHECK_INVISIBLE);
	wxCheckBox *chkSystem = (wxCheckBox *)parent->FindWindow(IDC_CHECK_SYSTEM);
	wxCheckBox *chkInUse = (wxCheckBox *)parent->FindWindow(IDC_CHECK_INUSE);
	wxCheckBox *chkOverflow = (wxCheckBox *)parent->FindWindow(IDC_CHECK_OVERFLOW);
	wxSpinCtrl *spnAccess = (wxSpinCtrl *)parent->FindWindow(IDC_SPIN_ACCESS);

	int val = 0;
	int ori = parent->GetUserData();

	ori &= ~(FILETYPE_MASK_TRSDOS_INVISIBLE
		   | FILETYPE_MASK_TRSDOS_SYSTEM
		   | FILETYPE_MASK_TRSDOS_INUSE
		   | FILETYPE_MASK_TRSDOS_OVERFLOW
		   | 7);

	ori |= (chkInvisible->IsChecked() ? FILETYPE_MASK_TRSDOS_INVISIBLE : 0);
	ori |= (chkSystem->IsChecked() ? FILETYPE_MASK_TRSDOS_SYSTEM : 0);
	ori |= (chkInUse->IsChecked() ? FILETYPE_MASK_TRSDOS_INUSE : 0);
	ori |= (chkOverflow->IsChecked() ? FILETYPE_MASK_TRSDOS_OVERFLOW : 0);
	ori |= (spnAccess->GetValue() & 7);

	attr.SetFileAttr(basic->GetFormatTypeNumber(), val, ori);

	return true;
}

#if 0
/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemTRSDOS::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	return true;
}

/// その他の属性値を設定する
void DiskBasicDirItemTRSDOS::SetOptionalAttr(DiskBasicDirItemAttr &attr)
{
}

/// プロパティで表示する内部データを設定
void DiskBasicDirItemTRSDOS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("ACCESS_CONTROL"), m_data.Data()->access_control);
	vals.Add(wxT("OVERFLOW"), m_data.Data()->overflow);
	vals.Add(wxT("EOF_BYTE_OFFSET"), m_data.Data()->eof_byte_offset);
	vals.Add(wxT("RECORD_LENGTH"), m_data.Data()->record_length);
	vals.Add(wxT("FILE_NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("EXTENSION"), m_data.Data()->ext, sizeof(m_data.Data()->ext));
	vals.Add(wxT("UPDATE_PASSWORD"), m_data.Data()->update_password);
	vals.Add(wxT("ACCESS_PASSWORD"), m_data.Data()->access_password);
	vals.Add(wxT("EOF_SECTOR"), m_data.Data()->eof_sector);
	for(int i=0; i<5; i++) {
		vals.Add(wxString::Format("GAP%d TRACK", i+1), m_data.Data()->gap[i].track);
		vals.Add(wxString::Format("GAP%d GRANULES", i+1), m_data.Data()->gap[i].granules);
	}
}
#endif

//////////////////////////////////////////////////////////////////////
//
// ディレクトリ１アイテム TRSDOS 2.x
//
DiskBasicDirItemTRSD23::DiskBasicDirItemTRSD23(DiskBasic *basic)
	: DiskBasicDirItemTRSDOS(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemTRSD23::DiskBasicDirItemTRSD23(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemTRSDOS(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	if (n_sector) {
		m_position_in_hit = DiskBasicTypeTRSD23::GetHIPosition(n_sector->GetSectorNumber() - basic->GetSectorNumberBase(), n_secpos / sizeof(directory_trsd23_t));
	}
}
DiskBasicDirItemTRSD23::DiskBasicDirItemTRSD23(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemTRSDOS(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);
	m_position_in_hit = DiskBasicTypeTRSD23::GetHIPosition(n_sector->GetSectorNumber() - basic->GetSectorNumberBase(), n_secpos / sizeof(directory_trsd23_t));

	Used(CheckUsed(n_unuse));
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemTRSD23::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemTRSDOS::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
	m_position_in_hit = DiskBasicTypeTRSD23::GetHIPosition(n_sector->GetSectorNumber() - basic->GetSectorNumberBase(), n_secpos / sizeof(directory_trsd23_t));
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemTRSD23::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemTRSD23::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	int ov = GetOverflow();
	if (ov > 0 && ov < 254) {
		// 参照元アイテムと関連付ける
		int ov_sec_num = 0;
		int ov_sec_pos = 0;
		((DiskBasicTypeTRSD23 *)type)->GetFromHIPosition(ov, ov_sec_num, ov_sec_pos);
		// 通し番号を計算
		int num = (ov_sec_num - 2) * basic->GetSectorSize() / GetDataSize() + ov_sec_pos;
		int mnum = basic->GetDirEndSector() - basic->GetDirStartSector() + 1;
		mnum = mnum * basic->GetSectorSize() / GetDataSize();
		if (num >= mnum) {
			// invalid chain
			return false;
		}
	}

	return true;
}

/// 属性１を返す
int	DiskBasicDirItemTRSD23::GetFileType1() const
{
	return (m_data.Data()->access_control);
}

/// 属性１を設定
void DiskBasicDirItemTRSD23::SetFileType1(int val)
{
	m_data.Data()->access_control = (val & 0xff);
}

/// Overflowを返す
wxUint8 DiskBasicDirItemTRSD23::GetOverflow() const
{
	return m_data.Data()->overflow;
}

/// Overflowをセット
void DiskBasicDirItemTRSD23::SetOverflow(wxUint8 val)
{
	m_data.Data()->overflow = (val & 0xff);
}

/// 拡張子を格納する位置を返す
/// @param [out] len    バッファサイズ
/// @return 格納先バッファポインタ
wxUint8 *DiskBasicDirItemTRSD23::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data.Data()->ext);
	return m_data.Data()->ext;
}

/// 新規ファイルとして設定
void DiskBasicDirItemTRSD23::SetAsNewFile()
{
	Used(true);
	SetFileType1(GetFileType1() | FILETYPE_MASK_TRSDOS_INUSE);
	// HITエントリに登録
	wxUint8 h = DiskBasicTypeTRSD23::ComputeHI(m_data.Data()->name);
	if (m_position_in_hit >= 0) {
		((DiskBasicTypeTRSDOS *)type)->SetHI(m_position_in_hit, h);
	}
	// パスワード
	m_data.Data()->access_password = m_data.Data()->update_password = wxUINT16_SWAP_ON_BE(0x4296);

	// エントリのクリア
	for(int pos = 0; pos < (int)(sizeof(m_data.Data()->gap)/sizeof(m_data.Data()->gap[0])); pos++) {
		ClearGranulesOnGap(pos, 0xff, 0xff);
	}
}

/// Overflowファイルとして設定
/// @param[in] position_in_hit 元エントリの位置
/// @param[in] hash_code 元エントリのハッシュコード
void DiskBasicDirItemTRSD23::SetAsOverflowFile(wxUint8 position_in_hit, wxUint8 hash_code)
{
	ClearData();

	Used(true);
	Visible(false);

	SetFileType1(FILETYPE_MASK_TRSDOS_OVERFLOW | FILETYPE_MASK_TRSDOS_INUSE);
	// HITエントリに登録
	if (m_position_in_hit >= 0) {
		((DiskBasicTypeTRSDOS *)type)->SetHI(m_position_in_hit, hash_code);
	}
	SetOverflow(position_in_hit);
//	// パスワード
//	m_data.Data()->access_password = m_data.Data()->update_password = wxUINT16_SWAP_ON_BE(0x4296);

	// エントリのクリア
	for(int pos = 0; pos < (int)(sizeof(m_data.Data()->gap)/sizeof(m_data.Data()->gap[0])); pos++) {
		ClearGranulesOnGap(pos, 0xff, 0xff);
	}
}

/// "BOOT/SYS"として設定
void DiskBasicDirItemTRSD23::SetAsBootSysEntry()
{
	SetFileNameStr("BOOT/SYS");
	SetAsNewFile();
	SetFileType1(FILETYPE_MASK_TRSDOS_SYSTEM | FILETYPE_MASK_TRSDOS_INUSE | FILETYPE_MASK_TRSDOS_INVISIBLE | 6);
	SetStartGroup(0, 1);
	SetFileSize(basic->GetSectorSize());
}

/// "DIR/SYS"として設定
void DiskBasicDirItemTRSD23::SetAsDirSysEntry()
{
	SetFileNameStr("DIR/SYS");
	SetAsNewFile();
	SetFileType1(FILETYPE_MASK_TRSDOS_SYSTEM | FILETYPE_MASK_TRSDOS_INUSE | FILETYPE_MASK_TRSDOS_INVISIBLE | 5);
	SetStartGroup(0, basic->GetManagedTrackNumber() * basic->GetGroupsPerTrack() * basic->GetSidesPerDiskOnBasic(), basic->GetGroupsPerTrack());
	SetFileSize(basic->GetSectorsPerTrack() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorSize());
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemTRSD23::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemTRSD23::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
//	if (!chain.IsValid()) return;

	int calc_groups = 0;
	int calc_file_size = 0;

//	int track_num = 0;
//	int side_num = 0;
//	int sector_num = 0;

	int sector_size = basic->GetSectorSize();
	int block_size = sector_size * basic->GetSectorsPerGroup();
	wxUint32 max_group = basic->GetFatEndGroup();

	int remain_size = GetFileSize();

	for(int pos = 0; pos < (int)(sizeof(m_data.Data()->gap)/sizeof(m_data.Data()->gap[0])); pos++) {
		wxUint32 count = 0;
		wxUint32 group_num = GetGranulesOnGap(pos, &count);
		if (group_num >= max_group) break;

		for(int i = 0; i < (int)count; i++) {
			basic->GetNumsFromGroup(group_num, 0, sector_size, remain_size, group_items);
			group_num++;
			calc_groups++;
			calc_file_size += block_size;
			remain_size -= block_size;
		}
	}
	// overflowがあるとき
	if (next_item) {
		next_item->GetUnitGroups(fileunit_num, group_items);
	}

	group_items.AddNums(calc_groups);
	group_items.AddSize(calc_file_size);
	group_items.SetSizePerGroup(block_size);

	// ファイル内部のアドレスを得る
	TakeAddressesInFile(group_items);
}

/// ファイルサイズをセット
void DiskBasicDirItemTRSD23::SetFileSize(int val)
{
	wxUint16 diva = (val / basic->GetSectorSize());
	int moda = (val % basic->GetSectorSize());
	if (moda) {
		diva++;
	}
	m_data.Data()->eof_sector = wxUINT16_SWAP_ON_BE(diva);
	m_data.Data()->eof_byte_offset = (moda & 0xff);
}

/// ファイルサイズを返す
int DiskBasicDirItemTRSD23::GetFileSize() const
{
	int val = wxUINT16_SWAP_ON_BE(m_data.Data()->eof_sector) * basic->GetSectorSize();
	if (m_data.Data()->eof_byte_offset) {
		val -= basic->GetSectorSize();
		val += m_data.Data()->eof_byte_offset;
	}
	return val;
}

/// GAPのGranule番号をセット
/// @param[in] pos : GAP位置
/// @param[in] val : 開始granule番号
/// @param[in] cnt : 連続したgranule数
void DiskBasicDirItemTRSD23::SetGranulesOnGap(int pos, wxUint32 val, wxUint32 cnt)
{
	int blk = basic->GetGroupsPerTrack() * basic->GetSidesPerDiskOnBasic();
	int trk = val / blk;
	int sta = val % blk;
	m_data.Data()->gap[pos].track = (trk & 0xff);
	m_data.Data()->gap[pos].granules = (((sta << 5) & 0xe0) | ((cnt & 0x1f) - 1));
}

/// GAPのGranule番号をクリア
void DiskBasicDirItemTRSD23::ClearGranulesOnGap(int pos, wxUint32 track, wxUint32 granule)
{
	m_data.Data()->gap[pos].track = (track & 0xff);
	m_data.Data()->gap[pos].granules = (granule & 0xff);
}

/// GAPのGranule番号を返す
/// @param[in] pos : GAP位置
/// @param[out] cnt : 連続したgranule数
/// @return 開始granule番号
wxUint32 DiskBasicDirItemTRSD23::GetGranulesOnGap(int pos, wxUint32 *cnt) const
{
	wxUint32 val = m_data.Data()->gap[pos].track * basic->GetGroupsPerTrack() * basic->GetSidesPerDiskOnBasic();
	wxUint32 sta = ((m_data.Data()->gap[pos].granules & 0xe0) >> 5);
	val += sta;
	if (cnt) {
		*cnt = (m_data.Data()->gap[pos].granules & 0x1f) + 1;
	}
	return val;
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemTRSD23::GetDataSize() const
{
	return sizeof(directory_trsd23_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemTRSD23::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemTRSD23::CopyData(const directory_t *val)
{
	return m_data.Copy(val);
}

/// ディレクトリをクリア
void DiskBasicDirItemTRSD23::ClearData()
{
	m_data.Fill(0);
}

/// プロパティで表示する内部データを設定
void DiskBasicDirItemTRSD23::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("ACCESS_CONTROL"), m_data.Data()->access_control);
	vals.Add(wxT("OVERFLOW"), m_data.Data()->overflow);
	vals.Add(wxT("EOF_BYTE_OFFSET"), m_data.Data()->eof_byte_offset);
	vals.Add(wxT("RECORD_LENGTH"), m_data.Data()->record_length);
	vals.Add(wxT("FILE_NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("EXTENSION"), m_data.Data()->ext, sizeof(m_data.Data()->ext));
	vals.Add(wxT("UPDATE_PASSWORD"), m_data.Data()->update_password);
	vals.Add(wxT("ACCESS_PASSWORD"), m_data.Data()->access_password);
	vals.Add(wxT("EOF_SECTOR"), m_data.Data()->eof_sector);
	for(int i=0; i<(int)(sizeof(m_data.Data()->gap)/sizeof(m_data.Data()->gap[0])); i++) {
		vals.Add(wxString::Format("GAP%d TRACK", i+1), m_data.Data()->gap[i].track);
		vals.Add(wxString::Format("GAP%d GRANULES", i+1), m_data.Data()->gap[i].granules);
	}
}

//////////////////////////////////////////////////////////////////////
//
// ディレクトリ１アイテム TRSDOS 1.3
//
DiskBasicDirItemTRSD13::DiskBasicDirItemTRSD13(DiskBasic *basic)
	: DiskBasicDirItemTRSDOS(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemTRSD13::DiskBasicDirItemTRSD13(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemTRSDOS(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	if (n_sector) {
		int n = (basic->GetSectorSize() / (int)sizeof(directory_trsd13_t));
		m_position_in_hit = DiskBasicTypeTRSD13::GetHIPosition((n_sector->GetSectorNumber() - basic->GetSectorNumberBase() - 2) * n + (n_secpos / (int)sizeof(directory_trsd13_t)));
	}
}
DiskBasicDirItemTRSD13::DiskBasicDirItemTRSD13(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemTRSDOS(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);
	m_position_in_hit = DiskBasicTypeTRSD13::GetHIPosition(n_num);

	Used(CheckUsed(n_unuse));
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemTRSD13::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemTRSDOS::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
	m_position_in_hit = DiskBasicTypeTRSD13::GetHIPosition(n_num);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemTRSD13::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	return true;
}

/// 作成日付を得る
/// @param [out] tm 日付
void DiskBasicDirItemTRSD13::GetFileCreateDate(TM &tm) const
{
	tm.ClearDate();
	int yy = m_data.Data()->year;
	if (yy < 80) {
		yy += 100;
	}
	tm.SetYear(yy);
	tm.SetMonth(m_data.Data()->month - 1);
	tm.SetDay(1);
}

/// 作成日付を返す
/// @return 日付文字列
wxString DiskBasicDirItemTRSD13::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 作成日付をセット
void DiskBasicDirItemTRSD13::SetFileCreateDate(const TM &tm)
{
	m_data.Data()->year = (tm.GetYear() & 0xff);
	m_data.Data()->month = ((tm.GetMonth() + 1) & 0xff);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemTRSD13::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// 拡張子を格納する位置を返す
/// @param [out] len    バッファサイズ
/// @return 格納先バッファポインタ
wxUint8 *DiskBasicDirItemTRSD13::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data.Data()->ext);
	return m_data.Data()->ext;
}

/// 属性１を返す
int	DiskBasicDirItemTRSD13::GetFileType1() const
{
	return (m_data.Data()->access_control);
}

/// 属性１を設定
void DiskBasicDirItemTRSD13::SetFileType1(int val)
{
	m_data.Data()->access_control = (val & 0xff);
}

#if 0
/// Overflowを返す
wxUint8 DiskBasicDirItemTRSD13::GetOverflow() const
{
	return m_data.Data()->overflow;
}

/// Overflowをセット
void DiskBasicDirItemTRSD13::SetOverflow(wxUint8 val)
{
	m_data.Data()->overflow = (val & 0xff);
}
#endif

/// 新規ファイルとして設定
void DiskBasicDirItemTRSD13::SetAsNewFile()
{
	Used(true);
	SetFileType1(GetFileType1() | FILETYPE_MASK_TRSDOS_INUSE);
	// HITエントリに登録
	wxUint8 h = DiskBasicTypeTRSD23::ComputeHI(m_data.Data()->name);
	if (m_position_in_hit >= 0) {
		((DiskBasicTypeTRSDOS *)type)->SetHI(m_position_in_hit, h);
	}
	// パスワード
	m_data.Data()->access_password = m_data.Data()->update_password = wxUINT16_SWAP_ON_BE(0x5cef);

	// エントリのクリア
	for(int pos = 0; pos < (int)(sizeof(m_data.Data()->gap)/sizeof(m_data.Data()->gap[0])); pos++) {
		ClearGranulesOnGap(pos, 0xff, 0xff);
	}
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemTRSD13::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemTRSD13::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
//	if (!chain.IsValid()) return;

	int calc_groups = 0;
	int calc_file_size = 0;

//	int track_num = 0;
//	int side_num = 0;
//	int sector_num = 0;

	int sector_size = basic->GetSectorSize();
	int block_size = sector_size * basic->GetSectorsPerGroup();
	wxUint32 max_group = basic->GetFatEndGroup();

	int remain_size = GetFileSize();

	for(int pos = 0; pos < (int)(sizeof(m_data.Data()->gap)/sizeof(m_data.Data()->gap[0])); pos++) {
		wxUint32 count = 0;
		wxUint32 group_num = GetGranulesOnGap(pos, &count);
		if (group_num >= max_group) break;

		for(int i = 0; i < (int)count; i++) {
			basic->GetNumsFromGroup(group_num, 0, sector_size, remain_size, group_items);
			group_num++;
			calc_groups++;
			calc_file_size += block_size;
			remain_size -= block_size;
		}
	}
	// overflowがあるとき
	if (next_item) {
		next_item->GetUnitGroups(fileunit_num, group_items);
	}

	group_items.AddNums(calc_groups);
	group_items.AddSize(calc_file_size);
	group_items.SetSizePerGroup(block_size);

	// ファイル内部のアドレスを得る
	TakeAddressesInFile(group_items);
}

/// ファイルサイズをセット
void DiskBasicDirItemTRSD13::SetFileSize(int val)
{
	wxUint16 diva = (val >> 8);
	m_data.Data()->eof_sector = wxUINT16_SWAP_ON_BE(diva);
	m_data.Data()->eof_byte_offset = (val & 0xff);
}

/// ファイルサイズを返す
int DiskBasicDirItemTRSD13::GetFileSize() const
{
	int val = wxUINT16_SWAP_ON_BE(m_data.Data()->eof_sector);
	val <<= 8;
	val |= m_data.Data()->eof_byte_offset;
	return val;
}

/// GAPのGranule番号をセット
/// @param[in] pos : GAP位置
/// @param[in] val : 開始granule番号
/// @param[in] cnt : 連続したgranule数
void DiskBasicDirItemTRSD13::SetGranulesOnGap(int pos, wxUint32 val, wxUint32 cnt)
{
	int blk = basic->GetGroupsPerTrack() * basic->GetSidesPerDiskOnBasic();
	int trk = val / blk;
	int sta = val % blk;
	m_data.Data()->gap[pos].track = (trk & 0xff);
	m_data.Data()->gap[pos].granules = (((sta << 5) & 0xe0) | (cnt & 0x1f));
}

/// GAPのGranule番号をクリア
void DiskBasicDirItemTRSD13::ClearGranulesOnGap(int pos, wxUint32 track, wxUint32 granule)
{
	m_data.Data()->gap[pos].track = (track & 0xff);
	m_data.Data()->gap[pos].granules = (granule & 0xff);
}

/// GAPのGranule番号を返す
/// @param[in] pos : GAP位置
/// @param[out] cnt : 連続したgranule数
/// @return 開始granule番号
wxUint32 DiskBasicDirItemTRSD13::GetGranulesOnGap(int pos, wxUint32 *cnt) const
{
	wxUint32 val = m_data.Data()->gap[pos].track * basic->GetGroupsPerTrack() * basic->GetSidesPerDiskOnBasic();
	wxUint32 sta = ((m_data.Data()->gap[pos].granules & 0xe0) >> 5);
	val += sta;
	if (cnt) {
		*cnt = (m_data.Data()->gap[pos].granules & 0x1f);
	}
	return val;
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemTRSD13::GetDataSize() const
{
	return sizeof(directory_trsd13_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemTRSD13::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemTRSD13::CopyData(const directory_t *val)
{
	return m_data.Copy(val);
}

/// ディレクトリをクリア
void DiskBasicDirItemTRSD13::ClearData()
{
	m_data.Fill(0);
}

/// プロパティで表示する内部データを設定
void DiskBasicDirItemTRSD13::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("ACCESS_CONTROL"), m_data.Data()->access_control);
	vals.Add(wxT("MONTH"), m_data.Data()->month);
	vals.Add(wxT("YEAR"), m_data.Data()->year);
	vals.Add(wxT("EOF_BYTE_OFFSET"), m_data.Data()->eof_byte_offset);
	vals.Add(wxT("RECORD_LENGTH"), m_data.Data()->record_length);
	vals.Add(wxT("FILE_NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("EXTENSION"), m_data.Data()->ext, sizeof(m_data.Data()->ext));
	vals.Add(wxT("UPDATE_PASSWORD"), m_data.Data()->update_password);
	vals.Add(wxT("ACCESS_PASSWORD"), m_data.Data()->access_password);
	vals.Add(wxT("EOF_SECTOR"), m_data.Data()->eof_sector);
	for(int i=0; i<(int)(sizeof(m_data.Data()->gap)/sizeof(m_data.Data()->gap[0])); i++) {
		vals.Add(wxString::Format("GAP%d TRACK", i+1), m_data.Data()->gap[i].track);
		vals.Add(wxString::Format("GAP%d GRANULES", i+1), m_data.Data()->gap[i].granules);
	}
}
