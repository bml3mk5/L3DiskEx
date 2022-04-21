/// @file uifilelist.cpp
///
/// @brief BASICファイル名一覧
///

#include "uifilelist.h"
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/numformatter.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/clipbrd.h>
#include "main.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "intnamebox.h"
#include "basicselbox.h"
#include "diskresult.h"
#include "utils.h"


#ifndef USE_DND_ON_TOP_PANEL
// ドラッグアンドドロップ時のフォーマットID
wxDataFormat *L3DiskFileListDataFormat = NULL;
#endif

/// ドラッグ時のデータ構成
typedef struct st_directory_for_dnd {
	wxUint8		name[32];
	int			file_size;
	int			file_type;
	int			start_addr;
	int			exec_addr;
	int			external_attr;
	wxUint8		date[3];
	wxUint8		time[3];
	wxUint8		reserved[4];
} directory_for_dnd_t;

#if 0
//
//
//
L3DiskListItem::L3DiskListItem(int newfiletype, int newdatatype)
{
	file_type = newfiletype;
	data_type = newdatatype;
}
#endif

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

#if 0
//
//
//
L3DiskFileListRenderer::L3DiskFileListRenderer()
	: wxDataViewTextRenderer(wxT("string") , wxDATAVIEW_CELL_EDITABLE)
{
}

wxWindow *L3DiskFileListRenderer::CreateEditorCtrl(wxWindow *parent, wxRect labelRect, const wxVariant &value)
{
	wxWindow *win = wxDataViewTextRenderer::CreateEditorCtrl(parent, labelRect, value);
	return win;
}

bool L3DiskFileListRenderer::Validate(wxVariant &value)
{
	wxWindow *text = GetEditorCtrl();
	if (!text) return false;
	wxValidator *vali = text->GetValidator();
	if (!vali) return true;
	return vali->Validate(text->GetParent());
}

//
//
//
L3DiskFileListCtrl::L3DiskFileListCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator)
	: wxDataViewListCtrl(parent, id, pos, size, style, validator)
{
}
bool L3DiskFileListCtrl::ProcessEvent(wxEvent &event)
{
	const wxEventType t = event.GetEventType();
	if (t == wxEVT_CHAR_HOOK || t == wxEVT_KEY_UP) {
		wxGetApp().SetModifiers(((const wxKeyEvent &)event).GetModifiers());
	}
	
	bool sts = wxDataViewListCtrl::ProcessEvent(event);
	return sts;
}
#endif

