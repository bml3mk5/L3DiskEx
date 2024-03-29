/// @file rawsectorbox.h
///
/// @brief Raw セクタダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef RAWSECTORBOX_H
#define RAWSECTORBOX_H

#define SECTORBOX_TRANS \
_("Required information entry is empty.") \
_("'%s' is invalid") \
_("Validation conflict") \
_("'%s' should only contain digits.")

#include "../common.h"
#include <wx/dialog.h>


#define SECTORBOX_HIDE_SECTOR_NUMS	0x0040

class wxTextCtrl;
class wxCheckBox;

/// セクタ情報ボックス
class RawSectorBox : public wxDialog
{
private:
	wxTextCtrl *txtID_C;
	wxTextCtrl *txtID_H;
	wxTextCtrl *txtID_R;
	wxTextCtrl *txtID_N;
	
	wxCheckBox *chkDeleted;
	wxCheckBox *chkDensity;

	wxTextCtrl *txtSecNums;

	wxTextCtrl *txtStatus;

public:
	RawSectorBox(wxWindow* parent, wxWindowID id, const wxString &caption, int id_c, int id_h, int id_r, int id_n, int sec_nums, bool deleted, bool sdensity, int status, int hide_flags = 0);

	enum {
		IDC_TEXT_ID_C = 1,
		IDC_TEXT_ID_H,
		IDC_TEXT_ID_R,
		IDC_TEXT_ID_N,
		IDC_CHK_DELETED,
		IDC_CHK_DENSITY,
		IDC_TEXT_SECNUMS,
		IDC_TEXT_STATUS,
	};

	/// @name functions
	//@{
	int ShowModal();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);

	// properties
	int GetIdC();
	int GetIdH();
	int GetIdR();
	int GetIdN();
	int GetSectorNums();
	bool GetDeletedMark();
	bool GetSingleDensity();
	int GetStatus();


	wxDECLARE_EVENT_TABLE();
};

#endif /* RAWSECTORBOX_H */

