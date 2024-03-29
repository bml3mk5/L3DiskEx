/// @file uidisklist.cpp
///
/// @brief ディスクリスト
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uidisklist.h"
#include <wx/imaglist.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include "mymenu.h"
#include "../main.h"
#include "uimainframe.h"
#include "uidiskattr.h"
#include "uifilelist.h"
#include "diskparambox.h"
#include "basicselbox.h"
#include "../basicfmt/basicparam.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basicdiritem.h"
#include "../utils.h"


// icon
extern const char *fd_5inch_16_1_xpm[];
extern const char *fd_5inch_16_2_xpm[];
extern const char *foldericon_open_xpm[];
extern const char *foldericon_close_xpm[];


const char **icons_for_tree[] = {
	fd_5inch_16_1_xpm,
	fd_5inch_16_2_xpm,
	foldericon_close_xpm,
	foldericon_open_xpm,
	NULL
};
enum en_icons_for_tree {
	ICON_FOR_TREE_NONE = -1,
	ICON_FOR_TREE_SINGLE = 0,
	ICON_FOR_TREE_ROOT,
	ICON_FOR_TREE_CLOSE,
	ICON_FOR_TREE_OPEN,
};

//////////////////////////////////////////////////////////////////////
//
//
//
/// ツリー構造を保存する
/// @param [in] n_disknum   : ディスク番号(0から) ルートは-1
/// @param [in] n_typenum   : ディスク内にAB面があるとき-2、ディレクトリがあるとき-4、その他-1
/// @param [in] n_sidenum   : ディスク内にAB面があるときサイド番号、それ以外は-1
/// @param [in] n_pos       : ディレクトリ位置
/// @param [in] n_editable  : ディスク名の編集が可能か
/// @param [in] n_ditem     : DISK BASIC ディレクトリアイテム
UiDiskPositionData::UiDiskPositionData(int n_disknum, int n_typenum, int n_sidenum, int n_pos, bool n_editable, DiskBasicDirItem *n_ditem)
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	: wxClientData()
#else
	: wxTreeItemData()
#endif
{
	disknum = n_disknum;
	typenum = n_typenum;
	sidenum = n_sidenum;
	pos = n_pos;
	editable = n_editable;
	ditem = n_ditem;
	shown = false;
}
UiDiskPositionData::~UiDiskPositionData()
{
}

//////////////////////////////////////////////////////////////////////

#ifndef USE_TREE_CTRL_ON_DISK_LIST
//
//
//
UiDiskTreeStoreModel::UiDiskTreeStoreModel(UiDiskFrame *parentframe)
	: wxDataViewTreeStore()
{
	frame = parentframe;
}
/// ディスク名編集できるか
bool UiDiskTreeStoreModel::IsEnabled(const wxDataViewItem &item, unsigned int col) const
{
#if defined(__WXOSX__)
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(item);
	if (!cd) return false;
	return cd->GetEditable();
#else
	return true;
#endif
}
/// ディスク名を変更した
bool UiDiskTreeStoreModel::SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
{
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(item);
	if (!cd || cd->GetDiskNumber() < 0 || cd->GetEditable() != true) return false;

	wxDataViewIconText data;
	data << variant;
	if (frame->GetDiskImage().SetDiskName(cd->GetDiskNumber(), data.GetText())) {
		SetItemText(item, frame->GetDiskImage().GetDiskName(cd->GetDiskNumber()));
	}
	return true;
}
#endif

//////////////////////////////////////////////////////////////////////
//
// ツリーコントロール
//
UiDiskTreeCtrl::UiDiskTreeCtrl(wxWindow *parentwindow, wxWindowID id)
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	: MyCDTreeCtrl(parentwindow, id)
#else
	: MyCTreeCtrl(parentwindow, id)
#endif
{
}

//////////////////////////////////////////////////////////////////////
//
// Left Panel
//
// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskList, UiDiskTreeCtrl)
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, UiDiskList::OnContextMenu)

	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, UiDiskList::OnSelectionChanged)

	EVT_DATAVIEW_ITEM_EXPANDING(wxID_ANY, UiDiskList::OnItemExpanding)

	EVT_DATAVIEW_ITEM_BEGIN_DRAG(wxID_ANY, UiDiskList::OnBeginDrag)

// OSXでは発生しない！
	EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY, UiDiskList::OnStartEditing)
//	EVT_DATAVIEW_ITEM_EDITING_DONE(wxID_ANY, UiDiskList::OnEditingDone)
#else
	EVT_TREE_ITEM_MENU(wxID_ANY, UiDiskList::OnContextMenu)
	EVT_TREE_SEL_CHANGED(wxID_ANY, UiDiskList::OnSelectionChanged)
	EVT_TREE_ITEM_EXPANDING(wxID_ANY, UiDiskList::OnItemExpanding)
	EVT_TREE_BEGIN_LABEL_EDIT(wxID_ANY, UiDiskList::OnStartEditing)
	EVT_TREE_END_LABEL_EDIT(wxID_ANY, UiDiskList::OnEditingDone)
	EVT_TREE_BEGIN_DRAG(wxID_ANY, UiDiskList::OnBeginDrag)
#endif

	EVT_MENU(IDM_SAVE_DISK, UiDiskList::OnSaveDisk)
	EVT_MENU(IDM_ADD_DISK_NEW, UiDiskList::OnAddNewDisk)
	EVT_MENU(IDM_ADD_DISK_FROM_FILE, UiDiskList::OnAddDiskFromFile)
	EVT_MENU(IDM_REPLACE_DISK_FROM_FILE, UiDiskList::OnReplaceDisk)
	EVT_MENU(IDM_DELETE_DISK_FROM_FILE, UiDiskList::OnDeleteDisk)
	EVT_MENU(IDM_RENAME_DISK, UiDiskList::OnRenameDisk)

	EVT_MENU(IDM_DELETE_DIRECTORY, UiDiskList::OnDeleteDirectory)

	EVT_MENU(IDM_INITIALIZE_DISK, UiDiskList::OnInitializeDisk)
	EVT_MENU(IDM_FORMAT_DISK, UiDiskList::OnFormatDisk)

	EVT_MENU(IDM_COPY_FILE, UiDiskList::OnCopyFile)
	EVT_MENU(IDM_PASTE_FILE, UiDiskList::OnPasteFile)

	EVT_MENU(IDM_PROPERTY_DISK, UiDiskList::OnPropertyDisk)
	EVT_MENU(IDM_PROPERTY_BASIC, UiDiskList::OnPropertyBasic)
wxEND_EVENT_TABLE()

UiDiskList::UiDiskList(UiDiskFrame *parentframe, wxWindow *parentwindow)
       : UiDiskTreeCtrl(parentwindow, wxID_ANY)
{
	parent   = parentwindow;
	frame    = parentframe;

	m_initialized = false;

	AssignTreeIcons( icons_for_tree );

#ifndef USE_TREE_CTRL_ON_DISK_LIST
	UiDiskTreeStoreModel *model = new UiDiskTreeStoreModel(parentframe);
	AssociateModel(model);
	model->DecRef();
#endif

	wxFont font;
	frame->GetDefaultListFont(font);
	SetFont(font);

	// fit size on parent window
    SetSize(parentwindow->GetClientSize());

	// popup menu
	MakePopupMenu();

	// key
	Bind(wxEVT_CHAR, &UiDiskList::OnChar, this);

	ClearFileName();

	m_initialized = true;
}

