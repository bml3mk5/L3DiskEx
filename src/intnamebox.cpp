/// @file intnamebox.cpp
///
/// @brief 内部ファイル名ダイアログ
///

#include "intnamebox.h"
#include <wx/numformatter.h>
#include "basicfmt.h"
#include "charcodes.h"

// Attach Event
BEGIN_EVENT_TABLE(IntNameBox, wxDialog)
	EVT_BUTTON(wxID_OK, IntNameBox::OnOK)
	EVT_RADIOBOX(IDC_RADIO_TYPE1, IntNameBox::OnChangeType1)
END_EVENT_TABLE()

IntNameBox::IntNameBox(wxWindow* parent, wxWindowID id, const wxString &caption, int format_type, const wxString &name, size_t name_len, int file_type, int data_type, bool isproperty)
	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	this->format_type = format_type;
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSize size;
	long style = 0;
	mNameMaxLen = name_len;
	IntNameValidator validate(name_len);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	szrAll->Add(new wxStaticText(this, wxID_ANY, _("File Name In The Disk Image")), flags);
	size.x = DEFAULT_TEXTWIDTH; size.y = -1;
	txtIntName = new wxTextCtrl(this, IDC_TEXT_INTNAME, name, wxDefaultPosition, size, style, validate);
	txtIntName->SetMaxLength((unsigned long)mNameMaxLen);
	szrAll->Add(txtIntName, flags);

	wxArrayString types1;
	for(size_t i=0; !gTypeName1[i].IsEmpty(); i++) {
		types1.Add(wxGetTranslation(gTypeName1[i]));
	}
	radType1 = new wxRadioBox(this, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 0, wxRA_SPECIFY_COLS);
	radType1->SetSelection(file_type);
	szrAll->Add(radType1, flags);

	wxArrayString types2;
	for(size_t i=0; !gTypeName2[i].IsEmpty(); i++) {
		types2.Add(wxGetTranslation(gTypeName2[i]));
	}
	if (format_type == 0) {
		// 1S
		types2.Add(wxGetTranslation(gTypeName3[1]));
	} else {
		if (data_type == DISKBASIC_DATATYPE_RANDOM) {
			// 1Sから2Dへのコピーでランダムアクセスのデータはアスキーとする。
			data_type = DISKBASIC_DATATYPE_ASCII;
		}
	}
	radType2 = new wxRadioBox(this, IDC_RADIO_TYPE2, _("Data Type"), wxDefaultPosition, wxDefaultSize, types2, 0, wxRA_SPECIFY_COLS);
	radType2->SetSelection(data_type);
	szrAll->Add(radType2, flags);

	// プロパティの場合は詳細表示
	if (isproperty) {
		// ファイルサイズ
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("File Size:")), flags);
		txtFileSize = new wxTextCtrl(this, IDC_TEXT_FILE_SIZE, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT | wxTE_READONLY | wxBORDER_NONE);
		hbox->Add(txtFileSize, flags);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("bytes")), flags);
		szrAll->Add(hbox, flags);
		// グループのリスト
		hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("Occupied Groups:")), flags);
		txtGroups = new wxTextCtrl(this, IDC_TEXT_GROUPS, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_RIGHT | wxTE_READONLY | wxBORDER_NONE);
		hbox->Add(txtGroups, flags);
		szrAll->Add(hbox, flags);

		lstGroups = new wxListCtrl(this, IDC_LIST_GROUPS, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
		lstGroups->AppendColumn(_("Group"), wxLIST_FORMAT_LEFT, 40);
		lstGroups->AppendColumn(_("Track"), wxLIST_FORMAT_RIGHT, 40);
		lstGroups->AppendColumn(_("Side"), wxLIST_FORMAT_RIGHT, 40);
		lstGroups->AppendColumn(_("Start Sector"), wxLIST_FORMAT_RIGHT, 40);
		lstGroups->AppendColumn(_("End Sector"), wxLIST_FORMAT_RIGHT, 40);
		szrAll->Add(lstGroups, flags);

	} else {
		txtFileSize = NULL;
		txtGroups = NULL;
		lstGroups = NULL;
	}

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);

	ChangedType1(file_type);
}

int IntNameBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void IntNameBox::OnOK(wxCommandEvent& event)
{
	if (Validate() && TransferDataFromWindow()) {
		if (IsModal()) {
			EndModal(wxID_OK);
		} else {
			SetReturnCode(wxID_OK);
			this->Show(false);
		}
	}
}

void IntNameBox::OnChangeType1(wxCommandEvent& event)
{
	int pos = event.GetSelection();
	ChangedType1(pos);
}

