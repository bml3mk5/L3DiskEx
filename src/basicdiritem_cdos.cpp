/// @file basicdiritem_cdos.cpp
///
/// @brief disk basic directory item for C-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_cdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "config.h"
#include "utils.h"


//
//
//

/// 属性名
const char *gTypeNameCDOS[] = {
	"???",
	"OBJ",
	"TEX",
	"CMD",
	"REL",
	NULL
};

const char *gTypeNameCDOS2[] = {
	wxTRANSLATE("Write Protected"),
	NULL
};

///
///
///
DiskBasicDirItemCDOS::DiskBasicDirItemCDOS(DiskBasic *basic)
	: DiskBasicDirItemMZBase(basic)
{
}
DiskBasicDirItemCDOS::DiskBasicDirItemCDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItemMZBase(basic, sector, secpos, data)
{
}
DiskBasicDirItemCDOS::DiskBasicDirItemCDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemMZBase(basic, num, track, side, sector, secpos, data, unuse)
{
	Used(CheckUsed(unuse));
	if (GetFileType1() == 0xfe) {
		// IPL部分は表示しない
		Visible(false);
	}

	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemCDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = sizeof(m_data->cdos.name);
		len = size - 1;
		return m_data->cdos.name; 
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// 属性１を返す
int	DiskBasicDirItemCDOS::GetFileType1() const
{
	return basic->InvertUint8(m_data->cdos.type);	// invert;
}

/// 属性２を返す
int	DiskBasicDirItemCDOS::GetFileType2() const
{
	return basic->InvertUint8(m_data->cdos.type2);	// invert;
}

/// 属性３を返す
int	DiskBasicDirItemCDOS::GetFileType3() const
{
	return basic->InvertUint8(m_data->cdos.byte_order);	// invert;
}

/// 属性１を設定
void DiskBasicDirItemCDOS::SetFileType1(int val)
{
	m_data->cdos.type = basic->InvertUint8(val);	// invert
}

/// 属性２を設定
void DiskBasicDirItemCDOS::SetFileType2(int val)
{
	m_data->cdos.type2 = basic->InvertUint8(val);	// invert
}

/// 属性３を設定
void DiskBasicDirItemCDOS::SetFileType3(int val)
{
	m_data->cdos.byte_order = basic->InvertUint8(val);	// invert
}

/// 使用しているアイテムか
bool DiskBasicDirItemCDOS::CheckUsed(bool unuse)
{
	return (GetFileType1() != 0);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemCDOS::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;
	wxUint8 t = GetFileType1();
	if (m_num != 0 && (t & 0x70) != 0 && basic->GetSpecialAttributes().FindValue(t) == NULL) {
		valid = false;
	}
	return valid;
}

/// 属性をセット
void DiskBasicDirItemCDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	int t2 = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		// 同じフォーマット
		t1 = file_type.GetOrigin() & 0xff;
		t2 = (file_type.GetOrigin() >> 8) & 0xff;
	} else {
		// 異なるフォーマット
		t1 = ConvToNativeType(ftype);
		if (ftype & FILE_TYPE_READONLY_MASK) {
			t2 |= DATATYPE_CDOS_READ_ONLY;
		}
	}
	SetFileType1(t1);
	SetFileType2(t2);
	SetFileType3(basic->IsBigEndian() ? 1 : 0);
}

/// 属性を変換
int DiskBasicDirItemCDOS::ConvToNativeType(int file_type) const
{
	int val = 0;
	if (file_type & FILE_TYPE_SYSTEM_MASK) {
		val = FILETYPE_CDOS_SYS;
	} else if (file_type & FILE_TYPE_MACHINE_MASK) {
		val = FILETYPE_CDOS_CMD;
	} else if (file_type & FILE_TYPE_BINARY_MASK) {
		val = FILETYPE_CDOS_OBJ;
	} else if (file_type & FILE_TYPE_ASCII_MASK) {
		val = FILETYPE_CDOS_TEX;
	}
	return val;
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemCDOS::ClearData()
{
	if (!m_data) return;
	memset(m_data, 0, sizeof(directory_cdos_t));
	memset(m_data->cdos.name, 0x0d, sizeof(m_data->cdos.name));
	basic->InvertMem(m_data, sizeof(directory_cdos_t));	// invert
}

DiskBasicFileType DiskBasicDirItemCDOS::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	switch(t1) {
	case FILETYPE_CDOS_OBJ:
		val = FILE_TYPE_DATA_MASK;		// DATA
		val |= FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_CDOS_TEX:
		val = FILE_TYPE_DATA_MASK;		// DATA
		val |= FILE_TYPE_ASCII_MASK;	// ascii
		break;
	case FILETYPE_CDOS_CMD:
		val = FILE_TYPE_MACHINE_MASK;	// machine
		val |= FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_CDOS_SYS:
		val = FILE_TYPE_SYSTEM_MASK;

		break;
	default:
		val = basic->GetSpecialAttributes().GetTypeByValue(t1);
		break;
	}
	int t2 = GetFileType2();
	if (t2 & DATATYPE_CDOS_READ_ONLY) {
		// write protect
		val |= FILE_TYPE_READONLY_MASK;
	}

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t2 << 8 | t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemCDOS::GetFileAttrStr() const
{
	wxString attr; 
	GetFileAttrName(ConvFileType1Pos(GetFileType1()), gTypeNameCDOS, TYPE_NAME_CDOS_UNKNOWN, attr);

	int t2 = GetFileType2();
	if (t2 & DATATYPE_CDOS_READ_ONLY) {
		// write protect
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameCDOS2[TYPE_NAME_CDOS2_READ_ONLY]);
	}

	return attr;
}

