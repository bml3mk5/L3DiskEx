/// @file aboutbox.cpp
///
/// @brief Aboutダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "aboutbox.h"

#ifndef USE_CONSOLE

#include "../main.h"
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/sizer.h>
#include "../version.h"


//////////////////////////////////////////////////////////////////////
//
// About dialog
//
UiDiskAbout::UiDiskAbout(wxWindow* parent, wxWindowID id)
	: wxDialog(parent, id, _("About..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, FromDIP(4));

	wxBoxSizer *szrLeft   = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrRight  = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrMain   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *szrAll    = new wxBoxSizer(wxVERTICAL);

	szrLeft->Add(new wxStaticBitmap(this, wxID_ANY,
		wxBitmap(APPLICATION_XPMICON_NAME), wxDefaultPosition, wxSize(64, 64))
		, flags);

	wxString str = _T(APPLICATION_FULLNAME);
	str += _T(", Version ");
	str += _T(APPLICATION_VERSION);
	str += _T(" \"");
	str += _T(PLATFORM);
	str += _T("\"\n\n");
#ifdef _DEBUG
	str += _T("(Debug Version)\n\n");
#endif
	str	+= _T("using ");
	str += wxVERSION_STRING;
	str += _T("\n\n");
	str	+= _T(APP_COPYRIGHT);

	szrRight->Add(new wxStaticText(this, wxID_ANY, str), flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK);
	szrMain->Add(szrLeft, flags);
	szrMain->Add(szrRight, flags);
	szrAll->Add(szrMain, flags);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

#endif /* !USE_CONSOLE */
