/// @file uidisklist.cpp
///
/// @brief ディスクリスト
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uidisklist.h"
#include <wx/imaglist.h>
#include "main.h"
#include "uidiskattr.h"
#include "uifilelist.h"
#include "diskparambox.h"
#include "basicselbox.h"
#include "basicparam.h"
#include "basicfmt.h"
#include "basicdiritem.h"
#include "utils.h"


// icon
extern const char *fd_5inch_16_1_xpm[];
extern const char *fd_5inch_16_2_xpm[];


const char **icons_for_tree[] = {
	fd_5inch_16_1_xpm,
	fd_5inch_16_2_xpm,
	NULL
};
enum en_icons_for_tree {
	ICON_FOR_TREE_NONE = -1,
	ICON_FOR_TREE_SINGLE = 0,
	ICON_FOR_TREE_ROOT,
};

//////////////////////////////////////////////////////////////////////
//
//
//
/// ツリー構造を保存する
/// @param [in] n_disknum   : ディスク番号(0から) ルートは-1
/// @param [in] n_typenum   : ディスク内にAB面があるとき-2、その他-1
/// @param [in] n_sidenum   : ディスク内にAB面があるときサイド番号、それ以外は-1
/// @param [in] n_pos       : ディレクトリ位置
/// @param [in] n_editable  : ディスク名の編集が可能か
/// @param [in] n_ditem     : DISK BASIC ディレクトリアイテム
L3DiskPositionData::L3DiskPositionData(int n_disknum, int n_typenum, int n_sidenum, int n_pos, bool n_editable, DiskBasicDirItem *n_ditem)
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
L3DiskPositionData::~L3DiskPositionData()
{
}

//////////////////////////////////////////////////////////////////////

#ifndef USE_TREE_CTRL_ON_DISK_LIST
//
//
//
L3DiskTreeStoreModel::L3DiskTreeStoreModel(L3DiskFrame *parentframe)
	: wxDataViewTreeStore()
{
	frame = parentframe;
}
/// ディスク名編集できるか
bool L3DiskTreeStoreModel::IsEnabled(const wxDataViewItem &item, unsigned int col) const
{
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd) return false;
	return cd->GetEditable();
}
/// ディスク名を変更した
bool L3DiskTreeStoreModel::SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
{
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd || cd->GetDiskNumber() < 0 || cd->GetEditable() != true) return false;

	wxDataViewIconText data;
	data << variant;
	if (frame->GetDiskD88().SetDiskName(cd->GetDiskNumber(), data.GetText())) {
		SetItemText(item, frame->GetDiskD88().GetDiskName(cd->GetDiskNumber()));
	}
	return true;
}
#endif

//////////////////////////////////////////////////////////////////////
//
// ツリーコントロール
//
L3DiskTreeCtrl::L3DiskTreeCtrl(wxWindow *parentwindow, wxWindowID id)
#ifndef USE_TREE_CTRL_ON_DISK_LIST
       : wxDataViewTreeCtrl(parentwindow, id, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER)
#else
       : wxTreeCtrl(parentwindow, id, wxDefaultPosition, wxDefaultSize, wxTR_EDIT_LABELS | wxTR_NO_LINES | wxTR_HAS_BUTTONS | wxTR_TWIST_BUTTONS)
#endif
{
}

/// ツリーアイテムを選択
void L3DiskTreeCtrl::SelectTreeNode(const L3DiskListItem &node)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	Select(node);
#else
	SelectItem(node);
#endif
}

/// ツリーノードが子供を持つか
bool L3DiskTreeCtrl::TreeNodeHasChildren(const L3DiskListItem &node)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	return IsContainer(node);
#else
	return HasChildren(node);
#endif
}

/// ツリーノードの子供の数を返す
int L3DiskTreeCtrl::GetTreeChildCount(const L3DiskListItem &parent)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	return GetChildCount(parent);
#else
	return (int)GetChildrenCount(parent);
#endif
}

/// ツリーノードを編集
void L3DiskTreeCtrl::EditTreeNode(const L3DiskListItem &node)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	EditItem(node, GetColumn(0));
#else
	EditLabel(node);
#endif
}

/// ツリーノードを削除
void L3DiskTreeCtrl::DeleteTreeNode(const L3DiskListItem &node)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	DeleteItem(node);
#else
	Delete(node);
#endif
}

