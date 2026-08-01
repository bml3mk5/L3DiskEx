/// @file basicselbox.h
///
/// @brief BASIC種類選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICSELBOX_H
#define BASICSELBOX_H

#include "../common.h"
#include "../basicfmt/basicparam.h"

class DiskBasic;
class DiskBasicParam;
class DiskBasicParamPtrs;
class DiskBasicFormat;
class DiskImageDisk;

#ifndef USE_CONSOLE

#include <wx/dialog.h>
#include <wx/dynarray.h>

class wxListBox;
class wxTextCtrl;
class wxStaticText;

/// VOLUMEコントロール
class VolumeCtrl
{
private:
	enum {
		VOLUME_NAME = 0,
		VOLUME_NUM,
		VOLUME_DATE,
		VOLUME_SKEW,
		VOLUME_ROWS
	};

protected:
	wxStaticText *lblVolume[VOLUME_ROWS];
	wxTextCtrl *txtVolume[VOLUME_ROWS];

	int m_volume_skew;

	void EnableVolumeC(int idx, bool enable);
	void SetVolumeS(int idx, const wxString &val);
	void SetVolumeI(int idx, int val, bool is_hexa);
	wxString GetVolumeS(int idx) const;
	int GetVolumeI(int idx) const;

public:
	VolumeCtrl();
	virtual ~VolumeCtrl() {}
	wxSizer *CreateVolumeCtrl(wxWindow* parent, wxWindowID id, const DiskBasicFormat *fmt);

	void EnableVolumeName(bool enable, size_t max_length, const ValidNameRule &rule);
	void EnableVolumeNumber(bool enable);
	void EnableVolumeDate(bool enable);
	void EnableVolumeSkew(bool enable, int min_value, int max_value);

	void SetVolumeName(const wxString &val);
	void SetVolumeNumber(int val, bool is_hexa);
	void SetVolumeDate(const wxString &val);
	void SetVolumeSkew(int val, bool is_hexa);

	wxString GetVolumeName() const;
	int GetVolumeNumber() const;
	wxString GetVolumeDate() const;
	int GetVolumeSkew() const;
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

#endif /* USE_CONSOLE */

#endif /* BASICSELBOX_H */
