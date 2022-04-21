/// @file uicdlistctrl.cpp
///
/// @brief データ―ビューリストコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uicdlistctrl.h"
#include "config.h"
#include "uicommon.h"


//////////////////////////////////////////////////////////////////////
//
//
//
L3CDListValue::L3CDListValue()
{
	row = 0;
	icon = -1;
}
void L3CDListValue::Set(long n_row, int n_icon, const wxString &n_value)
{
	row = (int)n_row;
	icon = n_icon;
	value = n_value;
}
void L3CDListValue::Set(long n_row, const wxString &n_value)
{
	row = (int)n_row;
	value = n_value;
}

//////////////////////////////////////////////////////////////////////
//
// リストのカラム情報
//
L3CDListColumn::L3CDListColumn(int n_idx, const struct st_list_columns *n_info, int n_width)
{
	Set(n_idx, n_info, n_width);
}

void L3CDListColumn::Set(int n_idx, const struct st_list_columns *n_info, int n_width)
{
	idx = n_idx;
	col = n_idx;
	info = n_info;
	width = n_width;
	sort_dir = 0;
	label = wxGetTranslation(n_info->label);
}

bool L3CDListColumn::HaveIcon() const
{
	return info->have_icon;
}

wxAlignment L3CDListColumn::GetAlign() const
{
	return info->align;
}

bool L3CDListColumn::IsSortable() const
{
	return info->sortable;
}

//////////////////////////////////////////////////////////////////////
//
// データ―ビューリストコントロール
//
/// @param [in] parentframe 親フレーム
/// @param [in] parent      親コントロール
/// @param [in] id          ID
/// @param [in] columns     カラム情報
/// @param [in] ini         設定データ
/// @param [in] style       スタイル
/// @param [in] model       モデル
/// @param [in] pos         位置
/// @param [in] size        サイズ
L3CDListCtrl::L3CDListCtrl(L3DiskFrame *parentframe, wxWindow *parent, wxWindowID id,
	const struct st_list_columns *columns,
	Config *ini,
	long style,
	wxDataViewModel *model,
	const wxPoint &pos, const wxSize &size)
	: wxDataViewListCtrl(parent, id, pos, size, wxDV_ROW_LINES | wxDV_MULTIPLE
) {
	this->frame = parentframe;
	this->m_ini = ini;

	if (model) {
		AssociateModel(model);
		model->DecRef();
	}

	// カラムデータの設定
	for(int idx=0; columns[idx].name != NULL; idx++) {
		const struct st_list_columns *c = &columns[idx];
		int w =	ini ? ini->GetListColumnWidth(idx) : -1;
		m_columns.Add(new L3CDListColumn(idx, c, w >= 0 ? w : c->width));
	}

	// カラム位置の設定
	int column_count = (int)m_columns.Count();

	m_idOnFirstColumn = 0;
	for(int idx = 0; idx < column_count; idx++) {
		int col = ini ? ini->GetListColumnPos(idx) : idx;

		if (col < 0) col = -1;
		if (column_count <= col) col = (column_count-1);
		if (idx == 0 && col < 0) col = 0;

		m_columns[idx]->SetColumn(col);
	}

	// カラムを設定する
	InsertListColumns();
#if 0
	// カラム位置を振り直し
	L3CDListColumns arr;
	for(int idx = 0; idx < column_count; idx++) {
		arr.Add(m_columns[idx]);
	}
	arr.Sort(&SortByColumn);
	int n_col = 0;
	// 表示するカラムの順番を設定
	for(size_t idx = 0; idx < arr.Count(); idx++) {
		L3CDListColumn *column = arr.Item(idx);
		if (column->GetColumn() >= 0) {
			column->SetColumn(n_col);
			column->Shown(true);
			if (n_col == 0) {
				m_idOnFirstColumn = column->GetIndex();
			}
			n_col++;
		}
	}
	// 表示しないカラムの順番を設定
	for(size_t idx = 0; idx < arr.Count(); idx++) {
		L3CDListColumn *column = arr.Item(idx);
		if (column->GetColumn() < 0) {
			column->SetColumn(n_col);
			column->Shown(false);
			n_col++;
		}
	}

	// リストにカラムを設定
	for(int col = 0; col < column_count; col++) {
		InsertListColumn(col);
	}
	AppendTextColumn(wxT(""), wxDATAVIEW_CELL_INERT, 1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE );
#endif
	// event handler

	// カラムを入れ替えた
	// REORDERED event is implemented in wxWidgets 3.1.1 or higher
	Bind(wxEVT_DATAVIEW_COLUMN_REORDERED , &L3CDListCtrl::OnColumnReordered, this);
	// ソートした
	Bind(wxEVT_DATAVIEW_COLUMN_SORTED, &L3CDListCtrl::OnColumnSorted, this);
}
L3CDListCtrl::~L3CDListCtrl()
{
	// カラム幅を取得
	for(int col = 0; col < (int)GetColumnCount(); col++) {
		wxDataViewColumn *c = GetColumn(col);
		L3CDListColumn *column = FindColumn(c, NULL);
		if (column) {
			column->SetWidth(GetListColumnWidth(col));
		}
	}

	if (m_ini) {
		for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
			m_ini->SetListColumnWidth(idx, m_columns[idx]->GetWidth());
			m_ini->SetListColumnPos(idx, m_columns[idx]->GetColumn());
		}
	}

	for(size_t idx=0; idx<m_columns.Count(); idx++) {
		delete m_columns[idx];
	}
	for(size_t idx=0; idx<m_icons.Count(); idx++) {
		delete m_icons[idx];
	}
}

