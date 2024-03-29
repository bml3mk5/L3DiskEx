/// @file rawexpbox.h
///
/// @brief Rawエクスポート＆インポートダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef RAWEXPBOX_H
#define RAWEXPBOX_H

#define RAWEXPBOX_TRANS \
_("Required information entry is empty.") \
_("'%s' is invalid") \
_("Validation conflict") \
_("'%s' should only contain digits.")

#include "../common.h"
#include <wx/dialog.h>


class wxTextCtrl;
class wxCheckBox;
class DiskImageDisk;

/// Rawエクスポート＆インポートボックス
class RawExpBox : public wxDialog
{
private:
	wxTextCtrl *txtTrack[2];
	wxTextCtrl *txtSide[2];
	wxTextCtrl *txtSector[2];
	wxCheckBox *chkInvData;
	wxCheckBox *chkRevSide;

	DiskImageDisk *p_disk;
	int m_sel_side_num;

public:
	RawExpBox(wxWindow* parent, wxWindowID id, const wxString &caption, DiskImageDisk *disk, int sel_side_num
		, int start_track_num, int start_side_num, int start_sector_num
		, int end_track_num = -1, int end_side_num = -1, int end_sector_num = -1
		, bool invert_data = false, bool reverse_side = false
	);

	enum {
		IDC_TEXT_TRACK_ST = 1,
		IDC_TEXT_TRACK_ED,
		IDC_TEXT_SIDE_ST,
		IDC_TEXT_SIDE_ED,
		IDC_TEXT_SECTOR_ST,
		IDC_TEXT_SECTOR_ED,
		IDC_CHK_INV_DATA,
		IDC_CHK_REV_SIDE,
	};

	/// @name functions
	//@{
	int ShowModal();
	bool ValidateParam();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);

	// properties
	int GetTrackNumber(int num) const;
	int GetSideNumber(int num) const;
	int GetSectorNumber(int num) const;

	bool InvertData() const;
	bool ReverseSide() const;

	wxDECLARE_EVENT_TABLE();
};

#endif /* RAWEXPBOX_H */

