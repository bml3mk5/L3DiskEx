/// @file uicdtreectrl.cpp
///
/// @brief データビューツリーコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uicdtreectrl.h"
#include <wx/imaglist.h>
#include <wx/icon.h>
#include "../config.h"
#include "uicommon.h"


//////////////////////////////////////////////////////////////////////
//
// データビューツリーコントロール
//
L3CDTreeCtrl::L3CDTreeCtrl(wxWindow *parentwindow, wxWindowID id)
       : wxDataViewTreeCtrl(parentwindow, id, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER)
{
}

/// アイコンを追加
void L3CDTreeCtrl::AssignTreeIcons(const char ***icons)
{
	wxImageList *ilist = new wxImageList( 16, 16 );
	for(int i=0; icons[i] != NULL; i++) {
		ilist->Add( wxIcon(icons[i]) );
	}
	AssignImageList( ilist );
}

/// ツリーアイテムを選択
void L3CDTreeCtrl::SelectTreeNode(const L3CDTreeItem &node)
{
	Select(node);
}

/// ツリーノードが子供を持つか
bool L3CDTreeCtrl::TreeNodeHasChildren(const L3CDTreeItem &node)
{
	return IsContainer(node);
}

/// ツリーノードの子供の数を返す
int L3CDTreeCtrl::GetTreeChildCount(const L3CDTreeItem &parent)
{
	return GetChildCount(parent);
}

/// ツリーノードを編集
void L3CDTreeCtrl::EditTreeNode(const L3CDTreeItem &node)
{
	EditItem(node, GetColumn(0));
}

/// ツリーノードを削除
void L3CDTreeCtrl::DeleteTreeNode(const L3CDTreeItem &node)
{
	DeleteItem(node);
}

/// 親ツリーノードを返す
L3CDTreeItem L3CDTreeCtrl::GetParentTreeNode(const L3CDTreeItem &node)
{
	L3CDTreeItem invalid;
	wxDataViewTreeStore *model = GetStore();
	if (!model) return invalid;
	return model->GetParent(node);
}

/// ルートノードを追加する
L3CDTreeItem L3CDTreeCtrl::AddRootTreeNode(const wxString &text, int def_icon, int sel_icon, wxClientData *n_data)
{
	return AppendContainer(wxDataViewItem(0), text, def_icon, sel_icon, n_data);
}

/// ノードを追加する
L3CDTreeItem L3CDTreeCtrl::AddTreeContainer(const L3CDTreeItem &parent, const wxString &text, int def_icon, int sel_icon, wxClientData *n_data)
{
	return AppendContainer(parent, text, def_icon, sel_icon, n_data);
}

/// ノードを追加する
L3CDTreeItem L3CDTreeCtrl::AddTreeNode(const L3CDTreeItem &parent, const wxString &text, int def_icon, int sel_icon, wxClientData *n_data)
{
	return AppendItem(parent, text, def_icon, n_data);
}

/// 最初の子ノードを返す
L3CDTreeItem L3CDTreeCtrl::GetFirstChild(const L3CDTreeItem &parent, unsigned int &cookie)
{
	cookie = 0;
	return (int)cookie < GetChildCount(parent) ? GetNthChild(parent, cookie) : L3CDTreeItem();
}

/// 次の子ノードを返す
L3CDTreeItem L3CDTreeCtrl::GetNextChild(const L3CDTreeItem &parent, unsigned int &cookie)
{
	cookie++;
	return (int)cookie < GetChildCount(parent) ? GetNthChild(parent, cookie) : L3CDTreeItem();
}
