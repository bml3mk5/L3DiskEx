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
#include "mymenu.h"
#include "../main.h"
#include "uimainframe.h"
#include "../config.h"
#include "../diskimg/diskimage.h"
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
const struct st_list_columns gUiDiskFileListColumnDefs[] = {
	{ "Name",		wxTRANSLATE("File Name"),		true,	160,	wxALIGN_LEFT,	true },
	{ "Attr",		wxTRANSLATE("Attributes"),		false,	150,	wxALIGN_LEFT,	true },
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
UiDiskFileListStoreModel::UiDiskFileListStoreModel(UiDiskFrame *parentframe, wxWindow *parent)
	: wxDataViewListStore()
{
	frame = parentframe;
	ctrl = (UiDiskFileList *)parent;
}
bool UiDiskFileListStoreModel::SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
{
	// 編集後のファイル名を反映しない
	return false;
}
int UiDiskFileListStoreModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const
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
		cmp = UiDiskFileListCtrl::CompareName(dir_items, i1, i2, dir);
		break;
	case LISTCOL_SIZE:
		cmp = UiDiskFileListCtrl::CompareSize(dir_items, i1, i2, dir);
		break;
	case LISTCOL_GROUPS:
		cmp = UiDiskFileListCtrl::CompareGroups(dir_items, i1, i2, dir);
		break;
	case LISTCOL_START:
		cmp = UiDiskFileListCtrl::CompareStart(dir_items, i1, i2, dir);
		break;
	case LISTCOL_DATE:
		cmp = UiDiskFileListCtrl::CompareDate(dir_items, i1, i2, dir);
		break;
	case LISTCOL_NUM:
		cmp = UiDiskFileListCtrl::CompareNum(dir_items, i1, i2, dir);
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
UiDiskFileListCtrl::UiDiskFileListCtrl(UiDiskFrame *parentframe, wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	: MyCDListCtrl(
		parentframe, parent, id,
		gUiDiskFileListColumnDefs,
		&gConfig,
		wxDV_MULTIPLE,
		new UiDiskFileListStoreModel(parentframe, parent),
		pos, size
	)
#else
	: MyCListCtrl(
		parentframe, parent, id,
		gUiDiskFileListColumnDefs,
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
void UiDiskFileListCtrl::SetListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, MyFileListValue *values)
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
	int		 number =	item->GetNumber();			// 番号

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
//	values[LISTCOL_NUM].Set(row, wxString::Format(wxT("%d"), num));
	values[LISTCOL_NUM].Set(row, wxString::Format(wxT("%d"), number));
}

/// リストにデータを挿入
/// @param [in] basic	DISK BASIC
/// @param [in] item    ディレクトリアイテム
/// @param [in] row     行番号
/// @param [in] num     ディレクトリアイテムの位置
/// @param [in] data    ディレクトリアイテムの位置
void UiDiskFileListCtrl::InsertListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, wxUIntPtr data)
{
	MyFileListValue values[LISTCOL_END];

	SetListData(basic, item, row, num, values);

	InsertListItem(row, values, LISTCOL_END, data);
}

/// リストデータを更新
/// @param [in] basic	DISK BASIC
/// @param [in] item    ディレクトリアイテム
/// @param [in] row     行番号
/// @param [in] num     ディレクトリアイテムの位置
/// @param [in] data    ディレクトリアイテムの位置
void UiDiskFileListCtrl::UpdateListData(DiskBasic *basic, const DiskBasicDirItem *item, long row, int num, wxUIntPtr data)
{
	MyFileListValue values[LISTCOL_END];

	SetListData(basic, item, row, num, values);

	UpdateListItem(row, values, LISTCOL_END, data);
}

/// アイコンを指定
/// @param [in] item    ディレクトリアイテム
int UiDiskFileListCtrl::ChooseIconNumber(const DiskBasicDirItem *item) const
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
void UiDiskFileListCtrl::SetListItems(DiskBasic *basic)
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
void UiDiskFileListCtrl::UpdateListItems(DiskBasic *basic)
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
int wxCALLBACK UiDiskFileListCtrl::Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata)
{
	struct st_file_list_sort_exp *exp = (struct st_file_list_sort_exp *)sortdata;

	int cmp = exp->cmpfunc != NULL ? exp->cmpfunc(exp->items, (int)item1, (int)item2, exp->dir) : 0;
	if (cmp == 0) cmp = ((int)item1 - (int)item2);
	return cmp;
}

/// アイテムをソート
void UiDiskFileListCtrl::SortDataItems(DiskBasic *basic, int col)
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
			case LISTCOL_ATTR:
				exp.cmpfunc = &CompareAttr;
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

int UiDiskFileListCtrl::CompareName(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return items->Item(i1)->GetFileNameStr().CompareTo(items->Item(i2)->GetFileNameStr()) * dir;
}
int UiDiskFileListCtrl::CompareAttr(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	int cmp = (items->Item(i1)->GetFileAttrStr().CompareTo(items->Item(i2)->GetFileAttrStr()) * dir);
	if (cmp == 0) cmp = (items->Item(i1)->GetFileNameStr().CompareTo(items->Item(i2)->GetFileNameStr()) * dir);
	return cmp;
}
int UiDiskFileListCtrl::CompareSize(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return (items->Item(i1)->GetFileSize() - items->Item(i2)->GetFileSize()) * dir;
}
int UiDiskFileListCtrl::CompareGroups(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return (items->Item(i1)->GetGroupSize() - items->Item(i2)->GetGroupSize()) * dir;
}
int UiDiskFileListCtrl::CompareStart(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	return ((int)items->Item(i1)->GetStartGroup(0) - (int)items->Item(i2)->GetStartGroup(0)) * dir;
}
int UiDiskFileListCtrl::CompareDate(DiskBasicDirItems *items, int i1, int i2, int dir)
{
	TM tm1, tm2;
	int cmp;
	items->Item(i1)->GetFileCreateDateTime(tm1);
	items->Item(i2)->GetFileCreateDateTime(tm2);
	cmp = TM::Compare(tm1, tm2) * dir; if (cmp) return cmp;
	items->Item(i1)->GetFileModifyDateTime(tm1);
	items->Item(i2)->GetFileModifyDateTime(tm2);
	cmp = TM::Compare(tm1, tm2) * dir;
	return cmp;
}
int UiDiskFileListCtrl::CompareNum(DiskBasicDirItems *items, int i1, int i2, int dir)
{
//	return (i1 - i2) * dir;
	return (items->Item(i1)->GetNumber() - items->Item(i2)->GetNumber()) * dir;
}

//////////////////////////////////////////////////////////////////////
//
// 右パネルのファイルリスト
//
#define TEXT_ATTR_SIZE 440

// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskFileList, wxPanel)
	EVT_SIZE(UiDiskFileList::OnSize)

