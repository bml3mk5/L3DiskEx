/// @file basicparambox.h
///
/// @brief BASIC情報ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICPARAMBOX_H_
#define _BASICPARAMBOX_H_

#include "common.h"
#include <wx/dialog.h>
#include <wx/dynarray.h>
#include "basicparam.h"


class wxListBox;
class wxTextCtrl;
class wxChoice;
class wxStaticText;
class DiskBasic;
class DiskBasicParam;
class DiskD88Disk;

/// BASIC情報ボックス
class BasicParamBox : public wxDialog
{
private:
	DiskBasic *basic;
	DiskBasicParamPtrs params;
	int show_flags;

//	wxTextCtrl	*txtBasic;
	wxChoice	*comBasic;
	int selected_basic;

	wxStaticText *lblVolName;
	wxTextCtrl *txtVolName;
	wxStaticText *lblVolNum;
	wxTextCtrl *txtVolNum;
	wxStaticText *lblVolDate;
	wxTextCtrl *txtVolDate;

public:
	BasicParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, DiskD88Disk *disk, DiskBasic *basic, int show_flags);

	enum {
		IDC_TEXT_BASIC = 1,
		IDC_LIST_BASIC,
		IDC_TEXT_VOLNAME,
		IDC_TEXT_VOLNUM,
		IDC_TEXT_VOLDATE,
	};

	enum en_show_flags {
		BASIC_SELECTABLE = 0x01
	};

	/// @name functions
	//@{
	int ShowModal();
	void CommitData();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);

	// properties
	bool IsChangedBasic() const;
	const DiskBasicParam *GetBasicParam() const;
	wxString GetVolumeName() const;
	int GetVolumeNumber() const;
	wxString GetVolumeDate() const;

	wxDECLARE_EVENT_TABLE();
};

#endif /* _BASICPARAMBOX_H_ */

