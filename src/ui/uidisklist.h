/// @file uidisklist.h
///
/// @brief ディスクリスト
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef UIDISKLIST_H
#define UIDISKLIST_H

#include "uicommon.h"
#include <wx/string.h>


#ifndef USE_TREE_CTRL_ON_DISK_LIST
#include "uicdtreectrl.h"
#else
#include "uictreectrl.h"
#endif


class wxCustomDataObject;
class wxFileDataObject;
class MyMenu;
class UiDiskFrame;
class DiskImageFile;
class DiskImageDisks;
class DiskImageDisk;
class DiskBasic;
class DiskBasicParam;
class DiskBasicDirItem;
class DiskBasicDirItems;

enum UiDiskPositionDataNum {
	CD_DISKNUM_ROOT = -1,
	CD_TYPENUM_NODE = -1,
	CD_TYPENUM_NODE_AB = -2,
	CD_TYPENUM_NODE_BOTH = -3,
	CD_TYPENUM_NODE_DIR = -4,
};

//////////////////////////////////////////////////////////////////////

/// ディスク情報に紐づける属性
#ifndef USE_TREE_CTRL_ON_DISK_LIST
class UiDiskPositionData : public wxClientData
#else
class UiDiskPositionData : public wxTreeItemData
#endif
{
private:
	int disknum;
	int typenum;
	int sidenum;
	int pos;
	bool editable;
	bool shown;
	DiskBasicDirItem *ditem;
public:
	UiDiskPositionData(int n_disknum, int n_typenum, int n_sidenum, int n_pos, bool n_editable, DiskBasicDirItem *n_ditem = NULL);
	~UiDiskPositionData();

	int  GetDiskNumber() const { return disknum; }
	int  GetTypeNumber() const { return typenum; }
	void SetTypeNumber(int val) { typenum = val; }
	int  GetSideNumber() const { return sidenum; }
	int  GetPosition() const { return pos; }
	bool GetEditable() const { return editable; }
	/// このノードの子供を表示したか
	bool IsShown() const { return shown; }
	/// このノードの子供を設定して表示したらtrueにする
	void Shown(bool val) { shown = val; }
	DiskBasicDirItem *GetDiskBasicDirItem() { return ditem; }
	void SetDiskBasicDirItem(DiskBasicDirItem *val) { ditem = val; }
};

//////////////////////////////////////////////////////////////////////

#ifndef USE_TREE_CTRL_ON_DISK_LIST
/// ディスク１枚部分のツリーアイテムの挙動を設定する
class UiDiskTreeStoreModel : public wxDataViewTreeStore
{
private:
	UiDiskFrame *frame;
public:
	UiDiskTreeStoreModel(UiDiskFrame *parentframe);

	/// ディスク名編集できるか
	virtual bool IsEnabled(const wxDataViewItem &item, unsigned int col) const;
	/// ディスク名を変更した
	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
};

#define UiDiskListItem  wxDataViewItem
#define UiDiskListEvent wxDataViewEvent
#define UiDiskTreeIdVal	unsigned int
#define UiDiskListItems wxDataViewItemArray

#define UiDiskListItem_IsOk(itm) itm.IsOk()
#define UiDiskListItem_Unset(itm) itm = UiDiskListItem()

#else

#define UiDiskListItem  wxTreeItemId
#define UiDiskListEvent wxTreeEvent
#define UiDiskTreeIdVal	wxTreeItemIdValue
#define UiDiskListItems wxArrayTreeItemIds

#define UiDiskListItem_IsOk(itm) itm.IsOk()
#define UiDiskListItem_Unset(itm) itm.Unset()

#endif

//////////////////////////////////////////////////////////////////////

/// ツリーコントロール
#ifndef USE_TREE_CTRL_ON_DISK_LIST
class UiDiskTreeCtrl: public MyCDTreeCtrl
#else
class UiDiskTreeCtrl: public MyCTreeCtrl
#endif
{
public:
	UiDiskTreeCtrl(wxWindow *parentwindow, wxWindowID id);
};

//////////////////////////////////////////////////////////////////////

/// 左Panel ディスク情報
class UiDiskList: public UiDiskTreeCtrl
{
private:
	wxWindow		*parent;
	UiDiskFrame		*frame;

	MyMenu			*menuPopup;

