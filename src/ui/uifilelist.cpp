/// @file uifilelist.cpp
///
/// @brief BASICファイル名一覧
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uifilelist.h"
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/numformatter.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include "../main.h"
#include "../config.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basicdir.h"
#include "../basicfmt/basicdiritem.h"
#include "intnamebox.h"
#include "intnamevalid.h"
#include "basicselbox.h"
#include "basicparambox.h"
#include "../diskimg/diskresult.h"
#include "../utils.h"


#ifndef USE_LIST_CTRL_ON_FILE_LIST
#else
#include <wx/imaglist.h>
#endif

extern const char *fileicon_normal_xpm[];
extern const char *foldericon_close_xpm[];
extern const char *labelicon_normal_xpm[];
extern const char *fileicon_delete_xpm[];
extern const char *fileicon_hidden_xpm[];

//////////////////////////////////////////////////////////////////////

const char **icons_for_flist[] = {
	fileicon_normal_xpm,
	foldericon_close_xpm,
	labelicon_normal_xpm,
	fileicon_delete_xpm,
	fileicon_hidden_xpm,
	NULL
};
enum en_icons_for_flist {
	ICON_FOR_LIST_NONE = -1,
	ICON_FOR_LIST_FILE_NORMAL = 0,
	ICON_FOR_LIST_FOLDER,
	ICON_FOR_LIST_LABEL,
	ICON_FOR_LIST_FILE_DELETE,
	ICON_FOR_LIST_FILE_HIDDEN,
};

/// ファイル一覧の各カラム属性
const struct st_list_columns gL3DiskFileListColumnDefs[] = {
	{ "Name",		wxTRANSLATE("File Name"),		true,	160,	wxALIGN_LEFT,	true },
	{ "Attr",		wxTRANSLATE("Attributes"),		false,	150,	wxALIGN_LEFT,	false },
	{ "Size",		wxTRANSLATE("Size"),			false,	 60,	wxALIGN_RIGHT,	true },
	{ "Groups",		wxTRANSLATE("Groups"),			false,	 40,	wxALIGN_RIGHT,	true },
	{ "Start",		wxTRANSLATE("Start Group"),		false,	 40,	wxALIGN_RIGHT,	true },
	{ "Track",		wxTRANSLATE("Track"),			false,	 40,	wxALIGN_RIGHT,	false },
	{ "Side",		wxTRANSLATE("Side"),			false,	 40,	wxALIGN_RIGHT,	false },
	{ "Sector",		wxTRANSLATE("Sector"),			false,	 40,	wxALIGN_RIGHT,	false },
	{ "Division",	wxTRANSLATE("Division"),		false,	 40,	wxALIGN_RIGHT,	false },
	{ "Date",		wxTRANSLATE("Date Time"),		false,	150,	wxALIGN_LEFT,	true },
	{ "StartAddr",	wxTRANSLATE("Load Address"),	false,	 60,	wxALIGN_RIGHT,	false },
	{ "EndAddr",	wxTRANSLATE("End Address"),		false,	 60,	wxALIGN_RIGHT,	false },
	{ "ExecAddr",	wxTRANSLATE("Execute Address"),	false,	 60,	wxALIGN_RIGHT,	false },
	{ "Num",		wxTRANSLATE("Num"),				false,	 40,	wxALIGN_RIGHT,	true },
	{ NULL,			NULL,							false,	  0,	wxALIGN_LEFT,	false }
};

#ifdef DeleteFile
#undef DeleteFile
#endif

//////////////////////////////////////////////////////////////////////

#ifndef USE_LIST_CTRL_ON_FILE_LIST
//
//
//
L3DiskFileListStoreModel::L3DiskFileListStoreModel(L3DiskFrame *parentframe, wxWindow *parent)
	: wxDataViewListStore()
{
	frame = parentframe;
	ctrl = (L3DiskFileList *)parent;
}
bool L3DiskFileListStoreModel::SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
{
	// 編集後のファイル名を反映しない
	return false;
}
int L3DiskFileListStoreModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const
{
	DiskBasic *basic = ctrl->GetDiskBasic();
	if (!basic) return 0;

	DiskBasicDirItems *dir_items = basic->GetCurrentDirectoryItems();
	if (!dir_items) return 0;

	int idx = -1;
	if (!ctrl->GetListCtrl()->FindColumn(col, &idx)) return 0;

	int cmp = 0;
	int i1 = (int)GetItemData(item1);
	int i2 = (int)GetItemData(item2);
	int dir = ascending ? 1 : -1;
	switch(idx) {
	case LISTCOL_NAME:
		cmp = L3DiskFileListCtrl::CompareName(dir_items, i1, i2, dir);
		break;
	case LISTCOL_SIZE:
		cmp = L3DiskFileListCtrl::CompareSize(dir_items, i1, i2, dir);
		break;
	case LISTCOL_GROUPS:
		cmp = L3DiskFileListCtrl::CompareGroups(dir_items, i1, i2, dir);
		break;
	case LISTCOL_START:
		cmp = L3DiskFileListCtrl::CompareStart(dir_items, i1, i2, dir);
		break;
#if 0
	case LISTCOL_TRACK:
		cmp = L3DiskFileListCtrl::CompareTrack(dir_items, i1, i2, dir);
		break;
	case LISTCOL_SIDE:
		cmp = L3DiskFileListCtrl::CompareSide(dir_items, i1, i2, dir);
		break;
	case LISTCOL_SECTOR:
		cmp = L3DiskFileListCtrl::CompareSector(dir_items, i1, i2, dir);
		break;
#endif
	case LISTCOL_DATE:
		cmp = L3DiskFileListCtrl::CompareDate(dir_items, i1, i2, dir);
		break;
	case LISTCOL_NUM:
		cmp = L3DiskFileListCtrl::CompareNum(dir_items, i1, i2, dir);
		break;
	default:
		break;
	}
	return cmp;
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
	: L3CDListCtrl(
		parentframe, parent, id,
		gL3DiskFileListColumnDefs,
		&gConfig,
		wxDV_MULTIPLE,
		new L3DiskFileListStoreModel(parentframe, parent),
		pos, size
	)
#else
	: L3CListCtrl(
		parentframe, parent, id,
		gL3DiskFileListColumnDefs,
		-1, -1,
		&gConfig,
		wxLC_EDIT_LABELS,
		pos, size
	)
#endif
{
	// icon
	AssignListIcons(icons_for_flist);
}

/// リストデータを設定
/// @param [in]  basic	DISK BASIC
/// @param [in]  item    ディレクトリアイテム
/// @param [in]  row     行番号
/// @param [in]  num     ディレクトリアイテムの位置
/// @param [out] values  整形したデータ
void L3DiskFileListCtrl::SetListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, L3FileListValue *values)
{
	int icon = ChooseIconNumber(item);
	int track_num = -1;
	int side_num = -1;
	int sector_start = -1;
	int div_num = 0;
	int div_nums = 1;
	if (!basic->CalcStartNumFromGroupNum((wxUint32)item->GetStartGroup(0), track_num, side_num, sector_start, &div_num, &div_nums)) {
		track_num = -1;
		side_num = -1;
		sector_start = -1;
	}

	wxString filename = item->GetFileNameStr();		// ファイル名
	wxString attr =		item->GetFileAttrStr();		// ファイル属性
	int		 size =		item->GetFileSize();		// ファイルサイズ
	int		 groups =	item->GetGroupSize();		// 使用グループ数
	int		 start = 	item->GetStartGroup(0);		// 開始グループ
	wxString date =		item->GetFileDateTimeStr();	// 日時
	int		 staddr =	item->GetStartAddress();	// 開始アドレス
	int		 edaddr = 	item->GetEndAddress();		// 終了アドレス
	int		 exaddr = 	item->GetExecuteAddress();	// 実行アドレス

	values[LISTCOL_NAME].Set(row, icon, filename);
	values[LISTCOL_ATTR].Set(row, attr);
	values[LISTCOL_SIZE].Set(row, size >= 0 ? wxNumberFormatter::ToString((long)size) : wxT("---"));
	values[LISTCOL_GROUPS].Set(row, groups >= 0 ? wxNumberFormatter::ToString((long)groups) : wxT("---"));
	values[LISTCOL_START].Set(row, wxString::Format(wxT("%02x"), start));
	values[LISTCOL_TRACK].Set(row, track_num >= 0 ? wxString::Format(wxT("%d"), track_num) : wxT("-"));
	values[LISTCOL_SIDE].Set(row, side_num >= 0 ? wxString::Format(wxT("%d"), side_num) : wxT("-"));
	values[LISTCOL_SECTOR].Set(row, sector_start >= 0 ? wxString::Format(wxT("%d"), sector_start) : wxT("-"));
	values[LISTCOL_DIVISION].Set(row, div_nums > 0 ? wxString::Format(wxT("%d/%d"), div_num + 1, div_nums) : wxT("-"));
	values[LISTCOL_DATE].Set(row, date);
	values[LISTCOL_STADDR].Set(row, staddr >= 0 ? wxString::Format(wxT("%x"), staddr) : wxT("--"));
	if (staddr >= 0 && edaddr < 0) edaddr = staddr + size - (size > 0 ? 1 : 0);
	values[LISTCOL_EDADDR].Set(row, staddr >= 0 && edaddr >= 0 ? wxString::Format(wxT("%x"), edaddr) : wxT("--"));
	values[LISTCOL_EXADDR].Set(row, exaddr >= 0 ? wxString::Format(wxT("%x"), exaddr) : wxT("--"));
	values[LISTCOL_NUM].Set(row, wxString::Format(wxT("%d"), num));
}

