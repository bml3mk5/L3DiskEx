/// @file rawparambox.h
///
/// @brief Rawパラメータダイアログ
///

#ifndef _RAWPARAMBOX_H_
#define _RAWPARAMBOX_H_

#define RAWPARAMBOX_TRANS \
_("Required information entry is empty.") \
_("'%s' is invalid") \
_("Validation conflict") \
_("'%s' should only contain digits.")

#include "common.h"
#include <wx/wx.h>
#include <wx/listctrl.h>

/// Rawパラメータボックス
class RawParamBox : public wxDialog
{
private:
	int type;
	int maxvalue;
	wxTextCtrl *txtValue;

public:
	RawParamBox(wxWindow* parent, wxWindowID id, int type, int value, int maxvalue);

	enum {
		IDC_TEXT_VALUE = 1,
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

#endif /* _RAWPARAMBOX_H_ */

