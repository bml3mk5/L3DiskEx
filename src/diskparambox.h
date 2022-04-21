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
#include <wx/dialog.h>
#include <wx/arrstr.h>

class wxComboBox;
class wxChoice;
class wxTextCtrl;
class wxCheckBox;
class wxRadioButton;

class DiskParam;
class DiskD88Disk;

#define DiskParamBox_DiskName		0x01
#define DiskParamBox_Density		0x02
#define DiskParamBox_WriteProtect	0x04
#define DiskParamBox_Flags_All		0x07

/// ディスクパラメータボックス
class DiskParamBox : public wxDialog
{
private:
	wxChoice   *comCategory;
	wxChoice   *comTemplate;
	wxTextCtrl *txtTracks;
	wxTextCtrl *txtSides;
	wxTextCtrl *txtSectors;
	wxComboBox *comSecSize;
	wxTextCtrl *txtSecIntl;
	wxChoice   *comNumbSec;

	wxTextCtrl *txtDiskName;
	wxChoice   *comDensity;
	wxCheckBox *chkWprotect;
	wxRadioButton *radSingle[4];
	wxTextCtrl *txtSingleSectors;
	wxComboBox *comSingleSecSize;

	wxUint32 disable_flags;

	wxArrayString type_names;

	int FindTemplate(DiskD88Disk *disk);

public:
	DiskParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, int select_number, DiskD88Disk *disk, bool use_template, wxUint32 disable_flags = 0);

	enum {
		IDC_COMBO_CATEGORY = 1,
		IDC_COMBO_TEMPLATE,
		IDC_TEXT_TRACKS,
		IDC_TEXT_SIDES,
		IDC_TEXT_SECTORS,
		IDC_COMBO_SECSIZE,
		IDC_TEXT_INTERLEAVE,
		IDC_COMBO_NUMBSEC,
		IDC_TEXT_DISKNAME,
		IDC_COMBO_DENSITY,
		IDC_CHK_WPROTECT,
		IDC_RADIO_SINGLE_NONE,
		IDC_RADIO_SINGLE_ALL,
		IDC_RADIO_SINGLE_T00,
		IDC_RADIO_SINGLE_T0A,
		IDC_TEXT_SINGLE_SECTORS,
		IDC_COMBO_SINGLE_SECSIZE,
	};

	/// @name functions
	//@{
	int ShowModal();
	bool ValidateAllParam();
	//@}

	// event procedures
	void OnCategoryChanged(wxCommandEvent& event);
	void OnTemplateChanged(wxCommandEvent& event);
	void OnSingleChanged(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);

	// properties
	void SetTemplateValues();
	void SetParamOfIndex(size_t index);
	void SetParamFromDisk(const DiskD88Disk *disk);
	bool GetParam(DiskParam &param);
	bool GetParamToDisk(DiskD88Disk &disk);
	wxString GetCategory() const;
	int GetTracksPerSide() const;
	int GetSidesPerDisk() const;
	int GetSectorsPerTrack() const;
	int GetSectorSize() const;
	int GetInterleave() const;
	int GetNumberingSector() const;
	wxString GetDiskName() const;
	int GetDensity() const;
	bool IsWriteProtected() const;
	int  GetSingleNumber() const;
	int  GetSingleSectorsPerTrack() const;
	int  GetSingleSectorSize() const;

	wxDECLARE_EVENT_TABLE();
};

#endif /* _DISKPARAMBOX_H_ */

