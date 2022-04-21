/// @file basicdiritem_frost.cpp
///
/// @brief disk basic directory item for Frost-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_frost.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "config.h"
#include <wx/msgdlg.h>


//
//
//

/// Frost-DOS 属性名
const char *gTypeNameFROST_1[] = {
	"BAS",
	"BIN",
	"RGB",
	"???",
	NULL
};

//
//
//
DiskBasicDirItemFROST::DiskBasicDirItemFROST(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemFROST::DiskBasicDirItemFROST(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, secpos, data)
{
}
DiskBasicDirItemFROST::DiskBasicDirItemFROST(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	Used(CheckUsed(unuse));
	unuse = (unuse || (m_data->frost.name[0] == 0xff));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemFROST::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data->frost.name);
		return m_data->frost.name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemFROST::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data->frost.ext);
	return m_data->frost.ext;
}

/// 属性１を返す
int	DiskBasicDirItemFROST::GetFileType1() const
{
	return m_data->frost.type;
}

/// 属性１の文字列
wxString DiskBasicDirItemFROST::ConvFileType1Str(int t1) const
{
	return wxGetTranslation(gTypeNameFROST_1[ConvFileType1Pos(t1)]);
}

/// 属性１を設定
void DiskBasicDirItemFROST::SetFileType1(int val)
{
	m_data->frost.type = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemFROST::CheckUsed(bool unuse)
{
	return (!unuse && this->m_data->frost.name[0] != 0 && this->m_data->frost.name[0] != 0xff);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemFROST::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;
	if (m_data->frost.name[0] == 0xff) {
		last = true;
		return valid;
	}
	// 属性に不正な値がある
	if (GetFileType1() & 0x0c) {
		valid = false;
	}
	return valid;
}

/// 属性を設定
/// @param [in] file_type
void DiskBasicDirItemFROST::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		// 同じOS
		t1 = file_type.GetOrigin();
	} else {
		// 違うOS
		if (ftype & FILE_TYPE_MACHINE_MASK) {
			t1 = FILETYPE_FROST_BIN;
		} else if (ftype & FILE_TYPE_DATA_MASK) {
			t1 = FILETYPE_FROST_RGB;
		} else {
			t1 = FILETYPE_FROST_BAS;
		}
	}
	SetFileType1(t1);

	// BASのときは開始アドレスを1にする
	if (GetFileType1() == FILETYPE_FROST_BAS) {
		SetStartAddress(1);
	}
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemFROST::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	switch (t1) {
	case FILETYPE_FROST_BIN:
		val = FILE_TYPE_MACHINE_MASK;		// machine
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	case FILETYPE_FROST_RGB:
		val = FILE_TYPE_DATA_MASK;			// data
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	default:
		val = FILE_TYPE_BASIC_MASK;			// basic
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	}

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemFROST::GetFileAttrStr() const
{
	wxString attr = ConvFileType1Str(GetFileType1());
	return attr;
}

/// ファイルサイズをセット
void DiskBasicDirItemFROST::SetFileSize(int val)
{
	m_data->frost.size = basic->OrderUint16(val);
	m_groups.SetSize(val);
}

