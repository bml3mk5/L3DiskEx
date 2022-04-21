/// @file resultbox.h
///
/// @brief 結果表示ダイアログ
///

#ifndef _RESULTBOX_H_
#define _RESULTBOX_H_

#include "common.h"
#include <wx/dialog.h>

//class wxStaticBitmap;
class wxTextCtrl;

/// 結果表示ボックス
class ResultBox : public wxDialog
{
private:
//	wxStaticBitmap *staBmp;
	wxTextCtrl *txtInfo;

public:
	ResultBox(wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &msg);

	enum {
		IDC_BITMAP = 1,
		IDC_TEXT_INFO,
	};

	/// @name functions
	//@{
	int ShowModal();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);

	// properties

	wxDECLARE_EVENT_TABLE();
};

#endif /* _RESULTBOX_H_ */

