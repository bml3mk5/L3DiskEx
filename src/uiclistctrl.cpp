/// @file uiclistctrl.cpp
///
/// @brief リストコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uiclistctrl.h"
#include <wx/imaglist.h>
#include <wx/icon.h>
#include "config.h"
#include "uicommon.h"


//////////////////////////////////////////////////////////////////////
//
//
//
L3CListValue::L3CListValue() : wxListItem()
{
	SetMask(wxLIST_MASK_TEXT);
}
void L3CListValue::Set(long n_row, int n_icon, const wxString &n_value)
{
	SetId((int)n_row);
	SetImage(n_icon);
	SetText(n_value);
	SetMask(wxLIST_MASK_IMAGE | wxLIST_MASK_TEXT);
}
void L3CListValue::Set(long n_row, const wxString &n_value)
{
	SetId((int)n_row);
	SetText(n_value);
}

//////////////////////////////////////////////////////////////////////
//
// リストのカラム情報
//
L3CListColumn::L3CListColumn(int n_idx, const struct st_list_columns *n_info, int n_width)
{
	Set(n_idx, n_info, n_width);
}

void L3CListColumn::Set(int n_idx, const struct st_list_columns *n_info, int n_width)
{
	idx = n_idx;
	col = n_idx;
	info = n_info;
	width = n_width;
	sort_dir = 0;
	label = wxGetTranslation(n_info->label);
}

bool L3CListColumn::HaveIcon() const
{
	return info->have_icon;
}

wxListColumnFormat L3CListColumn::GetAlign() const
{
	return (info->align == wxALIGN_RIGHT ? wxLIST_FORMAT_RIGHT : (info->align == wxALIGN_CENTER ? wxLIST_FORMAT_CENTER : wxLIST_FORMAT_LEFT));
}

bool L3CListColumn::IsSortable() const
{
	return info->sortable;
}

#ifdef USE_VIRTUAL_ON_LIST_CTRL
//////////////////////////////////////////////////////////////////////
//
// リストコントロール内の1行データ
//
L3CListRow::L3CListRow()
{
	m_data = 0;
}
L3CListRow::L3CListRow(L3CListValue *n_values, size_t n_count, wxUIntPtr n_data)
{
	Set(n_values, n_count, n_data);
}
L3CListRow::~L3CListRow()
{
	for(size_t i=0; i<m_values.Count(); i++) {
		delete m_values.Item(i);
	}
}
void L3CListRow::Set(L3CListValue *n_values, size_t n_count, wxUIntPtr n_data)
{
	for(size_t i=0; i<n_count; i++) {
		L3CListValue *val = new L3CListValue(n_values[i]);
		m_values.Add(val);
	}
	m_data = n_data;
}
void L3CListRow::Replace(L3CListValue *n_values, size_t n_count, wxUIntPtr n_data)
{
	for(size_t i=0; i<n_count; i++) {
		L3CListValue *val = m_values[i];
		if (val) *val = n_values[i];
	}
	m_data = n_data;
}
void L3CListRow::Clear()
{
	for(size_t i=0; i<m_values.Count(); i++) {
		delete m_values.Item(i);
	}
	m_values.Empty();
}
size_t L3CListRow::Count() const
{
	return m_values.Count();
}
L3CListValue *L3CListRow::Item(size_t idx) const
{
	return m_values.Item(idx);
}
int L3CListRow::GetImage() const
{
	int image = -1;
	for(size_t idx=0; idx<m_values.Count(); idx++) {
		int i = m_values.Item(idx)->GetImage();
		if (i >= 0) {
			image = i;
			break;
		}
	}
	return image;
}
int L3CListRow::GetImage(size_t idx) const
{
	return m_values.Item(idx)->GetImage();
}
const wxString &L3CListRow::GetText(size_t idx) const
{
	return m_values.Item(idx)->GetText();
}

