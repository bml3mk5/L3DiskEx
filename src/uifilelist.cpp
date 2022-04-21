/// @file uifilelist.cpp
///
/// @brief BASICファイル名一覧
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uifilelist.h"
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/numformatter.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/imaglist.h>
#include "main.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "intnamebox.h"
#include "basicselbox.h"
#include "basicparambox.h"
#include "diskresult.h"
#include "utils.h"


extern const char *fileicon_normal_xpm[];
extern const char *foldericon_close_xpm[];
extern const char *labelicon_normal_xpm[];

const char **icons_for_flist[] = {
	fileicon_normal_xpm,
	foldericon_close_xpm,
	labelicon_normal_xpm,
	NULL
};
enum en_icons_for_flist {
	ICON_FOR_LIST_NONE = -1,
	ICON_FOR_LIST_FILE_NORMAL = 0,
	ICON_FOR_LIST_FOLDER,
	ICON_FOR_LIST_LABEL,
};

#ifdef DeleteFile
#undef DeleteFile
#endif

/// ドラッグ時のデータ構成
typedef struct st_directory_for_dnd {
	wxUint32	data_size;
	DiskBasicFormatType	format_type;
	wxUint8		name[40];
	int			file_size;
	int			file_type;
	int			original_type;
	int			start_addr;
	int			exec_addr;
	int			external_attr;
	wxUint8		date[3];
	wxUint8		time[3];
	wxUint8		reserved[2];
} directory_for_dnd_t;

//////////////////////////////////////////////////////////////////////

#ifndef USE_LIST_CTRL_ON_FILE_LIST
//
//
//
L3DiskFileListStoreModel::L3DiskFileListStoreModel(L3DiskFrame *parentframe)
	: wxDataViewListStore()
{
	frame = parentframe;
}
bool L3DiskFileListStoreModel::SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
{
	// 編集後のファイル名を反映しない
	return false;
}
#endif