UiDiskList::~UiDiskList()
{
	delete menuPopup;
}

/// コピー選択
/// @param[in] event コマンドイベント
void UiDiskList::OnCopyFile(wxCommandEvent& WXUNUSED(event))
{
	CopyToClipboard();
}

/// ペースト選択
/// @param[in] event コマンドイベント
void UiDiskList::OnPasteFile(wxCommandEvent& WXUNUSED(event))
{
	PasteFromClipboard();
}

/// ツリー上でドラッグ開始
/// @param[in] event リストイベント
void UiDiskList::OnBeginDrag(UiDiskListEvent& event)
{
	// ドラッグ
	UiDiskListItem item = event.GetItem();
	DragDataSource(item);
}

/// 右クリック選択
/// @param[in] event リストイベント
void UiDiskList::OnContextMenu(UiDiskListEvent& event)
{
	UiDiskListItem item = event.GetItem();
	SelectTreeNode(item);
	ShowPopupMenu();
}

/// ツリーアイテム選択
/// @param[in] event リストイベント
void UiDiskList::OnSelectionChanged(UiDiskListEvent& event)
{
	UiDiskListItem item = event.GetItem();
	ChangeSelection(item);
}

/// アイテム展開
/// @param[in] event リストイベント
void UiDiskList::OnItemExpanding(UiDiskListEvent& event)
{
	UiDiskListItem item = event.GetItem();
	ExpandItemNode(item);
}

/// アイテム編集開始
/// @note wxDataViewTreeCtrl OSXでは発生しない
/// @param[in] event リストイベント
void UiDiskList::OnStartEditing(UiDiskListEvent& event)
{
	UiDiskListItem item = event.GetItem();
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(item);
	if (!cd || !cd->GetEditable()) {
		event.Veto();
	}
}

/// アイテム編集終了
/// @param[in] event リストイベント
void UiDiskList::OnEditingDone(UiDiskListEvent& event)
{
#ifdef USE_TREE_CTRL_ON_DISK_LIST
	if (event.IsEditCancelled()) return;
	UiDiskListItem item = event.GetItem();
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(item);
	if (!cd || cd->GetDiskNumber() < 0 || cd->GetEditable() != true) return;

	if (frame->GetDiskImage().SetDiskName(cd->GetDiskNumber(), event.GetLabel())) {
		SetItemText(item, frame->GetDiskImage().GetDiskName(cd->GetDiskNumber()));
	}
	event.Veto();
#endif
}

/// ディスクを保存選択
/// @param[in] event コマンドイベント
void UiDiskList::OnSaveDisk(wxCommandEvent& WXUNUSED(event))
{
	ShowSaveDiskDialog();
}

/// ディスクを新規に追加選択
/// @param[in] event コマンドイベント
void UiDiskList::OnAddNewDisk(wxCommandEvent& WXUNUSED(event))
{
	frame->ShowAddNewDiskDialog();
}

/// ディスクをファイルから追加選択
/// @param[in] event コマンドイベント
void UiDiskList::OnAddDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	frame->ShowAddFileDialog();
}

/// ディスクイメージを置換選択
/// @param[in] event コマンドイベント
void UiDiskList::OnReplaceDisk(wxCommandEvent& WXUNUSED(event))
{
	ReplaceDisk();
}

/// ディスクを削除選択
/// @param[in] event コマンドイベント
void UiDiskList::OnDeleteDisk(wxCommandEvent& WXUNUSED(event))
{
	DeleteDisk();
}

/// ディスク名を変更選択
/// @param[in] event コマンドイベント
void UiDiskList::OnRenameDisk(wxCommandEvent& WXUNUSED(event))
{
	RenameDisk();
}

/// ディレクトリを削除選択
/// @param[in] event コマンドイベント
void UiDiskList::OnDeleteDirectory(wxCommandEvent& WXUNUSED(event))
{
	DeleteDirectory();
}

/// 初期化選択
/// @param[in] event コマンドイベント
void UiDiskList::OnInitializeDisk(wxCommandEvent& WXUNUSED(event))
{
	InitializeDisk();
}

/// フォーマット選択
/// @param[in] event コマンドイベント
void UiDiskList::OnFormatDisk(wxCommandEvent& WXUNUSED(event))
{
	frame->FormatDisk();
}

/// プロパティ選択
/// @param[in] event コマンドイベント
void UiDiskList::OnPropertyDisk(wxCommandEvent& WXUNUSED(event))
{
	ShowDiskAttr();
}

/// BASIC情報選択
/// @param[in] event コマンドイベント
void UiDiskList::OnPropertyBasic(wxCommandEvent& WXUNUSED(event))
{
	frame->ShowBasicAttr();
}

/// キー入力
/// @param[in] event キーイベント
void UiDiskList::OnChar(wxKeyEvent& event)
{
	switch(event.GetKeyCode()) {
	case WXK_RETURN:
		ShowDiskAttr();
		break;
	case WXK_DELETE:
		SelectDeleting();
		break;
	default:
		event.Skip();
		break;
	}
}

/// ポップアップメニュー作成
void UiDiskList::MakePopupMenu()
{
	menuPopup = new MyMenu;
	MyMenu *sm = new MyMenu();
	menuPopup->Append( IDM_SAVE_DISK, _("&Save Disk...") );
	menuPopup->AppendSeparator();
		sm->Append( IDM_ADD_DISK_NEW, _("&New Disk...") );
		sm->Append( IDM_ADD_DISK_FROM_FILE, _("From &File...") );
	menuPopup->AppendSubMenu(sm, _("&Add Disk") );
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_REPLACE_DISK_FROM_FILE, _("R&eplace Disk Data...") );
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_DELETE_DISK_FROM_FILE, _("&Delete Disk...") );
	menuPopup->Append(IDM_RENAME_DISK, _("&Rename Disk") );
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_DELETE_DIRECTORY, _("De&lete Directory...") );
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_INITIALIZE_DISK, _("I&nitialize..."));
	menuPopup->Append(IDM_FORMAT_DISK, _("F&ormat For BASIC..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_COPY_FILE, _("&Copy"));
	menuPopup->Append(IDM_PASTE_FILE, _("&Paste..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY_DISK, _("Disk &Information..."));
	menuPopup->Append(IDM_PROPERTY_BASIC, _("&BASIC Information..."));
}

/// ポップアップメニュー表示
void UiDiskList::ShowPopupMenu()
{
	if (!menuPopup) return;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());

	bool opened = (cd != NULL);
	menuPopup->Enable(IDM_ADD_DISK_NEW, opened);
	menuPopup->Enable(IDM_ADD_DISK_FROM_FILE, opened);

	opened = (opened && (m_selected_disk != NULL));
	menuPopup->Enable(IDM_REPLACE_DISK_FROM_FILE, opened);
	menuPopup->Enable(IDM_SAVE_DISK, opened);
	menuPopup->Enable(IDM_DELETE_DISK_FROM_FILE, opened);
	menuPopup->Enable(IDM_RENAME_DISK, opened);
	menuPopup->Enable(IDM_INITIALIZE_DISK, opened);
	menuPopup->Enable(IDM_PROPERTY_DISK, opened);

//	UiDiskFileList *list = frame->GetFileListPanel();
	opened = (opened && m_disk_selecting);
	menuPopup->Enable(IDM_FORMAT_DISK, opened && frame->IsFormattableDisk());
	menuPopup->Enable(IDM_PROPERTY_BASIC, opened && frame->CanUseBasicDisk());

	opened = (opened && cd->GetTypeNumber() == CD_TYPENUM_NODE_DIR);
	menuPopup->Enable(IDM_DELETE_DIRECTORY, opened);

	PopupMenu(menuPopup);
}

/// ディレクトリ/ディスク削除選択
void UiDiskList::SelectDeleting()
{
	UiDiskListItem node = GetSelection();
	if (!UiDiskListItem_IsOk(node)) return;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);
	if (!cd) return;

	if (cd->GetTypeNumber() == CD_TYPENUM_NODE_DIR) {
		// ディレクトリ
		DiskBasicDirItem *dir_item = cd->GetDiskBasicDirItem();
		if (dir_item) frame->DeleteDirectory(dir_item->GetBasic(), dir_item);
	} else {
		// ディスク
		DeleteDisk();
	}
}

/// フォーカスのあるディレクトリを削除
/// @return true:OK false:Error
bool UiDiskList::DeleteDirectory()
{
	UiDiskListItem node = GetSelection();
	if (!UiDiskListItem_IsOk(node)) return false;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);
	if (!cd) return false;

	if (cd->GetTypeNumber() == CD_TYPENUM_NODE_DIR) {
		// ディレクトリ
		DiskBasicDirItem *dir_item = cd->GetDiskBasicDirItem();
		if (dir_item) {
			return frame->DeleteDirectory(dir_item->GetBasic(), dir_item);
		} else {
			return false;
		}
	} else {
		return false;
	}
}

