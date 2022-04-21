/// @file basicselbox.h
///
/// @brief BASIC種類選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICSELBOX_H_
#define _BASICSELBOX_H_

#include "common.h"
#include <wx/dialog.h>
#include <wx/dynarray.h>
#include "basicparam.h"


class wxListBox;
class wxTextCtrl;
class wxStaticText;
class DiskBasic;
class DiskBasicParam;
class DiskBasicParamPtrs;
class DiskD88Disk;

/// BASIC種類選択ボックス
class BasicSelBox : public wxDialog
{
private:
	wxListBox *comBasic;
	wxStaticText *lblVolName;
	wxTextCtrl *txtVolName;
	wxStaticText *lblVolNum;
	wxTextCtrl *txtVolNum;

	DiskBasicParamPtrs params;

	DiskD88Disk *disk;

public:
	BasicSelBox(wxWindow* parent, wxWindowID id, DiskD88Disk *disk, DiskBasic *basic, int show_flags);

	enum {
		IDC_LIST_BASIC = 1,
		IDC_TEXT_VOLNAME,
		IDC_TEXT_VOLNUM,
	};

	enum en_show_flags {
		SHOW_ATTR_CONTROLS = 0x01
	};

	/// @name functions
	//@{
	int ShowModal();
	bool ValidateAllParam();

	void ChangeBasic(int sel);
	//@}

	// event procedures
	void OnBasicChanged(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);

	// properties
	const DiskBasicParam *GetBasicParam() const;
	wxString GetVolumeName() const;
	int GetVolumeNumber() const;

	wxDECLARE_EVENT_TABLE();
};

#endif /* _BASICSELBOX_H_ */

