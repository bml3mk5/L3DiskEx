/// @file intnamebox.cpp
///
/// @brief 内部ファイル名ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "intnamebox.h"
#include "main.h"
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/regex.h>
#include <wx/numformatter.h>
#include <wx/msgdlg.h>
#include "uifilelist.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "charcodes.h"
#include "utils.h"


#define TEXT_VALIDATOR_TRANS \
_("'%s' should only contain ASCII characters."), \
_("'%s' should only contain alphabetic characters."), \
_("'%s' should only contain alphabetic or numeric characters."), \
_("'%s' should only contain digits."), \
_("'%s' should be numeric."), \
_("'%s' doesn't consist only of valid characters"), \
_("'%s' contains illegal characters")

#if 0
IntNameParam::IntNameParam()
{
	user_data = 0;
}
#endif

// Attach Event
BEGIN_EVENT_TABLE(IntNameBox, wxDialog)
	EVT_TEXT(IDC_TEXT_START_ADDR, IntNameBox::OnChangeStartAddr)
	EVT_CHECKBOX(IDC_CHK_IGNORE_DATE, IntNameBox::OnChangeIgnoreDate)
	EVT_BUTTON(wxID_OK, IntNameBox::OnOK)
END_EVENT_TABLE()

/// @brief コンストラクタ
/// @param [in] frame      親フレーム
/// @param [in] parent     親ウィンドウ
/// @param [in] id         ウィンドウID
/// @param [in] caption    ウィンドウキャプション
/// @param [in] message    メッセージ
/// @param [in] basic      DISK BASIC
/// @param [in] item       ディレクトリアイテム
/// @param [in] file_path  元ファイル名(show_flagsが #INTNAME_SPECIFY_FILE_NAME のとき指定)
/// @param [in] file_name  ファイル名(show_flagsが #INTNAME_SPECIFY_FILE_NAME のとき指定)
/// @param [in] file_size  ファイルサイズ(show_flagsが #INTNAME_SPECIFY_FILE_NAME のとき指定)
/// @param [in] date_time  日時(show_flagsが #INTNAME_SPECIFY_DATE_TIME のとき指定)
/// @param [in] show_flags 表示フラグ
IntNameBox::IntNameBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &message,
	DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, const wxString &file_name, int file_size, struct tm *date_time, int show_flags)
	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX, wxT(INTNAMEBOX_CLASSNAME))
{
	CreateBox(frame, parent, id, caption, message, basic, item, file_path, file_name, file_size, date_time, show_flags);
}

