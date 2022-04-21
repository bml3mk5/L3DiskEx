/// @file uicdtreectrl.h
///
/// @brief データビューツリーコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UICDTREECTRL_H_
#define _UICDTREECTRL_H_

#include "../common.h"
#include <wx/dataview.h>
#include <wx/clntdata.h>
#include "uicommon.h"


#define L3CDTreeItem  wxDataViewItem
#define L3CDTreeEvent wxDataViewEvent

//////////////////////////////////////////////////////////////////////

/// ツリーコントロール
class L3CDTreeCtrl: public wxDataViewTreeCtrl
{
protected:
	/// アイコンを追加
	void AssignTreeIcons(const char ***icons);

public:
	L3CDTreeCtrl(wxWindow *parentwindow, wxWindowID id);

	/// ツリーノードを選択
	void SelectTreeNode(const L3CDTreeItem &node);
	/// ツリーノードが子供を持つか
	bool TreeNodeHasChildren(const L3CDTreeItem &node);
	/// ツリーノードの子供の数を返す
	int  GetTreeChildCount(const L3CDTreeItem &parent);
	/// ツリーノードを編集
	void EditTreeNode(const L3CDTreeItem &node);
	/// ツリーノードを削除
	void DeleteTreeNode(const L3CDTreeItem &node);
	/// 親ツリーノードを返す
	L3CDTreeItem GetParentTreeNode(const L3CDTreeItem &node);
	/// ルートノードを追加する
	L3CDTreeItem AddRootTreeNode(const wxString &text, int def_icon = -1, int sel_icon = -1, wxClientData *n_data = NULL);
	/// ノードを追加する
	L3CDTreeItem AddTreeContainer(const L3CDTreeItem &parent, const wxString &text, int def_icon = -1, int sel_icon = -1, wxClientData *n_data = NULL);
	/// ノードを追加する
	L3CDTreeItem AddTreeNode(const L3CDTreeItem &parent, const wxString &text, int def_icon = -1, int sel_icon = -1, wxClientData *n_data = NULL);

	/// 最初の子ノードを返す
	L3CDTreeItem GetFirstChild(const L3CDTreeItem &parent, unsigned int &cookie);
	/// 次の子ノードを返す
	L3CDTreeItem GetNextChild(const L3CDTreeItem &parent, unsigned int &cookie);
};

#endif /* _UICDTREECTRL_H_ */

