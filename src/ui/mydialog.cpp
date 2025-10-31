/// @file mydialog.cpp
///
/// @brief 基本ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "mydialog.h"
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/notebook.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/spinctrl.h>

MyDialog::MyDialog(wxWindow* parent, wxWindowID id, const wxString &title)
	: wxDialog(parent, id, title, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
}

wxCheckBox *MyDialog::CreateCheckBoxH(wxWindow *parent, wxWindowID id, const wxString &label, bool val, wxBoxSizer *sizer, const wxSizerFlags &flags)
{
	wxBoxSizer *szrH = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox *chk = new wxCheckBox(parent, id, label);
	chk->SetValue(val);
	szrH->Add(chk, flags);
	sizer->Add(szrH, flags);
	return chk;
}

wxSpinCtrl *MyDialog::CreateSpinCtrlH(wxWindow *parent, wxWindowID id, const wxString &label, int val, wxBoxSizer *sizer, const wxSizerFlags &flags)
{
	return CreateSpinCtrlH(parent, id, label, 1, 100, val, wxEmptyString, sizer, flags);
}

wxSpinCtrl *MyDialog::CreateSpinCtrlH(wxWindow *parent, wxWindowID id, const wxString &label, int minval, int maxval, int val, const wxString &postfix, wxBoxSizer *sizer, const wxSizerFlags &flags)
{
	wxBoxSizer *szrH = new wxBoxSizer(wxHORIZONTAL);
	szrH->Add(new wxStaticText(parent, wxID_ANY, label), flags);
	wxSpinCtrl *spn = new wxSpinCtrl(parent, id);
	spn->SetRange(minval, maxval);
	spn->SetValue(val);
	szrH->Add(spn, flags);
	if (!postfix.IsEmpty()) {
		szrH->Add(new wxStaticText(parent, wxID_ANY, postfix), flags);
	}
	sizer->Add(szrH);
	return spn;
}

wxTextCtrl *MyDialog::CreateTextCtrlWithButtonBox(wxWindow *parent, const wxString &boxlabel, wxWindowID textid, const wxString &text, wxWindowID btnid, const wxString &btnlabel, wxBoxSizer *sizer, const wxSizerFlags &flags)
{
	wxStaticBoxSizer *bszr = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, boxlabel), wxVERTICAL);
	wxTextCtrl *txt = CreateTextCtrlWithButton(parent, textid, text, btnid, btnlabel, NULL, bszr, flags);
	sizer->Add(bszr, flags);
	return txt;
}

wxTextCtrl *MyDialog::CreateTextCtrlWithButton(wxWindow *parent, wxWindowID textid, const wxString &text, wxWindowID btnid, const wxString &btnlabel, wxButton **button, wxBoxSizer *sizer, const wxSizerFlags &flags)
{
	wxBoxSizer *szrH = new wxBoxSizer(wxHORIZONTAL);
	wxTextCtrl *txt = new wxTextCtrl(parent, textid, text, wxDefaultPosition, wxSize(320, -1));
	szrH->Add(txt, flags);
	wxButton *btn = new wxButton(parent, btnid, btnlabel);
	szrH->Add(btn, flags);
	sizer->Add(szrH);
	if (button) *button = btn;
	return txt;
}

