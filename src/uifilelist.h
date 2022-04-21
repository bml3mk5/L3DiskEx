/// @file uifilelist.h
///
/// @brief BASICファイル名一覧
///
#ifndef _UIFILELIST_H_
#define _UIFILELIST_H_

#include "common.h"
#include <wx/string.h>
#include <wx/panel.h>
#include <wx/dataview.h>
#include <wx/clntdata.h>
#include <wx/radiobut.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>


class DiskBasic;
class DiskBasicGroupItem;
class DiskBasicDirItem;
class DiskBasicDirItems;
class L3DiskFrame;
class DiskD88Disk;
class DiskD88Sector;
class IntNameBox;

#define L3FileListColumn	wxDataViewColumn*
#define L3FileListItem		wxDataViewItem
#define L3FileListItems		wxDataViewItemArray
#define L3FileListEvent		wxDataViewEvent
/// ファイルリストの挙動を設定
class L3DiskFileListStoreModel : public wxDataViewListStore
{
private:
	L3DiskFrame *frame;

public:
	L3DiskFileListStoreModel(L3DiskFrame *parentframe);

	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
};

/// リストコントロール
class MyListCtrl : public wxDataViewListCtrl
{
private:
	enum {
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

	L3FileListColumn	listColumns[LISTCOL_END];
public:
	MyListCtrl(L3DiskFrame *parentframe, wxWindow *parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	~MyListCtrl();

	long InsertListData(long row, int icon, const wxString &filename, const wxString &attr, int size, int groups, int start, int trk, int sid, int sec, const wxString &date, wxUIntPtr data);
	void SetListData(DiskBasic *basic, DiskBasicDirItems *dir_items);

	int GetDirItemPos(const L3FileListItem &item) const;
};

/// 右パネルのファイルリスト
class L3DiskFileList : public wxPanel
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	wxTextCtrl *textAttr;
	wxButton *btnChange;
	wxRadioButton *radCharAscii;
	wxRadioButton *radCharSJIS;
	MyListCtrl *list;

	wxMenu *menuPopup;

	/// BASICフォーマットの情報
	DiskBasic *basic;

	bool initialized;

	/// 現在選択している行のディレクトリアイテムを得る
	DiskBasicDirItem *GetSelectedDirItem();
	/// リストの指定行のディレクトリアイテムを得る
	DiskBasicDirItem *GetDirItem(const L3FileListItem &view_item, int *item_pos = NULL);
	/// リストの指定行のディレクトリアイテムとそのファイル名を得る
	DiskBasicDirItem *GetFileName(const L3FileListItem &view_item, wxString &name, int *item_pos = NULL);
	/// ファイル名ダイアログ表示と同じファイル名が存在する際のメッセージダイアログ表示
	int ShowIntNameBoxAndCheckSameFile(IntNameBox &dlg, DiskBasicDirItem *item, int file_size);

	// ファイルリスト作成（DnD, クリップボード用）
	bool CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object);
	// ファイルリストを解放（DnD, クリップボード用）
	void ReleaseFileObject(const wxString &tmp_dir_name);

public:
	L3DiskFileList(L3DiskFrame *parentframe, wxWindow *parent);
	~L3DiskFileList();

	/// @name event procedures
	//@{
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

	/// ファイル名をリストにセット
	void SetFiles(DiskD88Disk *newdisk, int newsidenum);
	/// ファイル名をリストに再設定
	void RefreshFiles();
	/// リストをクリア
	void ClearFiles();

	/// 行選択
	void SelectItem(wxUint32 group_num, DiskD88Sector *sector, int char_code, bool invert);
//	void SelectItem(const DiskBasicGroups *group_items, int char_code, bool invert);
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

	/// ファイルリストをドラッグ 内部へドロップする場合
	bool DragDataSourceForInternal();
	/// ファイルリストをドラッグ 外部へドロップする場合
	bool DragDataSourceForExternal();
	/// クリップボードへコピー
	bool CopyToClipboard();
	/// クリップボードからペースト
	bool PasteFromClipboard();

	/// インポートダイアログ
	bool ShowImportDataFileDialog();
	/// 指定したファイルをインポート 外部から
	bool ImportDataFile(const wxString &path);
	/// 指定したデータをインポート（内部でのドラッグ＆ドロップ時など）
	bool ImportDataFile(const wxUint8 *buffer, size_t buflen);

	/// 指定したファイルを削除
	bool DeleteDataFile();

	/// ファイル名の編集開始
	void StartEditingFileName();
	/// 指定したファイル名を変更
	bool RenameDataFile(const L3FileListItem &view_item, const wxString &newname);

	/// ダブルクリックしたとき
	void DoubleClicked();

	/// ディレクトリを移動する
	bool ChangeDirectory(DiskBasicDirItem *dir_item);

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

	/// ディスクを論理フォーマット
	bool FormatDisk();

	/// ディレクトリを作成できるか
	bool CanMakeDirectory() const;
	/// ディレクトリを作成
	bool MakeDirectory();

	/// 選択している行の位置を返す
	int  GetListSelectedRow() const;
	/// 選択している行数
	int  GetListSelectedItemCount() const;
	/// 選択している行アイテムを得る
	L3FileListItem GetListSelection() const;
	/// 選択している行アイテムを得る
	int GetListSelections(L3FileListItems &arr) const;

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

#ifndef USE_DND_ON_TOP_PANEL
/// ディスクファイル ドラッグ＆ドロップ
class L3DiskFileListDropTarget : public wxDropTarget
{
	L3DiskFileList *parent;
    L3DiskFrame *frame;

public:
    L3DiskFileListDropTarget(L3DiskFrame *parentframe, L3DiskFileList *parentwindow);

	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
    bool OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames);
};
#endif

#endif /* _UIFILELIST_H_ */