/// 再選択
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void UiDiskList::ReSelect(const DiskBasicParam *newparam)
{
	UiDiskListItem item = GetSelection();
	ChangeSelection(item, newparam);
}

/// ツリーを選択
/// @param [in] node     選択したノード
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void UiDiskList::ChangeSelection(UiDiskListItem &node, const DiskBasicParam *newparam)
{
	SetDataOnItemNode(node, NODE_SELECTED, newparam);
}

/// ディスクを指定して選択状態にする
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号 両面なら-1
void UiDiskList::ChangeSelection(int disk_number, int side_number)
{
	UiDiskListItem match_node = FindNodeByDiskAndSideNumber(m_root_node, disk_number, side_number);
	if (UiDiskListItem_IsOk(match_node)) {
		SelectTreeNode(match_node);
		ChangeSelection(match_node);
	}
}

/// ツリーを展開
/// @param [in] node     選択したノード
void UiDiskList::ExpandItemNode(UiDiskListItem &node)
{
	if (!UiDiskListItem_IsOk(node)) return;

	UiDiskListItem sel_node = GetSelection();

	if (sel_node.GetID() != node.GetID()) {
		SetDataOnItemNode(node, NODE_EXPANDED);
	}
}

/// 指定ノードにデータを設定する
/// @param [in] node     選択したノード
/// @param [in] flag     NODE_SELECTED / NODE_EXPANDED
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void UiDiskList::SetDataOnItemNode(const UiDiskListItem &node, SetDataOnItemNodeFlags flag, const DiskBasicParam *newparam)
{
	if (!m_initialized) return;

	if (!UiDiskListItem_IsOk(node)) {
		// rootアイテムを選択したらファイル一覧をクリア
		m_selected_disk = NULL;
		// 全パネルのデータをクリアする
		frame->ClearAllData();
		return;
	}

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);

	if (flag == NODE_SELECTED) m_disk_selecting = false;

	DiskImage *image = &frame->GetDiskImage();

	if (cd == NULL || (TreeNodeHasChildren(node) && cd->GetDiskNumber() < 0)) {
		// rootアイテムを選択したらファイル一覧をクリア
		m_selected_disk = NULL;
		// 全パネルのデータをクリアする
		frame->ClearAllData();
		return;
	}

	DiskImageDisk *disk = image->GetDisk(cd->GetDiskNumber());
	if (!disk) {
		return;
	}

	// ディスク属性をセット
	if (flag == NODE_SELECTED) {
		m_selected_disk = disk;
		frame->SetDiskAttrData(disk);
	}

	int typenum = cd->GetTypeNumber();
	int sidenum = cd->GetSideNumber();
	// サイドA,Bがある場合
	if ((typenum == CD_TYPENUM_NODE_AB || typenum == CD_TYPENUM_NODE_BOTH) && sidenum < 0) {
		// RAWパネルだけデータをセットする
		frame->ClearAllAndSetRawData(disk, -2);
		if (flag == NODE_SELECTED) Expand(node);
		return;
	}

	// ディスク１枚 or ディレクトリを選択
	if (flag == NODE_SELECTED) m_disk_selecting = true;

	// サブディレクトリの場合
	if (typenum == CD_TYPENUM_NODE_DIR) {
		frame->AttachDiskBasicOnFileList(disk, sidenum);
		DiskBasicDirItem *dir_item = cd->GetDiskBasicDirItem();
		if (dir_item) {
			if (flag == NODE_SELECTED) {
				frame->ChangeDirectory(dir_item->GetBasic(), dir_item, true);
			} else {
				frame->AssignDirectory(dir_item->GetBasic(), dir_item);
			}
		}
		return;
	}

	// 右パネルにファイル名一覧を設定
	if (ParseDiskBasic(node, cd, disk, sidenum, newparam)) {
		frame->SetDataOnDisk(disk, sidenum, flag == NODE_SELECTED);
		RefreshRootDirectoryNode(disk, node);
	}
}