/// @brief ダイアログ内の作成
/// @param [in] frame      親フレーム
/// @param [in] parent     親ウィンドウ
/// @param [in] id         ウィンドウID
/// @param [in] caption    ウィンドウキャプション
/// @param [in] message    メッセージ
/// @param [in] basic      DISK BASIC
/// @param [in] item       ディレクトリアイテム
/// @param [in] file_path  元ファイル名(show_flagsが #INTNAME_SPECIFY_FILE_NAME のとき指定)
/// @param [in] file_name  ファイル名(show_flagsが #INTNAME_SPECIFY_FILE_NAME のとき指定)
/// @param [in] file_size  ファイルサイズ(show_flagsが #INTNAME_SPECIFY_FILE_NAME のとき指定)
/// @param [in] date_time  日時(show_flagsが #INTNAME_SPECIFY_DATE_TIME のとき指定)
/// @param [in] show_flags 表示フラグ
void IntNameBox::CreateBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &message,
	DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, const wxString &file_name, int file_size, struct tm *date_time, int show_flags)
{
	this->frame = frame;
	this->item = item;
	this->unique_number = frame->GetUniqueNumber();
	this->basic = basic;

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags stflags = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);
	wxSize size;
	long style = 0;
	mNameMaxLen = item->GetFileNameStrSize();

	wxFont font;
	frame->GetDefaultListFont(font);

	wxString filename;
	if (show_flags & INTNAME_SPECIFY_FILE_NAME) {
		// ファイル名の指定があるとき
		filename = file_name;
		this->file_size = file_size;
	} else {
		// アイテム内のファイル名
		filename = item->GetFileNameStr();
		this->file_size = item->GetFileSize();
	}

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox;
	wxSize sz;

	if (!message.IsEmpty()) {
		szrAll->Add(new wxStaticText(this, wxID_ANY, message), flags);
	}

	txtIntName = NULL;
	if (show_flags & INTNAME_SHOW_TEXT) {
		szrAll->Add(new wxStaticText(this, wxID_ANY, _("File Name In The Disk Image")), flags);
		size.x = DEFAULT_TEXTWIDTH; size.y = -1;
		if (item->IsFileNameEditable()) {
			IntNameValidator validate(basic, item);
			txtIntName = new wxTextCtrl(this, IDC_TEXT_INTNAME, filename, wxDefaultPosition, size, style, validate);
			txtIntName->SetMaxLength((unsigned long)mNameMaxLen);
		} else {
			txtIntName = new wxTextCtrl(this, IDC_TEXT_INTNAME, filename, wxDefaultPosition, size, style | wxTE_READONLY);
		}
		txtIntName->SetFont(font);

		szrAll->Add(txtIntName, flags);
	}


	user_data = 0;

	txtStartAddr = NULL;
	txtEndAddr = NULL;
	txtExecAddr = NULL;
	txtCDate = NULL;
	txtCTime = NULL;
	chkIgnoreDate = NULL;
	txtFileSize = NULL;
	if (show_flags & INTNAME_SHOW_ATTR) {
		// 属性の表示は機種依存
		item->CreateControlsForAttrDialog(this, show_flags, file_path, szrAll, flags);

		// 開始アドレス、終了アドレス、実行アドレス
		if (item->HasAddress()) {
			AddressValidator validate;

			bool has_exec_addr = item->HasExecuteAddress();

			int rows = 2 + (has_exec_addr ? 1 : 0);
			wxGridSizer *szrG = new wxGridSizer(rows, 2, 2, 4);

			szrG->Add(new wxStaticText(this, wxID_ANY, _("Load Address") + _("(Hex)")), stflags);
			txtStartAddr = new wxTextCtrl(this, IDC_TEXT_START_ADDR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, validate);
			txtStartAddr->SetMaxLength(4);
			szrG->Add(txtStartAddr, flags);


			szrG->Add(new wxStaticText(this, wxID_ANY, _("End Address") + _("(Hex)")), stflags);
			txtEndAddr = new wxTextCtrl(this, IDC_TEXT_END_ADDR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, validate);
			txtEndAddr->SetMaxLength(4);
			szrG->Add(txtEndAddr, flags);

			if (has_exec_addr) {
				szrG->Add(new wxStaticText(this, wxID_ANY, _("Execute Address") + _("(Hex)")), stflags);
				txtExecAddr = new wxTextCtrl(this, IDC_TEXT_EXEC_ADDR, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, validate);
				txtExecAddr->SetMaxLength(4);
				szrG->Add(txtExecAddr, flags);
			}

			szrAll->Add(szrG, flags);

			// fire on-change event when set start address.
			int addr = -1;
			if (txtStartAddr) {
				addr = item->GetStartAddress();
				txtStartAddr->SetValue(addr >= 0 ? wxString::Format(wxT("%x"), addr) : wxT("----"));
				txtStartAddr->SetEditable(item->IsAddressEditable() && addr >= 0);
			}
			if (txtExecAddr) {
				addr = item->GetExecuteAddress();
				txtExecAddr->SetValue(addr >= 0 ? wxString::Format(wxT("%x"), addr) : wxT("----"));
				txtExecAddr->SetEditable(item->IsAddressEditable() && addr >= 0);
			}
		}

		// 作成日付を表示
		if (item->HasDateTime()) {
			hbox = new wxBoxSizer(wxHORIZONTAL);
			hbox->Add(new wxStaticText(this, wxID_ANY, item->GetFileDateTimeTitle()), stflags);

			if (item->HasDate()) {
				DateTimeValidator date_validate(false, !item->CanIgnoreDateTime());
				txtCDate = new wxTextCtrl(this, IDC_TEXT_CDATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, date_validate);
				txtCDate->SetMinSize(GetDateTextExtent(txtCDate));
				txtCDate->SetMaxLength(10);
				hbox->Add(txtCDate, flags);
			}
			if (item->HasTime()) {
				DateTimeValidator time_validate(true, !item->CanIgnoreDateTime());
				txtCTime = new wxTextCtrl(this, IDC_TEXT_CTIME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, time_validate);
				txtCTime->SetMinSize(GetTimeTextExtent(txtCTime));
				txtCTime->SetMaxLength(8);
				hbox->Add(txtCTime, flags);
			}
			szrAll->Add(hbox, flags);

			if (item->CanIgnoreDateTime()) {
				hbox = new wxBoxSizer(wxHORIZONTAL);
				hbox->Add(new wxStaticText(this, wxID_ANY, wxT("    ")), flags);
				chkIgnoreDate = new wxCheckBox(this, IDC_CHK_IGNORE_DATE, _("Ignore date and time."));
				chkIgnoreDate->SetValue(gConfig.DoesIgnoreDateTime());
				ChangedIgnoreDate(chkIgnoreDate->GetValue());
				hbox->Add(chkIgnoreDate, flags);
				szrAll->Add(hbox, wxSizerFlags().Expand().Border(wxBOTTOM, 4));
			}
		}

		// ファイルサイズ
		hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("File Size:")), stflags);
		txtFileSize = new wxTextCtrl(this, IDC_TEXT_FILE_SIZE, wxEmptyString, wxDefaultPosition, size, wxTE_RIGHT | wxTE_READONLY);
		size = txtFileSize->GetTextExtent(wxString((char)'0', 20));
		size.y = -1;
		txtFileSize->SetMinSize(size);
		hbox->Add(txtFileSize, flags);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("bytes")), stflags);
		szrAll->Add(hbox, flags);
	}

	// プロパティの場合は詳細表示
	txtGroups = NULL;
	txtGrpSize = NULL;
	lstGroups = NULL;
	if (show_flags & INTNAME_SHOW_PROPERTY) {
		// グループのリスト
		hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("Occupied Groups:")), stflags);
		txtGroups = new wxTextCtrl(this, IDC_TEXT_GROUPS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT | wxTE_READONLY);
		size = txtGroups->GetTextExtent(wxString((char)'0', 12));
		size.y = -1;
		txtGroups->SetMinSize(size);
		hbox->Add(txtGroups, flags);
		szrAll->Add(hbox, flags);

		hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("Occupied Size:")), stflags);
		txtGrpSize = new wxTextCtrl(this, IDC_TEXT_GROUP_SIZE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT | wxTE_READONLY);
		size = txtGrpSize->GetTextExtent(wxString((char)'0', 20));
		size.y = -1;
		txtGrpSize->SetMinSize(size);
		hbox->Add(txtGrpSize, flags);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("bytes")), stflags);
		szrAll->Add(hbox, flags);

		sz.Set(272, -1);
		lstGroups = new wxListCtrl(this, IDC_LIST_GROUPS, wxDefaultPosition, sz, wxLC_REPORT | wxLC_SINGLE_SEL);
		lstGroups->AppendColumn(_("Group"), wxLIST_FORMAT_LEFT, 40);
		lstGroups->AppendColumn(_("Track"), wxLIST_FORMAT_RIGHT, 40);
		lstGroups->AppendColumn(_("Side"), wxLIST_FORMAT_RIGHT, 40);
		lstGroups->AppendColumn(_("Start Sector"), wxLIST_FORMAT_RIGHT, 40);
		lstGroups->AppendColumn(_("End Sector"), wxLIST_FORMAT_RIGHT, 40);
		lstGroups->AppendColumn(_("Division"), wxLIST_FORMAT_RIGHT, 40);
		szrAll->Add(lstGroups, flags);

		lstGroups->SetFont(font);
		lstGroups->Bind(wxEVT_LIST_ITEM_SELECTED, &IntNameBox::OnListItemSelected, this);

	}

	chkSkipDlg = NULL;
	if (show_flags & INTNAME_SHOW_SKIP_DIALOG) {
		// 以降スキップ
		sz.Set(200, 2);
		szrAll->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, sz, wxLI_HORIZONTAL), flags);
		chkSkipDlg = new wxCheckBox(this, IDC_CHK_SKIP_DLG, _("Skip the confirmation after this."));
		chkSkipDlg->SetValue(gConfig.IsSkipImportDialog());
		szrAll->Add(chkSkipDlg, flags);
	}

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);

	if (show_flags & INTNAME_NEW_FILE) {
		// 新規作成時、日付は現在日付をセット
		struct tm tm_now;
		wxDateTime::GetTmNow(&tm_now);
		SetDateTime(&tm_now);
	} else if ((show_flags & INTNAME_SPECIFY_DATE_TIME) != 0 && date_time != NULL) {
		// 日時は引数のものを指定
		SetDateTime(date_time);
	} else {
		// アイテム内の日付をセット
		SetDateTime(item->GetFileDateStr(), item->GetFileTimeStr());
	}

	SetFileSize(this->file_size);

	item->InitializeForAttrDialog(this, show_flags, &user_data);

	ChangedType1();
}

