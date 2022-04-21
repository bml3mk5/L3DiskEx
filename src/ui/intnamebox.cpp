/// @file intnamebox.cpp
///
/// @brief 内部ファイル名ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "intnamebox.h"
#include "intnamevalid.h"
#include "../main.h"
#include <wx/listctrl.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/numformatter.h>
#include "uifilelist.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basicdir.h"
#include "../basicfmt/basicdiritem.h"
#include "../charcodes.h"
#include "../utils.h"


#define INTNAME_LISTCOL_WIDTH	42


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
/// @param [in] date_time  日時(show_flagsが #INTNAME_SPECIFY_CDATE_TIME,#INTNAME_SPECIFY_MDATE_TIME のとき指定)
/// @param [in] show_flags 表示フラグ
IntNameBox::IntNameBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &message,
	DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, const wxString &file_name, int file_size, DiskBasicDirItemAttr *date_time, int show_flags)
	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER, wxT(INTNAMEBOX_CLASSNAME))
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
/// @param [in] date_time  日時(show_flagsが #INTNAME_SPECIFY_CDATE_TIME,#INTNAME_SPECIFY_MDATE_TIME のとき指定)
/// @param [in] show_flags 表示フラグ
void IntNameBox::CreateBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &message,
	DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, const wxString &file_name, int file_size, DiskBasicDirItemAttr *date_time, int show_flags)
{
	this->frame = frame;
	this->item = item;
	this->unique_number = frame->GetUniqueNumber();
	this->basic = basic;

	wxSizerFlags border = wxSizerFlags().Expand().Border(wxALL, 4);
//	wxSizerFlags aleft = wxSizerFlags().Left();
	wxSizerFlags atitle = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);
	wxSize tsize(INTNAME_COLUMN_WIDTH, -1);
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
		szrAll->Add(new wxStaticText(this, wxID_ANY, message), border);
	}

	txtIntName = NULL;
	if (show_flags & INTNAME_SHOW_TEXT) {
		szrAll->Add(new wxStaticText(this, wxID_ANY, _("File Name In The Disk Image")), border);
		size.x = DEFAULT_TEXTWIDTH; size.y = -1;
		if (item->IsFileNameEditable()) {
			IntNameValidator validate(item, _("file name"), basic->GetValidFileName());
			txtIntName = new wxTextCtrl(this, IDC_TEXT_INTNAME, filename, wxDefaultPosition, size, style, validate);
			txtIntName->SetMaxLength((unsigned long)mNameMaxLen);
		} else {
			txtIntName = new wxTextCtrl(this, IDC_TEXT_INTNAME, filename, wxDefaultPosition, size, style | wxTE_READONLY);
		}
		txtIntName->SetFont(font);

		szrAll->Add(txtIntName, border);
	}

	user_data = 0;

	txtStartAddr = NULL;
	txtEndAddr = NULL;
	txtExecAddr = NULL;
	txtCDate = NULL;
	txtCTime = NULL;
	txtMDate = NULL;
	txtMTime = NULL;
	txtADate = NULL;
	txtATime = NULL;
	chkIgnoreDate = NULL;
	txtFileSize = NULL;