//
// リストコントロール内の全行データ
//
L3CListRows::L3CListRows() : ArrayOfL3CListRow()
{
}
L3CListRows::~L3CListRows()
{
	ClearAll();
}
void L3CListRows::Insert(L3CListValue *n_values, size_t n_count, wxUIntPtr n_data)
{
	Add(new L3CListRow(n_values, n_count, n_data));
}
void L3CListRows::Set(long row, L3CListValue *n_values, size_t n_count, wxUIntPtr n_data)
{
	if (row >= (long)Count()) return;

	L3CListRow *item = Item(row);
	item->Replace(n_values, n_count, n_data);
}
void L3CListRows::ClearAll()
{
	for(size_t i=0; i<Count(); i++) {
		delete Item(i);
	}
	Empty();
}

static wxListCtrlCompare s_fnSortCallBack;
static wxIntPtr			 s_sortData;

bool L3CListRows::SortItems(wxListCtrlCompare fnSortCallBack, wxIntPtr sortData)
{
	s_fnSortCallBack = fnSortCallBack;
	s_sortData = sortData;

	Sort(&Compare);

	return true;
}
int L3CListRows::Compare(L3CListRow **item1, L3CListRow **item2)
{
	int i1 = (int)(*item1)->GetData();
	int i2 = (int)(*item2)->GetData();
	return s_fnSortCallBack(i1, i2, s_sortData);
}
#endif

//////////////////////////////////////////////////////////////////////
//
// リストコントロール
//
/// @param [in] parentframe 親フレーム
/// @param [in] parent      親コントロール
/// @param [in] id          ID
/// @param [in] columns     カラム情報
/// @param [in] icon_sort_down ソートアイコン
/// @param [in] icon_sort_up   ソートアイコン
/// @param [in] ini         設定データ
/// @param [in] style       スタイル
/// @param [in] pos         位置
/// @param [in] size        サイズ
L3CListCtrl::L3CListCtrl(L3DiskFrame *parentframe, wxWindow *parent, wxWindowID id,
	const struct st_list_columns *columns,
	int icon_sort_down, int icon_sort_up,
	Config *ini,
	int style,
	const wxPoint &pos, const wxSize &size)
	: wxListCtrl(parent, id, pos, size, style | wxLC_REPORT
#ifdef USE_VIRTUAL_ON_LIST_CTRL
	| wxLC_VIRTUAL
#endif
) {
	this->frame = parentframe;
	this->m_ini = ini;
	this->m_icon_sort_down = icon_sort_down;
	this->m_icon_sort_up = icon_sort_up;

#ifdef USE_VIRTUAL_ON_LIST_CTRL
	// ストライプ
	EnableAlternateRowColours(true);
#endif

	// カラムデータの設定
	for(int idx=0; columns[idx].name != NULL; idx++) {
		const struct st_list_columns *c = &columns[idx];
		int w =	ini ? ini->GetListColumnWidth(idx) : -1;
		m_columns.Add(new L3CListColumn(idx, c, w >= 0 ? w : c->width));
		m_indexes.Add(-1);
	}

	// カラム位置の設定
	int column_count = (int)m_columns.Count();

	m_idOnFirstColumn = 0;
	for(int idx = 0; idx < column_count; idx++) {
		int col = ini ? ini->GetListColumnPos(idx) : idx;

		if (idx == 0 && col < 0) col = 0;

		m_columns[idx]->SetColumn(col);
	}

	// カラムを設定する
	InsertListColumns();

	// event handler
//	Bind(wxEVT_LIST_COL_END_DRAG, &L3CListCtrl::OnColumnEndDrag, this);

}
L3CListCtrl::~L3CListCtrl()
{
	// カラム幅を取得
	for(int col = 0; col < (int)GetColumnCount(); col++) {
		L3CListColumn *column = FindColumn(col, NULL);
		if (column) {
			column->SetWidth(GetListColumnWidth(col));
		}
	}

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
	// wxWSNでは、カラム順序を内部で管理しているので、
	// 内部から順序を得る
	wxArrayInt order = GetColumnsOrder();
	wxArrayInt new_col(m_columns.Count());
	for(int idx = 0; idx < (int)new_col.Count(); idx++) {
		new_col[idx] = -1;
	}
	for(int col = 0; col < (int)order.Count(); col++) {
		L3CListColumn *column = FindColumn(order[col], NULL);
		if (column) {
			new_col[column->GetIndex()] = col;
		}
	}
	for(int idx = 0; idx < (int)new_col.Count(); idx++) {
		m_columns[idx]->SetColumn(new_col[idx]);
	}
#endif

	if (m_ini) {
		for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
			m_ini->SetListColumnWidth(idx, m_columns[idx]->GetWidth());
			m_ini->SetListColumnPos(idx, m_columns[idx]->GetColumn());
		}
	}

	for(size_t idx=0; idx<m_columns.Count(); idx++) {
		delete m_columns[idx];
	}
}