/// ダイアログ表示
int IntNameBox::ShowModal()
{
	return wxDialog::ShowModal();
}

/// OKボタン押下
void IntNameBox::OnOK(wxCommandEvent& event)
{
	if (Validate() && TransferDataFromWindow()) {
		if (IsModal()) {
			EndModal(wxID_OK);
		} else {
			// モードレスの場合、値を反映
			L3DiskFileList *file_list = frame->GetFileListPanel();
			if (file_list) {
				file_list->AcceptSubmittedFileAttr(this);
			}
			SetReturnCode(wxID_OK);
			this->Show(false);
		}
	}
}

/// 属性を変更した時(機種依存)
void IntNameBox::OnChangeType1(wxCommandEvent& WXUNUSED(event))
{
	ChangedType1();
}

/// 属性を変更(機種依存)
void IntNameBox::ChangedType1()
{
	if (item) {
		item->ChangeTypeInAttrDialog(this);
	}
}

/// 開始アドレスを変更
void IntNameBox::OnChangeStartAddr(wxCommandEvent& event)
{
	CalcEndAddress();
}

/// 日付を無視するにチェック
void IntNameBox::OnChangeIgnoreDate(wxCommandEvent& event)
{
	ChangedIgnoreDate(event.IsChecked());
}

/// 日付を無視するにチェック
void IntNameBox::ChangedIgnoreDate(bool check)
{
	if (txtCDate) {
		txtCDate->Enable(!check);
	}
	if (txtCTime) {
		txtCTime->Enable(!check);
	}
}

