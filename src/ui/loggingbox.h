/// @file loggingbox.h
///
/// @brief ロギングダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef LOGGINGBOX_H
#define LOGGINGBOX_H

#include "../common.h"
#include <wx/dialog.h>

class wxSizerItem;
class wxTextCtrl;

/// ロギングボックス
class LoggingBox : public wxDialog
{
private:
	wxTextCtrl *txtLogging;
	wxSizerItem *szriHed;
	wxSizerItem *szriTxt;

public:
	LoggingBox(wxWindow* parent);
	~LoggingBox();

	enum {
		IDC_TEXT_LOGGING = 1,
		IDC_BUTTON_UPDATE
	};

	/// @name functions
	//@{
	//@}
	/// @name event procedures
	//@{
	void OnSize(wxSizeEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnButtonClose(wxCommandEvent& event);
	void OnButtonUpdate(wxCommandEvent& event);
	//@}
	/// @name properties
	//@{
	//@}

	DECLARE_EVENT_TABLE()
};

#endif /* LOGGINGBOX_H */