//	wxStaticText *sta = NULL;
	wxGridSizer *szrG = NULL;

	if (show_flags & INTNAME_SHOW_ATTR) {
		// 属性の表示は機種依存
		item->CreateControlsForAttrDialog(this, show_flags, file_path, szrAll, border);
	}

	if (show_flags & INTNAME_SHOW_ATTR) {
		// 開始アドレス、終了アドレス、実行アドレス
		int rows;
		if (item->HasAddress()) {
			bool has_exec_addr = item->HasExecuteAddress();

			rows = 2 + (has_exec_addr ? 1 : 0);

			szrG = new wxFlexGridSizer(2, 4, 4);

			CreateAddress(this, IDC_TEXT_START_ADDR, _("Load Address") + _("(Hex)"), 4, tsize, atitle, szrG, txtStartAddr);

			CreateAddress(this, IDC_TEXT_END_ADDR, _("End Address") + _("(Hex)"), 4, tsize, atitle, szrG, txtEndAddr);

			if (has_exec_addr) {
				CreateAddress(this, IDC_TEXT_END_ADDR, _("Execute Address") + _("(Hex)"), 4, tsize, atitle, szrG, txtExecAddr);
			}

			szrAll->Add(szrG, border);
			szrG = NULL;

			// fire on-change event when set start address.
			int addr = -1;
			if (txtStartAddr) {
				addr = item->GetStartAddress();
				if (addr >= 0) txtStartAddr->SetValue(wxString::Format(wxT("%x"), addr));
				else txtStartAddr->SetValue(wxT("----"));
				txtStartAddr->SetEditable(item->IsAddressEditable() && addr >= 0);
			}
			if (txtExecAddr) {
				addr = item->GetExecuteAddress();
				if (addr >= 0) txtExecAddr->SetValue(wxString::Format(wxT("%x"), addr));
				else txtExecAddr->SetValue(wxT("----"));
				txtExecAddr->SetEditable(item->IsAddressEditable() && addr >= 0);
			}
		}
	}

	int rows_of_datetime = 0;
	if (item->HasCreateDateTime()) rows_of_datetime++;
	if (item->HasModifyDateTime()) rows_of_datetime++;
	if (item->HasAccessDateTime()) rows_of_datetime++;

	if (show_flags & INTNAME_SHOW_ATTR) {
		// 日付表示部分
		if (rows_of_datetime > 0) {
			szrG = new wxFlexGridSizer(3, 4, 4);
		}

		for(int row = 0; row < 3; row++) {
			int idx = item->GetFileDateTimeOrder(row);
			switch(idx) {
			case 0:
				// 作成日付を表示
				CreateDateTime(this, IDC_TEXT_CDATE, IDC_TEXT_CTIME, item->GetFileCreateDateTimeTitle()
						, item->HasCreateDateTime(), item->HasCreateDate(), item->HasCreateTime(), (item->CanIgnoreDateTime() & DiskBasicDirItem::DATETIME_CREATE) != 0
						, tsize, atitle, szrG, txtCDate, txtCTime);
				break;
			case 1:
				// 変更日付を表示
				CreateDateTime(this, IDC_TEXT_MDATE, IDC_TEXT_MTIME, item->GetFileModifyDateTimeTitle()
						, item->HasModifyDateTime(), item->HasModifyDate(), item->HasModifyTime(), (item->CanIgnoreDateTime() & DiskBasicDirItem::DATETIME_MODIFY) != 0
						, tsize, atitle, szrG, txtMDate, txtMTime);
				break;
			case 2:
				// アクセス日付を表示
				CreateDateTime(this, IDC_TEXT_ADATE, IDC_TEXT_ATIME, item->GetFileAccessDateTimeTitle()
						, item->HasAccessDateTime(), item->HasAccessDate(), item->HasAccessTime(), (item->CanIgnoreDateTime() & DiskBasicDirItem::DATETIME_ACCESS) != 0
						, tsize, atitle, szrG, txtADate, txtATime);
				break;
			default:
				break;
			}
		}

		if (rows_of_datetime > 0) {
			szrAll->Add(szrG, border);
			szrG = NULL;
		}
	} else {
		// 日付表示部分
		DiskBasicDirItem::enDateTime ignore_type = item->CanIgnoreDateTime();
		if (ignore_type && rows_of_datetime > 0) {
			szrAll->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), border);
		}
	}

	{
		// 日時を無視するか
		DiskBasicDirItem::enDateTime ignore_type = item->CanIgnoreDateTime();
		if (rows_of_datetime > 0) {
			wxString smsg;
			if ((ignore_type & DiskBasicDirItem::DATETIME_ALL) == DiskBasicDirItem::DATETIME_ALL) {
					smsg += _("all date");
			} else {
				if (ignore_type & DiskBasicDirItem::DATETIME_CREATE) {
					if (!smsg.IsEmpty()) smsg += _(" and ");
					smsg += item->GetFileCreateDateTimeTitle();
				}
				if (ignore_type & DiskBasicDirItem::DATETIME_MODIFY) {
					if (!smsg.IsEmpty()) smsg += _(" and ");
					smsg += item->GetFileModifyDateTimeTitle();
				}
				if (ignore_type & DiskBasicDirItem::DATETIME_ACCESS) {
					if (!smsg.IsEmpty()) smsg += _(" and ");
					smsg += item->GetFileAccessDateTimeTitle();
				}
			}

			if (ignore_type) {
				hbox = new wxBoxSizer(wxHORIZONTAL);
				hbox->Add(new wxStaticText(this, wxID_ANY, wxT("    ")), atitle);
				wxString msg = wxString::Format(_("Ignore %s."), smsg);
				chkIgnoreDate = new wxCheckBox(this, IDC_CHK_IGNORE_DATE, msg);
				chkIgnoreDate->SetValue(gConfig.DoesIgnoreDateTime());
				ChangedIgnoreDate(chkIgnoreDate->GetValue());
				hbox->Add(chkIgnoreDate, atitle);
				szrAll->Add(hbox, wxSizerFlags().Expand().Border(wxBOTTOM, 4));
			}
		}
	}

	//

	if (show_flags & INTNAME_SHOW_ATTR) {

		szrG = new wxFlexGridSizer(3, 4, 4);

		// ファイルサイズ
		CreateFileSize(this, IDC_TEXT_FILE_SIZE, _("File Size"), 20, true, tsize, atitle, szrG, txtFileSize);
	}

	// プロパティの場合は詳細表示
	txtGroups = NULL;
	txtGrpSize = NULL;
	lstGroups = NULL;
	if ((show_flags & INTNAME_SHOW_PROPERTY) == 0) {
		if (szrG) szrAll->Add(szrG, border);
		szrG = NULL;
	} else {
		// グループのリスト
		if (!szrG) szrG = new wxFlexGridSizer(3, 4, 4);

		CreateFileSize(this, IDC_TEXT_GROUPS, _("Occupied Groups"), 12, false, tsize, atitle, szrG, txtGroups);

		CreateFileSize(this, IDC_TEXT_GROUP_SIZE, _("Occupied Size"), 20, true, tsize, atitle, szrG, txtGrpSize);

		szrAll->Add(szrG, border);
		szrG = NULL;


		sz.Set(INTNAME_LISTCOL_WIDTH * 6 + 32, -1);
		lstGroups = new wxListCtrl(this, IDC_LIST_GROUPS, wxDefaultPosition, sz, wxLC_REPORT | wxLC_SINGLE_SEL);
		lstGroups->AppendColumn(_("Group"), wxLIST_FORMAT_LEFT, INTNAME_LISTCOL_WIDTH);
		lstGroups->AppendColumn(_("Track"), wxLIST_FORMAT_RIGHT, INTNAME_LISTCOL_WIDTH);
		lstGroups->AppendColumn(_("Side"), wxLIST_FORMAT_RIGHT, INTNAME_LISTCOL_WIDTH);
		lstGroups->AppendColumn(_("Start Sector"), wxLIST_FORMAT_RIGHT, INTNAME_LISTCOL_WIDTH);
		lstGroups->AppendColumn(_("End Sector"), wxLIST_FORMAT_RIGHT, INTNAME_LISTCOL_WIDTH);
		lstGroups->AppendColumn(_("Division"), wxLIST_FORMAT_RIGHT, INTNAME_LISTCOL_WIDTH);
		szrAll->Add(lstGroups, border);

		lstGroups->SetFont(font);
		lstGroups->Bind(wxEVT_LIST_ITEM_SELECTED, &IntNameBox::OnListItemSelected, this);

		lstInternal = NULL;
		if (gConfig.DoesShowInterDirItem()) {
			lstInternal = new wxListCtrl(this, IDC_LIST_INTERNAL, wxDefaultPosition, sz, wxLC_REPORT | wxLC_SINGLE_SEL);
			lstInternal->AppendColumn(_("Name"), wxLIST_FORMAT_LEFT, INTNAME_LISTCOL_WIDTH * 2);
			lstInternal->AppendColumn(_("Value"), wxLIST_FORMAT_LEFT, INTNAME_LISTCOL_WIDTH * 4);
			szrAll->Add(lstInternal, border);

			lstInternal->SetFont(font);
		}
	}

	chkSkipDlg = NULL;
	if (show_flags & INTNAME_SHOW_SKIP_DIALOG) {
		// 以降スキップ
		sz.Set(200, 2);
		szrAll->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, sz, wxLI_HORIZONTAL), border);
		chkSkipDlg = new wxCheckBox(this, IDC_CHK_SKIP_DLG, _("Skip the confirmation after this."));
		chkSkipDlg->SetValue(gConfig.IsSkipImportDialog());
		szrAll->Add(chkSkipDlg, border);
	}

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, border);

	SetSizerAndFit(szrAll);

