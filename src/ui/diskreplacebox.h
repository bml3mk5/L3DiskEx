/// @file diskreplacebox.h
///
/// @brief ディスク置き換え確認ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKREPLACEBOX_H_
#define _DISKREPLACEBOX_H_

#include "../common.h"
#include <wx/dialog.h>
#include <wx/dynarray.h>


class wxChoice;
class DiskD88File;
class DiskD88Disk;

/// ディスク置き換え確認ボックス内で使用
class DiskReplaceNumber
{
public:
	int disknum;
	int sidenum;
	DiskReplaceNumber();
	DiskReplaceNumber(int disknum_, int sidenum_);
	~DiskReplaceNumber();
};

/// @class DiskReplaceNumbers
///
/// @brief DiskReplaceNumber のリスト
WX_DECLARE_OBJARRAY(DiskReplaceNumber, DiskReplaceNumbers);

/// ディスク置き換え確認ボックス
class DiskReplaceBox : public wxDialog
{
private:
	wxChoice *comDisk;
	DiskReplaceNumbers numDisk;

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
	int GetSelectedDiskNumber() const;
	int GetSelectedSideNumber() const;

	wxDECLARE_EVENT_TABLE();
};

#endif /* _DISKREPLACEBOX_H_ */

