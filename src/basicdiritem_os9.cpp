/// @file basicdiritem_os9.cpp
///
/// @brief disk basic directory item for OS-9
///
#include "basicdiritem_os9.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "utils.h"

//
//
//

/// OS-9属性名
const char *gTypeNameOS9[] = {
	wxTRANSLATE("<DIR>"),
	wxTRANSLATE("Non-sharable"),
	NULL
};
const char gTypeNameOS9_2[] = {
	'X','W','R','x','w','r',0
};
const char *gTypeNameOS9_2l[] = {
	wxTRANSLATE("Execute"),
	wxTRANSLATE("Write"),
	wxTRANSLATE("Read"),
	NULL
};

//
//
//
DiskBasicDirItemOS9FD::DiskBasicDirItemOS9FD()
{
	sector = NULL;
	fd = NULL;
}
/// ポインタをセット
void DiskBasicDirItemOS9FD::Set(DiskD88Sector *n_sector, directory_os9_fd_t *n_fd)
{
	sector = n_sector;
	fd = n_fd;
}
/// 属性を返す
wxUint8 DiskBasicDirItemOS9FD::GetATT() const
{
	return fd ? fd->FD_ATT : 0;
}
/// 属性をセット
void DiskBasicDirItemOS9FD::SetATT(wxUint8 val)
{
	if (fd) fd->FD_ATT = val;
}
/// セグメントのLSNを返す
wxUint32 DiskBasicDirItemOS9FD::GetLSN(int idx) const
{
	return GET_OS9_LSN(fd->FD_SEG[idx].LSN);
}
/// セグメントのセクタ数を返す
wxUint16 DiskBasicDirItemOS9FD::GetSIZ(int idx) const
{
	return wxUINT16_SWAP_ON_LE(fd->FD_SEG[idx].SIZ);
}
/// ファイルサイズを返す
wxUint32 DiskBasicDirItemOS9FD::GetSIZ() const
{
	return wxUINT32_SWAP_ON_LE(fd->FD_SIZ);
}
/// 更新日付を返す
const os9_date_t &DiskBasicDirItemOS9FD::GetDAT() const
{
	return fd->FD_DAT;
}
/// 更新日付をセット
void DiskBasicDirItemOS9FD::SetDAT(const os9_date_t &val)
{
	fd->FD_DAT = val;
}
/// 更新日付をセット
void DiskBasicDirItemOS9FD::SetDAT(const os9_cdate_t &val)
{
	fd->FD_DAT.yy = val.yy;
	fd->FD_DAT.mm = val.mm;
	fd->FD_DAT.dd = val.dd;
}
/// 作成日付を返す
const os9_cdate_t &DiskBasicDirItemOS9FD::GetDCR() const
{
	return fd->FD_DCR;
}
/// 作成日付をセット
void DiskBasicDirItemOS9FD::SetDCR(const os9_cdate_t &val)
{
	fd->FD_DCR = val;
}
// 更新にする
void DiskBasicDirItemOS9FD::SetModify()
{
	if (sector) sector->SetModify();
}

