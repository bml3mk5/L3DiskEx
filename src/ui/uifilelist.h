/// @file uifilelist.h
///
/// @brief BASICファイル名一覧
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UIFILELIST_H_
#define _UIFILELIST_H_

#include "uicommon.h"
#include <wx/string.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/stopwatch.h>
#include <wx/msgdlg.h>
#include "intnamebox.h"


#ifndef USE_LIST_CTRL_ON_FILE_LIST
#include "uicdlistctrl.h"
#else
#include "uiclistctrl.h"
#endif

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
class L3DiskFrame;
class L3DiskFileList;
class DiskD88Disk;
class DiskD88Sector;

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

extern const struct st_list_columns gL3DiskFileListColumnDefs[];

//////////////////////////////////////////////////////////////////////

#ifndef USE_LIST_CTRL_ON_FILE_LIST
/// ファイルリストの挙動を設定
class L3DiskFileListStoreModel : public wxDataViewListStore
{
private:
	L3DiskFrame *frame;
	L3DiskFileList *ctrl;
public:
	L3DiskFileListStoreModel(L3DiskFrame *parentframe, wxWindow *parent);

	void SetControl(L3DiskFileList *n_ctrl) { ctrl = n_ctrl; }

	bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);

	int  Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const;
};

#define L3FileListColumn	wxDataViewColumn*
#define L3FileListItem		wxDataViewItem
#define L3FileListItems		wxDataViewItemArray
#define L3FileListEvent		wxDataViewEvent
#define L3FileListValue		L3CDListValue
#else
#define L3FileListColumn	long
#define L3FileListItem		long
#define L3FileListItems		wxArrayLong
#define L3FileListEvent		wxListEvent
#define L3FileListValue		L3CListValue
#endif

//////////////////////////////////////////////////////////////////////

/// リストコントロール
#ifndef USE_LIST_CTRL_ON_FILE_LIST
class L3DiskFileListCtrl : public L3CDListCtrl
#else
class L3DiskFileListCtrl : public L3CListCtrl
#endif
{
protected:
	/// リストデータを設定
	void SetListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, L3FileListValue *values);
	/// リストにデータを挿入
	void InsertListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, wxUIntPtr data);
	/// リストデータを更新
	void UpdateListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, wxUIntPtr data);
	/// アイコンを指定
	int ChooseIconNumber(const DiskBasicDirItem *item) const;