void IntNameBox::ChangedType1(int sel_idx)
{
	if (!radType2) return;
	int cnt = (int)radType2->GetCount();
	int cur_pos = radType2->GetSelection();

	radType2->Enable(0, true);
	radType2->Enable(1, true);
	if (cnt > 2) radType2->Enable(2, true);

	if (sel_idx == 0) {
		// BASIC
		if (cnt > 2) {
			if (cur_pos == 2) {
				radType2->SetSelection(0);
			}
			radType2->Enable(2, false);	// ランダムアクセス指定不可
		}
	} else if (sel_idx == 1) {
		// データ
		if (cur_pos == 0) {
			radType2->SetSelection(1);
		}
		radType2->Enable(0, false);	// バイナリ指定不可
	} else if (sel_idx == 2) {
		// 機械語
		radType2->SetSelection(0);
		radType2->Enable(1, false);	// アスキー指定不可
		if (cnt > 2) {
			radType2->Enable(2, false);	// ランダムアクセス指定不可
		}
	}
	// 拡張子を付加
	AddExtension(sel_idx);
}

void IntNameBox::AddExtension(int file_type)
{
	if (format_type == 0) return;
	wxString intname = txtIntName->GetValue();
	int len = (int)intname.Length();
	wxString ext = intname.Right(4).Upper();
	bool match = (ext == wxT(".BAS") || ext == wxT(".DAT") || ext == wxT(".BIN"));
	if (match) {
		len -= 4;
		if (len >= 0) intname = intname.Left(len);
		else intname.Empty();
	} else {
		len = intname.Find('.', false);
		if (len >= 0) return;
	}
	switch(file_type) {
	case 1:
		intname += wxT(".DAT");
		break;
	case 2:
		intname += wxT(".BIN");
		break;
	default:
		intname += wxT(".BAS");
		break;
	}
	txtIntName->SetValue(intname);
}

void IntNameBox::SetInternalName(const wxString &item)
{
	txtIntName->SetValue(item);
}
wxString IntNameBox::GetInternalName() const
{
	wxString val = txtIntName->GetValue();
	size_t len = val.Length();
	if (len > mNameMaxLen) {
		val = val.Mid(0, mNameMaxLen);
	}
	return val;
}
int IntNameBox::GetFileType() const
{
	return radType1->GetSelection();
}
int IntNameBox::GetDataType() const
{
	return radType2->GetSelection();
}
void IntNameBox::SetFileSize(long val)
{
	if (txtFileSize) {
		txtFileSize->SetValue(val >= 0 ? wxNumberFormatter::ToString(val) : wxT("---"));
	}
}
void IntNameBox::SetGroups(long val, DiskBasicGroupItems &vals)
{
	if (txtGroups) {
		txtGroups->SetValue(val >= 0 ? wxNumberFormatter::ToString(val) : wxT("---"));
	}
	if (lstGroups) {
		lstGroups->DeleteAllItems();

		for(size_t i=0; i < vals.Count(); i++) {
			lstGroups->InsertItem(i, wxString::Format(wxT("%02x"), vals[i].group));
			lstGroups->SetItem(i, 1, wxString::Format(wxT("%d"), vals[i].track));
			lstGroups->SetItem(i, 2, wxString::Format(wxT("%d"), vals[i].side));
			lstGroups->SetItem(i, 3, wxString::Format(wxT("%d"), vals[i].sector_start));
			lstGroups->SetItem(i, 4, wxString::Format(wxT("%d"), vals[i].sector_end));
		}
	}
}

//
//
//
// Attach Events
wxBEGIN_EVENT_TABLE(IntNameValidator, wxValidator)
    EVT_CHAR(IntNameValidator::OnChar)
wxEND_EVENT_TABLE()

IntNameValidator::IntNameValidator(size_t maxlength) : wxValidator()
{
	invchrs.Add("\"");
	invchrs.Add("(");
	invchrs.Add(")");
	invchrs.Add(";");
	maxlen = maxlength;
}
IntNameValidator::IntNameValidator(const IntNameValidator& val) : wxValidator()
{
    Copy(val);
}

bool IntNameValidator::Copy(const IntNameValidator& val)
{
    wxValidator::Copy(val);
	invchrs = val.invchrs;
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

	wxString errormsg;

	if ( val.empty() )
		errormsg = _("File name is required.");
	else if ( val.length() > maxlen )
		errormsg = _("File name is too long.");
	else if ( invchrs.Index(val) != wxNOT_FOUND )
		errormsg = _("Invalid char is contained in file name.");
	else if ( !(errormsg = IsValid(val)).empty() ) {
		errormsg = _("Invalid char is contained in file name.");
	} else if ( !gCharCodes.ConvToChars(val, NULL, 20) ) {
		errormsg = _("Invalid char is contained in file name.");
	}

	if ( !errormsg.empty() ) {
		m_validatorWindow->SetFocus();
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

wxString IntNameValidator::IsValid(const wxString& val) const
{
	if (ContainsExcludedCharacters(val))
		return wxT("invalid");

	return wxEmptyString;
}

bool IntNameValidator::ContainsExcludedCharacters(const wxString& val) const
{
	for ( wxString::const_iterator i = val.begin(); i != val.end(); ++i )
		if (invchrs.Index((wxString) *i) != wxNOT_FOUND)
			return true;

	return false;
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
	if (!IsValid(str).empty()) {
		if ( !wxValidator::IsSilent() )	wxBell();
		// eat message
		return;
	} else {
		event.Skip();
	}
}
