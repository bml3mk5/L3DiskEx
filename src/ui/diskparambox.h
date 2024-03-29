/// @file diskparambox.h
///
/// @brief ディスクパラメータダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKPARAMBOX_H
#define DISKPARAMBOX_H

#define DISKPARAMBOX_TRANS \
_("Required information entry is empty.") \
_("'%s' is invalid") \
_("Validation conflict") \
_("'%s' should only contain digits.") \
_("'%s' should only contain ASCII characters.")

#include "../common.h"
#include <wx/dialog.h>
#include <wx/arrstr.h>


class wxComboBox;
class wxChoice;
class wxTextCtrl;
class wxCheckBox;
class wxRadioButton;

class DiskParam;
class DiskParamPtrs;
class DiskImage;
class DiskImageDisk;

/// ディスクパラメータボックス
class DiskParamBox : public wxDialog
{
public:
	enum OpeFlags {
		SELECT_DISK_TYPE = 1,
		ADD_NEW_DISK,
		CREATE_NEW_DISK,
		CHANGE_DISK_PARAM,
		SHOW_DISK_PARAM,
		REBUILD_TRACKS,
	};

	enum en_show_flags {
		SHOW_ALL			 = 0xffff,
		SHOW_CATEGORY		 = 0x0001,
		SHOW_TEMPLATE		 = 0x0002,
		SHOW_TEMPLATE_ALL	 = 0x0003,
		SHOW_DISKLABEL_ALL	 = 0x0010,
	};

private:
	wxChoice   *comCategory;
	wxChoice   *comTemplate;
	wxTextCtrl *txtTracks;
	wxTextCtrl *txtSides;
	wxTextCtrl *txtSectors;
	wxComboBox *comSecSize;
	wxTextCtrl *txtSecIntl;
	wxChoice   *comNumbSec;
	wxTextCtrl *txtFirstTrack;
	wxTextCtrl *txtFirstSector;
	wxTextCtrl *txtDiskSize;

	wxTextCtrl *txtDiskName;
	wxChoice   *comDensity;
	wxCheckBox *chkWprotect;
	wxRadioButton *radSingle[4];
	wxTextCtrl *txtSingleSectors;
	wxComboBox *comSingleSecSize;

	DiskImage *p_image;
	OpeFlags m_ope_flags;
	int m_show_flags;
	const DiskParamPtrs *p_disk_params;
	const DiskParam *p_manual_param;
	bool now_manual_setting;

	wxArrayString m_type_names;

	int FindTemplate(DiskImageDisk *disk);
	void SetParamFromTemplate(const DiskParam *item);
	void SetParamOfIndexFromGlobals(size_t index);
	void SetParamOfIndexFromParams(size_t index);
	void SetParamForManual();
	void SetParamToControl(const DiskParam *item);
	bool GetParamFromGlobals(DiskParam &param);
	bool GetParamFromParams(DiskParam &param);
	void GetParamForManual(DiskParam &param);

public:
	DiskParamBox(wxWindow* parent, wxWindowID id, DiskImage &image, OpeFlags ope_flags, int select_number, DiskImageDisk *disk, const DiskParamPtrs *params, const DiskParam *manual_param, int show_flags);

	enum {
		IDC_COMBO_CATEGORY = 1,
		IDC_COMBO_TEMPLATE,
		IDC_TEXT_TRACKS,
		IDC_TEXT_SIDES,
		IDC_TEXT_SECTORS,
		IDC_COMBO_SECSIZE,
		IDC_TEXT_INTERLEAVE,
		IDC_COMBO_NUMBSEC,
		IDC_TEXT_FIRST_TRACK,
		IDC_TEXT_FIRST_SECTOR,
		IDC_TEXT_DISKSIZE,
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
	void OnParameterChanged(wxCommandEvent& event);
	void OnSingleChanged(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);

	// properties
	void SetTemplateValues(bool all);
	void SetTemplateValuesFromGlobals(bool all);
	void SetTemplateValuesFromGlobalsSub(int flags);
	void SetTemplateValuesFromParams();
	void SetParamOfIndex(size_t index);
	void SetParamFromDisk(const DiskImageDisk *disk);
	void SetDensity(int val);
	void CalcDiskSize();
	bool GetParam(DiskParam &param);
//	bool GetParamToDisk(DiskImageDisk &disk);
	wxString GetCategory() const;
	int GetTracksPerSide() const;
	int GetSidesPerDisk() const;
	int GetSectorsPerTrack() const;
	int GetSectorSize() const;
	int GetInterleave() const;
	int GetNumberingSector() const;
	int GetFirstTrackNumber() const;
	int GetFirstSectorNumber() const;
	wxString GetDiskName() const;
	int GetDensity() const;
	wxUint8 GetDensityValue() const;
	bool IsWriteProtected() const;
	int  GetSingleNumber() const;
	int  GetSingleSectorsPerTrack() const;
	int  GetSingleSectorSize() const;

	wxDECLARE_EVENT_TABLE();
};

#endif /* DISKPARAMBOX_H */