	DiskImageDisk	*m_selected_disk;
	bool			 m_disk_selecting;

	bool			 m_initialized;

	UiDiskListItem	 m_root_node;

	UiDiskListItem	 m_dragging_node;

	/// ディレクトリを更新
	void RefreshDirectorySub(DiskImageDisk *disk, const UiDiskListItem &node, UiDiskPositionData *cd, DiskBasicDirItem *dir_item);
	/// 選択位置のディスクイメージ
	UiDiskListItem SetSelectedItemAtDiskImage();
	/// サブキャプション
	void SubCaption(int type, int side_number, wxString &caption) const;
	/// ファイルリスト作成（DnD, クリップボード用）
	bool CreateFileObject(const UiDiskListItem &sel_node, wxString &tmp_dir_name, const wxString &start_msg, const wxString &end_msg, wxFileDataObject &file_object);

public:
	UiDiskList(UiDiskFrame *parentframe, wxWindow *parent);
	~UiDiskList();

	enum SetDataOnItemNodeFlags {
		NODE_SELECTED = 0,
		NODE_EXPANDED
	};

	/// @name event procedures
	//@{
	/// コピー選択
	void OnCopyFile(wxCommandEvent& event);
	/// ペースト選択
	void OnPasteFile(wxCommandEvent& event);
	/// ツリー上でドラッグ開始
	void OnBeginDrag(UiDiskListEvent& event);
	/// 右クリック選択
	void OnContextMenu(UiDiskListEvent& event);
	/// ツリーアイテム選択
	void OnSelectionChanged(UiDiskListEvent& event);
	/// アイテム展開
	void OnItemExpanding(UiDiskListEvent& event);
	/// アイテム編集開始
	void OnStartEditing(UiDiskListEvent& event);
	/// アイテム編集終了
	void OnEditingDone(UiDiskListEvent& event);
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
	/// ディレクトリを削除選択
	void OnDeleteDirectory(wxCommandEvent& event);
	/// 初期化選択
	void OnInitializeDisk(wxCommandEvent& event);
	/// フォーマット選択
	void OnFormatDisk(wxCommandEvent& event);
	/// プロパティ選択
	void OnPropertyDisk(wxCommandEvent& event);
	/// BASIC情報選択
	void OnPropertyBasic(wxCommandEvent& event);
	/// キー入力
	void OnChar(wxKeyEvent& event);
	//@}

	/// @name functions
	//@{
	/// ポップアップメニュー作成
	void MakePopupMenu();
	/// ポップアップメニュー表示
	void ShowPopupMenu();

	/// ディレクトリ/ディスク削除選択
	void SelectDeleting();
	/// フォーカスのあるディレクトリを削除
	bool DeleteDirectory();

	/// 再選択
	void ReSelect(const DiskBasicParam *newparam = NULL);
	/// ツリーを選択
	void ChangeSelection(UiDiskListItem &node, const DiskBasicParam *newparam = NULL);
	/// ディスクを指定して選択状態にする
	void ChangeSelection(int disk_number, int side_number);
	/// ツリーを展開
	void ExpandItemNode(UiDiskListItem &node);
	/// 指定ノードにデータを設定する
	void SetDataOnItemNode(const UiDiskListItem &node, SetDataOnItemNodeFlags flag, const DiskBasicParam *newparam = NULL);
	/// DISK BASICをアタッチ＆解析
	bool ParseDiskBasic(const UiDiskListItem &node, UiDiskPositionData *cd, DiskImageDisk *newdisk, int newsidenum, const DiskBasicParam *newparam = NULL);
	/// 選択しているディスクの子供を削除
	void DeleteChildrenOnSelectedDisk();
	/// 選択しているディスクのルートを初期化＆再選択
	void RefreshSelectedDisk(const DiskBasicParam *newparam = NULL);
	/// 選択しているサイドを再選択
	void RefreshSelectedSide(const DiskBasicParam *newparam = NULL);
	/// ファイル名をリストにセット
	void SetFileName();
	/// ファイル名をリストにセット
	void SetFileName(const wxString &filename);
//	/// ファイル名をリストにセット
//	void SetFileName(const wxString &filename, UiDiskNameStrings &disknames);
	/// リストをクリア
	void ClearFileName();
	/// ファイルパスをリストにセット
	void SetFilePath(const wxString &filename);

