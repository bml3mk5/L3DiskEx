/// @file fontminibox.cpp
///
/// @brief フォントミニダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "fontminibox.h"
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#include <wx/fontenum.h>


// Attach Event
BEGIN_EVENT_TABLE(FontMiniBox, wxDialog)
	EVT_TEXT(IDC_COMBO_FONTSIZE, FontMiniBox::OnTextSize)
	EVT_BUTTON(IDC_BUTTON_DEFAULT, FontMiniBox::OnButtonDefault)
END_EVENT_TABLE()

FontMiniBox::FontMiniBox(wxWindow* parent, wxWindowID id, const wxFont &default_font)
	: wxDialog(parent, id, _("Font"), wxDefaultPosition, wxDefaultSize, wxCAPTION)
{
	mDefaultFont = default_font;

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSize size;

	wxTextValidator tVali(wxFILTER_NUMERIC);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	size.x = DEFAULT_TEXTWIDTH * 2; size.y = -1;
	comFontName = new wxComboBox(this, IDC_COMBO_FONTNAME, wxEmptyString, wxDefaultPosition, size, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	size.x = 80; size.y = -1;
	comFontSize = new wxComboBox(this, IDC_COMBO_FONTSIZE, wxEmptyString, wxDefaultPosition, size, 0, NULL, wxCB_DROPDOWN, tVali);
	hbox->Add(comFontName, flags);
	hbox->Add(comFontSize, flags);
	szrAll->Add(hbox, flags);

	wxGridSizer *gszr = new wxGridSizer(2);
	btnDefault = new wxButton(this, IDC_BUTTON_DEFAULT, _("Default"));
	gszr->Add(btnDefault);
	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	gszr->Add(szrButtons, wxSizerFlags().Align(wxALIGN_RIGHT));
	szrAll->Add(gszr, flags);

	init_dialog();

	SetSizerAndFit(szrAll);
}

int FontMiniBox::ShowModal()
{
	int rc = wxDialog::ShowModal();
	if (rc == wxID_OK) {
		term_dialog();
	}
	return rc;
}

void FontMiniBox::init_dialog()
{
	mFontNames = wxFontEnumerator::GetFacenames();
	if (mFontNames.Index(mDefaultFont.GetFaceName()) == wxNOT_FOUND) {
		mFontNames.Add(mDefaultFont.GetFaceName());
	}
	mFontNames.Sort();

	wxString size;
	mFontSizes.Empty();
	for(int i=4; i<=24; i++) {
		size.Printf(_T("%d"), i);
		mFontSizes.Add(size);
	}

	comFontName->Clear();
	comFontName->Insert(mFontNames, 0);
	comFontSize->Clear();
	comFontSize->Insert(mFontSizes, 0);

}

void FontMiniBox::SetFontName(const wxString &val)
{
	mSelectedName = val;
	comFontName->SetValue(mSelectedName);
}

void FontMiniBox::SetFontSize(int val)
{
	wxString size;
	mSelectedSize = val;
	size.Printf(_T("%d"), mSelectedSize);
	comFontSize->SetValue(size);
}

void FontMiniBox::term_dialog()
{
	mSelectedName = comFontName->GetValue();
	long val;
	comFontSize->GetValue().ToLong(&val);
	if (val < 1) val = 1;
	if (val > 99) val = 99;
	mSelectedSize = (int)val;
}

void FontMiniBox::OnTextSize(wxCommandEvent& event)
{
	wxString mInputSize = event.GetString().Left(2);
	comFontSize->ChangeValue(mInputSize);
}

void FontMiniBox::OnButtonDefault(wxCommandEvent& event)
{
	SetFontName(mDefaultFont.GetFaceName());
	SetFontSize(mDefaultFont.GetPointSize());
}
