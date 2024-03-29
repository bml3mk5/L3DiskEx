/// @file rawtrackbox.h
///
/// @brief Rawトラックダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef RAWTRACKBOX_H
#define RAWTRACKBOX_H

#include "../common.h"
#include <wx/dialog.h>


class wxTextCtrl;
class DiskImageDisk;

/// Rawパラメータボックス
class RawTrackBox : public wxDialog
{
private:
//	wxTextCtrl *txtOffset;

public:
	RawTrackBox(wxWindow* parent, wxWindowID id, int num, wxUint32 offset, DiskImageDisk *disk);

	enum {
		IDC_TEXT_OFFSET = 1,
	};

	/// @name functions
	//@{
	int ShowModal();
	//@}

	wxDECLARE_EVENT_TABLE();
};

#endif /* RAWTRACKBOX_H */