#ifndef USE_LIST_CTRL_ON_FILE_LIST
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(IDC_VIEW_LIST, UiDiskFileList::OnListContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(IDC_VIEW_LIST, UiDiskFileList::OnListActivated)

	EVT_DATAVIEW_SELECTION_CHANGED(IDC_VIEW_LIST, UiDiskFileList::OnSelectionChanged)

	EVT_DATAVIEW_ITEM_BEGIN_DRAG(IDC_VIEW_LIST, UiDiskFileList::OnBeginDrag)

	EVT_DATAVIEW_ITEM_START_EDITING(IDC_VIEW_LIST, UiDiskFileList::OnFileNameStartEditing)
	EVT_DATAVIEW_ITEM_EDITING_STARTED(IDC_VIEW_LIST, UiDiskFileList::OnFileNameEditingStarted)
	EVT_DATAVIEW_ITEM_EDITING_DONE(IDC_VIEW_LIST, UiDiskFileList::OnFileNameEditedDone)

//	EVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK(IDC_VIEW_LIST, UiDiskFileList::OnListColumnContextMenu)
//	EVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK(IDC_VIEW_LIST, UiDiskFileList::OnListColumnDetail)
#else
	EVT_LIST_ITEM_ACTIVATED(IDC_VIEW_LIST, UiDiskFileList::OnListActivated)

	EVT_LIST_ITEM_SELECTED(IDC_VIEW_LIST, UiDiskFileList::OnSelect)
	EVT_LIST_ITEM_DESELECTED(IDC_VIEW_LIST, UiDiskFileList::OnDeselect)

	EVT_LIST_BEGIN_DRAG(IDC_VIEW_LIST, UiDiskFileList::OnBeginDrag)

	EVT_LIST_BEGIN_LABEL_EDIT(IDC_VIEW_LIST, UiDiskFileList::OnFileNameStartEditing)
	EVT_LIST_END_LABEL_EDIT(IDC_VIEW_LIST, UiDiskFileList::OnFileNameEditedDone)

	EVT_LIST_COL_CLICK(IDC_VIEW_LIST, UiDiskFileList::OnColumnClick)
	EVT_LIST_COL_RIGHT_CLICK(IDC_VIEW_LIST, UiDiskFileList::OnListColumnContextMenu)

	EVT_CONTEXT_MENU(UiDiskFileList::OnContextMenu)

	EVT_MENU(IDM_COLUMN_DETAIL, UiDiskFileList::OnListColumnDetail)
	EVT_MENU(IDM_COLUMN_RESET, UiDiskFileList::OnListColumnReset)
#endif

	EVT_BUTTON(IDC_BTN_CHANGE, UiDiskFileList::OnButtonChange)
	EVT_CHOICE(IDC_COMBO_CHAR_CODE, UiDiskFileList::OnChangeCharCode)

	EVT_MENU(IDM_EXPORT_FILE, UiDiskFileList::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, UiDiskFileList::OnImportFile)

	EVT_MENU(IDM_DELETE_FILE, UiDiskFileList::OnDeleteFile)
	EVT_MENU(IDM_RENAME_FILE, UiDiskFileList::OnRenameFile)

	EVT_MENU(IDM_COPY_FILE, UiDiskFileList::OnCopyFile)
	EVT_MENU(IDM_PASTE_FILE, UiDiskFileList::OnPasteFile)

	EVT_MENU(IDM_EDIT_FILE_BINARY, UiDiskFileList::OnEditFile)
	EVT_MENU(IDM_EDIT_FILE_TEXT, UiDiskFileList::OnEditFile)

	EVT_MENU(IDM_MAKE_DIRECTORY, UiDiskFileList::OnMakeDirectory)

	EVT_MENU(IDM_PROPERTY, UiDiskFileList::OnProperty)

wxEND_EVENT_TABLE()

UiDiskFileList::UiDiskFileList(UiDiskFrame *parentframe, wxWindow *parentwindow)
       : wxPanel(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	m_initialized	 = false;
	parent			 = parentwindow;
	frame			 = parentframe;
	m_current_basic	 = NULL;
	listCtrl		 = NULL;
	m_disk_selecting = false;

	MyFileListItem_Unset(m_dragging_item);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrHed = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *szrBtn = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxSize size(TEXT_ATTR_SIZE, -1);
	textAttr = new wxTextCtrl(this, IDC_TEXT_ATTR, wxT(""), wxDefaultPosition, size, wxTE_READONLY | wxTE_LEFT);
	szriTxt = szrHed->Add(textAttr, wxSizerFlags().Expand().Border(wxBOTTOM | wxTOP, 2));

	size.x = 60;
	btnChange = new wxButton(this, IDC_BTN_CHANGE, _("Change"), wxDefaultPosition, size);
	btnChange->Enable(false);
	szrBtn->Add(btnChange, flags);

	lblCharCode = new wxStaticText(this, wxID_ANY, _("Charactor Code"));
	szrBtn->Add(lblCharCode,  wxSizerFlags().Center().Border(wxBOTTOM | wxTOP, 2).Border(wxLEFT | wxRIGHT, 8));
	comCharCode = new wxChoice(this, IDC_COMBO_CHAR_CODE, wxDefaultPosition, wxDefaultSize);
	const CharCodeChoice *choice = gCharCodeChoices.Find(wxT("main"));
	if (choice) {
		for(size_t i=0; i<choice->Count(); i++) {
			const CharCodeMap *map = choice->Item(i);
			comCharCode->Append( map->GetDescription() );
		}
	}
	szrBtn->Add(comCharCode, flags);

	szriBtn = szrHed->Add(szrBtn);
	szriHed = vbox->Add(szrHed);

	wxFont font;
	frame->GetDefaultListFont(font);

	listCtrl = new UiDiskFileListCtrl(parentframe, this, IDC_VIEW_LIST);
	textAttr->SetFont(font);
	listCtrl->SetFont(font);
	szriLst = vbox->Add(listCtrl, wxSizerFlags().Expand().Border(wxALL, 1));

	SetSizerAndFit(vbox);
	Layout();

	// popup menu
	MakePopupMenu();

	// popup on list column header
	MakeColumnPopupMenu();

	// key
	listCtrl->Bind(wxEVT_CHAR, &UiDiskFileList::OnChar, this);

	m_initialized = true;
}

UiDiskFileList::~UiDiskFileList()
{
	delete menuPopup;
	delete menuColumnPopup;
}

/// アイテムを得る
/// @param[in] event リストイベント
/// @return リストアイテム
MyFileListItem UiDiskFileList::GetEventItem(const MyFileListEvent& event) const
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	return event.GetItem();
#else
	return event.GetIndex();
#endif
}