/// DISK BASICをアタッチ＆解析
/// @param [in] node       選択したノード
/// @param [in] cd         ノードの位置情報
/// @param [in] newdisk    ディスクイメージ
/// @param [in] newsidenum サイド番号 片面選択時0/1 両面なら-1
/// @param [in] newparam   BASICパラメータ 通常NULL BASICを変更した際に設定する
/// @return 片面のみ使用するBASICの場合false -> さらに下位ノードで解析
bool UiDiskList::ParseDiskBasic(const UiDiskListItem &node, UiDiskPositionData *cd, DiskImageDisk *newdisk, int newsidenum, const DiskBasicParam *newparam)
{
	DiskBasic *newbasic = newdisk->GetDiskBasic(newsidenum);

	newbasic->SetCharCode(frame->GetCharCode());

	// BASICモードのときはディスクを解析
	if (frame->GetSelectedMode() == 0) {
		// トラックが存在するか
		bool valid = false;
		bool found = newdisk->ExistTrack(newsidenum);
		if (found) {
			// ディスクをDISK BASICとして解析
			valid = (newbasic->ParseBasic(newdisk, newsidenum, newparam, false) == 0);
		} else {
			// トラックがない
			newbasic->GetErrinfo().SetWarn(DiskBasicError::ERR_NO_FOUND_TRACK);
		}

		// ルートディレクトリをセット
		if (valid) {
			valid = newbasic->AssignRootDirectory();
		}

		// エラーメッセージ
		if (!valid) {
			newbasic->ShowErrorMessage();
		}
	}

	// 片面のみ使用するOSで表裏面それぞれの使用が可能な場合
	// ディスクがリバーシブルな場合
	if (cd->GetTypeNumber() == CD_TYPENUM_NODE) {
		if (newdisk->IsReversible() || newbasic->CanMountEachSides()) {
			DiskBasics *basics = newdisk->GetDiskBasics();
			if (basics->Count() <= 1) {
				basics->Add(new DiskBasic);
			}
			// サイド番号を設定
//			for(size_t i=0; i<2; i++) {
//				DiskBasic* basic = basics->Item(i);
//				basic->SetSelectedSide((int)i);
//			}
			int type;
			if (newdisk->IsReversible()) {
				type = CD_TYPENUM_NODE_AB;
			} else {
				type = CD_TYPENUM_NODE_BOTH;
			}
			if (newsidenum < 0) {
				cd->SetTypeNumber(type);
			}
			// 現在のツリーの下にサイド分のツリーノードを作成
			for(int i=0; i<2; i++) {
				wxString caption;
				if (newdisk->IsReversible()) {
					type = CD_TYPENUM_NODE_AB;
				} else {
					type = CD_TYPENUM_NODE_BOTH;
				}
				SubCaption(type, i, caption);
				AddTreeContainer(node, caption, ICON_FOR_TREE_SINGLE, ICON_FOR_TREE_NONE,
					new UiDiskPositionData((int)cd->GetDiskNumber(), type, i, i, false));
			}
			Expand(node);
			cd->Shown(false);
			return false;
		}
	}
	return true;
}

/// サブキャプション
/// 表裏を入れ替えるようなディスクの場合
/// @param[in] type        タイプ
/// @param[in] side_number サイド番号
/// @param[out] caption    キャプション
void UiDiskList::SubCaption(int type, int side_number, wxString &caption) const
{
	if (side_number < 0) return;

	caption = Utils::GetSideStr(side_number, type != CD_TYPENUM_NODE_AB);
}

/// 選択しているディスクの子供を削除
void UiDiskList::DeleteChildrenOnSelectedDisk()
{
	UiDiskListItem node = GetSelection();
	if (!UiDiskListItem_IsOk(node)) return;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);
	if (!cd) return;

	int disk_number = cd->GetDiskNumber();
	node = FindNodeByDiskNumber(m_root_node, disk_number); 
	if (!UiDiskListItem_IsOk(node)) return;

	cd = (UiDiskPositionData *)GetItemData(node);

	Collapse(node);
	DeleteChildren(node);
	cd->SetTypeNumber(CD_TYPENUM_NODE);
	cd->Shown(false);
	SelectTreeNode(node);
}

/// 選択しているディスクのルートを初期化＆再選択
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void UiDiskList::RefreshSelectedDisk(const DiskBasicParam *newparam)
{
	// 子供を削除
	DeleteChildrenOnSelectedDisk();
	// 再選択
	ReSelect(newparam);
}

/// 選択しているサイドを再選択
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void UiDiskList::RefreshSelectedSide(const DiskBasicParam *newparam)
{
	if (newparam && !newparam->CanMountEachSides()) {
		// 両面の場合
		// 子供を削除
		DeleteChildrenOnSelectedDisk();
	}
	// 再選択
	ReSelect(newparam);
}

/// ファイル名をリストにセット
void UiDiskList::SetFileName()
{
	SetFileName(frame->GetFileName());
}

/// ファイル名をリストにセット
/// @param [in] filename ファイル名
void UiDiskList::SetFileName(const wxString &filename)
{
	DiskImageDisks *disks = frame->GetDiskImage().GetDisks();
	if (!disks) return;

	DeleteAllItems();

	UiDiskListItem node = AddRootTreeNode(filename, ICON_FOR_TREE_ROOT, ICON_FOR_TREE_NONE
		, new UiDiskPositionData(CD_DISKNUM_ROOT, CD_TYPENUM_NODE, -1, 0, false));
	m_root_node = node;

	for(size_t i=0; i<disks->Count(); i++) {
		DiskImageDisk *diskn = disks->Item(i);
		// ディスク１つ
		AddTreeContainer(node, diskn->GetName(), ICON_FOR_TREE_SINGLE, ICON_FOR_TREE_NONE
			, new UiDiskPositionData((int)i, CD_TYPENUM_NODE, CD_TYPENUM_NODE, CD_TYPENUM_NODE, true));
	}

	Expand(node);
	SelectTreeNode(node);
	frame->ClearDiskAttrData();
}

/// リストをクリア
void UiDiskList::ClearFileName()
{
	DeleteAllItems();

	UiDiskListItem node = AddRootTreeNode( _("(none)") );
	m_root_node = node;

	Expand(node);

	m_selected_disk = NULL;
	m_disk_selecting = false;
}

/// ファイルパスをリストにセット
/// @param [in] filename ファイル名
void UiDiskList::SetFilePath(const wxString &filename)
{
	if (!filename.IsEmpty()) {
		SetItemText(m_root_node, filename);
	} else {
		SetItemText(m_root_node, _("(none)"));
	}
}

/// ディレクトリアイテムと一致するノードをさがす
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param [in] node        ノード
/// @param [in] disk_number ディスク番号
/// @param [in] tag_item    対象ディレクトリアイテム
/// @param [in] depth       深さ
/// @return 一致したノード
UiDiskListItem UiDiskList::FindNodeByDirItem(const UiDiskListItem &node, int disk_number, DiskBasicDirItem *tag_item, int depth)
{
	UiDiskListItem match_node;

	if (depth < 100 && TreeNodeHasChildren(node)) {
		UiDiskTreeIdVal cookie;
		UiDiskListItem child_node = GetFirstChild(node, cookie);
		while(UiDiskListItem_IsOk(child_node) && !UiDiskListItem_IsOk(match_node)) {
			UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(child_node);
			if (cd) {
				// ディスク番号を比較
				if (cd->GetDiskNumber() == disk_number) {
					// ディレクトリアイテムのポインタを比較
					if (cd->GetDiskBasicDirItem() == tag_item) {
						match_node = child_node;
						break;
					} else if (TreeNodeHasChildren(child_node)) {
						// 再帰的に探す
						match_node = FindNodeByDirItem(child_node, disk_number, tag_item, depth + 1);
					}
				}
			}
			child_node = GetNextChild(node, cookie);
		}
	}
	return match_node;
}

/// ディスク番号と一致するノードをさがす
/// @param [in] node        ノード
/// @param [in] disk_number ディスク番号
/// @param [in] depth       深さ
/// @return 一致したノード
UiDiskListItem UiDiskList::FindNodeByDiskNumber(const UiDiskListItem &node, int disk_number, int depth)
{
	UiDiskListItem match_node;

	if (depth < 100 && TreeNodeHasChildren(node)) {
		UiDiskTreeIdVal cookie;
		UiDiskListItem child_node = GetFirstChild(node, cookie);
		while(UiDiskListItem_IsOk(child_node) && !UiDiskListItem_IsOk(match_node)) {
			UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(child_node);
			if (cd) {
				// ディスク番号を比較
				if (cd->GetDiskNumber() == disk_number) {
					match_node = child_node;
					break;
				}
			}
			child_node = GetNextChild(node, cookie);
		}
	}
	return match_node;
}

