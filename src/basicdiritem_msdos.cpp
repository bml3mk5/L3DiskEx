/// @file basicdiritem_msdos.cpp
///
/// @brief disk basic directory item for MS-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_msdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "utils.h"


//
//
//

/// MS-DOS (MSX-DOS)
const char *gTypeNameMS[] = {
	wxTRANSLATE("Read Only"),
	wxTRANSLATE("Hidden"),
	wxTRANSLATE("Sys"),
	wxTRANSLATE("<VOL>"),
	wxTRANSLATE("<DIR>"),
	wxTRANSLATE("Arc"),
	wxTRANSLATE("(LFN)"),
	NULL
};
/// MS-DOS (MSX-DOS)
const char *gTypeNameMS_l[] = {
	wxTRANSLATE("Read Only"),
	wxTRANSLATE("Hidden"),
	wxTRANSLATE("System"),
	wxTRANSLATE("Volume Label"),
	wxTRANSLATE("Directory"),
	wxTRANSLATE("Archive"),
	wxTRANSLATE("Long File Name"),
	NULL
};

///
///
///
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, secpos, data)
{
}
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	// MS-DOS
	Used(CheckUsed(unuse));
	Visible((GetFileType1() & FILETYPE_MASK_MS_LFN) != FILETYPE_MASK_MS_LFN);
	unuse = (unuse || (m_data->msdos.name[0] == 0));

	// グループ数を計算
	CalcFileSize();

	// カレント or 親ディレクトリはツリーに表示しない
	wxString name = GetFileNamePlainStr();
	VisibleOnTree(!(IsDirectory() && (name == wxT(".") || name == wxT(".."))));
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemMSDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// MS-DOS
	int t1 = GetFileType1();
	if (num == 0) {
		size = len = sizeof(m_data->msdos.name);
		return m_data->msdos.name;
	} else if (num == 1) {
		if (t1 & FILETYPE_MASK_MS_VOLUME) {
			// ボリュームラベルは拡張子もラベル名とする
			size = len = sizeof(m_data->msdos.ext);
			return m_data->msdos.ext;
		}
	}
	size = len = 0;
	return NULL;
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemMSDOS::GetFileExtPos(size_t &len) const
{
	wxUint8 *p = NULL;
	len = 0;
	// ボリュームラベルは拡張子なしにする
	if (!(GetFileType1() & FILETYPE_MASK_MS_VOLUME)) {
		len = sizeof(m_data->msdos.ext);
		p = m_data->msdos.ext;
	}
	return p;
}

/// 属性１を返す
int	DiskBasicDirItemMSDOS::GetFileType1() const
{
	return m_data->msdos.type;
}

/// 属性１を設定
void DiskBasicDirItemMSDOS::SetFileType1(int val)
{
	m_data->msdos.type = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemMSDOS::CheckUsed(bool unuse)
{
	return (m_data->msdos.name[0] != 0 && m_data->msdos.name[0] != 0xe5);
}

/// ファイル名を設定
void DiskBasicDirItemMSDOS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	if (length > 0) {
		// 0xe5は削除コードなので0x05に変換(Sjift JISなど2バイト系文字など)
		if (filename[0] == 0xe5) filename[0] = 0x05;
	}

	wxUint8 *n;
	size_t nl = 0;
	size_t s, l;

	int num = 0;
	do {
		s = l = 0;
		n = GetFileNamePos(num, s, l);
		if (n == NULL || s == 0) {
			break;
		}
		if (num > 0) {
			size_t pl = nl;
			size_t ps = s;
			if (nl < length) {
				pl = length;
				ps = s + nl - length;
			}
			if (s + nl > length) {
				memset(&filename[pl], 0, ps);
			}
		}

		if (s > size) s = size;
		memcpy(n, &filename[nl], s);

		nl += s;
		size -= s;
		num++;
	} while(num <= 1);
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemMSDOS::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t nl = 0;
	int num = 0;
	do {
		wxUint8 *dname = &filename[nl];
		size_t s, l;

		n = GetFileNamePos(num, s, l);
		if (n == NULL || s == 0) {
			break;
		}
		if (s > size) s = size;
		memcpy(dname, n, s);

		nl += s;
		size -= s;
		num++;
	} while(num <= 1);

	if (nl > 0) {
		// 0x05を0xe5に変換(Sjift JISなど2バイト系文字など)
		if (filename[0] == 0x05) filename[0] = 0xe5;
	}

	length = nl;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemMSDOS::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;
	// ファイルサイズが16MBを超えている
	if (CheckUsed(false) && wxUINT32_SWAP_ON_BE(m_data->msdos.file_size) > 0xffffff) {
		valid = false;
	}
	return valid;
}