//////////////////////////////////////////////////////////////////////
//
// リストコントロール
//
/// @param [in] parentframe 親フレーム
/// @param [in] parent      親コントロール
/// @param [in] id          ID
/// @param [in] pos         位置
/// @param [in] size    　　サイズ
L3DiskFileListCtrl::L3DiskFileListCtrl(L3DiskFrame *parentframe, wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	: wxDataViewListCtrl(parent, id, pos, size, wxDV_ROW_LINES | wxDV_MULTIPLE
#else
	: wxListCtrl(parent, id, pos, size, wxLC_REPORT | wxLC_EDIT_LABELS
#endif
) {
#ifndef USE_LIST_CTRL_ON_FILE_LIST

	L3DiskFileListStoreModel *model = new L3DiskFileListStoreModel(parentframe);
	AssociateModel(model);
	model->DecRef();

	listColumns[LISTCOL_NAME] = AppendIconTextColumn(_("File Name"), wxDATAVIEW_CELL_EDITABLE, 160, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );
	listColumns[LISTCOL_ATTR] = AppendTextColumn(_("Attributes"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );
	listColumns[LISTCOL_SIZE] = AppendTextColumn(_("Size"), wxDATAVIEW_CELL_INERT, 60, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_GROUPS] = AppendTextColumn(_("Groups"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_START] = AppendTextColumn(_("Start Group"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_TRACK] = AppendTextColumn(_("Track"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_SIDE] = AppendTextColumn(_("Side"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_SECTOR] = AppendTextColumn(_("Sector"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_DATE] = AppendTextColumn(_("Date Time"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );

#else

	listColumns[LISTCOL_NAME] = AppendColumn(_("File Name"), wxLIST_FORMAT_LEFT, 160);
	listColumns[LISTCOL_ATTR] = AppendColumn(_("Attributes"), wxLIST_FORMAT_LEFT, 150);
	listColumns[LISTCOL_SIZE] = AppendColumn(_("Size"), wxLIST_FORMAT_RIGHT, 60);
	listColumns[LISTCOL_GROUPS] = AppendColumn(_("Groups"), wxLIST_FORMAT_RIGHT, 40);
	listColumns[LISTCOL_START] = AppendColumn(_("Start Group"), wxLIST_FORMAT_RIGHT, 40);
	listColumns[LISTCOL_TRACK] = AppendColumn(_("Track"), wxLIST_FORMAT_RIGHT, 40);
	listColumns[LISTCOL_SIDE] = AppendColumn(_("Side"), wxLIST_FORMAT_RIGHT, 40);
	listColumns[LISTCOL_SECTOR] = AppendColumn(_("Sector"), wxLIST_FORMAT_RIGHT, 40);
	listColumns[LISTCOL_DATE] = AppendColumn(_("Date Time"), wxLIST_FORMAT_LEFT, 150);

	// icon
	wxImageList *ilist = new wxImageList( 16, 16 );
	for(int i=0; icons_for_flist[i] != NULL; i++) {
		ilist->Add( wxIcon(icons_for_flist[i]) );
	}
	AssignImageList( ilist, wxIMAGE_LIST_SMALL );
#endif
}
L3DiskFileListCtrl::~L3DiskFileListCtrl()
{
}
/// リストにデータを挿入
/// @param [in] row	    行番号
/// @param [in] icon    アイコン番号
/// @param [in] filename ファイル名
/// @param [in] attr    属性文字列
/// @param [in] size    ファイルサイズ
/// @param [in] groups  グループ数
/// @param [in] start   開始グループ番号
/// @param [in] trk     トラック番号
/// @param [in] sid     サイド番号
/// @param [in] sec     セクタ番号
/// @param [in] date    日付文字列
/// @param [in] data    ディレクトリアイテムの位置
/// @return 行番号
long L3DiskFileListCtrl::InsertListData(long row, int icon, const wxString &filename, const wxString &attr, int size, int groups, int start, int trk, int sid, int sec, const wxString &date, wxUIntPtr data)
{
	m_icon = icon;
	m_values[LISTCOL_NAME] = filename;
	m_values[LISTCOL_ATTR] = attr;
	m_values[LISTCOL_SIZE] = (size >= 0 ? wxNumberFormatter::ToString((long)size) : wxT("---"));
	m_values[LISTCOL_GROUPS] = (groups >= 0 ? wxNumberFormatter::ToString((long)groups) : wxT("---"));
	m_values[LISTCOL_START] = wxString::Format(wxT("%02x"), start);
	m_values[LISTCOL_TRACK] = (trk >= 0 ? wxString::Format(wxT("%d"), trk) : wxT("-"));
	m_values[LISTCOL_SIDE] = (sid >= 0 ? wxString::Format(wxT("%d"), sid) : wxT("-"));
	m_values[LISTCOL_SECTOR] = (sec >= 0 ? wxString::Format(wxT("%d"), sec) : wxT("-"));
	m_values[LISTCOL_DATE] = date;

#ifndef USE_LIST_CTRL_ON_FILE_LIST
	wxVector<wxVariant> values;
	values.push_back( wxVariant(wxDataViewIconText(m_values[LISTCOL_NAME], icons_for_flist[m_icon])) );
	values.push_back( m_values[LISTCOL_ATTR] );
	values.push_back( m_values[LISTCOL_SIZE] );
	values.push_back( m_values[LISTCOL_GROUPS] );
	values.push_back( m_values[LISTCOL_START] );
	values.push_back( m_values[LISTCOL_TRACK] );
	values.push_back( m_values[LISTCOL_SIDE] );
	values.push_back( m_values[LISTCOL_SECTOR] );
	values.push_back( m_values[LISTCOL_DATE] );
	AppendItem( values, data );
#else
	InsertItem(row, m_values[LISTCOL_NAME], m_icon);
	SetItem(row, (int)listColumns[LISTCOL_ATTR], m_values[LISTCOL_ATTR]);
	SetItem(row, (int)listColumns[LISTCOL_SIZE], m_values[LISTCOL_SIZE]);
	SetItem(row, (int)listColumns[LISTCOL_GROUPS], m_values[LISTCOL_GROUPS]);
	SetItem(row, (int)listColumns[LISTCOL_START], m_values[LISTCOL_START]);
	SetItem(row, (int)listColumns[LISTCOL_TRACK], m_values[LISTCOL_TRACK]);
	SetItem(row, (int)listColumns[LISTCOL_SIDE], m_values[LISTCOL_SIDE]);
	SetItem(row, (int)listColumns[LISTCOL_SECTOR], m_values[LISTCOL_SECTOR]);
	SetItem(row, (int)listColumns[LISTCOL_DATE], m_values[LISTCOL_DATE]);
	SetItemPtrData(row, data);
#endif
	return row;
}
/// リストにデータを設定する
/// @param [in] basic       DISK BASIC
/// @param [in] dir_items   ディレクトリアイテムのリスト
void L3DiskFileListCtrl::SetListData(DiskBasic *basic, const DiskBasicDirItems *dir_items)
{
	long row = 0;

	for(size_t idx = 0; idx < dir_items->Count(); idx++) {
		DiskBasicDirItem *item = dir_items->Item(idx);
		if (!item->IsUsedAndVisible()) continue;

		DiskBasicFileType file_type = item->GetFileAttr();
		int icon = -1;
		if (file_type.IsDirectory()) {
			icon = ICON_FOR_LIST_FOLDER;
		} else if (file_type.IsVolume()) {
			icon = ICON_FOR_LIST_LABEL;
		} else {
			icon = ICON_FOR_LIST_FILE_NORMAL;
		}
		int track_num = -1;
		int side_num = -1;
		int sector_start = -1;
		if (!basic->CalcStartNumFromGroupNum((wxUint32)item->GetStartGroup(), track_num, side_num, sector_start)) {
			track_num = -1;
			side_num = -1;
			sector_start = -1;
		}
		InsertListData(
			row,
			icon,						// アイコン
			item->GetFileNameStr(),		// ファイル名
			item->GetFileAttrStr(),		// ファイル属性
			item->GetFileSize(),		// ファイルサイズ
			item->GetGroupSize(),		// 使用グループ数
			item->GetStartGroup(),		// 開始グループ
			track_num,					// トラック
			side_num,					// サイド
			sector_start,				// セクタ
			item->GetFileDateTimeStr(),	// 日時
			(wxUIntPtr)idx
		);

		row++;
	}
}

/// ファイル名テキストを設定
/// @param [in] item リストアイテム
/// @param [in] text ファイル名
void L3DiskFileListCtrl::SetListText(const L3FileListItem &item, const wxString &text)
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	int row = ItemToRow(item);
	int col = GetColumnPosition(listColumns[LISTCOL_NAME]);
	wxVariant value;
	GetValue(value, row, col);
	wxDataViewIconText newvalue;
	newvalue << value;
	newvalue.SetText(text);
	value << newvalue;
	SetValue(value, row, col);
#else
	SetItem(item, (int)listColumns[LISTCOL_NAME], text);
#endif
}

/// 選択している行の位置を返す
/// @return 複数行選択している場合 wxNOT_FOUND
int L3DiskFileListCtrl::GetListSelectedRow() const
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	return GetSelectedRow();
#else
	return (int)GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
#endif
}
/// 選択している行数
int L3DiskFileListCtrl::GetListSelectedItemCount() const
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	return GetSelectedItemsCount();
#else
	return GetSelectedItemCount();
#endif
}
/// 選択行を得る
L3FileListItem L3DiskFileListCtrl::GetListSelection() const
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	return GetSelection();
#else
	return GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
#endif
}

/// 選択している行アイテムを得る
int L3DiskFileListCtrl::GetListSelections(L3FileListItems &arr) const
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	return GetSelections(arr);
#else
	long item = -1;
	arr.Empty();
	for(;;) {
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		if (item == -1) break;
		arr.Add(item);
	}
	return (int)arr.Count();
#endif
}

/// アイテムを編集
void L3DiskFileListCtrl::EditListItem(const L3FileListItem &item)
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	wxDataViewColumn *column = GetColumn(0);
	EditItem(item, column);
#else
	EditLabel(item);
#endif
}

/// リストを削除
bool L3DiskFileListCtrl::DeleteAllListItems()
{
	return DeleteAllItems();
}

/// アイテムの固有データを返す
wxUIntPtr L3DiskFileListCtrl::GetListItemData(const L3FileListItem &item) const
{
	return GetItemData(item);
}

//////////////////////////////////////////////////////////////////////
//
// 右パネルのファイルリスト
//
#define TEXT_ATTR_SIZE 440

// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskFileList, wxPanel)
	EVT_SIZE(L3DiskFileList::OnSize)

#ifndef USE_LIST_CTRL_ON_FILE_LIST
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(IDC_VIEW_LIST, L3DiskFileList::OnListContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(IDC_VIEW_LIST, L3DiskFileList::OnListActivated)

	EVT_DATAVIEW_SELECTION_CHANGED(IDC_VIEW_LIST, L3DiskFileList::OnSelectionChanged)

	EVT_DATAVIEW_ITEM_BEGIN_DRAG(IDC_VIEW_LIST, L3DiskFileList::OnBeginDrag)

	EVT_DATAVIEW_ITEM_START_EDITING(IDC_VIEW_LIST, L3DiskFileList::OnFileNameStartEditing)
	EVT_DATAVIEW_ITEM_EDITING_STARTED(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditingStarted)
	EVT_DATAVIEW_ITEM_EDITING_DONE(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditedDone)
#else
//	EVT_LIST_ITEM_RIGHT_CLICK(IDC_VIEW_LIST, L3DiskFileList::OnListContextMenu)
	EVT_LIST_ITEM_ACTIVATED(IDC_VIEW_LIST, L3DiskFileList::OnListActivated)

	EVT_LIST_ITEM_SELECTED(IDC_VIEW_LIST, L3DiskFileList::OnSelectionChanged)

	EVT_LIST_BEGIN_DRAG(IDC_VIEW_LIST, L3DiskFileList::OnBeginDrag)

	EVT_LIST_BEGIN_LABEL_EDIT(IDC_VIEW_LIST, L3DiskFileList::OnFileNameStartEditing)
//	EVT_DATAVIEW_ITEM_EDITING_STARTED(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditingStarted)
	EVT_LIST_END_LABEL_EDIT(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditedDone)

	EVT_CONTEXT_MENU(L3DiskFileList::OnContextMenu)
#endif

	EVT_BUTTON(IDC_BTN_CHANGE, L3DiskFileList::OnButtonChange)
	EVT_RADIOBUTTON(IDC_RADIO_CHAR_ASCII, L3DiskFileList::OnChangeCharCode)
	EVT_RADIOBUTTON(IDC_RADIO_CHAR_SJIS , L3DiskFileList::OnChangeCharCode)

	EVT_MENU(IDM_EXPORT_FILE, L3DiskFileList::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, L3DiskFileList::OnImportFile)

	EVT_MENU(IDM_DELETE_FILE, L3DiskFileList::OnDeleteFile)
	EVT_MENU(IDM_RENAME_FILE, L3DiskFileList::OnRenameFile)

	EVT_MENU(IDM_MAKE_DIRECTORY, L3DiskFileList::OnMakeDirectory)

	EVT_MENU(IDM_PROPERTY, L3DiskFileList::OnProperty)
wxEND_EVENT_TABLE()

L3DiskFileList::L3DiskFileList(L3DiskFrame *parentframe, wxWindow *parentwindow)
       : wxPanel(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	initialized = false;
	parent   = parentwindow;
	frame    = parentframe;
	basic	 = NULL;
	list	 = NULL;
	disk_selecting = false;

//	wxSize sz = parentwindow->GetClientSize();
//	SetSize(sz);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	szrButtons = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxSize size(TEXT_ATTR_SIZE, -1);
	textAttr = new wxTextCtrl(this, IDC_TEXT_ATTR, wxT(""), wxDefaultPosition, size, wxTE_READONLY | wxTE_LEFT);
	hbox->Add(textAttr, wxSizerFlags().Expand().Border(wxBOTTOM | wxTOP, 2));

	size.x = 60;
	btnChange = new wxButton(this, IDC_BTN_CHANGE, _("Change"), wxDefaultPosition, size);
	btnChange->Enable(false);
	szrButtons->Add(btnChange, flags);

	lblCharCode = new wxStaticText(this, wxID_ANY, _("Charactor Code"));
	szrButtons->Add(lblCharCode,  wxSizerFlags().Center().Border(wxBOTTOM | wxTOP, 2).Border(wxLEFT | wxRIGHT, 8));
	radCharAscii = new wxRadioButton(this, IDC_RADIO_CHAR_ASCII, _("Ascii"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	szrButtons->Add(radCharAscii, flags);
	radCharSJIS = new wxRadioButton(this, IDC_RADIO_CHAR_SJIS, _("Shift JIS"), wxDefaultPosition, wxDefaultSize);
	szrButtons->Add(radCharSJIS, flags);

	radCharAscii->SetValue(true);
	radCharSJIS->SetValue(false);

	hbox->Add(szrButtons);
	vbox->Add(hbox);

	wxFont font;
	frame->GetDefaultListFont(font);

	list = new L3DiskFileListCtrl(parentframe, this, IDC_VIEW_LIST, wxDefaultPosition, wxDefaultSize);
	list->SetFont(font);
	vbox->Add(list, wxSizerFlags().Expand().Border(wxALL, 1));

//	vbox->SetSizeHints(this);

	SetSizerAndFit(vbox);
	Layout();

	// popup menu
	menuPopup = new wxMenu;
	menuPopup->Append(IDM_EXPORT_FILE, _("&Export..."));
	menuPopup->Append(IDM_IMPORT_FILE, _("&Import..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_DELETE_FILE, _("&Delete..."));
	menuPopup->Append(IDM_RENAME_FILE, _("&Rename"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MAKE_DIRECTORY, _("&Make Directory..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY, _("&Property"));

	// key
	list->Bind(wxEVT_CHAR, &L3DiskFileList::OnChar, this);
	
	initialized = true;
}

L3DiskFileList::~L3DiskFileList()
{
	delete menuPopup;
}

/// アイテムを得る
L3FileListItem L3DiskFileList::GetEventItem(const L3FileListEvent& event) const
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	return event.GetItem();
#else
	return event.GetIndex();
#endif
}

/// リサイズ
void L3DiskFileList::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();
	wxSize sizz = szrButtons->GetSize();
	if (sizz.x == 0) return;

	wxPoint listpt = list->GetPosition();
	list->SetSize(size.x - listpt.x, size.y - listpt.y);

	int pos_x = size.x - sizz.x;
	if (pos_x < 0) return;

	wxPoint bp;
	bp = btnChange->GetPosition();

	pos_x -= bp.x;

	wxSize tz = textAttr->GetSize();
	tz.x += pos_x;
	if (tz.x < TEXT_ATTR_SIZE) return;

	textAttr->SetSize(tz);

	bp.x += pos_x;

	wxSizerItemList *slist = &szrButtons->GetChildren();
	wxSizerItemList::iterator it;
	for(it = slist->begin(); it != slist->end(); it++) {
		wxSizerItem *item = *it;
		if (item->IsWindow()) {
			wxWindow *win = item->GetWindow();
			bp = win->GetPosition();
			bp.x += pos_x;
			win->SetPosition(bp);
		}
	}
}

/// 選択行を変更した
void L3DiskFileList::OnSelectionChanged(L3FileListEvent& event)
{
	if (!initialized || !basic) return;

	if (list->GetListSelectedRow() == wxNOT_FOUND) {
		// 非選択
		UnselectItem();
		return;
	}
	L3FileListItem selected_item = list->GetListSelection();
#if 1
	wxUint32 start_group = (wxUint32)-1;
	DiskD88Sector *sector = basic->GetSectorFromPosition(list->GetListItemData(selected_item), &start_group);
	if (!sector) {
		// セクタなし
		UnselectItem();
		return;
	}
	// 選択
	SelectItem(start_group, sector, basic->GetCharCode(), basic->IsDataInverted());
#else
	DiskBasicGroups arr;
	if (!basic->GetGroupsFromPosition(list->GetItemData(selected_item), arr)) {
		// セクタなし
		UnselectItem();
		return;
	}

	// 選択
	SelectItem(arr);
#endif
}

/// リストの編集開始
void L3DiskFileList::OnFileNameStartEditing(L3FileListEvent& event)
{
	if (!basic) return;

	L3FileListItem listitem = GetEventItem(event);
	int pos = (int)list->GetListItemData(listitem);
	DiskBasicDirItem *ditem = basic->GetDirItem(pos);
	if (!ditem || !ditem->IsFileNameEditable()) {
		// 編集不可
		event.Veto();
	}
}

/// リストの編集開始した
void L3DiskFileList::OnFileNameEditingStarted(L3FileListEvent& event)
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	if (!basic) return;

	wxDataViewColumn *column = event.GetDataViewColumn();
	wxDataViewRenderer *renderer = column->GetRenderer();
	if (!renderer) return;
	wxTextCtrl *text = (wxTextCtrl *)renderer->GetEditorCtrl();
	if (!text) return;

	wxDataViewItem listitem = event.GetItem();
	int pos = (int)list->GetListItemData(listitem);
	DiskBasicDirItem *ditem = basic->GetDirItem(pos);
	if (ditem && ditem->IsFileNameEditable()) {
		int max_len = ditem->GetFileNameStrSize();
		IntNameValidator validate(basic, ditem);
		text->SetMaxLength(max_len);
		text->SetValidator(validate);
	}
#endif
}

/// リストの編集終了
void L3DiskFileList::OnFileNameEditedDone(L3FileListEvent& event)
{
	if (event.IsEditCancelled()) return;

#ifndef USE_LIST_CTRL_ON_FILE_LIST
	wxDataViewItem listitem = event.GetItem();
	wxVariant value = event.GetValue();
	wxDataViewIconText newvalue;
	newvalue << value;
	RenameDataFile(listitem, newvalue.GetText());
#else
	long listitem = event.GetIndex();
	wxString newname = event.GetLabel();
	RenameDataFile(listitem, newname);
	// 編集結果はRenameDataFileで設定するのでイベントの後で
	// 文字列を設定しないようにキャンセルにする
	event.Veto();
#endif
}

/// リスト上で右クリック
void L3DiskFileList::OnListContextMenu(L3FileListEvent& event)
{
	ShowPopupMenu();
}

/// 右クリック
void L3DiskFileList::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// リスト上でダブルクリック
void L3DiskFileList::OnListActivated(L3FileListEvent& event)
{
	DoubleClicked();
}

/// リスト上でドラッグ開始
void L3DiskFileList::OnBeginDrag(L3FileListEvent& event)
{
	// ドラッグ
	DragDataSource();
}

/// エクスポート選択
void L3DiskFileList::OnExportFile(wxCommandEvent& event)
{
	ShowExportDataFileDialog();
}

/// インポート選択
void L3DiskFileList::OnImportFile(wxCommandEvent& event)
{
	ShowImportDataFileDialog();
}

/// 削除選択
void L3DiskFileList::OnDeleteFile(wxCommandEvent& event)
{
	DeleteDataFile();
}

/// リネーム選択
void L3DiskFileList::OnRenameFile(wxCommandEvent& event)
{
	StartEditingFileName();
}

/// ディレクトリ作成選択
void L3DiskFileList::OnMakeDirectory(wxCommandEvent& event)
{
	MakeDirectory();
}

/// プロパティ選択
void L3DiskFileList::OnProperty(wxCommandEvent& event)
{
	ShowFileAttr();
}

/// リスト上でキー押下
void L3DiskFileList::OnChar(wxKeyEvent& event)
{
	switch(event.GetKeyCode()) {
	case WXK_RETURN:
		// Enter    ダブルクリックと同じ
		DoubleClicked();
		break;
	case WXK_DELETE:
		// Delete   ファイル削除
		DeleteDataFile();
		break;
	case WXK_CONTROL_C:
		// Ctrl + C クリップボードヘコピー
		CopyToClipboard();
		break;
	case WXK_CONTROL_V:
		// Ctrl + V クリップボードからペースト
		PasteFromClipboard();
		break;
	default:
		event.Skip();
		break;
	}
}

/// 変更ボタン押下
void L3DiskFileList::OnButtonChange(wxCommandEvent& event)
{
	ChangeBasicType();
}

/// キャラクターコード変更ボタン押下
void L3DiskFileList::OnChangeCharCode(wxCommandEvent& event)
{
	frame->ChangeCharCode(event.GetId() - IDC_RADIO_CHAR_ASCII);
}

/// ポップアップメニュー表示
void L3DiskFileList::ShowPopupMenu()
{
	if (!menuPopup) return;

	bool opened = (basic ? basic->IsFormatted() : false);
	menuPopup->Enable(IDM_IMPORT_FILE, opened /* && IsWritableBasicFile() */);

	menuPopup->Enable(IDM_MAKE_DIRECTORY, opened && CanMakeDirectory());

	opened = (opened && (list->GetListSelectedItemCount() > 0));
	menuPopup->Enable(IDM_EXPORT_FILE, opened);
	menuPopup->Enable(IDM_DELETE_FILE, opened /* && IsDeletableBasicFile() */);

	opened = (opened && (list->GetListSelectedRow() != wxNOT_FOUND));
	menuPopup->Enable(IDM_RENAME_FILE, opened);
	menuPopup->Enable(IDM_PROPERTY, opened);

	PopupMenu(menuPopup);
}

/// BASIC種類テキストボックスに設定
void L3DiskFileList::SetAttr(const wxString &val)
{
	textAttr->SetValue(val);
}

/// BASIC種類テキストボックスをクリア
void L3DiskFileList::ClearAttr()
{
	textAttr->Clear();
}

/// キャラクターコード変更
void L3DiskFileList::ChangeCharCode(int sel)
{
	if (basic) {
		basic->SetCharCode(sel);
		RefreshFiles();
	}
	PushCharCode(sel);
}

/// キャラクターコードの選択位置を変える
void L3DiskFileList::PushCharCode(int sel)
{
	radCharAscii->SetValue(sel == 0);
	radCharSJIS->SetValue(sel == 1);
}

/// ファイル名をリストにセット
/// @param [in] newdisk    ディスク
/// @param [in] newsidenum サイド番号 -1:両面 >0:裏表あり
void L3DiskFileList::AttachDiskBasic(DiskD88Disk *newdisk, int newsidenum)
{
	basic = newdisk->GetDiskBasic(newsidenum);

	disk_selecting = true;
}

/// DISK BASICをデタッチ
void L3DiskFileList::DetachDiskBasic()
{
	disk_selecting = false;

	basic = NULL;
}

/// ファイル名をリストに設定
void L3DiskFileList::SetFiles()
{
	// ファイル名一覧を設定
	RefreshFiles();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// ファイル名をリストに再設定
void L3DiskFileList::RefreshFiles()
{
	list->DeleteAllListItems();
	btnChange->Enable(false);

	if (!disk_selecting) return;
	if (!basic) return;
	if (basic->GetBasicTypeName().IsEmpty()) return;

	// 属性をセット
	SetAttr(basic->GetDescriptionDetail());
	btnChange->Enable(true);

	// キャラクターコードをセット
	PushCharCode(basic->GetCharCode());

	// ファイル名一覧を設定
	DiskBasicDirItems *dir_items = basic->GetCurrentDirectoryItems();
	if (dir_items) {
		list->SetListData(basic, dir_items);
	}

#ifndef __WXGTK__
//	list->Scroll(0, 0);
	list->ScrollWindow(0, 0);
#endif

	// FAT空き状況を確認
	if (frame->GetFatAreaFrame()) {
		frame->SetFatAreaData();
	}
}

/// リストをクリア
void L3DiskFileList::ClearFiles()
{
	btnChange->Enable(false);

	list->DeleteAllListItems();
	ClearAttr();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// 行選択
/// @param [in] group_num 開始グループ番号
/// @param [in] sector    セクタ
/// @param [in] char_code キャラクターコード
/// @param [in] invert    データ反転するか
void L3DiskFileList::SelectItem(wxUint32 group_num, DiskD88Sector *sector, int char_code, bool invert)
{
	// ダンプリストをセット
	frame->SetBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize(), char_code, invert);
	// FAT使用状況を更新
	frame->SetFatAreaGroup(group_num);

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

// 行非選択
void L3DiskFileList::UnselectItem()
{
//	// ダンプリストをクリア
//	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// データをダンプウィンドウに設定
bool L3DiskFileList::SetDumpData(DiskBasicGroupItem &group_item)
{
	if (!basic) return false;

	DiskD88Disk *disk = basic->GetDisk();
	DiskD88Track *ptrack = disk->GetTrack(group_item.track, group_item.side);
	if (!ptrack) {
		return false;
	}
	for(int s=group_item.sector_start; s<=group_item.sector_end; s++) {
		DiskD88Sector *sector;
		sector = ptrack->GetSector(s);
		if (!sector) {
			break;
		}
		if (s == group_item.sector_start) {
			frame->SetBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize());
		} else {
			frame->AppendBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize());
		}
	}
	return true;
}

/// データをダンプウィンドウに設定
bool L3DiskFileList::SetDumpData(int track, int side, int sector_start, int sector_end)
{
	if (!basic) return false;

	DiskD88Disk *disk = basic->GetDisk();
	DiskD88Track *ptrack = disk->GetTrack(track, side);
	if (!ptrack) {
		return false;
	}
	for(int s=sector_start; s<=sector_end; s++) {
		DiskD88Sector *psector = ptrack->GetSector(s);
		if (!psector) {
			break;
		}
		if (s == sector_start) {
			frame->SetBinDumpData(psector->GetIDC(), psector->GetIDH(), psector->GetIDR(), psector->GetSectorBuffer(), psector->GetSectorSize());
		} else {
			frame->AppendBinDumpData(psector->GetIDC(), psector->GetIDH(), psector->GetIDR(), psector->GetSectorBuffer(), psector->GetSectorSize());
		}
	}
	return true;
}

/// 現在選択している行のディレクトリアイテムを得る
DiskBasicDirItem *L3DiskFileList::GetSelectedDirItem()
{
	if (!basic) return NULL;
	if (!list) return NULL;
	int row = list->GetListSelectedRow();
	if (row == wxNOT_FOUND) return NULL;
	L3FileListItem view_item = list->GetListSelection();
	return GetDirItem(view_item);
}

/// リストの指定行のディレクトリアイテムを得る
DiskBasicDirItem *L3DiskFileList::GetDirItem(const L3FileListItem &view_item, int *item_pos)
{
	if (!basic) return NULL;
	if (!list) return NULL;
	if (list->GetItemCount() <= 0) return NULL;
	size_t dir_pos = list->GetListItemData(view_item);
	if (item_pos) *item_pos = (int)dir_pos;
	DiskBasicDirItem *dir_item = basic->GetDirItem(dir_pos);
	return dir_item;
}

/// リストの指定行のディレクトリアイテムとそのファイル名を得る
DiskBasicDirItem *L3DiskFileList::GetFileName(const L3FileListItem &view_item, wxString &name, int *item_pos)
{
	DiskBasicDirItem *dir_item = GetDirItem(view_item, item_pos);
	if (!dir_item) return NULL;

	name = dir_item->GetFileNameStr();

	return dir_item;
}

/// エクスポートダイアログ
bool L3DiskFileList::ShowExportDataFileDialog()
{
	if (!basic) return false;

	bool sts = true;

	L3FileListItems selected_items;
	int selcount = list->GetListSelections(selected_items);
	if (selcount <= 0) return false;

	if (selcount == 1) {
		// １つだけ選択
		wxString filename;
		DiskBasicDirItem *item = GetFileName(selected_items.Item(0), filename);
		if (!item) {
			return false;
		}

		// エクスポートする前にダイアログを表示するか
		if (!item->PreExportDataFile(filename)) {
			return false;
		}

		L3DiskFileDialog dlg(
			_("Export a file"),
			frame->GetIniExportFilePath(),
			filename,
			_("All files (*.*)|*.*"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		int dlgsts = dlg.ShowModal();
		if (dlgsts != wxID_OK) {
			return false;
		}

		wxString path = dlg.GetPath();

		sts = ExportDataFile(item, path);

	} else {
		// 複数選択時

		// ディレクトリを選択
		L3DiskDirDialog dlg(
			_("Choose folder to export files"),
			frame->GetIniExportFilePath()
		);

		int dlgsts = dlg.ShowModal();
		if (dlgsts != wxID_OK) {
			return false;
		}

		wxString dir_path = dlg.GetPath();

		for(int n = 0; n < selcount; n++) {
			wxString filename;
			DiskBasicDirItem *item = GetFileName(selected_items.Item(n), filename);
			if (!item) {
				continue;
			}
			wxFileName file_path(dir_path, filename);

			sts = ExportDataFile(item, file_path.GetFullPath());
		}
	}

	return sts;
}

/// 指定したファイルにエクスポート
bool L3DiskFileList::ExportDataFile(DiskBasicDirItem *item, const wxString &path)
{
	if (!basic) return false;

	frame->SetIniExportFilePath(path);

	bool sts = basic->LoadFile(item, path);
	if (!sts) {
		basic->ShowErrorMessage();
	}
	return sts;
}

/// ドラッグする
bool L3DiskFileList::DragDataSource()
{
	wxString tmp_dir_name;
	wxDataObjectComposite compo;

	wxCustomDataObject *custom_object = NULL;
	wxFileDataObject *file_object = NULL;

	bool sts = true;
	if (sts) {
		custom_object = new wxCustomDataObject(*L3DiskPanelDataFormat);
		sts = CreateCustomDataObject(*custom_object);
	}
	if (sts) {
		file_object = new wxFileDataObject();
		sts = CreateFileObject(tmp_dir_name, *file_object);
	}
	if (!sts) {
		delete file_object;
		delete custom_object;
		return false;
	}
	// ファイルデータは外部用
	compo.Add(file_object);
	// カスタムデータを優先（内部用）
	compo.Add(custom_object);

#ifdef __WXMSW__
	wxDropSource dragSource(compo);
#else
	wxDropSource dragSource(compo, frame);
#endif
	dragSource.DoDragDrop();

	return true;
}

// クリップボードへコピー
bool L3DiskFileList::CopyToClipboard()
{
	wxString tmp_dir_name;
	wxFileDataObject *file_object = new wxFileDataObject();
	bool sts = CreateFileObject(tmp_dir_name, *file_object);
	if (sts) {
		if (wxTheClipboard->Open())	{
		    // This data objects are held by the clipboard,
			// so do not delete them in the app.
			wxTheClipboard->SetData(file_object);
			wxTheClipboard->Close();
		}
	} else {
		delete file_object;
	}
	return sts;
}

// カスタムデータリスト作成（DnD, クリップボード用）
bool L3DiskFileList::CreateCustomDataObject(wxCustomDataObject &data_object)
{
	if (!basic) return false;

	if (!list) return false;

	L3FileListItems selected_items;
	int selcount = list->GetListSelections(selected_items);
	if (selcount <= 0) return false;

	// 同じリストへのDnD
	wxMemoryOutputStream ostream;

	for(int n = 0; n < selcount; n++) {
		// ファイルパスを作成
		DiskBasicDirItem *item = GetDirItem(selected_items.Item(n));
		if (!item) continue;
		// コピーできるか
		if (!item->IsCopyable()) continue;

		// ディレクトリヘッダ部分を設定
		directory_for_dnd_t dnd_header;
		memset(&dnd_header, 0, sizeof(dnd_header));
		DiskBasicFileType file_type = item->GetFileAttr();
		item->GetFileName(dnd_header.name, sizeof(dnd_header.name));
		dnd_header.file_size = item->GetFileSize();
		dnd_header.file_type = file_type.GetType();
		dnd_header.original_type = file_type.GetOrigin();
		dnd_header.start_addr = item->GetStartAddress();
		dnd_header.exec_addr = item->GetExecuteAddress();
		dnd_header.external_attr = item->GetExternalAttr();
		dnd_header.format_type = basic->GetFormatTypeNumber();
		struct tm tm;
		item->GetFileDateTime(&tm);
		L3DiskUtils::ConvTmToDateTime(&tm, dnd_header.date,	dnd_header.time);
		wxFileOffset ostart = ostream.TellO();
		ostream.Write(&dnd_header, sizeof(directory_for_dnd_t));

		// ファイルの読み込み
		size_t outsize = 0;
		bool sts = basic->AccessData(item, NULL, &ostream, &outsize);
		if (!sts) return false;

		wxFileOffset oend = ostream.TellO();
		dnd_header.data_size = (wxUint32)outsize;
		ostream.SeekO(ostart);
		ostream.Write(&dnd_header, sizeof(directory_for_dnd_t));
		ostream.SeekO(oend);
	}

	wxStreamBuffer *buffer = ostream.GetOutputStreamBuffer();
	data_object.SetData(buffer->GetBufferSize(), buffer->GetBufferStart());

	return true;
}

/// ファイルをテンポラリディレクトリにエクスポートしファイルリストを作成する（DnD, クリップボード用）
bool L3DiskFileList::CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object)
{
	if (!basic) return false;

	if (!list) return false;

	L3FileListItems selected_items;
	int selcount = list->GetListSelections(selected_items);
	if (selcount <= 0) return false;

	L3DiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	if (!app->MakeTempDir(tmp_dir_name)) {
		return false;
	}

	for(int n = 0; n < selcount; n++) {
		// ファイルパスを作成
		DiskBasicDirItem *item = GetDirItem(selected_items.Item(n));
		if (!item) continue;

		wxString file_name = item->GetFileNameStr();
		if (file_name.IsEmpty()) continue;

		// 必要ならファイル名を変更
		if (!item->PreDropDataFile(file_name)) continue;

		wxFileName file_path(tmp_dir_name, file_name);

		// ファイルパス（フルパス）
		file_name = file_path.GetFullPath();

		bool sts = true;
		wxFileOutputStream ostream(file_name);
		sts = ostream.IsOk();
		if (!sts) {
			continue;
		}

		// ファイルの読み込み
		sts = basic->AccessData(item, NULL, &ostream);
		ostream.Close();

		// ファイルリストに追加
		file_object.AddFile(file_name);
	}

	return true;
}

// ファイルリストを解放（DnD, クリップボード用）
void L3DiskFileList::ReleaseFileObject(const wxString &tmp_dir_name)
{
	L3DiskApp *app = &wxGetApp();

	// テンポラリディレクトリを削除
	app->RemoveTempDir(tmp_dir_name);
}

/// クリップボードからペースト
bool L3DiskFileList::PasteFromClipboard()
{
	if (!basic) return false;

	// Read some text
	wxFileDataObject file_object;

	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported( wxDF_FILENAME )) {
			wxTheClipboard->GetData( file_object );
		}
		wxTheClipboard->Close();
	}

	if (!basic->IsFormatted()) {
		return false;
	}
	if (!basic->IsWritableIntoDisk()) {
		basic->ShowErrorMessage();
		return false;
	}

	wxArrayString file_names = file_object.GetFilenames(); 
	bool sts = true;
	for(size_t n = 0; n < file_names.Count() && sts; n++) {
		sts &= ImportDataFile(file_names.Item(n));
	}
//	if (!sts) {
//		basic->ShowErrorMessage();
//	}
	return sts;
}

/// インポートダイアログ
bool L3DiskFileList::ShowImportDataFileDialog()
{
	if (!basic) return false;

	if (!basic->IsFormatted()) {
		return false;
	}
	if (!basic->IsWritableIntoDisk()) {
		basic->ShowErrorMessage();
		return false;
	}

	L3DiskFileDialog dlg(
		_("Import file"),
		frame->GetIniExportFilePath(),
		wxEmptyString,
		_("All files (*.*)|*.*"),
		wxFD_OPEN);

	int dlgsts = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (dlgsts == wxID_OK) {
		return ImportDataFile(path);
	} else {
		return false;
	}
}

/// ファイル名ダイアログ表示と同じファイル名が存在する際のメッセージダイアログ表示
int L3DiskFileList::ShowIntNameBoxAndCheckSameFile(IntNameBox &dlg, DiskBasicDirItem *item, int file_size)
{
	int ans = wxYES;
	DiskBasicFileName intname;
	do {
		int dlgsts = dlg.ShowModal();
		dlg.GetInternalName(intname);

		if (dlgsts == wxID_OK) {
			// ファイルサイズのチェック
			int limit = 0;
			if (!item->IsFileValidSize(&dlg, file_size, &limit)) {
				wxString msg = wxString::Format(_("File size is larger than %d bytes, do you want to continue?"), limit); 
				ans = wxMessageBox(msg, _("File is too large"), wxYES_NO | wxCANCEL);
				if (ans == wxNO) continue;
				else if (ans == wxCANCEL) break;
			} else {
				ans = wxYES;
			}
		} else {
			ans = wxCANCEL;
			break;
		}

		if (ans == wxYES) {
			// ファイル名重複チェック
			int sts = basic->IsFileNameDuplicated(intname);
			if (sts < 0) {
				// 既に存在します
				wxString msg = wxString::Format(_("File '%s' already exists and cannot overwrite, please rename it."), intname.GetName());
				ans = wxMessageBox(msg, _("File exists"), wxOK | wxCANCEL);
				if (ans == wxOK) continue;
				else break;
			} else if (sts == 1) {
				// 上書き確認ダイアログ
				wxString msg = wxString::Format(_("File '%s' already exists, do you really want to overwrite it?"), intname.GetName());
				ans = wxMessageBox(msg, _("File exists"), wxYES_NO | wxCANCEL);
				if (ans == wxNO) continue;
				else if (ans == wxCANCEL) break;
			} else {
				ans = wxYES;
			}
		} else {
			ans = wxCANCEL;
			break;
		}
	} while (ans != wxYES && ans != wxCANCEL);

	return ans;
}

/// 指定したファイルをインポート 外部から
bool L3DiskFileList::ImportDataFile(const wxString &path)
{
	if (!basic) return false;

	frame->SetIniExportFilePath(path);

	if (!basic->IsFormatted()) {
		return false;
	}

	// ディスクの残りサイズのチェックと入力ファイルのサイズを得る
	int file_size = 0;
	if (!basic->CheckFile(path, &file_size)) {
		return false;
	}

	// ダイアログ生成
	DiskBasicDirItem *temp_item = basic->CreateDirItem();
	IntNameBox dlg(frame, this, wxID_ANY, _("Import File"), basic, temp_item, path
		, INTNAME_NEW_FILE | INTNAME_SHOW_NO_PROPERTY | INTNAME_INVALID_FILE_TYPE | INTNAME_SPECIFY_FILE_NAME);

	// ダイアログ表示と同じファイルがあるかチェック
	int ans = ShowIntNameBoxAndCheckSameFile(dlg, temp_item, file_size);

	bool sts = false;
	if (ans == wxYES) {
		// ダイアログで指定した値をアイテムに反映
		dlg.SetValuesToDirItem();
		// 機種依存の属性をアイテムに反映
		sts = temp_item->SetAttrInAttrDialog(&dlg, basic->GetErrinfo());
		// ディスク内にセーブする
		DiskBasicDirItem *madeitem = NULL;
		if (sts) {
			sts = basic->SaveFile(path, temp_item, &madeitem);
		}
		if (!sts) {
			basic->ShowErrorMessage();
		}
		RefreshFiles();
	}

	delete temp_item;
	return sts;
}

/// 指定したデータをインポート（内部でのドラッグ＆ドロップ時など）
bool L3DiskFileList::ImportDataFiles(const wxUint8 *buffer, size_t buflen)
{
	if (!basic) return false;

	if (!basic->IsFormatted()) {
		return false;
	}
	if (!basic->IsWritableIntoDisk()) {
		basic->ShowErrorMessage();
		return false;
	}

	const wxUint8 *buffer1 = buffer;
	size_t buflen1 = 0;
	while(buflen1 < buflen) {
		// バッファの始めはヘッダ
		directory_for_dnd_t *dnd_header = (directory_for_dnd_t *)buffer1;
		size_t data_buflen = (size_t)dnd_header->data_size + sizeof(directory_for_dnd_t);
		if (buflen1 + sizeof(directory_for_dnd_t) >= buflen) {
			break;
		}

		if (!ImportDataFile(buffer1, data_buflen)) {
			return false;
		}

		buffer1 += data_buflen;
		buflen1 += data_buflen;
	}
	return true;
}

/// 指定したデータをインポート（内部でのドラッグ＆ドロップ時など）
bool L3DiskFileList::ImportDataFile(const wxUint8 *buffer, size_t buflen)
{
	if (!basic) return false;

	if (!basic->IsFormatted()) {
		return false;
	}
	if (!basic->IsWritableIntoDisk()) {
		basic->ShowErrorMessage();
		return false;
	}

	// バッファの始めはヘッダ
	directory_for_dnd_t *dnd_header = (directory_for_dnd_t *)buffer;
	const wxUint8 *data_buffer = buffer + sizeof(directory_for_dnd_t);
	size_t data_buflen = buflen - sizeof(directory_for_dnd_t);

	// ディスクの残りサイズ
	if (!basic->CheckFile(data_buffer, data_buflen)) {
		return false;
	}

	// ディレクトリアイテムを新規に作成して、各パラメータをセット
	wxString filename;
	DiskBasicDirItem *temp_item = basic->CreateDirItem();
	temp_item->SetFileAttr(DiskBasicFileType(dnd_header->format_type, dnd_header->file_type, dnd_header->original_type));
	filename = temp_item->RemakeFileName(dnd_header->name, sizeof(dnd_header->name));
	temp_item->SetStartAddress(dnd_header->start_addr);
	temp_item->SetExecuteAddress(dnd_header->exec_addr);
	temp_item->SetExternalAttr(dnd_header->external_attr);
	struct tm tm;
	L3DiskUtils::ConvDateTimeToTm(dnd_header->date, dnd_header->time, &tm);
	temp_item->SetFileDateTime(&tm);

	// ダイアログ表示
	IntNameBox dlg(frame, this, wxID_ANY, _("Copy File"), basic, temp_item, filename
		, INTNAME_IMPORT_INTERNAL | INTNAME_SHOW_NO_PROPERTY);

	// ダイアログ表示と同じファイルがあるかチェック
	int ans = ShowIntNameBoxAndCheckSameFile(dlg, temp_item, (int)data_buflen);

	bool sts = false;
	if (ans == wxYES) {
		// ダイアログで指定した値をアイテムに反映
		dlg.SetValuesToDirItem();
		// 機種依存の属性をアイテムに反映
		sts = temp_item->SetAttrInAttrDialog(&dlg, basic->GetErrinfo());
		// ディスク内にセーブ
		DiskBasicDirItem *madeitem = NULL;
		if (sts) {
			sts = basic->SaveFile(data_buffer, data_buflen, temp_item, &madeitem);
		}
		if (!sts) {
			basic->ShowErrorMessage();
		}
		RefreshFiles();
	}

	delete temp_item;
	return sts;
}

/// 指定したファイルを削除
bool L3DiskFileList::DeleteDataFile(DiskBasic *tmp_basic, DiskBasicDirItem *dst_item)
{
	if (!dst_item) return false;

	DiskBasicGroups group_items;
	bool sts = tmp_basic->IsDeletableFile(dst_item, group_items);
	if (sts) {
		bool is_directory = dst_item->IsDirectory();
		wxString filename = dst_item->GetFileNameStr();
		wxString msg = wxString::Format(_("Do you really want to delete '%s'?"), filename);
		int ans = wxMessageBox(msg, is_directory ? _("Delete a directory") : _("Delete a file"), wxYES_NO);
		if (ans != wxYES) {
			return false;
		}
		sts = tmp_basic->DeleteFile(dst_item, group_items);
		if (sts) {
			// リスト更新
			RefreshFiles();
		}
	}
	if (!sts) {
		tmp_basic->ShowErrorMessage();
	}
	return sts;
}

/// 指定したファイルを一括削除
bool L3DiskFileList::DeleteDataFiles(DiskBasic *tmp_basic, L3FileListItems &selected_items)
{
	wxString msg = _("Do you really want to delete selected files?");
	int ans = wxMessageBox(msg, _("Delete files"), wxYES_NO);
	if (ans != wxYES) {
		return false;
	}

	bool sts = true;
	int selcount = (int)selected_items.Count();
	for(int n = 0; n < selcount; n++) {
		wxString filename;
		DiskBasicDirItem *item = GetFileName(selected_items.Item(n), filename);
		if (!item) continue;

		DiskBasicGroups group_items;
		bool del_ok = tmp_basic->IsDeletableFile(item, group_items, false);
		sts &= del_ok;
		if (del_ok) {
//			bool is_directory = item->IsDirectory();
			bool ssts = tmp_basic->DeleteFile(item, group_items);
			sts &= ssts;
		}
	}
	// リスト更新
	RefreshFiles();

	if (!sts) {
		tmp_basic->ShowErrorMessage();
	}

	return sts;
}

/// 指定したファイルを削除
bool L3DiskFileList::DeleteDataFile()
{
	if (!basic) return false;

	bool sts = true;
	L3FileListItems selected_items;
	int selcount = list->GetListSelections(selected_items);
	if (selcount < 0) return false;

	if (!basic->IsDeletableFiles()) {
		basic->ShowErrorMessage();
		return false;
	}

	if (selcount == 1) {
		// 1ファイル選択時
		DiskBasicDirItem *item = GetDirItem(selected_items.Item(0));
		sts = DeleteDataFile(basic, item);
	} else {
		// 複数ファイル選択時
		sts = DeleteDataFiles(basic, selected_items);
	}
	return sts;
}

/// ファイル名の編集開始
void L3DiskFileList::StartEditingFileName()
{
	if (!basic) return;

	if (list->GetListSelectedRow() == wxNOT_FOUND) return;

	L3FileListItem selected_item = list->GetListSelection();

	int pos = (int)list->GetListItemData(selected_item);
	DiskBasicDirItem *ditem = basic->GetDirItem(pos);

	if (!basic->CanRenameFile(ditem)) {
		basic->ShowErrorMessage();
		return;
	}

	StartEditingFileName(selected_item);
}

/// ファイル名の編集開始
void L3DiskFileList::StartEditingFileName(const L3FileListItem &selected_item)
{
	// 編集可能にする
	list->EditListItem(selected_item);
}

/// 指定したファイル名を変更
bool L3DiskFileList::RenameDataFile(const L3FileListItem &view_item, const wxString &newname)
{
	if (!basic) return false;

	DiskBasicDirItem *item = GetDirItem(view_item);
	if (!item) return false;

	if (!basic->CanRenameFile(item)) {
		basic->ShowErrorMessage();
		return false;
	}

	DiskBasicFileName filename;
	filename.SetName(newname);

	// ダイアログ入力後のファイル名文字列を変換
	item->ConvertFromFileNameStr(filename.GetName());
	// 拡張属性を得る
	filename.SetOptional(item->GetOptionalName());

	bool sts = true;
	wxString errmsg;
	IntNameValidator validator(basic, item);
	if (sts && !validator.Validate(frame, filename.GetName())) {
		sts = false;
	}
	if (sts && basic->IsFileNameDuplicated(filename, item) != 0) {
		errmsg = wxString::Format(_("File '%s' already exists."), filename.GetName());
		wxMessageBox(errmsg, _("File exists"), wxOK | wxICON_EXCLAMATION, parent);
		sts = false;
	}
	if (sts) {
		sts = basic->RenameFile(item, filename);
		if (!sts) {
			basic->ShowErrorMessage();
		}
		list->SetListText(view_item, item->GetFileNameStr());
	}
	return sts;
}

/// ダブルクリックしたとき
/// ディレクトリならその内部に入る、それ以外ならプロパティダイアログ表示
void L3DiskFileList::DoubleClicked()
{
	DiskBasicDirItem *item = GetSelectedDirItem();
	if (!item) return;

	if (item->GetFileAttr().IsDirectory()) {
		ChangeDirectory(NULL, 0, item, true);
	} else {
		ShowFileAttr(item);
	}
}

/// ディレクトリをアサインする
/// @param [in] disk     選択したディスク NULLの時、現在のディスク
/// @param [in] side_num AB面ありの時サイド番号 両面なら-1
/// @param [in] dst_item ディレクトリのアイテム
bool L3DiskFileList::AssignDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dst_item)
{
	if (disk) {
		// ディスクを指定した時は、そのディスクのBASICに切り替える
		basic = disk->GetDiskBasic(side_num);
	}
	if (!basic) return false;

	bool sts = basic->AssignDirectory(dst_item);

	return sts;
}

/// ディレクトリを移動する
/// @param [in] disk     選択したディスク NULLの時、現在のディスク
/// @param [in] side_num AB面ありの時サイド番号 両面なら-1
/// @param [in] dst_item 移動先ディレクトリのアイテム
/// @param [in] refresh_list ファイルリストを更新するか
bool L3DiskFileList::ChangeDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dst_item, bool refresh_list)
{
	if (disk) {
		// ディスクを指定した時は、そのディスクのBASICに切り替える
		basic = disk->GetDiskBasic(side_num);
	}
	if (!basic) return false;

	bool sts = basic->ChangeDirectory(dst_item);
	if (sts) {
		// リスト更新
		if (refresh_list) RefreshFiles();
	}
	return sts;
}

/// ディレクトリを削除する
/// @param [in] disk     選択したディスク NULLの時、現在のディスク
/// @param [in] side_num AB面ありの時サイド番号 両面なら-1
/// @param [in] dst_item 削除するディレクトリのアイテム
bool L3DiskFileList::DeleteDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dst_item)
{
	DiskBasic *tmp_basic = basic;
	if (disk) {
		// ディスクを指定した時は、そのディスクのBASICに切り替える
		tmp_basic = disk->GetDiskBasic(side_num);
	}
	if (!tmp_basic) return false;

	return DeleteDataFile(tmp_basic, dst_item);
}

/// ファイル属性プロパティダイアログを表示
void L3DiskFileList::ShowFileAttr()
{
	DiskBasicDirItem *item = GetSelectedDirItem();
	if (!item) return;

	ShowFileAttr(item);
}

/// ファイル属性プロパティダイアログを表示
bool L3DiskFileList::ShowFileAttr(DiskBasicDirItem *item)
{
	IntNameBox *dlg = new IntNameBox(frame, this, wxID_ANY, _("File Attribute"), basic, item,
		wxEmptyString, INTNAME_SHOW_ALL);

	// 占有しているグループを一覧にする
	DiskBasicGroups arr;
	item->GetAllGroups(arr);
	dlg->SetGroups(item->GetGroupSize(), arr);

	// モードレス
	dlg->Show();

	// FAT使用状況を更新
	frame->SetFatAreaGroup(&arr, item->GetExtraGroup());

	return true;
}

/// ファイル属性プロパティダイアログの内容を反映
/// ダイアログ内から呼ばれるコールバック
void L3DiskFileList::AcceptSubmittedFileAttr(IntNameBox *dlg)
{
	if (frame->GetUniqueNumber() != dlg->GetUniqueNumber()) {
		// リストが既に切り替わっている
		return;
	}

	DiskBasic *dlg_basic = dlg->GetDiskBasic();
	// これが有効か
	if (!frame->GetDiskD88().MatchDiskBasic(dlg_basic)) {
		return;
	}

	DiskBasicDirItem *item = dlg->GetDiskBasicDirItem();
	if (!item) return;

	bool sts = true;
	DiskBasicError *err_info = &dlg_basic->GetErrinfo();

	// 書き込み禁止か
	sts = basic->IsWritableIntoDisk();

	struct tm tm;
	dlg->GetDateTime(&tm);
	DiskBasicFileName newname;
	if (sts) {
		dlg->GetInternalName(newname);
		if (!item->IsUsed()) {
			// 使われてない 削除した？
			err_info->SetError(DiskBasicError::ERRV_ALREADY_DELETED, newname.GetName().wc_str());
			sts = false;
		}
	}
	if (sts) {
		if (item->IsFileNameEditable()) {
			// 同じファイル名があるか
			if (dlg_basic->IsFileNameDuplicated(newname, item) != 0) {
				err_info->SetError(DiskBasicError::ERRV_ALREADY_EXISTS, newname.GetName().wc_str());
				sts = false;
			}
		}
	}
	if (sts) {
		// 更新できるか
		sts = dlg_basic->CanRenameFile(item, false);
	}
	if (sts) {
		// 名前を更新
		sts = dlg_basic->RenameFile(item, newname);
	}
	if (sts) {
		// その他の属性更新
		sts = item->SetAttrInAttrDialog(dlg, *err_info);
	}
	if (sts) {
		// 残りの属性を更新
		sts = dlg_basic->ChangeAttr(item, dlg->GetStartAddress(), dlg->GetExecuteAddress(), &tm);
	}
	if (!sts) {
		dlg_basic->ShowErrorMessage();
	}

	if (basic == dlg_basic) {
		RefreshFiles();
	}
}

/// プロパティダイアログを閉じる
/// ダイアログはモードレスで複数開いていることがある。
void L3DiskFileList::CloseAllFileAttr()
{
	wxWindowList::compatibility_iterator node;
	for (node = m_children.GetFirst(); node; node = node->GetNext()) {
		wxWindow *child = node->GetData();
		if (child->IsTopLevel()) {
			if (child->GetName() == wxT(INTNAMEBOX_CLASSNAME)) {
				child->Close();
			}
		}
	}
}

/// BASIC種類を変更
bool L3DiskFileList::ChangeBasicType()
{
	if (!basic) return false;

	DiskD88Disk *disk = basic->GetDisk();
	if (!disk) return false;

	BasicParamBox dlg(this, wxID_ANY, _("Select BASIC Type"), disk, basic, BasicParamBox::BASIC_SELECTABLE);
	int sts = dlg.ShowModal();

	if (sts == wxID_OK) {
		if (dlg.IsChangedBasic()) {
			ClearFiles();
			basic->ClearParseAndAssign();
			frame->RefreshDiskListOnSelectedSide(dlg.GetBasicParam());
		} else {
			dlg.CommitData();
			RefreshFiles();
		}
	}
	return (sts == wxID_OK);
}

/// BASIC情報ダイアログ
void L3DiskFileList::ShowBasicAttr()
{
	if (!basic) return;

	DiskD88Disk *disk = basic->GetDisk();
	if (!disk) return;

	BasicParamBox dlg(this, wxID_ANY, _("BASIC Information"), disk, basic, 0);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		dlg.CommitData();
		RefreshFiles();
	}
}

/// ディレクトリを作成できるか
bool L3DiskFileList::CanMakeDirectory() const
{
	return basic ? basic->CanMakeDirectory() : false;
}

/// ディレクトリを作成
bool L3DiskFileList::MakeDirectory()
{
	if (!basic) return false;

	DiskD88Disk *disk = basic->GetDisk();
	if (!disk) return false;

	// 名前の入力
	DiskBasicDirItem *temp_item = basic->CreateDirItem();
	IntNameBox dlg(frame, this, wxID_ANY, _("New Directory Name"), basic, temp_item, wxEmptyString
		, INTNAME_NEW_FILE | INTNAME_SHOW_TEXT | INTNAME_SPECIFY_FILE_NAME);

	int ans = dlg.ShowModal();

	bool sts = false;
	if (ans == wxID_OK) {
		DiskBasicDirItem *madeitem = NULL;
		DiskBasicFileName dirname;
		dlg.GetInternalName(dirname);
		sts = basic->MakeDirectory(dirname.GetName(), &madeitem);
		if (!sts) {
			basic->ShowErrorMessage();
		} else {
			RefreshFiles();
		}
	}

	delete temp_item;
	return sts;
}

/// 選択している行数
int L3DiskFileList::GetListSelectedItemCount() const
{
	return list->GetListSelectedItemCount();
}

/// BASICディスクとして使用できるか
bool L3DiskFileList::CanUseBasicDisk() const
{
	return basic ? basic->CanUse() : false;
}
/// BASICディスク＆フォーマットされているか
bool L3DiskFileList::IsFormattedBasicDisk() const
{
	return basic ? basic->IsFormatted() : false;
}
/// ファイルの書き込み可能か
bool L3DiskFileList::IsWritableBasicFile()
{
	return basic ? basic->IsWritableIntoDisk() : false;
}
/// ファイルの削除可能か
bool L3DiskFileList::IsDeletableBasicFile()
{
	return basic ? basic->IsDeletableFiles() : false;
}
/// FATエリアの空き状況を取得
void L3DiskFileList::GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const
{
	if (basic) basic->GetFatAvailability(offset, arr);
}
/// フォントをセット
void L3DiskFileList::SetListFont(const wxFont &font)
{
	list->SetFont(font);
	Refresh();
}
