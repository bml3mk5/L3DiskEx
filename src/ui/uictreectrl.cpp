/// @file uictreectrl.cpp
///
/// @brief ツリーコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uictreectrl.h"
#include <wx/imaglist.h>
#include <wx/icon.h>
#include "../config.h"
#include "uicommon.h"


//////////////////////////////////////////////////////////////////////
//
// ツリーコントロール
//
MyCTreeCtrl::MyCTreeCtrl(wxWindow *parentwindow, wxWindowID id)
	: wxTreeCtrl(parentwindow, id, wxDefaultPosition, wxDefaultSize, wxTR_EDIT_LABELS | wxTR_NO_LINES | wxTR_HAS_BUTTONS | wxTR_TWIST_BUTTONS)
{
}

/// アイコンを追加
void MyCTreeCtrl::AssignTreeIcons(const char ***icons)
{
	wxImageList *ilist = new wxImageList( 16, 16 );
	for(int i=0; icons[i] != NULL; i++) {
		ilist->Add( wxIcon(icons[i]) );
	}
	AssignImageList( ilist );
}

/// ツリーアイテムを選択
void MyCTreeCtrl::SelectTreeNode(const MyCTreeItem &node)
{
	SelectItem(node);
}

/// ツリーノードが子供を持つか
bool MyCTreeCtrl::TreeNodeHasChildren(const MyCTreeItem &node)
{
	return HasChildren(node);
}

/// ツリーノードの子供の数を返す
int MyCTreeCtrl::GetTreeChildCount(const MyCTreeItem &parent)
{
	return (int)GetChildrenCount(parent);
}

/// ツリーノードを編集
void MyCTreeCtrl::EditTreeNode(const MyCTreeItem &node)
{
	EditLabel(node);
}

/// ツリーノードを削除
void MyCTreeCtrl::DeleteTreeNode(const MyCTreeItem &node)
{
	Delete(node);
}

/// 親ツリーノードを返す
MyCTreeItem MyCTreeCtrl::GetParentTreeNode(const MyCTreeItem &node)
{
	return GetItemParent(node);
}

/// ルートノードを追加する
MyCTreeItem MyCTreeCtrl::AddRootTreeNode(const wxString &text, int def_icon, int sel_icon, wxTreeItemData *n_data)
{
	return AddRoot(text, def_icon, sel_icon, n_data);
}

/// ノードを追加する
MyCTreeItem MyCTreeCtrl::AddTreeContainer(const MyCTreeItem &parent, const wxString &text, int def_icon, int sel_icon, wxTreeItemData *n_data)
{
	MyCTreeItem node = AppendItem(parent, text, def_icon, sel_icon, n_data);
	SetItemHasChildren(node, true);
	return node;
}

/// ノードを追加する
MyCTreeItem MyCTreeCtrl::AddTreeNode(const MyCTreeItem &parent, const wxString &text, int def_icon, int sel_icon, wxTreeItemData *n_data)
{
	MyCTreeItem node = AppendItem(parent, text, def_icon, sel_icon, n_data);
	SetItemHasChildren(node, false);
	return node;
}

/// 指定した座標にノードがあるか
bool MyCTreeCtrl::HasNodeAtPoint(int x, int y) const
{
	wxPoint pt(x, y);
	return HitTest(pt).IsOk();
}

/// 指定した座標にあるノードを返す
MyCTreeItem MyCTreeCtrl::GetNodeAtPoint(int x, int y) const
{
	wxPoint pt(x, y);
	return HitTest(pt);
}
