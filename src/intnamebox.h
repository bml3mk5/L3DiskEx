/// @file intnamebox.h
///
/// @brief 内部ファイル名ダイアログ
///

#ifndef _INTNAMEBOX_H_
#define _INTNAMEBOX_H_

#include "common.h"
#include <wx/dialog.h>
#include <wx/validate.h>
#include <wx/valtext.h>
//#include "basicfmt.h"
#include "basicdiritem.h"

#define INTNAMEBOX_CLASSNAME "INTNAMEBOX"

class wxTextCtrl;
class wxListCtrl;
class wxListEvent;
class L3DiskFrame;
class DiskBasic;
class DiskBasicDirItem;
class DiskBasicGroups;

typedef enum en_intnamebox_showitems {
	INTNAME_SHOW_TEXT = 0x0001,
	INTNAME_SHOW_ATTR = 0x0002,
	INTNAME_SHOW_NO_PROPERTY = 0x0003,
	INTNAME_SHOW_PROPERTY = 0x0004,
	INTNAME_SHOW_ALL = 0xff,
	INTNAME_SPECIFY_FILE_NAME = 0x100,
	INTNAME_INVALID_FILE_TYPE = 0x200,
} IntNameBoxShowItems;

/// 内部ファイル名ボックス
class IntNameBox : public wxDialog
{
private:
	L3DiskFrame *frame;

	int unique_number;
	DiskBasic		 *basic;
	DiskBasicDirItem *item;

	wxTextCtrl *txtIntName;
	size_t mNameMaxLen;

	AttrControls controls;

	int user_data;	// machine depended

	wxTextCtrl *txtStartAddr;
	wxTextCtrl *txtExecAddr;

	wxTextCtrl *txtCDate;
	wxTextCtrl *txtCTime;

	// 詳細表示用
	wxTextCtrl *txtFileSize;
	wxTextCtrl *txtGroups;
	wxListCtrl *lstGroups;

	DiskBasicFormatType format_type;

public:
	IntNameBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption
		, DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, int showitems);

	enum {
		IDC_TEXT_INTNAME = 1,
//		IDC_RADIO_TYPE1,
//		IDC_RADIO_TYPE2,
		IDC_TEXT_START_ADDR,
		IDC_TEXT_EXEC_ADDR,
		IDC_TEXT_CDATE,
		IDC_TEXT_CTIME,
		IDC_TEXT_FILE_SIZE,
		IDC_TEXT_GROUPS,
		IDC_LIST_GROUPS,
//		IDC_CHECK_READONLY,
//		IDC_CHECK_HIDDEN,
//		IDC_CHECK_READWRITE,
//		IDC_CHECK_ENCRYPT,
	};

	/// @name functions
	//@{
	int ShowModal();

	void SetValuesToDirItem();
	//@}

	// event procedures
	void OnOK(wxCommandEvent& event);
	void OnChangeType1(wxCommandEvent& event);

	void OnListItemSelected(wxListEvent& event);

	// properties
	void SetDiskBasicDirItem(DiskBasicDirItem *item);
	DiskBasicDirItem *GetDiskBasicDirItem() const { return item; }
	int GetUniqueNumber() const { return unique_number; }

	void ChangedType1();
//	void AddExtension(int file_type);
	void SetInternalName(const wxString &name);
	wxString GetInternalName() const;
	int CalcFileType();
	int GetFileType1() const;
	int GetFileType2() const;

	int GetStartAddress() const;
	int GetExecuteAddress() const;

	void GetDateTime(struct tm *tm);
	struct tm GetDateTime();
//	const wxString &GetDate() const;
//	const wxString &GetTime() const;

	void SetDateTime(const wxString &date, const wxString &time);
	void SetDateTime(const wxDateTime &date_time);

	void SetFileSize(long val);
	void SetGroups(long val, DiskBasicGroups &vals);

//	void SetHidden(bool val);
	AttrControls &GetAttrControls() { return controls; }

	wxDECLARE_EVENT_TABLE();
};

/// 内部ファイル名バリデータ
class IntNameValidator : public wxValidator
{
private:
	DiskBasic *basic;
	DiskBasicDirItem *item;
	wxArrayString invchrs;
	size_t maxlen;

	wxString IsValid(const wxString& val) const;
	bool ContainsExcludedCharacters(const wxString& val) const;

public:
	IntNameValidator(DiskBasic *basic, DiskBasicDirItem *item);
	IntNameValidator(const IntNameValidator& val);

	wxObject *Clone() const { return new IntNameValidator(*this); }

	bool Copy(const IntNameValidator& val);

	wxTextEntry *GetTextEntry();

	bool Validate(wxWindow *parent);
	bool Validate(wxWindow *parent, const wxString &val);
	bool TransferToWindow();
	bool TransferFromWindow();

	void OnChar(wxKeyEvent& event);

    wxDECLARE_EVENT_TABLE();
};

/// 日付用バリデータ
class DateTimeValidator : public wxTextValidator
{
private:
	bool is_time;

public:
	DateTimeValidator(bool is_time);
	DateTimeValidator(const DateTimeValidator &src);
	wxObject *Clone() const;
	bool Validate(wxWindow *parent);
};

#endif /* _INTNAMEBOX_H_ */

