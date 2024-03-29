/// @file rawparambox.h
///
/// @brief Rawパラメータダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef RAWPARAMBOX_H
#define RAWPARAMBOX_H

#define RAWPARAMBOX_TRANS \
_("Required information entry is empty.") \
_("'%s' is invalid") \
_("Validation conflict") \
_("'%s' should only contain digits.")

#include "../common.h"
#include <wx/dialog.h>


class wxTextCtrl;
class wxChoice;
class wxRadioBox;

/// Rawパラメータボックス
class RawParamBox : public wxDialog
{
private:
	int type;
	int maxvalue;
	wxTextCtrl *txtValue;
	wxChoice   *comValue;

public:
	RawParamBox(wxWindow* parent, wxWindowID id, int type, int value, int maxvalue);

	enum {
		IDC_TEXT_VALUE = 1,
		IDC_COMBO_VALUE,
	};

	enum {
		TYPE_IDC = 0,
		TYPE_IDH,
		TYPE_IDR,
		TYPE_IDN,
		TYPE_NUM_OF_SECTORS,
		TYPE_SECTOR_SIZE,
	};

	/// @name functions
	//@{
	int ShowModal();
	bool ValidateParam();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);

	// properties
	int GetValue();

	wxDECLARE_EVENT_TABLE();
};

/// 密度パラメータボックス
class DensityParamBox : public wxDialog
{
private:
	wxRadioBox *radDensity;

public:
	DensityParamBox(wxWindow* parent, wxWindowID id, bool sdensity);

	enum {
		IDC_RADIO_DENSITY = 1,
	};

	/// @name functions
	//@{
	//@}

	// event procedures

	// properties
	bool IsSingleDensity();

	wxDECLARE_EVENT_TABLE();
};

#endif /* RAWPARAMBOX_H */

