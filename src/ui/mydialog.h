/// @file mydialog.h
///
/// @brief 基本ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef MYDIALOG_H
#define MYDIALOG_H

#include "../common.h"
#include <wx/dialog.h>
#include <wx/dynarray.h>

class wxBoxSizer;
class wxSizerFlags;
class wxCheckBox;
class wxTextCtrl;
class wxChoice;
class wxStaticText;
class wxSpinCtrl;
class wxButton;

/// 基本ダイアログ
class MyDialog : public wxDialog
{
private:

public:
	MyDialog(wxWindow* parent, wxWindowID id, const wxString &title);

	wxCheckBox *CreateCheckBoxH(wxWindow *parent, wxWindowID id, const wxString &label, bool val, wxBoxSizer *sizer, const wxSizerFlags &flags);

	wxSpinCtrl *CreateSpinCtrlH(wxWindow *parent, wxWindowID id, const wxString &label, int val, wxBoxSizer *sizer, const wxSizerFlags &flags);
	wxSpinCtrl *CreateSpinCtrlH(wxWindow *parent, wxWindowID id, const wxString &label, int minval, int maxval, int val, const wxString &postfix, wxBoxSizer *sizer, const wxSizerFlags &flags);

	wxTextCtrl *CreateTextCtrlWithButtonBox(wxWindow *parent, const wxString &boxlabel, wxWindowID textid, const wxString &text, wxWindowID btnid, const wxString &btnlabel, wxBoxSizer *sizer, const wxSizerFlags &flags);
	wxTextCtrl *CreateTextCtrlWithButton(wxWindow *parent, wxWindowID textid, const wxString &text, wxWindowID btnid, const wxString &btnlabel, wxButton **button, wxBoxSizer *sizer, const wxSizerFlags &flags);
};

#endif /* MYDIALOG_H */