/// グループリストのアイテムを選択
void IntNameBox::OnListItemSelected(wxListEvent& event)
{
	int idx = (int)event.GetIndex();

	if (frame->GetBinDumpFrame()) {
		L3DiskFileList *file_list = frame->GetFileListPanel();
		if (!file_list) return;

		long grp, trk, sid, sec_start, sec_end;
		lstGroups->GetItemText(idx, 0).ToLong(&grp, 16);
		lstGroups->GetItemText(idx, 1).ToLong(&trk);
		lstGroups->GetItemText(idx, 2).ToLong(&sid);
		lstGroups->GetItemText(idx, 3).ToLong(&sec_start);
		lstGroups->GetItemText(idx, 4).ToLong(&sec_end);

		file_list->SetDumpData((int)trk, (int)sid, (int)sec_start, (int)sec_end);
	}
}

/// ディレクトリアイテムを設定
void IntNameBox::SetDiskBasicDirItem(DiskBasicDirItem *item)
{
	this->item = item;

//	SetDateTime(item->GetFileDateStr(), item->GetFileTimeStr());
//	SetFileSize(item->GetFileSize());
}

/// 内部ファイル名を設定
void IntNameBox::SetInternalName(const wxString &name)
{
	if (txtIntName) txtIntName->SetValue(name);
}

/// 内部ファイル名を得る
/// DiskBasicDirItem::ConvertFromFileNameStr()で変換したファイル名
void IntNameBox::GetInternalName(wxString &name) const
{
	if (!txtIntName) return;

	wxString val = txtIntName->GetValue();
	size_t len = val.Length();
	if (len > mNameMaxLen) {
		val = val.Mid(0, mNameMaxLen);
	}
	if (item) {
		// ダイアログ入力後のファイル名文字列を変換 機種依存の処理 大文字にするなど
		item->ConvertFromFileNameStr(val);
	}

	name = val;
}