/// 削除できるか
bool DiskBasicDirItemMSDOS::IsDeletable() const
{
	// ".", ".."は不可
	bool valid = true;
	wxString name =	GetFileNamePlainStr();
	if (name == wxT(".") || name == wxT("..")) {
		valid = false;
	}
	return valid;
}

/// ファイル名を編集できるか
bool DiskBasicDirItemMSDOS::IsFileNameEditable() const
{
	// ".", ".."は不可
	return IsDeletable();
}

/// アイテムをロード・エクスポートできるか
/// @return true ロードできる
bool DiskBasicDirItemMSDOS::IsLoadable() const
{
	// ボリュームラベルは不可
	int t1 = GetFileType1();
	bool valid = ((t1 & (FILETYPE_MASK_MS_DIRECTORY | FILETYPE_MASK_MS_VOLUME)) != FILETYPE_MASK_MS_VOLUME);
	// ".", ".."は不可
	valid &= IsDeletable();
	return valid;
}

/// アイテムをコピーできるか
/// @return true コピーできる
bool DiskBasicDirItemMSDOS::IsCopyable() const
{
	// ".", ".."は不可
	return IsDeletable();
}

/// アイテムを上書きできるか
/// @return true 上書きできる
bool DiskBasicDirItemMSDOS::IsOverWritable() const
{
	// ディレクトリ、ボリュームラベルは不可
	int t1 = GetFileType1();
	bool valid = ((t1 & (FILETYPE_MASK_MS_DIRECTORY | FILETYPE_MASK_MS_VOLUME)) == 0);
	// ".", ".."は不可
	valid &= IsDeletable();
	return valid;
}

#if 0
/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemMSDOS::GetDefaultInvalidateChars() const
{
	return wxT(" \"\\/:*?<>|");
}
#endif

/// 属性を設定
void DiskBasicDirItemMSDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	// MS-DOS
	SetFileType1((ftype & 0xff00) >> 8);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemMSDOS::GetFileAttr() const
{
	int t1 = GetFileType1();
	return DiskBasicFileType(basic->GetFormatTypeNumber(), t1 << 8, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMSDOS::GetFileAttrStr() const
{
	wxString attr;
	int ftype = GetFileAttr().GetType();
	// MS-DOS
	GetFileAttrStrSub(ftype, attr);
	if (attr.IsEmpty()) {
		attr = wxT("---");
	}
	return attr;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
void DiskBasicDirItemMSDOS::GetFileAttrStrSub(int ftype, wxString &attr) const
{
	if (ftype & FILE_TYPE_READONLY_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_READ_ONLY]);	// read only
	}
	if (ftype & FILE_TYPE_HIDDEN_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_HIDDEN]);	// hidden
	}
	if (ftype & FILE_TYPE_SYSTEM_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_SYSTEM]);	// system
	}
	if (ftype & FILE_TYPE_VOLUME_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_VOLUME]);	// volume
	}
	if (ftype & FILE_TYPE_DIRECTORY_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_DIRECTORY]);	// directory
	}
	if (ftype & FILE_TYPE_ARCHIVE_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_ARCHIVE]);	// archive
	}
}

/// ファイルサイズをセット
void DiskBasicDirItemMSDOS::SetFileSize(int val)
{
	m_groups.SetSize(val);
	m_data->msdos.file_size = wxUINT32_SWAP_ON_BE(val);
}

