/// @file fileselbox.h
///
/// @brief ファイル種類選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef FILESELBOX_H
#define FILESELBOX_H

#include "../common.h"
#include <wx/dialog.h>


class wxListBox;

/// ファイル種類選択ボックス
class FileSelBox : public wxDialog
{
private:
	wxListBox *comFile;

public:
	FileSelBox(wxWindow* parent, wxWindowID id);

	enum {
		IDC_COMBO_FILE = 1,
	};

	/// @name functions
	//@{
	int ShowModal();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);
	int GetSelection() const;
	wxString GetFormatType() const;

	// properties

	wxDECLARE_EVENT_TABLE();
};

#endif /* FILESELBOX_H */