/// データ内にファイルサイズをセット
void DiskBasicDirItemCDOS::SetFileSizeBase(int val)
{
	m_data->cdos.file_size = basic->InvertAndOrderUint16(val);	// invert
}

/// データ内のファイルサイズを返す
int DiskBasicDirItemCDOS::GetFileSizeBase() const
{
	return basic->InvertAndOrderUint16(m_data->cdos.file_size);	// invert
}

void DiskBasicDirItemCDOS::GetFileDate(struct tm *tm) const
{
	Utils::ConvYYMMDDToTm(
		basic->InvertUint8(m_data->cdos.yy),
		basic->InvertUint8(m_data->cdos.mm),
		basic->InvertUint8(m_data->cdos.dd),
		tm);
}

wxString DiskBasicDirItemCDOS::GetFileDateStr() const
{
	struct tm tm;
	GetFileDate(&tm);
	return Utils::FormatYMDStr(&tm);
}

void DiskBasicDirItemCDOS::SetFileDate(const struct tm *tm)
{
	if (tm->tm_year < 0 || tm->tm_mon < -1) return;

	wxUint8 yy, mm, dd;
	Utils::ConvTmToYYMMDD(tm, yy, mm, dd);
	m_data->cdos.yy = basic->InvertUint8(yy & 0xff);
	m_data->cdos.mm = basic->InvertUint8(mm & 0xff);
	m_data->cdos.dd = basic->InvertUint8(dd & 0xff);
}

// 開始アドレスを返す
int DiskBasicDirItemCDOS::GetStartAddress() const
{
	return basic->InvertAndOrderUint16(m_data->cdos.load_addr);	// invert and byte order
}

// 実行アドレスを返す
int DiskBasicDirItemCDOS::GetExecuteAddress() const
{
	return basic->InvertAndOrderUint16(m_data->cdos.exec_addr);	// invert and byte order
}

/// 開始アドレスをセット
void DiskBasicDirItemCDOS::SetStartAddress(int val)
{
	m_data->cdos.load_addr = basic->InvertAndOrderUint16(val);	// invert and byte order
}