/// アイコンを設定
void L3CDListCtrl::AssignListIcons(const char ***icons)
{
	for(size_t idx=0; icons[idx] != NULL; idx++) {
		m_icons.Add(new wxIcon(icons[idx]));
	}
}

#if 0
/// カラム幅を変えた
void L3CDListCtrl::OnColumnEndDrag(L3CDListEvent& event)
{
	int col = event.GetColumn();
	L3DiskFileListColumn *column = FindColumn(col, NULL);
	if (column) {
		column->SetWidth(GetColumnWidth(col));
	}
}
#endif

/// カラムを入れ替え
/// @note implemented in wxWidgets 3.1.1 or higher
void L3CDListCtrl::OnColumnReordered(L3CDListEvent& event)
{
//	int col = 1;
}

/// カラムをソート
void L3CDListCtrl::OnColumnSorted(L3CDListEvent& event)
{
	int col = event.GetColumn();
	L3CDListColumn *column = FindColumn(col, NULL);
	if (column) {
	}
}

/// リストにカラムを設定する
void L3CDListCtrl::InsertListColumns()
{
	// カラム位置の設定
	int column_count = (int)m_columns.Count();

	// カラム位置を振り直し
	L3CDListColumns arr;
	for(int idx = 0; idx < column_count; idx++) {
		arr.Add(m_columns[idx]);
	}
	arr.Sort(&SortByColumn);
	int n_col = 0;
	// 表示するカラムの順番を設定
	for(size_t idx = 0; idx < arr.Count(); idx++) {
		L3CDListColumn *column = arr.Item(idx);
		if (column->GetColumn() >= 0) {
			column->SetColumn(n_col);
			if (n_col == 0) {
				m_idOnFirstColumn = column->GetIndex();
			}
			n_col++;
		}
	}
	// 表示しないカラムの順番を設定
	for(size_t idx = 0; idx < arr.Count(); idx++) {
		L3CDListColumn *column = arr.Item(idx);
		if (column->GetColumn() < 0) {
			column->SetColumn(~n_col);
			n_col++;
		}
	}

	// リストにカラムを設定
	for(int col = 0; col < column_count; col++) {
		InsertListColumn(col);
	}
	AppendTextColumn(wxT(""), wxDATAVIEW_CELL_INERT, 1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE );
}

/// カラムを削除
void L3CDListCtrl::DeleteAllListColumns()
{
	int count = (int)GetColumnCount();
	for(int col = count - 1; col >= 0; col--) {
		wxDataViewColumn *column = GetColumn((unsigned int)col);
		DeleteColumn(column);
	}
}

/// 初期 カラムを設定
void L3CDListCtrl::InsertListColumn(int col)
{
	L3CDListColumn *c = FindColumn(col, NULL, true);
	if (!c) return;

	InsertListColumn(col, c->GetIndex(), c);
}