/// ディスク番号と一致するノードをさがす
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param [in] node        ノード
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号 両面なら-1
/// @param [in] depth       深さ
/// @return 一致したノード
UiDiskListItem UiDiskList::FindNodeByDiskAndSideNumber(const UiDiskListItem &node, int disk_number, int side_number, int depth)
{
	UiDiskListItem match_node;

	if (depth < 100 && TreeNodeHasChildren(node)) {
		UiDiskTreeIdVal cookie;
		UiDiskListItem child_node = GetFirstChild(node, cookie);
		while(UiDiskListItem_IsOk(child_node) && !UiDiskListItem_IsOk(match_node)) {
			UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(child_node);
			if (cd) {
				// ディスク番号を比較
				if (cd->GetDiskNumber() == disk_number) {
					if (cd->GetSideNumber() == side_number) {
						match_node = child_node;
						break;
					} else if (TreeNodeHasChildren(child_node)) {
						// 再帰的に探す
						match_node = FindNodeByDiskAndSideNumber(child_node, disk_number, side_number, depth + 1);
					}
				}
			}
			child_node = GetNextChild(node, cookie);
		}
	}
	return match_node;
}

/// ルートディレクトリを更新
/// @param [in] disk        ディスク
/// @param [in] side_number サイド番号
void UiDiskList::RefreshRootDirectoryNode(DiskImageDisk *disk, int side_number)
{
	UiDiskListItem node = FindNodeByDiskAndSideNumber(m_root_node, disk->GetNumber(), side_number);
	if (!UiDiskListItem_IsOk(node)) return;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);
	DiskBasic *basic = disk->GetDiskBasic(side_number);
	if (!basic) return;
	DiskBasicDirItem *root_item = basic->GetRootDirectory();
	if (!root_item) return;
	RefreshDirectorySub(disk, node, cd, root_item);
}

/// ルートディレクトリを更新
/// @param [in] disk      ディスク
/// @param [in] node      ノード
void UiDiskList::RefreshRootDirectoryNode(DiskImageDisk *disk, const UiDiskListItem &node)
{
	if (!UiDiskListItem_IsOk(node)) return;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);
	DiskBasic *basic = disk->GetDiskBasic(cd->GetSideNumber());
	if (!basic) return;
	DiskBasicDirItem *root_item = basic->GetRootDirectory();
	cd->SetDiskBasicDirItem(root_item);
	if (!root_item) return;
	RefreshDirectorySub(disk, node, cd, root_item);
}

/// ディレクトリノードを更新
/// @param [in] disk     ディスク
/// @param [in] dir_item 対象ディレクトリアイテム
void UiDiskList::RefreshDirectoryNode(DiskImageDisk *disk, DiskBasicDirItem *dir_item)
{
	UiDiskListItem node = FindNodeByDirItem(m_root_node, disk->GetNumber(), dir_item);
	if (!UiDiskListItem_IsOk(node)) return;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);

	RefreshDirectorySub(disk, node, cd, dir_item);
}

/// 全てのディレクトリを更新
/// @param [in] disk        ディスク
/// @param [in] side_number サイド番号
/// @param [in] dir_item    対象ディレクトリアイテム
void UiDiskList::RefreshAllDirectoryNodes(DiskImageDisk *disk, int side_number, DiskBasicDirItem *dir_item)
{
	if (!disk) return;

	if (!dir_item->GetParent()) {
		// ルートツリーを更新
		RefreshRootDirectoryNode(disk, side_number);
	} else {
		// ディレクトリツリーを更新
		RefreshDirectoryNode(disk, dir_item);
	}
}

/// ディレクトリノードを選択
/// @param [in] disk     ディスク
/// @param [in] dir_item 対象ディレクトリアイテム
void UiDiskList::SelectDirectoryNode(DiskImageDisk *disk, DiskBasicDirItem *dir_item)
{
	UiDiskListItem node = FindNodeByDirItem(m_root_node, disk->GetNumber(), dir_item);
	if (!UiDiskListItem_IsOk(node)) return;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);

	RefreshDirectorySub(disk, node, cd, dir_item);

	SelectTreeNode(node);
}

/// ディレクトリノードを削除
/// @param [in] disk      ディスク
/// @param [in] dir_item  対象ディレクトリアイテム
void UiDiskList::DeleteDirectoryNode(DiskImageDisk *disk, DiskBasicDirItem *dir_item)
{
	UiDiskListItem node = FindNodeByDirItem(m_root_node, disk->GetNumber(), dir_item);
	if (!UiDiskListItem_IsOk(node)) return;

	UiDiskListItem parent_node = GetParentTreeNode(node);

	DeleteTreeNode(node);

	if (!UiDiskListItem_IsOk(parent_node)) return;

	SelectTreeNode(parent_node);
	ChangeSelection(parent_node);
}

/// ディレクトリノードを一括削除
/// @param [in] disk      ディスク
/// @param [in] dir_items 対象ディレクトリアイテム
void UiDiskList::DeleteDirectoryNodes(DiskImageDisk *disk, DiskBasicDirItems &dir_items)
{
	for(size_t i=0; i<dir_items.Count(); i++) {
		DiskBasicDirItem *dir_item = dir_items.Item(i);

		UiDiskListItem node = FindNodeByDirItem(m_root_node, disk->GetNumber(), dir_item);
		if (!UiDiskListItem_IsOk(node)) continue;

		DeleteTreeNode(node);
	}
}

/// ディレクトリを更新
/// @param [in] disk      ディスク
/// @param [in] node      ノード
/// @param [in] cd        ノード情報
/// @param [in] dir_item  対象ディレクトリアイテム
void UiDiskList::RefreshDirectorySub(DiskImageDisk *disk, const UiDiskListItem &node, UiDiskPositionData *cd, DiskBasicDirItem *dir_item)
{
	if (dir_item->IsValidDirectory() && cd->IsShown()) {
		return;
	}

	bool expanded = IsExpanded(node);

	// サブディレクトリを消す
	DeleteChildren(node);

	// サブディレクトリを消して選択位置が変わるとイベントが発生する
	// その結果、現ディレクトリが選択された場合は、選択時に既に
	// 更新されているのでここで終了する。
	if (dir_item->IsValidDirectory() && cd->IsShown()) {
		return;
	}
	
	// ディレクトリ一覧を設定
	DiskBasicDirItems *dir_items = dir_item->GetChildren();
	if (dir_items) {
		for(size_t i=0; i<dir_items->Count(); i++) {
			DiskBasicDirItem *ditem = dir_items->Item(i);
			if (!ditem->IsUsedAndVisible()) continue;

			if (ditem->IsDirectory()) {
				AppendDirectory(node, ditem);
			}
		}
	}

	dir_item->ValidDirectory(true);
	cd->Shown(true);

	if (expanded) Expand(node);
}