//	if (show_flags & INTNAME_NEW_FILE) {
//		// 新規作成時、日付は現在日付をセット
//		TM tm_now;
//		wxDateTime::GetTmNow(tm_now);
//		SetCreateDateTime(tm_now);
//		SetModifyDateTime(tm_now);
//		SetAccessDateTime(tm_now);
	if (date_time != NULL) {
		// 日時は引数のものを指定
		if ((show_flags & INTNAME_SPECIFY_CDATE_TIME) != 0) SetCreateDateTime(date_time->GetCreateDateTime());
		if ((show_flags & INTNAME_SPECIFY_MDATE_TIME) != 0) SetModifyDateTime(date_time->GetModifyDateTime());
		if ((show_flags & INTNAME_SPECIFY_ADATE_TIME) != 0) SetAccessDateTime(date_time->GetAccessDateTime());
	} else {
		// アイテム内の日付をセット
		SetCreateDateTime(item->GetFileCreateDateStr(), item->GetFileCreateTimeStr());
		SetModifyDateTime(item->GetFileModifyDateStr(), item->GetFileModifyTimeStr());
		SetAccessDateTime(item->GetFileAccessDateStr(), item->GetFileAccessTimeStr());
	}

	SetFileSize(this->file_size);

	item->InitializeForAttrDialog(this, show_flags, &user_data);

	ChangedType1();
}

