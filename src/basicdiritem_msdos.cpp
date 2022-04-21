/// @file basicdiritem_msdos.cpp
///
/// @brief disk basic directory item for MS-DOS
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
	NULL
};

///
///
///
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, data)
{
}
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	// MS-DOS
	used = CheckUsed(unuse);
	unuse = (unuse || (this->data->msdos.name[0] == 0));

	// ファイルサイズをセット
	file_size = wxUINT32_SWAP_ON_BE(this->data->msdos.file_size);

	// グループ数を計算
	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemMSDOS::GetFileNamePos(size_t &len, bool *invert) const
{
	// MS-DOS
	len = sizeof(data->msdos.name);
	if (GetFileType1() & FILETYPE_MASK_MS_VOLUME) {
		// ボリュームラベルは拡張子なしにする
		len += sizeof(data->msdos.ext);
	}
	return data->msdos.name;
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemMSDOS::GetFileExtPos(size_t &len) const
{
	wxUint8 *p = NULL;
	len = 0;
	// ボリュームラベルは拡張子なしにする
	if (!(GetFileType1() & FILETYPE_MASK_MS_VOLUME)) {
		len = sizeof(data->msdos.ext);
		p = data->msdos.ext;
	}
	return p;
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemMSDOS::GetFileNameSize(bool *invert) const
{
	size_t len = sizeof(data->msdos.name);
	if (GetFileType1() & FILETYPE_MASK_MS_VOLUME) {
		// ボリュームラベルは拡張子なしにする
		len += sizeof(data->msdos.ext);
	}
	return (int)len;
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemMSDOS::GetFileExtSize(bool *invert) const
{
	size_t len = 0;
	// ボリュームラベルは拡張子なしにする
	if (!(GetFileType1() & FILETYPE_MASK_MS_VOLUME)) {
		len = sizeof(data->msdos.ext);
	}
	return (int)len;
}

/// 属性１を返す
int	DiskBasicDirItemMSDOS::GetFileType1() const
{
	return data->msdos.type;
}

/// 属性１を設定
void DiskBasicDirItemMSDOS::SetFileType1(int val)
{
	data->msdos.type = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemMSDOS::CheckUsed(bool unuse)
{
	return (data->msdos.name[0] != 0 && data->msdos.name[0] != 0xe5);
}

/// ファイル名を設定
void DiskBasicDirItemMSDOS::SetFileName(const wxUint8 *filename, int length)
{
	wxUint8 name[FILENAME_BUFSIZE];
	memcpy(name, filename, length);
	if (length > 0) {
		if (name[0] == 0xe5) name[0] = 0x05;
	}
	DiskBasicDirItem::SetFileName(name, length);
}

/// ファイル名と拡張子を得る
void DiskBasicDirItemMSDOS::GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	DiskBasicDirItem::GetFileName(name, nlen, ext, elen);
	if (nlen > 0) {
		if (name[0] == 0x05) name[0] = 0xe5;
	}
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemMSDOS::Check(bool &last)
{
	if (!data) return false;

	bool valid = true;
	// ファイルサイズが16MBを超えている
	if (CheckUsed(false) && wxUINT32_SWAP_ON_BE(data->msdos.file_size) > 0xffffff) {
		valid = false;
	}
	return valid;
}

/// 削除できるか
bool DiskBasicDirItemMSDOS::IsDeletable()
{
	bool valid = true;
	int attr = GetFileAttr();
	if (attr & FILE_TYPE_DIRECTORY_MASK) {
		wxString name =	GetFileNamePlainStr();
		if (name == wxT(".") || name == wxT("..")) {
			// ディレクトリ ".", ".."は削除不可
			valid = false;
		}
	}
	return valid;
}

/// ファイル名を編集できるか
bool DiskBasicDirItemMSDOS::IsFileNameEditable()
{
	bool valid = true;
	int attr = GetFileAttr();
	if (attr & FILE_TYPE_DIRECTORY_MASK) {
		wxString name =	GetFileNamePlainStr();
		if (name == wxT(".") || name == wxT("..")) {
			// ディレクトリ ".", ".."は削除不可
			valid = false;
		}
	}
	return valid;
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemMSDOS::InvalidateChars()
{
	return wxT(" \"\\/:*?<>|");
}

void DiskBasicDirItemMSDOS::SetFileAttr(int file_type)
{
	if (file_type == -1) return;

	// MS-DOS
	SetFileType1((file_type & 0xff00) >> 8);
}

int DiskBasicDirItemMSDOS::GetFileAttr()
{
	return GetFileType1() << 8;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemMSDOS::GetFileType1Pos()
{
	return GetFileAttr();
}

int	DiskBasicDirItemMSDOS::CalcFileTypeFromPos(int pos1, int pos2)
{
	return pos1;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMSDOS::GetFileAttrStr()
{
	wxString attr;
	// MS-DOS
	if (GetFileAttr() & FILE_TYPE_READONLY_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_READ_ONLY]);	// read only
	}
	if (GetFileAttr() & FILE_TYPE_HIDDEN_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_HIDDEN]);	// hidden
	}
	if (GetFileAttr() & FILE_TYPE_SYSTEM_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_SYSTEM]);	// system
	}
	if (GetFileAttr() & FILE_TYPE_VOLUME_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_VOLUME]);	// volume
	}
	if (GetFileAttr() & FILE_TYPE_DIRECTORY_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_DIRECTORY]);	// directory
	}
	if (GetFileAttr() & FILE_TYPE_ARCHIVE_MASK) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_ARCHIVE]);	// archive
	}
	if (attr.IsEmpty()) {
		attr = wxT("---");
	}
	return attr;
}