/// 親ツリーノードを返す
L3DiskListItem L3DiskTreeCtrl::GetParentTreeNode(const L3DiskListItem &node)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	L3DiskListItem invalid;
	L3DiskTreeStoreModel *model = (L3DiskTreeStoreModel *)GetModel();
	if (!model) return invalid;
	return model->GetParent(node);
#else
	return GetItemParent(node);
#endif
}

/// ルートノードを追加する
L3DiskListItem L3DiskTreeCtrl::AddRootTreeNode(const wxString &text, int def_icon, int sel_icon, L3DiskPositionData *n_data)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	return AppendContainer(wxDataViewItem(0), text, def_icon, sel_icon, (wxClientData *)n_data);
#else
	return AddRoot(text, def_icon, sel_icon, (wxTreeItemData *)n_data);
#endif
}

/// ノードを追加する
L3DiskListItem L3DiskTreeCtrl::AddTreeContainer(const L3DiskListItem &parent, const wxString &text, int def_icon, int sel_icon, L3DiskPositionData *n_data)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	return AppendContainer(parent, text, def_icon, sel_icon, (wxClientData *)n_data);
#else
	L3DiskListItem node = AppendItem(parent, text, def_icon, sel_icon, (wxTreeItemData *)n_data);
	SetItemHasChildren(node, true);
	return node;
#endif
}

/// ノードを追加する
L3DiskListItem L3DiskTreeCtrl::AddTreeNode(const L3DiskListItem &parent, const wxString &text, int def_icon, int sel_icon, L3DiskPositionData *n_data)
{
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	return AppendItem(parent, text, def_icon, (wxClientData *)n_data);
#else
	L3DiskListItem node = AppendItem(parent, text, def_icon, sel_icon, (wxTreeItemData *)n_data);
	SetItemHasChildren(node, false);
	return node;
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Left Panel
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskList, L3DiskTreeCtrl)
#ifndef USE_TREE_CTRL_ON_DISK_LIST
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, L3DiskList::OnContextMenu)

	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, L3DiskList::OnSelectionChanged)

	EVT_DATAVIEW_ITEM_EXPANDING(wxID_ANY, L3DiskList::OnItemExpanding)

//	EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY, L3DiskList::OnStartEditing)
//	EVT_DATAVIEW_ITEM_EDITING_DONE(wxID_ANY, L3DiskList::OnEditingDone)
#else
	EVT_TREE_ITEM_MENU(wxID_ANY, L3DiskList::OnContextMenu)
	EVT_TREE_SEL_CHANGED(wxID_ANY, L3DiskList::OnSelectionChanged)
	EVT_TREE_ITEM_EXPANDING(wxID_ANY, L3DiskList::OnItemExpanding)
	EVT_TREE_BEGIN_LABEL_EDIT(wxID_ANY, L3DiskList::OnStartEditing)
	EVT_TREE_END_LABEL_EDIT(wxID_ANY, L3DiskList::OnEditingDone)
#endif

	EVT_MENU(IDM_SAVE_DISK, L3DiskList::OnSaveDisk)
	EVT_MENU(IDM_ADD_DISK_NEW, L3DiskList::OnAddNewDisk)
	EVT_MENU(IDM_ADD_DISK_FROM_FILE, L3DiskList::OnAddDiskFromFile)
	EVT_MENU(IDM_REPLACE_DISK_FROM_FILE, L3DiskList::OnReplaceDisk)
	EVT_MENU(IDM_DELETE_DISK_FROM_FILE, L3DiskList::OnDeleteDisk)
	EVT_MENU(IDM_RENAME_DISK, L3DiskList::OnRenameDisk)

	EVT_MENU(IDM_INITIALIZE_DISK, L3DiskList::OnInitializeDisk)
	EVT_MENU(IDM_FORMAT_DISK, L3DiskList::OnFormatDisk)

	EVT_MENU(IDM_PROPERTY_DISK, L3DiskList::OnPropertyDisk)
	EVT_MENU(IDM_PROPERTY_BASIC, L3DiskList::OnPropertyBasic)
wxEND_EVENT_TABLE()

