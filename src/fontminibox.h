/// @file fontminibox.h
///
/// @brief フォントミニダイアログ
///

#ifndef _FONTMINIBOX_H_
#define _FONTMINIBOX_H_

#include "common.h"
#include <wx/dialog.h>
#include <wx/arrstr.h>

class wxComboBox;

/// フォントボックス
class FontMiniBox : public wxDialog
{
private:
	wxComboBox *comFontName;
	wxComboBox *comFontSize;

	wxArrayString mFontNames;
	wxArrayString mFontSizes;

	wxString mSelectedName;
	int      mSelectedSize;

	// parameter

	void init_dialog();
	void term_dialog();

public:
	FontMiniBox(wxWindow* parent, wxWindowID id);

	enum {
		IDC_COMBO_FONTNAME = 1,
		IDC_COMBO_FONTSIZE
	};

	/// @name functions
	//@{
	int ShowModal();
	//@}
	/// @name event procedures
	//@{
	void OnTextSize(wxCommandEvent& event);
	//@}
	/// @name properties
	//@{
	wxString &GetFontName() { return mSelectedName; }
	int GetFontSize() { return mSelectedSize; }
	void SetFontName(const wxString &val);
	void SetFontSize(int val);
	//@}

	DECLARE_EVENT_TABLE()
};

#endif /* _FONTMINIBOX_H_ */

