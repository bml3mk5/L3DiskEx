/// @file uifilelist.h
///
/// @brief BASICファイル名一覧
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UIFILELIST_H_
#define _UIFILELIST_H_

#include "common.h"
#include <wx/string.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>


#define USE_LIST_CTRL_ON_FILE_LIST
#undef  USE_VIRTUAL_ON_LIST_CTRL

#ifndef USE_LIST_CTRL_ON_FILE_LIST
#include <wx/dataview.h>
#include <wx/clntdata.h>
#else
#include <wx/listctrl.h>
#include <wx/dynarray.h>
#endif

class wxCustomDataObject;
class wxFileDataObject;
class wxButton;
class wxTextCtrl;
class wxRadioButton;
class wxStaticText;
class wxBoxSizer;
class DiskBasic;
class DiskBasics;
class DiskBasicGroupItem;
class DiskBasicDirItem;
class DiskBasicDirItems;
class L3DiskFrame;
class DiskD88Disk;
class DiskD88Sector;
class IntNameBox;

// リストアイテム
#ifndef USE_LIST_CTRL_ON_FILE_LIST
#define L3FileListColumn	wxDataViewColumn*
#define L3FileListItem		wxDataViewItem
#define L3FileListItems		wxDataViewItemArray
#define L3FileListEvent		wxDataViewEvent
#else
#define L3FileListColumn	long
#define L3FileListItem		long
#define L3FileListItems		wxArrayLong
#define L3FileListEvent		wxListEvent
#endif

enum en_disk_file_list_columns {
	LISTCOL_NAME = 0,
	LISTCOL_ATTR,
	LISTCOL_SIZE,
	LISTCOL_GROUPS,
	LISTCOL_START,
	LISTCOL_TRACK,
	LISTCOL_SIDE,
	LISTCOL_SECTOR,
	LISTCOL_DATE,
	LISTCOL_END
};

//////////////////////////////////////////////////////////////////////

#ifndef USE_LIST_CTRL_ON_FILE_LIST
/// ファイルリストの挙動を設定
class L3DiskFileListStoreModel : public wxDataViewListStore
{
private:
	L3DiskFrame *frame;

public:
	L3DiskFileListStoreModel(L3DiskFrame *parentframe);

	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
};
#endif

//////////////////////////////////////////////////////////////////////