L3DiskList::L3DiskList(L3DiskFrame *parentframe, wxWindow *parentwindow)
       : L3DiskTreeCtrl(parentwindow, wxID_ANY)
{
	parent   = parentwindow;
	frame    = parentframe;

	initialized = false;

	wxImageList *ilist = new wxImageList( 16, 16 );
	for(int i=0; icons_for_tree[i] != NULL; i++) {
		ilist->Add( wxIcon(icons_for_tree[i]) );
	}
	AssignImageList( ilist );

#ifndef USE_TREE_CTRL_ON_DISK_LIST
	L3DiskTreeStoreModel *model = new L3DiskTreeStoreModel(parentframe);
	AssociateModel(model);
	model->DecRef();
#endif

	wxFont font;
	frame->GetDefaultListFont(font);
	SetFont(font);

	// fit size on parent window
    SetSize(parentwindow->GetClientSize());

	// popup menu
	menuPopup = new wxMenu;
	wxMenu *sm = new wxMenu();
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
	menuPopup->Append(IDM_INITIALIZE_DISK, _("I&nitialize..."));
	menuPopup->Append(IDM_FORMAT_DISK, _("F&ormat For BASIC..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY_DISK, _("Disk &Information..."));
	menuPopup->Append(IDM_PROPERTY_BASIC, _("&BASIC Information..."));

	// key
	Bind(wxEVT_CHAR, &L3DiskList::OnChar, this);

	ClearFileName();

	initialized = true;
}

L3DiskList::~L3DiskList()
{
	// save ini file
//	Config *ini = wxGetApp().GetConfig();

	delete menuPopup;
}

/// 右クリック選択
void L3DiskList::OnContextMenu(L3DiskListEvent& event)
{
	L3DiskListItem item = event.GetItem();
	SelectTreeNode(item);
	ShowPopupMenu();
}

/// ツリーアイテム選択
void L3DiskList::OnSelectionChanged(L3DiskListEvent& event)
{
	L3DiskListItem item = event.GetItem();
	ChangeSelection(item);
}

/// アイテム展開
void L3DiskList::OnItemExpanding(L3DiskListEvent& event)
{
	L3DiskListItem item = event.GetItem();
	ExpandItemNode(item);
}

/// アイテム編集開始
/// @attention wxDataViewTreeCtrlではこのイベントは発生しない！？
void L3DiskList::OnStartEditing(L3DiskListEvent& event)
{
#ifdef USE_TREE_CTRL_ON_DISK_LIST
	L3DiskListItem item = event.GetItem();
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd || !cd->GetEditable()) {
		event.Veto();
	}
#endif
}

/// アイテム編集終了
void L3DiskList::OnEditingDone(L3DiskListEvent& event)
{
#ifdef USE_TREE_CTRL_ON_DISK_LIST
	if (event.IsEditCancelled()) return;
	L3DiskListItem item = event.GetItem();
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd || cd->GetDiskNumber() < 0 || cd->GetEditable() != true) return;

	if (frame->GetDiskD88().SetDiskName(cd->GetDiskNumber(), event.GetLabel())) {
		SetItemText(item, frame->GetDiskD88().GetDiskName(cd->GetDiskNumber()));
	}
#endif
}

/// ディスクを保存選択
void L3DiskList::OnSaveDisk(wxCommandEvent& WXUNUSED(event))
{
	ShowSaveDiskDialog();
}

/// ディスクを新規に追加選択
void L3DiskList::OnAddNewDisk(wxCommandEvent& WXUNUSED(event))
{
	frame->ShowAddNewDiskDialog();
}

/// ディスクをファイルから追加選択
void L3DiskList::OnAddDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	frame->ShowAddFileDialog();
}

/// ディスクイメージを置換選択
void L3DiskList::OnReplaceDisk(wxCommandEvent& WXUNUSED(event))
{
	ReplaceDisk();
}

/// ディスクを削除選択
void L3DiskList::OnDeleteDisk(wxCommandEvent& WXUNUSED(event))
{
	DeleteDisk();
}

/// ディスク名を変更選択
void L3DiskList::OnRenameDisk(wxCommandEvent& event)
{
	RenameDisk();
}

/// 初期化選択
void L3DiskList::OnInitializeDisk(wxCommandEvent& event)
{
	InitializeDisk();
}

/// フォーマット選択
void L3DiskList::OnFormatDisk(wxCommandEvent& event)
{
	frame->FormatDisk();
}

/// プロパティ選択
void L3DiskList::OnPropertyDisk(wxCommandEvent& event)
{
	ShowDiskAttr();
}

/// BASIC情報選択
void L3DiskList::OnPropertyBasic(wxCommandEvent& event)
{
	frame->ShowBasicAttr();
}

/// キー入力
void L3DiskList::OnChar(wxKeyEvent& event)
{
	switch(event.GetKeyCode()) {
	case WXK_RETURN:
		ShowDiskAttr();
		break;
	case WXK_DELETE:
		DeleteDisk();
		break;
	default:
		event.Skip();
		break;
	}
}

