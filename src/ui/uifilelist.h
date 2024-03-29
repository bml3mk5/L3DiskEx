/// @file uifilelist.h
///
/// @brief BASICファイル名一覧
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef UIFILELIST_H
#define UIFILELIST_H

#include "uicommon.h"
#include <wx/string.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include "intnamebox.h"


#ifndef USE_LIST_CTRL_ON_FILE_LIST
#include "uicdlistctrl.h"
#else
#include "uiclistctrl.h"
#endif

class wxMenu;
class MyMenu;
class wxCustomDataObject;
class wxFileDataObject;
class wxButton;
class wxTextCtrl;
class wxRadioButton;
class wxChoice;
class wxStaticText;
class wxBoxSizer;
class DiskBasic;
class DiskBasics;
class DiskBasicGroupItem;
class DiskBasicDirItem;
class DiskBasicDirItems;
class DiskBasicFileName;
class DiskBasicDirItemAttr;
class UiDiskFrame;
class UiDiskFileList;
class DiskImageDisk;
class DiskImageSector;

enum en_disk_file_list_columns {
	LISTCOL_NAME = 0,
	LISTCOL_ATTR,
	LISTCOL_SIZE,
	LISTCOL_GROUPS,
	LISTCOL_START,
	LISTCOL_TRACK,
	LISTCOL_SIDE,
	LISTCOL_SECTOR,
	LISTCOL_DIVISION,
	LISTCOL_DATE,
	LISTCOL_STADDR,
	LISTCOL_EDADDR,
	LISTCOL_EXADDR,
	LISTCOL_NUM,
	LISTCOL_END
};

extern const struct st_list_columns gUiDiskFileListColumnDefs[];

//////////////////////////////////////////////////////////////////////

#ifndef USE_LIST_CTRL_ON_FILE_LIST
/// ファイルリストの挙動を設定
class UiDiskFileListStoreModel : public wxDataViewListStore
{
private:
	UiDiskFrame *frame;
	UiDiskFileList *ctrl;
public:
	UiDiskFileListStoreModel(UiDiskFrame *parentframe, wxWindow *parent);

	void SetControl(UiDiskFileList *n_ctrl) { ctrl = n_ctrl; }

	bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);

	int  Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const;
};

#define MyFileListColumn	wxDataViewColumn*
#define MyFileListItem		wxDataViewItem
#define MyFileListItems		wxDataViewItemArray
#define MyFileListEvent		wxDataViewEvent
#define MyFileListValue		MyCDListValue

#define MyFileListItem_IsOk(itm) itm.IsOk()
#define MyFileListItem_Unset(itm) itm = MyFileListItem()

#else
#define MyFileListColumn	long
#define MyFileListItem		long
#define MyFileListItems		wxArrayLong
#define MyFileListEvent		wxListEvent
#define MyFileListValue		MyCListValue

#define MyFileListItem_IsOk(itm) (itm != wxNOT_FOUND)
#define MyFileListItem_Unset(itm) itm = wxNOT_FOUND

#endif

//////////////////////////////////////////////////////////////////////

/// リストコントロール
#ifndef USE_LIST_CTRL_ON_FILE_LIST
class UiDiskFileListCtrl : public MyCDListCtrl
#else
class UiDiskFileListCtrl : public MyCListCtrl
#endif
{
protected:
	/// リストデータを設定
	void SetListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, MyFileListValue *values);
	/// リストにデータを挿入
	void InsertListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, wxUIntPtr data);
	/// リストデータを更新
	void UpdateListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, wxUIntPtr data);
	/// アイコンを指定
	int ChooseIconNumber(const DiskBasicDirItem *item) const;

public:
	UiDiskFileListCtrl(UiDiskFrame *parentframe, wxWindow *parent, wxWindowID id,
		const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);

	/// リストにデータを設定する
	void SetListItems(DiskBasic *basic);
	/// リストデータを更新
	void UpdateListItems(DiskBasic *basic);

#ifdef USE_LIST_CTRL_ON_FILE_LIST
	/// アイテムをソート
	void SortDataItems(DiskBasic *basic, int col);
	/// ソート用コールバック
	static int wxCALLBACK Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata);
#endif
	static int CompareName(DiskBasicDirItems *items, int i1, int i2, int dir);
	static int CompareAttr(DiskBasicDirItems *items, int i1, int i2, int dir);
	static int CompareSize(DiskBasicDirItems *items, int i1, int i2, int dir);
	static int CompareGroups(DiskBasicDirItems *items, int i1, int i2, int dir);
	static int CompareStart(DiskBasicDirItems *items, int i1, int i2, int dir);
//	static int CompareTrack(DiskBasicDirItems *items, int i1, int i2, int dir);
//	static int CompareSide(DiskBasicDirItems *items, int i1, int i2, int dir);
//	static int CompareSector(DiskBasicDirItems *items, int i1, int i2, int dir);
	static int CompareDate(DiskBasicDirItems *items, int i1, int i2, int dir);
	static int CompareNum(DiskBasicDirItems *items, int i1, int i2, int dir);
};

