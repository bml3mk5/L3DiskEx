/// @file basicselbox.h
///
/// @brief BASIC種類選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICSELBOX_H
#define BASICSELBOX_H

#include "../common.h"
#include <wx/dialog.h>
#include <wx/dynarray.h>
#include "../basicfmt/basicparam.h"

#define VOLUME_ROWS 3

class wxListBox;
class wxTextCtrl;
class wxStaticText;
class DiskBasic;
class DiskBasicParam;
class DiskBasicParamPtrs;
class DiskImageDisk;

/// VOLUMEコントロール
class VolumeCtrl
{
protected:
	wxStaticText *lblVolume[VOLUME_ROWS];
	wxTextCtrl *txtVolume[VOLUME_ROWS];

public:
	VolumeCtrl();
	virtual ~VolumeCtrl() {}
	wxSizer *CreateVolumeCtrl(wxWindow* parent, wxWindowID id);

	void EnableVolumeName(bool enable, size_t max_length, const ValidNameRule &rule);
	void EnableVolumeNumber(bool enable);
	void EnableVolumeDate(bool enable);

	void SetVolumeName(const wxString &val);
	void SetVolumeNumber(int val, bool is_hexa);
	void SetVolumeDate(const wxString &val);

	wxString GetVolumeName() const;
	int GetVolumeNumber() const;
	wxString GetVolumeDate() const;
};


/// BASIC種類選択ボックス
class BasicSelBox : public wxDialog, public VolumeCtrl
{
private:
	wxListBox *comBasic;

	DiskBasicParamPtrs params;

	DiskImageDisk *p_disk;

public:
	BasicSelBox(wxWindow* parent, wxWindowID id, DiskImageDisk *disk, DiskBasic *basic, int show_flags);

	enum {
		IDC_LIST_BASIC = 1,
		IDC_VOLUME_CTRL,
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

	wxDECLARE_EVENT_TABLE();
};

#endif /* BASICSELBOX_H */

