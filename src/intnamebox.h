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
	INTNAME_SHOW_PROPERTY		 = 0x0004,	///< プロパティ表示（グループ一覧表示）
	INTNAME_SHOW_SKIP_DIALOG	 = 0x0008,	///< スキップするかチェックボックス表示
	INTNAME_NEW_FILE			 = 0x0010,	///< 新規ファイル時
	INTNAME_IMPORT_INTERNAL		 = 0x0020,	///< アプリ内インポート
	INTNAME_SPECIFY_FILE_NAME	 = 0x0100,	///< ファイル名を別途指定
	INTNAME_SPECIFY_DATE_TIME	 = 0x0200,	///< 日時を別途指定
} IntNameBoxShowFlags;

//////////////////////////////////////////////////////////////////////

#if 0
class IntNameParam
{
private:
	int		 user_data;

public:
	IntNameParam();
	~IntNameParam() {}

	void	SetUserData(int val) { user_data = val; }
	int		GetUserData() { return user_data; }
};
#endif

//////////////////////////////////////////////////////////////////////

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
	wxTextCtrl *txtGrpSize;
	wxListCtrl *lstGroups;

	wxCheckBox *chkSkipDlg;

	int		GetAddress(wxTextCtrl *ctrl) const;
	void	SetAddress(wxTextCtrl *ctrl, int val);
	void	EnableAddress(wxTextCtrl *ctrl, bool val);
	void	SetEditableAddress(wxTextCtrl *ctrl, bool val);

	void	ChangedIgnoreDate(bool check);

public:
	IntNameBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &message,
		DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, const wxString &file_name, int file_size, struct tm *date_time, int show_flags);

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
		IDC_TEXT_GROUP_SIZE,
		IDC_LIST_GROUPS,
		IDC_CHK_SKIP_DLG,
	};

	/// @name functions
	//@{
	void CreateBox(L3DiskFrame *frame, wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &message,
		DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, const wxString &file_name, int file_size, struct tm *date_time, int show_flags);

	int ShowModal();

//	void SetValuesToDirItem();
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
	void	SetDiskBasicDirItem(DiskBasicDirItem *item);
	DiskBasic *GetDiskBasic() { return basic; }
	DiskBasicDirItem *GetDiskBasicDirItem() { return item; }
	int		GetUniqueNumber() const { return unique_number; }

	void	ChangedType1();

	/// 内部ファイル名を設定
	void	SetInternalName(const wxString &name);
	/// 内部ファイル名を得る
	void	GetInternalName(wxString &name) const;

	/// 終了アドレスを計算
	void	CalcEndAddress();

	/// 開始アドレスを設定
	void	SetStartAddress(int val);
	/// 終了アドレスを設定
	void	SetEndAddress(int val);
	/// 実行アドレスを設定
	void	SetExecuteAddress(int val);
	/// 開始アドレスを返す
	int		GetStartAddress() const;
	/// 終了アドレスを返す
	int		GetEndAddress() const;
	/// 実行アドレスを返す
	int		GetExecuteAddress() const;
	/// 開始アドレスの有効を設定
	void	EnableStartAddress(bool val);
	/// 終了アドレスの有効を設定
	void	EnableEndAddress(bool val);
	/// 実行アドレスの有効を設定
	void	EnableExecuteAddress(bool val);
	/// 開始アドレスが編集可能かを設定
	void	SetEditableStartAddress(bool val);
	/// 終了アドレスが編集可能かを設定
	void	SetEditableEndAddress(bool val);
	/// 実行アドレスが編集可能かを設定
	void	SetEditableExecuteAddress(bool val);

	/// 日時を得る
	void	GetDateTime(struct tm *tm) const;
	/// 日時を返す
	struct tm GetDateTime() const;

	/// 日時を設定
	void	SetDateTime(const wxString &date, const wxString &time);
	/// 日時を設定
	void	SetDateTime(const struct tm *tm);

	/// 日付を無視するか
	bool	DoesIgnoreDateTime(bool def_val) const;
	/// 日付を無視する
	void	IgnoreDateTime(bool val);

	/// ファイルサイズを設定
	void	SetFileSize(long val);
	/// グループリストを設定
	void	SetGroups(const DiskBasicGroups &vals);

	/// ユーザ依存データを返す
	int		GetUserData() const { return user_data; }
	/// ユーザ依存データを設定
	void	SetUserData(int val) { user_data = val; }

	/// 以降スキップを返す
	bool	IsSkipDialog(bool def_val) const;
	//@}

	/// @name utilities
	//@{
	static wxSize GetDateTextExtent(wxTextCtrl *ctrl);
	static wxSize GetTimeTextExtent(wxTextCtrl *ctrl);
	//@}

	wxDECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////////

//
//
//

/// 内部ファイル名バリデータ
class IntNameValidator : public wxValidator
{
private:
	DiskBasic *basic;
	DiskBasicDirItem *item;
	wxString valchrs;	///< 入力できる文字
	wxString invchrs;	///< 入力できない文字
	wxString dupchrs;	///< 重複指定できない文字
	size_t maxlen;

//	wxString IsValid(const wxString& val) const;
	bool ContainsIncludedCharacters(const wxString& val, wxString &invchr) const;
	bool ContainsExcludedCharacters(const wxString& val, wxString &invchr) const;
	bool ContainsDuplicatedCharacters(const wxString& val, wxString &invchr) const;

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

//////////////////////////////////////////////////////////////////////

/// 日付用バリデータ
class DateTimeValidator : public wxTextValidator
{
private:
	bool m_is_time;
	bool m_require;

public:
	DateTimeValidator(bool is_time, bool required);
	DateTimeValidator(const DateTimeValidator &src);
	wxObject *Clone() const;
	bool Validate(wxWindow *parent);
};

//////////////////////////////////////////////////////////////////////

/// アドレス用バリデータ
class AddressValidator : public wxTextValidator
{
private:
public:
	AddressValidator();
	AddressValidator(const AddressValidator &src);
	wxObject *Clone() const;
	bool Validate(wxWindow *parent);
};

#endif /* _INTNAMEBOX_H_ */