//////////////////////////////////////////////////////////////////////

/// 右パネルのファイルリスト
class UiDiskFileList : public wxPanel
{
private:
	wxWindow			*parent;
	UiDiskFrame			*frame;

	wxTextCtrl			*textAttr;
	wxButton			*btnChange;
	wxStaticText		*lblCharCode;
	wxChoice			*comCharCode;
	UiDiskFileListCtrl	*listCtrl;
	wxBoxSizer			*szrButtons;

	MyMenu				*menuPopup;
	wxMenu				*menuColumnPopup;

	/// BASICフォーマットの情報 左パネルのディスクを選択すると設定される
	DiskBasic		*m_current_basic;

	bool			 m_initialized;

	bool			 m_disk_selecting;

	MyFileListItem	 m_dragging_item;

	/// 現在選択している行のディレクトリアイテムを得る
	DiskBasicDirItem *GetSelectedDirItem();
	/// リストの指定行のディレクトリアイテムを得る
	DiskBasicDirItem *GetDirItem(const MyFileListItem &view_item, int *item_pos = NULL);
	/// リストの指定行のディレクトリアイテムとそのファイル名を得る
	DiskBasicDirItem *GetFileName(const MyFileListItem &view_item, wxString &name, int *item_pos = NULL);

	/// ファイルリスト作成（DnD, クリップボード用）
	bool CreateFileObject(wxString &tmp_dir_name, const wxString &start_msg, const wxString &end_msg, wxFileDataObject &file_object);
	/// ファイルリストを解放（DnD, クリップボード用）
	void ReleaseFileObject(const wxString &tmp_dir_name);

	/// 指定したフォルダにエクスポート
	int  ExportDataFiles(const MyFileListItems &selected_items, const wxString &data_dir, const wxString &attr_dir, const wxString &start_msg, const wxString &end_msg, wxFileDataObject *file_object = NULL);
	/// 指定したファイルを一括削除
	int  DeleteDataFiles(DiskBasic *tmp_basic, MyFileListItems &selected_items);

public:
	UiDiskFileList(UiDiskFrame *parentframe, wxWindow *parent);
	~UiDiskFileList();

	/// @name event procedures
	//@{
	MyFileListItem GetEventItem(const MyFileListEvent& event) const;
	/// リサイズ
	void OnSize(wxSizeEvent& event);
	/// リスト上で右クリック
	void OnListContextMenu(MyFileListEvent& event);
	/// セクタリスト カラムをクリック
	void OnColumnClick(MyFileListEvent& event);
	/// リストのカラム上で右クリック
	void OnListColumnContextMenu(MyFileListEvent& event);
#ifdef USE_LIST_CTRL_ON_FILE_LIST
	/// リストを選択
	void OnSelect(MyFileListEvent& event);
	/// リストを非選択
	void OnDeselect(MyFileListEvent& event);
#else
	/// リスト選択行を変更
	void OnSelectionChanged(MyFileListEvent& event);
#endif
	/// リストの編集開始
	void OnFileNameStartEditing(MyFileListEvent& event);
	/// リストの編集開始した
	void OnFileNameEditingStarted(MyFileListEvent& event);
	/// リストの編集終了
	void OnFileNameEditedDone(MyFileListEvent& event);
	/// リスト上でダブルクリック
	void OnListActivated(MyFileListEvent& event);
	/// リスト上でドラッグ開始
	void OnBeginDrag(MyFileListEvent& event);
	/// 右クリック
	void OnContextMenu(wxContextMenuEvent& event);
	/// エクスポート選択
	void OnExportFile(wxCommandEvent& event);
	/// インポート選択
	void OnImportFile(wxCommandEvent& event);
	/// 削除選択
	void OnDeleteFile(wxCommandEvent& event);
	/// リネーム選択
	void OnRenameFile(wxCommandEvent& event);
	/// コピー選択
	void OnCopyFile(wxCommandEvent& event);
	/// ペースト選択
	void OnPasteFile(wxCommandEvent& event);
	/// ディレクトリ作成選択
	void OnMakeDirectory(wxCommandEvent& event);
	/// ファイル編集選択
	void OnEditFile(wxCommandEvent& event);
	/// プロパティ選択
	void OnProperty(wxCommandEvent& event);
	/// リスト上でキー押下
	void OnChar(wxKeyEvent& event);
	/// 変更ボタン押下
	void OnButtonChange(wxCommandEvent& event);
	/// キャラクターコード変更ボタン押下
	void OnChangeCharCode(wxCommandEvent& event);
	/// リストのカラムを変更
	void OnListColumnChange(wxCommandEvent& event);
//	/// リストのカラム詳細設定
//	void OnListColumnDetail(wxCommandEvent& event);
//	/// リストのカラム詳細設定
//	void OnListColumnDetail(MyFileListEvent& event);
	//@}

