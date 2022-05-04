/// @file basicparambox.h
///
/// @brief BASIC情報ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICPARAMBOX_H_
#define _BASICPARAMBOX_H_

#include "../common.h"
#include <wx/dialog.h>
#include <wx/dynarray.h>
#include "basicselbox.h"
#include "../basicfmt/basicparam.h"


class wxListBox;
class wxTextCtrl;
class wxChoice;
class wxStaticText;
class wxListView;
class DiskBasic;
class DiskBasicParam;
class DiskD88Disk;

/// BASIC情報ボックス
class BasicParamBox : public wxDialog, public VolumeCtrl
{
private:
	DiskBasic *basic;
	DiskBasicParamPtrs params;
	int show_flags;

	wxChoice	*comBasic;
	int selected_basic;

public:
	BasicParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, DiskD88Disk *disk, DiskBasic *basic, int show_flags);

	enum {
		IDC_TEXT_BASIC = 1,
		IDC_LIST_BASIC,
		IDC_VOLUME_CTRL,
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

	wxDECLARE_EVENT_TABLE();
};

#endif /* _BASICPARAMBOX_H_ */