/// ディレクトリを追加
/// @param [in] parent    親ノード
/// @param [in] dir_item  対象ディレクトリアイテム
UiDiskListItem UiDiskList::AppendDirectory(const UiDiskListItem &parent, DiskBasicDirItem *dir_item)
{
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(parent);
	int disk_number = cd->GetDiskNumber();
	int side_number = cd->GetSideNumber();
	wxString name = dir_item->GetFileNameStr();
	// 対象外ディレクトリならスキップ "."や".."など
	if (!dir_item->IsVisibleOnTree()) return UiDiskListItem();

	int pos = GetTreeChildCount(parent);
	UiDiskListItem newitem = AddTreeContainer(parent, name, ICON_FOR_TREE_CLOSE, ICON_FOR_TREE_OPEN
		,new UiDiskPositionData(disk_number, CD_TYPENUM_NODE_DIR, side_number, pos, false, dir_item));

	return newitem;
}

/// ツリービューのディレクトリ名を再設定（キャラクターコードを変更した時）
/// @param [in] disk      ディスク
void UiDiskList::RefreshDirectoryName(DiskImageDisk *disk)
{
	if (!disk) return;

	RefreshDirectoryName(m_root_node, disk->GetNumber());
}

/// ツリービューのディレクトリ名を再設定
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param [in] node        ノード
/// @param [in] disk_number ディスク番号
/// @param [in] depth       深さ
void UiDiskList::RefreshDirectoryName(const UiDiskListItem &node, int disk_number, int depth)
{
	if (depth < 100 && TreeNodeHasChildren(node)) {
		UiDiskTreeIdVal cookie;
		UiDiskListItem child_node = GetFirstChild(node, cookie);
		while(UiDiskListItem_IsOk(child_node)) {
			UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(child_node);
			if (cd) {
				// ディスク番号を比較
				if (cd->GetDiskNumber() == disk_number) {
					if (cd->GetTypeNumber() == CD_TYPENUM_NODE_DIR) {
						// ディレクトリアイテムの名前を再設定
						DiskBasicDirItem *dir_item = cd->GetDiskBasicDirItem();
						if (dir_item) {
							SetItemText(child_node, dir_item->GetFileNameStr());
						}
					}
					// 再帰的に探す
					RefreshDirectoryName(child_node, disk_number, depth + 1);
				}
			}
			child_node = GetNextChild(node, cookie);
		}
	}
}

/// ディスクの初期化
/// @return true:OK false:Error
bool UiDiskList::InitializeDisk()
{
	if (!m_selected_disk) return false;
	if (m_selected_disk->IsWriteProtected()) {
		DiskResult err;
		err.SetError(DiskResult::ERR_WRITE_PROTECTED);
		err.Show();
		return false;
	}

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());

	int ans = wxYES;
	wxString diskname = wxT("'")+m_selected_disk->GetName()+wxT("'");
	int selected_side = cd->GetPosition();
	bool found = m_selected_disk->ExistTrack(selected_side);
	bool sts = false;
	if (found) {
		// トラックがある場合は、初期化
		if (selected_side >= 0) {
			diskname += Utils::GetSideStr(selected_side, cd->GetTypeNumber() != CD_TYPENUM_NODE_AB);
		}
		wxString msg = wxString::Format(_("All files and datas will delete on %s. Do you really want to initialize it?"), diskname);
		ans = wxMessageBox(msg, _("Initialize Disk"), wxYES_NO);

		if (ans == wxYES) {
			// ディスク初期化
			sts = m_selected_disk->Initialize(selected_side);
			// 右パネルをクリア
			frame->ClearRPanelData();
			// DISK BASICを改めて解析させる
			DiskBasics *basics = m_selected_disk->GetDiskBasics();
			if (basics) {
				basics->ClearParseAndAssign(selected_side);
			}
			// ディスク再選択
			RefreshSelectedDisk();

			frame->IncreaseUniqueNumber();
		}
	} else {
		// トラックが全くない場合は、ディスク作成
		if (selected_side >= 0) {
			// 選択したサイドだけ作り直す
			DiskParamBox dlg(this, wxID_ANY, frame->GetDiskImage(), DiskParamBox::REBUILD_TRACKS, 0, m_selected_disk, NULL, NULL, 0);
			int rc = dlg.ShowModal();
			if (rc == wxID_OK) {
				DiskParam param;
				dlg.GetParam(param);
				m_selected_disk->GetFile()->SetBasicTypeHint(dlg.GetCategory());
				sts = m_selected_disk->Rebuild(param, selected_side);

				// ファイル名一覧を更新
				SetFileName();

				frame->IncreaseUniqueNumber();
			}
		} else {
			// パラメータを選択するダイアログを表示
			DiskParamBox dlg(this, wxID_ANY, frame->GetDiskImage(), DiskParamBox::REBUILD_TRACKS, -1, m_selected_disk, NULL, NULL, DiskParamBox::SHOW_ALL);
			int rc = dlg.ShowModal();
			if (rc == wxID_OK) {
				DiskParam param;
				dlg.GetParam(param);
				m_selected_disk->SetName(dlg.GetDiskName());
				m_selected_disk->SetDensity(dlg.GetDensityValue());
				m_selected_disk->SetWriteProtect(dlg.IsWriteProtected());
				m_selected_disk->GetFile()->SetBasicTypeHint(dlg.GetCategory());
				sts = m_selected_disk->Rebuild(param, selected_side);

				// ファイル名一覧を更新
				SetFileName();

				frame->IncreaseUniqueNumber();
			}
		}
	}

	return sts;
}

/// ディスクの論理フォーマット
/// @return true:OK false:Error
bool UiDiskList::FormatDisk()
{
	if (!m_selected_disk) return false;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());
	int selected_side = cd->GetPosition();

	DiskBasic *current_basic = m_selected_disk->GetDiskBasic(selected_side);

	if (!current_basic->IsWritableIntoDisk()) {
		current_basic->ShowErrorMessage();
		return false;
	}

	// BASIC種類を選択
	BasicSelBox dlg(this, wxID_ANY, m_selected_disk, current_basic, BasicSelBox::SHOW_ATTR_CONTROLS);
	int ans = dlg.ShowModal();
	if (ans != wxID_OK) {
		return false;
	}

	// 選択したBASICでフォーマット可能か
	DiskBasic new_basic;
	new_basic.ParseBasic(m_selected_disk, selected_side, dlg.GetBasicParam(), true);
	if (!new_basic.IsFormattable()) {
		new_basic.ShowErrorMessage();
		return false;
	}

	// 最終確認
	ans = wxYES;
	wxString diskname = wxT("'")+m_selected_disk->GetName()+wxT("'");
	diskname += new_basic.GetSelectedSideStr();
	wxString msg = wxString::Format(_("All files and datas will delete on %s. Do you really want to format it?"), diskname);
	ans = wxMessageBox(msg, _("Format"), wxYES_NO);

	int sts = 0;
	if (ans == wxYES) {
		// ディスクを初期化
		DiskBasicIdentifiedData data(
			dlg.GetVolumeName(),
			dlg.GetVolumeNumber(),
			dlg.GetVolumeDate()
		);

		m_selected_disk->Initialize(current_basic->GetSelectedSide());
		m_selected_disk->SetModify();

		// DISK BASIC用にフォーマット
		current_basic->ClearParseAndAssign();
		if (current_basic->GetBasicTypeName() != new_basic.GetBasicTypeName()) {
			// 違うDISK BASICの場合はパラメータを設定
			m_selected_disk->GetFile()->SetBasicTypeHint(new_basic.GetBasicCategoryName());
			current_basic->ParseBasic(m_selected_disk, selected_side, dlg.GetBasicParam(), true);
		}

		sts = current_basic->FormatDisk(data);
		if (sts != 0) {
			current_basic->ShowErrorMessage();

		}
		frame->ClearFatAreaData();
		// ディスク再選択
		RefreshSelectedDisk();

		frame->IncreaseUniqueNumber();
	}