/// リサイズ
/// @param[in] event サイズイベント
void UiDiskFileList::OnSize(wxSizeEvent& event)
{
	if (!m_initialized) {
		event.Skip();
		return;
	}
	if (event.GetEventObject() != this) {
		event.Skip();
		return;
	}
	if (!GetSizer()) {
		event.Skip();
		return;
	}

	wxSize szCli = GetClientSize();
	if (szCli.x < 32) return;

	// コントロールのサイズを計算
	wxSize szTxt = szriTxt->CalcMin();
	wxSize szBtn = szriBtn->CalcMin();
	wxSize szHed = szriHed->CalcMin();
	wxSize szLst = szriLst->CalcMin();

	// ファイルリストのサイズを変更
	wxPoint ptLst(0, szHed.GetHeight());
	szLst.SetWidth(szCli.GetWidth());
	szLst.SetHeight(szCli.GetHeight() - szHed.GetHeight());

	// テキストエリアのサイズを変更
	szTxt.SetWidth(szCli.GetWidth() - szBtn.GetWidth());
	int text_attr_size = FromDIP(TEXT_ATTR_SIZE);
	if (szTxt.GetWidth() < text_attr_size) {
		// 最小サイズ
		szTxt.SetWidth(text_attr_size);
	}

	// コントロールの再配置
	wxPoint pt;
	szriTxt->SetDimension(pt, szTxt);
	pt.x += szTxt.GetWidth();
	szriBtn->SetDimension(pt, szBtn);
	szriLst->SetDimension(ptLst, szLst);
}

#ifdef USE_LIST_CTRL_ON_FILE_LIST
/// リストを選択
/// @param[in] event リストイベント
void UiDiskFileList::OnSelect(MyFileListEvent& event)
{
	if (!m_initialized || !m_current_basic) return;

	MyFileListItem selected_item = event.GetIndex();
	if (selected_item == wxNOT_FOUND) return;

	/// SelectItem()を呼ぶ
	SelectItem(selected_item, listCtrl->GetListSelectedItemCount());
}

/// リストを非選択にした
/// @param[in] event リストイベント
void UiDiskFileList::OnDeselect(MyFileListEvent& event)
{
	if (!m_initialized || !m_current_basic) return;

	MyFileListItem deselected_item = event.GetIndex();
	if (deselected_item == wxNOT_FOUND) return;

	/// UnselectItem()を呼ぶ
	UnselectItem(deselected_item, listCtrl->GetListSelectedItemCount());
}
#else
/// リスト選択行を変更
/// @param[in] event リストイベント
void UiDiskFileList::OnSelectionChanged(MyFileListEvent& event)
{
	if (!m_initialized || !m_current_basic) return;

//	if (list->GetListSelectedRow() == wxNOT_FOUND) {
//		return;
//	}

	int count = listCtrl->GetListSelectedItemCount();

	for(int row=0; row<listCtrl->GetItemCount(); row++) {
		bool sel = listCtrl->IsRowSelected(row);
		int tog = listCtrl->GetListSelected(row);
		MyFileListItem item = listCtrl->RowToItem(row);
		if (sel && (tog == 0)) {
			SelectItem(item, count);
		} else if (!sel && (tog != 0)) {
			UnselectItem(item, count);
		}
		listCtrl->SetListSelected(row, sel ? 1 : 0);
	}
}
#endif

/// リストの編集開始
/// @param[in] event リストイベント
void UiDiskFileList::OnFileNameStartEditing(MyFileListEvent& event)
{
	if (!m_current_basic) return;

	MyFileListItem listitem = GetEventItem(event);
	int pos = (int)listCtrl->GetListItemData(listitem);
	DiskBasicDirItem *ditem = m_current_basic->GetDirItem(pos);
	if (!ditem || !ditem->IsFileNameEditable()) {
		// 編集不可
		event.Veto();
	}
}

/// リストの編集開始した
/// @param[in] event リストイベント
void UiDiskFileList::OnFileNameEditingStarted(MyFileListEvent& event)
{
#ifndef USE_LIST_CTRL_ON_FILE_LIST
	if (!m_current_basic) return;

	wxDataViewColumn *column = event.GetDataViewColumn();
	wxDataViewRenderer *renderer = column->GetRenderer();
	if (!renderer) return;
	wxTextCtrl *text = (wxTextCtrl *)renderer->GetEditorCtrl();
	if (!text) return;

	wxDataViewItem listitem = event.GetItem();
	int pos = (int)listCtrl->GetListItemData(listitem);
	DiskBasicDirItem *ditem = m_current_basic->GetDirItem(pos);
	if (ditem && ditem->IsFileNameEditable()) {
		int max_len = ditem->GetFileNameStrSize();
		IntNameValidator validate(ditem, _("file name"), m_current_basic->GetValidFileName());
		text->SetMaxLength(max_len);
		text->SetValidator(validate);
	}
#endif
}

/// リストの編集終了
/// @param[in] event リストイベント
void UiDiskFileList::OnFileNameEditedDone(MyFileListEvent& event)
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
void UiDiskFileList::OnListContextMenu(MyFileListEvent& event)
{
	ShowPopupMenu();
}

/// リストのカラム上で右クリック
/// @param[in] event リストイベント
void UiDiskFileList::OnListColumnContextMenu(MyFileListEvent& event)
{
	ShowColumnPopupMenu();
}

/// セクタリスト カラムをクリック
/// @param[in] event リストイベント
void UiDiskFileList::OnColumnClick(MyFileListEvent& event)
{
	int col = event.GetColumn();
#ifdef USE_LIST_CTRL_ON_FILE_LIST
	listCtrl->SortDataItems(m_current_basic, col);
#endif
}

/// 右クリック
void UiDiskFileList::OnContextMenu(wxContextMenuEvent& WXUNUSED(event))
{
	ShowPopupMenu();
}

/// リスト上でダブルクリック
/// @param[in] event リストイベント
void UiDiskFileList::OnListActivated(MyFileListEvent& WXUNUSED(event))
{
	DoubleClicked();
}

/// リスト上でドラッグ開始
/// @param[in] event リストイベント
void UiDiskFileList::OnBeginDrag(MyFileListEvent& WXUNUSED(event))
{
	// ドラッグ
	DragDataSource();
}

/// エクスポート選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnExportFile(wxCommandEvent& WXUNUSED(event))
{
	ShowExportDataFileDialog();
}

/// インポート選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnImportFile(wxCommandEvent& WXUNUSED(event))
{
	ShowImportDataFileDialog();
}

/// 削除選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnDeleteFile(wxCommandEvent& WXUNUSED(event))
{
	DeleteDataFile();
}

/// リネーム選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnRenameFile(wxCommandEvent& WXUNUSED(event))
{
	StartEditingFileName();
}

/// コピー選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnCopyFile(wxCommandEvent& WXUNUSED(event))
{
	CopyToClipboard();
}

/// ペースト選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnPasteFile(wxCommandEvent& WXUNUSED(event))
{
	PasteFromClipboard();
}

/// ディレクトリ作成選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnMakeDirectory(wxCommandEvent& WXUNUSED(event))
{
	ShowMakeDirectoryDialog();
}

/// ファイル編集選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnEditFile(wxCommandEvent& event)
{
	EditDataFile(event.GetId() == IDM_EDIT_FILE_BINARY ? EDITOR_TYPE_BINARY : EDITOR_TYPE_TEXT);
}