/// ファイルサイズを返す
int DiskBasicDirItemMSDOS::GetFileSize() const
{
	int val = m_data->msdos.file_size;
	return wxUINT32_SWAP_ON_BE(val);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemMSDOS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemMSDOS::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	int calc_groups = 0; 
	int calc_file_size = GetFileSize();

	// 12bit FAT
	bool rc = true;
	wxUint32 group_num = GetStartGroup(fileunit_num);
	bool working = (group_num >= 2);
	int remain = (calc_file_size > 0 ? calc_file_size : 0x7ffffff);
	int limit = basic->GetFatEndGroup() + 1;
	while(working) {
		wxUint32 next_group = type->GetGroupNumber(group_num);
		if (next_group == group_num) {
			// 同じポジションならエラー
			rc = false;
		} else if (next_group >= 0xff8) {
			// 最終グループ
			working = false;
		} else if (next_group > (wxUint32)basic->GetFatEndGroup()) {
			// グループ番号がおかしい
			rc = false;
		}
		if (rc) {
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), remain, group_items);
//			m_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			remain -= (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			calc_groups++;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}

	group_items.SetNums(calc_groups);
	// 元のファイルサイズが０ならグループ数から計算したサイズを格納
	group_items.SetSize(calc_file_size > 0 ? calc_file_size : basic->GetSectorSize() * basic->GetSectorsPerGroup() * calc_groups);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
}

void DiskBasicDirItemMSDOS::GetFileDate(struct tm *tm) const
{
	// MS-DOS
	wxUint16 wdate = wxUINT16_SWAP_ON_BE(m_data->msdos.wdate);
	ConvDateToTm(wdate, tm);
}

void DiskBasicDirItemMSDOS::GetFileTime(struct tm *tm) const
{
	// MS-DOS
	wxUint16 wtime = wxUINT16_SWAP_ON_BE(m_data->msdos.wtime);
	ConvTimeToTm(wtime, tm);
}