//	frame->UpdateMenuAndToolBarFileList(this);

	return (sts >= 0);
}

/// ディスクをファイルに保存ダイアログ
void UiDiskList::ShowSaveDiskDialog()
{
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());
	if (!cd) return;
	frame->ShowSaveDiskDialog(cd->GetDiskNumber(), cd->GetPosition(), cd->GetTypeNumber() != CD_TYPENUM_NODE_AB);
}

/// ディスクを置換
void UiDiskList::ReplaceDisk()
{
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());
	if (!cd) return;

	wxString caption;
	SubCaption(cd->GetTypeNumber(), cd->GetPosition(), caption);

	frame->ShowReplaceDiskDialog(cd->GetDiskNumber(), cd->GetPosition(), caption);
}

/// ディスクをファイルから削除
/// @return true:OK false:Error
bool UiDiskList::DeleteDisk()
{
	if (!m_selected_disk) return false;

	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());

	int ans = wxYES;
	wxString diskname = wxT("'")+m_selected_disk->GetName()+wxT("'");
	wxString msg = wxString::Format(_("%s will be deleted. Do you really want to delete it?"), diskname);
	ans = wxMessageBox(msg, _("Delete Disk"), wxYES_NO);

	bool sts = false;
	if (ans == wxYES) {
		sts = frame->GetDiskImage().Delete(cd->GetDiskNumber());

		// 画面を更新
		frame->UpdateDataOnWindow(false);

		// プロパティダイアログを閉じる
		frame->CloseAllFileAttr();
	}
	return sts;
}

/// ディスク名を変更
void UiDiskList::RenameDisk()
{
	UiDiskListItem node = SetSelectedItemAtDiskImage();
	if (!UiDiskListItem_IsOk(node)) return;
	EditTreeNode(node);
}

/// ディスク情報ダイアログ
void UiDiskList::ShowDiskAttr()
{
	if (!m_selected_disk) return;

	DiskParamBox dlg(this, wxID_ANY, frame->GetDiskImage(), DiskParamBox::SHOW_DISK_PARAM, -1, m_selected_disk, NULL, NULL, DiskParamBox::SHOW_DISKLABEL_ALL);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		DiskParam param;
		dlg.GetParam(param);
		m_selected_disk->SetName(dlg.GetDiskName());
		m_selected_disk->SetDensity(dlg.GetDensityValue());
		m_selected_disk->SetWriteProtect(dlg.IsWriteProtected());
		m_selected_disk->SetModify();
		// ディスク名をセット
		SetDiskName(m_selected_disk->GetName());
		// ディスク属性をセット
		frame->SetDiskAttrData(m_selected_disk);
	}
}

/// 選択位置のディスクイメージ
UiDiskListItem UiDiskList::SetSelectedItemAtDiskImage()
{
	UiDiskListItem invalid;
	if (!m_selected_disk) return invalid;
	UiDiskListItem node = GetSelection();
	if (!UiDiskListItem_IsOk(node)) return invalid;
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);
	if (!cd) return invalid;
	if (cd->GetPosition() >= 0) {
		// ディスク名は親アイテムになる
		node = GetParentTreeNode(node);
		if (!UiDiskListItem_IsOk(node)) return invalid;
		UiDiskPositionData *pcd = (UiDiskPositionData *)GetItemData(node);
		if (pcd->GetDiskNumber() != cd->GetDiskNumber()) return invalid;
	}
	return node;
}

/// 選択位置のディスク名をセット
/// @param[in] val ディスク名
void UiDiskList::SetDiskName(const wxString &val)
{
	UiDiskListItem item = SetSelectedItemAtDiskImage();
	if (!UiDiskListItem_IsOk(item)) return;
	SetItemText(item, val);
}

/// キャラクターコード変更
/// @param[in] name コード名
void UiDiskList::ChangeCharCode(const wxString &name)
{
	DiskImage *image = &frame->GetDiskImage();
	if (!image) return;
	DiskImageDisks *disks = image->GetDisks();
	if (!disks) return;
	for(size_t i=0; i<disks->Count(); i++) {
		RefreshDirectoryName(disks->Item(i));
	}
}

/// フォントをセット
/// @param[in] font フォント
void UiDiskList::SetListFont(const wxFont &font)
{
	SetFont(font);
	Refresh();
}

/// ドラッグする
bool UiDiskList::DragDataSource(const UiDiskListItem &sel_node)
{
	wxString tmp_dir_name;

#ifdef USE_DATA_OBJECT_COMPOSITE
	wxDataObjectComposite compo;

	wxFileDataObject *file_object = NULL;

	bool sts = true;
	if (sts) {
		file_object = new wxFileDataObject();
		sts = CreateFileObject(sel_node, tmp_dir_name, _("dragging..."), _("dragged."), *file_object);
	}
	if (!sts) {
		delete file_object;
		return false;
	}
	// ドラッグしたアイテムを覚えておく
	m_dragging_node = sel_node;

	// ファイルデータは外部用
	if (file_object) compo.Add(file_object);

#ifdef __WXMSW__
	wxDropSource dragSource(compo);
#else
	wxDropSource dragSource(compo, frame);
#endif
#else

	wxFileDataObject file_object;
	bool sts = true;
	if (sts) {
		sts = CreateFileObject(sel_node, tmp_dir_name, _("dragging..."), _("dragged."), file_object);
	}
	if (!sts) {
		return false;
	}
	// ドラッグしたアイテムを覚えておく
	m_dragging_node = sel_node;

#ifdef __WXMSW__
	wxDropSource dragSource(file_object);
#else
	wxDropSource dragSource(file_object, frame);
#endif
#endif

	dragSource.DoDragDrop();

	return true;
}

/// ファイルをテンポラリディレクトリにエクスポートしファイルリストを作成する（DnD, クリップボード用）
/// @param [in]     sel_node     選択したツリーアイテム
/// @param [in]     tmp_dir_name テンポラリフォルダ
/// @param [in]     start_msg    開始メッセージ
/// @param [in]     end_msg      終了メッセージ
/// @param [in,out] file_object ファイルオブジェクト
/// @return true:OK false:Error
bool UiDiskList::CreateFileObject(const UiDiskListItem &sel_node, wxString &tmp_dir_name, const wxString &start_msg, const wxString &end_msg, wxFileDataObject &file_object)
{
	UiDiskListItems selected_items;

	selected_items.Add(sel_node);

	wxString tmp_data_path, tmp_attr_path;
	if (!frame->CreateTemporaryFolder(tmp_dir_name, tmp_data_path, tmp_attr_path)) {
		return false;
	}

	ExportDataFiles(selected_items, tmp_data_path, tmp_attr_path, start_msg, end_msg, &file_object);

	return true;
}

