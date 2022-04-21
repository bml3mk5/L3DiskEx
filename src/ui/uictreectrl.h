/// @file uictreectrl.h
///
/// @brief ツリーコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UICTREECTRL_H_
#define _UICTREECTRL_H_

#include "../common.h"
#include <wx/treectrl.h>
#include <wx/treebase.h>
#include "uicommon.h"


#define L3CTreeItem  wxTreeItemId
#define L3CTreeEvent wxTreeEvent

//////////////////////////////////////////////////////////////////////

/// ツリーコントロール
class L3CTreeCtrl: public wxTreeCtrl
{
protected:
	/// アイコンを追加
	void AssignTreeIcons(const char ***icons);

public:
	L3CTreeCtrl(wxWindow *parentwindow, wxWindowID id);

	/// ツリーノードを選択
	void SelectTreeNode(const L3CTreeItem &node);
	/// ツリーノードが子供を持つか
	bool TreeNodeHasChildren(const L3CTreeItem &node);
	/// ツリーノードの子供の数を返す
	int  GetTreeChildCount(const L3CTreeItem &parent);
	/// ツリーノードを編集
	void EditTreeNode(const L3CTreeItem &node);
	/// ツリーノードを削除
	void DeleteTreeNode(const L3CTreeItem &node);
	/// 親ツリーノードを返す
	L3CTreeItem GetParentTreeNode(const L3CTreeItem &node);
	/// ルートノードを追加する
	L3CTreeItem AddRootTreeNode(const wxString &text, int def_icon = -1, int sel_icon = -1, wxTreeItemData *n_data = NULL);
	/// ノードを追加する
	L3CTreeItem AddTreeContainer(const L3CTreeItem &parent, const wxString &text, int def_icon = -1, int sel_icon = -1, wxTreeItemData *n_data = NULL);
	/// ノードを追加する
	L3CTreeItem AddTreeNode(const L3CTreeItem &parent, const wxString &text, int def_icon = -1, int sel_icon = -1, wxTreeItemData *n_data = NULL);
};

#endif /* _UICTREECTRL_H_ */

