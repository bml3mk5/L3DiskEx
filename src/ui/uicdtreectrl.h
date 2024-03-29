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


#define MyCDTreeItem  wxDataViewItem
#define MyCDTreeEvent wxDataViewEvent

//////////////////////////////////////////////////////////////////////

/// ツリーコントロール
class MyCDTreeCtrl: public wxDataViewTreeCtrl
{
protected:
	/// アイコンを追加
	void AssignTreeIcons(const char ***icons);

public:
	MyCDTreeCtrl(wxWindow *parentwindow, wxWindowID id);

	/// ツリーノードを選択
	void SelectTreeNode(const MyCDTreeItem &node);
	/// ツリーノードが子供を持つか
	bool TreeNodeHasChildren(const MyCDTreeItem &node);
	/// ツリーノードの子供の数を返す
	int  GetTreeChildCount(const MyCDTreeItem &parent);
	/// ツリーノードを編集
	void EditTreeNode(const MyCDTreeItem &node);
	/// ツリーノードを削除
	void DeleteTreeNode(const MyCDTreeItem &node);
	/// 親ツリーノードを返す
	MyCDTreeItem GetParentTreeNode(const MyCDTreeItem &node);
	/// ルートノードを追加する
	MyCDTreeItem AddRootTreeNode(const wxString &text, int def_icon = -1, int sel_icon = -1, wxClientData *n_data = NULL);
	/// ノードを追加する
	MyCDTreeItem AddTreeContainer(const MyCDTreeItem &parent, const wxString &text, int def_icon = -1, int sel_icon = -1, wxClientData *n_data = NULL);
	/// ノードを追加する
	MyCDTreeItem AddTreeNode(const MyCDTreeItem &parent, const wxString &text, int def_icon = -1, int sel_icon = -1, wxClientData *n_data = NULL);

	/// 最初の子ノードを返す
	MyCDTreeItem GetFirstChild(const MyCDTreeItem &parent, unsigned int &cookie);
	/// 次の子ノードを返す
	MyCDTreeItem GetNextChild(const MyCDTreeItem &parent, unsigned int &cookie);

	/// 指定した座標にノードがあるか
	bool HasNodeAtPoint(int x, int y) const;
	/// 指定した座標にあるノードを返す
	MyCDTreeItem GetNodeAtPoint(int x, int y) const;
};

#endif /* _UICDTREECTRL_H_ */