wxString DiskBasicDirItemMSDOS::GetFileDateStr() const
{
	struct tm tm;
	GetFileDate(&tm);
	return Utils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemMSDOS::GetFileTimeStr() const
{
	struct tm tm;
	GetFileTime(&tm);
	return Utils::FormatHMSStr(&tm);
}

void DiskBasicDirItemMSDOS::SetFileDate(const struct tm *tm)
{
	if (tm->tm_year >= 0 && tm->tm_mon >= -1) {
		wxUint16 wdate = ConvTmToDate(tm);
		m_data->msdos.wdate = wxUINT16_SWAP_ON_BE(wdate);
	}
}

void DiskBasicDirItemMSDOS::SetFileTime(const struct tm *tm)
{
	if (tm->tm_hour >= 0 && tm->tm_min >= 0) {
		wxUint16 wtime = ConvTmToTime(tm);
		m_data->msdos.wtime = wxUINT16_SWAP_ON_BE(wtime); 
	}
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItemMSDOS::GetFileDateTimeTitle() const
{
	return _("Updated Date:");
}

/// 日付を変換
void DiskBasicDirItemMSDOS::ConvDateToTm(wxUint16 date, struct tm *tm)
{
	int yy = ((date & 0xfe00) >> 9) + 80;
//	if (yy >= 128) yy -= 28; 
	tm->tm_year = yy;
	tm->tm_mon = ((date & 0x01e0) >> 5) - 1;
	tm->tm_mday = (date & 0x001f);
}
/// 時間を変換
void DiskBasicDirItemMSDOS::ConvTimeToTm(wxUint16 time, struct tm *tm)
{
	tm->tm_hour = (time & 0xf800) >> 11;
	tm->tm_min = (time & 0x07e0) >> 5;
	tm->tm_sec = (time & 0x001f) << 1;
}
/// 日付に変換
wxUint16 DiskBasicDirItemMSDOS::ConvTmToDate(const struct tm *tm)
{
	int yy = tm->tm_year;
//	if (yy >= 100) yy += 28;
	return (wxUint16)
		(((yy - 80) & 0x7f) << 9)
		| (((tm->tm_mon + 1) & 0xf) << 5)
		| (tm->tm_mday & 0x1f);
}
/// 時間に変換
wxUint16 DiskBasicDirItemMSDOS::ConvTmToTime(const struct tm *tm)
{
	return (wxUint16)
		((tm->tm_hour & 0x1f) << 11)
		| ((tm->tm_min & 0x3f) << 5)
		| ((tm->tm_sec & 0x3f) >> 1);
}

size_t DiskBasicDirItemMSDOS::GetDataSize() const
{
	return sizeof(directory_msdos_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemMSDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// MS-DOS
	m_data->msdos.start_group = wxUINT16_SWAP_ON_BE(val);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemMSDOS::GetStartGroup(int fileunit_num) const
{
	// MS-DOS
	return wxUINT16_SWAP_ON_BE(m_data->msdos.start_group);
}

/// ファイル名から属性を決定する
int DiskBasicDirItemMSDOS::ConvFileTypeFromFileName(const wxString &filename) const
{
	return FILE_TYPE_ARCHIVE_MASK;
}

/// ダイアログ入力前のファイル名文字列を変換
void DiskBasicDirItemMSDOS::ConvertToFileNameStr(wxString &filename) const
{
	// 大文字にする
	filename = filename.Upper();
}

/// ダイアログ入力後のファイル名文字列を変換
/// ダイアログで指定したファイル名はすべて大文字にする
void DiskBasicDirItemMSDOS::ConvertFromFileNameStr(wxString &filename) const
{
	// 大文字にする
	filename = filename.Upper();
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_CHECK_READONLY	51
#define IDC_CHECK_HIDDEN	52
#define IDC_CHECK_SYSTEM	53
#define IDC_CHECK_VOLUME	54
#define IDC_CHECK_DIRECTORY	55
#define IDC_CHECK_ARCHIVE	56

/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemMSDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = ConvFileTypeFromFileName(name);
	}
	if (show_flags & INTNAME_IMPORT_INTERNAL) {
		// 内部からインポート時
		file_type_1 |= FILE_TYPE_ARCHIVE_MASK;
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemMSDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileAttr().GetType();
	int file_type_2 = 0;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	// 属性チェックボックスを作成
	CreateControlsSubForAttrDialog(parent, show_flags, sizer, flags, file_type_1);
}

/// ダイアログ内の属性部分のレイアウトを作成
wxStaticBoxSizer *DiskBasicDirItemMSDOS::CreateControlsSubForAttrDialog(IntNameBox *parent, int show_flags, wxBoxSizer *sizer, wxSizerFlags &flags, int file_type_1)
{
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkHidden;
	wxCheckBox *chkSystem;
	wxCheckBox *chkVolume;
	wxCheckBox *chkDirectory;
	wxCheckBox *chkArchive;

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameMS_l[TYPE_NAME_MS_READ_ONLY]));
	chkReadOnly->SetValue((file_type_1 & FILE_TYPE_READONLY_MASK) != 0);
	hbox->Add(chkReadOnly, flags);
	chkHidden = new wxCheckBox(parent, IDC_CHECK_HIDDEN, wxGetTranslation(gTypeNameMS_l[TYPE_NAME_MS_HIDDEN]));
	chkHidden->SetValue((file_type_1 & FILE_TYPE_HIDDEN_MASK) != 0);
	hbox->Add(chkHidden, flags);
	staType4->Add(hbox);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkSystem = new wxCheckBox(parent, IDC_CHECK_SYSTEM, wxGetTranslation(gTypeNameMS_l[TYPE_NAME_MS_SYSTEM]));
	chkSystem->SetValue((file_type_1 & FILE_TYPE_SYSTEM_MASK) != 0);
	hbox->Add(chkSystem, flags);
	chkVolume = new wxCheckBox(parent, IDC_CHECK_VOLUME, wxGetTranslation(gTypeNameMS_l[TYPE_NAME_MS_VOLUME]));
	chkVolume->SetValue((file_type_1 & FILE_TYPE_VOLUME_MASK) != 0);
	hbox->Add(chkVolume, flags);
	staType4->Add(hbox);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkDirectory = new wxCheckBox(parent, IDC_CHECK_DIRECTORY, wxGetTranslation(gTypeNameMS_l[TYPE_NAME_MS_DIRECTORY]));
	chkDirectory->SetValue((file_type_1 & FILE_TYPE_DIRECTORY_MASK) != 0);
	hbox->Add(chkDirectory, flags);
	chkArchive = new wxCheckBox(parent, IDC_CHECK_ARCHIVE, wxGetTranslation(gTypeNameMS_l[TYPE_NAME_MS_ARCHIVE]));
	chkArchive->SetValue((file_type_1 & FILE_TYPE_ARCHIVE_MASK) != 0);
	hbox->Add(chkArchive, flags);
	staType4->Add(hbox);

	sizer->Add(staType4, flags);

	return staType4;
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemMSDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	// 属性
	SetAttrSubInAttrDialog(parent, attr);
	return true;
}

/// 属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
void DiskBasicDirItemMSDOS::SetAttrSubInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr) const
{
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkHidden = (wxCheckBox *)parent->FindWindow(IDC_CHECK_HIDDEN);
	wxCheckBox *chkSystem = (wxCheckBox *)parent->FindWindow(IDC_CHECK_SYSTEM);
	wxCheckBox *chkVolume = (wxCheckBox *)parent->FindWindow(IDC_CHECK_VOLUME);
	wxCheckBox *chkDirectory = (wxCheckBox *)parent->FindWindow(IDC_CHECK_DIRECTORY);
	wxCheckBox *chkArchive = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ARCHIVE);

	int val = chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkHidden->GetValue() ? FILE_TYPE_HIDDEN_MASK : 0;
	val |= chkSystem->GetValue() ? FILE_TYPE_SYSTEM_MASK : 0;
	val |= chkVolume->GetValue() ? FILE_TYPE_VOLUME_MASK : 0;
	val |= chkDirectory->GetValue() ? FILE_TYPE_DIRECTORY_MASK : 0;
	val |= chkArchive->GetValue() ? FILE_TYPE_ARCHIVE_MASK : 0;

	attr.SetFileAttr(FORMAT_TYPE_UNKNOWN, val);
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemMSDOS::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	bool valid = true;
	wxString name =	filename;
	// "."で始まる名前は設定できない
	if (name.Left(1) == wxT(".")) {
		errormsg = wxString::Format(wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERRV_CANNOT_SET_NAME]), name);
		valid = false;
	}
	return valid;
}