/// リストにデータを挿入
/// @param [in] basic	DISK BASIC
/// @param [in] item    ディレクトリアイテム
/// @param [in] row     行番号
/// @param [in] num     ディレクトリアイテムの位置
/// @param [in] data    ディレクトリアイテムの位置
void L3DiskFileListCtrl::InsertListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, wxUIntPtr data)
{
	L3FileListValue values[LISTCOL_END];

	SetListData(basic, item, row, num, values);

	InsertListItem(row, values, LISTCOL_END, data);
}

/// リストデータを更新
/// @param [in] basic	DISK BASIC
/// @param [in] item    ディレクトリアイテム
/// @param [in] row     行番号
/// @param [in] num     ディレクトリアイテムの位置
/// @param [in] data    ディレクトリアイテムの位置
/// @return 行番号
void L3DiskFileListCtrl::UpdateListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, wxUIntPtr data)
{
	L3FileListValue values[LISTCOL_END];

	SetListData(basic, item, row, num, values);

	UpdateListItem(row, values, LISTCOL_END, data);
}

/// アイコンを指定
/// @param [in] item    ディレクトリアイテム
int L3DiskFileListCtrl::ChooseIconNumber(const DiskBasicDirItem *item) const
{
	DiskBasicFileType file_type = item->GetFileAttr();
	int icon = -1;
	if (!item->IsUsed()) {
		icon = ICON_FOR_LIST_FILE_DELETE;
	} else if (!item->IsVisible()) {
		icon = ICON_FOR_LIST_FILE_HIDDEN;
	} else if (file_type.IsDirectory()) {
		icon = ICON_FOR_LIST_FOLDER;
	} else if (file_type.IsVolume()) {
		icon = ICON_FOR_LIST_LABEL;
	} else {
		icon = ICON_FOR_LIST_FILE_NORMAL;
	}
	return icon;
}

/// リストにデータを設定する
/// @param [in] basic       DISK BASIC
void L3DiskFileListCtrl::SetListItems(DiskBasic *basic)
{
	long row = 0;
	long row_count = (long)GetItemCount();

	DiskBasicDirItems *dir_items = basic->GetCurrentDirectoryItems();
	if (dir_items) {
		bool show_all = gConfig.IsShownDeletedFile();

		for(int idx = 0; idx < (int)dir_items->Count(); idx++) {
			DiskBasicDirItem *item = dir_items->Item(idx);
			if (!show_all && !item->IsUsedAndVisible()) continue;

			if (row < row_count) {
				UpdateListData(basic, item, row, idx, (wxUIntPtr)idx);
			} else {
				InsertListData(basic, item, row, idx, (wxUIntPtr)idx);
			}
			row++;
		}
	}
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	// 余分な行は消す
	for(long idx = row; idx < row_count; idx++) {
		DeleteItem((unsigned)row);
	}
#else
#ifndef USE_VIRTUAL_ON_LIST_CTRL
	// 余分な行は消す
	for(long idx = row; idx < row_count; idx++) {
		DeleteItem(row);
	}
#else
	SetItemCount(row);
#endif

	// ソート
	SortDataItems(basic, -1);
#endif
}

/// リストデータを更新
/// @param [in] basic       DISK BASIC
void L3DiskFileListCtrl::UpdateListItems(DiskBasic *basic)
{
	DiskBasicDirItems *dir_items = basic->GetCurrentDirectoryItems();
	if (!dir_items) return;

	long count = (long)GetItemCount();
	for(long row = 0; row < count; row++) {
		int idx = (int)GetListItemDataByRow(row);

		DiskBasicDirItem *item = dir_items->Item(idx);

		UpdateListData(basic, item, row, idx, (wxUIntPtr)idx);
	}
}

#ifdef USE_LIST_CTRL_ON_FILE_LIST
//
//
//

/// ソート用アイテム
struct st_file_list_sort_exp {
	DiskBasicDirItems *items;
	int (*cmpfunc)(DiskBasicDirItems *items, int i1, int i2, int dir);
	int dir;
};

/// ソート用コールバック
int wxCALLBACK L3DiskFileListCtrl::Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata)
{
	struct st_file_list_sort_exp *exp = (struct st_file_list_sort_exp *)sortdata;

	int cmp = exp->cmpfunc != NULL ? exp->cmpfunc(exp->items, (int)item1, (int)item2, exp->dir) : 0;
	if (cmp == 0) cmp = ((int)item1 - (int)item2);
	return cmp;
}

/// アイテムをソート
void L3DiskFileListCtrl::SortDataItems(DiskBasic *basic, int col)
{
	struct st_file_list_sort_exp exp;

	// ソート対象か
	int idx;
	bool match_col;
	exp.dir = SelectColumnSortDir(col, idx, match_col);

	DiskBasicDirItems *dir_items = NULL;
	if (basic) {
		dir_items = basic->GetCurrentDirectoryItems();
	}

	if (col >= 0 && match_col) {
		if (dir_items) {
			// ソート
			exp.items = dir_items;
			switch(idx) {
			case LISTCOL_NAME:
				exp.cmpfunc = &CompareName;
				break;
			case LISTCOL_SIZE:
				exp.cmpfunc = &CompareSize;
				break;
			case LISTCOL_GROUPS:
				exp.cmpfunc = &CompareGroups;
				break;
			case LISTCOL_START:
				exp.cmpfunc = &CompareStart;
				break;
#if 0
			case LISTCOL_TRACK:
				exp.cmpfunc = &CompareTrack;
				break;
			case LISTCOL_SIDE:
				exp.cmpfunc = &CompareSide;
				break;
			case LISTCOL_SECTOR:
				exp.cmpfunc = &CompareSector;
				break;
#endif
			case LISTCOL_DATE:
				exp.cmpfunc = &CompareDate;
				break;
			case LISTCOL_NUM:
				exp.cmpfunc = &CompareNum;
				break;
			default:
				exp.cmpfunc = NULL;
				break;
			}
			SortItems(&Compare, (wxIntPtr)&exp);
		}

		SetColumnSortIcon(idx);
	}
}
#endif

int L3DiskFileListCtrl::CompareName(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return items->Item(i1)->GetFileNameStr().CompareTo(items->Item(i2)->GetFileNameStr()) * dir;
}
int L3DiskFileListCtrl::CompareSize(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return (items->Item(i1)->GetFileSize() - items->Item(i2)->GetFileSize()) * dir;
}
int L3DiskFileListCtrl::CompareGroups(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return (items->Item(i1)->GetGroupSize() - items->Item(i2)->GetGroupSize()) * dir;
}
int L3DiskFileListCtrl::CompareStart(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return ((int)items->Item(i1)->GetStartGroup(0) - (int)items->Item(i2)->GetStartGroup(0)) * dir;
}
#if 0
int L3DiskFileListCtrl::CompareTrack(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return ((int)items->Item(i1)->GetStartGroup(0) - (int)items->Item(i2)->GetStartGroup(0)) * dir;
}
int L3DiskFileListCtrl::CompareSide(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return ((int)items->Item(i1)->GetStartGroup(0) - (int)items->Item(i2)->GetStartGroup(0)) * dir;
}
int L3DiskFileListCtrl::CompareSector(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return ((int)items->Item(i1)->GetStartGroup(0) - (int)items->Item(i2)->GetStartGroup(0)) * dir;
}
#endif
int L3DiskFileListCtrl::CompareDate(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	TM tm1, tm2;
	items->Item(i1)->GetFileCreateDateTime(tm1);
	items->Item(i2)->GetFileCreateDateTime(tm2);
	int cmp = (tm1.GetYear() - tm2.GetYear()) * dir;
	if (cmp == 0) cmp = (tm1.GetMonth() - tm2.GetMonth()) * dir;
	if (cmp == 0) cmp = (tm1.GetDay() - tm2.GetDay()) * dir;
	if (cmp == 0) cmp = (tm1.GetHour() - tm2.GetHour()) * dir;
	if (cmp == 0) cmp = (tm1.GetMinute() - tm2.GetMinute()) * dir;
	if (cmp == 0) cmp = (tm1.GetSecond() - tm2.GetSecond()) * dir;
	return cmp;
}
int L3DiskFileListCtrl::CompareNum(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return (i1 - i2) * dir;
}

