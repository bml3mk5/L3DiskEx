/// @file basicselbox.h
///
/// @brief BASIC種類選択ダイアログ
///

#ifndef _BASICSELBOX_H_
#define _BASICSELBOX_H_

#include "common.h"
#include <wx/dialog.h>

class wxListBox;
class DiskBasicParam;
class DiskD88Disk;

/// BASIC種類選択ボックス
class BasicSelBox : public wxDialog
{
private:
	wxListBox *comBasic;
//	wxRadioBox *radChar;

	DiskD88Disk *disk;

public:
	BasicSelBox(wxWindow* parent, wxWindowID id, DiskD88Disk *disk);

	enum {
		IDC_COMBO_BASIC = 1,
		IDC_RADIO_CHAR,
		IDC_RADIO_CHAR_ASCII,
		IDC_RADIO_CHAR_SJIS
	};

	/// @name functions
	//@{
	int ShowModal();
	bool ValidateAllParam();
	//@}

	// event procedures
	void OnBasicChanged(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);

	// properties
	DiskBasicParam *GetBasicParam();
//	int GetCharCode();

	wxDECLARE_EVENT_TABLE();
};

#endif /* _BASICSELBOX_H_ */

