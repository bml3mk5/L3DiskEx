/// @file uifilelist.h
///
/// @brief BASICファイル名一覧
///
#ifndef _UIFILELIST_H_
#define _UIFILELIST_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dataview.h>
#include <wx/clntdata.h>
#include <wx/radiobut.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include "basicfmt.h"

class DiskBasicGroupItem;
class L3DiskFrame;
class DiskD88Disk;
class IntNameBox;

#if 0
/// 
class L3DiskListItem
{
public:
	int file_type;
	int data_type;
public:
	L3DiskListItem(int newfiletype, int newdatatype);
	~L3DiskListItem() {}
};
#endif

/// ファイルリストの挙動を設定
class L3DiskFileListStoreModel : public wxDataViewListStore
{
private:
	L3DiskFrame *frame;

public:
	L3DiskFileListStoreModel(L3DiskFrame *parentframe);

	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
};

#if 0
/// ファイルリスト内のテキストコントロール
class L3DiskFileListRenderer : public wxDataViewTextRenderer
{
public:
	L3DiskFileListRenderer();

	wxWindow *CreateEditorCtrl(wxWindow *parent, wxRect labelRect, const wxVariant &value);
	bool Validate(wxVariant &value);
};

/// ファイルリストコントロール
class L3DiskFileListCtrl : public wxDataViewListCtrl
{
public:
	L3DiskFileListCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxDV_ROW_LINES, const wxValidator &validator=wxDefaultValidator);
	
	bool ProcessEvent(wxEvent &event);
};
#endif

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
	wxDataViewListCtrl *list;

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

	wxDataViewColumn *listColumns[LISTCOL_END];

	wxMenu *menuPopup;

	// BASICフォーマットの情報
	DiskBasic basic;

	bool initialized;

	/// 現在選択している行のディレクトリアイテムを得る
	DiskBasicDirItem *GetSelectedDirItem();
	/// リストの指定行のディレクトリアイテムを得る
	DiskBasicDirItem *GetDirItem(const wxDataViewItem &view_item, int *item_pos = NULL);
	/// リストの指定行のディレクトリアイテムとそのファイル名を得る
	DiskBasicDirItem *GetFileName(const wxDataViewItem &view_item, wxString &name, int *item_pos = NULL);
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
	void OnDataViewItemContextMenu(wxDataViewEvent& event);
	/// リストの選択行を変更した
	void OnSelectionChanged(wxDataViewEvent& event);
	/// リストの編集開始
	void OnFileNameStartEditing(wxDataViewEvent& event);
	/// リストの編集開始した
	void OnFileNameEditingStarted(wxDataViewEvent& event);
	/// リストの編集終了
	void OnFileNameEditedDone(wxDataViewEvent& event);
	/// 右クリック
	void OnContextMenu(wxContextMenuEvent& event);
	/// リスト上でダブルクリック
	void OnDataViewItemActivated(wxDataViewEvent& event);
	/// リスト上でドラッグ開始
	void OnBeginDrag(wxDataViewEvent& event);
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
	bool RenameDataFile(const wxDataViewItem &view_item, const wxString &newname);

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
	int  GetSelectedRow() const;
	/// 選択している行数
	int  GetSelectedItemCount() const;

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