/// 終了アドレスを計算
void IntNameBox::CalcEndAddress()
{
	if (txtEndAddr && item) {
		int end_addr = item->GetEndAddressInAttrDialog(this);
		bool editable = true;
		if (end_addr < 0) {
			// 終了アドレスは開始アドレスとサイズから計算する
			editable = false;
			int start_addr = GetStartAddress();
			if (start_addr >= 0) {
				end_addr = start_addr + file_size - (file_size > 0 ? 1 : 0);
			}
		}
		SetEndAddress(end_addr);
		SetEditableEndAddress(item->IsEndAddressEditableInAttrDialog(this) && editable);
	}
}

/// 開始アドレスを設定
void IntNameBox::SetStartAddress(int val)
{
	SetAddress(txtStartAddr, val);
}

/// 終了アドレスを設定
void IntNameBox::SetEndAddress(int val)
{
	SetAddress(txtEndAddr, val);
}

/// 実行アドレスを設定
void IntNameBox::SetExecuteAddress(int val)
{
	SetAddress(txtExecAddr, val);
}

void IntNameBox::SetAddress(wxTextCtrl *ctrl, int val)
{
	if (ctrl) {
		ctrl->SetValue(val >= 0 ? wxString::Format(wxT("%x"), (val & 0xffff)) : wxT("----"));
	}
}

/// 開始アドレスを返す
/// @return -1 エラーあり
int IntNameBox::GetStartAddress() const
{
	return GetAddress(txtStartAddr);
}

/// 終了アドレスを返す
/// @return -1 エラーあり
int IntNameBox::GetEndAddress() const
{
	return GetAddress(txtEndAddr);
}

/// 実行アドレスを返す
/// @return -1 エラーあり
int IntNameBox::GetExecuteAddress() const
{
	return GetAddress(txtExecAddr);
}

/// アドレスを返す
/// @return -1 エラーあり
int IntNameBox::GetAddress(wxTextCtrl *ctrl) const
{
	int val = -1;
	if (ctrl) {
		wxString sval = ctrl->GetValue();
		long lval = 0;
		if (sval.Left(0) != wxT("-") && sval.ToLong(&lval, 16)) {
			val = (int)lval;
		}
	}
	return val;
}

/// 開始アドレスの有効を設定
void IntNameBox::EnableStartAddress(bool val)
{
	EnableAddress(txtStartAddr, val);
}

/// 終了アドレスの有効を設定
void IntNameBox::EnableEndAddress(bool val)
{
	EnableAddress(txtEndAddr, val);
}

/// 実行アドレスの有効を設定
void IntNameBox::EnableExecuteAddress(bool val)
{
	EnableAddress(txtExecAddr, val);
}

void IntNameBox::EnableAddress(wxTextCtrl *ctrl, bool val)
{
	if (ctrl) {
		ctrl->Enable(val);
	}
}

/// 開始アドレスが編集可能かを設定
void IntNameBox::SetEditableStartAddress(bool val)
{
	SetEditableAddress(txtStartAddr, val);
}

/// 終了アドレスが編集可能かを設定
void IntNameBox::SetEditableEndAddress(bool val)
{
	SetEditableAddress(txtEndAddr, val);
}

/// 実行アドレスが編集可能かを設定
void IntNameBox::SetEditableExecuteAddress(bool val)
{
	SetEditableAddress(txtExecAddr, val);
}

void IntNameBox::SetEditableAddress(wxTextCtrl *ctrl, bool val)
{
	if (ctrl) {
		ctrl->SetEditable(val);
	}
}

/// 日時を得る
void IntNameBox::GetDateTime(struct tm *tm) const
{
	if (tm) {
		memset(tm, 0, sizeof(struct tm));

		if (txtCDate) {
			Utils::ConvDateStrToTm(txtCDate->GetValue(), tm);
		}
		if (txtCTime) {
			Utils::ConvTimeStrToTm(txtCTime->GetValue(), tm);
		}
	}
}

