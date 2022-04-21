/// @file uidisklist.h
///
/// @brief ディスクリスト
///
#ifndef _UIDISKLIST_H_
#define _UIDISKLIST_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dataview.h>
#include <wx/clntdata.h>

class L3DiskFrame;
class DiskD88File;
class DiskD88Disk;

/// ディスク名を保存するクラス
class L3DiskNameString
{
public:
	wxString      name;
	wxArrayString sides;
public:
	L3DiskNameString();
	L3DiskNameString(const wxString &newname);
	~L3DiskNameString() {}
};

WX_DECLARE_OBJARRAY(L3DiskNameString, L3DiskNameStrings);

/// ディスク情報に紐づける属性
class L3DiskPositionData : public wxClientData
{
private:
	int num;
	int subnum;
	bool editable;
public:
	L3DiskPositionData(int val1, int val2, bool edt) : wxClientData() { num = val1; subnum = val2; editable = edt; }

	void SetNumber(int val1, int val2, bool edt) { num = val1; subnum = val2; editable = edt; }
	int  GetNumber() { return num; }
	int  GetSubNumber() { return subnum; }
	bool GetEditable() { return editable; }
};

/// ディスク１枚部分のツリーアイテムの挙動を設定する
class L3DiskTreeStoreModel : public wxDataViewTreeStore
{
private:
	L3DiskFrame *frame;
public:
	L3DiskTreeStoreModel(L3DiskFrame *parentframe);

	/// ディスク名編集できるか
	virtual bool IsEnabled(const wxDataViewItem &item, unsigned int col) const;
	/// ディスク名を変更した
	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
};

/// 左Panel ディスク情報
class L3DiskList: public wxDataViewTreeCtrl
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	wxMenu *menuPopup;

	DiskD88Disk *disk;
	bool selected_disk;

	/// 選択位置のディスクイメージ
	wxDataViewItem SetSelectedItemAtDiskImage();

public:
	L3DiskList(L3DiskFrame *parentframe, wxWindow *parent);
	~L3DiskList();

	/// @name event procedures
	//@{
//	void OnSize(wxSizeEvent& event);
	/// 右クリック選択
	void OnDataViewItemContextMenu(wxDataViewEvent& event);
	/// ツリーアイテム選択
	void OnSelectionChanged(wxDataViewEvent& event);
	/// アイテム編集開始
	void OnStartEditing(wxDataViewEvent& event);
	/// アイテム編集終了
	void OnEditingDone(wxDataViewEvent& event);
	/// ディスクを保存選択
	void OnSaveDisk(wxCommandEvent& event);
	/// ディスクを新規に追加選択
	void OnAddNewDisk(wxCommandEvent& event);
	/// ディスクをファイルから追加選択
	void OnAddDiskFromFile(wxCommandEvent& event);
	/// ディスクイメージを置換選択
	void OnReplaceDisk(wxCommandEvent& event);
	/// ディスクを削除選択
	void OnDeleteDisk(wxCommandEvent& event);
	/// ディスク名を変更選択
	void OnRenameDisk(wxCommandEvent& event);
	/// 初期化選択
	void OnInitializeDisk(wxCommandEvent& event);
	/// フォーマット選択
	void OnFormatDisk(wxCommandEvent& event);
	/// プロパティ選択
	void OnPropertyDisk(wxCommandEvent& event);
	/// キー入力
	void OnChar(wxKeyEvent& event);
	//@}

	/// @name functions
	//@{
	/// ポップアップメニュー表示
	void ShowPopupMenu();

	/// 再選択
	void ReSelect();
	/// 選択
	void ChangeSelection(wxDataViewItem &item);
	/// 選択
	void ChangeSelection(int disk_number, int side_number);
	/// ファイル名をリストにセット
	void SetFileName();
	/// ファイル名をリストにセット
	void SetFileName(const wxString &filename, L3DiskNameStrings &disknames);
	/// リストをクリア
	void ClearFileName();

	/// ディスクの初期化
	bool InitializeDisk();
	/// ディスクをファイルに保存ダイアログ
	void ShowSaveDiskDialog();
//	/// ディスクをファイルに保存
//	bool SaveDisk(const wxString &path);
	/// ディスクをファイルから削除
	bool DeleteDisk();
	/// ディスク名を変更
	void RenameDisk();
	/// ディスク情報ダイアログ
	void ShowDiskAttr();
	/// 選択位置のディスク名をセット
	void SetDiskName(const wxString &val);
	//@}

	/// @name properties
	//@{
	/// 選択しているディスクイメージのディスク番号を返す
	int  GetSelectedDiskNumber();
	/// 選択しているディスクイメージのサイド番号を返す
	int  GetSelectedDiskSide();
	/// ディスクイメージを選択しているか
	bool IsSelectedDiskImage();
	/// ディスクを選択しているか
	bool IsSelectedDisk();
	/// ディスクを選択しているか(AB面どちらか)
	bool IsSelectedDiskSide();
	//@}

	enum {
		IDM_SAVE_DISK = 1,
		IDM_ADD_DISK_NEW,
		IDM_ADD_DISK_FROM_FILE,
		IDM_REPLACE_DISK_FROM_FILE,
		IDM_DELETE_DISK_FROM_FILE,
		IDM_RENAME_DISK,
		IDM_INITIALIZE_DISK,
		IDM_FORMAT_DISK,
		IDM_PROPERTY_DISK,
	};

	wxDECLARE_EVENT_TABLE();
};

#ifndef USE_DND_ON_TOP_PANEL
/// ディスクファイル ドラッグ＆ドロップ
class L3DiskListDropTarget : public wxFileDropTarget
{
	L3DiskList  *parent;
    L3DiskFrame *frame;

public:
    L3DiskListDropTarget(L3DiskFrame *parentframe, L3DiskList *parentwindow);
    bool OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames);
};
#endif

#endif /* _UIDISKLIST_H_ */