/// プロパティ選択
/// @param[in] event コマンドイベント
void UiDiskFileList::OnProperty(wxCommandEvent& WXUNUSED(event))
{
	ShowFileAttr();
}

/// リスト上でキー押下
/// @param[in] event キーイベント
void UiDiskFileList::OnChar(wxKeyEvent& event)
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
void UiDiskFileList::OnButtonChange(wxCommandEvent& WXUNUSED(event))
{
	ChangeBasicType();
}

/// キャラクターコード変更ボタン押下
/// @param[in] event コマンドイベント
void UiDiskFileList::OnChangeCharCode(wxCommandEvent& event)
{
	int sel = event.GetSelection();
	wxString name = gCharCodeChoices.GetItemName(wxT("main"), (size_t)sel);
	frame->ChangeCharCode(name);
}

#if 0
/// リストのカラムを変更
/// @param[in] event コマンドイベント
void UiDiskFileList::OnListColumnChange(wxCommandEvent& event)
{
	int id = event.GetId() - IDM_COLUMN_0;

	if (listCtrl->ShowColumn(id, event.IsChecked())) {
		// リストを更新
		RefreshFiles();
	}
}
#endif

/// リストのカラム詳細設定
/// @param[in] event コマンドイベント
void UiDiskFileList::OnListColumnDetail(wxCommandEvent& WXUNUSED(event))
{
	ShowListColumnDialog();
}

/// リストのカラム幅リセット
/// @param[in] event コマンドイベント
void UiDiskFileList::OnListColumnReset(wxCommandEvent& WXUNUSED(event))
{
	ResetAllListColumnWidth();
}

////////////////////////////////////////