///
///
///
DiskBasicDirItemVFAT::DiskBasicDirItemVFAT(DiskBasic *basic)
	: DiskBasicDirItemMSDOS(basic)
{
}
DiskBasicDirItemVFAT::DiskBasicDirItemVFAT(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItemMSDOS(basic, sector, secpos, data)
{
}
DiskBasicDirItemVFAT::DiskBasicDirItemVFAT(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemMSDOS(basic, num, track, side, sector, secpos, data, unuse)
{
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemVFAT::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// MS-DOS
	int t1 = GetFileType1();
	if (num == 0) {
		if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
			// ロングファイルネーム
			size = len = sizeof(m_data->mslfn.name);
			return m_data->mslfn.name;
		} else {
			size = len = sizeof(m_data->msdos.name);
			return m_data->msdos.name;
		}
	} else if (num == 1) {
		if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
			// ロングファイルネーム
			size = len = sizeof(m_data->mslfn.name2);
			return m_data->mslfn.name2;
		} else if (t1 & FILETYPE_MASK_MS_VOLUME) {
			// ボリュームラベルは拡張子もラベル名とする
			size = len = sizeof(m_data->msdos.ext);
			return m_data->msdos.ext;
		}
	} else if (num == 2) {
		if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
			// ロングファイルネーム
			size = len = sizeof(m_data->mslfn.name3);
			return m_data->mslfn.name3;
		}
	}
	size = len = 0;
	return NULL;
}

/// ファイル名を設定
void DiskBasicDirItemVFAT::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	if (length > 0) {
		// 0xe5は削除コードなので0x05に変換(Sjift JISなど2バイト系文字など)
		if (filename[0] == 0xe5) filename[0] = 0x05;
	}

	wxUint8 *n;
	size_t nl = 0;
	size_t s, l;

	int num = 0;
	do {
		s = l = 0;
		n = GetFileNamePos(num, s, l);
		if (n == NULL || s == 0) {
			break;
		}
		if (num > 0) {
			size_t pl = nl;
			size_t ps = s;
			if (nl < length) {
				pl = length;
				ps = s + nl - length;
			}
			memset(&filename[pl], 0, ps);
		}

		if (s > size) s = size;
		memcpy(n, &filename[nl], s);

		nl += s;
		size -= s;
		num++;
	} while(num <= 4);
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemVFAT::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t nl = 0;
	int num = 0;
	do {
		wxUint8 *dname = &filename[nl];
		size_t s, l;

		n = GetFileNamePos(num, s, l);
		if (n == NULL || s == 0) {
			break;
		}
		if (s > size) s = size;
		memcpy(dname, n, s);

		nl += s;
		size -= s;
		num++;
	} while(num <= 4);

	if (nl > 0) {
		// 0x05を0xe5に変換(Sjift JISなど2バイト系文字など)
		if (filename[0] == 0x05) filename[0] = 0xe5;
	}

	length = nl;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemVFAT::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;
	// ファイルサイズが16MBを超えている
	if (CheckUsed(false) && (m_data->msdos.type & FILETYPE_MASK_MS_LFN) != FILETYPE_MASK_MS_LFN && wxUINT32_SWAP_ON_BE(m_data->msdos.file_size) > 0xffffff) {
		valid = false;
	}
	return valid;
}

