/// @file fontminibox.h
///
/// @brief フォントミニダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _FONTMINIBOX_H_
#define _FONTMINIBOX_H_

#include "../common.h"
#include <wx/dialog.h>
#include <wx/arrstr.h>


class wxComboBox;
class wxButton;

/// フォントボックス
class FontMiniBox : public wxDialog
{
private:
	wxComboBox *comFontName;
	wxComboBox *comFontSize;
	wxButton   *btnDefault;

	wxArrayString mFontNames;
	wxArrayString mFontSizes;

	wxFont mDefaultFont;

	wxString mSelectedName;
	int      mSelectedSize;

	// parameter

	void init_dialog();
	void term_dialog();

public:
	FontMiniBox(wxWindow* parent, wxWindowID id, const wxFont &default_font);

	enum {
		IDC_COMBO_FONTNAME = 1,
		IDC_COMBO_FONTSIZE,
		IDC_BUTTON_DEFAULT
	};

	/// @name functions
	//@{
	int ShowModal();
	//@}
	/// @name event procedures
	//@{
	void OnTextSize(wxCommandEvent& event);
	void OnButtonDefault(wxCommandEvent& event);
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