/// カラムを挿入
void L3CDListCtrl::InsertListColumn(int col, int idx, L3CDListColumn *c)
{
	wxDataViewColumn *item_id;
	int flags = wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE;
	if (c->IsSortable()) flags |= wxDATAVIEW_COL_SORTABLE;
	if (c->HaveIcon()) {
		item_id = AppendIconTextColumn(c->GetText(), wxDATAVIEW_CELL_EDITABLE, c->GetWidth(), c->GetAlign(), flags );
	} else {
		item_id = AppendTextColumn(c->GetText(), wxDATAVIEW_CELL_INERT, c->GetWidth(), c->GetAlign(), flags );
	}
	c->SetId(item_id);
	item_id->SetHidden(c->GetColumn() < 0);
}

/// カラムの幅
int L3CDListCtrl::GetListColumnWidth(int col) const
{
	return GetColumn(col)->GetWidth();
}

/// カラムを削除
void L3CDListCtrl::DeleteListColumn(int col)
{
	DeleteColumn(GetColumn(col));
}

/// データを挿入
void L3CDListCtrl::InsertListItem(long row, L3CDListValue *values, size_t count, wxUIntPtr data)
{
	wxVector<wxVariant> nvalues;

	for(int col = 0; col < (int)m_columns.Count(); col++) {
		int idx = -1;
		L3CDListColumn *c = FindColumn(col, &idx, true);
		if (c != NULL && idx >= 0) {
			if (c->HaveIcon()) {
				int icon = values[idx].GetImage();
				nvalues.push_back( wxVariant(wxDataViewIconText(values[idx].GetText(), *m_icons[icon])) );
			} else {
				nvalues.push_back( values[idx].GetText() );
			}
		}
	}
	nvalues.push_back( wxT("") );

	AppendItem( nvalues, data );
	m_selecting.Add(0);
}

/// データを更新
void L3CDListCtrl::UpdateListItem(long row, L3CDListValue *values, size_t count, wxUIntPtr data)
{
	for(int col = 0; col < (int)m_columns.Count(); col++) {
		int idx = -1;
		wxVariant value;
		L3CDListColumn *c = FindColumn(col, &idx, true);
		if (c != NULL && idx >= 0) {
			if (c->HaveIcon()) {
				int icon = values[idx].GetImage();
				value = wxVariant(wxDataViewIconText(values[idx].GetText(), *m_icons[icon]));
			} else {
				value = values[idx].GetText();
			}
			SetValue(value, (unsigned int)row, (unsigned int)col);
		}
	}
}

#if 0
/// ディレクトリアイテムの位置を返す
int L3CDListCtrl::GetDirItemPos(const L3CDListItem &item) const
{
	return (int)GetListItemData(item);
}
#endif

/// ファイル名テキストを設定
/// @param [in] item リストアイテム
/// @param [in] idx  カラムの位置
/// @param [in] text ファイル名
void L3CDListCtrl::SetListText(const L3CDListItem &item, int idx, const wxString &text)
{
	int row = ItemToRow(item);
	int col = GetColumnPosition(m_columns[idx]->GetId());
	wxVariant value;
	GetValue(value, row, col);
	wxString type_name = value.GetType();
	if (type_name == wxT("string")) {
		value = text;
		SetValue(value, row, col);
	} else if (type_name == wxT("wxDataViewIconText")) {
		wxDataViewIconText newvalue;
		newvalue << value;
		newvalue.SetText(text);
		value << newvalue;
		SetValue(value, row, col);
	}
}

/// 選択している行の位置を返す
/// @return 複数行選択している場合 wxNOT_FOUND
int L3CDListCtrl::GetListSelectedRow() const
{
	return GetSelectedRow();
}
/// アイテム位置を返す
int L3CDListCtrl::GetListSelectedNum() const
{
	int row = wxNOT_FOUND;
	wxDataViewItem item = GetSelection();
	if (item.IsOk()) {
		row = (int)GetItemData(item);
	}
	return row;
}
/// 選択している行数
int L3CDListCtrl::GetListSelectedItemCount() const
{
	return GetSelectedItemsCount();
}
/// 選択行を得る
L3CDListItem L3CDListCtrl::GetListSelection() const
{
	return GetSelection();
}