/// ポップアップメニュー表示
void L3DiskList::ShowPopupMenu()
{
	if (!menuPopup) return;

	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());

	bool opened = (cd != NULL);
	menuPopup->Enable(IDM_ADD_DISK_NEW, opened);
	menuPopup->Enable(IDM_ADD_DISK_FROM_FILE, opened);

	opened = (opened && (selected_disk != NULL));
	menuPopup->Enable(IDM_REPLACE_DISK_FROM_FILE, opened);
	menuPopup->Enable(IDM_SAVE_DISK, opened);
	menuPopup->Enable(IDM_DELETE_DISK_FROM_FILE, opened);
	menuPopup->Enable(IDM_RENAME_DISK, opened);
	menuPopup->Enable(IDM_INITIALIZE_DISK, opened);
	menuPopup->Enable(IDM_PROPERTY_DISK, opened);

	L3DiskFileList *list = frame->GetFileListPanel();
	opened = (opened && disk_selecting && (list != NULL) && list->CanUseBasicDisk());
	menuPopup->Enable(IDM_FORMAT_DISK, opened);
	menuPopup->Enable(IDM_PROPERTY_BASIC, opened);

	PopupMenu(menuPopup);
}

/// 再選択
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void L3DiskList::ReSelect(const DiskBasicParam *newparam)
{
	L3DiskListItem item = GetSelection();
	ChangeSelection(item, newparam);
}

/// ツリーを選択
/// @param [in] node     選択したノード
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void L3DiskList::ChangeSelection(L3DiskListItem &node, const DiskBasicParam *newparam)
{
	SetDataOnItemNode(node, NODE_SELECTED, newparam);
}

/// ディスクを指定して選択状態にする
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号 両面なら-1
void L3DiskList::ChangeSelection(int disk_number, int side_number)
{
	L3DiskListItem match_node = FindNodeByDiskAndSideNumber(root_node, disk_number, side_number);
	if (match_node.IsOk()) {
		SelectTreeNode(match_node);
		ChangeSelection(match_node);
	}
}

/// ツリーを展開
/// @param [in] node     選択したノード
void L3DiskList::ExpandItemNode(L3DiskListItem &node)
{
	if (!node.IsOk()) return;

	L3DiskListItem sel_node = GetSelection();

	if (sel_node.GetID() != node.GetID()) {
		SetDataOnItemNode(node, NODE_EXPANDED);
	}
}

/// 指定ノードにデータを設定する
/// @param [in] node     選択したノード
/// @param [in] flag     NODE_SELECT / NODE_EXPANDED
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void L3DiskList::SetDataOnItemNode(const L3DiskListItem &node, SetDataOnItemNodeFlags flag, const DiskBasicParam *newparam)
{
	if (!initialized) return;

	if (!node.IsOk()) {
		// rootアイテムを選択したらファイル一覧をクリア
		selected_disk = NULL;
		// 全パネルのデータをクリアする
		frame->ClearAllData();
		return;
	}

	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(node);

	if (flag == NODE_SELECTED) disk_selecting = false;

	if (cd == NULL || (TreeNodeHasChildren(node) && cd->GetDiskNumber() < 0)) {
		// rootアイテムを選択したらファイル一覧をクリア
		selected_disk = NULL;
		// 全パネルのデータをクリアする
		frame->ClearAllData();
		return;
	}

//	if (!cd || cd->GetDiskNumber() < 0) {
//		selected_disk = NULL;
//		return;
//	}

	DiskD88 *d88 = &frame->GetDiskD88();
	DiskD88Disk *disk = d88->GetDisk(cd->GetDiskNumber());
	if (!disk) {
		return;
	}

	// ディスク属性をセット
	if (flag == NODE_SELECTED) {
		selected_disk = disk;
		frame->SetDiskAttrData(disk);
	}

	int typenum = cd->GetTypeNumber();
	int sidenum = cd->GetSideNumber();
	// サイドA,Bがある場合
	if (typenum == CD_TYPENUM_NODE_AB && sidenum < 0) {
		// RAWパネルだけデータをセットする
		frame->ClearAllAndSetRawData(disk, -2);
		if (flag == NODE_SELECTED) Expand(node);
		return;
	}

	if (flag == NODE_SELECTED) disk_selecting = true;

	// 右パネルにファイル名一覧を設定
	if (ParseDiskBasic(node, cd, disk, sidenum, newparam)) {
		frame->SetDataOnDisk(disk, sidenum, flag == NODE_SELECTED);
	}
}