/// リストコントロール
#ifndef USE_LIST_CTRL_ON_FILE_LIST
class L3DiskFileListCtrl : public wxDataViewListCtrl
#else
class L3DiskFileListCtrl : public wxListCtrl
#endif
{
private:
	L3FileListColumn	listColumns[LISTCOL_END];
	wxString			m_values[LISTCOL_END];
	int					m_icon;

public:
	L3DiskFileListCtrl(L3DiskFrame *parentframe, wxWindow *parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	~L3DiskFileListCtrl();

	/// リストにデータを挿入
	long InsertListData(long row, int icon, const wxString &filename, const wxString &attr, int size, int groups, int start, int trk, int sid, int sec, const wxString &date, wxUIntPtr data);
	/// リストにデータを設定する
	void SetListData(DiskBasic *basic, const DiskBasicDirItems *dir_items);

//	int GetDirItemPos(const L3FileListItem &item) const;
	/// ファイル名テキストを設定
	void SetListText(const L3FileListItem &item, const wxString &text);
	/// 選択している行の位置を返す
	int  GetListSelectedRow() const;
	/// 選択している行数
	int  GetListSelectedItemCount() const;
	/// 選択している行アイテムを得る
	L3FileListItem GetListSelection() const;
	/// 選択している行アイテムを得る
	int GetListSelections(L3FileListItems &arr) const;
	/// アイテムを編集
	void EditListItem(const L3FileListItem &item);
	/// リストを削除
	bool DeleteAllListItems();
	/// アイテムの固有データを返す
	wxUIntPtr GetListItemData(const L3FileListItem &item) const;
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
	wxRadioButton *radCharAscii;
	wxRadioButton *radCharSJIS;
	L3DiskFileListCtrl *list;
	wxBoxSizer *szrButtons;

	wxMenu *menuPopup;

	/// BASICフォーマットの情報 左パネルのディスクを選択すると設定される
	DiskBasic *basic;

	bool initialized;

	bool disk_selecting;

	/// 現在選択している行のディレクトリアイテムを得る
	DiskBasicDirItem *GetSelectedDirItem();
	/// リストの指定行のディレクトリアイテムを得る
	DiskBasicDirItem *GetDirItem(const L3FileListItem &view_item, int *item_pos = NULL);
	/// リストの指定行のディレクトリアイテムとそのファイル名を得る
	DiskBasicDirItem *GetFileName(const L3FileListItem &view_item, wxString &name, int *item_pos = NULL);
	/// ファイル名ダイアログ表示と同じファイル名が存在する際のメッセージダイアログ表示
	int ShowIntNameBoxAndCheckSameFile(IntNameBox &dlg, DiskBasicDirItem *item, int file_size);

	// カスタムデータリスト作成（DnD, クリップボード用）
	bool CreateCustomDataObject(wxCustomDataObject &data_object);
	// ファイルリスト作成（DnD, クリップボード用）
	bool CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object);
	// ファイルリストを解放（DnD, クリップボード用）
	void ReleaseFileObject(const wxString &tmp_dir_name);

	/// 指定したファイルを削除
	bool DeleteDataFile(DiskBasic *tmp_basic, DiskBasicDirItem *dst_item);
	/// 指定したファイルを一括削除
	bool DeleteDataFiles(DiskBasic *tmp_basic, L3FileListItems &selected_items);

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
	/// リストの選択行を変更した
	void OnSelectionChanged(L3FileListEvent& event);
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
	//@}

	/// ポップアップメニュー表示
	void ShowPopupMenu();

	/// BASIC種類テキストボックスに設定
	void SetAttr(const wxString &val);
	/// BASIC種類テキストボックスをクリア
	void ClearAttr();

	/// キャラクターコード変更
	void ChangeCharCode(int sel);
	/// キャラクターコードの選択位置を変える
	void PushCharCode(int sel);

	/// DISK BASICをアタッチ
	void AttachDiskBasic(DiskD88Disk *newdisk, int newsidenum);
	/// DISK BASICをデタッチ
	void DetachDiskBasic();
	/// ファイル名をリストに設定
	void SetFiles();
	/// ファイル名をリストに再設定
	void RefreshFiles();
	/// リストをクリア
	void ClearFiles();

	/// 行選択
	void SelectItem(wxUint32 group_num, DiskD88Sector *sector, int char_code, bool invert);
	// 行非選択
	void UnselectItem();

	/// データをダンプウィンドウに設定
	bool SetDumpData(DiskBasicGroupItem &group_item);
	/// データをダンプウィンドウに設定
	bool SetDumpData(int track, int side, int sector_start, int sector_end);

	/// エクスポートダイアログ
	bool ShowExportDataFileDialog();
	/// 指定したファイルにエクスポート
	bool ExportDataFile(DiskBasicDirItem *item, const wxString &path);

	/// ファイルリストをドラッグ
	bool DragDataSource();
	/// クリップボードへコピー
	bool CopyToClipboard();
	/// クリップボードからペースト
	bool PasteFromClipboard();

	/// インポートダイアログ
	bool ShowImportDataFileDialog();
	/// 指定したファイルをインポート 外部から
	bool ImportDataFile(const wxString &path);
	/// 指定したデータをインポート（内部でのドラッグ＆ドロップ時など）
	bool ImportDataFiles(const wxUint8 *buffer, size_t buflen);
	/// 指定したデータをインポート（内部でのドラッグ＆ドロップ時など）
	bool ImportDataFile(const wxUint8 *buffer, size_t buflen);

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

	/// ディレクトリを作成できるか
	bool CanMakeDirectory() const;
	/// ディレクトリを作成
	bool MakeDirectory();

	/// 選択している行数
	int  GetListSelectedItemCount() const;

	/// BASICディスクとして使用できるか
	bool CanUseBasicDisk() const;
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

	enum {
		IDM_EXPORT_FILE = 1,
		IDM_IMPORT_FILE,
		IDM_DELETE_FILE,
		IDM_RENAME_FILE,
		IDM_MAKE_DIRECTORY,
		IDM_PROPERTY,
		IDC_TEXT_ATTR,
		IDC_BTN_CHANGE,
		IDC_RADIO_CHAR_ASCII,
		IDC_RADIO_CHAR_SJIS,
		IDC_VIEW_LIST,
	};

	wxDECLARE_EVENT_TABLE();
};

#endif /* _UIFILELIST_H_ */

