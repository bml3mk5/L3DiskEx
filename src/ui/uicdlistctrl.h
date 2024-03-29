/// @file uicdlistctrl.h
///
/// @brief データ―ビューリストコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UICDLISTCTRL_H_
#define _UICDLISTCTRL_H_

#include "../common.h"
#include <wx/dataview.h>
#include <wx/clntdata.h>
#include <wx/rearrangectrl.h>
#include "uicommon.h"


#define MyCDListItem		wxDataViewItem
#define MyCDListItems		wxDataViewItemArray
#define MyCDListEvent		wxDataViewEvent

class wxMenu;
class wxIcon;
class UiDiskFrame;
class Config;

//////////////////////////////////////////////////////////////////////

/// リストコントロール内の値
class MyCDListValue
{
protected:
	int row;
	int icon;
	wxString value;
public:
	MyCDListValue();

	void Set(long n_row, int n_icon, const wxString &n_value);
	void Set(long n_row, const wxString &n_value);
	void SetColumn(int val) {}
	int  GetImage() const { return icon; }
	const wxString &GetText() const { return value; }
};

//////////////////////////////////////////////////////////////////////

/// リストのカラム情報
class MyCDListColumn
{
protected:
	int idx;
	int col;
	const struct st_list_columns *info;
	wxString label;
	int width;
	int sort_dir;
	wxDataViewColumn *id;
public:
	MyCDListColumn(int n_idx, const struct st_list_columns *n_info, int n_width);

	void	Set(int n_idx, const struct st_list_columns *n_info, int n_width);
	int		GetIndex() const { return idx; }
	int		GetColumn() const { return col; }
	void	SetColumn(int val) { col = val; }
	bool	HaveIcon() const;
	int		GetWidth() const { return width; }
	void	SetWidth(int val) { width = val; }
	const wxString &GetText() const { return label; }
	wxAlignment GetAlign() const;
	bool	IsSortable() const;
	int		GetSortDir() const { return sort_dir; }
	void	SetSortDir(int val) { sort_dir = val; }
	wxDataViewColumn *GetId() const { return id; }
	void	SetId(wxDataViewColumn *val) { id = val; }
};

WX_DEFINE_ARRAY(MyCDListColumn *, MyCDListColumns);

//////////////////////////////////////////////////////////////////////

WX_DEFINE_ARRAY(const wxIcon *, MyCDListIcons);

/// リストコントロール
class MyCDListCtrl : public wxDataViewListCtrl
{
protected:
	UiDiskFrame		*frame;
	MyCDListColumns	 m_columns;
	int				 m_idOnFirstColumn;
	Config			*m_ini;
	MyCDListIcons	 m_icons;
	wxArrayInt		 m_selecting;

	/// アイコンを設定
	void AssignListIcons(const char ***icons);

public:
	MyCDListCtrl(UiDiskFrame *parentframe, wxWindow *parent, wxWindowID id,
		const struct st_list_columns *columns,
		Config *ini,
		long style,
		wxDataViewModel *model=NULL,
		const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	virtual ~MyCDListCtrl();

//	/// カラム幅を変えた
//	void OnColumnEndDrag(MyCDListEvent& event);
	/// カラムを入れ替え
	void OnColumnReordered(MyCDListEvent& event);
	/// カラムをソート
	void OnColumnSorted(MyCDListEvent& event);

	/// リストにカラムを設定する
	void InsertListColumns();
	/// カラムを削除
	void DeleteAllListColumns();

	/// 初期 カラムを設定
	void InsertListColumn(int col);
	/// カラムを挿入
	void InsertListColumn(int col, int idx, MyCDListColumn *c);
	/// カラムの幅
	int  GetListColumnWidth(int col) const;
	/// カラムを削除
	void DeleteListColumn(int col);

	/// データを挿入
	void InsertListItem(long row, MyCDListValue *values, size_t count, wxUIntPtr data);
	/// データを更新
	void UpdateListItem(long row, MyCDListValue *values, size_t count, wxUIntPtr data);

//	int GetDirItemPos(const MyCDListItem &item) const;
	/// ファイル名テキストを設定
	void SetListText(const MyCDListItem &item, int idx, const wxString &text);
	/// 選択している行の位置を返す
	int  GetListSelectedRow() const;
	/// アイテム位置を返す
	int  GetListSelectedNum() const;
	/// 選択している行数
	int  GetListSelectedItemCount() const;
	/// 選択している行アイテムを得る
	MyCDListItem GetListSelection() const;
	/// 選択している行アイテムを得る
	int  GetListSelections(MyCDListItems &arr) const;
	/// 全行を選択
	void SelectAllListItem();
	/// 行アイテムを選択
	void SelectListItem(const MyCDListItem &item);
	/// 行を選択
	void SelectListRow(int row);
	/// 全て非選択にする
	void UnselectAllListItem();
	/// 非選択にする
	void UnselectListItem(const MyCDListItem &item);
	/// 指定した行が選択しているか
	int  GetListSelected(int row) const;
	/// 指定した行が選択しているか
	void SetListSelected(int row, int val);
	/// フォーカスしている行アイテムを得る
	MyCDListItem GetListFocusedItem() const;
	/// 行アイテムをフォーカス
	void FocusListItem(const MyCDListItem &item);
	/// アイテムを編集
	void EditListItem(const MyCDListItem &item);
	/// リストを削除
	bool DeleteAllListItems();
	/// アイテムの固有データを返す
	wxUIntPtr GetListItemData(const MyCDListItem &item) const;
	/// アイテムの固有データを返す
	wxUIntPtr GetListItemDataByRow(long row) const;

	/// カラムヘッダのタイトルを返す
	const wxString &GetColumnText(int idx) const;
	/// カラム表示中か
	bool ColumnIsShown(int idx) const;
	/// カラムの表示を変更
	bool ShowColumn(int idx, bool show);
	/// 表示位置のカラム情報を返す
	MyCDListColumn *FindColumn(int col, int *n_idx, bool all = false) const;
	/// 表示位置のカラム情報を返す
	MyCDListColumn *FindColumn(wxDataViewColumn *col, int *n_idx, bool all = false) const;

//	/// カラム用のポップアップメニューを作成する
//	void CreateColumnPopupMenu(wxMenu* &menu, int menu_id, int menu_detail_id);
	/// カラム情報を現在の並び順で返す
	void GetListColumnsByCurrentOrder(MyCDListColumns &items) const;
	/// カラム入れ替えダイアログを表示
	bool ShowListColumnRearrangeBox();

	/// カラム番号ソート用
	static int SortByColumn(MyCDListColumn **i1, MyCDListColumn **i2);

	/// 現在のカラム位置を再取得
	void ReorderColumns();

	/// 指定した座標に行アイテムがあるか
	bool HasItemAtPoint(int x, int y) const;
	/// 指定した座標にある行アイテムを返す
	MyCDListItem GetItemAtPoint(int x, int y) const;
};

//////////////////////////////////////////////////////////////////////

/// カラム入れ替えボックス
class MyCDListRearrangeBox : public wxRearrangeDialog
{
public:
	MyCDListRearrangeBox(MyCDListCtrl *parent, const wxArrayInt &order, const wxArrayString &items);

};

#endif /* _UICDLISTCTRL_H_ */