/// DISK BASICをアタッチ＆解析
/// @param [in] node       選択したノード
/// @param [in] cd         ノードの位置情報
/// @param [in] newdisk    ディスクイメージ
/// @param [in] newsidenum サイド番号 片面選択時0/1 両面なら-1
/// @param [in] newparam   BASICパラメータ 通常NULL BASICを変更した際に設定する
/// @return 片面のみ使用するBASICの場合false -> さらに下位ノードで解析
bool L3DiskList::ParseDiskBasic(const L3DiskListItem &node, L3DiskPositionData *cd, DiskD88Disk *newdisk, int newsidenum, const DiskBasicParam *newparam)
{
	DiskBasic *newbasic = newdisk->GetDiskBasic(newsidenum);

	newbasic->SetCharCode(frame->GetCharCode());

	// BASICモードのときはディスクを解析
	if (frame->GetSelectedMode() == 0) {
		// トラックが存在するか
		bool found = newdisk->ExistTrack(newsidenum);
		if (found) {
			// ディスクをDISK BASICとして解析
			if (newbasic->ParseDisk(newdisk, newsidenum, newparam, false) != 0) {
				newbasic->ShowErrorMessage();
			}
		} else {
			// トラックがない
			DiskResult result;
			result.SetWarn(DiskResult::ERR_NO_FOUND_TRACK);
			ResultInfo::ShowMessage(result.GetValid(), result.GetMessages());
			newbasic->Clear();
		}

		// ルートディレクトリをセット
		newbasic->AssignRootDirectory();
//		if (!valid) return true;
	}

	// ディスクがリバーシブルな場合
	if (cd->GetTypeNumber() == CD_TYPENUM_NODE) {
		if (newdisk->IsReversible() || newbasic->CanMountEachSides()) {
			DiskBasics *basics = newdisk->GetDiskBasics();
			if (basics->Count() <= 1) {
				basics->Add(new DiskBasic);
			}
			int type;
			type = CD_TYPENUM_NODE_AB;
			if (newsidenum < 0) {
				cd->SetTypeNumber(type);
			}
			// 現在のツリーの下にサイド分のツリーノードを作成
			for(int i=0; i<2; i++) {
				wxString caption;
				type = CD_TYPENUM_NODE_AB;
				SubCaption(type, i, caption);
				AddTreeContainer(node, caption, ICON_FOR_TREE_SINGLE, ICON_FOR_TREE_NONE,
					new L3DiskPositionData((int)cd->GetDiskNumber(), type, i, i, false));
			}
			Expand(node);
			cd->Shown(false);
			return false;
		}
	}
	return true;
}

/// サブキャプション
void L3DiskList::SubCaption(int type, int side_number, wxString &caption) const
{
	if (side_number < 0) return;

	caption = L3DiskUtils::GetSideStr(side_number, type != CD_TYPENUM_NODE_AB);
}

/// 選択しているディスクの子供を削除
void L3DiskList::DeleteChildrenOnSelectedDisk()
{
	L3DiskListItem node = GetSelection();
	if (!node.IsOk()) return;

	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(node);
	if (!cd) return;

	int disk_number = cd->GetDiskNumber();
	node = FindNodeByDiskNumber(root_node, disk_number); 
	if (!node.IsOk()) return;

	cd = (L3DiskPositionData *)GetItemData(node);

	Collapse(node);
	DeleteChildren(node);
	cd->SetTypeNumber(CD_TYPENUM_NODE);
	cd->Shown(false);
	SelectTreeNode(node);
}

/// 選択しているディスクのルートを初期化＆再選択
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void L3DiskList::RefreshSelectedDisk(const DiskBasicParam *newparam)
{
	// 子供を削除
	DeleteChildrenOnSelectedDisk();
	// 再選択
	ReSelect(newparam);
}

/// 選択しているサイドを再選択
void L3DiskList::RefreshSelectedSide(const DiskBasicParam *newparam)
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
void L3DiskList::SetFileName()
{
	SetFileName(frame->GetFileName());
}