//////////////////////////////////////////////////////////////////////
//
// ストップウォッチ
//
L3StopWatch::L3StopWatch()
	: wxStopWatch()
{
	m_now_wait_cursor = false;
}
void L3StopWatch::Busy()
{
	if (!m_now_wait_cursor) {
		wxBeginBusyCursor();
		m_now_wait_cursor = true;
	}
	Restart();
}
void L3StopWatch::Restart()
{
	wxWakeUpIdle();
	Start();
}
void L3StopWatch::Finish()
{
	if (m_now_wait_cursor) {
		wxEndBusyCursor();
		m_now_wait_cursor = false;
	}
	wxWakeUpIdle();
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

	EVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK(IDC_VIEW_LIST, L3DiskFileList::OnListColumnContextMenu)
#else
//	EVT_LIST_ITEM_RIGHT_CLICK(IDC_VIEW_LIST, L3DiskFileList::OnListContextMenu)
	EVT_LIST_ITEM_ACTIVATED(IDC_VIEW_LIST, L3DiskFileList::OnListActivated)

	EVT_LIST_ITEM_SELECTED(IDC_VIEW_LIST, L3DiskFileList::OnSelect)
	EVT_LIST_ITEM_DESELECTED(IDC_VIEW_LIST, L3DiskFileList::OnDeselect)

	EVT_LIST_BEGIN_DRAG(IDC_VIEW_LIST, L3DiskFileList::OnBeginDrag)

	EVT_LIST_BEGIN_LABEL_EDIT(IDC_VIEW_LIST, L3DiskFileList::OnFileNameStartEditing)
//	EVT_DATAVIEW_ITEM_EDITING_STARTED(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditingStarted)
	EVT_LIST_END_LABEL_EDIT(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditedDone)

	EVT_LIST_COL_CLICK(IDC_VIEW_LIST, L3DiskFileList::OnColumnClick)
	EVT_LIST_COL_RIGHT_CLICK(IDC_VIEW_LIST, L3DiskFileList::OnListColumnContextMenu)

	EVT_CONTEXT_MENU(L3DiskFileList::OnContextMenu)
#endif

	EVT_BUTTON(IDC_BTN_CHANGE, L3DiskFileList::OnButtonChange)
	EVT_CHOICE(IDC_COMBO_CHAR_CODE, L3DiskFileList::OnChangeCharCode)

	EVT_MENU(IDM_EXPORT_FILE, L3DiskFileList::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, L3DiskFileList::OnImportFile)

	EVT_MENU(IDM_DELETE_FILE, L3DiskFileList::OnDeleteFile)
	EVT_MENU(IDM_RENAME_FILE, L3DiskFileList::OnRenameFile)

	EVT_MENU(IDM_COPY_FILE, L3DiskFileList::OnCopyFile)
	EVT_MENU(IDM_PASTE_FILE, L3DiskFileList::OnPasteFile)

	EVT_MENU(IDM_MAKE_DIRECTORY, L3DiskFileList::OnMakeDirectory)

	EVT_MENU(IDM_PROPERTY, L3DiskFileList::OnProperty)

	EVT_MENU_RANGE(IDM_COLUMN_0, IDM_COLUMN_0 + LISTCOL_END - 1, L3DiskFileList::OnListColumnChange)
	EVT_MENU(IDM_COLUMN_DETAIL, L3DiskFileList::OnListColumnDetail)
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
	m_sc_export = false;
	m_sc_import = false;

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
	comCharCode = new wxChoice(this, IDC_COMBO_CHAR_CODE, wxDefaultPosition, wxDefaultSize);
	const CharCodeChoice *choice = gCharCodeChoices.Find(wxT("main"));
	if (choice) {
		for(size_t i=0; i<choice->Count(); i++) {
			const CharCodeMap *map = choice->Item(i);
			comCharCode->Append( map->GetDescription() );
		}
	}
	szrButtons->Add(comCharCode, flags);

	hbox->Add(szrButtons);
	vbox->Add(hbox);

	wxFont font;
	frame->GetDefaultListFont(font);

	list = new L3DiskFileListCtrl(parentframe, this, IDC_VIEW_LIST);
	textAttr->SetFont(font);
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
	menuPopup->Append(IDM_RENAME_FILE, _("Rena&me"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_COPY_FILE, _("&Copy"));
	menuPopup->Append(IDM_PASTE_FILE, _("&Paste..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MAKE_DIRECTORY, _("Make Directory(&F)..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY, _("P&roperty"));

	// popup on list column header
	menuColumnPopup = NULL;

	// key
	list->Bind(wxEVT_CHAR, &L3DiskFileList::OnChar, this);

	initialized = true;
}

L3DiskFileList::~L3DiskFileList()
{
	delete menuPopup;
	delete menuColumnPopup;
}

/// アイテムを得る
/// @param[in] event リストイベント
/// @return リストアイテム
L3FileListItem L3DiskFileList::GetEventItem(const L3FileListEvent& event) const
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	return event.GetItem();
#else
	return event.GetIndex();
#endif
}

/// リサイズ
/// @param[in] event サイズイベント
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

#ifdef USE_LIST_CTRL_ON_FILE_LIST
/// リストを選択
/// @param[in] event リストイベント
void L3DiskFileList::OnSelect(L3FileListEvent& event)
{
	if (!initialized || !basic) return;

	L3FileListItem selected_item = event.GetIndex();
	if (selected_item == wxNOT_FOUND) return;

	/// SelectItem()を呼ぶ
	SelectItem(selected_item, list->GetListSelectedItemCount());
}

/// リストを非選択にした
/// @param[in] event リストイベント
void L3DiskFileList::OnDeselect(L3FileListEvent& event)
{
	if (!initialized || !basic) return;

	L3FileListItem deselected_item = event.GetIndex();
	if (deselected_item == wxNOT_FOUND) return;

	/// UnselectItem()を呼ぶ
	UnselectItem(deselected_item, list->GetListSelectedItemCount());
}
#else
/// リスト選択行を変更
/// @param[in] event リストイベント
void L3DiskFileList::OnSelectionChanged(L3FileListEvent& event)
{
	if (!initialized || !basic) return;

//	if (list->GetListSelectedRow() == wxNOT_FOUND) {
//		return;
//	}

	int count = list->GetListSelectedItemCount();

	for(int row=0; row<list->GetItemCount(); row++) {
		bool sel = list->IsRowSelected(row);
		int tog = list->GetListSelected(row);
		L3FileListItem item = list->RowToItem(row);
		if (sel && (tog == 0)) {
			SelectItem(item, count);
		} else if (!sel && (tog != 0)) {
			UnselectItem(item, count);
		}
		list->SetListSelected(row, sel ? 1 : 0);
	}
}
#endif

/// リストの編集開始
/// @param[in] event リストイベント
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
/// @param[in] event リストイベント
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
/// @param[in] event リストイベント
void L3DiskFileList::OnFileNameEditedDone(L3FileListEvent& event)
{
	if (event.IsEditCancelled()) return;

#ifndef USE_LIST_CTRL_ON_FILE_LIST
	wxDataViewItem listitem = event.GetItem();
	wxVariant value = event.GetValue();
	wxString type_name = value.GetType();
	if (type_name == wxT("string")) {
		RenameDataFile(listitem, value.GetString());
	} else if (type_name == wxT("wxDataViewIconText")) {
		wxDataViewIconText newvalue;
		newvalue << value;
		RenameDataFile(listitem, newvalue.GetText());
	}
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
/// @param[in] event リストイベント
void L3DiskFileList::OnListContextMenu(L3FileListEvent& event)
{
	ShowPopupMenu();
}

/// リストのカラム上で右クリック
/// @param[in] event リストイベント
void L3DiskFileList::OnListColumnContextMenu(L3FileListEvent& event)
{
	ShowColumnPopupMenu();
}

/// セクタリスト カラムをクリック
/// @param[in] event リストイベント
void L3DiskFileList::OnColumnClick(L3FileListEvent& event)
{
	int col = event.GetColumn();
#ifdef USE_LIST_CTRL_ON_FILE_LIST
	list->SortDataItems(basic, col);
#endif
}

/// 右クリック
void L3DiskFileList::OnContextMenu(wxContextMenuEvent& WXUNUSED(event))
{
	ShowPopupMenu();
}

/// リスト上でダブルクリック
/// @param[in] event リストイベント
void L3DiskFileList::OnListActivated(L3FileListEvent& WXUNUSED(event))
{
	DoubleClicked();
}

/// リスト上でドラッグ開始
/// @param[in] event リストイベント
void L3DiskFileList::OnBeginDrag(L3FileListEvent& WXUNUSED(event))
{
	// ドラッグ
	DragDataSource();
}

/// エクスポート選択
/// @param[in] event コマンドイベント
void L3DiskFileList::OnExportFile(wxCommandEvent& WXUNUSED(event))
{
	ShowExportDataFileDialog();
}

/// インポート選択
/// @param[in] event コマンドイベント
void L3DiskFileList::OnImportFile(wxCommandEvent& WXUNUSED(event))
{
	ShowImportDataFileDialog();
}

/// 削除選択
/// @param[in] event コマンドイベント
void L3DiskFileList::OnDeleteFile(wxCommandEvent& WXUNUSED(event))
{
	DeleteDataFile();
}

/// リネーム選択
/// @param[in] event コマンドイベント
void L3DiskFileList::OnRenameFile(wxCommandEvent& WXUNUSED(event))
{
	StartEditingFileName();
}

/// コピー選択
/// @param[in] event コマンドイベント
void L3DiskFileList::OnCopyFile(wxCommandEvent& WXUNUSED(event))
{
	CopyToClipboard();
}

/// ペースト選択
/// @param[in] event コマンドイベント
void L3DiskFileList::OnPasteFile(wxCommandEvent& WXUNUSED(event))
{
	PasteFromClipboard();
}

/// ディレクトリ作成選択
/// @param[in] event コマンドイベント
void L3DiskFileList::OnMakeDirectory(wxCommandEvent& WXUNUSED(event))
{
	ShowMakeDirectoryDialog();
}

/// プロパティ選択
/// @param[in] event コマンドイベント
void L3DiskFileList::OnProperty(wxCommandEvent& WXUNUSED(event))
{
	ShowFileAttr();
}

/// リスト上でキー押下
/// @param[in] event キーイベント
void L3DiskFileList::OnChar(wxKeyEvent& event)
{
	int kc = event.GetKeyCode();
	switch(kc) {
	case WXK_RETURN:
		// Enter    ダブルクリックと同じ
		DoubleClicked();
		break;
	case WXK_DELETE:
	case WXK_BACK:
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
	case WXK_CONTROL_A:
		// Ctrl + A すべて選択
		SelectAll();
		break;
	default:
		event.Skip();
		break;
	}
}

/// 変更ボタン押下
/// @param[in] event コマンドイベント
void L3DiskFileList::OnButtonChange(wxCommandEvent& WXUNUSED(event))
{
	ChangeBasicType();
}

/// キャラクターコード変更ボタン押下
/// @param[in] event コマンドイベント
void L3DiskFileList::OnChangeCharCode(wxCommandEvent& event)
{
	int sel = event.GetSelection();
	wxString name = gCharCodeChoices.GetItemName(wxT("main"), (size_t)sel);
	frame->ChangeCharCode(name);
}

/// リストのカラムを変更
/// @param[in] event コマンドイベント
void L3DiskFileList::OnListColumnChange(wxCommandEvent& event)
{
	int id = event.GetId() - IDM_COLUMN_0;

	if (list->ShowColumn(id, event.IsChecked())) {
		// リストを更新
		RefreshFiles();
	}
}

/// リストのカラム詳細設定
/// @param[in] event コマンドイベント
void L3DiskFileList::OnListColumnDetail(wxCommandEvent& WXUNUSED(event))
{
	ShowListColumnDialog();
}

////////////////////////////////////////

/// ポップアップメニュー表示
void L3DiskFileList::ShowPopupMenu()
{
	if (!menuPopup) return;

	bool opened = IsAssignedBasicDisk();
	menuPopup->Enable(IDM_IMPORT_FILE, opened);
	menuPopup->Enable(IDM_PASTE_FILE, opened);

	menuPopup->Enable(IDM_MAKE_DIRECTORY, opened && CanMakeDirectory());

	opened = (opened && (list->GetListSelectedItemCount() > 0));
	menuPopup->Enable(IDM_EXPORT_FILE, opened);
	menuPopup->Enable(IDM_DELETE_FILE, opened);
	menuPopup->Enable(IDM_COPY_FILE, opened);

	opened = (opened && (list->GetListSelectedRow() != wxNOT_FOUND));
	menuPopup->Enable(IDM_RENAME_FILE, opened);
	menuPopup->Enable(IDM_PROPERTY, opened);

	PopupMenu(menuPopup);
}

/// リストカラムのポップアップメニュー表示
void L3DiskFileList::ShowColumnPopupMenu()
{
	list->CreateColumnPopupMenu(menuColumnPopup, IDM_COLUMN_0, IDM_COLUMN_DETAIL);

	PopupMenu(menuColumnPopup);
}

/// BASIC種類テキストボックスに設定
/// @param[in] val 文字列
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
/// @param[in] name コード名
void L3DiskFileList::ChangeCharCode(const wxString &name)
{
	if (basic) {
		basic->SetCharCode(name);
		RefreshFiles();
	}
	PushCharCode(name);
}

/// キャラクターコードの選択位置を変える
/// @param[in] name コード名
void L3DiskFileList::PushCharCode(const wxString &name)
{
	int sel = gCharCodeChoices.IndexOf(wxT("main"), name);
	comCharCode->SetSelection(sel);
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
///
/// リストアイテムを非選択にする。
/// メニューを更新する。
void L3DiskFileList::SetFiles()
{
	// ファイル名一覧を設定
	RefreshFiles();
	// リストを非選択
	list->UnselectAllListItem();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// ファイル名をリストに再設定
void L3DiskFileList::RefreshFiles()
{
	if (!disk_selecting
	 || !basic
	 || basic->GetBasicTypeName().IsEmpty()) {
		list->DeleteAllListItems();
		btnChange->Enable(false);
		return;
	}

	// 属性をセット
	SetAttr(basic->GetDescriptionDetail());
	btnChange->Enable(true);

	// キャラクターコードをセット
	PushCharCode(basic->GetCharCode());

	// ファイル名一覧を設定
	list->Freeze();
	list->SetListItems(basic);
	list->Thaw();

	// FAT空き状況を確認
	if (frame->GetFatAreaFrame()) {
		frame->SetFatAreaData();
	}
}

#if 0
/// リスト内容を更新
void L3DiskFileList::UpdateFiles()
{
	// ファイル名一覧を更新
	list->Freeze();
	list->UpdateListItems(basic);
	list->Thaw();
}
#endif

/// リストをクリア
void L3DiskFileList::ClearFiles()
{
	btnChange->Enable(false);

	list->DeleteAllListItems();
	ClearAttr();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// 全行を選択
void L3DiskFileList::SelectAll()
{
	list->SelectAllListItem();
}

/// 行選択
/// @param[in] selected_item 選択した行の情報
/// @param[in] count         選択した行数
bool L3DiskFileList::SelectItem(const L3FileListItem &selected_item, int count)
{
	DiskBasicDirItem *ditem = basic->GetDirItem(list->GetListItemData(selected_item));
	if (!ditem) {
		return false;
	}

	DiskD88Sector *sector = basic->GetSectorFromGroup(ditem->GetStartGroup(0));
	if (!sector) {
		return false;
	}

	// FAT使用状況を更新
	wxArrayInt extra_group_nums;
	ditem->GetExtraGroups(extra_group_nums);
	frame->SetFatAreaGroup(ditem->GetGroups(), extra_group_nums);

	if (count == 1) {
		// ダンプリストをセット
		frame->SetBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize(), basic->GetCharCode(), basic->IsDataInverted());

		// メニューを更新
		frame->UpdateMenuAndToolBarFileList(this);
	}

	return true;
}

#if 0
/// 行選択
/// @param [in] group_num 開始グループ番号
/// @param [in] sector    セクタ
/// @param [in] char_code キャラクターコード
/// @param [in] invert    データ反転するか
void L3DiskFileList::SelectItem(wxUint32 group_num, DiskD88Sector *sector, const wxString &char_code, bool invert)
{
	// ダンプリストをセット
	frame->SetBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize(), char_code, invert);
	// FAT使用状況を更新
	frame->SetFatAreaGroup(group_num);

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}
#endif

// 行非選択
/// @param[in] deselected_item 選択していた行の情報
/// @param[in] count           選択していた行数
void L3DiskFileList::UnselectItem(const L3FileListItem &deselected_item, int count)
{
	DiskBasicDirItem *ditem = basic->GetDirItem(list->GetListItemData(deselected_item));
	if (!ditem) {
		return;
	}

	// FAT使用状況を更新
	wxArrayInt extra_group_nums;
	ditem->GetExtraGroups(extra_group_nums);
	frame->UnsetFatAreaGroup(ditem->GetGroups(), extra_group_nums);

	// メニューを更新
	if (count == 0) {
		frame->UpdateMenuAndToolBarFileList(this);
	}
}

/// データをダンプウィンドウに設定
/// @param[in] group_item グループ番号
/// @return true:正常 false:トラックなし 
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
/// @param[in] track        トラック番号
/// @param[in] side         サイド番号
/// @param[in] sector_start 開始セクタ番号
/// @param[in] sector_end   終了セクタ番号
/// @return true:正常 false:トラックなし 
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
/// @return ディレクトリアイテム or NULL
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
/// @param[in]  view_item 指定行のリストアイテム
/// @param[out] item_pos  ディレクトリアイテムの位置
/// @return ディレクトリアイテム or NULL
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
/// @param[in]  view_item 指定行のリストアイテム
/// @param[out] name      ディレクトリアイテムのファイル名
/// @param[out] item_pos  ディレクトリアイテムの位置
/// @return ディレクトリアイテム or NULL
DiskBasicDirItem *L3DiskFileList::GetFileName(const L3FileListItem &view_item, wxString &name, int *item_pos)
{
	DiskBasicDirItem *dir_item = GetDirItem(view_item, item_pos);
	if (!dir_item) return NULL;

	name = dir_item->GetFileNameStr();

	return dir_item;
}

/// エクスポートダイアログ
void L3DiskFileList::ShowExportDataFileDialog()
{
	if (!basic) return;

	basic->ClearErrorMessage();

	L3FileListItems selected_items;
	int selcount = list->GetListSelections(selected_items);
	if (selcount <= 0) return;

	wxString filename;
	DiskBasicDirItem *item = NULL;
	if (selcount == 1) {
		// １つだけ選択
		item = GetFileName(selected_items.Item(0), filename);
		if (!item) {
			return;
		}
		// エクスポートできるか
		if (!basic->IsLoadableFile(item)) {
			basic->ShowErrorMessage();
			return;
		}
		// エクスポートする前の処理（ファイル名を変更するか）
		if (!item->PreExportDataFile(filename)) {
			return;
		}
	}

	if (selcount == 1 && item != NULL && !item->IsDirectory()) {
		// ファイルを１つだけ選択

		// ファイル名を変換
		filename = Utils::EncodeFileName(filename);

		L3DiskFileDialog dlg(
			_("Export a file"),
			frame->GetIniExportFilePath(),
			filename,
			_("All files (*.*)|*.*"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		int dlgsts = dlg.ShowModal();
		if (dlgsts != wxID_OK) {
			return;
		}

		wxString path = dlg.GetPath();

		// エクスポート元パスを覚えておく
		frame->SetIniExportFilePath(path);

		ExportDataFile(item, path, _("exporting..."), _("exported."));

	} else {
		// 複数選択時やディレクトリ選択時

		// 出力先フォルダを選択
		L3DiskDirDialog dlg(
			_("Choose folder to export files"),
			frame->GetIniExportFilePath()
		);

		int dlgsts = dlg.ShowModal();
		if (dlgsts != wxID_OK) {
			return;
		}

		wxString dir_path = dlg.GetPath();

		// エクスポート元パスを覚えておく
		frame->SetIniExportFilePath(dir_path, true);

		// 再帰的にエクスポート
		ExportDataFiles(selected_items, dir_path, wxT(""), _("exporting..."), _("exported."));
	}
}

/// 指定したファイルにエクスポート
/// @param[in] item         ディレクトリアイテム
/// @param[in] path         ファイルパス
/// @param[in] start_msg    開始メッセージ
/// @param[in] end_msg      終了メッセージ
/// @return true:OK false:Error
bool L3DiskFileList::ExportDataFile(DiskBasicDirItem *item, const wxString &path, const wxString &start_msg, const wxString &end_msg)
{
	if (!basic) return false;

	m_sc_export = frame->StartStatusCounter(1, start_msg);
	// ロード
	bool valid = basic->LoadFile(item, path);
	// 日付を反映
	if (valid) {
		item->WriteFileDateTime(path);
	}
	frame->IncreaseStatusCounter(m_sc_export);
	frame->FinishStatusCounter(m_sc_export, end_msg);
	if (!valid) {
		basic->ShowErrorMessage();
	}
	return valid;
}

/// 指定したフォルダにエクスポート
/// @param [in]     selected_items 選択したリスト
/// @param [in]     data_dir       データファイル出力先フォルダ
/// @param [in]     attr_dir       属性ファイル出力先フォルダ
/// @param [in]     start_msg      開始メッセージ
/// @param [in]     end_msg        終了メッセージ
/// @param [in,out] file_object    ファイルオブジェクト
/// @return 0:OK >0:Warning <0:Error
int L3DiskFileList::ExportDataFiles(const L3FileListItems &selected_items, const wxString &data_dir, const wxString &attr_dir, const wxString &start_msg, const wxString &end_msg, wxFileDataObject *file_object)
{
	m_sw_export.Start();
	int selcount = (int)selected_items.Count();
	DiskBasicDirItems dir_items;
	for(int i=0; i<selcount; i++) {
		DiskBasicDirItem *item = GetDirItem(selected_items.Item(i));
		if (!item) {
			continue;
		}
		if (!basic->IsLoadableFile(item)) {
			continue;
		}
		dir_items.Add(item);
	}
	m_sc_export = frame->StartStatusCounter(0, start_msg);
	int sts = ExportDataFiles(&dir_items, data_dir, attr_dir, file_object, 0);
	frame->FinishStatusCounter(m_sc_export, end_msg);
	m_sw_export.Finish();
	if (sts != 0) {
		basic->ShowErrorMessage();
	}
	return sts;
}

/// 指定したフォルダにエクスポート
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param [in]     dir_items   選択したリスト
/// @param [in]     data_dir    データファイル出力先フォルダ
/// @param [in]     attr_dir    属性ファイル出力先フォルダ
/// @param [in,out] file_object ファイルオブジェクト
/// @param [in]     depth       深さ
/// @retval  1 警告あり
/// @retval  0 正常
/// @retval -1 エラー
int L3DiskFileList::ExportDataFiles(DiskBasicDirItems *dir_items, const wxString &data_dir, const wxString &attr_dir, wxFileDataObject *file_object, int depth)
{
	if (!dir_items) return 0;
	if (depth > 20) return -1;

	int sts = 0;
	size_t count = dir_items->Count();
	DiskBasicDirItems valid_items;
	for(size_t n = 0; n < count && sts >= 0; n++) {
		DiskBasicDirItem *item = dir_items->Item(n);
		if (!item) {
			continue;
		}
		if (!item->IsUsed()) {
			continue;
		}
		if (!item->IsLoadable()) {
			continue;
		}
		valid_items.Add(item);
	}

	count = valid_items.Count();
	frame->AppendStatusCounter(m_sc_export, (int)count);

	bool attr_exists = !attr_dir.IsEmpty();
	for(size_t n = 0; n < count && sts >= 0; n++) {
		frame->IncreaseStatusCounter(m_sc_export);
		if (m_sw_export.Time() > 3000) {
			m_sw_export.Busy();
		}
		DiskBasicDirItem *item = valid_items.Item(n);

		wxString native_name = item->GetFileNameStr();
		// エクスポートする前の処理（ファイル名を変更するか）
		if (!item->PreExportDataFile(native_name)) {
			sts = -1;
			break;
		}
		if (native_name.IsEmpty()) {
			continue;
		}
		// ファイル名に設定できない文字をエスケープ
		wxString file_name = Utils::EncodeFileName(native_name);
		// フルパスを作成
		wxString full_data_name = wxFileName(data_dir, file_name).GetFullPath();
		wxString full_attr_name = attr_exists ? wxFileName(attr_dir, file_name, wxT("xml")).GetFullPath() : attr_dir;
		if (full_data_name.Length() > 255 || full_attr_name.Length() > 255) {
			// パスが長すぎる
			sts = 1;
			basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
			basic->GetErrinfo().SetError(DiskBasicError::ERR_PATH_TOO_DEEP);
			continue;
		}
		if (item->IsDirectory()) {
			// ディレクトリの場合
			// ディレクトリをアサイン
			bool valid = basic->AssignDirectory(item);
			if (!valid) {
				sts = 1;
				basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
				continue;
			}
			// データサブフォルダを作成
			if (wxFileName::FileExists(full_data_name) || wxFileName::DirExists(full_data_name)) {
				// 既にある
				sts = 1;
				basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
				basic->GetErrinfo().SetError(DiskBasicError::ERR_FILE_ALREADY_EXIST);
				continue;
			}
			if (!wxMkdir(full_data_name)) {
				sts = 1;
				basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
				continue;
			}
			if (attr_exists) {
				// 属性サブフォルダを作成
				if (!wxMkdir(full_attr_name)) {
					sts = 1;
					basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
					continue;
				}
			}
			// 再帰的にエクスポート
			sts |= ExportDataFiles(item->GetChildren(), full_data_name, full_attr_name, file_object, depth + 1);
		} else {
			// ファイルの場合
			bool rc = basic->LoadFile(item, full_data_name);
			sts |= (rc ? 0 : -1);
			// 日付を反映
			if (rc) {
				item->WriteFileDateTime(full_data_name);
			}
			// 属性情報をXMLで出力
			if (attr_exists) {
				item->WriteFileAttrToXml(full_attr_name);
			}
		}

		// ファイルオブジェクトを追加(DnD用)
		// トップレベルのみ追加
		if (depth == 0 && file_object != NULL && sts >= 0) {
			file_object->AddFile(full_data_name);
		}
	}
	return sts;
}

/// ドラッグする
bool L3DiskFileList::DragDataSource()
{
	wxString tmp_dir_name;
	wxDataObjectComposite compo;

	wxFileDataObject *file_object = NULL;

	bool sts = true;
	if (sts) {
		file_object = new wxFileDataObject();
		sts = CreateFileObject(tmp_dir_name, _("dragging..."), _("dragged."), *file_object);
	}
	if (!sts) {
		delete file_object;
		return false;
	}
	// ファイルデータは外部用
	if (file_object) compo.Add(file_object);

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
	bool sts = CreateFileObject(tmp_dir_name, _("copying..."), _("copied."), *file_object);
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

/// ファイルをテンポラリディレクトリにエクスポートしファイルリストを作成する（DnD, クリップボード用）
/// @param [in]     tmp_dir_name テンポラリフォルダ
/// @param [in]     start_msg    開始メッセージ
/// @param [in]     end_msg      終了メッセージ
/// @param [in,out] file_object ファイルオブジェクト
/// @return true:OK false:Error
bool L3DiskFileList::CreateFileObject(wxString &tmp_dir_name, const wxString &start_msg, const wxString &end_msg, wxFileDataObject &file_object)
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
	// データディレクトリを作成
	wxString tmp_data_path = wxFileName(tmp_dir_name, wxT("Datas")).GetFullPath();
	if (!wxMkdir(tmp_data_path)) {
		return false;
	}
	// 属性ディレクトリを作成
	wxString tmp_attr_path = wxFileName(tmp_dir_name, wxT("Attrs")).GetFullPath();
	if (!wxMkdir(tmp_attr_path)) {
		return false;
	}

	ExportDataFiles(selected_items, tmp_data_path, tmp_attr_path, start_msg, end_msg, &file_object);

	return true;
}

/// ファイルリストを解放（DnD, クリップボード用）
/// @param [in]     tmp_dir_name テンポラリフォルダ
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

	// インポート
	return ImportDataFiles(file_object.GetFilenames(), _("pasting..."), _("pasted."));
}

/// インポートダイアログ
void L3DiskFileList::ShowImportDataFileDialog()
{
	if (!basic) return;

	if (!basic->IsFormatted()) {
		return;
	}
	if (!basic->IsWritableIntoDisk()) {
		basic->ShowErrorMessage();
		return;
	}

	L3DiskFileDialog dlg(
		_("Import file"),
		frame->GetIniExportFilePath(),
		wxEmptyString,
		_("All files (*.*)|*.*"),
		wxFD_OPEN | wxFD_MULTIPLE);

	int dlgsts = dlg.ShowModal();

	if (dlgsts != wxID_OK) {
		return;
	}

	// パスを覚えておく
	frame->SetIniExportFilePath(dlg.GetPath());

	wxArrayString paths;
	dlg.GetPaths(paths);
	ImportDataFiles(paths, _("importing..."), _("imported."));
}

/// ファイルをドロップ
/// @param[in] paths ファイルパスのリスト
/// @return true:OK false:Error
bool L3DiskFileList::DropDataFiles(const wxArrayString &paths)
{
	return ImportDataFiles(paths, _("dropping..."), _("dropped."));
}

/// 指定したファイルをインポート
/// @param [in] paths     ファイルパスのリスト
/// @param [in] start_msg 開始メッセージ
/// @param [in] end_msg   終了メッセージ
/// @return true:OK false:Error
bool L3DiskFileList::ImportDataFiles(const wxArrayString &paths, const wxString &start_msg, const wxString &end_msg)
{
	if (!basic) return false;

	m_sc_import = frame->StartStatusCounter(0, start_msg);
	m_sw_import.Start();
	int sts = 0;
	for(size_t n = 0; n < paths.Count() && sts >= 0; n++) {
		wxFileName file_path(paths.Item(n));
		wxString data_dir = file_path.GetPath();
		wxArrayString names;
		names.Add(file_path.GetFullName());
		file_path.RemoveLastDir();
		file_path.AppendDir(wxT("Attrs"));
		wxString attr_dir = file_path.GetPath();
		sts |= ImportDataFiles(data_dir, attr_dir, names, 0);
	}
	m_sw_import.Finish();
	frame->FinishStatusCounter(m_sc_import, end_msg);

	// リストを更新
	RefreshFiles();
	// 左パネルのツリーを更新
	frame->RefreshAllDirectoryNodesOnDiskList(basic->GetDisk(), basic->GetSelectedSide());
	if (sts != 0) {
		basic->ShowErrorMessage();
	}
	return (sts >= 0);
}

/// 指定したファイルをインポート
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param [in] data_dir データフォルダ
/// @param [in] attr_dir 属性フォルダ
/// @param [in] names    ファイル名のリスト
/// @param [in] depth    深さ
/// @retval  1 警告あり
/// @retval  0 正常
/// @retval -1 エラー
int L3DiskFileList::ImportDataFiles(const wxString &data_dir, const wxString &attr_dir, const wxArrayString &names, int depth)
{
	if (depth > 20) return false;

	int sts = 0;
	size_t count = names.Count();
	frame->AppendStatusCounter(m_sc_import, (int)count);
	for(size_t n = 0; n < count && sts >= 0; n++) {
		if (m_sw_import.Time() > 3000) {
			// マウスアイコンを変更
			m_sw_import.Busy();
		}
		frame->IncreaseStatusCounter(m_sc_import);
		wxString name = names.Item(n);
		wxString full_data_path = wxFileName(data_dir, name).GetFullPath();
		wxString full_attr_path = wxFileName(attr_dir, name, wxT("xml")).GetFullPath();
		// ファイル名を変換
		name = Utils::DecodeFileName(name);

		if (wxFileName::DirExists(full_data_path)) {
			// フォルダの場合

			// ディレクトリ作成
			DiskBasicDirItem *dir_item = NULL;
			if (!MakeDirectory(name, _("Import Directory"), &dir_item)) {
				sts = -1;
				break;
			}

			// フォルダ内のファイルリスト
			wxArrayString sub_names;
			wxDir dir(full_data_path);
			wxString sub_name;
			bool valid = dir.GetFirst(&sub_name, wxEmptyString);
			while(valid) {
				sub_names.Add(sub_name);
				valid = dir.GetNext(&sub_name);
			}
			if (sub_names.Count() == 0) {
				continue;
			}

			// サブディレクトリに移動してインポート
			DiskBasicDirItem *cur_item = basic->GetCurrentDirectory();
			if (!basic->ChangeDirectory(dir_item)) {
				sts = -1;
				break;
			}
			// 再帰的にインポート
			sts |= ImportDataFiles(full_data_path, full_attr_path, sub_names, depth + 1);
			basic->ChangeDirectory(cur_item);
		} else {
			// ファイルの場合
			sts |= ImportDataFile(full_data_path, full_attr_path, name);
		}
	}

	return sts;
}

/// 指定したファイルをインポート
/// @param [in] full_data_path データファイルパス
/// @param [in] full_attr_path 属性ファイルパス
/// @param [in] file_name      ファイル名
/// @retval  1 警告あり処理継続
/// @retval  0 正常
/// @retval -1 エラー継続不可
int L3DiskFileList::ImportDataFile(const wxString &full_data_path, const wxString &full_attr_path, const wxString &file_name)
{
	if (!basic) return -1;

	if (!basic->IsFormatted()) {
		return -1;
	}

//	wxString full_data_path = wxFileName(data_dir, name).GetFullPath();

	// ディスクの残りサイズのチェックと入力ファイルのサイズを得る
	int file_size = 0;
	if (!basic->CheckFile(full_data_path, &file_size)) {
		return -1;
	}

	// 外部からインポートのスタイル
	int style = INTNAME_NEW_FILE | INTNAME_SHOW_TEXT | INTNAME_SHOW_ATTR | INTNAME_SPECIFY_FILE_NAME | INTNAME_SHOW_SKIP_DIALOG;

	int sts = 0;
	DiskBasicDirItem *temp_item = basic->CreateDirItem();

	// ファイル情報があれば読み込む
	wxString filename = file_name;
	DiskBasicDirItemAttr date_time;
	if (temp_item->ReadFileAttrFromXml(full_attr_path, &date_time)) {
		// ファイル名
		filename = temp_item->GetFileNameStr();
		// 内部からインポートに変更
		style = INTNAME_IMPORT_INTERNAL | INTNAME_SHOW_TEXT | INTNAME_SHOW_ATTR
			| INTNAME_SPECIFY_CDATE_TIME | INTNAME_SPECIFY_MDATE_TIME | INTNAME_SPECIFY_ADATE_TIME
			| INTNAME_SHOW_SKIP_DIALOG;
		// コピーできるか
		if (!temp_item->IsCopyable()) {
			// エラーにはしない
			sts = 1;
		}
	} else {
		// ファイルから日付を得る
		temp_item->ReadFileDateTime(full_data_path, date_time);
		style |= INTNAME_SPECIFY_CDATE_TIME | INTNAME_SPECIFY_MDATE_TIME | INTNAME_SPECIFY_ADATE_TIME;
	}
	if (sts == 0) {
		// ダイアログ表示
		int ans = ShowIntNameBoxAndCheckSameFile(temp_item, filename, file_size, date_time, style);
		if (ans == wxYES) {
			// ディスク内にセーブする
			DiskBasicDirItem *madeitem = NULL;
			bool valid = basic->SaveFile(full_data_path, temp_item, &madeitem);
			if (!valid) {
				sts = -1;
			}
		} else {
			sts = -1;
		}
	}
	delete temp_item;
	return sts;
}

/// ファイル名ダイアログ表示と同じファイル名が存在する際のメッセージダイアログ表示
/// @param [in] temp_item ディレクトリアイテム
/// @param [in] file_name ファイルパス
/// @param [in] file_size ファイルサイズ
/// @param [in] date_time 日時
/// @param [in] style     スタイル(IntNameBoxShowFlags)
/// @retval wxYES
/// @retval wxCANCEL
int L3DiskFileList::ShowIntNameBoxAndCheckSameFile(DiskBasicDirItem *temp_item, const wxString &file_name, int file_size, DiskBasicDirItemAttr &date_time, int style)
{
	int ans = wxNO;
	bool skip_dlg = gConfig.IsSkipImportDialog();
	IntNameBox *dlg = NULL;
	wxString int_name = file_name;

	// ファイルパスからファイル名を生成
	if (style & INTNAME_NEW_FILE) {
		// 外部からインポート時
		if (!temp_item->PreImportDataFile(int_name)) {
			// エラー
			ans = wxCANCEL;
		}
	}
	while (ans != wxYES && ans != wxCANCEL) {
		if (!skip_dlg) {
			// ファイル名ダイアログを表示
			if (!dlg) dlg = new IntNameBox(frame, this, wxID_ANY, _("Import File"), wxT(""), basic, temp_item, file_name, int_name, file_size, &date_time, style);
			int dlgsts = dlg->ShowModal();
			m_sw_import.Restart();
			if (dlgsts == wxID_OK) {
				dlg->GetInternalName(int_name);
				// ダイアログで指定したファイル名や属性値をアイテムに反映
				if (!SetDirItemFromIntNameDialog(temp_item, *dlg, basic, true)) {
					ans = wxCANCEL;
					break;
				}
				// ファイルサイズのチェック
				int limit = 0;
				if (!temp_item->IsFileValidSize(dlg, file_size, &limit)) {
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
		} else {
			// ダイアログを表示しないとき
			if (style & INTNAME_NEW_FILE) {
				// 外部からインポート時でダイアログなし
				// ファイル名が適正か
				IntNameValidator vali(temp_item, _("file name"), basic->GetValidFileName());
				if (!vali.Validate(this, int_name)) {
					// ファイル名が不適切
					skip_dlg = false;
					ans = wxNO;
					continue;
				}
				// 属性をファイル名から判定してアイテムに反映
				if (!SetDirItemFromIntNameParam(temp_item, file_name, int_name, date_time, basic, true)) {
					ans = wxCANCEL;
					break;
				}
			} else {
				// 内部からインポート時
				bool ignore_datetime = gConfig.DoesIgnoreDateTime();
				DiskBasicDirItem::enDateTime ignore_type = temp_item->CanIgnoreDateTime();
				if (!(ignore_datetime && (ignore_type & DiskBasicDirItem::DATETIME_CREATE) != 0)) {
					date_time.SetCreateDateTime(temp_item->GetFileCreateDateTime());
				}
				if (!(ignore_datetime && (ignore_type & DiskBasicDirItem::DATETIME_MODIFY) != 0)) {
					date_time.SetModifyDateTime(temp_item->GetFileModifyDateTime());
				}
				if (!(ignore_datetime && (ignore_type & DiskBasicDirItem::DATETIME_ACCESS) != 0)) {
					date_time.SetAccessDateTime(temp_item->GetFileAccessDateTime());
				}
			}
			ans = wxYES;
		}

		if (ans == wxYES) {
			// ファイル名重複チェック
			int sts = basic->IsFileNameDuplicated(temp_item);
			if (sts < 0) {
				// 既に存在します 上書き不可
				skip_dlg = false;
				wxString msg = wxString::Format(_("File '%s' already exists and cannot overwrite, please rename it."), temp_item->GetFileNameStr());
				ans = wxMessageBox(msg, _("File exists"), wxOK | wxCANCEL);
				if (ans == wxOK) continue;
				else break;
			} else if (sts == 1) {
				// 上書き確認ダイアログ
				skip_dlg = false;
				wxString msg = wxString::Format(_("File '%s' already exists, do you really want to overwrite it?"), temp_item->GetFileNameStr());
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
	}

	delete dlg;
	return ans;
}

/// ファイル名ダイアログの内容を反映させる
/// @param [in] item   ディレクトリアイテム
/// @param [in] dlg    ファイル名ダイアログ
/// @param [in] basic  BASIC
/// @param [in] rename ファイル名を変更できるか
/// @return true:OK false:Error
bool L3DiskFileList::SetDirItemFromIntNameDialog(DiskBasicDirItem *item, IntNameBox &dlg, DiskBasic *basic, bool rename)
{
	DiskBasicDirItemAttr attr;

	// パラメータを設定に反映
	gConfig.SkipImportDialog(dlg.IsSkipDialog(gConfig.IsSkipImportDialog()));
	if (item->CanIgnoreDateTime()) {
		gConfig.IgnoreDateTime(dlg.DoesIgnoreDateTime(gConfig.DoesIgnoreDateTime()));
	}

	// 属性をアイテムに反映
	wxString newname;

	dlg.GetInternalName(newname);
	attr.Renameable(rename);
	attr.SetFileName(newname, item->GetOptionalNameInAttrDialog(&dlg));

	attr.IgnoreDateTime(gConfig.DoesIgnoreDateTime());
	
	attr.SetCreateDateTime(dlg.GetCreateDateTime());
	attr.SetModifyDateTime(dlg.GetModifyDateTime());
	attr.SetAccessDateTime(dlg.GetAccessDateTime());

	attr.SetStartAddress(dlg.GetStartAddress());
	attr.SetEndAddress(dlg.GetEndAddress());
	attr.SetExecuteAddress(dlg.GetExecuteAddress());

	// 機種依存の属性をアイテムに反映
	bool sts = item->SetAttrInAttrDialog(&dlg, attr, basic->GetErrinfo());

	if (sts) {
		// 必要なら属性値を加工する
		sts = item->ProcessAttr(attr, basic->GetErrinfo());
	}
	if (sts) {
		// 属性を更新
		sts = basic->ChangeAttr(item, attr);
	}

	return sts;
}

/// ファイル名を反映させる
/// @param [in] item      ディレクトリアイテム
/// @param [in] file_path ファイルパス
/// @param [in] intname   内部ファイル名
/// @param [in] date_time 日時
/// @param [in] basic     BASIC
/// @param [in] rename    ファイル名を変更できるか
/// @return true:OK false:Error
bool L3DiskFileList::SetDirItemFromIntNameParam(DiskBasicDirItem *item, const wxString &file_path, const wxString &intname, DiskBasicDirItemAttr &date_time, DiskBasic *basic, bool rename)
{
	DiskBasicDirItemAttr attr;

	// 属性をアイテムに反映
	wxString newname;
//	struct tm tm;

	attr.Renameable(rename);
	attr.SetFileName(intname, item->ConvOptionalNameFromFileName(file_path));

	attr.IgnoreDateTime(gConfig.DoesIgnoreDateTime());
	attr.SetCreateDateTime(date_time.GetCreateDateTime());
	attr.SetModifyDateTime(date_time.GetModifyDateTime());
	attr.SetAccessDateTime(date_time.GetAccessDateTime());

	// ファイル名から属性を設定
	attr.SetFileAttr(basic->GetFormatTypeNumber(), item->ConvFileTypeFromFileName(file_path), item->ConvOriginalTypeFromFileName(file_path));

	bool sts = true;
	if (sts) {
		// 必要なら属性値を加工する
		sts = item->ProcessAttr(attr, basic->GetErrinfo());
	}
	if (sts) {
		// 属性を更新
		sts = basic->ChangeAttr(item, attr);
	}
	return sts;
}

/// 指定したファイルを削除
/// @param[in]     tmp_basic BASIC
/// @param[in,out] dst_item  削除対象アイテム
/// @return 0:OK >0:Warning <0:Error
int L3DiskFileList::DeleteDataFile(DiskBasic *tmp_basic, DiskBasicDirItem *dst_item)
{
	if (!dst_item) return -1;

	int sts = tmp_basic->IsDeletableFile(dst_item);
	if (sts == 0) {
		bool is_directory = dst_item->IsDirectory();
		wxString filename = dst_item->GetFileNameStr();
		wxString msg = wxString::Format(_("Do you really want to delete '%s'?"), filename);
		int ans = wxMessageBox(msg, is_directory ? _("Delete a directory") : _("Delete a file"), wxYES_NO);
		if (ans != wxYES) {
			return -1;
		}
		DiskBasicDirItems dst_items;
		dst_items.Add(dst_item);
		sts = DeleteDataFiles(tmp_basic, dst_items, 0, NULL);

		// リスト更新
		frame->DeleteDirectoryNodeOnDiskList(tmp_basic->GetDisk(), dst_item);
		RefreshFiles();
	}
	if (sts != 0) {
		tmp_basic->ShowErrorMessage();
	}
	return sts;
}

/// 指定したファイルを一括削除
/// @param[in]     tmp_basic       BASIC
/// @param[in,out] selected_items  削除対象アイテムリスト
/// @return 0:OK >0:Warning <0:Error
int L3DiskFileList::DeleteDataFiles(DiskBasic *tmp_basic, L3FileListItems &selected_items)
{
	wxString msg = _("Do you really want to delete selected files?");
	int ans = wxMessageBox(msg, _("Delete files"), wxYES_NO);
	if (ans != wxYES) {
		return false;
	}

	DiskBasicDirItems items;
	int selcount = (int)selected_items.Count();
	for(int n = 0; n < selcount; n++) {
		wxString filename;
		DiskBasicDirItem *item = GetFileName(selected_items.Item(n), filename);
		if (!item) continue;
		items.Add(item);
	}

	int sts = 0;
	DiskBasicDirItems dir_items;
	sts = DeleteDataFiles(tmp_basic, items, 0, &dir_items);

	// リスト更新
	frame->DeleteDirectoryNodesOnDiskList(basic->GetDisk(), dir_items);
	RefreshFiles();

	if (sts != 0) {
		tmp_basic->ShowErrorMessage();
	}

	return sts;
}

/// 指定したファイルを一括削除（再帰的）
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param[in]     tmp_basic       BASIC
/// @param[in,out] items           削除対象アイテムリスト
/// @param[in]     depth           深さ
/// @param[in,out] dir_items       サブディレクトリアイテムリスト
/// @return 0:OK >0:Warning <0:Error
int L3DiskFileList::DeleteDataFiles(DiskBasic *tmp_basic, DiskBasicDirItems &items, int depth, DiskBasicDirItems *dir_items)
{
	if (depth > 20) return 1;

	int sts = 0;
	// 機種によってはアイテムをリストから削除するので予めリストをコピー
	DiskBasicDirItems tmp_items = items;
	size_t tmp_count = (int)items.Count();
	for(size_t n = 0; n < tmp_count && sts >= 0; n++) {
		DiskBasicDirItem *item = tmp_items.Item(n);
		if (!item) {
			continue;
		}
		if (!item->IsUsed()) {
			continue;
		}
		// 削除できるか
		if (!item->IsDeletable()) {
			continue;
		}
		bool is_directory = item->IsDirectory();
		if (is_directory) {
			// アサイン
			tmp_basic->AssignDirectory(item);
			// ディレクトリのときは先にディレクトリ内ファイルを削除
			DiskBasicDirItems *sitems = item->GetChildren();
			if (sitems) {
				int ssts = DeleteDataFiles(tmp_basic, *sitems, depth + 1, NULL);
				if (ssts == 0 && dir_items) {
					dir_items->Add(item);
				}
				sts |= (ssts != 0 ? -1 : 0);
			}
		}
		if (sts >= 0) {
			// 削除
			bool ssts = tmp_basic->DeleteFile(item, false);
			sts |= (ssts ? 0 : -1);
		}
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
		sts = (DeleteDataFile(basic, item) != 0);
	} else {
		// 複数ファイル選択時
		sts = (DeleteDataFiles(basic, selected_items) != 0);
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
/// @param[in] selected_item 選択行のアイテム
void L3DiskFileList::StartEditingFileName(const L3FileListItem &selected_item)
{
	// 編集可能にする
	list->EditListItem(selected_item);
}

/// 指定したファイル名を変更
/// @param[in] view_item 選択行のアイテム
/// @param[in] newname   変更するファイル名
/// @return true:OK false:Error
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

	// ダイアログ入力後のファイル名文字列を大文字に変換
	if (basic->ToUpperAfterRenamed()) {
		filename.GetName().MakeUpper();
	}
//	// ダイアログ入力後のファイル名文字列を変換
//	item->ConvertFileNameAfterRenamed(filename.GetName());
	// 拡張属性を得る
	filename.SetOptional(item->GetOptionalName());

	bool sts = true;
	wxString errmsg;
	IntNameValidator validator(item, _("file name"), basic->GetValidFileName());
	if (sts && !validator.Validate(frame, filename.GetName())) {
		sts = false;
	}
	if (sts && basic->IsFileNameDuplicated(filename, item) != 0) {
		errmsg = wxString::Format(_("File '%s' already exists."), filename.GetName());
		wxMessageBox(errmsg, _("File exists"), wxOK | wxICON_EXCLAMATION, parent);
		sts = false;
	}
	if (sts) {
		sts = basic->RenameFile(item, filename.GetName());
		if (!sts) {
			basic->ShowErrorMessage();
		}
		// リストのファイル名を更新
		list->SetListText(view_item, LISTCOL_NAME, item->GetFileNameStr());
		// ディレクトリのときはツリー側も更新
		if (item->IsDirectory()) {
			frame->RefreshDiskListDirectoryName(basic->GetDisk());
		}
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
/// @return true:OK false:Error
bool L3DiskFileList::AssignDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dst_item)
{
	if (disk) {
		// ディスクを指定した時は、そのディスクのBASICに切り替える
		basic = disk->GetDiskBasic(side_num);
	}
	if (!basic) return false;

	bool sts = basic->AssignDirectory(dst_item);
	if (sts) {
		// リスト更新
		frame->RefreshDirectoryNodeOnDiskList(basic->GetDisk(), dst_item);
	}
	return sts;
}

/// ディレクトリを移動する
/// @param [in] disk     選択したディスク NULLの時、現在のディスク
/// @param [in] side_num AB面ありの時サイド番号 両面なら-1
/// @param [in] dst_item 移動先ディレクトリのアイテム
/// @param [in] refresh_list ファイルリストを更新するか
/// @return true:OK false:Error
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
		if (refresh_list) SetFiles();
		frame->SelectDirectoryNodeOnDiskList(basic->GetDisk(), dst_item);
	}
	return sts;
}

/// ディレクトリを削除する
/// @param [in] disk     選択したディスク NULLの時、現在のディスク
/// @param [in] side_num AB面ありの時サイド番号 両面なら-1
/// @param [in] dst_item 削除するディレクトリのアイテム
/// @return true:OK false:Error
bool L3DiskFileList::DeleteDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dst_item)
{
	DiskBasic *tmp_basic = basic;
	if (disk) {
		// ディスクを指定した時は、そのディスクのBASICに切り替える
		tmp_basic = disk->GetDiskBasic(side_num);
	}
	if (!tmp_basic) return false;

	return (DeleteDataFile(tmp_basic, dst_item) != 0);
}

/// ファイル属性プロパティダイアログを表示
void L3DiskFileList::ShowFileAttr()
{
	DiskBasicDirItem *item = GetSelectedDirItem();
	if (!item) return;

	ShowFileAttr(item);
}

/// ファイル属性プロパティダイアログを表示
/// @param[in] item ディレクトリアイテム
/// @return true
bool L3DiskFileList::ShowFileAttr(DiskBasicDirItem *item)
{
	// 占有しているグループを再計算
	DiskBasicGroups group_items;
	item->GetAllGroups(group_items);
	item->SetGroups(group_items);

	IntNameBox *dlg = new IntNameBox(frame, this, wxID_ANY, _("File Attribute"), wxT(""), basic, item,
		wxEmptyString, wxEmptyString, 0, NULL,
		INTNAME_SHOW_TEXT | INTNAME_SHOW_ATTR | INTNAME_SHOW_PROPERTY);

	// 占有しているグループを一覧にする
	dlg->SetGroups(group_items);

	// 内部データ表示（機種依存）
	if (gConfig.DoesShowInterDirItem()) {
		KeyValArray datas;
		item->SetCommonDataInAttrDialog(datas);
		item->SetInternalDataInAttrDialog(datas);
		dlg->SetInternalDatas(datas);
	}

	// モードレス
	dlg->Show();

	// FAT使用状況を更新
	wxArrayInt extra_group_nums;
	item->GetExtraGroups(extra_group_nums);
	frame->SetFatAreaGroup(group_items, extra_group_nums);

	return true;
}

/// ファイル属性プロパティダイアログの内容を反映
/// @note ダイアログ内から呼ばれるコールバック
/// @param[in] dlg ダイアログ
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

	wxString newname;
	if (sts) {
		dlg->GetInternalName(newname);
		if (!item->IsUsed()) {
			// 使われてない 削除した？
			err_info->SetError(DiskBasicError::ERRV_ALREADY_DELETED, newname.wc_str());
			sts = false;
		}
	}
	if (sts) {
		if (item->IsFileNameEditable()) {
			// 同じファイル名があるか
			if (dlg_basic->IsFileNameDuplicated(newname, item) != 0) {
				err_info->SetError(DiskBasicError::ERRV_ALREADY_EXISTS, newname.wc_str());
				sts = false;
			}
		}
	}
	if (sts) {
		// ダイアログで指定した値をアイテムに反映
		sts = SetDirItemFromIntNameDialog(item, *dlg, dlg_basic, dlg_basic->CanRenameFile(item, false));
	}
	if (!sts) {
		dlg_basic->ShowErrorMessage();
	}

	if (basic == dlg_basic) {
		// ファイルリストを更新
		RefreshFiles();
		// ディレクトリのときはツリーも更新
		if (item->IsDirectory()) {
			frame->RefreshDiskListDirectoryName(dlg_basic->GetDisk());
		}
	}

	item->ComittedAttrInAttrDialog(dlg, sts);
}

/// プロパティダイアログを閉じる
/// @note ダイアログはモードレスで複数開いていることがある。
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
/// @return true:OKボタン押下
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

/// カラム変更ダイアログ
void L3DiskFileList::ShowListColumnDialog()
{
	if (list->ShowListColumnRearrangeBox()) {
		// リストを更新
		RefreshFiles();
	}
}

/// ディレクトリを作成できるか
/// @return true:できる false:できない
bool L3DiskFileList::CanMakeDirectory() const
{
	return basic ? basic->CanMakeDirectory() : false;
}

/// ディレクトリ作成ダイアログ
void L3DiskFileList::ShowMakeDirectoryDialog()
{
	if (!basic) return;

	DiskD88Disk *disk = basic->GetDisk();
	if (!disk) return;

	// 名前の入力
	DiskBasicDirItem *temp_item = basic->CreateDirItem();
	IntNameBox dlg(frame, this, wxID_ANY, _("New Directory Name"), wxT(""), basic, temp_item, wxEmptyString, wxEmptyString, 0, NULL
		, INTNAME_NEW_FILE | INTNAME_SHOW_TEXT | INTNAME_SPECIFY_FILE_NAME);

	int ans = dlg.ShowModal();

	int sts = 0;
	if (ans == wxID_OK) {
		wxString dirname;
		dlg.GetInternalName(dirname);
		if (temp_item->CanIgnoreDateTime()) {
			gConfig.IgnoreDateTime(dlg.DoesIgnoreDateTime(gConfig.DoesIgnoreDateTime()));
		}
		sts = basic->MakeDirectory(dirname, gConfig.DoesIgnoreDateTime());
		if (sts != 0) {
			basic->ShowErrorMessage();
		} else {
			// リスト更新
			RefreshFiles();
			// 左パネルのツリーを更新
			frame->RefreshAllDirectoryNodesOnDiskList(disk, basic->GetSelectedSide());
		}
	}

	delete temp_item;
}

/// ディレクトリ作成
/// ディレクトリ名が重複する時にダイアログを表示
/// @param[in]  name  ディレクトリ名
/// @param[in]  title ダイアログのタイトル
/// @param[out] nitem 作成したディレクトリアイテム
/// @return true:OK
bool L3DiskFileList::MakeDirectory(const wxString &name, const wxString &title, DiskBasicDirItem **nitem)
{
	int sts = 1;
	wxString dir_name = name;
	{
		// 必要なら名前を変更
		DiskBasicDirItem *pre_item = basic->CreateDirItem();
		if (!pre_item->PreImportDataFile(dir_name)) {
			sts = -1;
		}
		delete pre_item;
	}
	while (sts > 0) {
		sts = basic->MakeDirectory(dir_name, gConfig.DoesIgnoreDateTime(), nitem);
		if (sts == 1) {
			// 同じ名前があるのでダイアログ表示
			basic->ClearErrorMessage();
			wxString msgs = _("The same file name or directory already exists.");
			msgs += wxT("\n");
			msgs += _("Please rename this.");
			DiskBasicDirItem *temp_item = basic->CreateDirItem();
			IntNameBox dlg(frame, this, wxID_ANY, title
				, msgs
				, basic, temp_item, dir_name, dir_name, 0, NULL
				, INTNAME_NEW_FILE | INTNAME_SHOW_TEXT | INTNAME_SPECIFY_FILE_NAME);
			int ans = dlg.ShowModal();
			if (ans != wxID_OK) {
				sts = -1;
			} else {
				dlg.GetInternalName(dir_name);
				if (temp_item->CanIgnoreDateTime()) {
					gConfig.IgnoreDateTime(dlg.DoesIgnoreDateTime(gConfig.DoesIgnoreDateTime()));
				}
			}
			delete temp_item;
		}
	}

	return (sts >= 0);
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
/// BASICディスクを解析したか
bool L3DiskFileList::IsAssignedBasicDisk() const
{
	return basic? basic->IsAssigned() : false;
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
	textAttr->SetFont(font);
	list->SetFont(font);
	Refresh();
}
