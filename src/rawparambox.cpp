/// @file rawparambox.cpp
///
/// @brief Rawパラメータダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "rawparambox.h"
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/radiobox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h>


// Attach Event
BEGIN_EVENT_TABLE(RawParamBox, wxDialog)
	EVT_BUTTON(wxID_OK, RawParamBox::OnOK)
END_EVENT_TABLE()

RawParamBox::RawParamBox(wxWindow* parent, wxWindowID id, int type, int value, int maxvalue)
	: wxDialog(parent, id, _("Modify Parameter"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	long style = 0;
	wxTextValidator validate(wxFILTER_EMPTY | wxFILTER_DIGITS);

	this->type = type;
	this->maxvalue = maxvalue;

	wxArrayString typenames;
	typenames.Add(wxT("ID C"));
	typenames.Add(wxT("ID H"));
	typenames.Add(wxT("ID R"));
	typenames.Add(wxT("ID N"));
	typenames.Add(_("NumOfSectors"));
	typenames.Add(_("SectorSize"));
	const int maxlens[] = { 2, 1, 2, 2, 3, 4 };

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	wxString typestr = typenames[type] + wxT(" : ") + wxString::Format(wxT("%d  --> "), value);
	hbox->Add(new wxStaticText(this, wxID_ANY, typestr), flags);
	txtValue = NULL;
	comValue = NULL;
	if (type != TYPE_SECTOR_SIZE) {
		txtValue = new wxTextCtrl(this, IDC_TEXT_VALUE, wxEmptyString, wxDefaultPosition, wxDefaultSize, style, validate);
		txtValue->SetMaxLength(maxlens[type]);
		hbox->Add(txtValue, 0);
	} else {
		// choose sector size
		wxArrayString choices;
		int choice = 0;
		for (int i=0; i<=4; i++) {
			int nsize = (128 * (1 << i));
			choices.Add(wxString::Format(wxT("%d"), nsize));
			if (value == nsize) {
				choice = i;
			}
		}
		comValue = new wxChoice(this, IDC_COMBO_VALUE, wxDefaultPosition, wxDefaultSize, choices);
		comValue->SetSelection(choice);
		hbox->Add(comValue, 0);
	}

	szrAll->Add(hbox, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int RawParamBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void RawParamBox::OnOK(wxCommandEvent& event)
{
	if (Validate() && TransferDataFromWindow() && ValidateParam()) {
		if (IsModal()) {
			EndModal(wxID_OK);
		} else {
			SetReturnCode(wxID_OK);
			this->Show(false);
		}
	}
}

bool RawParamBox::ValidateParam()
{
	int val = GetValue();
	if (val > maxvalue) {
		wxString msg = wxString::Format(_("The value need less equal %d."), maxvalue);
		wxMessageBox(msg, _("Error"), wxOK | wxICON_EXCLAMATION); 
		return false;
	}

	return true;
}

int RawParamBox::GetValue()
{
	long val = 0;
	if (txtValue) {
		txtValue->GetValue().ToLong(&val);
	} else if (comValue) {
		int sel = comValue->GetSelection();
		val = 128 * (1 << sel);
	}
	return (int)val;
}


// Attach Event
BEGIN_EVENT_TABLE(DensityParamBox, wxDialog)
END_EVENT_TABLE()

DensityParamBox::DensityParamBox(wxWindow* parent, wxWindowID id, bool sdensity)
	: wxDialog(parent, id, _("Density Parameter"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
//	long style = 0;

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	wxArrayString arr;
	arr.Add(_("Double Density"));
	arr.Add(_("Single Density"));
	radDensity = new wxRadioBox(this, IDC_RADIO_DENSITY, _("Density"), wxDefaultPosition, wxDefaultSize, arr);
	hbox->Add(radDensity, flags);

	szrAll->Add(hbox, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);

	radDensity->SetSelection(sdensity ? 1 : 0);
}

bool DensityParamBox::IsSingleDensity()
{
	return (radDensity->GetSelection() == 1);
}
