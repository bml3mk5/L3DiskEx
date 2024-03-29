/// @file uictreectrl.h
///
/// @brief ツリーコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef UICTREECTRL_H
#define UICTREECTRL_H

#include "../common.h"
#include <wx/treectrl.h>
#include <wx/treebase.h>
#include "uicommon.h"


#define MyCTreeItem  wxTreeItemId
#define MyCTreeEvent wxTreeEvent

//////////////////////////////////////////////////////////////////////

/// ツリーコントロール
class MyCTreeCtrl: public wxTreeCtrl
{
protected:
	/// アイコンを追加
	void AssignTreeIcons(const char ***icons);

public:
	MyCTreeCtrl(wxWindow *parentwindow, wxWindowID id);

	/// ツリーノードを選択
	void SelectTreeNode(const MyCTreeItem &node);
	/// ツリーノードが子供を持つか
	bool TreeNodeHasChildren(const MyCTreeItem &node);
	/// ツリーノードの子供の数を返す
	int  GetTreeChildCount(const MyCTreeItem &parent);
	/// ツリーノードを編集
	void EditTreeNode(const MyCTreeItem &node);
	/// ツリーノードを削除
	void DeleteTreeNode(const MyCTreeItem &node);
	/// 親ツリーノードを返す
	MyCTreeItem GetParentTreeNode(const MyCTreeItem &node);
	/// ルートノードを追加する
	MyCTreeItem AddRootTreeNode(const wxString &text, int def_icon = -1, int sel_icon = -1, wxTreeItemData *n_data = NULL);
	/// ノードを追加する
	MyCTreeItem AddTreeContainer(const MyCTreeItem &parent, const wxString &text, int def_icon = -1, int sel_icon = -1, wxTreeItemData *n_data = NULL);
	/// ノードを追加する
	MyCTreeItem AddTreeNode(const MyCTreeItem &parent, const wxString &text, int def_icon = -1, int sel_icon = -1, wxTreeItemData *n_data = NULL);
	/// 指定した座標にノードがあるか
	bool HasNodeAtPoint(int x, int y) const;
	/// 指定した座標にあるノードを返す
	MyCTreeItem GetNodeAtPoint(int x, int y) const;
};

#endif /* UICTREECTRL_H */