/// ポップアップメニュー作成
void UiDiskFileList::MakePopupMenu()
{
	menuPopup = new MyMenu;
	menuPopup->Append(IDM_EXPORT_FILE, _("&Export..."));
	menuPopup->Append(IDM_IMPORT_FILE, _("&Import..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_DELETE_FILE, _("&Delete..."));
	menuPopup->Append(IDM_RENAME_FILE, _("Rena&me"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_COPY_FILE, _("&Copy"));
	menuPopup->Append(IDM_PASTE_FILE, _("&Paste..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_EDIT_FILE_BINARY, _("Edit using binary editor..."));
	menuPopup->Append(IDM_EDIT_FILE_TEXT, _("Edit using text editor..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MAKE_DIRECTORY, _("Make Directory(&F)..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY, _("P&roperty"));
}

/// ポップアップメニュー表示
void UiDiskFileList::ShowPopupMenu()
{
	if (!menuPopup) return;

	bool opened = IsAssignedBasicDisk();
	menuPopup->Enable(IDM_IMPORT_FILE, opened);
	menuPopup->Enable(IDM_PASTE_FILE, opened);

	menuPopup->Enable(IDM_MAKE_DIRECTORY, opened && frame->CanMakeDirectory(m_current_basic));

	int cnt = listCtrl->GetListSelectedItemCount();
	opened = (opened && cnt > 0);
	menuPopup->Enable(IDM_EXPORT_FILE, opened);
	menuPopup->Enable(IDM_DELETE_FILE, opened);
	menuPopup->Enable(IDM_COPY_FILE, opened);

	opened = (opened && cnt == 1 && (listCtrl->GetListSelectedRow() != wxNOT_FOUND));
	menuPopup->Enable(IDM_RENAME_FILE, opened);
	menuPopup->Enable(IDM_EDIT_FILE_BINARY, opened);
	menuPopup->Enable(IDM_EDIT_FILE_TEXT, opened);
	menuPopup->Enable(IDM_PROPERTY, opened);

	PopupMenu(menuPopup);
}

/// リストカラムのポップアップメニュー作成
void UiDiskFileList::MakeColumnPopupMenu()
{
	menuColumnPopup = new MyMenu;
	menuColumnPopup->Append(IDM_COLUMN_DETAIL, _("Columns of File &List..."));
	menuColumnPopup->Append(IDM_COLUMN_RESET, _("&Reset Width of File List"));
}

/// リストカラムのポップアップメニュー表示
void UiDiskFileList::ShowColumnPopupMenu()
{
	if (!menuColumnPopup) return;

	PopupMenu(menuColumnPopup);
}

/// BASIC種類テキストボックスに設定
/// @param[in] val 文字列
void UiDiskFileList::SetAttr(const wxString &val)
{
	textAttr->SetValue(val);
}

/// BASIC種類テキストボックスをクリア
void UiDiskFileList::ClearAttr()
{
	textAttr->Clear();
}

/// キャラクターコード変更
/// @param[in] name コード名
void UiDiskFileList::ChangeCharCode(const wxString &name)
{
	if (m_current_basic) {
		m_current_basic->SetCharCode(name);
		RefreshFiles();
	}
	PushCharCode(name);
}

/// キャラクターコードの選択位置を変える
/// @param[in] name コード名
void UiDiskFileList::PushCharCode(const wxString &name)
{
	int sel = gCharCodeChoices.IndexOf(wxT("main"), name);
	comCharCode->SetSelection(sel);
}

/// ファイル名をリストにセット
/// @param [in] newdisk    ディスク
/// @param [in] newsidenum サイド番号 -1:両面 >0:裏表あり
void UiDiskFileList::AttachDiskBasic(DiskImageDisk *newdisk, int newsidenum)
{
	m_current_basic = newdisk->GetDiskBasic(newsidenum);

	m_disk_selecting = true;
}

/// DISK BASICをデタッチ
void UiDiskFileList::DetachDiskBasic()
{
	m_disk_selecting = false;

	m_current_basic = NULL;
}

/// ファイル名をリストに設定
///
/// リストアイテムを非選択にする。
/// メニューを更新する。
void UiDiskFileList::SetFiles()
{
	// ファイル名一覧を設定
	RefreshFiles();
	// リストを非選択
	listCtrl->UnselectAllListItem();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// ファイル名をリストに再設定
void UiDiskFileList::RefreshFiles()
{
	if (!m_disk_selecting
	 || !m_current_basic
	 || m_current_basic->GetBasicTypeName().IsEmpty()) {
		listCtrl->DeleteAllListItems();
		btnChange->Enable(false);
		return;
	}

	// 属性をセット
	SetAttr(m_current_basic->GetDescriptionDetails());
	btnChange->Enable(true);

	// キャラクターコードをセット
	PushCharCode(m_current_basic->GetCharCode());

	// ファイル名一覧を設定
	listCtrl->Freeze();
	listCtrl->SetListItems(m_current_basic);
	listCtrl->Thaw();

	// FAT空き状況を確認
	if (frame->GetFatAreaFrame()) {
		frame->SetFatAreaData();
	}
}

#if 0
/// リスト内容を更新
void UiDiskFileList::UpdateFiles()
{
	// ファイル名一覧を更新
	list->Freeze();
	list->UpdateListItems(basic);
	list->Thaw();
}
#endif

/// リストをクリア
void UiDiskFileList::ClearFiles()
{
	btnChange->Enable(false);

	listCtrl->DeleteAllListItems();
	ClearAttr();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// 全行を選択
void UiDiskFileList::SelectAll()
{
	listCtrl->SelectAllListItem();
}

/// 行選択
/// @param[in] selected_item 選択した行の情報
/// @param[in] count         選択した行数
bool UiDiskFileList::SelectItem(const MyFileListItem &selected_item, int count)
{
	DiskBasicDirItem *ditem = m_current_basic->GetDirItem(listCtrl->GetListItemData(selected_item));
	if (!ditem) {
		return false;
	}

	DiskImageSector *sector = m_current_basic->GetSectorFromGroup(ditem->GetStartGroup(0));
	if (!sector) {
		return false;
	}

	// FAT使用状況を更新
	wxArrayInt extra_group_nums;
	ditem->GetExtraGroups(extra_group_nums);
	frame->SetFatAreaGroup(ditem->GetGroups(), extra_group_nums);

	if (count == 1) {
		// ダンプリストをセット
		frame->SetBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize(), m_current_basic->GetCharCode(), m_current_basic->IsDataInverted());
	}

	if (count <= 2) {
		// メニューを更新
		frame->UpdateMenuAndToolBarFileList(this);
	}

	return true;
}

// 行非選択
/// @param[in] deselected_item 選択していた行の情報
/// @param[in] count           選択していた行数
void UiDiskFileList::UnselectItem(const MyFileListItem &deselected_item, int count)
{
	DiskBasicDirItem *ditem = m_current_basic->GetDirItem(listCtrl->GetListItemData(deselected_item));
	if (!ditem) {
		return;
	}

	// FAT使用状況を更新
	wxArrayInt extra_group_nums;
	ditem->GetExtraGroups(extra_group_nums);
	frame->UnsetFatAreaGroup(ditem->GetGroups(), extra_group_nums);

	// メニューを更新
	if (count <= 1) {
		frame->UpdateMenuAndToolBarFileList(this);
	}
}

/// データをダンプウィンドウに設定
/// @param[in] group_item グループ番号
/// @return true:正常 false:トラックなし 
bool UiDiskFileList::SetDumpData(DiskBasicGroupItem &group_item)
{
	if (!m_current_basic) return false;

	DiskImageDisk *disk = m_current_basic->GetDisk();
	DiskImageTrack *track = disk->GetTrack(group_item.track, group_item.side);
	if (!track) {
		return false;
	}
	for(int s=group_item.sector_start; s<=group_item.sector_end; s++) {
		DiskImageSector *sector;
		sector = track->GetSector(s);
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
/// @param[in] track_num    トラック番号
/// @param[in] side_num     サイド番号
/// @param[in] sector_start 開始セクタ番号
/// @param[in] sector_end   終了セクタ番号
/// @return true:正常 false:トラックなし 
bool UiDiskFileList::SetDumpData(int track_num, int side_num, int sector_start, int sector_end)
{
	if (!m_current_basic) return false;

	DiskImageDisk *disk = m_current_basic->GetDisk();
	DiskImageTrack *track = disk->GetTrack(track_num, side_num);
	if (!track) {
		return false;
	}
	for(int s=sector_start; s<=sector_end; s++) {
		DiskImageSector *sector = track->GetSector(s);
		if (!sector) {
			break;
		}
		if (s == sector_start) {
			frame->SetBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize());
		} else {
			frame->AppendBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize());
		}
	}
	return true;
}

/// 現在選択している行のディレクトリアイテムを得る
/// @return ディレクトリアイテム or NULL
DiskBasicDirItem *UiDiskFileList::GetSelectedDirItem()
{
	if (!m_current_basic) return NULL;
	if (!listCtrl) return NULL;
	int row = listCtrl->GetListSelectedRow();
	if (row == wxNOT_FOUND) return NULL;
	MyFileListItem view_item = listCtrl->GetListSelection();
	return GetDirItem(view_item);
}

/// リストの指定行のディレクトリアイテムを得る
/// @param[in]  view_item 指定行のリストアイテム
/// @param[out] item_pos  ディレクトリアイテムの位置
/// @return ディレクトリアイテム or NULL
DiskBasicDirItem *UiDiskFileList::GetDirItem(const MyFileListItem &view_item, int *item_pos)
{
	if (!m_current_basic) return NULL;
	if (!listCtrl) return NULL;
	if (listCtrl->GetItemCount() <= 0) return NULL;
	size_t dir_pos = listCtrl->GetListItemData(view_item);
	if (item_pos) *item_pos = (int)dir_pos;
	DiskBasicDirItem *dir_item = m_current_basic->GetDirItem(dir_pos);
	return dir_item;
}

/// リストの指定行のディレクトリアイテムとそのファイル名を得る
/// @param[in]  view_item 指定行のリストアイテム
/// @param[out] name      ディレクトリアイテムのファイル名
/// @param[out] item_pos  ディレクトリアイテムの位置
/// @return ディレクトリアイテム or NULL
DiskBasicDirItem *UiDiskFileList::GetFileName(const MyFileListItem &view_item, wxString &name, int *item_pos)
{
	DiskBasicDirItem *dir_item = GetDirItem(view_item, item_pos);
	if (!dir_item) return NULL;

	name = dir_item->GetFileNameStr();

	return dir_item;
}

/// エクスポートダイアログ
void UiDiskFileList::ShowExportDataFileDialog()
{
	if (!m_current_basic) return;

	m_current_basic->ClearErrorMessage();

	MyFileListItems selected_items;
	int selcount = listCtrl->GetListSelections(selected_items);
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
		if (!m_current_basic->IsLoadableFile(item)) {
			m_current_basic->ShowErrorMessage();
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

		UiDiskSaveFileDialog dlg(
			_("Export a file"),
			frame->GetIniExportFilePath(),
			filename,
			_("All files (*.*)|*.*"));

		int dlgsts = dlg.ShowModal();
		if (dlgsts != wxID_OK) {
			return;
		}

		wxString path = dlg.GetPath();

		// エクスポート元パスを覚えておく
		frame->SetIniExportFilePath(path);

		frame->ExportDataFile(m_current_basic, item, path, _("exporting..."), _("exported."));

	} else {
		// 複数選択時やディレクトリ選択時

		// 出力先フォルダを選択
		UiDiskDirDialog dlg(
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

/// 指定したフォルダにエクスポート
/// @param [in]     selected_items 選択したリスト
/// @param [in]     data_dir       データファイル出力先フォルダ
/// @param [in]     attr_dir       属性ファイル出力先フォルダ
/// @param [in]     start_msg      開始メッセージ
/// @param [in]     end_msg        終了メッセージ
/// @param [in,out] file_object    ファイルオブジェクト
/// @return 0:OK >0:Warning <0:Error
int UiDiskFileList::ExportDataFiles(const MyFileListItems &selected_items, const wxString &data_dir, const wxString &attr_dir, const wxString &start_msg, const wxString &end_msg, wxFileDataObject *file_object)
{
	frame->StartExportCounter(0, start_msg);

	// 選択したファイルをリストにする。
	int selcount = (int)selected_items.Count();
	DiskBasicDirItems dir_items;
	for(int i=0; i<selcount; i++) {
		DiskBasicDirItem *item = GetDirItem(selected_items.Item(i));
		if (!item) {
			continue;
		}
		dir_items.Add(item);
	}
	int sts = frame->ExportDataFiles(m_current_basic, &dir_items, data_dir, attr_dir, file_object, 0);
	if (sts != 0) {
		m_current_basic->ShowErrorMessage();
	}

	frame->FinishExportCounter(end_msg);

	return sts;
}

/// ドラッグする
bool UiDiskFileList::DragDataSource()
{
	wxString tmp_dir_name;

#ifdef USE_DATA_OBJECT_COMPOSITE
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
	// １つだけ選択時ドラッグしたアイテムを覚えておく
	if (listCtrl->GetListSelectedItemCount() == 1) {
		m_dragging_item = listCtrl->GetListSelection();
	} else {
		MyFileListItem_Unset(m_dragging_item);
	}

	// ファイルデータは外部用
	if (file_object) compo.Add(file_object);

#ifdef __WXMSW__
	wxDropSource dragSource(compo);
#else
	wxDropSource dragSource(compo, frame);
#endif
#else

	wxFileDataObject file_object;

	bool sts = true;
	if (sts) {
		sts = CreateFileObject(tmp_dir_name, _("dragging..."), _("dragged."), file_object);
	}
	if (!sts) {
		return false;
	}
	// １つだけ選択時ドラッグしたアイテムを覚えておく
	if (listCtrl->GetListSelectedItemCount() == 1) {
		m_dragging_item = listCtrl->GetListSelection();
	} else {
		MyFileListItem_Unset(m_dragging_item);
	}

#ifdef __WXMSW__
	wxDropSource dragSource(file_object);
#else
	wxDropSource dragSource(file_object, frame);
#endif
#endif

	dragSource.DoDragDrop();

	return true;
}

// クリップボードへコピー
bool UiDiskFileList::CopyToClipboard()
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
bool UiDiskFileList::CreateFileObject(wxString &tmp_dir_name, const wxString &start_msg, const wxString &end_msg, wxFileDataObject &file_object)
{
	if (!m_current_basic) return false;

	if (!listCtrl) return false;

	MyFileListItems selected_items;
	int selcount = listCtrl->GetListSelections(selected_items);
	if (selcount <= 0) return false;

	wxString tmp_data_path, tmp_attr_path;
	if (!frame->CreateTemporaryFolder(tmp_dir_name, tmp_data_path, tmp_attr_path)) {
		return false;
	}

	ExportDataFiles(selected_items, tmp_data_path, tmp_attr_path, start_msg, end_msg, &file_object);

	return true;
}

/// ファイルリストを解放（DnD, クリップボード用）
/// @param [in]     tmp_dir_name テンポラリフォルダ
void UiDiskFileList::ReleaseFileObject(const wxString &tmp_dir_name)
{
	UiDiskApp *app = &wxGetApp();

	// テンポラリディレクトリを削除
	app->RemoveTempDir(tmp_dir_name);
}

/// クリップボードからペースト
bool UiDiskFileList::PasteFromClipboard()
{
	if (!m_current_basic) return false;

	if (!m_current_basic->IsFormatted()) {
		return false;
	}
	if (!m_current_basic->IsWritableIntoDisk()) {
		m_current_basic->ShowErrorMessage();
		return false;
	}

	// ディレクトリを選択しているときはそこにペースト
	DiskBasicDirItem *dir_item = m_current_basic->GetCurrentDirectory();
	MyFileListItem item = listCtrl->GetListSelection();
	if (MyFileListItem_IsOk(item)) {
		DiskBasicDirItem *tmp_dir_item = GetDirItem(item);
		if (tmp_dir_item && tmp_dir_item->IsDirectory()) {
			dir_item = tmp_dir_item;
		}
	}

	// Read some text
	wxFileDataObject file_object;

	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported( wxDF_FILENAME )) {
			wxTheClipboard->GetData( file_object );
		}
		wxTheClipboard->Close();
	}

	// インポート
	return frame->ImportDataFiles(file_object.GetFilenames(), m_current_basic, dir_item, false, _("pasting..."), _("pasted."));
}

/// インポートダイアログ
void UiDiskFileList::ShowImportDataFileDialog()
{
	if (!m_current_basic) return;

	if (!m_current_basic->IsFormatted()) {
		return;
	}
	if (!m_current_basic->IsWritableIntoDisk()) {
		m_current_basic->ShowErrorMessage();
		return;
	}

	UiDiskOpenFileDialog dlg(
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
	frame->ImportDataFiles(paths, m_current_basic, m_current_basic->GetCurrentDirectory(), false, _("importing..."), _("imported."));
}

/// ファイルをドロップ
/// @param[in] base         座標の基準となるウィンドウ
/// @param[in] x            ドロップした位置X
/// @param[in] y            ドロップした位置Y
/// @param[in] paths        ファイルパスのリスト
/// @param[in] dir_included ディレクトリを含むか
/// @return true:OK false:Error
bool UiDiskFileList::DropDataFiles(wxWindow *base, int x, int y, const wxArrayString &paths, bool dir_included)
{
	if (paths.Count() == 0) {
		return false;
	}

	if (!m_current_basic->IsFormatted()) {
		return false;
	}
	if (!m_current_basic->IsWritableIntoDisk()) {
		m_current_basic->ShowErrorMessage();
		return false;
	}

	int bx = 0;
	int by = 0;
	// リストコントロールの親との相対位置
	UiDiskFrame::GetPositionFromBaseWindow(base, listCtrl, bx, by);
	MyFileListItem item = listCtrl->GetItemAtPoint(x - bx, y - by);
	// ドラッグしたアイテムと同じならドロップしない
	bool sts = (MyFileListItem_IsOk(item) && item == m_dragging_item);
	MyFileListItem_Unset(m_dragging_item);
	if (sts) {
		return true;
	}

	// 指定先がディレクトリならそこにドロップ
	// そうでないなら現在のディレクトリにドロップ
	DiskBasicDirItem *dir_item = m_current_basic->GetCurrentDirectory();
	if (MyFileListItem_IsOk(item)) {
		DiskBasicDirItem *tmp_dir_item = GetDirItem(item);
		if (tmp_dir_item && tmp_dir_item->IsDirectory()) {
			dir_item = tmp_dir_item;
			dir_included = true;	// 確認ダイアログ表示を指示
		}
	}

	return frame->ImportDataFiles(paths, m_current_basic, dir_item, dir_included, _("dropping..."), _("dropped."));
}

/// 指定したファイルを削除
bool UiDiskFileList::DeleteDataFile()
{
	if (!m_current_basic) return false;

	bool sts = true;
	MyFileListItems selected_items;
	int selcount = listCtrl->GetListSelections(selected_items);
	if (selcount < 0) return false;

	if (!m_current_basic->IsDeletableFiles()) {
		m_current_basic->ShowErrorMessage();
		return false;
	}

	if (selcount == 1) {
		// 1ファイル選択時
		DiskBasicDirItem *item = GetDirItem(selected_items.Item(0));
		sts = (frame->DeleteDataFile(m_current_basic, item) != 0);
	} else {
		// 複数ファイル選択時
		sts = (DeleteDataFiles(m_current_basic, selected_items) != 0);
	}
	return sts;
}

/// 指定したファイルを一括削除
/// @param[in]     tmp_basic       BASIC
/// @param[in,out] selected_items  削除対象アイテムリスト
/// @return 0:OK >0:Warning <0:Error
int UiDiskFileList::DeleteDataFiles(DiskBasic *tmp_basic, MyFileListItems &selected_items)
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
	sts = frame->DeleteDataFiles(tmp_basic, items, 0, &dir_items);

	// リスト更新
	frame->DeleteDirectoryNodesOnDiskList(m_current_basic->GetDisk(), dir_items);
	RefreshFiles();

	if (sts != 0) {
		tmp_basic->ShowErrorMessage();
	}

	return sts;
}

/// ファイル名の編集開始
void UiDiskFileList::StartEditingFileName()
{
	if (!m_current_basic) return;

	if (listCtrl->GetListSelectedRow() == wxNOT_FOUND) return;

	MyFileListItem selected_item = listCtrl->GetListSelection();

	int pos = (int)listCtrl->GetListItemData(selected_item);
	DiskBasicDirItem *ditem = m_current_basic->GetDirItem(pos);

	if (!m_current_basic->CanRenameFile(ditem)) {
		m_current_basic->ShowErrorMessage();
		return;
	}

	StartEditingFileName(selected_item);
}

/// ファイル名の編集開始
/// @param[in] selected_item 選択行のアイテム
void UiDiskFileList::StartEditingFileName(const MyFileListItem &selected_item)
{
	// 編集可能にする
	listCtrl->EditListItem(selected_item);
}

/// 指定したファイル名を変更
/// @param[in] view_item 選択行のアイテム
/// @param[in] newname   変更するファイル名
/// @return true:OK false:Error
bool UiDiskFileList::RenameDataFile(const MyFileListItem &view_item, const wxString &newname)
{
	if (!m_current_basic) return false;

	DiskBasicDirItem *item = GetDirItem(view_item);
	if (!item) return false;

	if (!m_current_basic->CanRenameFile(item)) {
		m_current_basic->ShowErrorMessage();
		return false;
	}

	DiskBasicFileName filename;
	filename.SetName(newname);

	// ダイアログ入力後のファイル名文字列を大文字に変換
	if (m_current_basic->ToUpperAfterRenamed()) {
		filename.GetName().MakeUpper();
	}
	// 拡張属性を得る
	filename.SetOptional(item->GetOptionalName());

	bool sts = true;
	wxString errmsg;
	IntNameValidator validator(item, _("file name"), m_current_basic->GetValidFileName());
	if (sts && !validator.Validate(frame, filename.GetName())) {
		sts = false;
	}
	if (sts && m_current_basic->IsFileNameDuplicated(m_current_basic->GetCurrentDirectory(), filename, item) != 0) {
		errmsg = wxString::Format(_("File '%s' already exists."), filename.GetName());
		wxMessageBox(errmsg, _("File exists"), wxOK | wxICON_EXCLAMATION, parent);
		sts = false;
	}
	if (sts) {
		sts = m_current_basic->RenameFile(item, filename.GetName());
		if (!sts) {
			m_current_basic->ShowErrorMessage();
		}
		// リストのファイル名を更新
		listCtrl->SetListText(view_item, LISTCOL_NAME, item->GetFileNameStr());
		// ディレクトリのときはツリー側も更新
		if (item->IsDirectory()) {
			frame->RefreshDiskListDirectoryName(m_current_basic->GetDisk());
		}
	}
	return sts;
}

/// ダブルクリックしたとき
/// ディレクトリならその内部に入る、それ以外ならプロパティダイアログ表示
void UiDiskFileList::DoubleClicked()
{
	DiskBasicDirItem *item = GetSelectedDirItem();
	if (!item) return;

	if (item->GetFileAttr().IsDirectory()) {
		frame->ChangeDirectory(m_current_basic, item, true);
	} else {
		ShowFileAttr(item);
	}
}

/// 指定したファイルを編集
void UiDiskFileList::EditDataFile(enEditorTypes editor_type)
{
	if (!m_current_basic) return;

	if (!listCtrl) return;

	MyFileListItems selected_items;
	int selcount = listCtrl->GetListSelections(selected_items);
	if (selcount <= 0) return;

	UiDiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	wxString tmp_dir_name;
	if (!app->MakeTempDir(tmp_dir_name)) {
		return;
	}

	m_current_basic->ClearErrorMessage();

	bool sts = true;
	do {
		wxString filename;
		DiskBasicDirItem *item = NULL;

		// １つだけ選択
		item = GetFileName(selected_items.Item(0), filename);
		if (!item) {
			sts = false;
			break;
		}
		// エクスポートできるか
		if (!m_current_basic->IsLoadableFile(item)) {
			sts = false;
			break;
		}
		// エクスポートする前の処理（ファイル名を変更するか）
		if (!item->PreExportDataFile(filename)) {
			sts = false;
			break;
		}
		// ファイル名を変換
		filename = Utils::EncodeFileName(filename);
		// フルパスを作成
		wxFileName path = wxFileName(tmp_dir_name, filename);

		if (!frame->ExportDataFile(m_current_basic, item, path.GetFullPath(), _("exporting..."), _("exported."))) {
			sts = false;
			break;
		}

		wxFile file;
		file.Open(path.GetFullPath());
		wxFileOffset in_file_size = file.Length();
		file.Close();

		// エディタを起動
		if (!frame->OpenFileWithEditor(editor_type, path)) {
			// コマンド起動失敗
			sts = false;
			break;
		}

		file.Open(path.GetFullPath());
		wxFileOffset out_file_size = file.Length();
		file.Close();

		if (in_file_size == out_file_size) {
			// ファイルを変更しているか
			int sts = m_current_basic->VerifyFile(item, path.GetFullPath());
			if (sts < 0) {
				// エラーあり
				sts = false;
				break;
			} else if (sts == 0) {
				// 変更なし
				break;
			}
		}

		// ディスク内にセーブする
		sts = frame->ImportDataFile(item, path.GetFullPath(), m_current_basic, m_current_basic->GetCurrentDirectory(), _("importing..."), _("imported."));

	} while(0);

	if (!sts) {
		m_current_basic->ShowErrorMessage();
	}
}

/// ファイル属性プロパティダイアログを表示
void UiDiskFileList::ShowFileAttr()
{
	DiskBasicDirItem *item = GetSelectedDirItem();
	if (!item) return;

	ShowFileAttr(item);
}

/// ファイル属性プロパティダイアログを表示
/// @param[in] item ディレクトリアイテム
/// @return true
bool UiDiskFileList::ShowFileAttr(DiskBasicDirItem *item)
{
	// 占有しているグループを再計算
	DiskBasicGroups group_items;
	item->GetAllGroups(group_items);
	item->SetGroups(group_items);

	IntNameBox *dlg = new IntNameBox(frame, this, wxID_ANY, _("File Attribute"), wxT(""), m_current_basic, item,
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
void UiDiskFileList::AcceptSubmittedFileAttr(IntNameBox *dlg)
{
	if (frame->GetUniqueNumber() != dlg->GetUniqueNumber()) {
		// リストが既に切り替わっている
		return;
	}

	DiskBasic *dlg_basic = dlg->GetDiskBasic();
	// これが有効か
	if (!frame->GetDiskImage().MatchDiskBasic(dlg_basic)) {
		return;
	}

	DiskBasicDirItem *item = dlg->GetDiskBasicDirItem();
	if (!item) return;

	bool sts = true;
	DiskBasicError *err_info = &dlg_basic->GetErrinfo();

	// 書き込み禁止か
	sts = m_current_basic->IsWritableIntoDisk();

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
			if (dlg_basic->IsFileNameDuplicated(item->GetParent(), newname, item) != 0) {
				err_info->SetError(DiskBasicError::ERRV_ALREADY_EXISTS, newname.wc_str());
				sts = false;
			}
		}
	}
	if (sts) {
		// ダイアログで指定した値をアイテムに反映
		sts = frame->SetDirItemFromIntNameDialog(item, *dlg, dlg_basic, dlg_basic->CanRenameFile(item, false));
	}
	if (!sts) {
		dlg_basic->ShowErrorMessage();
	}

	if (m_current_basic == dlg_basic) {
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
void UiDiskFileList::CloseAllFileAttr()
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
bool UiDiskFileList::ChangeBasicType()
{
	if (!m_current_basic) return false;

	DiskImageDisk *disk = m_current_basic->GetDisk();
	if (!disk) return false;

	BasicParamBox dlg(this, wxID_ANY, _("Select BASIC Type"), disk, m_current_basic, BasicParamBox::BASIC_SELECTABLE);
	int sts = dlg.ShowModal();

	if (sts == wxID_OK) {
		bool forcely = dlg.WillOpenForcely();
		if (dlg.IsChangedBasic() || forcely) {
			ClearFiles();
			m_current_basic->ClearParseAndAssign(forcely);
			frame->RefreshDiskListOnSelectedSide(dlg.GetBasicParam());
//		} else {
//			dlg.CommitData();
//			RefreshFiles();
		}
	}
	return (sts == wxID_OK);
}

/// BASIC情報ダイアログ
void UiDiskFileList::ShowBasicAttr()
{
	if (!m_current_basic) return;

	DiskImageDisk *disk = m_current_basic->GetDisk();
	if (!disk) return;

	BasicParamBox dlg(this, wxID_ANY, _("BASIC Information"), disk, m_current_basic, 0);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		dlg.CommitData();
		RefreshFiles();
	}
}

/// カラム変更ダイアログ
void UiDiskFileList::ShowListColumnDialog()
{
	if (listCtrl->ShowListColumnRearrangeBox()) {
		// リストを更新
		RefreshFiles();
	}
}

/// カラムの幅をデフォルトに戻す
void UiDiskFileList::ResetAllListColumnWidth()
{
	listCtrl->ResetAllListColumnWidth();
	// リストを更新
	RefreshFiles();
}

/// ディレクトリ作成ダイアログ
void UiDiskFileList::ShowMakeDirectoryDialog()
{
	if (!m_current_basic) return;

	if (!m_current_basic->IsWritableIntoDisk()) {
		m_current_basic->ShowErrorMessage();
		return;
	}

	DiskImageDisk *disk = m_current_basic->GetDisk();
	if (!disk) return;

	// 名前の入力
	DiskBasicDirItem *temp_item = m_current_basic->CreateDirItem();
	IntNameBox dlg(frame, this, wxID_ANY, _("New Directory Name"), wxT(""), m_current_basic, temp_item, wxEmptyString, wxEmptyString, 0, NULL
		, INTNAME_NEW_FILE | INTNAME_SHOW_TEXT | INTNAME_SPECIFY_FILE_NAME);

	int ans = dlg.ShowModal();

	int sts = 0;
	if (ans == wxID_OK) {
		wxString dirname;
		dlg.GetInternalName(dirname);
		if (temp_item->CanIgnoreDateTime()) {
			gConfig.IgnoreDateTime(dlg.DoesIgnoreDateTime(gConfig.DoesIgnoreDateTime()));
		}
		sts = m_current_basic->MakeDirectory(m_current_basic->GetCurrentDirectory(), dirname, gConfig.DoesIgnoreDateTime());
		if (sts != 0) {
			m_current_basic->ShowErrorMessage();
		} else {
			// リスト更新
			RefreshFiles();
			// 左パネルのツリーを更新
			DiskBasicDirItem *curr_dir = m_current_basic->GetCurrentDirectory();
			if (curr_dir) {
				// 更新させるためカレントディレクトリを未確定にする
				curr_dir->ValidDirectory(false);
			}
			frame->RefreshAllDirectoryNodesOnDiskList(disk, m_current_basic->GetSelectedSide(), curr_dir);
		}
	}

	delete temp_item;
}

/// 選択している行数
int UiDiskFileList::GetListSelectedItemCount() const
{
	return listCtrl->GetListSelectedItemCount();
}

/// BASICディスクとして使用できるか
bool UiDiskFileList::CanUseBasicDisk() const
{
	return m_current_basic ? m_current_basic->CanUse() : false;
}
/// BASICディスクを解析したか
bool UiDiskFileList::IsAssignedBasicDisk() const
{
	return m_current_basic? m_current_basic->IsAssigned() : false;
}
/// BASICディスク＆フォーマットできるか
bool UiDiskFileList::IsFormattableBasicDisk() const
{
	return m_current_basic? m_current_basic->IsFormattable() : false;
}
/// BASICディスク＆フォーマットされているか
bool UiDiskFileList::IsFormattedBasicDisk() const
{
	return m_current_basic ? m_current_basic->IsFormatted() : false;
}
/// ファイルの書き込み可能か
bool UiDiskFileList::IsWritableBasicFile()
{
	return m_current_basic ? m_current_basic->IsWritableIntoDisk() : false;
}
/// ファイルの削除可能か
bool UiDiskFileList::IsDeletableBasicFile()
{
	return m_current_basic ? m_current_basic->IsDeletableFiles() : false;
}
/// FATエリアの空き状況を取得
void UiDiskFileList::GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const
{
	if (m_current_basic) m_current_basic->GetFatAvailability(offset, arr);
}
/// フォントをセット
void UiDiskFileList::SetListFont(const wxFont &font)
{
	textAttr->SetFont(font);
	listCtrl->SetFont(font);
	Refresh();
}