/// 指定したフォルダにエクスポート
/// @param [in]     selected_items 選択したリスト
/// @param [in]     data_dir       データファイル出力先フォルダ
/// @param [in]     attr_dir       属性ファイル出力先フォルダ
/// @param [in]     start_msg      開始メッセージ
/// @param [in]     end_msg        終了メッセージ
/// @param [in,out] file_object    ファイルオブジェクト
/// @return 0:OK >0:Warning <0:Error
int UiDiskList::ExportDataFiles(const UiDiskListItems &selected_items, const wxString &data_dir, const wxString &attr_dir, const wxString &start_msg, const wxString &end_msg, wxFileDataObject *file_object)
{
	frame->StartExportCounter(0, start_msg);

	// 選択したファイルをリストにする。
	int selcount = (int)selected_items.Count();
	DiskBasicDirItems dir_items;
	for(int i=0; i<selcount; i++) {
		UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(selected_items.Item(i));
		DiskBasicDirItem *item = cd->GetDiskBasicDirItem();
		if (!item) {
			continue;
		}
		dir_items.Add(item);
	}

	int sts = 0;
	do {
		if (dir_items.Count() <= 0) {
			// 対象がない
			break;
		}
		// 選択したファイルのOS
		DiskBasic *basic = dir_items.Item(0)->GetBasic();
		if (!basic) {
			sts = -1;
			break;
		}
		// エクスポート
		sts = frame->ExportDataFiles(basic, &dir_items, data_dir, attr_dir, file_object, 0);
		if (sts != 0) {
			basic->ShowErrorMessage();
		}
	} while(0);

	frame->FinishExportCounter(end_msg);

	return sts;
}

/// ファイルをドロップ
/// @param[in] base         座標の基準となるウィンドウ
/// @param[in] x            ドロップした位置X
/// @param[in] y            ドロップした位置Y
/// @param[in] paths        ドロップしたファイル一覧
/// @param[in] dir_included ファイル一覧にディレクトリが含まれるか
bool UiDiskList::DropDataFiles(wxWindow *base, int x, int y, const wxArrayString &paths, bool dir_included)
{
	if (paths.Count() == 0) {
		return false;
	}

	int bx = 0;
	int by = 0;
	// リストコントロールの親との相対位置
	UiDiskFrame::GetPositionFromBaseWindow(base, this, bx, by);
	UiDiskListItem node = GetNodeAtPoint(x - bx, y - by);
	if (!UiDiskListItem_IsOk(node)) {
		return false;
	}
	// ドラッグしたアイテムと同じならドロップしない
	bool sts = (node == m_dragging_node);
	UiDiskListItem_Unset(m_dragging_node);
	if (sts) {
		return true;
	}

	// 指定位置のディレクトリアイテムを取得
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);
	DiskBasicDirItem *dir_item = cd->GetDiskBasicDirItem();
	if (!dir_item) {
		if (dir_included) {
			return false;
		}
		// ディスクイメージならディスク追加する
		return frame->PreAddDiskFile(paths.Item(0));
	}
	DiskBasic *dir_basic = dir_item->GetBasic();
	if (!dir_basic) {
		return false;
	}

	// フォーマットされていて書き込み可能か
	if (!dir_basic->IsFormatted()) {
		return false;
	}
	if (!dir_basic->IsWritableIntoDisk()) {
		dir_basic->ShowErrorMessage();
		return false;
	}

	// ディレクトリの構成をアサイン
	dir_basic->AssignDirectory(dir_item);

	// カーソル先にあるディレクトリにドロップ
	return frame->ImportDataFiles(paths, dir_basic, dir_item, dir_included, _("dropping..."), _("dropped."));
}

/// クリップボードへコピー
bool UiDiskList::CopyToClipboard()
{
	wxString tmp_dir_name;
	UiDiskListItem sel_node = GetSelection();
	if (!UiDiskListItem_IsOk(sel_node)) {
		return false;
	}

	wxFileDataObject *file_object = new wxFileDataObject();
	bool sts = CreateFileObject(sel_node, tmp_dir_name, _("copying..."), _("copied."), *file_object);
	if (sts) {
		if (wxTheClipboard->Open())	{
		    // This data objects are held by the clipboard,
			// so do not delete them in the app.
			wxTheClipboard->SetData(file_object);
			wxTheClipboard->Close();
		}
	} else {
		delete file_object;
	}
	return sts;
}

/// クリップボードからペースト
bool UiDiskList::PasteFromClipboard()
{
	// 現在選択中のディレクトリにインポート
	UiDiskListItem node = GetSelection();
	if (!UiDiskListItem_IsOk(node)) {
		return false;
	}

	// ディレクトリアイテムを得る
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(node);
	DiskBasicDirItem *dir_item = cd->GetDiskBasicDirItem();
	if (!dir_item) {
		return false;
	}
	// OSを得る
	DiskBasic *dir_basic = dir_item->GetBasic();
	if (!dir_basic) {
		return false;
	}

	// フォーマットされていて書き込み可能か
	if (!dir_basic->IsFormatted()) {
		return false;
	}
	if (!dir_basic->IsWritableIntoDisk()) {
		dir_basic->ShowErrorMessage();
		return false;
	}

	// ディレクトリの構成をアサイン
	dir_basic->AssignDirectory(dir_item);

	// Read some text
	wxFileDataObject file_object;

	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported( wxDF_FILENAME )) {
			wxTheClipboard->GetData( file_object );
		}
		wxTheClipboard->Close();
	}

	// インポート
	// ペーストの時はインポート先ディレクトリを指定しているので確認はしない。
	return frame->ImportDataFiles(file_object.GetFilenames(), dir_basic, dir_item, false, _("pasting..."), _("pasted."));
}

/// 選択しているディスクイメージのディスク番号を返す
int UiDiskList::GetSelectedDiskNumber()
{
	if (!m_selected_disk) return wxNOT_FOUND;
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());
	if (!cd) return wxNOT_FOUND;
	return cd->GetDiskNumber();
}
/// 選択しているディスクイメージのサイド番号を返す
int UiDiskList::GetSelectedDiskSide()
{
	if (!m_selected_disk) return wxNOT_FOUND;
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());
	if (!cd) return wxNOT_FOUND;
	return cd->GetPosition();
}
/// 選択しているディスクイメージのディスク番号とサイド番号を返す
/// @param[out] disk_number ディスク番号
/// @param[out] side_number サイド番号
void UiDiskList::GetSelectedDisk(int &disk_number, int &side_number)
{
	if (!m_selected_disk) return;
	UiDiskPositionData *cd = (UiDiskPositionData *)GetItemData(GetSelection());
	if (!cd) return;
	disk_number = cd->GetDiskNumber();
	side_number = cd->GetPosition();
}

/// ディスクイメージを選択しているか
bool UiDiskList::IsSelectedDiskImage()
{
	return (m_selected_disk != NULL);
}

/// ディスクを選択しているか
bool UiDiskList::IsSelectedDisk()
{
	return m_disk_selecting;
}

/// ディスクを選択しているか(AB面どちらか)
bool UiDiskList::IsSelectedDiskSide()
{
	return (m_disk_selecting && m_selected_disk != NULL && m_selected_disk->IsReversible());	// AB面あり;
}
