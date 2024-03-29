/// @file uiclistctrl.h
///
/// @brief リストコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef UICLISTCTRL_H
#define UICLISTCTRL_H

#include "../common.h"
#include <wx/listctrl.h>
#include <wx/dynarray.h>
#include <wx/rearrangectrl.h>
#include "uicommon.h"


//#define USE_VIRTUAL_ON_LIST_CTRL

#define MyCListItem			long
#define MyCListItems		wxArrayLong
#define MyCListEvent		wxListEvent

class wxMenu;
class UiDiskFrame;
class Config;

//////////////////////////////////////////////////////////////////////

/// リストコントロール内の値
class MyCListValue : public wxListItem
{
public:
	MyCListValue();

	void Set(long n_row, int n_icon, const wxString &n_value);
	void Set(long n_row, const wxString &n_value);
};

WX_DEFINE_ARRAY(MyCListValue *, MyCListValues);

//////////////////////////////////////////////////////////////////////

/// リストのカラム情報
class MyCListColumn
{
protected:
	int idx;
	int col;
	const struct st_list_columns *info;
	wxString label;
	int width;
	int sort_dir;
public:
	MyCListColumn(int n_idx, const struct st_list_columns *n_info, int n_width);

	void	Set(int n_idx, const struct st_list_columns *n_info, int n_width);
	int		GetIndex() const { return idx; }
	int		GetColumn() const { return col; }
	void	SetColumn(int val) { col = val; }
	bool	HaveIcon() const;
	int		GetWidth() const { return width; }
	void	SetWidth(int val) { width = val; }
	const wxString &GetText() const { return label; }
	wxListColumnFormat GetAlign() const;
	bool	IsSortable() const;
	int		GetSortDir() const { return sort_dir; }
	void	SetSortDir(int val) { sort_dir = val; }
};

WX_DEFINE_ARRAY(MyCListColumn *, MyCListColumns);

//////////////////////////////////////////////////////////////////////

#ifdef USE_VIRTUAL_ON_LIST_CTRL
/// リストコントロール内の1行データ
class MyCListRow
{
private:
	MyCListValues m_values;
	wxUIntPtr m_data;

public:
	MyCListRow();
	MyCListRow(MyCListValue *n_values, size_t n_count, wxUIntPtr n_data);
	~MyCListRow();

	void Set(MyCListValue *n_values, size_t n_count, wxUIntPtr n_data);
	void Replace(MyCListValue *n_values, size_t n_count, wxUIntPtr n_data);
	void Clear();
	size_t Count() const;
	MyCListValue *Item(size_t idx) const;
	int GetImage() const;
	int GetImage(size_t idx) const;
	const wxString &GetText(size_t idx) const;
	wxUIntPtr GetData() const { return m_data; }
};

WX_DEFINE_ARRAY(MyCListRow *, ArrayOfMyCListRow);

/// リストコントロール内の全行データ
class MyCListRows : public ArrayOfMyCListRow
{
private:
	static int Compare(MyCListRow **item1, MyCListRow **item2); 
public:
	MyCListRows();
	~MyCListRows();

	void Insert(MyCListValue *n_values, size_t n_count, wxUIntPtr n_data);
	void Set(long row, MyCListValue *n_values, size_t n_count, wxUIntPtr n_data);
	void ClearAll();
	bool SortItems(wxListCtrlCompare fnSortCallBack, wxIntPtr sortData);
};
#endif

//////////////////////////////////////////////////////////////////////

/// リストコントロール
class MyCListCtrl : public wxListCtrl
{
protected:
	UiDiskFrame		*frame;
	MyCListColumns	 m_columns;
	wxArrayInt		 m_indexes;
	int				 m_idOnFirstColumn;
	int				 m_icon_sort_down;
	int				 m_icon_sort_up;
	Config			*m_ini;
#ifdef USE_VIRTUAL_ON_LIST_CTRL
	MyCListRows		 m_rows;
#endif