/// ファイル名をリストにセット
/// @param [in] filename ファイル名
void L3DiskList::SetFileName(const wxString &filename)
{
	DiskD88Disks *disks = frame->GetDiskD88().GetDisks();
	if (!disks) return;

	DeleteAllItems();

	L3DiskListItem node = AddRootTreeNode(filename, ICON_FOR_TREE_ROOT, ICON_FOR_TREE_NONE
		, new L3DiskPositionData(CD_DISKNUM_ROOT, CD_TYPENUM_NODE, -1, 0, false));
	root_node = node;

	for(size_t i=0; i<disks->Count(); i++) {
		DiskD88Disk *diskn = disks->Item(i);
		// ディスク１つ
		AddTreeContainer(node, diskn->GetName(), ICON_FOR_TREE_SINGLE, ICON_FOR_TREE_NONE
			, new L3DiskPositionData((int)i, CD_TYPENUM_NODE, CD_TYPENUM_NODE, CD_TYPENUM_NODE, true));
	}

	Expand(node);
	SelectTreeNode(node);
	frame->ClearDiskAttrData();
}

/// リストをクリア
void L3DiskList::ClearFileName()
{
	DeleteAllItems();

	L3DiskListItem node = AddRootTreeNode( _("(none)") );
	root_node = node;

	Expand(node);

	selected_disk = NULL;
	disk_selecting = false;
}

/// ファイルパスをリストにセット
/// @param [in] filename ファイル名
void L3DiskList::SetFilePath(const wxString &filename)
{
	if (!filename.IsEmpty()) {
		SetItemText(root_node, filename);
	} else {
		SetItemText(root_node, _("(none)"));
	}
}