/// アイコンを設定
void L3CListCtrl::AssignListIcons(const char ***icons)
{
	wxImageList *ilist = new wxImageList( 16, 16 );
	for(size_t idx=0; icons[idx] != NULL; idx++) {
		ilist->Add( wxIcon(icons[idx]) );
	}
	AssignImageList( ilist, wxIMAGE_LIST_SMALL );
}

#if 0
/// カラム幅を変えた
void L3CListCtrl::OnColumnEndDrag(L3CListEvent& event)
{
	int col = event.GetColumn();
	L3DiskFileListColumn *column = FindColumn(col, NULL);
	if (column) {
		column->SetWidth(GetColumnWidth(col));
	}
}
#endif

/// カラムをソート
void L3CListCtrl::OnColumnSorted(L3CListEvent& event)
{
	int col = event.GetColumn();
	L3CListColumn *column = FindColumn(col, NULL);
	if (column) {
	}
}

/// リストにカラムを設定する
void L3CListCtrl::InsertListColumns()
{
	int column_count = (int)m_columns.Count();

	for(size_t idx = 0; idx < m_indexes.Count(); idx++) {
		m_indexes[idx] = -1;
	}

	L3CListColumns arr;
	for(int idx = 0; idx < column_count; idx++) {
		arr.Add(m_columns[idx]);
	}
	arr.Sort(&SortByColumn);
	int n_col = 0;
	for(size_t idx = 0; idx < arr.Count(); idx++) {
		L3CListColumn *column = arr.Item(idx);
		if (column->GetColumn() >= 0) {
			int n_idx = column->GetIndex();
			column->SetColumn(n_col);
			m_indexes[n_col]=n_idx;
			if (n_col == 0) {
				m_idOnFirstColumn = n_idx;
			}
			n_col++;
		}
	}

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
	// wxMSWでは、カラム順序を内部で管理しているので、
	// 配列に順序を記録して、アイテムとしてはindex順にする。
	wxArrayInt order(n_col);
	n_col = 0;
	for(int idx = 0; idx < column_count; idx++) {
		L3CListColumn *column = m_columns[idx];
		int col = column->GetColumn();
		if (col >= 0) {
			order[col] = n_col;
			int n_idx = column->GetIndex();
			column->SetColumn(n_col);
			m_indexes[n_col]=n_idx;
			if (n_col == 0) {
				m_idOnFirstColumn = n_idx;
			}
			n_col++;
		}
	}
#endif

	// リストにカラムを設定
	for(int col = 0; col < column_count; col++) {
		InsertListColumn(col);
	}

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
	// wxMSWでは、カラム順序を内部で管理しているので、
	// 内部に順序を教える
	SetColumnsOrder(order);
#endif
}

/// 初期 カラムを設定
void L3CListCtrl::InsertListColumn(int col)
{
	L3CListColumn *c = FindColumn(col, NULL);
	if (!c) return;

	InsertListColumn(col, c->GetIndex(), c);
}

/// カラムを挿入
void L3CListCtrl::InsertListColumn(int col, int idx, L3CListColumn *c)
{
	InsertColumn(col, c->GetText(), c->GetAlign(), -1);
	SetColumnWidth(col, c->GetWidth());
//	c->SetId(item_id);
}

/// カラムの幅
int L3CListCtrl::GetListColumnWidth(int col) const
{
	return GetColumnWidth(col);
}

/// カラムを削除
void L3CListCtrl::DeleteListColumn(int col)
{
	DeleteColumn(col);
}

