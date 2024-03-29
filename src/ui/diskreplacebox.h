/// @file diskreplacebox.h
///
/// @brief ディスク置き換え確認ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKREPLACEBOX_H
#define DISKREPLACEBOX_H

#include "../common.h"
#include <wx/dialog.h>
#include <wx/dynarray.h>


class wxChoice;
class DiskImageFile;
class DiskImageDisk;

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
	DiskReplaceBox(wxWindow* parent, wxWindowID id, int side_number, DiskImageFile &src_file, DiskImageDisk &tag_disk);

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

#endif /* DISKREPLACEBOX_H */