/// ファイルサイズを返す
int DiskBasicDirItemFROST::GetFileSize() const
{
	int val = basic->OrderUint16(m_data->frost.size);
	if (val == 0) val = (int)m_groups.GetSize();
	return val;
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemFROST::GetDataSize() const
{
	return sizeof(directory_frost_t);
}

/// ファイルサイズを計算
void DiskBasicDirItemFROST::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemFROST::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	int calc_file_size = 0;
	int calc_groups = 0; 

	// 16bit FAT (track & sector)
	bool rc = true;
	wxUint32 group_num = GetStartGroup(fileunit_num);
	bool working = true;
	int limit = basic->GetFatEndGroup() + 1;
	while(working) {
		wxUint32 next_group = type->GetGroupNumber(group_num);
		if (next_group == group_num) {
			// 同じポジションならエラー
			rc = false;
		} else if (next_group >= basic->GetGroupSystemCode()) {
			// システム領域はエラー(0xfefe)
			rc = false;
		} else if (next_group == basic->GetGroupFinalCode()) {
			// 最終グループ(0xfdfd)
			AddGroups(group_num, next_group, group_items);
			calc_file_size += (basic->GetSectorSize() / basic->GetGroupsPerSector());
			calc_groups++;
			working = false;
		} else if (next_group > basic->GetFatEndGroup()) {
			// グループ番号がおかしい
			rc = false;
		} else {
			AddGroups(group_num, next_group, group_items);
			calc_file_size += (basic->GetSectorSize() / basic->GetGroupsPerSector());
			calc_groups++;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}

	int inter_file_size = GetFileSize();
	if (inter_file_size == 0) {
		inter_file_size = calc_file_size;
	}
	group_items.SetNums(calc_groups);
	group_items.SetSize(inter_file_size);
	group_items.SetSizePerGroup(FROST_GROUP_SIZE);

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
}

/// グループを追加する
void DiskBasicDirItemFROST::AddGroups(wxUint32 group_num, wxUint32 next_group, DiskBasicGroups &group_items)
{
	int trk, sid, sec, div, divs;
	trk = sid = sec = -1;
	basic->CalcNumFromSectorPosForGroup(group_num, trk, sid, sec, &div, &divs);
	group_items.Add(group_num, next_group, trk, sid, sec, sec, div, divs);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemFROST::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	m_data->frost.track  = (val /  basic->GetGroupsPerTrack());
	m_data->frost.sector = (val %  basic->GetGroupsPerTrack()) + 1;
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemFROST::GetStartGroup(int fileunit_num) const
{
	return (wxUint32)m_data->frost.track * basic->GetGroupsPerTrack() + m_data->frost.sector - 1;
}

/// 開始アドレスを返す
int DiskBasicDirItemFROST::GetStartAddress() const
{
	return basic->OrderUint16(m_data->frost.load_addr);
}

/// 開始アドレスをセット
void DiskBasicDirItemFROST::SetStartAddress(int val)
{
	m_data->frost.load_addr = basic->OrderUint16(val);
}

/// ENDマークがあるか(一度も使用していないか)
bool DiskBasicDirItemFROST::HasEndMark()
{
	bool val = false;
	val = ((wxUint32)m_data->frost.name[0] == basic->GetGroupUnusedCode());
	return val;
}

/// 次のアイテムにENDマークを入れる
void DiskBasicDirItemFROST::SetEndMark(DiskBasicDirItem *next_item)
{
	if (!next_item) return;

	if (HasEndMark()) next_item->Delete(basic->GetGroupUnusedCode());
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemFROST::NeedCheckEofCode()
{
	// Asc形式のときはEOFコードが必要
	return false; //(((GetFileType1() & (FILETYPE_FROST_MACHINE | FILETYPE_FROST_BINARY)) == 0) && (external_attr == 0));
}

/// ディレクトリをクリア
void DiskBasicDirItemFROST::ClearData()
{
	if (!m_data) return;
	int c = basic->GetDeleteCode();
	size_t l;
	l = GetDataSize();
	memset(m_data, c, l);
}

/// データをエクスポートする前に必要な処理
/// 拡張子を付加する
bool DiskBasicDirItemFROST::PreExportDataFile(wxString &filename)
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
bool DiskBasicDirItemFROST::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		IsContainAttrByExtension(filename, gTypeNameFROST_1, TYPE_NAME_FROST_BAS, TYPE_NAME_FROST_RGB, &filename, NULL);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemFROST::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = TYPE_NAME_FROST_BIN;
	// 拡張子で属性を設定する
	IsContainAttrByExtension(filename, gTypeNameFROST_1, TYPE_NAME_FROST_BAS, TYPE_NAME_FROST_RGB, NULL, &t1);
	return t1;
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
int DiskBasicDirItemFROST::ConvFileType1Pos(int t1) const
{
	if (t1 < 0 || t1 > TYPE_NAME_FROST_UNKNOWN) {
		t1 = TYPE_NAME_FROST_UNKNOWN;
	}
	return t1;
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFROST::GetFileType2Pos() const
{
	return GetFileAttr().GetType();
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemFROST::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = ConvOriginalTypeFromFileName(name);
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemFROST::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = ConvFileType1Pos(GetFileType1());
	int file_type_2 = GetFileType2Pos();
	wxRadioBox *radType1;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString types1;
	for(size_t i=0; gTypeNameFROST_1[i] != NULL; i++) {
		types1.Add(wxGetTranslation(gTypeNameFROST_1[i]));
	}
	radType1 = new wxRadioBox(parent, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 0, wxRA_SPECIFY_COLS);
	radType1->SetSelection(file_type_1);
	sizer->Add(radType1, flags);
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemFROST::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);

	int t1 = radType1->GetSelection();
	if (t1 < 0) t1 = FILETYPE_FROST_BAS;

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t1);

	return true;
}

/// 属性値を加工する
bool DiskBasicDirItemFROST::ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int t1 = attr.GetFileOriginAttr();

	// BASの場合、開始アドレスを1にする
	if (t1 == FILETYPE_FROST_BAS) {
		attr.SetStartAddress(1);
	}

	return true;
}