//
// 右パネルのファイルリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskFileList, wxPanel)
	EVT_SIZE(L3DiskFileList::OnSize)

	EVT_DATAVIEW_ITEM_CONTEXT_MENU(IDC_VIEW_LIST, L3DiskFileList::OnDataViewItemContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(IDC_VIEW_LIST, L3DiskFileList::OnDataViewItemActivated)

	EVT_DATAVIEW_SELECTION_CHANGED(IDC_VIEW_LIST, L3DiskFileList::OnSelectionChanged)

	EVT_DATAVIEW_ITEM_BEGIN_DRAG(IDC_VIEW_LIST, L3DiskFileList::OnBeginDrag)

//	EVT_CONTEXT_MENU(L3DiskFileList::OnContextMenu)
	EVT_BUTTON(IDC_BTN_CHANGE, L3DiskFileList::OnButtonChange)
	EVT_RADIOBUTTON(IDC_RADIO_CHAR_ASCII, L3DiskFileList::OnChangeCharCode)
	EVT_RADIOBUTTON(IDC_RADIO_CHAR_SJIS , L3DiskFileList::OnChangeCharCode)

	EVT_MENU(IDM_EXPORT_FILE, L3DiskFileList::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, L3DiskFileList::OnImportFile)

	EVT_MENU(IDM_DELETE_FILE, L3DiskFileList::OnDeleteFile)
	EVT_MENU(IDM_RENAME_FILE, L3DiskFileList::OnRenameFile)

	EVT_MENU(IDM_MAKE_DIRECTORY, L3DiskFileList::OnMakeDirectory)

	EVT_MENU(IDM_PROPERTY, L3DiskFileList::OnProperty)

	EVT_DATAVIEW_ITEM_START_EDITING(IDC_VIEW_LIST, L3DiskFileList::OnFileNameStartEditing)
	EVT_DATAVIEW_ITEM_EDITING_STARTED(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditingStarted)
	EVT_DATAVIEW_ITEM_EDITING_DONE(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditedDone)
wxEND_EVENT_TABLE()

L3DiskFileList::L3DiskFileList(L3DiskFrame *parentframe, wxWindow *parentwindow)
       : wxPanel(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	initialized = false;
	parent   = parentwindow;
	frame    = parentframe;
	
	list = NULL;

//	wxSize sz = parentwindow->GetClientSize();
//	SetSize(sz);

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);

	wxSize size(440, -1);
	textAttr = new wxTextCtrl(this, IDC_TEXT_ATTR, wxT(""), wxDefaultPosition, size, wxTE_READONLY | wxTE_LEFT);
	hbox->Add(textAttr, wxSizerFlags().Expand().Border(wxBOTTOM | wxTOP, 2));
	size.x = 80;
	btnChange = new wxButton(this, IDC_BTN_CHANGE, _("Change"), wxDefaultPosition, size);
	btnChange->Enable(false);
	hbox->Add(btnChange, flags);

	hbox->Add(new wxStaticText(this, wxID_ANY, _("Charactor Code")), wxSizerFlags().Center().Border(wxBOTTOM | wxTOP, 2).Border(wxLEFT | wxRIGHT, 8));
	radCharAscii = new wxRadioButton(this, IDC_RADIO_CHAR_ASCII, _("Ascii"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	hbox->Add(radCharAscii, flags);
	radCharSJIS = new wxRadioButton(this, IDC_RADIO_CHAR_SJIS, _("Shift JIS"), wxDefaultPosition, wxDefaultSize);
	hbox->Add(radCharSJIS, flags);

	radCharAscii->SetValue(true);
	radCharSJIS->SetValue(false);

	vbox->Add(hbox);

	list = new wxDataViewListCtrl(this, IDC_VIEW_LIST, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES | wxDV_MULTIPLE);
	vbox->Add(list, wxSizerFlags().Expand());

	L3DiskFileListStoreModel *model = new L3DiskFileListStoreModel(parentframe);
	list->AssociateModel(model);
	model->DecRef();

//	listColumns[LISTCOL_NAME] = list->AppendTextEditColumn(_("File Name"), 160, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );
	listColumns[LISTCOL_NAME] = list->AppendTextColumn(_("File Name"), wxDATAVIEW_CELL_EDITABLE, 160, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );
	listColumns[LISTCOL_ATTR] = list->AppendTextColumn(_("Attributes"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );
	listColumns[LISTCOL_SIZE] = list->AppendTextColumn(_("Size"), wxDATAVIEW_CELL_INERT, 60, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_GROUPS] = list->AppendTextColumn(_("Groups"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_START] = list->AppendTextColumn(_("Start Group"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_TRACK] = list->AppendTextColumn(_("Track"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_SIDE] = list->AppendTextColumn(_("Side"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_SECTOR] = list->AppendTextColumn(_("Sector"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );
	listColumns[LISTCOL_DATE] = list->AppendTextColumn(_("Date Time"), wxDATAVIEW_CELL_INERT, 150, wxALIGN_LEFT, wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE  );

//	vbox->SetSizeHints(this);

	SetSizerAndFit(vbox);
	Layout();

#ifndef USE_DND_ON_TOP_PANEL
	// drag and drop
	if (!L3DiskFileListDataFormat) {
		L3DiskFileListDataFormat = new wxDataFormat(wxT("L3DISKFILELISTDATA"));
	}
	SetDropTarget(new L3DiskFileListDropTarget(parentframe, this));
#endif

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

#if 0
	UINT fno = 0;
	char fname[257];
	::OpenClipboard(::GetWindow(NULL, GW_OWNER));
	do {
		fno = ::EnumClipboardFormats(fno);
		::GetClipboardFormatNameA(fno, fname, 256);
	} while(fno != 0);
	::CloseClipboard();
#endif
	
	initialized = true;
}

L3DiskFileList::~L3DiskFileList()
{
	delete menuPopup;
#ifndef USE_DND_ON_TOP_PANEL
	delete L3DiskFileListDataFormat;
	L3DiskFileListDataFormat = NULL;
#endif
}

/// リサイズ
void L3DiskFileList::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();
//	if (textAttr) textAttr->SetSize(size.x - 80, textAttr->GetSize().y);
//	if (btnChange) btnChange->SetSize(80, btnChange->GetSize().y);
	if (list) {
		wxPoint pt = list->GetPosition();
		list->SetSize(size.x, size.y - pt.y);
	}
}

/// 選択行を変更した
void L3DiskFileList::OnSelectionChanged(wxDataViewEvent& event)
{
	if (!initialized) return;

	if (GetSelectedRow() == wxNOT_FOUND) {
		// 非選択
		UnselectItem();
		return;
	}
	wxDataViewItem selected_item = list->GetSelection();
#if 1
	wxUint32 start_group = (wxUint32)-1;
	DiskD88Sector *sector = basic.GetSectorFromPosition(list->GetItemData(selected_item), &start_group);
	if (!sector) {
		// セクタなし
		UnselectItem();
		return;
	}
	// 選択
	SelectItem(start_group, sector, basic.GetCharCode(), basic.IsDataInverted());
#else
	DiskBasicGroups arr;
	if (!basic.GetGroupsFromPosition(list->GetItemData(selected_item), arr)) {
		// セクタなし
		UnselectItem();
		return;
	}

	// 選択
	SelectItem(arr);
#endif
}

/// リストの編集開始
void L3DiskFileList::OnFileNameStartEditing(wxDataViewEvent& event)
{
	wxDataViewItem listitem = event.GetItem();
	int pos = (int)list->GetItemData(listitem);
	DiskBasicDirItem *ditem = basic.GetDirItem(pos);
	if (!ditem || !ditem->IsFileNameEditable()) {
		// 編集不可
		event.Veto();
	}
}

/// リストの編集開始した
void L3DiskFileList::OnFileNameEditingStarted(wxDataViewEvent& event)
{
	wxDataViewColumn *column = event.GetDataViewColumn();
	wxDataViewRenderer *renderer = column->GetRenderer();
	if (!renderer) return;
	wxTextCtrl *text = (wxTextCtrl *)renderer->GetEditorCtrl();
	if (!text) return;

	wxDataViewItem listitem = event.GetItem();
	int pos = (int)list->GetItemData(listitem);
	DiskBasicDirItem *ditem = basic.GetDirItem(pos);
	if (ditem && ditem->IsFileNameEditable()) {
		int max_len = ditem->GetFileNameStrSize();
		IntNameValidator validate(&basic, ditem);
		text->SetMaxLength(max_len);
		text->SetValidator(validate);
	}
}

/// リストの編集終了
void L3DiskFileList::OnFileNameEditedDone(wxDataViewEvent& event)
{
	if (event.IsEditCancelled()) return;
	wxDataViewItem listitem = event.GetItem();

	wxVariant vari = event.GetValue();

	wxString newname = vari.GetString();

	RenameDataFile(listitem, newname);
}

/// リスト上で右クリック
void L3DiskFileList::OnDataViewItemContextMenu(wxDataViewEvent& event)
{
	ShowPopupMenu();
}

/// 右クリック
void L3DiskFileList::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// リスト上でダブルクリック
void L3DiskFileList::OnDataViewItemActivated(wxDataViewEvent& event)
{
	DoubleClicked();
}

/// リスト上でドラッグ開始
void L3DiskFileList::OnBeginDrag(wxDataViewEvent& event)
{
#ifdef __WXMSW__
	if (wxGetApp().GetModifiers() & (wxMOD_CONTROL | wxMOD_RAW_CONTROL)) {
		// Ctrlキーを押している場合 外部へのドロップ
		DragDataSourceForExternal();
	} else {
		// 内部へのドロップ
		DragDataSourceForInternal();
	}
#else
	if (list->GetSelectedItemsCount() > 1) {
		// 複数選択時、外部へのドロップ
		DragDataSourceForExternal();
	} else {
		// 内部へのドロップ
		DragDataSourceForInternal();
	}
#endif
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
	int num = event.GetId() - IDC_RADIO_CHAR_ASCII;
	if (num != basic.GetCharCode()) {
		basic.SetCharCode(num);
		RefreshFiles();
	}
}

/// ポップアップメニュー表示
void L3DiskFileList::ShowPopupMenu()
{
	if (!menuPopup) return;

	bool opened = basic.IsFormatted();
	menuPopup->Enable(IDM_IMPORT_FILE, opened /* && IsWritableBasicFile() */);

	menuPopup->Enable(IDM_MAKE_DIRECTORY, opened && CanMakeDirectory());

	opened = (opened && (list->GetSelectedItemsCount() > 0));
	menuPopup->Enable(IDM_EXPORT_FILE, opened);
	menuPopup->Enable(IDM_DELETE_FILE, opened /* && IsDeletableBasicFile() */);

	opened = (opened && (list->GetSelectedRow() != wxNOT_FOUND));
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

/// ファイル名をリストにセット
/// @param [in] newdisk    ディスク
/// @param [in] newsidenum サイド番号 -1:両面 >0:裏表あり
void L3DiskFileList::SetFiles(DiskD88Disk *newdisk, int newsidenum)
{
	// トラックが存在するか
	bool found = newdisk->ExistTrack(newsidenum);
	if (found) {
		// ディスクをDISK BASICとして解析
		if (basic.ParseDisk(newdisk, newsidenum, false) != 0) {
			basic.ShowErrorMessage();
		}
	} else {
		// トラックがない
		DiskResult result;
		result.SetWarn(DiskResult::ERR_NO_FOUND_TRACK);
		ResultInfo::ShowMessage(result.GetValid(), result.GetMessages());
		basic.Clear();
	}

	// ルートディレクトリをセット
	bool valid = basic.AssignRootDirectory();
	if (!valid) return;

	// ファイル名一覧を設定
	RefreshFiles();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// ファイル名をリストに再設定
void L3DiskFileList::RefreshFiles()
{
	list->DeleteAllItems();

	// 属性をセット
	SetAttr(basic.GetDescription());

	// ファイル名一覧を設定
	DiskBasicDir *dir = basic.GetDir();
	for(size_t i=0; i<dir->Count(); i++) {
		DiskBasicDirItem *item = dir->ItemPtr(i);
		if (!item->IsUsedAndVisible()) continue;
		wxVector<wxVariant> data;
		long lval;

		data.push_back( item->GetFileNameStr() );	// ファイル名
		data.push_back( item->GetFileAttrStr() );	// ファイル属性
		lval = item->GetFileSize();
		data.push_back( lval >= 0 ? wxNumberFormatter::ToString(lval) : wxT("---") );	// ファイルサイズ
		lval = item->GetGroupSize();
		data.push_back( lval >= 0 ? wxNumberFormatter::ToString(lval) : wxT("---") );	// 使用グループ数
		lval = item->GetStartGroup();
		data.push_back( wxString::Format(wxT("%02x"), (int)lval));	// 開始グループ

		int track_num = -1;
		int side_num = -1;
		int sector_start = -1;
		if (basic.CalcStartNumFromGroupNum((wxUint32)lval, track_num, side_num, sector_start)) {
			data.push_back( wxString::Format(wxT("%d"), track_num));	// トラック
			data.push_back( wxString::Format(wxT("%d"), side_num));	// サイド
			data.push_back( wxString::Format(wxT("%d"), sector_start));	// セクタ
		} else {
			data.push_back( wxT("-") );	// トラック
			data.push_back( wxT("-") );	// サイド
			data.push_back( wxT("-") );	// セクタ
		}
		data.push_back( item->GetFileDateTimeStr() );	// 日時

		list->AppendItem( data, (wxUIntPtr)i );
	}

	btnChange->Enable(true);

//	list->Scroll(0, 0);
	list->ScrollWindow(0, 0);

	// FAT空き状況を確認
	if (frame->GetFatAreaFrame()) {
		frame->SetFatAreaData();
	}
}

/// リストをクリア
void L3DiskFileList::ClearFiles()
{
	btnChange->Enable(false);

	list->DeleteAllItems();
	ClearAttr();

	basic.Clear();

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
	frame->SetBinDumpData(sector->GetSectorBuffer(), sector->GetSectorSize(), char_code, invert);
	// FAT使用状況を更新
	frame->SetFatAreaGroup(group_num);

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

#if 0
void L3DiskFileList::SelectItem(const DiskBasicGroups *group_items, int char_code, bool invert)
{
	// ダンプリストをセット
	DiskD88Disk *disk = basic.GetDisk();
	DiskD88Track *ptrack;

	frame->ClearBinDumpData();

	// ダンプリストをセット
	for(size_t i=0; i<group_items->Count(); i++) {
		DiskBasicGroupItem *itm = &group_items->Item(i);
		ptrack = disk->GetTrack(itm->track, itm->side);
		if (!ptrack) {
			break;
		}
		for(int s=itm->sector_start; s<=itm->sector_end; s++) {
			DiskD88Sector *sector;
			sector = ptrack->GetSector(s);
			if (!sector) {
				break;
			}
			if (s == itm->sector_start) {
				frame->SetBinDumpData(sector->GetSectorBuffer(), sector->GetSectorSize(), char_code, invert);
			} else {
				frame->AppendBinDumpData(sector->GetSectorBuffer(), sector->GetSectorSize(), char_code, invert);
			}
		}
	}
	// FAT使用状況を更新
	frame->SetFatAreaGroup(group_items, INVALID_GROUP_NUMBER);

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}
#endif

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
	DiskD88Disk *disk = basic.GetDisk();
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
			frame->SetBinDumpData(sector->GetSectorBuffer(), sector->GetSectorSize());
		} else {
			frame->AppendBinDumpData(sector->GetSectorBuffer(), sector->GetSectorSize());
		}
	}
	return true;
}

/// データをダンプウィンドウに設定
bool L3DiskFileList::SetDumpData(int track, int side, int sector_start, int sector_end)
{
	DiskD88Disk *disk = basic.GetDisk();
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
			frame->SetBinDumpData(psector->GetSectorBuffer(), psector->GetSectorSize());
		} else {
			frame->AppendBinDumpData(psector->GetSectorBuffer(), psector->GetSectorSize());
		}
	}
	return true;
}

/// 現在選択している行のディレクトリアイテムを得る
DiskBasicDirItem *L3DiskFileList::GetSelectedDirItem()
{
	if (!list) return NULL;
	int row = list->GetSelectedRow();
	if (row == wxNOT_FOUND) return NULL;
	wxDataViewItem view_item = list->GetSelection();
	return GetDirItem(view_item);
}

/// リストの指定行のディレクトリアイテムを得る
DiskBasicDirItem *L3DiskFileList::GetDirItem(const wxDataViewItem &view_item, int *item_pos)
{
	if (!list) return NULL;
	if (list->GetItemCount() <= 0) return NULL;
	size_t dir_pos = list->GetItemData(view_item);
	if (item_pos) *item_pos = (int)dir_pos;
	DiskBasicDirItem *dir_item = basic.GetDirItem(dir_pos);
	return dir_item;
}

/// リストの指定行のディレクトリアイテムとそのファイル名を得る
DiskBasicDirItem *L3DiskFileList::GetFileName(const wxDataViewItem &view_item, wxString &name, int *item_pos)
{
	DiskBasicDirItem *dir_item = GetDirItem(view_item, item_pos);
	if (!dir_item) return NULL;

	name = dir_item->GetFileNameStr();

	return dir_item;
}

/// エクスポートダイアログ
bool L3DiskFileList::ShowExportDataFileDialog()
{
	bool sts = true;

	wxDataViewItemArray selected_items;
	int selcount = list->GetSelections(selected_items);
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
			frame->GetExportFilePath(),
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
			frame->GetExportFilePath()
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
	frame->SetExportFilePath(path);

	bool sts = basic.LoadFile(item, path);
	if (!sts) {
		basic.ShowErrorMessage();
	}
	return sts;
}

/// ドラッグする 内部へドロップする場合
bool L3DiskFileList::DragDataSourceForInternal()
{
	DiskBasicDirItem *item = GetSelectedDirItem();
	if (!item) return false;

	// 同じリストへのDnD
	wxMemoryOutputStream ostream;

	// ディレクトリヘッダ部分を設定
	directory_for_dnd_t dnd_header;
	memset(&dnd_header, 0, sizeof(dnd_header));
	item->GetFileName(dnd_header.name, sizeof(dnd_header.name));
	dnd_header.file_size = item->GetFileSize();
	dnd_header.file_type = item->GetFileType();
	dnd_header.start_addr = item->GetStartAddress();
	dnd_header.exec_addr = item->GetExecuteAddress();
	dnd_header.external_attr = item->GetExternalAttr();
	struct tm tm;
	item->GetFileDateTime(&tm);
	L3DiskUtils::ConvTmToDateTime(&tm, dnd_header.date,	dnd_header.time);
	ostream.Write(&dnd_header, sizeof(directory_for_dnd_t));

	// ファイルの読み込み
	bool sts = basic.AccessData(item, NULL, &ostream);
	if (!sts) return false;

	wxStreamBuffer *buffer = ostream.GetOutputStreamBuffer();

#ifdef USE_DND_ON_TOP_PANEL
	wxCustomDataObject custom_data(*L3DiskPanelDataFormat);
#else
	wxCustomDataObject custom_data(*L3DiskFileListDataFormat);
#endif
	custom_data.SetData(buffer->GetBufferSize(), buffer->GetBufferStart());

	wxDropSource dragSource(this);
	dragSource.SetData(custom_data);

	dragSource.DoDragDrop();

	return true;
}

/// ドラッグする 外部へドロップ場合
bool L3DiskFileList::DragDataSourceForExternal()
{
	wxString tmp_dir_name;
	wxFileDataObject file_object;
	bool sts = CreateFileObject(tmp_dir_name, file_object);
	if (sts) {
		// ファイルをドロップ
#ifdef __WXMSW__
		wxDropSource dragSource(file_object);
#else
		wxDropSource dragSource(file_object, frame);
#endif
		dragSource.DoDragDrop();

#ifdef __WXMSW__
		// macでは別プロセスで動くようなのでここで削除しない。
		ReleaseFileObject(tmp_dir_name);
#endif
	}
	return sts;
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

/// ファイルをテンポラリディレクトリにエクスポートしファイルリストを作成する（DnD, クリップボード用）
bool L3DiskFileList::CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object)
{
	if (!list) return false;

	wxDataViewItemArray selected_items;
	int selcount = list->GetSelections(selected_items);
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
		sts = basic.AccessData(item, NULL, &ostream);
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
	// Read some text
	wxFileDataObject file_object;

	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported( wxDF_FILENAME )) {
			wxTheClipboard->GetData( file_object );
		}
		wxTheClipboard->Close();
	}

	if (!basic.IsFormatted()) {
		return false;
	}
	if (!basic.IsWritableIntoDisk()) {
		basic.ShowErrorMessage();
		return false;
	}

	wxArrayString file_names = file_object.GetFilenames(); 
	bool sts = true;
	for(size_t n = 0; n < file_names.Count(); n++) {
		sts &= ImportDataFile(file_names.Item(n));
	}
	if (!sts) {
		basic.ShowErrorMessage();
	}
	return sts;
}

/// インポートダイアログ
bool L3DiskFileList::ShowImportDataFileDialog()
{
	if (!basic.IsFormatted()) {
		return false;
	}
	if (!basic.IsWritableIntoDisk()) {
		basic.ShowErrorMessage();
		return false;
	}

	L3DiskFileDialog dlg(
		_("Import file"),
		frame->GetExportFilePath(),
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
	wxString intname;
	do {
		int dlgsts = dlg.ShowModal();
		intname = dlg.GetInternalName();

		if (dlgsts == wxID_OK) {
			// ファイルサイズのチェック
			int limit = 0;
			if (!item->IsFileValidSize(dlg.GetFileType1(), file_size, &limit)) {
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
			if (basic.FindFile(intname, NULL, NULL)) {
				wxString msg = wxString::Format(_("File '%s' already exists, do you really want to overwrite it?"), intname);
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
	frame->SetExportFilePath(path);

	if (!basic.IsFormatted()) {
		return false;
	}

	// ディスクの残りサイズ
	int file_size = 0;
	if (!basic.CheckFile(path, file_size)) {
		return false;
	}

	// ダイアログ生成
	DiskBasicDirItem *temp_item = basic.CreateDirItem();
	IntNameBox dlg(frame, this, wxID_ANY, _("Import File"), &basic, temp_item, path
		, INTNAME_SHOW_NO_PROPERTY | INTNAME_INVALID_FILE_TYPE | INTNAME_SPECIFY_FILE_NAME);
	// 日付は現在日付をセット
	dlg.SetDateTime(wxDateTime::Now());

	// ダイアログ表示と同じファイルがあるかチェック
	int ans = ShowIntNameBoxAndCheckSameFile(dlg, temp_item, file_size);

	bool sts = false;
	if (ans == wxYES) {
		// ダイアログで指定した値をアイテムに反映
		dlg.SetValuesToDirItem();
		// 機種依存の属性をアイテムに反映
		sts = temp_item->SetAttrInAttrDialog(dlg.GetAttrControls(), basic.GetErrinfo());
		// ディスク内にセーブする
		DiskBasicDirItem *madeitem = NULL;
		if (sts) {
			sts = basic.SaveFile(path, temp_item, &madeitem);
		}
		if (!sts) {
			basic.ShowErrorMessage();
		}
		RefreshFiles();
	}

	delete temp_item;
	return sts;
}

/// 指定したデータをインポート（内部でのドラッグ＆ドロップ時など）
bool L3DiskFileList::ImportDataFile(const wxUint8 *buffer, size_t buflen)
{
	if (!basic.IsFormatted()) {
		return false;
	}
	if (!basic.IsWritableIntoDisk()) {
		basic.ShowErrorMessage();
		return false;
	}

	// バッファの始めはヘッダ
	directory_for_dnd_t *dnd_header = (directory_for_dnd_t *)buffer;
	const wxUint8 *data_buffer = buffer + sizeof(directory_for_dnd_t);
	size_t data_buflen = buflen - sizeof(directory_for_dnd_t);

	// ディスクの残りサイズ
	if (!basic.CheckFile(data_buffer, data_buflen)) {
		return false;
	}

	// ディレクトリアイテムを新規に作成して、各パラメータをセット
	wxString filename;
	DiskBasicDirItem *temp_item = basic.CreateDirItem();
	temp_item->SetFileAttr(dnd_header->file_type);
	filename = temp_item->RemakeFileName(dnd_header->name, sizeof(dnd_header->name));
	temp_item->SetStartAddress(dnd_header->start_addr);
	temp_item->SetExecuteAddress(dnd_header->exec_addr);
	temp_item->SetExternalAttr(dnd_header->external_attr);
	struct tm tm;
	L3DiskUtils::ConvDateTimeToTm(dnd_header->date, dnd_header->time, &tm);
	temp_item->SetFileDateTime(&tm);

	// ダイアログ表示
	IntNameBox dlg(frame, this, wxID_ANY, _("Copy File"), &basic, temp_item, wxEmptyString, INTNAME_SHOW_NO_PROPERTY);
	dlg.SetInternalName(filename);
	dlg.SetDateTime(temp_item->GetFileDateStr(), temp_item->GetFileTimeStr());

	// ダイアログ表示と同じファイルがあるかチェック
	int ans = ShowIntNameBoxAndCheckSameFile(dlg, temp_item, (int)data_buflen);

	bool sts = false;
	if (ans == wxYES) {
		// ダイアログで指定した値をアイテムに反映
		dlg.SetValuesToDirItem();
		// 機種依存の属性をアイテムに反映
		sts = temp_item->SetAttrInAttrDialog(dlg.GetAttrControls(), basic.GetErrinfo());
		// ディスク内にセーブ
		DiskBasicDirItem *madeitem = NULL;
		if (sts) {
			sts = basic.SaveFile(data_buffer, data_buflen, temp_item, &madeitem);
		}
		if (!sts) {
			basic.ShowErrorMessage();
		}
		RefreshFiles();
	}

	delete temp_item;
	return sts;
}

/// 指定したファイルを削除
bool L3DiskFileList::DeleteDataFile()
{
	bool sts = true;
	wxDataViewItemArray selected_items;
	int selcount = list->GetSelections(selected_items);
	if (selcount < 0) return false;

	if (!basic.IsDeletableFiles()) {
		basic.ShowErrorMessage();
		return false;
	}

	if (selcount == 1) {
		// 1ファイル選択時
		wxString filename;
		DiskBasicDirItem *item = GetFileName(selected_items.Item(0), filename);
		if (!item) return false;

		DiskBasicGroups group_items;
		sts = basic.IsDeletableFile(item, group_items);
		if (sts) {
			wxString msg = wxString::Format(_("Do you really want to delete '%s'?"), filename);
			int ans = wxMessageBox(msg, _("Delete a file"), wxYES_NO);
			if (ans != wxYES) {
				return false;
			}

			sts = basic.DeleteItem(item, group_items);
		}
	} else {
		// 複数ファイル選択時
		wxString msg = _("Do you really want to delete selected files?");
		int ans = wxMessageBox(msg, _("Delete files"), wxYES_NO);
		if (ans != wxYES) {
			return false;
		}

		for(int n = 0; n < selcount; n++) {
			wxString filename;
			DiskBasicDirItem *item = GetFileName(selected_items.Item(n), filename);
			if (!item) continue;

			DiskBasicGroups group_items;
			bool del_ok = basic.IsDeletableFile(item, group_items, false);
			sts &= del_ok;
			if (del_ok) {
				sts &= basic.DeleteItem(item, group_items);
			}
		}
	}

	if (!sts) {
		basic.ShowErrorMessage();
	}
	RefreshFiles();

	return sts;
}

/// ファイル名の編集開始
void L3DiskFileList::StartEditingFileName()
{
	if (list->GetSelectedRow() == wxNOT_FOUND) return;

	wxDataViewItem selected_item = list->GetSelection();

	// 編集可能にする
	wxDataViewColumn *column = list->GetColumn(0);
	list->EditItem(selected_item, column);
}

/// 指定したファイル名を変更
bool L3DiskFileList::RenameDataFile(const wxDataViewItem &view_item, const wxString &newname)
{
	DiskBasicDirItem *item = GetDirItem(view_item);
	if (!item) return false;

	wxString filename = newname;
	// ダイアログ入力後のファイル名文字列を変換
	item->ConvertFromFileNameStr(filename);

	bool sts = true;
	wxString errmsg;
	IntNameValidator validator(&basic, item);
	if (sts && !validator.Validate(frame, filename)) {
		sts = false;
	}
	if (sts && basic.FindFile(filename, item, NULL)) {
		errmsg = wxString::Format(_("File '%s' already exists."), filename);
		wxMessageBox(errmsg, _("File exists"), wxOK | wxICON_EXCLAMATION, parent);
		sts = false;
	}
	if (sts) {
		sts = basic.RenameFile(item, filename);
		if (!sts) {
			basic.ShowErrorMessage();
		}
		RefreshFiles();
	}
	return sts;
}

/// ダブルクリックしたとき
/// ディレクトリならその内部に入る、それ以外ならプロパティダイアログ表示
void L3DiskFileList::DoubleClicked()
{
	DiskBasicDirItem *item = GetSelectedDirItem();
	if (!item) return;

	if (item->GetFileType() & FILE_TYPE_DIRECTORY_MASK) {
		ChangeDirectory(item);
	} else {
		ShowFileAttr(item);
	}
}

/// ディレクトリを移動する
bool L3DiskFileList::ChangeDirectory(DiskBasicDirItem *dir_item)
{
	bool sts = basic.ChangeDirectory(dir_item);
	if (sts) {
		// リスト更新
		RefreshFiles();
	}
	return sts;
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
	IntNameBox *dlg = new IntNameBox(frame, this, wxID_ANY, _("File Attribute"), &basic, item, wxEmptyString, INTNAME_SHOW_ALL);

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
	if (basic.GetDir()->GetUniqueNumber() != dlg->GetUniqueNumber()) {
		// リストが既に切り替わっている
		return;
	}

	DiskBasicDirItem *item = dlg->GetDiskBasicDirItem();
	if (!item) return;

	wxString newname = dlg->GetInternalName();
	if (item->IsFileNameEditable()) {
		if (basic.FindFile(newname, item, NULL)) {
			wxString msg = wxString::Format(_("File '%s' already exists."), newname);
			wxMessageBox(msg, _("File exists"));
			return;
		}
	}
	struct tm tm;
	dlg->GetDateTime(&tm);
	bool sts = basic.RenameFile(item, newname, dlg->CalcFileType(), dlg->GetStartAddress(), dlg->GetExecuteAddress(), &tm, &dlg->GetAttrControls());
	if (!sts) {
		basic.ShowErrorMessage();
	}

	RefreshFiles();
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
	DiskD88Disk *disk = basic.GetDisk();
	if (!disk) return false;

	BasicSelBox dlg(this, wxID_ANY, disk);
	int dlgsts = dlg.ShowModal();

	bool sts = false;
	if (dlgsts == wxID_OK) {
		sts = true;
		disk->SetDiskBasicParam(dlg.GetBasicParam());
//		basic.SetCharCode(dlg.GetCharCode());
		SetFiles(disk, basic.GetSelectedSide());
	}
	return sts;
}

/// ディスクの論理フォーマット
bool L3DiskFileList::FormatDisk()
{
	DiskD88Disk *disk = basic.GetDisk();
	if (!disk) return false;

	int ans;

	// BASIC種類を選択
	BasicSelBox dlg(this, wxID_ANY, disk);
	ans = dlg.ShowModal();
	if (ans != wxID_OK) {
		return false;
	}
	disk->SetDiskBasicParam(dlg.GetBasicParam());
//	basic.SetCharCode(dlg.GetCharCode());
	basic.ParseDisk(disk, basic.GetSelectedSide(), true);

	if (!basic.IsFormattable()) {
		basic.ShowErrorMessage();
		return false;
	}

	ans = wxYES;
	wxString diskname = wxT("'")+disk->GetName()+wxT("'");
	int selected_side = basic.GetSelectedSide();
	if (selected_side >= 0) {
		diskname += wxString::Format(_("side %c"), selected_side + 0x41);
	}
	wxString msg = wxString::Format(_("All files and datas will delete on %s. Do you really want to format it?"), diskname);
	ans = wxMessageBox(msg, _("Format"), wxYES_NO);

	bool sts = false;
	if (ans == wxYES) {
		disk->Initialize(selected_side);
		disk->SetModify();
		sts = basic.FormatDisk();
		if (!sts) {
			basic.ShowErrorMessage();

		}
		RefreshFiles();
	}
	frame->UpdateMenuAndToolBarFileList(this);
	return sts;
}

/// ディレクトリを作成できるか
bool L3DiskFileList::CanMakeDirectory() const
{
	return basic.CanMakeDirectory();
}

/// ディレクトリを作成
bool L3DiskFileList::MakeDirectory()
{
	DiskD88Disk *disk = basic.GetDisk();
	if (!disk) return false;

	// 名前の入力
	DiskBasicDirItem *temp_item = basic.CreateDirItem();
	IntNameBox dlg(frame, this, wxID_ANY, _("New Directory Name"), &basic, temp_item, wxEmptyString, INTNAME_SHOW_TEXT | INTNAME_SPECIFY_FILE_NAME);

	int ans = dlg.ShowModal();

	bool sts = false;
	if (ans == wxID_OK) {
		DiskBasicDirItem *madeitem = NULL;
		sts = basic.MakeDirectory(dlg.GetInternalName(), &madeitem);
		if (!sts) {
			basic.ShowErrorMessage();
		}
		RefreshFiles();
	}

	delete temp_item;
	return sts;
}

/// 選択している行の位置を返す
/// @return 複数行選択している場合wxNOT_FOUND
int L3DiskFileList::GetSelectedRow() const
{
	return list->GetSelectedRow();
}
/// 選択している行数
int L3DiskFileList::GetSelectedItemCount() const
{
	return list->GetSelectedItemsCount();
}
/// BASICディスクとして使用できるか
bool L3DiskFileList::CanUseBasicDisk() const
{
	return basic.CanUse();
}
/// BASICディスク＆フォーマットされているか
bool L3DiskFileList::IsFormattedBasicDisk() const
{
	return basic.IsFormatted();
}
/// ファイルの書き込み可能か
bool L3DiskFileList::IsWritableBasicFile()
{
	return basic.IsWritableIntoDisk();
}
/// ファイルの削除可能か
bool L3DiskFileList::IsDeletableBasicFile()
{
	return basic.IsDeletableFiles();
}
/// FATエリアの空き状況を取得
void L3DiskFileList::GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const
{
	return basic.GetFatAvailability(offset, arr);
}

#ifndef USE_DND_ON_TOP_PANEL
//
// File Drag and Drop
//
L3DiskFileListDropTarget::L3DiskFileListDropTarget(L3DiskFrame *parentframe, L3DiskFileList *parentwindow)
	: wxDropTarget()
{
	parent = parentwindow;
	frame = parentframe;

	wxDataObjectComposite* dataobj = new wxDataObjectComposite();
	// from explorer, finder etc.
	dataobj->Add(new wxFileDataObject(), true);
	// from own appli
	dataobj->Add(new wxCustomDataObject(*L3DiskFileListDataFormat));
	SetDataObject(dataobj);
}

bool L3DiskFileListDropTarget::OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames)
{
	if (filenames.Count() > 0) {
		wxString name = filenames.Item(0);
		parent->ImportDataFile(name);
	}
    return true;
}

wxDragResult L3DiskFileListDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	if ( !GetData() ) return wxDragNone;
	bool sts = false;
	wxDataObjectComposite *comobj = (wxDataObjectComposite *)GetDataObject();
	if (comobj) {
		wxDataFormat fmt = comobj->GetReceivedFormat();
		if (fmt.GetType() == wxDF_FILENAME) {
			// エクスプローラからのDnD
			wxFileDataObject *dobj = (wxFileDataObject *)comobj->GetObject(fmt);
			sts = OnDropFiles( x, y, dobj->GetFilenames() );
		} else if (fmt == *L3DiskFileListDataFormat) {
			// このアプリからのDnD
			wxCustomDataObject *dobj = (wxCustomDataObject *)comobj->GetObject(fmt);
			size_t buflen = dobj->GetDataSize();
			wxUint8 *buffer = (wxUint8 *)dobj->GetData();
			sts = parent->ImportDataFile(buffer, buflen);
		}
	}
	return (sts ? def : wxDragNone);
}
#endif
