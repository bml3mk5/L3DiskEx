/// @file rawsectorbox.cpp
///
/// @brief セクタダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "rawsectorbox.h"
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#include <wx/numformatter.h>
#include "../basicfmt/basicfmt.h"
#include "../charcodes.h"


// Attach Event
BEGIN_EVENT_TABLE(RawSectorBox, wxDialog)
	EVT_BUTTON(wxID_OK, RawSectorBox::OnOK)
END_EVENT_TABLE()

RawSectorBox::RawSectorBox(wxWindow* parent, wxWindowID id, const wxString &caption, int id_c, int id_h, int id_r, int id_n, int sec_nums, bool deleted, bool sdensity, int status, int hide_flags)
	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flagsR = wxSizerFlags().Expand().Border(wxALL, 4).Border(wxRIGHT, 8);
	wxSize size;
	long style = 0;
	wxTextValidator validate(wxFILTER_EMPTY | wxFILTER_DIGITS);
	wxTextValidator anvalidate(wxFILTER_EMPTY | wxFILTER_ALPHANUMERIC);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	size.x = 32; size.y = -1;
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT("C")), flags);
	txtID_C = new wxTextCtrl(this, IDC_TEXT_ID_C, wxString::Format(wxT("%d"), id_c), wxDefaultPosition, size, style, validate);
	hbox->Add(txtID_C, flagsR);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT("H")), flags);
	txtID_H = new wxTextCtrl(this, IDC_TEXT_ID_H, wxString::Format(wxT("%d"), id_h), wxDefaultPosition, size, style, validate);
	hbox->Add(txtID_H, flagsR);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT("R")), flags);
	txtID_R = new wxTextCtrl(this, IDC_TEXT_ID_R, wxString::Format(wxT("%d"), id_r), wxDefaultPosition, size, style, validate);
	hbox->Add(txtID_R, flagsR);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT("N")), flags);
	txtID_N = new wxTextCtrl(this, IDC_TEXT_ID_N, wxString::Format(wxT("%d"), id_n), wxDefaultPosition, size, style, validate);
	hbox->Add(txtID_N, flagsR);

	szrAll->Add(hbox, flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkDeleted = new wxCheckBox(this, IDC_CHK_DELETED, _("Deleted Mark"));
	chkDeleted->SetValue(deleted);
	hbox->Add(chkDeleted, flags);

	chkDensity = new wxCheckBox(this, IDC_CHK_DENSITY, _("Single Density"));
	chkDensity->SetValue(sdensity);
	hbox->Add(chkDensity, flags);

	szrAll->Add(hbox, flags);

	txtSecNums = NULL;
	if ((hide_flags & SECTORBOX_HIDE_SECTOR_NUMS) == 0) {
		hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("Number Of Sector")), flags);
		txtSecNums = new wxTextCtrl(this, IDC_TEXT_SECNUMS, wxString::Format(wxT("%d"), sec_nums), wxDefaultPosition, wxDefaultSize, style, validate);
		hbox->Add(txtSecNums, flags);

		szrAll->Add(hbox, flags);
	}

	hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Status (Hex)")), flags);
	txtStatus = new wxTextCtrl(this, IDC_TEXT_STATUS, wxString::Format(wxT("%x"), status), wxDefaultPosition, size, style, anvalidate);
	hbox->Add(txtStatus, flags);

	szrAll->Add(hbox, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int RawSectorBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void RawSectorBox::OnOK(wxCommandEvent& event)
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

int RawSectorBox::GetIdC()
{
	long val;
	txtID_C->GetValue().ToLong(&val);
	return (int)val;
}
int RawSectorBox::GetIdH()
{
	long val;
	txtID_H->GetValue().ToLong(&val);
	return (int)val;
}
int RawSectorBox::GetIdR()
{
	long val;
	txtID_R->GetValue().ToLong(&val);
	return (int)val;
}
int RawSectorBox::GetIdN()
{
	long val;
	txtID_N->GetValue().ToLong(&val);
	return (int)val;
}
int RawSectorBox::GetSectorNums()
{
	long val = 0;
	if (txtSecNums) txtSecNums->GetValue().ToLong(&val);
	return (int)val;
}
bool RawSectorBox::GetDeletedMark()
{
	return chkDeleted->GetValue();
}
bool RawSectorBox::GetSingleDensity()
{
	return chkDensity->GetValue();
}
int RawSectorBox::GetStatus()
{
	long val = 0;
	txtStatus->GetValue().ToLong(&val, 16);
	return (int)val;
}