/// 実行アドレスをセット
void DiskBasicDirItemCDOS::SetExecuteAddress(int val)
{
	m_data->cdos.exec_addr = basic->InvertAndOrderUint16(val);	// invert and byte order
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemCDOS::GetDataSize() const
{
	return sizeof(directory_cdos_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemCDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	wxUint32 trk = val / basic->GetSectorsPerTrackOnBasic();
	wxUint32 sec = val % basic->GetSectorsPerTrackOnBasic();
	m_data->cdos.track  = basic->InvertUint8(trk);
	m_data->cdos.sector = basic->InvertUint8(sec + 1);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemCDOS::GetStartGroup(int fileunit_num) const
{
	wxUint32 val = basic->InvertUint8(m_data->cdos.track);
	val *= basic->GetSectorsPerTrackOnBasic();
	val += basic->InvertUint8(m_data->cdos.sector) - 1;
	return val;
}

/// データをエクスポートする前に必要な処理
bool DiskBasicDirItemCDOS::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	wxString ext;
	if (GetFileAttrName(ConvFileType1Pos(GetFileType1()), gTypeNameCDOS, TYPE_NAME_CDOS_UNKNOWN, ext)) {
		filename += wxT(".");
		if (Utils::IsUpperString(filename)) {
			filename += ext.Upper();
		} else {
			filename += ext.Lower();
		}
	}
	return true;
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
bool DiskBasicDirItemCDOS::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		IsContainAttrByExtension(filename, gTypeNameCDOS, TYPE_NAME_CDOS_OBJ, TYPE_NAME_CDOS_SYS, &filename, NULL);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemCDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	// 拡張子で属性を設定する
	int t1 = TYPE_NAME_CDOS_TEX;
	IsContainAttrByExtension(filename, gTypeNameCDOS, TYPE_NAME_CDOS_OBJ, TYPE_NAME_CDOS_SYS, NULL, &t1);
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
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_COMBO_TYPE1 51
#define IDC_CHECK_READONLY 52

/// 属性からリストの位置を返す(プロパティダイアログ用)
/// @return 位置不明の属性はマイナスにする
int DiskBasicDirItemCDOS::ConvFileType1Pos(int native_type) const
{
	int pos = TYPE_NAME_CDOS_UNKNOWN;
	switch(native_type) {
	case FILETYPE_CDOS_OBJ:
		pos = TYPE_NAME_CDOS_OBJ;
		break;
	case FILETYPE_CDOS_TEX:
		pos = TYPE_NAME_CDOS_TEX;
		break;
	case FILETYPE_CDOS_CMD:
		pos = TYPE_NAME_CDOS_CMD;
		break;
	case FILETYPE_CDOS_SYS:
		pos = TYPE_NAME_CDOS_SYS;
		break;
	default:
		pos = -native_type;
		break;
	}
	return pos;
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemCDOS::ConvFileType2Pos(int native_type) const
{
	int val = 0;
	if (native_type & DATATYPE_CDOS_READ_ONLY) {
		// write protect
		val |= FILE_TYPE_READONLY_MASK;
	}
	return val;
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemCDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
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
void DiskBasicDirItemCDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int type1 = GetFileType1();
	int type2 = GetFileType2();

	parent->SetUserData(type2 << 8 | type1);

	int file_type_1 = ConvFileType1Pos(type1);
	int file_type_2 = ConvFileType2Pos(type2);

	wxChoice   *comType1;
	wxCheckBox *chkReadOnly;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);

	wxArrayString types1;
	CreateChoiceForAttrDialog(basic, gTypeNameCDOS, TYPE_NAME_CDOS_END, types1);

	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	file_type_1 = SelectChoiceForAttrDialog(basic, comType1, file_type_1, TYPE_NAME_CDOS_END, TYPE_NAME_CDOS_UNKNOWN);
	staType1->Add(comType1, flags);
	sizer->Add(staType1, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, _("Write Protect"));
	chkReadOnly->SetValue((file_type_2 & FILE_TYPE_READONLY_MASK) != 0);
	staType4->Add(chkReadOnly, flags);
	sizer->Add(staType4, flags);

//	// event handler
//	parent->Bind(wxEVT_CHOICE, &IntNameBox::OnChangeType1, parent, IDC_COMBO_TYPE1);

}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemCDOS::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);

	return comType1->GetSelection();
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int	DiskBasicDirItemCDOS::CalcFileTypeFromPos(int pos) const
{
	int val = 0;
	switch(pos) {
	case TYPE_NAME_CDOS_OBJ:
		val = FILETYPE_CDOS_OBJ;
		break;
	case TYPE_NAME_CDOS_TEX:
		val = FILETYPE_CDOS_TEX;
		break;
	case TYPE_NAME_CDOS_CMD:
		val = FILETYPE_CDOS_CMD;
		break;
	case TYPE_NAME_CDOS_SYS:
		val = FILETYPE_CDOS_SYS;
		break;
	default:
		val = CalcSpecialOriginalTypeFromPos(basic, pos, TYPE_NAME_CDOS_END);
		break;
	}
	return val;
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemCDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);

	int t1 = CalcFileTypeFromPos(GetFileType1InAttrDialog(parent));
	if (t1 < 0) {
		t1 = parent->GetUserData() & 0xff;
	}
	int t2 = 0;
	t2 |= chkReadOnly->GetValue() ? DATATYPE_CDOS_READ_ONLY : 0;

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t2 << 8 | t1);

	return true;
}

/// ファイルサイズが適正か
/// @param [in]  parent     ダイアログ
/// @param [in]  size       ファイルサイズ
/// @param [out] limit      制限サイズ
/// @return true 適正
bool DiskBasicDirItemCDOS::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	int limit_size = 0xffff;
	if (limit) *limit = limit_size;
	return (size <= limit_size);
}