/// データを挿入
void L3CListCtrl::InsertListItem(long row, L3CListValue *values, size_t count, wxUIntPtr data)
{
#ifndef USE_VIRTUAL_ON_LIST_CTRL
	InsertItem(values[m_idOnFirstColumn]);
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		if (m_columns[idx]->GetColumn() >= 0 && idx != m_idOnFirstColumn) {
			values[idx].SetColumn(m_columns[idx]->GetColumn());
			SetItem(values[idx]);
		}
	}
	SetItemPtrData(row, data);
#else
	m_rows.Insert(values, count, data);
#endif
}

/// データを更新
void L3CListCtrl::UpdateListItem(long row, L3CListValue *values, size_t count, wxUIntPtr data)
{
#ifndef USE_VIRTUAL_ON_LIST_CTRL
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		if (m_columns[idx]->GetColumn() >= 0) {
			values[idx].SetColumn(m_columns[idx]->GetColumn());
			SetItem(values[idx]);
		}
	}
	SetItemPtrData(row, data);
#else
	m_rows.Set(row, values, count, data);
#endif
}

#if 0
/// ディレクトリアイテムの位置を返す
int L3CListCtrl::GetDirItemPos(const L3CListItem &item) const
{
	return (int)GetListItemData(item);
}
#endif

/// ファイル名テキストを設定
/// @param [in] item リストアイテム
/// @param [in] idx  カラムの位置
/// @param [in] text ファイル名
void L3CListCtrl::SetListText(const L3CListItem &item, int idx, const wxString &text)
{
	SetItem(item, m_columns[idx]->GetColumn(), text);
}

/// 選択している行の位置を返す
/// @return 複数行選択している場合 wxNOT_FOUND
int L3CListCtrl::GetListSelectedRow() const
{
	return (int)GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}
