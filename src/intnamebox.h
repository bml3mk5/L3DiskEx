/// @file intnamebox.h
///
/// @brief 内部ファイル名ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _INTNAMEBOX_H_
#define _INTNAMEBOX_H_

#include "common.h"
#include <wx/dialog.h>
#include <wx/validate.h>
#include <wx/valtext.h>
//#include "basicfmt.h"
//#include "basicdiritem.h"


#define INTNAMEBOX_CLASSNAME "INTNAMEBOX"

class wxTextCtrl;
class wxListCtrl;
class wxCheckBox;
class wxListEvent;
class L3DiskFrame;
class DiskBasic;
class DiskBasicDirItem;
class DiskBasicFileName;
class DiskBasicGroups;

/// 内部ファイル名ボックスの表示フラグ
typedef enum en_intnamebox_show_flags {
	INTNAME_SHOW_TEXT			 = 0x0001,	///< 内部ファイル名を表示する
	INTNAME_SHOW_ATTR			 = 0x0002,	///< 属性を表示する
	INTNAME_SHOW_NO_PROPERTY	 = 0x0003,
	INTNAME_SHOW_PROPERTY		 = 0x0004,	///< プロパティ表示（グループ一覧表示）
	INTNAME_SHOW_ALL			 = 0x000f,
	INTNAME_NEW_FILE			 = 0x0010,	///< 新規ファイル時
	INTNAME_IMPORT_INTERNAL		 = 0x0020,	///< アプリ内インポート
	INTNAME_SPECIFY_FILE_NAME	 = 0x0100,	///< ファイル名を別途指定
	INTNAME_INVALID_FILE_TYPE	 = 0x0200,	///< アイテム内の属性は無効
} IntNameBoxShowFlags;

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

	int file_size;
	int user_data;	// machine depended

	wxTextCtrl *txtStartAddr;
	wxTextCtrl *txtEndAddr;
	wxTextCtrl *txtExecAddr;

	wxTextCtrl *txtCDate;
	wxTextCtrl *txtCTime;
	wxCheckBox *chkIgnoreDate;

	// 詳細表示用
	wxTextCtrl *txtFileSize;
	wxTextCtrl *txtGroups;
	wxListCtrl *lstGroups;

	void ChangedIgnoreDate(bool check);

public:
	IntNameBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption,
		DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, int file_size, int show_flags);

	enum {
		IDC_TEXT_INTNAME = 1,
		IDC_TEXT_START_ADDR,
		IDC_TEXT_END_ADDR,
		IDC_TEXT_EXEC_ADDR,
		IDC_TEXT_CDATE,
		IDC_TEXT_CTIME,
		IDC_CHK_IGNORE_DATE,
		IDC_TEXT_FILE_SIZE,
		IDC_TEXT_GROUPS,
		IDC_LIST_GROUPS,
	};

	/// @name functions
	//@{
	void CreateBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption,
		DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, int file_size, int show_flags);

	int ShowModal();

	void SetValuesToDirItem();
	//@}

	/// @name event procedures
	//@{
	void OnOK(wxCommandEvent& event);
	void OnChangeType1(wxCommandEvent& event);
	void OnChangeStartAddr(wxCommandEvent& event);
	void OnChangeIgnoreDate(wxCommandEvent& event);

	void OnListItemSelected(wxListEvent& event);
	//@}

	/// @name properties
	//@{
	void SetDiskBasicDirItem(DiskBasicDirItem *item);
	DiskBasic *GetDiskBasic() { return basic; }
	DiskBasicDirItem *GetDiskBasicDirItem() { return item; }
	int GetUniqueNumber() const { return unique_number; }

	void ChangedType1();
	void SetInternalName(const wxString &name);
	void GetInternalName(DiskBasicFileName &name) const;

	int GetStartAddress() const;
	int GetExecuteAddress() const;

	void GetDateTime(struct tm *tm) const;
	struct tm GetDateTime() const;

	void SetDateTime(const wxString &date, const wxString &time);
	void SetDateTime(const wxDateTime &date_time);

	bool DoesIgnoreDateTime() const;
	void IgnoreDateTime(bool val);

	void SetFileSize(long val);
	void SetGroups(long val, DiskBasicGroups &vals);

	int GetUserData() const { return user_data; }
	void SetUserData(int val) { user_data = val; }
	//@}

	/// @name utilities
	//@{
	static wxSize GetDateTextExtent(wxTextCtrl *ctrl);
	static wxSize GetTimeTextExtent(wxTextCtrl *ctrl);
	//@}

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

	void CreateValidator(DiskBasic *basic, DiskBasicDirItem *item);

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

