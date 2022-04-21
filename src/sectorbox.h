/// @file sectorbox.h
///
/// @brief セクタダイアログ
///

#ifndef _SECTORBOX_H_
#define _SECTORBOX_H_

#define SECTORBOX_TRANS \
_("Required information entry is empty.") \
_("'%s' is invalid") \
_("Validation conflict") \
_("'%s' should only contain digits.")

#include "common.h"
#include <wx/dialog.h>

class wxTextCtrl;
class wxCheckBox;

/// セクタ情報ボックス
class SectorBox : public wxDialog
{
private:
	wxTextCtrl *txtID_C;
	wxTextCtrl *txtID_H;
	wxTextCtrl *txtID_R;
	wxTextCtrl *txtID_N;
	
	wxCheckBox *chkDeleted;
	wxCheckBox *chkDensity;

	wxTextCtrl *txtSecNums;

public:
	SectorBox(wxWindow* parent, wxWindowID id, int id_c, int id_h, int id_r, int id_n, int sec_nums, bool deleted, bool sdensity);

	enum {
		IDC_TEXT_ID_C = 1,
		IDC_TEXT_ID_H,
		IDC_TEXT_ID_R,
		IDC_TEXT_ID_N,
		IDC_CHK_DELETED,
		IDC_CHK_DENSITY,
		IDC_TEXT_SECNUMS,
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


	wxDECLARE_EVENT_TABLE();
};

#endif /* _SECTORBOX_H_ */