/// スタティックテキストを作成する
wxStaticText *IntNameBox::NewStaticText(wxWindow *parent, wxWindowID id, const wxString &label, const wxSize &size)
{
	wxStaticText *sta = new wxStaticText(parent, id, label);
	wxSize msize = sta->GetSizeFromTextSize(size.x);
	if (0 <= msize.x && msize.x < size.x) msize.x = size.x;
	sta->SetMinSize(msize);
	return sta;
}

/// アドレスコントロールの作成
void IntNameBox::CreateAddress(wxWindow *parent, wxWindowID id
	, const wxString &label, int max_length
	, const wxSize &size, const wxSizerFlags &atitle, wxGridSizer *szrG
	, wxTextCtrl * &txt_ctrl)
{
	AddressValidator validate;
	szrG->Add(NewStaticText(parent, wxID_ANY, label, size), atitle);
	txt_ctrl = new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, validate);
	txt_ctrl->SetMaxLength(max_length);
	szrG->Add(txt_ctrl);
}

/// 日付コントロールの作成
void IntNameBox::CreateDateTime(wxWindow *parent, wxWindowID date_id, wxWindowID time_id
		, const wxString &label, bool has_date_time, bool has_date, bool has_time, bool ignore
		, const wxSize &size, const wxSizerFlags &atitle, wxGridSizer *szrG
		, wxTextCtrl * &txt_date, wxTextCtrl * &txt_time)
{
	if (has_date_time) {
		szrG->Add(NewStaticText(parent, wxID_ANY, label, size), atitle);

		if (has_date) {
			DateTimeValidator date_validate(false, !ignore);
			txt_date = new wxTextCtrl(parent, date_id);
			txt_date->SetValidator(date_validate);
			txt_date->SetMinSize(GetDateTextExtent(txt_date));
			txt_date->SetMaxLength(10);
			szrG->Add(txt_date);
		} else {
			szrG->AddSpacer(1);
		}
		if (has_time) {
			DateTimeValidator time_validate(true, !ignore);
			txt_time = new wxTextCtrl(parent, time_id);
			txt_time->SetValidator(time_validate);
			txt_time->SetMinSize(GetTimeTextExtent(txt_time));
			txt_time->SetMaxLength(8);
			szrG->Add(txt_time);
		} else {
			szrG->AddSpacer(1);
		}
	}
}