/// アイテム位置を返す
int L3CListCtrl::GetListSelectedNum() const
{
	int row = GetListSelectedRow();
	if (row != wxNOT_FOUND) {
		row = (int)GetListItemData(row);
	}
	return row;
}
/// 選択している行数
int L3CListCtrl::GetListSelectedItemCount() const
{
	return GetSelectedItemCount();
}
/// 選択行を得る
L3CListItem L3CListCtrl::GetListSelection() const
{
	return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
}
/// 選択している行アイテムを得る
int L3CListCtrl::GetListSelections(L3CListItems &arr) const
{
	long item = -1;
	arr.Empty();
	for(;;) {
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1) break;
		arr.Add(item);
	}
	return (int)arr.Count();
}
/// 全行を選択
void L3CListCtrl::SelectAllListItem()
{
	for(int row=0; row<GetItemCount(); row++) {
		SetItemState((long)row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}
/// 行アイテムを選択
void L3CListCtrl::SelectListItem(const L3CListItem &item)
{
	SetItemState(item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}
/// 行を選択
void L3CListCtrl::SelectListRow(int row)
{
	SetItemState((long)row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}
/// 全て非選択にする
void L3CListCtrl::UnselectAllListItem()
{
	long item = -1;
	for(;;) {
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1) break;
		SetItemState(item, 0, wxLIST_STATE_SELECTED);
	}
}
/// 非選択にする
void L3CListCtrl::UnselectListItem(const L3CListItem &item)
{
	SetItemState(item, 0, wxLIST_STATE_SELECTED);
}
/// フォーカスしている行アイテムを得る
L3CListItem L3CListCtrl::GetListFocusedItem() const
{
	return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_FOCUSED);
}
/// 行アイテムをフォーカス
void L3CListCtrl::FocusListItem(const L3CListItem &item)
{
	SetItemState(item, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
}

/// アイテムを編集
void L3CListCtrl::EditListItem(const L3CListItem &item)
{
	EditLabel(item);
}

/// リストを削除
bool L3CListCtrl::DeleteAllListItems()
{
#ifndef USE_VIRTUAL_ON_LIST_CTRL
	return DeleteAllItems();
#else
	m_rows.ClearAll();
	SetItemCount(0);
	return true;
#endif
}

/// アイテムの固有データを返す
wxUIntPtr L3CListCtrl::GetListItemData(const L3CListItem &item) const
{
#ifndef USE_VIRTUAL_ON_LIST_CTRL
	return GetItemData(item);
#else
	if (m_rows.Count() <= (size_t)item) return 0;

	return m_rows.Item(item)->GetData();
#endif
}

/// アイテムの固有データを返す
wxUIntPtr L3CListCtrl::GetListItemDataByRow(long row) const
{
	return GetListItemData(row);
}

/// カラムヘッダのタイトルを返す
const wxString &L3CListCtrl::GetColumnText(int idx) const
{
	return m_columns[idx]->GetText();
}

/// カラム表示中か
bool L3CListCtrl::ColumnIsShown(int idx) const
{
	return m_columns[idx]->GetColumn() >= 0;
}

/// カラムの表示を変更
/// @return true:リスト更新が必要
bool L3CListCtrl::ShowColumn(int idx, bool show)
{
	int col = m_columns[idx]->GetColumn();
	if (col >= 0 && !show) {
		// 非表示
		DeleteListColumn(col);
		m_columns[idx]->SetColumn(-col-1);
		m_indexes[col]=-col-1;
		// カラム位置の再設定
		for(int d_idx = 0; d_idx < (int)m_columns.Count(); d_idx++) {
			int d_col = m_columns[d_idx]->GetColumn();
			if (col < d_col) {
				d_col--;
				m_columns[d_idx]->SetColumn(d_col);
				m_indexes[d_col]=d_idx;
				if (d_col == 0) {
					m_idOnFirstColumn = d_idx;
				}
			}
		}
		return false;

	} else if (col < 0 && show) {
		// 表示
#ifndef wxHAS_LISTCTRL_COLUMN_ORDER
		// 最後のカラムに追加
		col = GetColumnCount();
#else
		// カラム位置の再設定
		col = 0;
		for(int d_idx = 0; d_idx < (int)m_columns.Count(); d_idx++) {
			int d_col = m_columns[d_idx]->GetColumn();
			if (d_col == 0) {
				m_idOnFirstColumn = d_idx;
			}
			if (d_idx < idx) {
				if (d_col >= 0) col = d_col;
			} else if (idx == d_idx) {
				col++;
			} else if (idx < d_idx) {
				if (d_col >= 0) {
					d_col++;
					m_columns[d_idx]->SetColumn(d_col);
					m_indexes[d_col]=d_idx;
				}
			}
		}
#endif
		InsertListColumn(col, idx, m_columns[idx]);
		m_columns[idx]->SetColumn(col);
		m_indexes[col]=idx;

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
		// wxMSWでは、カラム順序を内部で管理しているので、
		// 内部順序を変更する 一番後ろにする
		wxArrayInt order = GetColumnsOrder();
		int count = (int)order.Count() - 1;
		for(int ord = col; ord < count; ord++) {
			// left shift
			order[ord] = order[ord + 1];
		}
		order[count] = col;
		SetColumnsOrder(order);
#endif
		return true;
	}
	return false;
}

/// 表示位置のカラム情報を返す
L3CListColumn *L3CListCtrl::FindColumn(int col, int *n_idx) const
{
	L3CListColumn *match = NULL;
#if 0
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		if (m_columns[idx]->GetColumn() == col) {
			match = m_columns[idx];
			if (n_idx) *n_idx = idx;
			break;
		}
	}
#else
	if (0 <= col && col < (int)m_columns.Count()) {
		int idx = m_indexes[col];
		if (idx >= 0) {
			match = m_columns[idx];
			if (n_idx) *n_idx = idx;
		}
	}
#endif
	return match;
}

/// カラム用のポップアップメニューを作成する
void L3CListCtrl::CreateColumnPopupMenu(wxMenu* &menu, int menu_id, int menu_detail_id)
{
	// メニューアイテムを削除
	if (menu) delete menu;
	menu = new wxMenu;

#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
	// wxMSWでは、カラム順序を内部で管理しているので、
	// 内部順序を得る
	wxArrayInt order = GetColumnsOrder();
	// 表示中のカラム
	for(int ord = 0; ord < (int)order.Count(); ord++) {
		int col = order[ord];
#else
	// 表示中のカラム
	for(int col = 0; col < (int)m_columns.Count(); col++) {
#endif
		int idx = -1;
		L3CListColumn *column = FindColumn(col, &idx);
		if (column) {
			wxMenuItem *mitem = menu->AppendCheckItem(menu_id + idx, column->GetText());
			mitem->Check(true);
			mitem->Enable(idx != 0);
		}
	}
	// 非表示のカラム
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		L3CListColumn *column = m_columns[idx];
		if (column->GetColumn() < 0) {
			wxMenuItem *mitem = menu->AppendCheckItem(menu_id + idx, column->GetText());
			mitem->Check(false);
		}
	}
	// 詳細
	menu->AppendSeparator();
	menu->Append(menu_detail_id, _("Detail..."));
}

/// カラムの表示位置を返す
void L3CListCtrl::GetListColumnsByCurrentOrder(L3CListColumns &items) const
{
#ifdef wxHAS_LISTCTRL_COLUMN_ORDER
	// wxMSWでは、カラム順序を内部で管理しているので、
	// 内部順序を得る
	wxArrayInt iorder = GetColumnsOrder();
	// 表示中のカラム
	for(int ord = 0; ord < (int)iorder.Count(); ord++) {
		int col = iorder[ord];
#else
	// 表示中のカラム
	for(int col = 0; col < (int)m_columns.Count(); col++) {
#endif
		int idx = -1;
		L3CListColumn *column = FindColumn(col, &idx);
		if (column) {
			items.Add(column);
		}
	}
	// 非表示のカラム
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		L3CListColumn *column = m_columns[idx];
		if (column->GetColumn() < 0) {
			items.Add(column);
		}
	}
}

/// カラム入れ替えダイアログを表示
/// @return true: submitted  false: canceled
bool L3CListCtrl::ShowListColumnRearrangeBox()
{
	L3CListColumns items;
	GetListColumnsByCurrentOrder(items);
	wxArrayInt order;
	wxArrayString labels;
	for(int i=0; i<(int)items.Count(); i++) {
		order.Add((items[i]->GetColumn() >= 0 ? i : ~i));
		labels.Add(items[i]->GetText());
	}
	L3CListRearrangeBox dlg(this, order, labels);
	int sts = dlg.ShowModal();
	if (sts != wxID_OK) return false;

	// カラムを作り直す
	order = dlg.GetOrder();
	for(int i=0; i<(int)order.Count(); i++) {
		int ord = order[i];
		int idx = (ord >= 0 ? ord : ~ord);
		items[idx]->SetColumn(ord >= 0 ? i : ~i);
		if (items[idx]->GetIndex() == 0 && ord < 0) {
			items[idx]->SetColumn(~ord);
		}
	}
	// カラムを作り直す
	DeleteAllColumns();
	InsertListColumns();

	return true;
}

/// カラム番号ソート用
int L3CListCtrl::SortByColumn(L3CListColumn **i1, L3CListColumn **i2)
{
	int n1 = (*i1)->GetColumn();
	int n2 = (*i2)->GetColumn();
	if (n1 < 0) n1 = -n1-1;
	if (n2 < 0) n2 = -n2-1;

	int n = n1 - n2;
	if (n == 0) n = (*i1)->GetIndex() - (*i2)->GetIndex();
	return n;
}


/// ソート方向を返す
int L3CListCtrl::GetColumnSortDir(int idx) const
{
	return m_columns[idx]->GetSortDir();
}

/// ソート方向をセット
void L3CListCtrl::SetColumnSortDir(int idx, int dir)
{
	m_columns[idx]->SetSortDir(dir);
}

/// ソート方向を切替
int L3CListCtrl::ToggleColumnSortDir(int idx)
{
	int dir = m_columns[idx]->GetSortDir();
//	dir = (dir > 0 ? -1 : 1);
	dir++;
	if (dir > 1) dir = -1;
	m_columns[idx]->SetSortDir(dir);
	return dir;
}

#if defined(__WXMSW__)
#include <windows.h>
#include <commctrl.h>
#endif

/// ソート方向のアイコンを設定
void L3CListCtrl::SetColumnSortIcon(int idx)
{
	int dir = m_columns[idx]->GetSortDir();
	int col = m_columns[idx]->GetColumn();
	if (col < 0) return;

#if defined(__WXMSW__)
	// Windowsの場合、リストヘッダに三角アイコンを追加
	WXWidget hList = GetHandle();
	HWND hCol = ListView_GetHeader(hList);

	HDITEM hDitem;
	memset(&hDitem, 0, sizeof(hDitem));
	hDitem.mask = HDI_FORMAT;
	Header_GetItem(hCol, col, &hDitem);
	hDitem.fmt &= ~HDF_SORTDOWN & ~HDF_SORTUP;
	hDitem.fmt |= (dir > 0 ? HDF_SORTUP : (dir < 0 ? HDF_SORTDOWN : 0)) | HDF_STRING;
	Header_SetItem(hCol, col, &hDitem);
#else
	wxListItem item;
//	item.SetMask(wxLIST_MASK_IMAGE);
//	item.SetImage(dir > 0 ? m_icon_sort_up : (dir < 0 ? m_icon_sort_down : -1));
	item.SetMask(wxLIST_MASK_TEXT);
	item.SetText(m_columns[idx]->GetText() + (dir > 0 ? wxT("˄") : (dir < 0 ? wxT("˅") : wxT(""))));
	SetColumn(col, item);
#endif
}

/// ソートカラムを選択
int L3CListCtrl::SelectColumnSortDir(int &col, int &idx, bool &match_col)
{
	int dir = 0;
	idx = -1;

	// ソートカラムを決定
	L3CListColumn *c = FindColumn(col, &idx);
	match_col = c ? c->IsSortable() : false;

	if (col < 0) {
		// 既存のソートカラムをさがす
		for(int i=0; i<(int)m_columns.Count(); i++) {
			int n_dir = GetColumnSortDir(i);
			if (n_dir != 0) {
				col = m_columns[i]->GetColumn();
				dir = n_dir;
				break;
			}
		}
	} else if (match_col) {
		// ソート方向を変更
		dir = ToggleColumnSortDir(idx);
	}

	// 改めてソートカラムを決定
	c = FindColumn(col, &idx);
	match_col = c ? c->IsSortable() : false;
	if (col >= 0 && match_col) {
		// 選択したカラム以外のソート方向をクリア
		for(int i=0; i<(int)m_columns.Count(); i++) {
			if (m_columns[i]->GetColumn() != col && GetColumnSortDir(i) != 0) {
				SetColumnSortDir(i, 0);
				SetColumnSortIcon(i);
			}
		}
	}

	return dir;
}

#ifdef USE_VIRTUAL_ON_LIST_CTRL
// ソート
bool L3CListCtrl::SortItems(wxListCtrlCompare fnSortCallBack, wxIntPtr sortData)
{
	// ソート
	bool sts = m_rows.SortItems(fnSortCallBack, sortData);
	// リストをリフレッシュ
	Refresh();

	return sts;
}
#if 0
/// アイテムの属性を返す
wxListItemAttr *L3CListCtrl::OnGetItemAttr(long item) const
{
	return (wxListItemAttr *)&m_attr;
}
#endif
/// アイテムのアイコンを返す
int L3CListCtrl::OnGetItemImage(long item) const
{
	if (m_rows.Count() <= (size_t)item) return -1;

	return m_rows.Item(item)->GetImage();
}
/// アイテムのアイコンを返す
int L3CListCtrl::OnGetItemColumnImage(long item, long column) const
{
	if (m_rows.Count() <= (size_t)item) return -1;

	int idx = -1;
	L3CListColumn *c = FindColumn((int)column, &idx);

	return idx >= 0 ? m_rows.Item(item)->GetImage((size_t)idx) : -1;
}
/// アイテムの値を返す
wxString L3CListCtrl::OnGetItemText(long item, long column) const
{
	if (m_rows.Count() <= (size_t)item) return wxT("");

	int idx = -1;
	L3CListColumn *c = FindColumn((int)column, &idx);

	return idx >= 0 ? m_rows.Item(item)->GetText((size_t)idx) : wxT("");
}
#endif /* USE_VIRTUAL_ON_LIST_CTRL */

//////////////////////////////////////////////////////////////////////

//
//
//
L3CListRearrangeBox::L3CListRearrangeBox(L3CListCtrl *parent, const wxArrayInt &order, const wxArrayString &items)
	: wxRearrangeDialog(parent, _("Configure the columns shown:"), _("Arrange Column Order"), order, items) 
{
}