	/// ポップアップメニュー作成
	void MakePopupMenu();
	/// ポップアップメニュー表示
	void ShowPopupMenu();
	/// リストカラムのポップアップメニュー表示
	void ShowColumnPopupMenu();

	/// BASIC種類テキストボックスに設定
	void SetAttr(const wxString &val);
	/// BASIC種類テキストボックスをクリア
	void ClearAttr();

	/// キャラクターコード変更
	void ChangeCharCode(const wxString &name);
	/// キャラクターコードの選択位置を変える
	void PushCharCode(const wxString &name);

	/// DISK BASICをアタッチ
	void AttachDiskBasic(DiskImageDisk *newdisk, int newsidenum);
	/// DISK BASICをデタッチ
	void DetachDiskBasic();
	/// DISK BASICを返す
	DiskBasic *GetDiskBasic() const { return m_current_basic; }
	/// ファイル名をリストに設定
	void SetFiles();
	/// ファイル名をリストに再設定
	void RefreshFiles();
//	/// リスト内容を更新
//	void UpdateFiles();
	/// リストをクリア
	void ClearFiles();

	/// 全行を選択
	void SelectAll();

	/// 行選択
	bool SelectItem(const MyFileListItem &selected_item, int count);
	// 行非選択
	void UnselectItem(const MyFileListItem &deselected_item, int count);

	/// データをダンプウィンドウに設定
	bool SetDumpData(DiskBasicGroupItem &group_item);
	/// データをダンプウィンドウに設定
	bool SetDumpData(int track_num, int side_num, int sector_start, int sector_end);

	/// エクスポートダイアログ
	void ShowExportDataFileDialog();

	/// ファイルリストをドラッグ
	bool DragDataSource();
	/// クリップボードへコピー
	bool CopyToClipboard();
	/// クリップボードからペースト
	bool PasteFromClipboard();

	/// インポートダイアログ
	void ShowImportDataFileDialog();
	/// ファイルをドロップ
	bool DropDataFiles(wxWindow *base, int x, int y, const wxArrayString &paths, bool dir_included);

	/// 指定したファイルを削除
	bool DeleteDataFile();

	/// 指定したファイルを編集
	void EditDataFile(enEditorTypes editor_type);

	/// ファイル名の編集開始
	void StartEditingFileName();
	/// ファイル名の編集開始
	void StartEditingFileName(const MyFileListItem &selected_item);
	/// 指定したファイル名を変更
	bool RenameDataFile(const MyFileListItem &view_item, const wxString &newname);

	/// ダブルクリックしたとき
	void DoubleClicked();

	/// ファイル属性プロパティダイアログを表示
	void ShowFileAttr();
	/// ファイル属性プロパティダイアログを表示
	bool ShowFileAttr(DiskBasicDirItem *item);
	/// ファイル属性プロパティダイアログの内容を反映
	void AcceptSubmittedFileAttr(IntNameBox *dlg);
	/// プロパティダイアログを閉じる
	void CloseAllFileAttr();

	/// BASIC種類を変更
	bool ChangeBasicType();
	/// BASIC情報ダイアログ
	void ShowBasicAttr();

	/// カラム変更ダイアログ
	void ShowListColumnDialog();

	/// ディレクトリ作成ダイアログ
	void ShowMakeDirectoryDialog();

	/// 選択している行数
	int  GetListSelectedItemCount() const;

	/// BASICディスクとして使用できるか
	bool CanUseBasicDisk() const;
	/// BASICディスクを解析したか
	bool IsAssignedBasicDisk() const;
	/// BASICディスク＆フォーマットできるか
	bool IsFormattableBasicDisk() const;
	/// BASICディスク＆フォーマットされているか
	bool IsFormattedBasicDisk() const;
	/// ファイルの書き込み可能か
	bool IsWritableBasicFile();
	/// ファイルの削除可能か
	bool IsDeletableBasicFile();

	/// FATエリアの空き状況を取得
	void GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const;

	/// フォントをセット
	void SetListFont(const wxFont &font);

	/// リストコントロールを返す
	UiDiskFileListCtrl *GetListCtrl() const { return listCtrl; }

	enum {
		IDM_EXPORT_FILE = 1,
		IDM_IMPORT_FILE,
		IDM_DELETE_FILE,
		IDM_RENAME_FILE,
		IDM_DUPLICATE_FILE,
		IDM_COPY_FILE,
		IDM_PASTE_FILE,
		IDM_EDIT_FILE_BINARY,
		IDM_EDIT_FILE_TEXT,
		IDM_MAKE_DIRECTORY,
		IDM_PROPERTY,
		IDC_TEXT_ATTR,
		IDC_BTN_CHANGE,
		IDC_RADIO_CHAR_ASCII,
		IDC_RADIO_CHAR_SJIS,
		IDC_COMBO_CHAR_CODE,
		IDC_VIEW_LIST,
		IDM_COLUMN_DETAIL,
		IDM_COLUMN_0,
	};

	wxDECLARE_EVENT_TABLE();
};

#endif /* UIFILELIST_H */

