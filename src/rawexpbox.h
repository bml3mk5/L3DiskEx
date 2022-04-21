/// @file rawexpbox.h
///
/// @brief Rawエクスポート＆インポートダイアログ
///

#ifndef _RAWEXPBOX_H_
#define _RAWEXPBOX_H_

#define RAWEXPBOX_TRANS \
_("Required information entry is empty.") \
_("'%s' is invalid") \
_("Validation conflict") \
_("'%s' should only contain digits.")

#include "common.h"
#include <wx/wx.h>
#include <wx/listctrl.h>

class DiskParam;

/// Rawエクスポート＆インポートボックス
class RawExpBox : public wxDialog
{
private:
	wxTextCtrl *txtTrack[2];
	wxTextCtrl *txtSide[2];
	wxTextCtrl *txtSector[2];

	const DiskParam *param;
	int sel_side_num;

public:
	RawExpBox(wxWindow* parent, wxWindowID id, const wxString &caption, const DiskParam *param, int sel_side_num
		, int start_track_num, int start_side_num, int start_sector_num
		, int end_track_num = -1, int end_side_num = -1, int end_sector_num = -1
	);

	enum {
		IDC_TEXT_TRACK_ST = 1,
		IDC_TEXT_TRACK_ED,
		IDC_TEXT_SIDE_ST,
		IDC_TEXT_SIDE_ED,
		IDC_TEXT_SECTOR_ST,
		IDC_TEXT_SECTOR_ED,
	};

	/// @name functions
	//@{
	int ShowModal();
	bool ValidateParam();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);

	// properties
	int GetTrackNumber(int num);
	int GetSideNumber(int num);
	int GetSectorNumber(int num);

	wxDECLARE_EVENT_TABLE();
};

#endif /* _RAWEXPBOX_H_ */