/// 日時を返す
struct tm IntNameBox::GetDateTime() const
{
	struct tm tm;
	GetDateTime(&tm);
	return tm;
}

// 日付部分のサイズを得る
wxSize IntNameBox::GetDateTextExtent(wxTextCtrl *ctrl)
{
	wxSize size = ctrl->GetTextExtent(wxT("000000/00/0000"));
	size.y = -1;
	return size;
}

// 時間部分のサイズを得る
wxSize IntNameBox::GetTimeTextExtent(wxTextCtrl *ctrl)
{
	wxSize size = ctrl->GetTextExtent(wxT("0000:00:0000"));
	size.y = -1;
	return size;
}

/// 日時を設定
void IntNameBox::SetDateTime(const wxString &date, const wxString &time)
{
	if (txtCDate && !date.IsEmpty()) {
		txtCDate->SetValue(date);
	}
	if (txtCTime && !time.IsEmpty()) {
		txtCTime->SetValue(time);
	}
}

/// 日時を設定
void IntNameBox::SetDateTime(const struct tm *tm)
{
	wxString sdate = Utils::FormatYMDStr(tm);
	wxString stime = Utils::FormatHMSStr(tm);
	SetDateTime(sdate, stime);
}

/// 日付を無視するか
bool IntNameBox::DoesIgnoreDateTime(bool def_val) const
{
	if (chkIgnoreDate) {
		return chkIgnoreDate->GetValue();
	} else {
		return def_val;
	}
}

/// 日付を無視する
void IntNameBox::IgnoreDateTime(bool val)
{
	if (chkIgnoreDate) {
		chkIgnoreDate->SetValue(val);
		ChangedIgnoreDate(val);
	}
}

/// ファイルサイズを設定
void IntNameBox::SetFileSize(long val)
{
	if (txtFileSize) {
		wxString str;
		if (val >= 0) {
			str = wxNumberFormatter::ToString(val);
			str += wxString::Format(wxT(" (0x%x)"), (int)val);
		} else {
			str = wxT("---");
		}
		txtFileSize->SetValue(str);
	}
}

/// グループリストを設定
void IntNameBox::SetGroups(const DiskBasicGroups &vals)
{
	long val = (long)vals.GetNums();

	if (txtGroups) {
		wxString str;
		if (val >= 0) {
			str = wxNumberFormatter::ToString(val);
			str += wxString::Format(wxT(" (0x%x)"), (int)val);
		} else {
			str = wxT("---");
		}
		txtGroups->SetValue(str);
	}
	if (txtGrpSize) {
		wxString str;
		if (val >= 0) {
			val = val * vals.GetSizePerGroup();
			str = wxNumberFormatter::ToString(val);
			str += wxString::Format(wxT(" (0x%x)"), (int)val);
		} else {
			str = wxT("---");
		}
		txtGrpSize->SetValue(str);
	}
	if (lstGroups) {
		lstGroups->DeleteAllItems();

		for(size_t i=0; i < vals.Count(); i++) {
			DiskBasicGroupItem *item = vals.ItemPtr(i);
			lstGroups->InsertItem(i, wxString::Format(wxT("%02x"), item->group));
			lstGroups->SetItem(i, 1, wxString::Format(wxT("%d"), item->track));
			lstGroups->SetItem(i, 2, wxString::Format(wxT("%d"), item->side));
			lstGroups->SetItem(i, 3, wxString::Format(wxT("%d"), item->sector_start));
			lstGroups->SetItem(i, 4, wxString::Format(wxT("%d"), item->sector_end));
			lstGroups->SetItem(i, 5, wxString::Format(wxT("%d/%d"), item->div_num + 1, item->div_nums));
		}
	}
}

/// 以降スキップを返す
bool IntNameBox::IsSkipDialog(bool def_val) const
{
	return (chkSkipDlg ? chkSkipDlg->IsChecked() : def_val);
}

//////////////////////////////////////////////////////////////////////
//
// 内部ファイル名バリデータ
//
// Attach Events
wxBEGIN_EVENT_TABLE(IntNameValidator, wxValidator)
    EVT_CHAR(IntNameValidator::OnChar)
wxEND_EVENT_TABLE()