/// ディスク番号と一致するノードをさがす
/// @param [in] node        ノード
/// @param [in] disk_number ディスク番号
/// @param [in] depth       深さ
/// @return 一致したノード
L3DiskListItem L3DiskList::FindNodeByDiskNumber(const L3DiskListItem &node, int disk_number, int depth)
{
	L3DiskListItem match_node;

	if (depth < 100 && TreeNodeHasChildren(node)) {
#ifndef USE_TREE_CTRL_ON_DISK_LIST
		int count = GetChildCount(node);
		for(int idx = 0; idx < count && !match_node.IsOk(); idx++) {
			L3DiskListItem child_node = GetNthChild(node, idx);
#else
		wxTreeItemIdValue cookie;
		L3DiskListItem child_node = GetFirstChild(node, cookie);
		while(child_node.IsOk() && !match_node.IsOk()) {
#endif
			L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(child_node);
			if (cd) {
				// ディスク番号を比較
				if (cd->GetDiskNumber() == disk_number) {
					match_node = child_node;
					break;
//				} else if (TreeNodeHasChildren(child_node)) {
//					// 再帰的に探す
//					match_node = FindNodeByDiskNumber(child_node, disk_number, depth + 1);
				}
			}
#ifdef USE_TREE_CTRL_ON_DISK_LIST
			child_node = GetNextChild(node, cookie);
#endif
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
L3DiskListItem L3DiskList::FindNodeByDiskAndSideNumber(const L3DiskListItem &node, int disk_number, int side_number, int depth)
{
	L3DiskListItem match_node;

	if (depth < 100 && TreeNodeHasChildren(node)) {
#ifndef USE_TREE_CTRL_ON_DISK_LIST
		int count = GetChildCount(node);
		for(int idx = 0; idx < count && !match_node.IsOk(); idx++) {
			L3DiskListItem child_node = GetNthChild(node, idx);
#else
		wxTreeItemIdValue cookie;
		L3DiskListItem child_node = GetFirstChild(node, cookie);
		while(child_node.IsOk() && !match_node.IsOk()) {
#endif
			L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(child_node);
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
#ifdef USE_TREE_CTRL_ON_DISK_LIST
			child_node = GetNextChild(node, cookie);
#endif
		}
	}
	return match_node;
}

/// ディスクの初期化
bool L3DiskList::InitializeDisk()
{
	if (!selected_disk) return false;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());

	int ans = wxYES;
	wxString diskname = wxT("'")+selected_disk->GetName()+wxT("'");
	int selected_side = cd->GetPosition();
	bool found = selected_disk->ExistTrack(selected_side);
	bool sts = false;
	if (found) {
		// トラックがある場合は、初期化
		if (selected_side >= 0) {
			diskname += L3DiskUtils::GetSideStr(selected_side, cd->GetTypeNumber() != CD_TYPENUM_NODE_AB);
		}
		wxString msg = wxString::Format(_("All files and datas will delete on %s. Do you really want to initialize it?"), diskname);
		ans = wxMessageBox(msg, _("Initialize Disk"), wxYES_NO);

		if (ans == wxYES) {
			// ディスク初期化
			sts = selected_disk->Initialize(selected_side);
			// 右パネルをクリア
			frame->ClearRPanelData();
			// DISK BASICを改めて解析させる
			DiskBasics *basics = selected_disk->GetDiskBasics();
			if (basics) {
				basics->ClearParseAndAssign(selected_side);
			}
			// ディスク再選択
			RefreshSelectedDisk();

			frame->IncrementUniqueNumber();
		}
	} else {
		// トラックが全くない場合は、ディスク作成
		if (selected_side >= 0) {
			// 選択したサイドだけ作り直す
			DiskParamBox dlg(this, wxID_ANY, _("Rebuild Tracks"), 0, selected_disk, NULL, NULL, 0);
			int rc = dlg.ShowModal();
			if (rc == wxID_OK) {
				DiskParam param;
				dlg.GetParam(param);
				selected_disk->GetFile()->SetBasicTypeHint(dlg.GetCategory());
				sts = selected_disk->Rebuild(param, selected_side);

				// ファイル名一覧を更新
				SetFileName();

				frame->IncrementUniqueNumber();
			}
		} else {
			// パラメータを選択するダイアログを表示
			DiskParamBox dlg(this, wxID_ANY, _("Rebuild Tracks"), -1, selected_disk, NULL, NULL, DiskParamBox::SHOW_ALL);
			int rc = dlg.ShowModal();
			if (rc == wxID_OK) {
				DiskParam param;
				dlg.GetParam(param);
				selected_disk->SetName(dlg.GetDiskName());
				selected_disk->SetDensity(dlg.GetDensity());
				selected_disk->SetWriteProtect(dlg.IsWriteProtected());
				selected_disk->GetFile()->SetBasicTypeHint(dlg.GetCategory());
				sts = selected_disk->Rebuild(param, selected_side);

				// ファイル名一覧を更新
				SetFileName();

				frame->IncrementUniqueNumber();
			}
		}
	}

	return sts;
}

/// ディスクの論理フォーマット
bool L3DiskList::FormatDisk()
{
	if (!selected_disk) return false;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());
	int selected_side = cd->GetPosition();

	DiskBasic *current_basic = selected_disk->GetDiskBasic(selected_side);

	// BASIC種類を選択
	BasicSelBox dlg(this, wxID_ANY, selected_disk, current_basic, BasicSelBox::SHOW_ATTR_CONTROLS);
	int ans = dlg.ShowModal();
	if (ans != wxID_OK) {
		return false;
	}

	DiskBasic new_basic;
	new_basic.ParseDisk(selected_disk, selected_side, dlg.GetBasicParam(), true);
	if (!new_basic.IsFormattable()) {
		new_basic.ShowErrorMessage();
		return false;
	}

	ans = wxYES;
	wxString diskname = wxT("'")+selected_disk->GetName()+wxT("'");
	diskname += current_basic->GetSelectedSideStr();
	wxString msg = wxString::Format(_("All files and datas will delete on %s. Do you really want to format it?"), diskname);
	ans = wxMessageBox(msg, _("Format"), wxYES_NO);

	int sts = 0;
	if (ans == wxYES) {
		// ディスクを初期化
		DiskBasicIdentifiedData data(
			dlg.GetVolumeName(),
			dlg.GetVolumeNumber()
		);

		selected_disk->Initialize(current_basic->GetSelectedSide());
		selected_disk->SetModify();

		// DISK BASIC用にフォーマット
		current_basic->ClearParseAndAssign();
		if (current_basic->GetBasicTypeName() != new_basic.GetBasicTypeName()) {
			// 違うDISK BASICの場合はパラメータを設定
			selected_disk->GetFile()->SetBasicTypeHint(new_basic.GetBasicCategoryName());
			current_basic->ParseDisk(selected_disk, selected_side, dlg.GetBasicParam(), true);
		}

		sts = current_basic->FormatDisk(data);
		if (sts != 0) {
			current_basic->ShowErrorMessage();

		}
		frame->ClearFatAreaData();
		// ディスク再選択
		RefreshSelectedDisk();

		frame->IncrementUniqueNumber();
	}
//	frame->UpdateMenuAndToolBarFileList(this);

	return (sts >= 0);
}

/// ディスクをファイルに保存ダイアログ
void L3DiskList::ShowSaveDiskDialog()
{
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());
	if (!cd) return;
	frame->ShowSaveDiskDialog(cd->GetDiskNumber(), cd->GetPosition(), cd->GetTypeNumber() != CD_TYPENUM_NODE_AB);
}

/// ディスクを置換
void L3DiskList::ReplaceDisk()
{
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());
	if (!cd) return;

	wxString caption;
	SubCaption(cd->GetTypeNumber(), cd->GetPosition(), caption);

	frame->ShowReplaceDiskDialog(cd->GetDiskNumber(), cd->GetPosition(), caption);
}

/// ディスクをファイルから削除
bool L3DiskList::DeleteDisk()
{
	if (!selected_disk) return false;

	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());

	int ans = wxYES;
	wxString diskname = wxT("'")+selected_disk->GetName()+wxT("'");
	wxString msg = wxString::Format(_("%s will be deleted. Do you really want to delete it?"), diskname);
	ans = wxMessageBox(msg, _("Delete Disk"), wxYES_NO);

	bool sts = false;
	if (ans == wxYES) {
		sts = frame->GetDiskD88().Delete(cd->GetDiskNumber());

		// 画面を更新
		frame->UpdateDataOnWindow(false);

		// プロパティダイアログを閉じる
		frame->CloseAllFileAttr();
	}
	return sts;
}