/// 選択している行アイテムを得る
int L3CDListCtrl::GetListSelections(L3CDListItems &arr) const
{
	return GetSelections(arr);
}
/// 全行を選択
void L3CDListCtrl::SelectAllListItem()
{
	SelectAll();
}
/// 行アイテムを選択
void L3CDListCtrl::SelectListItem(const L3CDListItem &item)
{
	Select(item);
}
/// 行を選択
void L3CDListCtrl::SelectListRow(int row)
{
	SelectRow((unsigned int)row);
}
/// 非選択にする
void L3CDListCtrl::UnselectListItem(const L3CDListItem &item)
{
	Unselect(item);
}
/// 指定した行が選択しているか
int L3CDListCtrl::GetListSelected(int row) const
{
	return m_selecting.Item((size_t)row);
}
/// 指定した行が選択しているか
void L3CDListCtrl::SetListSelected(int row, int val)
{
	m_selecting.Item((size_t)row) = val;
}
/// フォーカスしている行アイテムを得る
L3CDListItem L3CDListCtrl::GetListFocusedItem() const
{
	// TODO: not implemented
	return wxDataViewItem();
}
/// 行アイテムをフォーカス
void L3CDListCtrl::FocusListItem(const L3CDListItem &item)
{
	// TODO: not implemented
}

/// アイテムを編集
void L3CDListCtrl::EditListItem(const L3CDListItem &item)
{
	wxDataViewColumn *column = GetColumn(0);
	EditItem(item, column);
}

/// リストを削除
bool L3CDListCtrl::DeleteAllListItems()
{
	DeleteAllItems();
	m_selecting.Empty();
	return true;
}

/// アイテムの固有データを返す
wxUIntPtr L3CDListCtrl::GetListItemData(const L3CDListItem &item) const
{
	return GetItemData(item);
}

/// アイテムの固有データを返す
wxUIntPtr L3CDListCtrl::GetListItemDataByRow(long row) const
{
	wxDataViewItem item = RowToItem((int)row);
	return GetItemData(item);
}

/// カラムヘッダのタイトルを返す
const wxString &L3CDListCtrl::GetColumnText(int idx) const
{
	return m_columns[idx]->GetText();
}

/// カラム表示中か
bool L3CDListCtrl::ColumnIsShown(int idx) const
{
	return (m_columns[idx]->GetColumn() >= 0);
}

/// カラムの表示を変更
/// @return true:リスト更新が必要
bool L3CDListCtrl::ShowColumn(int idx, bool show)
{
	L3CDListColumn *c = m_columns[idx];
	int col = c->GetColumn();
	bool nshow = (col >= 0);
	if (nshow && !show) {
		// 非表示
		wxDataViewColumn *column = GetColumn(col);
		c->SetColumn(~col);
		column->SetHidden(true);
		return false;

	} else if (!nshow && show) {
		// 表示
		col = ~col;
		int count = (int)GetColumnCount() - 1;
		if (col < count) {
			// 隠していたカラムを表示する
			wxDataViewColumn *column = GetColumn(col);
			c->SetColumn(col);
			column->SetHidden(false);
		} else {
			// カラムを追加する
			// 最終カラムには空のカラムがある
			wxDataViewColumn *column = GetColumn(count);
			c->SetColumn(count);
			column->SetTitle(c->GetText());
			column->SetWidth(c->GetWidth());
			column->SetAlignment(c->GetAlign());
			int flags = wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE;
			if (c->IsSortable()) flags |= wxDATAVIEW_COL_SORTABLE;
			column->SetFlags(flags);
			column->SetHidden(false);
			AppendTextColumn(wxT(""), wxDATAVIEW_CELL_INERT, 1, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE );
		}
		return true;
	}
	return false;
}

/// 表示位置のカラム情報を返す
L3CDListColumn *L3CDListCtrl::FindColumn(int col, int *n_idx, bool all) const
{
	L3CDListColumn *match = NULL;
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		if (m_columns[idx]->GetColumn() == col && (all || (m_columns[idx]->GetColumn() >= 0))) {
			match = m_columns[idx];
			if (n_idx) *n_idx = idx;
			break;
		}
	}
	return match;
}