/// ファイルサイズコントロールの作成
void IntNameBox::CreateFileSize(wxWindow *parent, wxWindowID id
		, const wxString &label, int max_length, bool is_bytes
		, const wxSize &size, const wxSizerFlags &atitle, wxGridSizer *szrG
		, wxTextCtrl * &txt_ctrl)
{
	szrG->Add(NewStaticText(parent, wxID_ANY, label, size), atitle);
	txt_ctrl = new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT | wxTE_READONLY);
	wxSize tsize = txt_ctrl->GetTextExtent(wxString((char)'0', max_length));
	tsize.y = -1;
	txt_ctrl->SetMinSize(tsize);
	szrG->Add(txt_ctrl);
	if (is_bytes) {
		szrG->Add(new wxStaticText(parent, wxID_ANY, _("bytes")));
	} else {
		szrG->AddSpacer(1);
	}
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
	DiskBasicDirItem::enDateTime ignore_type = item->CanIgnoreDateTime();

	if (txtCDate) {
		txtCDate->Enable(!(check && (ignore_type & DiskBasicDirItem::DATETIME_CREATE) != 0));
	}
	if (txtCTime) {
		txtCTime->Enable(!(check && (ignore_type & DiskBasicDirItem::DATETIME_CREATE) != 0));
	}
	if (txtMDate) {
		txtMDate->Enable(!(check && (ignore_type & DiskBasicDirItem::DATETIME_MODIFY) != 0));
	}
	if (txtMTime) {
		txtMTime->Enable(!(check && (ignore_type & DiskBasicDirItem::DATETIME_MODIFY) != 0));
	}
	if (txtADate) {
		txtADate->Enable(!(check && (ignore_type & DiskBasicDirItem::DATETIME_ACCESS) != 0));
	}
	if (txtATime) {
		txtATime->Enable(!(check && (ignore_type & DiskBasicDirItem::DATETIME_ACCESS) != 0));
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

//	SetDateTime(item->GetFileCreateDateStr(), item->GetFileCreateTimeStr());
//	SetFileSize(item->GetFileSize());
}

/// 内部ファイル名を設定
void IntNameBox::SetInternalName(const wxString &name)
{
	if (txtIntName) txtIntName->SetValue(name);
}

