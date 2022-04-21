/// @file uidiskattr.h
///
/// @brief ディスク属性
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UIDISKATTR_H_
#define _UIDISKATTR_H_

#include "../common.h"
#include <wx/string.h>
#include <wx/panel.h>


class wxTextCtrl;
class wxButton;
class wxChoice;
class wxCheckBox;
class wxBoxSizer;

class L3DiskFrame;
class DiskD88Disk;

/// 右パネルの属性
class L3DiskDiskAttr : public wxPanel
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	wxTextCtrl *txtAttr;
	wxButton   *btnChange;
	wxChoice   *comDensity;
	wxCheckBox *chkWprotect;
	wxBoxSizer *szrButtons;

	DiskD88Disk *disk;

public:
	L3DiskDiskAttr(L3DiskFrame *parentframe, wxWindow *parent);
	~L3DiskDiskAttr();

	void OnSize(wxSizeEvent& event);
	void OnButtonChange(wxCommandEvent& event);
	void OnComboDensity(wxCommandEvent& event);
	void OnCheckWriteProtect(wxCommandEvent& event);

	void ShowChangeDisk();
	void SetAttr(DiskD88Disk *newdisk);

	void SetAttrText(const wxString &val);
//	void SetDiskDensity(const wxString &val);
	void SetDiskDensity(int val);
	int  GetDiskDensity() const;
	void SetWriteProtect(bool val, bool enable = true);
	bool GetWriteProtect() const;
	void ClearData();

	void SetListFont(const wxFont &font);

	enum {
		IDC_TXT_ATTR = 1,
		IDC_BTN_CHANGE,
		IDC_COM_DENSITY,
		IDC_CHK_WPROTECT,
	};

	wxDECLARE_EVENT_TABLE();
};

#endif /* _UIDISKATTR_H_ */