/// 削除できるか
bool DiskBasicDirItemVFAT::IsDeletable() const
{
	bool valid = true;
	int t1 = GetFileType1();
	if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		valid = false;
	} else if (t1 & FILETYPE_MASK_MS_DIRECTORY) {
		wxString name =	GetFileNamePlainStr();
		if (name == wxT(".") || name == wxT("..")) {
			// ディレクトリ ".", ".."は削除不可
			valid = false;
		}
	}
	return valid;
}

/// ファイル名を編集できるか
bool DiskBasicDirItemVFAT::IsFileNameEditable() const
{
	bool valid = true;
	int t1 = GetFileType1();
	if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		valid = false;
	} else if (t1 & FILETYPE_MASK_MS_DIRECTORY) {
		wxString name =	GetFileNamePlainStr();
		if (name == wxT(".") || name == wxT("..")) {
			// ディレクトリ ".", ".."は削除不可
			valid = false;
		}
	}
	return valid;
}

/// 属性を設定
void DiskBasicDirItemVFAT::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	// MS-DOS
	SetFileType1((ftype & 0xff00) >> 8);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemVFAT::GetFileAttr() const
{
	int t1 = GetFileType1();
	return DiskBasicFileType(basic->GetFormatTypeNumber(), t1 << 8, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemVFAT::GetFileAttrStr() const
{
	wxString attr;
	int ftype = GetFileAttr().GetType();
	// MS-DOS
	if ((GetFileAttr().GetOrigin() & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_LFN]);	// long file name
	} else {
		GetFileAttrStrSub(ftype, attr);
	}
	if (attr.IsEmpty()) {
		attr = wxT("---");
	}
	return attr;
}

wxString DiskBasicDirItemVFAT::GetCDateStr() const
{
	struct tm tm;
	wxUint16 date = wxUINT16_SWAP_ON_BE(m_data->msdos.cdate);
	ConvDateToTm(date, &tm);
	return Utils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemVFAT::GetCTimeStr() const
{
	struct tm tm;
	wxUint16 time = wxUINT16_SWAP_ON_BE(m_data->msdos.ctime);
	ConvTimeToTm(time, &tm);
	return Utils::FormatHMSStr(&tm);
}

wxString DiskBasicDirItemVFAT::GetADateStr() const
{
	struct tm tm;
	wxUint16 date = wxUINT16_SWAP_ON_BE(m_data->msdos.adate);
	ConvDateToTm(date, &tm);
	return Utils::FormatYMDStr(&tm);
}

/// 日付をセット
void DiskBasicDirItemVFAT::SetCDate(const struct tm *tm)
{
	if (tm->tm_year >= 0 && tm->tm_mon >= -1) {
		wxUint16 date = ConvTmToDate(tm);
		m_data->msdos.cdate = wxUINT16_SWAP_ON_BE(date);
	}
}
/// 時間をセット
void DiskBasicDirItemVFAT::SetCTime(const struct tm *tm)
{
	if (tm->tm_hour >= 0 && tm->tm_min >= 0) {
		wxUint16 time = ConvTmToTime(tm);
		m_data->msdos.ctime = wxUINT16_SWAP_ON_BE(time); 
	}
}
/// 日付をセット
void DiskBasicDirItemVFAT::SetADate(const struct tm *tm)
{
	if (tm->tm_year >= 0 && tm->tm_mon >= -1) {
		wxUint16 date = ConvTmToDate(tm);
		m_data->msdos.adate = wxUINT16_SWAP_ON_BE(date);
	}
}

/// 最初のグループ番号を設定
void DiskBasicDirItemVFAT::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// MS-DOS
	m_data->msdos.start_group = wxUINT16_SWAP_ON_BE(val);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemVFAT::GetStartGroup(int fileunit_num) const
{
	// MS-DOS
	return wxUINT16_SWAP_ON_BE(m_data->msdos.start_group);
}

/// 文字列をバイト列に変換 文字コードは機種依存
/// @return >=0 バイト数
int DiskBasicDirItemVFAT::ConvStringToChars(const wxString &src, wxUint8 *dst, size_t len) const
{
	if ((GetFileType1() & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		// ロングファイル名は常にUTF-16
		wxCharBuffer buf = src.mb_str(wxMBConvUTF16());
		if (buf.length() > 0) {
			size_t l = buf.length() > len ? len : buf.length();
			memcpy(dst, buf.data(), l);
			return (int)l;
		} else {
			return 0;
		}
	} else {
		return basic->GetCharCodes().ConvToChars(src, dst, len);
	}
}

/// バイト列を文字列に変換 文字コードは機種依存
void DiskBasicDirItemVFAT::ConvCharsToString(const wxUint8 *src, size_t len, wxString &dst) const
{
	if ((GetFileType1() & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		// ロングファイル名は常にUTF-16
		dst = wxString((const char *)src, wxMBConvUTF16());
	} else {
		basic->GetCharCodes().ConvToString(src, len, dst);
	}
}

//
// ダイアログ用
//

#define IDC_TEXT_CDATE		57
#define IDC_TEXT_CTIME		58
#define IDC_TEXT_ADATE		59

#if 0
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemVFAT::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = FILE_TYPE_ARCHIVE_MASK;
	}
	if (show_flags & INTNAME_IMPORT_INTERNAL) {
		// 内部からインポート時
		file_type_1 |= FILE_TYPE_ARCHIVE_MASK;
	}
}
#endif

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemVFAT::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileAttr().GetType();
	int file_type_2 = 0;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	// 属性チェックボックスを作成
	CreateControlsSubForAttrDialog(parent, show_flags, sizer, flags, file_type_1);

	wxTextCtrl *txtCDate;
	wxTextCtrl *txtCTime;
	wxTextCtrl *txtADate;

	DateTimeValidator date_validate(false, !CanIgnoreDateTime());
	DateTimeValidator time_validate(true, !CanIgnoreDateTime());

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags stflags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);
	hbox->Add(new wxStaticText(parent, wxID_ANY, _("Created Date:")), stflags);
	txtCDate = new wxTextCtrl(parent, IDC_TEXT_CDATE, GetCDateStr(), wxDefaultPosition, wxDefaultSize, 0, date_validate);
	txtCDate->SetMinSize(IntNameBox::GetDateTextExtent(txtCDate));
	hbox->Add(txtCDate, flags);
	txtCTime = new wxTextCtrl(parent, IDC_TEXT_CTIME, GetCTimeStr(), wxDefaultPosition, wxDefaultSize, 0, time_validate);
	txtCTime->SetMinSize(IntNameBox::GetTimeTextExtent(txtCTime));
	hbox->Add(txtCTime, flags);
	sizer->Add(hbox, flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxStaticText(parent, wxID_ANY, _("Accessed Date:")), stflags);
	txtADate = new wxTextCtrl(parent, IDC_TEXT_ADATE, GetADateStr(), wxDefaultPosition, wxDefaultSize, 0, date_validate);
	txtADate->SetMinSize(IntNameBox::GetDateTextExtent(txtADate));
	hbox->Add(txtADate, flags);
	sizer->Add(hbox, flags);
}