public:
	L3DiskFileListCtrl(L3DiskFrame *parentframe, wxWindow *parent, wxWindowID id,
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

/// ストップウォッチ
class L3StopWatch : public wxStopWatch
{
private:
	bool m_now_wait_cursor;
public:
	L3StopWatch();
	void Busy();
	void Restart();
	void Finish();
};

//////////////////////////////////////////////////////////////////////

/// 右パネルのファイルリスト
class L3DiskFileList : public wxPanel
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	wxTextCtrl *textAttr;
	wxButton *btnChange;
	wxStaticText *lblCharCode;
	wxChoice *comCharCode;
	L3DiskFileListCtrl *list;
	wxBoxSizer *szrButtons;

	wxMenu *menuPopup;
	wxMenu *menuColumnPopup;

	/// BASICフォーマットの情報 左パネルのディスクを選択すると設定される
	DiskBasic *basic;

	bool initialized;

	bool disk_selecting;

	L3StopWatch m_sw_export;	///< エクスポート時のストップウォッチ
	int m_sc_export;
	L3StopWatch m_sw_import;	///< インポート時のストップウォッチ
	int m_sc_import;

	/// 現在選択している行のディレクトリアイテムを得る
	DiskBasicDirItem *GetSelectedDirItem();
	/// リストの指定行のディレクトリアイテムを得る
	DiskBasicDirItem *GetDirItem(const L3FileListItem &view_item, int *item_pos = NULL);
	/// リストの指定行のディレクトリアイテムとそのファイル名を得る
	DiskBasicDirItem *GetFileName(const L3FileListItem &view_item, wxString &name, int *item_pos = NULL);
	/// ファイル名ダイアログ表示と同じファイル名が存在する際のメッセージダイアログ表示
	int ShowIntNameBoxAndCheckSameFile(DiskBasicDirItem *temp_item, const wxString &file_name, int file_size, DiskBasicDirItemAttr &date_time, int style);
	/// ファイル名ダイアログの内容を反映させる
	bool SetDirItemFromIntNameDialog(DiskBasicDirItem *item, IntNameBox &dlg, DiskBasic *basic, bool rename);
	/// ファイル名を反映させる
	bool SetDirItemFromIntNameParam(DiskBasicDirItem *item, const wxString &file_path, const wxString &intname, DiskBasicDirItemAttr &date_time, DiskBasic *basic, bool rename);

	/// ファイルリスト作成（DnD, クリップボード用）
	bool CreateFileObject(wxString &tmp_dir_name, const wxString &start_msg, const wxString &end_msg, wxFileDataObject &file_object);
	/// ファイルリストを解放（DnD, クリップボード用）
	void ReleaseFileObject(const wxString &tmp_dir_name);

	/// 指定したファイルにエクスポート
	bool ExportDataFile(DiskBasicDirItem *item, const wxString &path, const wxString &start_msg, const wxString &end_msg);
	/// 指定したフォルダにエクスポート
	int  ExportDataFiles(DiskBasicDirItems *dir_items, const wxString &data_dir, const wxString &attr_dir, wxFileDataObject *file_object, int depth);

	/// 指定したファイルをインポート
	int  ImportDataFiles(const wxString &data_dir, const wxString &attr_dir, const wxArrayString &names, int depth);
	/// 指定したファイルをインポート
	int  ImportDataFile(const wxString &full_data_path, const wxString &full_attr_path, const wxString &file_name);

	/// 指定したファイルを削除
	int  DeleteDataFile(DiskBasic *tmp_basic, DiskBasicDirItem *dst_item);
	/// 指定したファイルを一括削除
	int  DeleteDataFiles(DiskBasic *tmp_basic, L3FileListItems &selected_items);
	/// 指定したファイルを一括削除
	int  DeleteDataFiles(DiskBasic *tmp_basic, DiskBasicDirItems &items, int depth, DiskBasicDirItems *dir_items);

public:
	L3DiskFileList(L3DiskFrame *parentframe, wxWindow *parent);
	~L3DiskFileList();

	/// @name event procedures
	//@{
	L3FileListItem GetEventItem(const L3FileListEvent& event) const;
	/// リサイズ
	void OnSize(wxSizeEvent& event);
	/// リスト上で右クリック
	void OnListContextMenu(L3FileListEvent& event);
	/// セクタリスト カラムをクリック
	void OnColumnClick(L3FileListEvent& event);
	/// リストのカラム上で右クリック
	void OnListColumnContextMenu(L3FileListEvent& event);
#ifdef USE_LIST_CTRL_ON_FILE_LIST
	/// リストを選択
	void OnSelect(L3FileListEvent& event);
	/// リストを非選択
	void OnDeselect(L3FileListEvent& event);
#else
	/// リスト選択行を変更
	void OnSelectionChanged(L3FileListEvent& event);
#endif
	/// リストの編集開始
	void OnFileNameStartEditing(L3FileListEvent& event);
	/// リストの編集開始した
	void OnFileNameEditingStarted(L3FileListEvent& event);
	/// リストの編集終了
	void OnFileNameEditedDone(L3FileListEvent& event);
	/// リスト上でダブルクリック
	void OnListActivated(L3FileListEvent& event);
	/// リスト上でドラッグ開始
	void OnBeginDrag(L3FileListEvent& event);
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
	/// リストのカラム詳細設定
	void OnListColumnDetail(wxCommandEvent& event);
	//@}

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
	void AttachDiskBasic(DiskD88Disk *newdisk, int newsidenum);
	/// DISK BASICをデタッチ
	void DetachDiskBasic();
	/// DISK BASICを返す
	DiskBasic *GetDiskBasic() const { return basic; }
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
	bool SelectItem(const L3FileListItem &selected_item, int count);
//	/// 行選択
//	void SelectItem(wxUint32 group_num, DiskD88Sector *sector, const wxString &char_code, bool invert);
	// 行非選択
	void UnselectItem(const L3FileListItem &deselected_item, int count);

	/// データをダンプウィンドウに設定
	bool SetDumpData(DiskBasicGroupItem &group_item);
	/// データをダンプウィンドウに設定
	bool SetDumpData(int track, int side, int sector_start, int sector_end);

	/// エクスポートダイアログ
	void ShowExportDataFileDialog();
	/// 指定したフォルダにエクスポート
	int  ExportDataFiles(const L3FileListItems &selected_items, const wxString &data_dir, const wxString &attr_dir, const wxString &start_msg, const wxString &end_msg, wxFileDataObject *file_object = NULL);

	/// ファイルリストをドラッグ
	bool DragDataSource();
	/// クリップボードへコピー
	bool CopyToClipboard();
	/// クリップボードからペースト
	bool PasteFromClipboard();

	/// インポートダイアログ
	void ShowImportDataFileDialog();
	/// ファイルをドロップ
	bool DropDataFiles(const wxArrayString &paths);
	/// 指定したファイルをインポート
	bool ImportDataFiles(const wxArrayString &paths, const wxString &start_msg, const wxString &end_msg);

	/// 指定したファイルを削除
	bool DeleteDataFile();
	/// ディレクトリを削除する
	bool DeleteDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dst_item);

	/// ファイル名の編集開始
	void StartEditingFileName();
	/// ファイル名の編集開始
	void StartEditingFileName(const L3FileListItem &selected_item);
	/// 指定したファイル名を変更
	bool RenameDataFile(const L3FileListItem &view_item, const wxString &newname);

	/// ダブルクリックしたとき
	void DoubleClicked();

	/// ディレクトリをアサインする
	bool AssignDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dst_item);
	/// ディレクトリを移動する
	bool ChangeDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dst_item, bool refresh_list);

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

	/// ディレクトリを作成できるか
	bool CanMakeDirectory() const;
	/// ディレクトリ作成ダイアログ
	void ShowMakeDirectoryDialog();
	/// ディレクトリ作成
	bool MakeDirectory(const wxString &name, const wxString &title, DiskBasicDirItem **nitem);

	/// 選択している行数
	int  GetListSelectedItemCount() const;

	/// BASICディスクとして使用できるか
	bool CanUseBasicDisk() const;
	/// BASICディスクを解析したか
	bool IsAssignedBasicDisk() const;
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
	L3DiskFileListCtrl *GetListCtrl() const { return list; }

	enum {
		IDM_EXPORT_FILE = 1,
		IDM_IMPORT_FILE,
		IDM_DELETE_FILE,
		IDM_RENAME_FILE,
		IDM_DUPLICATE_FILE,
		IDM_COPY_FILE,
		IDM_PASTE_FILE,
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

#endif /* _UIFILELIST_H_ */