/// 内部ファイル名を得る
void IntNameBox::GetInternalName(wxString &name) const
{
	if (!txtIntName) return;

	wxString val = txtIntName->GetValue();
	size_t len = val.Length();
	if (len > mNameMaxLen) {
		val = val.Mid(0, mNameMaxLen);
	}
	if (item) {
		// ダイアログ入力後のファイル名文字列を大文字にする
		if (basic->ToUpperAfterRenamed()) {
			val.MakeUpper();
		}
//		// ダイアログ入力後のファイル名文字列を変換 機種依存の処理
//		item->ConvertFileNameAfterRenamed(val);
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
		if (val >= 0) ctrl->SetValue(wxString::Format(wxT("%x"), (val & 0xffff)));
		else ctrl->SetValue(wxT("----"));
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
		val = Utils::ConvFromHexa(sval);
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

/// 作成日時を得る
void IntNameBox::GetCreateDateTime(TM &tm) const
{
	tm.AllZero();

	if (txtCDate) {
		Utils::ConvDateStrToTm(txtCDate->GetValue(), tm);
	}
	if (txtCTime) {
		Utils::ConvTimeStrToTm(txtCTime->GetValue(), tm);
	}
}

/// 作成日時を返す
TM IntNameBox::GetCreateDateTime() const
{
	TM tm;
	GetCreateDateTime(tm);
	return tm;
}

/// 作成日時を設定
void IntNameBox::SetCreateDateTime(const wxString &date, const wxString &time)
{
	if (txtCDate && !date.IsEmpty()) {
		txtCDate->SetValue(date);
	}
	if (txtCTime && !time.IsEmpty()) {
		txtCTime->SetValue(time);
	}
}

/// 作成日時を設定
void IntNameBox::SetCreateDateTime(const TM &tm)
{
	wxString sdate = Utils::FormatYMDStr(tm);
	wxString stime = Utils::FormatHMSStr(tm);
	SetCreateDateTime(sdate, stime);
}

/// 変更日時を得る
void IntNameBox::GetModifyDateTime(TM &tm) const
{
	tm.AllZero();

	if (txtMDate) {
		Utils::ConvDateStrToTm(txtMDate->GetValue(), tm);
	}
	if (txtMTime) {
		Utils::ConvTimeStrToTm(txtMTime->GetValue(), tm);
	}
}

/// 変更日時を返す
TM IntNameBox::GetModifyDateTime() const
{
	TM tm;
	GetModifyDateTime(tm);
	return tm;
}

/// 変更日時を設定
void IntNameBox::SetModifyDateTime(const wxString &date, const wxString &time)
{
	if (txtMDate && !date.IsEmpty()) {
		txtMDate->SetValue(date);
	}
	if (txtMTime && !time.IsEmpty()) {
		txtMTime->SetValue(time);
	}
}

/// 変更日時を設定
void IntNameBox::SetModifyDateTime(const TM &tm)
{
	wxString sdate = Utils::FormatYMDStr(tm);
	wxString stime = Utils::FormatHMSStr(tm);
	SetModifyDateTime(sdate, stime);
}

/// アクセス日時を得る
void IntNameBox::GetAccessDateTime(TM &tm) const
{
	tm.AllZero();

	if (txtADate) {
		Utils::ConvDateStrToTm(txtADate->GetValue(), tm);
	}
	if (txtATime) {
		Utils::ConvTimeStrToTm(txtATime->GetValue(), tm);
	}
}

/// アクセス日時を返す
TM IntNameBox::GetAccessDateTime() const
{
	TM tm;
	GetAccessDateTime(tm);
	return tm;
}

/// アクセス日時を設定
void IntNameBox::SetAccessDateTime(const wxString &date, const wxString &time)
{
	if (txtADate && !date.IsEmpty()) {
		txtADate->SetValue(date);
	}
	if (txtATime && !time.IsEmpty()) {
		txtATime->SetValue(time);
	}
}

/// アクセス日時を設定
void IntNameBox::SetAccessDateTime(const TM &tm)
{
	wxString sdate = Utils::FormatYMDStr(tm);
	wxString stime = Utils::FormatHMSStr(tm);
	SetAccessDateTime(sdate, stime);
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
		ConvFileSize(val, str);
		txtFileSize->SetValue(str);
	}
}

/// ファイルサイズをフォーマット
void IntNameBox::ConvFileSize(long val, wxString &str)
{
	if (val >= 0) {
		str = wxNumberFormatter::ToString(val);
		str += wxString::Format(wxT(" (0x%x)"), (int)val);
	} else {
		str = wxT("---");
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

/// 内部データを設定
void IntNameBox::SetInternalDatas(const KeyValArray &vals)
{
	if (lstInternal) {
		lstInternal->DeleteAllItems();

		long row = 0;
		lstInternal->InsertItem(row, wxT("size"));
		wxSize sz = this->GetSize();
		lstInternal->SetItem(row, 1, wxString::Format(wxT("(%d,%d)"), sz.GetX(), sz.GetY()));
		row++;

		for(size_t i = 0; i < vals.Count(); i++, row++) {
			const KeyValItem *item = vals.Item(i);
			lstInternal->InsertItem(row, item->Key());
			lstInternal->SetItem(row, 1, item->GetValueString());
		}
	}
}

/// 以降スキップを返す
bool IntNameBox::IsSkipDialog(bool def_val) const
{
	return (chkSkipDlg ? chkSkipDlg->IsChecked() : def_val);
}