#define ATTR_CDATE_TIME_IDX	1
#define ATTR_ADATE_TIME_IDX	2

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemVFAT::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	// 属性
	SetAttrSubInAttrDialog(parent, attr);

	wxTextCtrl *txtCDate = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_CDATE);
	wxTextCtrl *txtCTime = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_CTIME);
	wxTextCtrl *txtADate = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_ADATE);

	struct tm tm;
	if (txtCDate && txtCTime) {
		Utils::ConvDateStrToTm(txtCDate->GetValue(), &tm);
		Utils::ConvTimeStrToTm(txtCTime->GetValue(), &tm);
		attr.SetDateTime(ATTR_CDATE_TIME_IDX, &tm);
	}
	if (txtADate) {
		Utils::ConvDateStrToTm(txtADate->GetValue(), &tm);
		attr.SetDateTime(ATTR_ADATE_TIME_IDX, &tm);
	}
	return true;
}

/// その他の属性値を設定する
void DiskBasicDirItemVFAT::SetAttr(DiskBasicDirItemAttr &attr)
{
	SetCDate(attr.GetDateTime(ATTR_CDATE_TIME_IDX));
	SetCTime(attr.GetDateTime(ATTR_CDATE_TIME_IDX));

	SetADate(attr.GetDateTime(ATTR_ADATE_TIME_IDX));
}
