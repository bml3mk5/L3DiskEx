/// @file resultbox.cpp
///
/// @brief 結果表示ダイアログ
///

#include "resultbox.h"
//#include <wx/statbmp.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include "fileparam.h"

// Attach Event
BEGIN_EVENT_TABLE(ResultBox, wxDialog)
	EVT_BUTTON(wxID_OK, ResultBox::OnOK)
END_EVENT_TABLE()

ResultBox::ResultBox(wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &msg)
	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

//	wxBitmap label(wxICON());

//	staBmp = new wxStaticBitmap(this, IDC_BITMAP, label);
//	szrAll->Add(staBmp, flags);

	wxSize sz(640, 240);

	txtInfo = new wxTextCtrl(this, IDC_TEXT_INFO, msg, wxDefaultPosition, sz
		, wxTE_MULTILINE | wxTE_READONLY);
	szrAll->Add(txtInfo, flags);

	txtInfo->ShowPosition(0);

	wxSizer *szrButtons = CreateButtonSizer(wxOK);
	szrAll->Add(szrButtons, flags);


	SetSizerAndFit(szrAll);
}

int ResultBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void ResultBox::OnOK(wxCommandEvent& event)
{
	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		this->Show(false);
	}
}

