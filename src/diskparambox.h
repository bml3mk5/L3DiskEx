/// @file diskparambox.h
///
/// @brief ディスクパラメータダイアログ
///

#ifndef _DISKPARAMBOX_H_
#define _DISKPARAMBOX_H_

#define DISKPARAMBOX_TRANS \
_("Required information entry is empty.") \
_("'%s' is invalid") \
_("Validation conflict") \
_("'%s' should only contain digits.") \
_("'%s' should only contain ASCII characters.")

#include "common.h"
#include <wx/wx.h>
#include <wx/listctrl.h>

class DiskParam;
class DiskD88Disk;

/// ディスクパラメータボックス
class DiskParamBox : public wxDialog
{
private:
	wxComboBox *comTemplate;
	wxTextCtrl *txtTracks;
	wxTextCtrl *txtSides;
	wxTextCtrl *txtSectors;
	wxComboBox *comSecSize;

	wxTextCtrl *txtDiskName;
	wxComboBox *comDensity;
	wxCheckBox *chkWprotect;

public:
	DiskParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, int select_number, DiskD88Disk *disk);

	enum {
		IDC_COMBO_TEMPLATE = 1,
		IDC_TEXT_TRACKS,
		IDC_TEXT_SIDES,
		IDC_TEXT_SECTORS,
		IDC_COMBO_SECSIZE,
		IDC_TEXT_DISKNAME,
		IDC_COMBO_DENSITY,
		IDC_CHK_WPROTECT,
	};

	/// @name functions
	//@{
	int ShowModal();
	bool ValidateAllParam();
	//@}

	// event procedures
	void OnTemplateChanged(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);

	// properties
	void SetParamOfIndex(size_t index);
	void SetParamFromDisk(DiskD88Disk *disk);
	bool GetParam(DiskParam &param);
	bool GetParamToDisk(DiskD88Disk &disk);
	int GetTracksPerSide();
	int GetSidesPerDisk();
	int GetSectorsPerTrack();
	int GetSectorSize();
	wxString GetDiskName() const;
	int GetDensity();
	bool GetWriteProtect();

	wxDECLARE_EVENT_TABLE();
};

#endif /* _DISKPARAMBOX_H_ */