/// ディスク名を変更
void L3DiskList::RenameDisk()
{
	L3DiskListItem node = SetSelectedItemAtDiskImage();
	if (!node.IsOk()) return;
	EditTreeNode(node);
}

/// ディスク情報ダイアログ
void L3DiskList::ShowDiskAttr()
{
	if (!selected_disk) return;

	DiskParamBox dlg(this, wxID_ANY, _("Disk Parameter"), -1, selected_disk, NULL, NULL, DiskParamBox::SHOW_DISKLABEL_ALL);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		DiskParam param;
		dlg.GetParam(param);
		selected_disk->SetName(dlg.GetDiskName());
		selected_disk->SetDensity(dlg.GetDensity());
		selected_disk->SetWriteProtect(dlg.IsWriteProtected());
		selected_disk->SetModify();
		// ディスク名をセット
		SetDiskName(selected_disk->GetName());
		// ディスク属性をセット
		frame->SetDiskAttrData(selected_disk);
	}
}

/// 選択位置のディスクイメージ
L3DiskListItem L3DiskList::SetSelectedItemAtDiskImage()
{
	L3DiskListItem invalid;
	if (!selected_disk) return invalid;
	L3DiskListItem node = GetSelection();
	if (!node.IsOk()) return invalid;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(node);
	if (!cd) return invalid;
	if (cd->GetPosition() >= 0) {
		// ディスク名は親アイテムになる
		node = GetParentTreeNode(node);
		if (!node.IsOk()) return invalid;
		L3DiskPositionData *pcd = (L3DiskPositionData *)GetItemData(node);
		if (pcd->GetDiskNumber() != cd->GetDiskNumber()) return invalid;
	}
	return node;
}

/// 選択位置のディスク名をセット
void L3DiskList::SetDiskName(const wxString &val)
{
	L3DiskListItem item = SetSelectedItemAtDiskImage();
	if (!item.IsOk()) return;
	SetItemText(item, val);
}

/// キャラクターコード変更
void L3DiskList::ChangeCharCode(int sel)
{
}

/// フォントをセット
void L3DiskList::SetListFont(const wxFont &font)
{
	SetFont(font);
	Refresh();
}

/// 選択しているディスクイメージのディスク番号を返す
int L3DiskList::GetSelectedDiskNumber()
{
	if (!selected_disk) return wxNOT_FOUND;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());
	if (!cd) return wxNOT_FOUND;
	return cd->GetDiskNumber();
}
/// 選択しているディスクイメージのサイド番号を返す
int L3DiskList::GetSelectedDiskSide()
{
	if (!selected_disk) return wxNOT_FOUND;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());
	if (!cd) return wxNOT_FOUND;
	return cd->GetPosition();
}
/// 選択しているディスクイメージのディスク番号とサイド番号を返す
void L3DiskList::GetSelectedDisk(int &disk_number, int &side_number)
{
	if (!selected_disk) return;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());
	if (!cd) return;
	disk_number = cd->GetDiskNumber();
	side_number = cd->GetPosition();
}

/// ディスクイメージを選択しているか
bool L3DiskList::IsSelectedDiskImage()
{
	return (selected_disk != NULL);
}

/// ディスクを選択しているか
bool L3DiskList::IsSelectedDisk()
{
	return disk_selecting;
}

/// ディスクを選択しているか(AB面どちらか)
bool L3DiskList::IsSelectedDiskSide()
{
	return (disk_selecting && selected_disk != NULL && selected_disk->IsReversible());	// AB面あり;
}