IntNameValidator::IntNameValidator(DiskBasic *basic, DiskBasicDirItem *item) : wxValidator()
{
	CreateValidator(basic, item);
}
IntNameValidator::IntNameValidator(const IntNameValidator& val) : wxValidator()
{
    Copy(val);
}
void IntNameValidator::CreateValidator(DiskBasic *basic, DiskBasicDirItem *item)
{
	this->basic = basic;
	this->item = item;
	valchrs = basic->GetValidChars();
	invchrs = basic->GetInvalidChars();
	dupchrs = basic->GetDeduplicateChars();
	maxlen = item->GetFileNameStrSize();
}
bool IntNameValidator::Copy(const IntNameValidator& val)
{
    wxValidator::Copy(val);
	basic = val.basic;
	item = val.item;
	valchrs = val.valchrs;
	invchrs = val.invchrs;
	dupchrs = val.dupchrs;
	maxlen = val.maxlen;
	return true;
}

wxTextEntry *IntNameValidator::GetTextEntry()
{
	if (wxDynamicCast(m_validatorWindow, wxTextCtrl)) {
		return (wxTextCtrl*)m_validatorWindow;
	}

	wxFAIL_MSG(
		"IntNameValidator can only be used with wxTextCtrl"
	);

	return NULL;
}

bool IntNameValidator::Validate(wxWindow *parent)
{
	// If window is disabled, simply return
	if ( !m_validatorWindow->IsEnabled() ) return true;

	wxTextEntry * const text = GetTextEntry();
	if ( !text ) return false;

	wxString val(text->GetValue());

	return Validate(parent, val);
}

/// ファイル名の検査
bool IntNameValidator::Validate(wxWindow *parent, const wxString &val)
{
	wxString errormsg;
	wxString invchr;

	if ( val.length() > maxlen )
		errormsg = _("File name is too long.");
	else if ( !ContainsIncludedCharacters(val, invchr) )
		errormsg = wxString::Format(_("The char '%s' is not able to use in file name."), invchr);
	else if ( ContainsExcludedCharacters(val, invchr) )
		errormsg = wxString::Format(_("The char '%s' is not able to use in file name."), invchr);
	else if ( ContainsDuplicatedCharacters(val, invchr) ) {
		errormsg = wxString::Format(_("The char '%s' is duplicated, so is not able to set in file name."), invchr);
	} else if ( item ) {
		if (item->IsFileNameRequired()) {
			wxString name = val;
			name = name.Trim().Trim(true);
			if (name.empty()) {
				errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILENAME_EMPTY]);
			}
		}
		if (errormsg.empty()) {
			if ( item->ConvStringToChars(val, NULL, 32) < 0 ) {
				errormsg = _("Invalid char is contained in file name.");
			}
		}
		if (errormsg.empty()) {
			item->ValidateFileName(parent, val, errormsg);
		}
	}

	if ( !errormsg.empty() ) {
		if (m_validatorWindow) m_validatorWindow->SetFocus();
		wxMessageBox(errormsg, _("Validation conflict"),
			wxOK | wxICON_EXCLAMATION, parent);

		return false;
	}

    return true;
}

bool IntNameValidator::TransferToWindow()
{
	return true;
}

bool IntNameValidator::TransferFromWindow()
{
	return true;
}

#if 0
wxString IntNameValidator::IsValid(const wxString& val) const
{
	if (ContainsExcludedCharacters(val))
		return wxT("invalid");

	return wxEmptyString;
}
#endif

/// 設定できる文字がファイル名に含まれるか
bool IntNameValidator::ContainsIncludedCharacters(const wxString& val, wxString &invchr) const
{
	if (valchrs.IsEmpty()) return true;

	for (wxString::const_iterator i = val.begin(); i != val.end(); i++) {
		if (valchrs.Find(*i) == wxNOT_FOUND) {
			invchr = wxString((wxUniChar)*i);
			return false;
		}
	}
	return true;
}

/// 設定できない文字がファイル名に含まれるか
bool IntNameValidator::ContainsExcludedCharacters(const wxString& val, wxString &invchr) const
{
	for (wxString::const_iterator i = invchrs.begin(); i != invchrs.end(); i++) {
		if (val.Find(*i) != wxNOT_FOUND) {
			invchr = wxString((wxUniChar)*i);
			return true;
		}
	}
	return false;
}

