/// @file intnamebox.h
///
/// @brief 内部ファイル名ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _INTNAMEBOX_H_
#define _INTNAMEBOX_H_

#include "../common.h"
#include <wx/dialog.h>
#include <wx/windowid.h>


#define INTNAMEBOX_CLASSNAME "INTNAMEBOX"

#define INTNAME_COLUMN_WIDTH	120

class wxStaticText;
class wxTextCtrl;
class wxListCtrl;
class wxCheckBox;
class wxListEvent;
class wxGridSizer;
class wxSizerFlags;
class UiDiskProcess;
class DiskBasic;
class DiskBasicDirItem;
class DiskBasicDirItemAttr;
class DiskBasicFileName;
class DiskBasicGroups;
class KeyValArray;

/// 内部ファイル名ボックスの表示フラグ
typedef enum en_intnamebox_show_flags {
	INTNAME_SHOW_TEXT			 = 0x0001,	///< 内部ファイル名を表示する
	INTNAME_SHOW_ATTR			 = 0x0002,	///< 属性を表示する
	INTNAME_SHOW_PROPERTY		 = 0x0004,	///< プロパティ表示（グループ一覧表示）
	INTNAME_SHOW_SKIP_DIALOG	 = 0x0008,	///< スキップするかチェックボックス表示
	INTNAME_NEW_FILE			 = 0x0010,	///< 新規ファイル時
	INTNAME_IMPORT_INTERNAL		 = 0x0020,	///< アプリ内インポート
	INTNAME_SPECIFY_FILE_NAME	 = 0x0100,	///< ファイル名を別途指定
	INTNAME_SPECIFY_CDATE_TIME	 = 0x0200,	///< 作成日時を別途指定
	INTNAME_SPECIFY_MDATE_TIME	 = 0x0400,	///< 更新日時を別途指定
	INTNAME_SPECIFY_ADATE_TIME	 = 0x0800,	///< アクセス日時を別途指定
} IntNameBoxShowFlags;

//////////////////////////////////////////////////////////////////////

/// 内部ファイル名ボックス
class IntNameBox : public wxDialog
{
private:
	UiDiskProcess *frame;

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
	wxTextCtrl *txtMDate;
	wxTextCtrl *txtMTime;
	wxTextCtrl *txtADate;
	wxTextCtrl *txtATime;
	wxCheckBox *chkIgnoreDate;

	// 詳細表示用
	wxTextCtrl *txtFileSize;
	wxTextCtrl *txtGroups;
	wxTextCtrl *txtGrpSize;
	wxListCtrl *lstGroups;
	wxListCtrl *lstInternal;

	wxCheckBox *chkSkipDlg;

	int		GetAddress(wxTextCtrl *ctrl) const;
	void	SetAddress(wxTextCtrl *ctrl, int val);
	void	EnableAddress(wxTextCtrl *ctrl, bool val);
	void	SetEditableAddress(wxTextCtrl *ctrl, bool val);

	void	ChangedIgnoreDate(bool check);

	static wxStaticText *NewStaticText(wxWindow *parent, wxWindowID id, const wxString &label, const wxSize &size);

public:
	IntNameBox(UiDiskProcess *frame, wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &message,
		DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, const wxString &file_name, int file_size, DiskBasicDirItemAttr *date_time, int show_flags);

	enum {
		IDC_TEXT_INTNAME = 1,
		IDC_TEXT_START_ADDR,
		IDC_TEXT_END_ADDR,
		IDC_TEXT_EXEC_ADDR,
		IDC_TEXT_CDATE,
		IDC_TEXT_CTIME,
		IDC_TEXT_MDATE,
		IDC_TEXT_MTIME,
		IDC_TEXT_ADATE,
		IDC_TEXT_ATIME,
		IDC_CHK_IGNORE_DATE,
		IDC_TEXT_FILE_SIZE,
		IDC_TEXT_GROUPS,
		IDC_TEXT_GROUP_SIZE,
		IDC_LIST_GROUPS,
		IDC_LIST_INTERNAL,
		IDC_CHK_SKIP_DLG,
	};

	/// @name functions
	//@{
	void CreateBox(UiDiskProcess *frame, wxWindow* parent, wxWindowID id, const wxString &caption, const wxString &message,
		DiskBasic *basic, DiskBasicDirItem *item, const wxString &file_path, const wxString &file_name, int file_size, DiskBasicDirItemAttr *date_time, int show_flags);

	int ShowModal();
	//@}

	/// @name create control
	//@{
	static void CreateAddress(wxWindow *parent, wxWindowID id
		, const wxString &label, int max_length
		, const wxSize &size, const wxSizerFlags &atitle, wxGridSizer *szrG
		, wxTextCtrl * &txt_ctrl);
	static void CreateDateTime(wxWindow *parent, wxWindowID date_id, wxWindowID time_id
		, const wxString &label, bool has_date_time, bool has_date, bool has_time, bool ignore
		, const wxSize &size, const wxSizerFlags &atitle, wxGridSizer *szrG
		, wxTextCtrl * &txt_date, wxTextCtrl * &txt_time);
	static void CreateFileSize(wxWindow *parent, wxWindowID id
		, const wxString &label, int max_length, bool is_bytes
		, const wxSize &size, const wxSizerFlags &atitle, wxGridSizer *szrG
		, wxTextCtrl * &txt_ctrl);
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

	/// 作成日時を得る
	void	GetCreateDateTime(TM &tm) const;
	/// 作成日時を返す
	TM		GetCreateDateTime() const;
	/// 作成日時を設定
	void	SetCreateDateTime(const wxString &date, const wxString &time);
	/// 作成日時を設定
	void	SetCreateDateTime(const TM &tm);
	/// 変更日時を得る
	void	GetModifyDateTime(TM &tm) const;
	/// 変更日時を返す
	TM		GetModifyDateTime() const;
	/// 変更日時を設定
	void	SetModifyDateTime(const wxString &date, const wxString &time);
	/// 変更日時を設定
	void	SetModifyDateTime(const TM &tm);
	/// アクセス日時を得る
	void	GetAccessDateTime(TM &tm) const;
	/// アクセス日時を返す
	TM		GetAccessDateTime() const;
	/// アクセス日時を設定
	void	SetAccessDateTime(const wxString &date, const wxString &time);
	/// アクセス日時を設定
	void	SetAccessDateTime(const TM &tm);

	/// 日付を無視するか
	bool	DoesIgnoreDateTime(bool def_val) const;
	/// 日付を無視する
	void	IgnoreDateTime(bool val);

	/// ファイルサイズを設定
	virtual void	SetFileSize(long val);
	/// グループリストを設定
	void	SetGroups(const DiskBasicGroups &vals);

	/// ユーザ依存データを返す
	int		GetUserData() const { return user_data; }
	/// ユーザ依存データを設定
	void	SetUserData(int val) { user_data = val; }

	/// 内部データを設定
	void	SetInternalDatas(const KeyValArray &vals);

	/// 以降スキップを返す
	bool	IsSkipDialog(bool def_val) const;
	//@}

	/// @name utilities
	//@{
	static wxSize GetDateTextExtent(wxTextCtrl *ctrl);
	static wxSize GetTimeTextExtent(wxTextCtrl *ctrl);

	/// ファイルサイズをフォーマット
	static void ConvFileSize(long val, wxString &str);
	//@}

	wxDECLARE_EVENT_TABLE();
};

#endif /* _INTNAMEBOX_H_ */

