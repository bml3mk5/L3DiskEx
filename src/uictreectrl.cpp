/// @file uictreectrl.cpp
///
/// @brief ツリーコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uictreectrl.h"
#include <wx/imaglist.h>
#include <wx/icon.h>
#include "config.h"
#include "uicommon.h"


//////////////////////////////////////////////////////////////////////
//
// ツリーコントロール
//
L3CTreeCtrl::L3CTreeCtrl(wxWindow *parentwindow, wxWindowID id)
       : wxTreeCtrl(parentwindow, id, wxDefaultPosition, wxDefaultSize, wxTR_EDIT_LABELS | wxTR_NO_LINES | wxTR_HAS_BUTTONS | wxTR_TWIST_BUTTONS)
{
}

/// アイコンを追加
void L3CTreeCtrl::AssignTreeIcons(const char ***icons)
{
	wxImageList *ilist = new wxImageList( 16, 16 );
	for(int i=0; icons[i] != NULL; i++) {
		ilist->Add( wxIcon(icons[i]) );
	}
	AssignImageList( ilist );
}

/// ツリーアイテムを選択
void L3CTreeCtrl::SelectTreeNode(const L3CTreeItem &node)
{
	SelectItem(node);
}

/// ツリーノードが子供を持つか
bool L3CTreeCtrl::TreeNodeHasChildren(const L3CTreeItem &node)
{
	return HasChildren(node);
}

/// ツリーノードの子供の数を返す
int L3CTreeCtrl::GetTreeChildCount(const L3CTreeItem &parent)
{
	return (int)GetChildrenCount(parent);
}

/// ツリーノードを編集
void L3CTreeCtrl::EditTreeNode(const L3CTreeItem &node)
{
	EditLabel(node);
}

/// ツリーノードを削除
void L3CTreeCtrl::DeleteTreeNode(const L3CTreeItem &node)
{
	Delete(node);
}

/// 親ツリーノードを返す
L3CTreeItem L3CTreeCtrl::GetParentTreeNode(const L3CTreeItem &node)
{
	return GetItemParent(node);
}

/// ルートノードを追加する
L3CTreeItem L3CTreeCtrl::AddRootTreeNode(const wxString &text, int def_icon, int sel_icon, wxTreeItemData *n_data)
{
	return AddRoot(text, def_icon, sel_icon, n_data);
}

/// ノードを追加する
L3CTreeItem L3CTreeCtrl::AddTreeContainer(const L3CTreeItem &parent, const wxString &text, int def_icon, int sel_icon, wxTreeItemData *n_data)
{
	L3CTreeItem node = AppendItem(parent, text, def_icon, sel_icon, n_data);
	SetItemHasChildren(node, true);
	return node;
}

/// ノードを追加する
L3CTreeItem L3CTreeCtrl::AddTreeNode(const L3CTreeItem &parent, const wxString &text, int def_icon, int sel_icon, wxTreeItemData *n_data)
{
	L3CTreeItem node = AppendItem(parent, text, def_icon, sel_icon, n_data);
	SetItemHasChildren(node, false);
	return node;
}