/// 重複指定できない文字がファイル名に含まれるか
bool IntNameValidator::ContainsDuplicatedCharacters(const wxString& val, wxString &invchr) const
{
	int n = 0;
	for (wxString::const_iterator i = dupchrs.begin(); i != dupchrs.end() && n < 2; i++) {
		wxString str = val;
		int found = wxNOT_FOUND;
		n = 0;
		do {
			found = str.Find(*i);
			if (found >= 0) {
				invchr = wxString((wxUniChar)*i);
				str = str.Mid(found + 1);
				n++;
			} else {
				break;
			}
		} while(!str.IsEmpty());
	}
	return (n >= 2);
}

void IntNameValidator::OnChar(wxKeyEvent& event)
{
	if (!m_validatorWindow) {
		event.Skip();
		return;
	}
	int keyCode = event.GetKeyCode();

	if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode >= WXK_START) {
		event.Skip();
		return;
	}

	wxString str((wxUniChar)keyCode, 1);
	wxString invchr;
	if (!ContainsIncludedCharacters(str, invchr)) {
		if ( !wxValidator::IsSilent() )	wxBell();
		return;
	} else if (ContainsExcludedCharacters(str, invchr)) {
		if ( !wxValidator::IsSilent() )	wxBell();
		return;
	} else {
		event.Skip();
	}
}

//////////////////////////////////////////////////////////////////////
//
// 日付用バリデータ
//
DateTimeValidator::DateTimeValidator(bool is_time, bool required)
	: wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
{
	m_is_time = is_time;
	m_require = required;
	SetCharIncludes("0123456789/:.-");
}
DateTimeValidator::DateTimeValidator(const DateTimeValidator &src)
	: wxTextValidator(src)
{
	m_is_time = src.m_is_time;
	m_require = src.m_require;
}

wxObject *DateTimeValidator::Clone() const
{
	return (new DateTimeValidator(*this));
}

bool DateTimeValidator::Validate(wxWindow *parent)
{
    // If window is disabled, simply return
    if (!m_validatorWindow->IsEnabled()) return true;

    wxTextEntry * const text = GetTextEntry();
    if (!text) return true;
	if (!text->IsEditable()) return true;

	wxString val(text->GetValue());
	wxString errormsg;
	struct tm tm;
	bool valid = true;
	if (!val.IsEmpty() && m_require) {
		if (m_is_time) {
			valid = Utils::ConvTimeStrToTm(val, &tm);
			if (!valid) {
				errormsg = _("Invalid format is contained in date or time.");
			}
		} else {
			valid = Utils::ConvDateStrToTm(val, &tm);
			if (!valid) {
				errormsg = _("Invalid format is contained in date or time.");
			}
		}
	}
	if ( !errormsg.empty() ) {
		m_validatorWindow->SetFocus();
		wxMessageBox(errormsg, _("Validation conflict"),
						wxOK | wxICON_WARNING, parent);
		// エラーにしない
//		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//
// アドレス用バリデータ
//
AddressValidator::AddressValidator()
	: wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
{
	SetCharIncludes("0123456789abcdefABCDEF");
}
AddressValidator::AddressValidator(const AddressValidator &src)
	: wxTextValidator(src)
{
}

wxObject *AddressValidator::Clone() const
{
	return (new AddressValidator(*this));
}

bool AddressValidator::Validate(wxWindow *parent)
{
    // If window is disabled, simply return
    if (!m_validatorWindow->IsEnabled()) return true;

    wxTextEntry * const text = GetTextEntry();
    if (!text) return true;
	if (!text->IsEditable()) return true;

	wxString val(text->GetValue());
	wxString errormsg;
	bool valid = true;
	if (!val.IsEmpty()) {
		if (!valid) {
			errormsg = _("Invalid format is contained in address.");
		}
	}
	if ( !errormsg.empty() ) {
		m_validatorWindow->SetFocus();
		wxMessageBox(errormsg, _("Validation conflict"),
						wxOK | wxICON_WARNING, parent);
		// エラーにしない
//		return false;
	}
	return true;
}
