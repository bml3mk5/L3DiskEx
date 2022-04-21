/// @file diskreplacebox.h
///
/// @brief ディスク置き換え確認ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKREPLACEBOX_H_
#define _DISKREPLACEBOX_H_

#include "common.h"
#include <wx/dialog.h>


class wxChoice;
class DiskD88File;
class DiskD88Disk;

/// ディスク置き換え確認ボックス
class DiskReplaceBox : public wxDialog
{
private:
	wxChoice *comDisk;

public:
	DiskReplaceBox(wxWindow* parent, wxWindowID id, int side_number, DiskD88File &src_file, DiskD88Disk &tag_disk);

	enum {
		IDC_COMBO_DISK = 1,
	};

	/// @name functions
	//@{
	int ShowModal();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);

	// properties
	int GetSelection() const;

	wxDECLARE_EVENT_TABLE();
};

#endif /* _DISKREPLACEBOX_H_ */

