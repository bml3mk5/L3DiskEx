/// @file intnamebox.h
///
/// @brief 内部ファイル名ダイアログ
///

#ifndef _INTNAMEBOX_H_
#define _INTNAMEBOX_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/listctrl.h>

class DiskBasicGroupItems;

/// 内部ファイル名ボックス
class IntNameBox : public wxDialog
{
private:
	wxTextCtrl *txtIntName;
	size_t mNameMaxLen;

	wxRadioBox *radType1;
	wxRadioBox *radType2;

	// 詳細表示用
	wxTextCtrl *txtFileSize;
	wxTextCtrl *txtGroups;
	wxListCtrl *lstGroups;

	int format_type;

public:
	IntNameBox(wxWindow* parent, wxWindowID id, const wxString &caption, int format_type, const wxString &name, size_t name_len, int file_type, int data_type, bool isproperty);

	enum {
		IDC_TEXT_INTNAME = 1,
		IDC_RADIO_TYPE1,
		IDC_RADIO_TYPE2,
		IDC_TEXT_FILE_SIZE,
		IDC_TEXT_GROUPS,
		IDC_LIST_GROUPS,
	};

	/// @name functions
	//@{
	int ShowModal();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);
	void OnChangeType1(wxCommandEvent& event);

	// properties
	void ChangedType1(int sel_idx);
	void AddExtension(int file_type);
	void SetInternalName(const wxString &item);
	wxString GetInternalName() const;
	int GetFileType() const;
	int GetDataType() const;

	void SetFileSize(long val);
	void SetGroups(long val, DiskBasicGroupItems &vals);

	wxDECLARE_EVENT_TABLE();
};

/// バリデータ
class IntNameValidator : public wxValidator
{
private:
	wxArrayString invchrs;
	size_t maxlen;

	wxString IsValid(const wxString& val) const;
	bool ContainsExcludedCharacters(const wxString& val) const;

public:
	IntNameValidator(size_t maxlength);
	IntNameValidator(const IntNameValidator& val);

	wxObject *Clone() const { return new IntNameValidator(*this); }

	bool Copy(const IntNameValidator& val);

	wxTextEntry *GetTextEntry();

	bool Validate(wxWindow *parent);
	bool TransferToWindow();
	bool TransferFromWindow();

	void OnChar(wxKeyEvent& event);

    wxDECLARE_EVENT_TABLE();
};

#endif /* _INTNAMEBOX_H_ */

