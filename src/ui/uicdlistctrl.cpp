/// @file uicdlistctrl.cpp
///
/// @brief データ―ビューリストコントロール
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uicdlistctrl.h"
#include <wx/menu.h>
#include "../config.h"
#include "uicommon.h"


//////////////////////////////////////////////////////////////////////
//
//
//
MyCDListValue::MyCDListValue()
{
	row = 0;
	icon = -1;
}
void MyCDListValue::Set(long n_row, int n_icon, const wxString &n_value)
{
	row = (int)n_row;
	icon = n_icon;
	value = n_value;
}
void MyCDListValue::Set(long n_row, const wxString &n_value)
{
	row = (int)n_row;
	value = n_value;
}

//////////////////////////////////////////////////////////////////////
//
// リストのカラム情報
//
MyCDListColumn::MyCDListColumn(int n_idx, const struct st_list_columns *n_info, int n_default_width, int n_width)
{
	Set(n_idx, n_info, n_default_width, n_width);
}

void MyCDListColumn::Set(int n_idx, const struct st_list_columns *n_info, int n_default_width, int n_width)
{
	idx = n_idx;
	col = n_idx;
	info = n_info;
	width = n_width;
	default_width = n_default_width;
	sort_dir = 0;
	label = wxGetTranslation(n_info->label);
}

bool MyCDListColumn::HaveIcon() const
{
	return info->have_icon;
}

wxAlignment MyCDListColumn::GetAlign() const
{
	return info->align;
}