/// 表示位置のカラム情報を返す
L3CDListColumn *L3CDListCtrl::FindColumn(wxDataViewColumn *col, int *n_idx, bool all) const
{
	L3CDListColumn *match = NULL;
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		if (m_columns[idx]->GetId() == col && (all || (m_columns[idx]->GetColumn() >= 0))) {
			match = m_columns[idx];
			if (n_idx) *n_idx = idx;
			break;
		}
	}
	return match;
}

/// カラム用のポップアップメニューを作成する
void L3CDListCtrl::CreateColumnPopupMenu(wxMenu* &menu, int menu_id, int menu_detail_id)
{
	// メニューアイテムを削除
	if (menu) delete menu;
	menu = new wxMenu;

	ReorderColumns();

	// 表示中のカラム
	int cols = (int)GetColumnCount();
	for(int col = 0; col < cols; col++) {
		int idx = -1;
		wxDataViewColumn *dcol = GetColumn(col);
		L3CDListColumn *column = FindColumn(dcol, &idx);
		if (column) {
			wxMenuItem *mitem = menu->AppendCheckItem(menu_id + idx, column->GetText());
			mitem->Check(true);
			mitem->Enable(idx != 0);
		}
	}
	// 非表示のカラム
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		L3CDListColumn *column = m_columns[idx];
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
void L3CDListCtrl::GetListColumnsByCurrentOrder(L3CDListColumns &items) const
{
	// 表示中のカラム
	for(int col = 0; col < (int)m_columns.Count(); col++) {
		int idx = -1;
		L3CDListColumn *column = FindColumn(col, &idx);
		if (column) {
			items.Add(column);
		}
	}
	// 非表示のカラム
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		L3CDListColumn *column = m_columns[idx];
		if (column->GetColumn() < 0) {
			items.Add(column);
		}
	}
}

/// カラム入れ替えダイアログを表示
/// @return true: submitted  false: canceled
bool L3CDListCtrl::ShowListColumnRearrangeBox()
{
	L3CDListColumns items;
	GetListColumnsByCurrentOrder(items);
	wxArrayInt order;
	wxArrayString labels;
	for(int i=0; i<(int)items.Count(); i++) {
		order.Add((items[i]->GetColumn() >= 0 ? i : ~i));
		labels.Add(items[i]->GetText());
	}
	L3CDListRearrangeBox dlg(this, order, labels);
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
	DeleteAllListColumns();
	InsertListColumns();

	return true;
}

/// カラム番号ソート用
int L3CDListCtrl::SortByColumn(L3CDListColumn **i1, L3CDListColumn **i2)
{
	int n = (*i1)->GetColumn() - (*i2)->GetColumn();
	if (n == 0) n = (*i1)->GetIndex() - (*i2)->GetIndex();
	return n;
}

#if 0 // defined(__WXMSW__)
#include <Windows.h>
#include <CommCtrl.h>
#endif

/// 現在のカラム位置を再取得
void L3CDListCtrl::ReorderColumns()
{
#if 0 //  defined(__WXMSW__)
	wxWindowList *list = &GetChildren();
	wxWindowListNode *node = list->GetFirst();
	HWND hHeader = NULL;
	while(node) {
		wxObject *obj = (wxObject *)node->GetData();
		wxClassInfo *info = obj->GetClassInfo();
		wxString name = info->GetClassName();
		if (name == wxT("wxControl")) {
			hHeader = ((wxWindow *)obj)->GetHandle();
		}

		node = node->GetNext();
	}

	if (hHeader) {
		int cnt = Header_GetItemCount(hHeader);
		int arr[20];

		Header_GetOrderArray(hHeader, cnt, &arr);
	}
#endif
}

//////////////////////////////////////////////////////////////////////

//
//
//
L3CDListRearrangeBox::L3CDListRearrangeBox(L3CDListCtrl *parent, const wxArrayInt &order, const wxArrayString &items)
	: wxRearrangeDialog(parent, _("Configure the columns shown:"), _("Arrange Column Order"), order, items) 
{
}
