/// @file configbox.h
///
/// @brief 設定ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _CONFIGBOX_H_
#define _CONFIGBOX_H_

#include "../common.h"
#include <wx/dialog.h>
#include <wx/dynarray.h>


class wxCheckBox;
class wxTextCtrl;
class wxChoice;
class wxStaticText;
class Config;

/// 設定ボックス
class ConfigBox : public wxDialog
{
private:
	Config *ini;

	wxCheckBox *chkTrimData;
	wxCheckBox *chkShowDelFile;
	wxCheckBox *chkAddExtExport;
	wxCheckBox *chkDateExport;
	wxCheckBox *chkSuppImport;
	wxCheckBox *chkDecAttrImport;
	wxCheckBox *chkDateImport;
	wxCheckBox *chkIgnoreDate;
	wxTextCtrl *txtTempFolder;
	wxCheckBox *chkTempFolder;
	wxButton   *btnTempFolder;
	wxTextCtrl *txtBinaryEditer;
	wxChoice   *comLanguage;

public:
	ConfigBox(wxWindow* parent, wxWindowID id, Config *ini);

	enum {
		IDC_CHECK_TRIM_DATA = 1,
		IDC_CHECK_SHOW_DELFILE,
		IDC_CHECK_ADD_EXT_EXPORT,
		IDC_CHECK_DATE_EXPORT,
		IDC_CHECK_SUPP_IMPORT,
		IDC_CHECK_DEC_ATTR_IMPORT,
		IDC_CHECK_DATE_IMPORT,
		IDC_CHECK_IGNORE_DATE,
		IDC_TEXT_TEMP_FOLDER,
		IDC_BUTTON_TEMP_FOLDER,
		IDC_CHECK_TEMP_FOLDER,
		IDC_TEXT_BINARY_EDITER,
		IDC_BUTTON_BINARY_EDITER,
		IDC_COMBO_LANGUAGE,
	};

	/// @name functions
	//@{
	int ShowModal();
	void CommitData();

	void InitializeTempFolder();
	void SetEditableTempFolder(bool val);
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);
	void OnCheckTempFolder(wxCommandEvent& event);
	void OnClickTempFolder(wxCommandEvent& event);
	void OnClickBinaryEditer(wxCommandEvent& event);

	// properties

	wxDECLARE_EVENT_TABLE();
};

#endif /* _CONFIGBOX_H_ */