//
//
//
DiskBasicDirItemOS9::DiskBasicDirItemOS9(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemOS9::DiskBasicDirItemOS9(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, data)
{
}
DiskBasicDirItemOS9::DiskBasicDirItemOS9(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	used = CheckUsed(unuse);

	// FDセクタへのポインタをセット
	if (used) {
		wxUint32 lsn = GET_OS9_LSN(((directory_os9_t *)data)->DE_LSN);
		if (lsn != 0) {
			DiskD88Sector *sector = basic->GetSectorFromGroup(lsn);
			if (sector) {
				fd.Set(sector, (directory_os9_fd_t *)sector->GetSectorBuffer());
			}
		}
	}

	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemOS9::GetFileNamePos(size_t &len, bool *invert) const
{
	len = sizeof(data->os9.DE_NAM);
	return data->os9.DE_NAM; 
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemOS9::GetFileNameSize(bool *invert) const
{
	return (int)sizeof(data->os9.DE_NAM);
}

/// 属性１を返す
int	DiskBasicDirItemOS9::GetFileType1() const
{
	return fd.GetATT();
}

/// 属性１を設定
void DiskBasicDirItemOS9::SetFileType1(int val)
{
	fd.SetATT(val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemOS9::CheckUsed(bool unuse)
{
	return (data->os9.DE_NAM[0] != 0);
}

/// ファイル名を設定
void DiskBasicDirItemOS9::SetFileName(const wxUint8 *filename, int length)
{
	DiskBasicDirItem::SetFileName(filename, length);

	size_t l;
	wxUint8 *n = GetFileNamePos(l);

	// 文字列の最後にMSBをセット
	for(int i = (int)l - 1; i >= 0; i--) {
		if (n[i] != 0) {
			n[i] |= 0x80;
			break;
		}
	}
}

/// ファイル名を得る
void DiskBasicDirItemOS9::GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen)
{
	DiskBasicDirItem::GetFileName(name, nlen, ext, elen);

	// 文字列の最後はMSBがセットされているのでクリア
	if (nlen > 0) {
		int last = (int)nlen;
		for(int n = last - 1; n >= 0; n--) {
			if (name[n] & 0x80) {
				name[n] &= 0x7f;
				last = n + 1;
			}
		}
		name[last] = 0;
	}
}

/// 削除
/// @param [in] code : 削除コード(デフォルト0)
bool DiskBasicDirItemOS9::Delete(wxUint8 code)
{
	// 削除はエントリの先頭にコードを入れるだけ
	data->os9.DE_NAM[0] = code;
	used = false;
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemOS9::Check(bool &last)
{
	if (!data) return false;

	return true;
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemOS9::InvalidateChars()
{
	return wxT(" !\"#$%&'()*+,-/:;<=>?@[\\]^{|}~");
}

void DiskBasicDirItemOS9::SetFileAttr(int file_type)
{
	int val = 0;
	if (file_type & FILE_TYPE_DIRECTORY_MASK) {
		val |= FILETYPE_MASK_OS9_DIRECTORY;
	}
	if (file_type & FILE_TYPE_NONSHARE_MASK) {
		val |= FILETYPE_MASK_OS9_NONSHARE;
	}
	val |= ((file_type & FILETYPE_OS9_PERMISSION_MASK) >> FILETYPE_OS9_PERMISSION_POS);

	SetFileType1(val);
}

#if 0
/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemOS9::ClearData()
{
	if (!data) return;
	memset(data, 0, sizeof(directory_os9_t));
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemOS9::InitialData()
{
	ClearData();
}
#endif

int DiskBasicDirItemOS9::GetFileType()
{
	int val = 0;
	int type1 = GetFileType1();
	if (type1 & FILETYPE_MASK_OS9_DIRECTORY) {
		val |= FILE_TYPE_DIRECTORY_MASK;
	}
	if (type1 & FILETYPE_MASK_OS9_NONSHARE) {
		val |= FILE_TYPE_NONSHARE_MASK;
	}
	val |= ((type1 << FILETYPE_OS9_PERMISSION_POS) & FILETYPE_OS9_PERMISSION_MASK);
	return val;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemOS9::GetFileType1Pos()
{
	return GetFileType();
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemOS9::GetFileType2Pos()
{
	return 0;
}

int	DiskBasicDirItemOS9::CalcFileTypeFromPos(int pos1, int pos2)
{
	return pos1;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemOS9::GetFileAttrStr()
{
	wxString str;
	if (fd.IsValid()) {
		if (fd.GetATT() & FILETYPE_MASK_OS9_DIRECTORY) {
			if (!str.IsEmpty()) str += wxT(", ");
			str += wxGetTranslation(gTypeNameOS9[TYPE_NAME_OS9_DIRECTORY]);
		}
		if (fd.GetATT() & FILETYPE_MASK_OS9_NONSHARE) {
			if (!str.IsEmpty()) str += wxT(", ");
			str += wxGetTranslation(gTypeNameOS9[TYPE_NAME_OS9_NONSHARE]);
		}
		if (!str.IsEmpty()) str += wxT(", ");
		for(int i=0; i<6; i++) {
			if (fd.GetATT() & (0x20 >> i)) {
				str += wxString(1, gTypeNameOS9_2[i]);
			} else {
				str += wxString(1, '-');
			}
		}
	}
	return str;
}

/// ファイルサイズをセット
void DiskBasicDirItemOS9::SetFileSize(int val)
{
	file_size = val;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemOS9::CalcFileSize()
{
	if (!used || !fd.IsValid()) return;

	// ファイルサイズ
	file_size = fd.GetSIZ();

	int calc_file_size = 0;
	int calc_groups = 0;
//	int last_group = 0;
//	int last_sector = 0;
	bool rc = true;

	for(int i=0; i<48; i++) {
		wxUint32 lsn = fd.GetLSN(i);
		if (lsn == 0) break;

		int siz = fd.GetSIZ(i);
		calc_file_size += (siz * basic->GetSectorsPerGroup() * basic->GetSectorSize());
		calc_groups += siz;
	}

	// グループ数を計算
	if (rc) {
//		file_size = calc_file_size;
		groups = calc_groups;
	}
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemOS9::GetAllGroups(DiskBasicGroups &group_items)
{
	if (!fd.IsValid()) return;

//	bool rc = true;

	group_items.SetSize(GetFileSize());

	for(int i=0; i<48; i++) {
		wxUint32 lsn = fd.GetLSN(i);
		int      siz = fd.GetSIZ(i);
		if (siz == 0) {
			break;
		}
		if (i != 0) {
			DiskBasicGroupItem *gitm = &group_items.Last();
			gitm->next = lsn;
		}
		for(int n = 0; n < siz; n++) {
			int track_num = 0;
			int side_num = 0;
			int sector_num = 1;
			wxUint32 next_lsn = (n + 1 != siz ? lsn + n + 1 : 0);
			basic->CalcNumFromSectorPosForGroup(lsn + n, track_num, side_num, sector_num);
			group_items.Add(lsn + n, next_lsn, track_num, side_num, sector_num, sector_num + basic->GetSectorsPerGroup() - 1);
		}

//		file_size += (siz * secs_per_group * sector_size);
//		groups += siz;
	}
}

void DiskBasicDirItemOS9::GetFileDate(struct tm *tm)
{
	if (fd.IsValid()) {
		ConvDateToTm((const os9_cdate_t &)fd.GetDAT(), tm);
	} else {
		tm->tm_year = 0;
		tm->tm_mon = 0; 
		tm->tm_mday = 0;
	}
}

void DiskBasicDirItemOS9::GetFileTime(struct tm *tm)
{
	if (fd.IsValid()) {
		ConvTimeToTm(fd.GetDAT(), tm);
	} else {
		tm->tm_hour = 0; 
		tm->tm_min = 0;
	}
	tm->tm_sec = 0;
}

wxString DiskBasicDirItemOS9::GetFileDateStr()
{
	struct tm tm;
	GetFileDate(&tm);
	return L3DiskUtils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemOS9::GetFileTimeStr()
{
	struct tm tm;
	GetFileTime(&tm);
	return L3DiskUtils::FormatHMStr(&tm);
}

void DiskBasicDirItemOS9::SetFileDate(const struct tm *tm)
{
	if (fd.IsValid()) {
		os9_cdate_t date;
		ConvTmToDate(tm, date);
		fd.SetDAT(date);
	}
}

void DiskBasicDirItemOS9::SetFileTime(const struct tm *tm)
{
	if (fd.IsValid()) {
		os9_date_t time = fd.GetDAT();
		ConvTmToTime(tm, time);
		fd.SetDAT(time);
	}
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItemOS9::GetFileDateTimeTitle()
{
	return _("Modified Date:");
}

wxString DiskBasicDirItemOS9::GetCDateStr()
{
	struct tm tm;
	if (fd.IsValid()) {
		ConvDateToTm(fd.GetDCR(), &tm);
	} else {
		tm.tm_year = 0;
		tm.tm_mon = 0;
		tm.tm_mday = 0;
	}
	return L3DiskUtils::FormatYMDStr(&tm);
}

/// 日付をセット
void DiskBasicDirItemOS9::SetCDate(const struct tm *tm)
{
	if (fd.IsValid()) {
		os9_cdate_t date;
		ConvTmToDate(tm, date);
		fd.SetDCR(date);
	}
}

/// 日付を変換
void DiskBasicDirItemOS9::ConvDateToTm(const os9_cdate_t &date, struct tm *tm)
{
	tm->tm_year = (date.yy % 100);
	if (tm->tm_year < 80) tm->tm_year += 100;
	tm->tm_mon = date.mm - 1; 
	tm->tm_mday = date.dd;
}
/// 時間を変換
void DiskBasicDirItemOS9::ConvTimeToTm(const os9_date_t &time, struct tm *tm)
{
	tm->tm_hour = time.hh; 
	tm->tm_min = time.mi;
}
/// 日付に変換
void DiskBasicDirItemOS9::ConvTmToDate(const struct tm *tm, os9_cdate_t &date)
{
	date.yy = (tm->tm_year % 100);
	date.mm = tm->tm_mon + 1;
	date.dd = tm->tm_mday;
}
/// 時間に変換
void DiskBasicDirItemOS9::ConvTmToTime(const struct tm *tm, os9_date_t &time)
{
	time.hh = tm->tm_hour;
	time.mi = tm->tm_min;
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemOS9::GetDataSize()
{
	return sizeof(directory_os9_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemOS9::SetStartGroup(wxUint32 val)
{
//	if (fd) {
//		SET_OS9_LSN(fd->FD_SEG[0].LSN, val);
//	}
	SET_OS9_LSN(data->os9.DE_LSN, val);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemOS9::GetStartGroup() const
{
//	wxUint32 val = 0;
//	if (fd) {
//		val = GET_OS9_LSN(fd->FD_SEG[0].LSN);
//	}
//	return val;
	return GET_OS9_LSN(data->os9.DE_LSN);
}

/// 追加のグループ番号をセット FDセクタへのLSNをセット
void DiskBasicDirItemOS9::SetExtraGroup(wxUint32 val)
{
	SET_OS9_LSN(data->os9.DE_LSN, val);
}

/// 追加のグループ番号を返す FDセクタへのLSNを返す
wxUint32 DiskBasicDirItemOS9::GetExtraGroup() const
{
	return GET_OS9_LSN(data->os9.DE_LSN);
}

/// 書き込み/上書き禁止か
bool DiskBasicDirItemOS9::IsWriteProtected()
{
	return false;
}
bool DiskBasicDirItemOS9::IsDeleteable()
{
	return false;
}
/// ファイル名を編集できるか
bool DiskBasicDirItemOS9::IsFileNameEditable()
{
	return true;
}

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemOS9::SetModify()
{
	DiskBasicDirItem::SetModify();
	fd.SetModify();
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_CHECK_DIRECTORY 51
#define IDC_CHECK_NONSHARE	52
#define IDC_CHECK_PUB_EXEC	53
#define IDC_CHECK_PUB_WRITE	54
#define IDC_CHECK_PUB_READ	55
#define IDC_CHECK_USR_EXEC	56
#define IDC_CHECK_USR_WRITE	57
#define IDC_CHECK_USR_READ	58
#define IDC_TEXT_CDATE		59

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] file_type_1    ファイル属性1 GetFileType1Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] file_type_2    ファイル属性2 GetFileType2Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] sizer
/// @param [in] flags
/// @param [in,out] controls   [0]: wxTextCtrl::txtIntNameで予約済み [1]からユーザ設定
/// @param [in,out] user_data  ユーザ定義データ
void DiskBasicDirItemOS9::CreateControlsForAttrDialog(IntNameBox *parent, int file_type_1, int file_type_2, wxBoxSizer *sizer, wxSizerFlags &flags, AttrControls &controls, int *user_data)
{
	wxCheckBox *chkDirectory;
	wxCheckBox *chkSharable;

	wxCheckBox *chkPubExec;
	wxCheckBox *chkPubWrite;
	wxCheckBox *chkPubRead;
	wxCheckBox *chkUsrExec;
	wxCheckBox *chkUsrWrite;
	wxCheckBox *chkUsrRead;

	wxTextCtrl *txtCDate;

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	chkDirectory = new wxCheckBox(parent, IDC_CHECK_DIRECTORY, wxGetTranslation(gTypeNameOS9[TYPE_NAME_OS9_DIRECTORY]));
	chkDirectory->SetValue((file_type_1 & FILE_TYPE_DIRECTORY_MASK) != 0);
	hbox->Add(chkDirectory, flags);

	chkSharable = new wxCheckBox(parent, IDC_CHECK_NONSHARE, wxGetTranslation(gTypeNameOS9[TYPE_NAME_OS9_NONSHARE]));
	chkSharable->SetValue((file_type_1 & FILE_TYPE_NONSHARE_MASK) != 0);
	hbox->Add(chkSharable, flags);
	staType4->Add(hbox);

	sizer->Add(staType4);

	wxStaticBoxSizer *staType5 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("Permission")), wxVERTICAL);
	staType5->Add(new wxStaticText(parent, wxID_ANY, _("Public")), flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkPubExec = new wxCheckBox(parent, IDC_CHECK_PUB_EXEC, wxGetTranslation(gTypeNameOS9_2l[0]));
	chkPubExec->SetValue((file_type_1 & (FILETYPE_MASK_OS9_PUBLIC_EXEC << FILETYPE_OS9_PERMISSION_POS)) != 0);
	hbox->Add(chkPubExec, flags);
	chkPubWrite = new wxCheckBox(parent, IDC_CHECK_PUB_WRITE, wxGetTranslation(gTypeNameOS9_2l[1]));
	chkPubWrite->SetValue((file_type_1 & (FILETYPE_MASK_OS9_PUBLIC_WRITE << FILETYPE_OS9_PERMISSION_POS)) != 0);
	hbox->Add(chkPubWrite, flags);
	chkPubRead = new wxCheckBox(parent, IDC_CHECK_PUB_READ, wxGetTranslation(gTypeNameOS9_2l[2]));
	chkPubRead->SetValue((file_type_1 & (FILETYPE_MASK_OS9_PUBLIC_READ << FILETYPE_OS9_PERMISSION_POS)) != 0);
	hbox->Add(chkPubRead, flags);
	staType5->Add(hbox);

	staType5->Add(new wxStaticText(parent, wxID_ANY, _("Owner")), flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkUsrExec = new wxCheckBox(parent, IDC_CHECK_USR_EXEC, wxGetTranslation(gTypeNameOS9_2l[0]));
	chkUsrExec->SetValue((file_type_1 & (FILETYPE_MASK_OS9_USER_EXEC << FILETYPE_OS9_PERMISSION_POS)) != 0);
	hbox->Add(chkUsrExec, flags);
	chkUsrWrite = new wxCheckBox(parent, IDC_CHECK_USR_WRITE, wxGetTranslation(gTypeNameOS9_2l[1]));
	chkUsrWrite->SetValue((file_type_1 & (FILETYPE_MASK_OS9_USER_WRITE << FILETYPE_OS9_PERMISSION_POS)) != 0);
	hbox->Add(chkUsrWrite, flags);
	chkUsrRead = new wxCheckBox(parent, IDC_CHECK_USR_READ, wxGetTranslation(gTypeNameOS9_2l[2]));
	chkUsrRead->SetValue((file_type_1 & (FILETYPE_MASK_OS9_USER_READ << FILETYPE_OS9_PERMISSION_POS)) != 0);
	hbox->Add(chkUsrRead, flags);
	staType5->Add(hbox);

	sizer->Add(staType5, flags);

	wxSize sz(80, -1);
	DateTimeValidator date_validate(false);
	DateTimeValidator time_validate(true);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags stflags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);
	hbox->Add(new wxStaticText(parent, wxID_ANY, _("Created Date:")), stflags);
	txtCDate = new wxTextCtrl(parent, IDC_TEXT_CDATE, GetCDateStr(), wxDefaultPosition, sz, 0, date_validate);
	hbox->Add(txtCDate, flags);
	sizer->Add(hbox, flags);

	controls.Add(chkDirectory);
	controls.Add(chkSharable);
	controls.Add(chkPubExec);
	controls.Add(chkPubWrite);
	controls.Add(chkPubRead);
	controls.Add(chkUsrExec);
	controls.Add(chkUsrWrite);
	controls.Add(chkUsrRead);
	controls.Add(txtCDate);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemOS9::ChangeTypeInAttrDialog(AttrControls &controls)
{
}

/// ダイアログ用に属性を設定する
/// インポート時ダイアログ表示前にファイルの属性を設定
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemOS9::SetFileTypeForAttrDialog(const wxString &name, int &file_type_1, int &file_type_2)
{
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemOS9::GetFileType1InAttrDialog(const AttrControls &controls) const
{
	wxCheckBox *chkDirectory = (wxCheckBox *)controls.Item(1);
	wxCheckBox *chkSharable = (wxCheckBox *)controls.Item(2);

	wxCheckBox *chkPubExec = (wxCheckBox *)controls.Item(3);
	wxCheckBox *chkPubWrite = (wxCheckBox *)controls.Item(4);
	wxCheckBox *chkPubRead = (wxCheckBox *)controls.Item(5);
	wxCheckBox *chkUsrExec = (wxCheckBox *)controls.Item(6);
	wxCheckBox *chkUsrWrite = (wxCheckBox *)controls.Item(7);
	wxCheckBox *chkUsrRead = (wxCheckBox *)controls.Item(8);

	int val = 0;
	val |= (chkDirectory->GetValue() ? FILE_TYPE_DIRECTORY_MASK : 0);
	val |= (chkSharable->GetValue() ? FILE_TYPE_NONSHARE_MASK : 0);
	val |= (chkPubExec->GetValue() ? (FILETYPE_MASK_OS9_PUBLIC_EXEC << FILETYPE_OS9_PERMISSION_POS) : 0);
	val |= (chkPubWrite->GetValue() ? (FILETYPE_MASK_OS9_PUBLIC_WRITE << FILETYPE_OS9_PERMISSION_POS) : 0);
	val |= (chkPubRead->GetValue() ? (FILETYPE_MASK_OS9_PUBLIC_READ << FILETYPE_OS9_PERMISSION_POS) : 0);
	val |= (chkUsrExec->GetValue() ? (FILETYPE_MASK_OS9_USER_EXEC << FILETYPE_OS9_PERMISSION_POS) : 0);
	val |= (chkUsrWrite->GetValue() ? (FILETYPE_MASK_OS9_USER_WRITE << FILETYPE_OS9_PERMISSION_POS) : 0);
	val |= (chkUsrRead->GetValue() ? (FILETYPE_MASK_OS9_USER_READ << FILETYPE_OS9_PERMISSION_POS) : 0);

	return val;
}

/// 属性2を得る
/// @return CalcFileTypeFromPos()のpos2に渡す値
int DiskBasicDirItemOS9::GetFileType2InAttrDialog(const AttrControls &controls, const int *user_data) const
{
	return 0;
}

/// 機種依存の属性を設定する
bool DiskBasicDirItemOS9::SetAttrInAttrDialog(const AttrControls &controls, DiskBasicError &errinfo)
{
	wxTextCtrl *txtCDate = (wxTextCtrl *)controls.Item(9);

	struct tm tm;
	if (txtCDate) {
		L3DiskUtils::ConvDateStrToTm(txtCDate->GetValue(), &tm);
		SetCDate(&tm);
	}
	return true;
}
