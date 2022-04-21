/// @file sectorbox.cpp
///
/// @brief セクタダイアログ
///

#include "sectorbox.h"
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#include <wx/numformatter.h>
#include "basicfmt.h"
#include "charcodes.h"

// Attach Event
BEGIN_EVENT_TABLE(SectorBox, wxDialog)
	EVT_BUTTON(wxID_OK, SectorBox::OnOK)
END_EVENT_TABLE()

SectorBox::SectorBox(wxWindow* parent, wxWindowID id, int id_c, int id_h, int id_r, int id_n, int sec_nums, bool deleted, bool sdensity)
	: wxDialog(parent, id, _("Sector Information"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flagsR = wxSizerFlags().Expand().Border(wxALL, 4).Border(wxRIGHT, 8);
	wxSize size;
	long style = 0;
	wxTextValidator validate(wxFILTER_EMPTY | wxFILTER_DIGITS);

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

	hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Number Of Sector")), flags);
	txtSecNums = new wxTextCtrl(this, IDC_TEXT_SECNUMS, wxString::Format(wxT("%d"), sec_nums), wxDefaultPosition, wxDefaultSize, style, validate);
	hbox->Add(txtSecNums, flags);

	szrAll->Add(hbox, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int SectorBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void SectorBox::OnOK(wxCommandEvent& event)
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

int SectorBox::GetIdC()
{
	long val;
	txtID_C->GetValue().ToLong(&val);
	return (int)val;
}
int SectorBox::GetIdH()
{
	long val;
	txtID_H->GetValue().ToLong(&val);
	return (int)val;
}
int SectorBox::GetIdR()
{
	long val;
	txtID_R->GetValue().ToLong(&val);
	return (int)val;
}
int SectorBox::GetIdN()
{
	long val;
	txtID_N->GetValue().ToLong(&val);
	return (int)val;
}
int SectorBox::GetSectorNums()
{
	long val;
	txtSecNums->GetValue().ToLong(&val);
	return (int)val;
}
bool SectorBox::GetDeletedMark()
{
	return chkDeleted->GetValue();
}
bool SectorBox::GetSingleDensity()
{
	return chkDensity->GetValue();
}