	/// ディレクトリアイテムと一致するノードをさがす
	UiDiskListItem FindNodeByDirItem(const UiDiskListItem &node, int disk_number, DiskBasicDirItem *tag_item, int depth = 0);
	/// ディスク番号と一致するノードをさがす
	UiDiskListItem FindNodeByDiskNumber(const UiDiskListItem &node, int disk_number, int depth = 0);
	/// ディスク番号＆サイド番号と一致するノードをさがす
	UiDiskListItem FindNodeByDiskAndSideNumber(const UiDiskListItem &node, int disk_number, int side_number, int depth = 0);

	/// ルートディレクトリを更新
	void RefreshRootDirectoryNode(DiskImageDisk *disk, int side_number);
	/// ルートディレクトリを更新
	void RefreshRootDirectoryNode(DiskImageDisk *disk, const UiDiskListItem &node);
	/// ディレクトリを更新
	void RefreshDirectoryNode(DiskImageDisk *disk, DiskBasicDirItem *dir_item);
	/// 全てのディレクトリを更新
	void RefreshAllDirectoryNodes(DiskImageDisk *disk, int side_number, DiskBasicDirItem *dir_item);
	/// ディレクトリを選択
	void SelectDirectoryNode(DiskImageDisk *disk, DiskBasicDirItem *dir_item);
	/// ディレクトリノードを削除
	void DeleteDirectoryNode(DiskImageDisk *disk, DiskBasicDirItem *dir_item);
	/// ディレクトリノードを一括削除
	void DeleteDirectoryNodes(DiskImageDisk *disk, DiskBasicDirItems &dir_items);
//	/// ディレクトリを追加
//	UiDiskListItem AppendDirectory(DiskBasicDirItem *dir_item);
	/// ディレクトリを追加
	UiDiskListItem AppendDirectory(const UiDiskListItem &parent, DiskBasicDirItem *dir_item);
	/// ツリービューのディレクトリ名を再設定（キャラクターコードを変更した時）
	void RefreshDirectoryName(DiskImageDisk *disk);
	/// ツリービューのディレクトリ名を再設定
	void RefreshDirectoryName(const UiDiskListItem &node, int disk_number, int depth = 0);

	/// ディスクの初期化
	bool InitializeDisk();
	/// ディスクを論理フォーマット
	bool FormatDisk();

	/// ディスクをファイルに保存ダイアログ
	void ShowSaveDiskDialog();
	/// ディスクを置換
	void ReplaceDisk();
	/// ディスクをファイルから削除
	bool DeleteDisk();
	/// ディスク名を変更
	void RenameDisk();
	/// ディスク情報ダイアログ
	void ShowDiskAttr();
	/// 選択位置のディスク名をセット
	void SetDiskName(const wxString &val);
	/// キャラクターコード変更
	void ChangeCharCode(const wxString &name);
	/// フォントをセット
	void SetListFont(const wxFont &font);

	/// ファイルリストをドラッグ
	bool DragDataSource(const UiDiskListItem &sel_node);
	/// 指定したフォルダにエクスポート
	int  ExportDataFiles(const UiDiskListItems &selected_items, const wxString &data_dir, const wxString &attr_dir, const wxString &start_msg, const wxString &end_msg, wxFileDataObject *file_object = NULL);
	/// ファイルをドロップ
	bool DropDataFiles(wxWindow *base, int x, int y, const wxArrayString &paths, bool dir_included);
	/// クリップボードへコピー
	bool CopyToClipboard();
	/// クリップボードからペースト
	bool PasteFromClipboard();
	//@}

	/// @name properties
	//@{
	/// 選択しているディスクイメージのディスク番号を返す
	int  GetSelectedDiskNumber();
	/// 選択しているディスクイメージのサイド番号を返す
	int  GetSelectedDiskSide();
	/// 選択しているディスクイメージのディスク番号とサイド番号を返す
	void GetSelectedDisk(int &disk_number, int &side_number);
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
		IDM_DELETE_DIRECTORY,
		IDM_INITIALIZE_DISK,
		IDM_FORMAT_DISK,
		IDM_COPY_FILE,
		IDM_PASTE_FILE,
		IDM_PROPERTY_DISK,
		IDM_PROPERTY_BASIC,
	};

	wxDECLARE_EVENT_TABLE();
};

#endif /* UIDISKLIST_H */

