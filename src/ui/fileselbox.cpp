/// @file fileselbox.cpp
///
/// @brief ファイル種類選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "fileselbox.h"
#include <wx/listbox.h>
#include <wx/sizer.h>
#include "../diskimg/fileparam.h"


// Attach Event
BEGIN_EVENT_TABLE(FileSelBox, wxDialog)
//	EVT_CHOICE(IDC_COMBO_FILE, FileSelBox::OnFileChanged)
	EVT_BUTTON(wxID_OK, FileSelBox::OnOK)
END_EVENT_TABLE()

FileSelBox::FileSelBox(wxWindow* parent, wxWindowID id)
	: wxDialog(parent, id, _("Select File Type"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	comFile = new wxListBox(this, IDC_COMBO_FILE, wxDefaultPosition, wxDefaultSize);
	szrAll->Add(comFile, flags);

	const FileFormats *fmts = &gFileTypes.GetFormats();
	for(size_t n = 0; n < fmts->Count(); n++) {
		FileFormat *fmt = &fmts->Item(n);
		if (!fmt) continue;
		comFile->Append(fmt->GetDescription());
	}
	comFile->SetSelection(0);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int FileSelBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void FileSelBox::OnOK(wxCommandEvent& event)
{
	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		this->Show(false);
	}
}

int FileSelBox::GetSelection() const
{
	return comFile->GetSelection();
}

wxString FileSelBox::GetFormatType() const
{
	wxString type;
	const FileFormats *fmts = &gFileTypes.GetFormats();
	for(size_t n = 0; n < fmts->Count(); n++) {
		FileFormat *fmt = &fmts->Item(n);
		if (!fmt) continue;
		if (n == (size_t)comFile->GetSelection()) {
			type = fmt->GetName();
			break;
		}
	}
	return type;
}
