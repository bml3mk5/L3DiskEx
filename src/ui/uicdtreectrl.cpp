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
MyCDTreeCtrl::MyCDTreeCtrl(wxWindow *parentwindow, wxWindowID id)
       : wxDataViewTreeCtrl(parentwindow, id, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER)
{
}

/// アイコンを追加
void MyCDTreeCtrl::AssignTreeIcons(const char ***icons)
{
	wxImageList *ilist = new wxImageList( 16, 16 );
	for(int i=0; icons[i] != NULL; i++) {
		ilist->Add( wxIcon(icons[i]) );
	}
	AssignImageList( ilist );
}

/// ツリーアイテムを選択
void MyCDTreeCtrl::SelectTreeNode(const MyCDTreeItem &node)
{
	Select(node);
}

/// ツリーノードが子供を持つか
bool MyCDTreeCtrl::TreeNodeHasChildren(const MyCDTreeItem &node)
{
	return IsContainer(node);
}

/// ツリーノードの子供の数を返す
int MyCDTreeCtrl::GetTreeChildCount(const MyCDTreeItem &parent)
{
	return GetChildCount(parent);
}

/// ツリーノードを編集
void MyCDTreeCtrl::EditTreeNode(const MyCDTreeItem &node)
{
	EditItem(node, GetColumn(0));
}

/// ツリーノードを削除
void MyCDTreeCtrl::DeleteTreeNode(const MyCDTreeItem &node)
{
	DeleteItem(node);
}

/// 親ツリーノードを返す
MyCDTreeItem MyCDTreeCtrl::GetParentTreeNode(const MyCDTreeItem &node)
{
	MyCDTreeItem invalid;
	wxDataViewTreeStore *model = GetStore();
	if (!model) return invalid;
	return model->GetParent(node);
}

/// ルートノードを追加する
MyCDTreeItem MyCDTreeCtrl::AddRootTreeNode(const wxString &text, int def_icon, int sel_icon, wxClientData *n_data)
{
	return AppendContainer(wxDataViewItem(0), text, def_icon, sel_icon, n_data);
}

/// ノードを追加する
MyCDTreeItem MyCDTreeCtrl::AddTreeContainer(const MyCDTreeItem &parent, const wxString &text, int def_icon, int sel_icon, wxClientData *n_data)
{
	return AppendContainer(parent, text, def_icon, sel_icon, n_data);
}

/// ノードを追加する
MyCDTreeItem MyCDTreeCtrl::AddTreeNode(const MyCDTreeItem &parent, const wxString &text, int def_icon, int sel_icon, wxClientData *n_data)
{
	return AppendItem(parent, text, def_icon, n_data);
}

/// 最初の子ノードを返す
MyCDTreeItem MyCDTreeCtrl::GetFirstChild(const MyCDTreeItem &parent, unsigned int &cookie)
{
	cookie = 0;
	return (int)cookie < GetChildCount(parent) ? GetNthChild(parent, cookie) : MyCDTreeItem();
}

/// 次の子ノードを返す
MyCDTreeItem MyCDTreeCtrl::GetNextChild(const MyCDTreeItem &parent, unsigned int &cookie)
{
	cookie++;
	return (int)cookie < GetChildCount(parent) ? GetNthChild(parent, cookie) : MyCDTreeItem();
}

/// 指定した座標にノードがあるか
bool MyCDTreeCtrl::HasNodeAtPoint(int x, int y) const
{
	wxPoint pt(x, y);
	MyCDTreeItem item;
	wxDataViewColumn* column;
	HitTest(pt, item, column);
	return item.IsOk();
}

/// 指定した座標にあるノードを返す
MyCDTreeItem MyCDTreeCtrl::GetNodeAtPoint(int x, int y) const
{
	wxPoint pt(x, y);
	MyCDTreeItem item;
	wxDataViewColumn* column;
	HitTest(pt, item, column);
	return item;
}