	/// アイコンを設定
	void AssignListIcons(const char ***icons);

public:
	MyCListCtrl(UiDiskFrame *parentframe, wxWindow *parent, wxWindowID id,
		const struct st_list_columns *columns,
		int icon_sort_down, int icon_sort_up,
		Config *ini,
		int style,
		const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	virtual ~MyCListCtrl();

//	/// カラム幅を変えた
//	void OnColumnEndDrag(MyCListEvent& event);
	/// カラムをソート
	void OnColumnSorted(MyCListEvent& event);

	/// リストにカラムを設定する
	void InsertListColumns();

	/// 初期 カラムを設定
	void InsertListColumn(int col);
	/// カラムを挿入
	void InsertListColumn(int col, int idx, MyCListColumn *c);
	/// カラムの幅
	int  GetListColumnWidth(int col) const;
	/// カラムを削除
	void DeleteListColumn(int col);

	/// データを挿入
	void InsertListItem(long row, MyCListValue *values, size_t count, wxUIntPtr data);
	/// データを更新
	void UpdateListItem(long row, MyCListValue *values, size_t count, wxUIntPtr data);

//	int GetDirItemPos(const MyCListItem &item) const;
	/// ファイル名テキストを設定
	void SetListText(const MyCListItem &item, int idx, const wxString &text);
	/// 選択している行の位置を返す
	int  GetListSelectedRow() const;
	/// アイテム位置を返す
	int  GetListSelectedNum() const;
	/// 選択している行数
	int  GetListSelectedItemCount() const;
	/// 選択している行アイテムを得る
	MyCListItem GetListSelection() const;
	/// 選択している行アイテムを得る
	int  GetListSelections(MyCListItems &arr) const;
	/// 全行を選択
	void SelectAllListItem();
	/// 行アイテムを選択
	void SelectListItem(const MyCListItem &item);
	/// 行を選択
	void SelectListRow(int row);
	/// 全て非選択にする
	void UnselectAllListItem();
	/// 非選択にする
	void UnselectListItem(const MyCListItem &item);
	/// フォーカスしている行アイテムを得る
	MyCListItem GetListFocusedItem() const;
	/// 行アイテムをフォーカス
	void FocusListItem(const MyCListItem &item);
	/// アイテムを編集
	void EditListItem(const MyCListItem &item);
	/// リストを削除
	bool DeleteAllListItems();
	/// アイテムの固有データを返す
	wxUIntPtr GetListItemData(const MyCListItem &item) const;
	/// アイテムの固有データを返す
	wxUIntPtr GetListItemDataByRow(long row) const;

	/// カラムヘッダのタイトルを返す
	const wxString &GetColumnText(int idx) const;
	/// カラム表示中か
	bool ColumnIsShown(int idx) const;
	/// カラムの表示を変更
	bool ShowColumn(int idx, bool show);
	/// 表示位置のカラム情報を返す
	MyCListColumn *FindColumn(int col, int *n_idx) const;

//	/// カラム用のポップアップメニューを作成する
//	void CreateColumnPopupMenu(wxMenu* &menu, int menu_id, int menu_detail_id);
	/// カラム情報を現在の並び順で返す
	void GetListColumnsByCurrentOrder(MyCListColumns &items) const;
	/// カラム入れ替えダイアログを表示
	bool ShowListColumnRearrangeBox();

	/// カラム番号ソート用
	static int SortByColumn(MyCListColumn **i1, MyCListColumn **i2);

	/// ソート方向を返す
	int  GetColumnSortDir(int idx) const;
	/// ソート方向をセット
	void SetColumnSortDir(int idx, int dir);
	/// ソート方向を切替
	int  ToggleColumnSortDir(int idx);
	/// ソート方向のアイコンを設定
	void SetColumnSortIcon(int idx);

	/// ソートカラムを選択
	int  SelectColumnSortDir(int &col, int &idx, bool &match_col);

	/// 指定した座標に行アイテムがあるか
	bool HasItemAtPoint(int x, int y) const;
	/// 指定した座標にある行アイテムを返す
	MyCListItem GetItemAtPoint(int x, int y) const;

#ifdef USE_VIRTUAL_ON_LIST_CTRL
	// ソート
	bool SortItems(wxListCtrlCompare fnSortCallBack, wxIntPtr sortData);
//	/// アイテムの属性を返す
//	wxListItemAttr *OnGetItemAttr(long item) const;
	/// アイテムのアイコンを返す
	int OnGetItemImage(long item) const;
	/// アイテムのアイコンを返す
	int OnGetItemColumnImage(long item, long column) const;
	/// アイテムの値を返す
	wxString OnGetItemText(long item, long column) const;
#endif
};

//////////////////////////////////////////////////////////////////////

/// カラム入れ替えボックス
class MyCListRearrangeBox : public wxRearrangeDialog
{
public:
	MyCListRearrangeBox(MyCListCtrl *parent, const wxArrayInt &order, const wxArrayString &items);

};

#endif /* UICLISTCTRL_H */