/// ファイルサイズをセット
void DiskBasicDirItemMSDOS::SetFileSize(int val)
{
	file_size = val;
	data->msdos.file_size = wxUINT32_SWAP_ON_BE(val);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemMSDOS::CalcFileSize()
{
	if (!used) return;

	int calc_file_size = 0;
	int calc_groups = 0; 
	wxUint32 last_group = GetStartGroup();
//	int last_sector = 0;

	// 12bit FAT
	bool rc = true;
	wxUint32 group_num = last_group;
	bool working = true;
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
			calc_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			calc_groups++;
			last_group = group_num;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}

	if (rc) {
		groups = calc_groups;
		if (file_size <= 0) {
			file_size = calc_file_size;
		}
	}
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemMSDOS::GetAllGroups(DiskBasicGroups &group_items)
{
//	file_size = 0;
//	groups = 0; 

	// 12bit FAT
	bool rc = true;
	wxUint32 group_num = GetStartGroup();
	bool working = true;
	int remain = file_size;
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
//			file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			remain -= (basic->GetSectorSize() * basic->GetSectorsPerGroup());
//			groups++;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}

	group_items.SetSize(file_size);

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
}

void DiskBasicDirItemMSDOS::GetFileDate(struct tm *tm)
{
	// MS-DOS
	wxUint16 wdate = wxUINT16_SWAP_ON_BE(data->msdos.wdate);
	ConvDateToTm(wdate, tm);
}

void DiskBasicDirItemMSDOS::GetFileTime(struct tm *tm)
{
	// MS-DOS
	wxUint16 wtime = wxUINT16_SWAP_ON_BE(data->msdos.wtime);
	ConvTimeToTm(wtime, tm);
}

wxString DiskBasicDirItemMSDOS::GetFileDateStr()
{
	struct tm tm;
	GetFileDate(&tm);
	return L3DiskUtils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemMSDOS::GetFileTimeStr()
{
	struct tm tm;
	GetFileTime(&tm);
	return L3DiskUtils::FormatHMSStr(&tm);
}

void DiskBasicDirItemMSDOS::SetFileDate(const struct tm *tm)
{
	if (tm->tm_year >= 0 && tm->tm_mon >= -1) {
		wxUint16 wdate = ConvTmToDate(tm);
		data->msdos.wdate = wxUINT16_SWAP_ON_BE(wdate);
	}
}

void DiskBasicDirItemMSDOS::SetFileTime(const struct tm *tm)
{
	if (tm->tm_hour >= 0 && tm->tm_min >= 0) {
		wxUint16 wtime = ConvTmToTime(tm);
		data->msdos.wtime = wxUINT16_SWAP_ON_BE(wtime); 
	}
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItemMSDOS::GetFileDateTimeTitle()
{
	return _("Updated Date:");
}

wxString DiskBasicDirItemMSDOS::GetCDateStr()
{
	struct tm tm;
	wxUint16 date = wxUINT16_SWAP_ON_BE(data->msdos.cdate);
	ConvDateToTm(date, &tm);
	return L3DiskUtils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemMSDOS::GetCTimeStr()
{
	struct tm tm;
	wxUint16 time = wxUINT16_SWAP_ON_BE(data->msdos.ctime);
	ConvTimeToTm(time, &tm);
	return L3DiskUtils::FormatHMSStr(&tm);
}

wxString DiskBasicDirItemMSDOS::GetADateStr()
{
	struct tm tm;
	wxUint16 date = wxUINT16_SWAP_ON_BE(data->msdos.adate);
	ConvDateToTm(date, &tm);
	return L3DiskUtils::FormatYMDStr(&tm);
}

/// 日付をセット
void DiskBasicDirItemMSDOS::SetCDate(const struct tm *tm)
{
	if (tm->tm_year >= 0 && tm->tm_mon >= -1) {
		wxUint16 date = ConvTmToDate(tm);
		data->msdos.cdate = wxUINT16_SWAP_ON_BE(date);
	}
}
/// 時間をセット
void DiskBasicDirItemMSDOS::SetCTime(const struct tm *tm)
{
	if (tm->tm_hour >= 0 && tm->tm_min >= 0) {
		wxUint16 time = ConvTmToTime(tm);
		data->msdos.ctime = wxUINT16_SWAP_ON_BE(time); 
	}
}
/// 日付をセット
void DiskBasicDirItemMSDOS::SetADate(const struct tm *tm)
{
	if (tm->tm_year >= 0 && tm->tm_mon >= -1) {
		wxUint16 date = ConvTmToDate(tm);
		data->msdos.adate = wxUINT16_SWAP_ON_BE(date);
	}
}

/// 日付を変換
void DiskBasicDirItemMSDOS::ConvDateToTm(wxUint16 date, struct tm *tm)
{
	tm->tm_year = ((date & 0xfe00) >> 9) + 80;
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
	return (wxUint16)
		(((tm->tm_year - 80) & 0x7f) << 9)
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

size_t DiskBasicDirItemMSDOS::GetDataSize()
{
	return sizeof(directory_msdos_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemMSDOS::SetStartGroup(wxUint32 val)
{
	// MS-DOS
	data->msdos.start_group = wxUINT16_SWAP_ON_BE(val);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemMSDOS::GetStartGroup() const
{
	// MS-DOS
	return wxUINT16_SWAP_ON_BE(data->msdos.start_group);
}

/// ダイアログ入力前のファイル名文字列を変換
void DiskBasicDirItemMSDOS::ConvertToFileNameStr(wxString &filename)
{
	// 大文字にする
	filename = filename.Upper();
}

/// ダイアログ入力後のファイル名文字列を変換
/// ダイアログで指定したファイル名はすべて大文字にする
void DiskBasicDirItemMSDOS::ConvertFromFileNameStr(wxString &filename)
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
#define IDC_TEXT_CDATE		57
#define IDC_TEXT_CTIME		58
#define IDC_TEXT_ADATE		59

/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemMSDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
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

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemMSDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkHidden;
	wxCheckBox *chkSystem;
	wxCheckBox *chkVolume;
	wxCheckBox *chkDirectory;
	wxCheckBox *chkArchive;
	wxTextCtrl *txtCDate;
	wxTextCtrl *txtCTime;
	wxTextCtrl *txtADate;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

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

	wxSize sz(80, -1);
	DateTimeValidator date_validate(false);
	DateTimeValidator time_validate(true);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags stflags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);
	hbox->Add(new wxStaticText(parent, wxID_ANY, _("Created Date:")), stflags);
	txtCDate = new wxTextCtrl(parent, IDC_TEXT_CDATE, GetCDateStr(), wxDefaultPosition, sz, 0, date_validate);
	hbox->Add(txtCDate, flags);
	txtCTime = new wxTextCtrl(parent, IDC_TEXT_CTIME, GetCTimeStr(), wxDefaultPosition, sz, 0, time_validate);
	hbox->Add(txtCTime, flags);
	sizer->Add(hbox, flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxStaticText(parent, wxID_ANY, _("Accessed Date:")), stflags);
	txtADate = new wxTextCtrl(parent, IDC_TEXT_ADATE, GetADateStr(), wxDefaultPosition, sz, 0, date_validate);
	hbox->Add(txtADate, flags);
	sizer->Add(hbox, flags);
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemMSDOS::GetFileType1InAttrDialog(const IntNameBox *parent) const
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

	return val;
}

/// 機種依存の属性を設定する
bool DiskBasicDirItemMSDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo)
{
	DiskBasicDirItem::SetAttrInAttrDialog(parent, errinfo);

	wxTextCtrl *txtCDate = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_CDATE);
	wxTextCtrl *txtCTime = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_CTIME);
	wxTextCtrl *txtADate = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_ADATE);

	struct tm tm;
	if (txtCDate) {
		L3DiskUtils::ConvDateStrToTm(txtCDate->GetValue(), &tm);
		SetCDate(&tm);
	}
	if (txtCTime) {
		L3DiskUtils::ConvTimeStrToTm(txtCTime->GetValue(), &tm);
		SetCTime(&tm);
	}
	if (txtADate) {
		L3DiskUtils::ConvDateStrToTm(txtADate->GetValue(), &tm);
		SetADate(&tm);
	}
	return true;
}