bool MyCDListColumn::IsSortable() const
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
MyCDListCtrl::MyCDListCtrl(UiDiskFrame *parentframe, wxWindow *parent, wxWindowID id,
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
		m_columns.Add(new MyCDListColumn(idx, c, c->width, w >= 0 ? w : c->width));
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
	MyCDListColumns arr;
	for(int idx = 0; idx < column_count; idx++) {
		arr.Add(m_columns[idx]);
	}
	arr.Sort(&SortByColumn);
	int n_col = 0;
	// 表示するカラムの順番を設定
	for(size_t idx = 0; idx < arr.Count(); idx++) {
		MyCDListColumn *column = arr.Item(idx);
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
		MyCDListColumn *column = arr.Item(idx);
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
	Bind(wxEVT_DATAVIEW_COLUMN_REORDERED , &MyCDListCtrl::OnColumnReordered, this);
	// ソートした
	Bind(wxEVT_DATAVIEW_COLUMN_SORTED, &MyCDListCtrl::OnColumnSorted, this);
}
MyCDListCtrl::~MyCDListCtrl()
{
	// カラム幅を取得
	for(int col = 0; col < (int)GetColumnCount(); col++) {
		wxDataViewColumn *c = GetColumn(col);
		MyCDListColumn *column = FindColumn(c, NULL);
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
void MyCDListCtrl::AssignListIcons(const char ***icons)
{
	for(size_t idx=0; icons[idx] != NULL; idx++) {
		m_icons.Add(new wxIcon(icons[idx]));
	}
}

#if 0
/// カラム幅を変えた
void MyCDListCtrl::OnColumnEndDrag(MyCDListEvent& event)
{
	int col = event.GetColumn();
	UiDiskFileListColumn *column = FindColumn(col, NULL);
	if (column) {
		column->SetWidth(GetColumnWidth(col));
	}
}
#endif

/// カラムを入れ替え
/// @note implemented in wxWidgets 3.1.1 or higher
void MyCDListCtrl::OnColumnReordered(MyCDListEvent& event)
{
//	int col = 1;
}

/// カラムをソート
void MyCDListCtrl::OnColumnSorted(MyCDListEvent& event)
{
	int col = event.GetColumn();
	MyCDListColumn *column = FindColumn(col, NULL);
	if (column) {
	}
}

/// リストにカラムを設定する
void MyCDListCtrl::InsertListColumns()
{
	// カラム位置の設定
	int column_count = (int)m_columns.Count();

	// カラム位置を振り直し
	MyCDListColumns arr;
	for(int idx = 0; idx < column_count; idx++) {
		arr.Add(m_columns[idx]);
	}
	arr.Sort(&SortByColumn);
	int n_col = 0;
	// 表示するカラムの順番を設定
	for(size_t idx = 0; idx < arr.Count(); idx++) {
		MyCDListColumn *column = arr.Item(idx);
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
		MyCDListColumn *column = arr.Item(idx);
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
void MyCDListCtrl::DeleteAllListColumns()
{
//	int count = (int)GetColumnCount();
//	for(int col = count - 1; col >= 0; col--) {
//		wxDataViewColumn *column = GetColumn((unsigned int)col);
//		DeleteColumn(column);
//	}
	ClearColumns();
}

/// 初期 カラムを設定
void MyCDListCtrl::InsertListColumn(int col)
{
	MyCDListColumn *c = FindColumn(col, NULL, true);
	if (!c) return;

	InsertListColumn(col, c->GetIndex(), c);
}

/// カラムを挿入
void MyCDListCtrl::InsertListColumn(int col, int idx, MyCDListColumn *c)
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
int MyCDListCtrl::GetListColumnWidth(int col) const
{
	return GetColumn(col)->GetWidth();
}

/// カラムの幅をセット
void MyCDListCtrl::SetListColumnWidth(int col, int w)
{
	GetColumn(col)->SetWidth(w);
}

/// カラムを削除
void MyCDListCtrl::DeleteListColumn(int col)
{
	DeleteColumn(GetColumn(col));
}

/// データを挿入
void MyCDListCtrl::InsertListItem(long row, MyCDListValue *values, size_t count, wxUIntPtr data)
{
	wxVector<wxVariant> nvalues;

	for(int col = 0; col < (int)m_columns.Count(); col++) {
		int idx = -1;
		MyCDListColumn *c = FindColumn(col, &idx, true);
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
void MyCDListCtrl::UpdateListItem(long row, MyCDListValue *values, size_t count, wxUIntPtr data)
{
	for(int col = 0; col < (int)m_columns.Count(); col++) {
		int idx = -1;
		wxVariant value;
		MyCDListColumn *c = FindColumn(col, &idx, true);
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
int MyCDListCtrl::GetDirItemPos(const MyCDListItem &item) const
{
	return (int)GetListItemData(item);
}
#endif

/// ファイル名テキストを設定
/// @param [in] item リストアイテム
/// @param [in] idx  カラムの位置
/// @param [in] text ファイル名
void MyCDListCtrl::SetListText(const MyCDListItem &item, int idx, const wxString &text)
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
int MyCDListCtrl::GetListSelectedRow() const
{
	return GetSelectedRow();
}
/// アイテム位置を返す
int MyCDListCtrl::GetListSelectedNum() const
{
	int row = wxNOT_FOUND;
	wxDataViewItem item = GetSelection();
	if (item.IsOk()) {
		row = (int)GetItemData(item);
	}
	return row;
}
/// 選択している行数
int MyCDListCtrl::GetListSelectedItemCount() const
{
	return GetSelectedItemsCount();
}
/// 選択行を得る
MyCDListItem MyCDListCtrl::GetListSelection() const
{
	return GetSelection();
}

/// 選択している行アイテムを得る
int MyCDListCtrl::GetListSelections(MyCDListItems &arr) const
{
	return GetSelections(arr);
}
/// 全行を選択
void MyCDListCtrl::SelectAllListItem()
{
	SelectAll();
}
/// 行アイテムを選択
void MyCDListCtrl::SelectListItem(const MyCDListItem &item)
{
	Select(item);
}
/// 行を選択
void MyCDListCtrl::SelectListRow(int row)
{
	SelectRow((unsigned int)row);
}
/// 全て非選択にする
void MyCDListCtrl::UnselectAllListItem()
{
	UnselectAll();
}
/// 非選択にする
void MyCDListCtrl::UnselectListItem(const MyCDListItem &item)
{
	Unselect(item);
}
/// 指定した行が選択しているか
int MyCDListCtrl::GetListSelected(int row) const
{
	return m_selecting.Item((size_t)row);
}
/// 指定した行が選択しているか
void MyCDListCtrl::SetListSelected(int row, int val)
{
	m_selecting.Item((size_t)row) = val;
}
/// フォーカスしている行アイテムを得る
MyCDListItem MyCDListCtrl::GetListFocusedItem() const
{
	// TODO: not implemented
	return wxDataViewItem();
}
/// 行アイテムをフォーカス
void MyCDListCtrl::FocusListItem(const MyCDListItem &item)
{
	// TODO: not implemented
}

/// アイテムを編集
void MyCDListCtrl::EditListItem(const MyCDListItem &item)
{
	wxDataViewColumn *column = GetColumn(0);
	EditItem(item, column);
}

/// リストを削除
bool MyCDListCtrl::DeleteAllListItems()
{
	DeleteAllItems();
	m_selecting.Empty();
	return true;
}

/// アイテムの固有データを返す
wxUIntPtr MyCDListCtrl::GetListItemData(const MyCDListItem &item) const
{
	return GetItemData(item);
}

/// アイテムの固有データを返す
wxUIntPtr MyCDListCtrl::GetListItemDataByRow(long row) const
{
	wxDataViewItem item = RowToItem((int)row);
	return GetItemData(item);
}

/// カラムヘッダのタイトルを返す
const wxString &MyCDListCtrl::GetColumnText(int idx) const
{
	return m_columns[idx]->GetText();
}

/// カラム表示中か
bool MyCDListCtrl::ColumnIsShown(int idx) const
{
	return (m_columns[idx]->GetColumn() >= 0);
}

/// カラムの表示を変更
/// @return true:リスト更新が必要
bool MyCDListCtrl::ShowColumn(int idx, bool show)
{
	MyCDListColumn *c = m_columns[idx];
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
MyCDListColumn *MyCDListCtrl::FindColumn(int col, int *n_idx, bool all) const
{
	MyCDListColumn *match = NULL;
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
MyCDListColumn *MyCDListCtrl::FindColumn(wxDataViewColumn *col, int *n_idx, bool all) const
{
	MyCDListColumn *match = NULL;
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		if (m_columns[idx]->GetId() == col && (all || (m_columns[idx]->GetColumn() >= 0))) {
			match = m_columns[idx];
			if (n_idx) *n_idx = idx;
			break;
		}
	}
	return match;
}

#if 0
/// カラム用のポップアップメニューを作成する
void MyCDListCtrl::CreateColumnPopupMenu(wxMenu* &menu, int menu_id, int menu_detail_id)
{
	// メニューアイテムを削除
	if (menu) delete menu;
	menu = new wxMenu;

	// 現在表示しているカラムの順序でカラム番号を再設定
	ReorderColumns();

	// 表示しているカラムのソート
	MyCDListColumns arr;
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		if (m_columns[idx]->GetColumn() >= 0) {
			arr.Add(m_columns[idx]);
		}
	}
	arr.Sort(SortByColumn);

	// 表示しているカラム
	for(int col = 0; col < (int)arr.Count(); col++) {
		MyCDListColumn *column = arr[col];
		int idx = column->GetIndex();
		wxMenuItem *mitem = menu->AppendCheckItem(menu_id + idx, column->GetText());
		mitem->Check(true);
		mitem->Enable(idx != 0);
	}
	// 非表示のカラム
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		MyCDListColumn *column = m_columns[idx];
		if (column->GetColumn() < 0) {
			wxMenuItem *mitem = menu->AppendCheckItem(menu_id + idx, column->GetText());
			mitem->Check(false);
		}
	}
	// 詳細
	menu->AppendSeparator();
	menu->Append(menu_detail_id, _("Detail..."));
}
#endif

/// カラムの表示位置を返す 表示中のカラムを優先
void MyCDListCtrl::GetListColumnsByCurrentOrder(MyCDListColumns &items) const
{
	// 表示中のカラム
	for(int col = 0; col < (int)m_columns.Count(); col++) {
		int idx = -1;
		MyCDListColumn *column = FindColumn(col, &idx);
		if (column) {
			items.Add(column);
		}
	}
	// 非表示のカラム
	for(int idx = 0; idx < (int)m_columns.Count(); idx++) {
		MyCDListColumn *column = m_columns[idx];
		if (column->GetColumn() < 0) {
			items.Add(column);
		}
	}
}

/// カラム入れ替えダイアログを表示
/// @return true: submitted  false: canceled
bool MyCDListCtrl::ShowListColumnRearrangeBox()
{
	// 現在表示しているカラムの順序でカラム番号を再設定
	ReorderColumns();

	// カラムの表示位置を返す 表示中のカラムを優先
	MyCDListColumns items;
	GetListColumnsByCurrentOrder(items);

	wxArrayInt order;
	wxArrayString labels;
	for(int i=0; i<(int)items.Count(); i++) {
		order.Add((items[i]->GetColumn() >= 0 ? i : ~i));
		labels.Add(items[i]->GetText());
	}

	MyCDListRearrangeBox dlg(this, order, labels);
	int sts = dlg.ShowModal();
	if (sts != wxID_OK) return false;

	// カラム表示、非表示を更新
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

/// 全てのカラムの幅をデフォルトに戻す
void MyCDListCtrl::ResetAllListColumnWidth()
{
	int column_count = (int)m_columns.Count();
	for(int idx = 0; idx < column_count; idx++) {
		MyCDListColumn *c = m_columns.Item(idx);
		SetListColumnWidth(c->GetColumn(), c->GetDefaultWidth());
	}
}

/// カラム番号ソート用
int MyCDListCtrl::SortByColumn(MyCDListColumn **i1, MyCDListColumn **i2)
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
void MyCDListCtrl::ReorderColumns()
{
	// 現在表示しているカラムの順序でカラム番号を再設定
	int cols = (int)GetColumnCount();
	for(int col = 0; col < cols; col++) {
		wxDataViewColumn *datacol = GetColumn(col);
		int idx = GetColumnPosition(datacol);
		MyCDListColumn *column = FindColumn(datacol, NULL);
		if (column) {
			column->SetColumn(idx);
		}
	}
}

/// 指定した座標に行アイテムがあるか
bool MyCDListCtrl::HasItemAtPoint(int x, int y) const
{
	wxPoint pt(x, y);
	MyCDListItem item;
	wxDataViewColumn* column;
	HitTest(pt, item, column);
	return item.IsOk();
}

/// 指定した座標にある行アイテムを返す
MyCDListItem MyCDListCtrl::GetItemAtPoint(int x, int y) const
{
	wxPoint pt(x, y);
	MyCDListItem item;
	wxDataViewColumn* column;
	HitTest(pt, item, column);
	return item;
}

//////////////////////////////////////////////////////////////////////

//
//
//
MyCDListRearrangeBox::MyCDListRearrangeBox(MyCDListCtrl *parent, const wxArrayInt &order, const wxArrayString &items)
	: wxRearrangeDialog(parent, _("Configure the columns shown:"), _("Arrange Column Order"), order, items) 
{
}
