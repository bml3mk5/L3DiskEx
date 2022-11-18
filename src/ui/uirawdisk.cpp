/// @file uirawdisk.cpp
///
/// @brief ディスクID一覧
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uirawdisk.h"
#include <wx/menu.h>
#include <wx/regex.h>
#include <wx/filename.h>
#include <wx/msgdlg.h>
#include <wx/dataobj.h>
#include <wx/clipbrd.h>
#include <wx/utils.h>
#include "../main.h"
#include "rawtrackbox.h"
#include "rawsectorbox.h"
#include "rawparambox.h"
#include "rawexpbox.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////

const struct st_list_columns gL3DiskRawTrackColumnDefs[] = {
	{ "Num",		wxTRANSLATE("Num"),				false,	42,	wxALIGN_RIGHT,	true },
	{ "Track",		wxTRANSLATE("Track"),			false,	32,	wxALIGN_RIGHT,	false },
	{ "Side",		wxTRANSLATE("Side"),			false,	32,	wxALIGN_RIGHT,	false },
	{ "Sectors",	wxTRANSLATE("NumOfSectors"),	false,	40,	wxALIGN_RIGHT,	false },
	{ "Offset",		wxTRANSLATE("Offset"),			false,	60,	wxALIGN_RIGHT,	true },
	{ NULL,			NULL,							false,	 0,	wxALIGN_LEFT,	false }
};

const struct st_list_columns gL3DiskRawSectorColumnDefs[] = {
	{ "Num",		wxTRANSLATE("Num"),				false,	40,	wxALIGN_RIGHT,	true },
	{ "IDC",		wxTRANSLATE("C"),				false,	40,	wxALIGN_RIGHT,	false },
	{ "IDH",		wxTRANSLATE("H"),				false,	40,	wxALIGN_RIGHT,	false },
	{ "IDR",		wxTRANSLATE("R"),				false,	40,	wxALIGN_RIGHT,	true },
	{ "IDN",		wxTRANSLATE("N"),				false,	40,	wxALIGN_RIGHT,	false },
	{ "Deleted",	wxTRANSLATE("Deleted"),			false,	36,	wxALIGN_CENTER,	false },
	{ "Single",		wxTRANSLATE("SingleDensity"),	false,	36,	wxALIGN_CENTER,	false },
	{ "Sectors",	wxTRANSLATE("NumOfSectors"),	false,	72,	wxALIGN_RIGHT,	false },
	{ "Size",		wxTRANSLATE("Size"),			false,	72,	wxALIGN_RIGHT,	false },
	{ "Status",		wxTRANSLATE("Status"),			false,	40,	wxALIGN_CENTER,	false },
	{ NULL,			NULL,							false,	 0,	wxALIGN_LEFT,	false }
};

//////////////////////////////////////////////////////////////////////
//
// 分割ウィンドウ
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRawPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

L3DiskRawPanel::L3DiskRawPanel(L3DiskFrame *parentframe, wxWindow *parentwindow)
                : wxSplitterWindow(parentwindow, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_BORDER | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
	parent = parentwindow;
    frame = parentframe;

	invert_data = false;
	reverse_side = false;

	// fit size on parent window
	wxSize sz = parentwindow->GetClientSize();
	SetSize(sz);

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	// control panel
	lpanel = new L3DiskRawTrack(frame, this);
	rpanel = new L3DiskRawSector(frame, this);
	SplitVertically(lpanel, rpanel, 236);

	SetMinimumPaneSize(10);
}

/// トラックリストにデータを設定する
void L3DiskRawPanel::SetTrackListData(DiskD88Disk *disk, int side_num)
{
	lpanel->SetTracks(disk, side_num);
	frame->UpdateMenuAndToolBarRawDisk(this);
}

/// トラックリストをクリアする
void L3DiskRawPanel::ClearTrackListData()
{
	lpanel->ClearTracks();
	frame->UpdateMenuAndToolBarRawDisk(this);
}

/// トラックリストを再描画する
void L3DiskRawPanel::RefreshTrackListData()
{
	lpanel->RefreshTracks();
}

/// トラックリストが存在するか
bool L3DiskRawPanel::TrackListExists() const
{
	return (lpanel->GetDisk() != NULL);
}

/// トラックリストの選択行を返す
int L3DiskRawPanel::GetTrackListSelectedRow() const
{
	return (int)lpanel->GetListSelectedRow();
}

/// セクタリストにデータを設定する
void L3DiskRawPanel::SetSectorListData(DiskD88Track *track)
{
	rpanel->SetSectors(track);
	frame->UpdateMenuAndToolBarRawDisk(this);
}

/// セクタリストをクリアする
void L3DiskRawPanel::ClearSectorListData()
{
	rpanel->ClearSectors();
	frame->UpdateMenuAndToolBarRawDisk(this);
}

/// セクタリストの選択行を返す
int L3DiskRawPanel::GetSectorListSelectedRow() const
{
	return rpanel->GetListSelectedRow();
}

/// トラックリストとセクタリストを更新
void L3DiskRawPanel::RefreshAllData()
{
	lpanel->RefreshTracks();
	rpanel->RefreshSectors();
}

/// クリップボードヘコピー
bool L3DiskRawPanel::CopyToClipboard()
{
	wxWindow *fwin = wxWindow::FindFocus();
	if (fwin == lpanel) {
		return lpanel->CopyToClipboard();
	} else if (fwin == rpanel) {
		return rpanel->CopyToClipboard();
	}
	return true;
}

/// クリップボードからペースト
bool L3DiskRawPanel::PasteFromClipboard()
{
	// トラック側
	return lpanel->PasteFromClipboard();
}

/// エクスポートダイアログ表示
bool L3DiskRawPanel::ShowExportDataDialog()
{
	wxWindow *fwin = wxWindow::FindFocus();
	if (fwin == lpanel) {
		return lpanel->ShowExportTrackDialog();
	} else if (fwin == rpanel) {
		return rpanel->ShowExportDataFileDialog();
	}
	return true;
}

/// インポートダイアログ表示
bool L3DiskRawPanel::ShowImportDataDialog()
{
	// トラック側のダイアログを表示
	return lpanel->ShowImportTrackDialog();
}

/// データを削除ダイアログ表示
bool L3DiskRawPanel::ShowDeleteDataDialog()
{
	wxWindow *fwin = wxWindow::FindFocus();
	if (fwin == lpanel) {
	} else if (fwin == rpanel) {
		rpanel->DeleteSector();
	}
	return true;
}

/// トラックへインポートダイアログ（トラックの範囲指定）表示
bool L3DiskRawPanel::ShowImportTrackRangeDialog(const wxString &path, int st_trk, int st_sid, int st_sec)
{
	return lpanel->ShowImportTrackRangeDialog(path, st_trk, st_sid, st_sec);
}

/// セクタからエクスポートダイアログ表示
bool L3DiskRawPanel::ShowExportDataFileDialog()
{
	return rpanel->ShowExportDataFileDialog();
}

/// セクタへインポートダイアログ表示
bool L3DiskRawPanel::ShowImportDataFileDialog()
{
	return rpanel->ShowImportDataFileDialog();
}

/// トラックのID一括変更
void L3DiskRawPanel::ModifyIDonTrack(int type_num)
{
	rpanel->ModifyIDonTrack(type_num);
}

/// トラックの密度一括変更
void L3DiskRawPanel::ModifyDensityOnTrack()
{
	rpanel->ModifyDensityOnTrack();
}

/// トラックのセクタ数一括変更
void L3DiskRawPanel::ModifySectorsOnTrack()
{
	rpanel->ModifySectorsOnTrack();
}

/// トラックのセクタサイズ一括変更
void L3DiskRawPanel::ModifySectorSizeOnTrack()
{
	rpanel->ModifySectorSizeOnTrack();
}

/// トラック or セクタのプロパティダイアログ表示
bool L3DiskRawPanel::ShowRawDiskAttr()
{
	wxWindow *fwin = wxWindow::FindFocus();
	if (fwin == lpanel) {
		lpanel->ShowTrackAttr();
	} else if (fwin == rpanel) {
		rpanel->ShowSectorAttr();
	}
	return true;
}

/// セクタのプロパティダイアログ表示
bool L3DiskRawPanel::ShowSectorAttr()
{
	return rpanel->ShowSectorAttr();
}

/// ファイル名を生成
/// @param[in] sector セクタ
/// @return ファイル名
wxString L3DiskRawPanel::MakeFileName(DiskD88Sector *sector)
{
	return MakeFileName(
		sector->GetIDC(), sector->GetIDH(), sector->GetIDR(),
		sector->GetIDC(), sector->GetIDH(), sector->GetIDR()
	);
}

/// ファイル名を生成
/// @param[in] st_c 開始トラック番号
/// @param[in] st_h 開始サイド番号
/// @param[in] st_r 開始セクタ番号
/// @param[in] ed_c 終了トラック番号
/// @param[in] ed_h 終了サイド番号
/// @param[in] ed_r 終了セクタ番号
/// @return ファイル名
wxString L3DiskRawPanel::MakeFileName(int st_c, int st_h, int st_r, int ed_c, int ed_h, int ed_r)
{
	return wxString::Format(wxT("%02d-%02d-%02d--%02d-%02d-%02d.bin"),
		st_c, st_h, st_r, ed_c, ed_h, ed_r
	);
}

/// フォントをセット
/// @param[in] font フォントデータ
void L3DiskRawPanel::SetListFont(const wxFont &font)
{
	lpanel->SetFont(font);
	lpanel->Refresh();
	rpanel->SetFont(font);
	rpanel->Refresh();
}

//////////////////////////////////////////////////////////////////////
//
//
//
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
L3DiskRawTrackListStoreModel::L3DiskRawTrackListStoreModel(wxWindow *parent)
	: wxDataViewListStore()
{
	ctrl = (L3DiskRawTrack *)parent;
}

/// トラックリストのソート
int L3DiskRawTrackListStoreModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const
{
	DiskD88Disk *disk = ctrl->GetDisk();
	if (!disk) return 0;

	int idx = -1;
	if (!ctrl->FindColumn(col, &idx)) return 0;

	int cmp = 0;
	int i1 = (int)GetItemData(item1);
	int i2 = (int)GetItemData(item2);
	switch(idx) {
	case TRACKCOL_OFFSET:
		cmp = L3DiskRawTrackListCtrl::CompareOffset(disk, i1, i2, ascending ? 1 : -1);
		break;
	case TRACKCOL_NUM:
		cmp = L3DiskRawTrackListCtrl::CompareNum(disk, i1, i2, ascending ? 1 : -1);
		break;
	default:
		break;
	}
	return cmp;
}
#endif

//////////////////////////////////////////////////////////////////////
//
// トラックリストコントロール
//
L3DiskRawTrackListCtrl::L3DiskRawTrackListCtrl(L3DiskFrame *parentframe, wxWindow *parent, L3DiskRawTrack *sub, wxWindowID id)
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
	: L3CDListCtrl(
		parentframe, parent, id,
		gL3DiskRawTrackColumnDefs,
		NULL,
		wxDV_SINGLE,
		new L3DiskRawTrackListStoreModel(sub)
	)
#else
	: L3CListCtrl(
		parentframe, parent, id,
		gL3DiskRawTrackColumnDefs,
		-1, -1,
		NULL,
		wxLC_SINGLE_SEL
	)
#endif
{
#if 0
	AssignListIcons(icons_for_seclist);
#endif
}

/// リストデータを設定
/// @param[in] disk ディスク
/// @param[in] pos  ディスク内のトラック位置(0～)
/// @param[in] row  リストの行位置
/// @param[in] idx  リスト内の通し番号
/// @param[out] values 整形した値を返す
void L3DiskRawTrackListCtrl::SetListData(DiskD88Disk *disk, int pos, int row, int idx, L3RawTrackListValue *values)
{
	wxUint32 offset = disk->GetOffset(pos);
	DiskD88Track *trk = NULL;
	if (offset > 0) {
		trk = disk->GetTrackByOffset(offset);
	} else {
		trk = disk->GetTrack(pos);
	}
	int trk_num = -1;
	int sid_num = -1;
	int sec_trk = -1;
	if (trk) {
		trk_num = trk->GetTrackNumber();
		sid_num = trk->GetSideNumber();
		sec_trk = trk->GetSectorsPerTrack();
	}

	values[TRACKCOL_NUM].Set(row, wxString::Format(wxT("%d"), idx));
	values[TRACKCOL_TRACK].Set(row, trk_num >= 0 ? wxString::Format(wxT("%d"), trk_num) : wxT("--"));
	values[TRACKCOL_SIDE].Set(row, sid_num >= 0 ? wxString::Format(wxT("%d"), sid_num) : wxT("--"));
	values[TRACKCOL_SECS].Set(row, sec_trk >= 0 ? wxString::Format(wxT("%d"), sec_trk) : wxT("--"));
	values[TRACKCOL_OFFSET].Set(row, wxString::Format(wxT("%x"), offset));
}

/// リストにデータを挿入
/// @param[in] disk ディスク
/// @param[in] pos  ディスク内のトラック位置(0～)
/// @param[in] row  リストの行位置
/// @param[in] idx  リスト内の通し番号
void L3DiskRawTrackListCtrl::InsertListData(DiskD88Disk *disk, int pos, int row, int idx)
{
	L3RawTrackListValue values[TRACKCOL_END];

	SetListData(disk, pos, row, idx, values);

	InsertListItem(row, values, TRACKCOL_END, (wxUIntPtr)idx);
}

/// リストデータを更新
/// @param[in] disk ディスク
/// @param[in] pos  ディスク内のトラック位置(0～)
/// @param[in] row  リストの行位置
/// @param[in] idx  リスト内の通し番号
void L3DiskRawTrackListCtrl::UpdateListData(DiskD88Disk *disk, int pos, int row, int idx)
{
	L3RawTrackListValue values[TRACKCOL_END];

	SetListData(disk, pos, row, idx, values);

	UpdateListItem(row, values, TRACKCOL_END, (wxUIntPtr)idx);
}

#ifdef USE_LIST_CTRL_ON_TRACK_LIST
//
//
//

/// ソート用アイテム
struct st_track_list_sort_exp {
	DiskD88Disk *disk;
	int (*cmpfunc)(DiskD88Disk *disk, int i1, int i2, int dir);
	int dir;
};

/// アイテムをソート
/// @param[in] disk        ディスク
/// @param[in] side_number サイド番号
/// @param[in] col         リストの列位置
void L3DiskRawTrackListCtrl::SortDataItems(DiskD88Disk *disk, int side_number, int col)
{
	struct st_track_list_sort_exp exp;

	// ソート対象か
	int idx;
	bool match_col;
	exp.dir = SelectColumnSortDir(col, idx, match_col);

	// 番号かID Rの時のみソート
	if (col >= 0 && match_col) {
		if (disk) {
			// ソート
			exp.disk = disk;
			switch(idx) {
			case TRACKCOL_OFFSET:
				exp.cmpfunc = &CompareOffset;
				break;
			case TRACKCOL_NUM:
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

/// ソート用コールバック
int wxCALLBACK L3DiskRawTrackListCtrl::Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata)
{
	struct st_track_list_sort_exp *exp = (struct st_track_list_sort_exp *)sortdata;

	int cmp = exp->cmpfunc != NULL ? exp->cmpfunc(exp->disk, (int)item1, (int)item2, exp->dir) : 0;
	if (cmp == 0) cmp = ((int)item1 - (int)item2);
	return cmp;
}
#endif

int L3DiskRawTrackListCtrl::CompareOffset(DiskD88Disk *disk, int i1, int i2, int dir)
{
	return (disk->GetOffset(i1) - disk->GetOffset(i2)) * dir;
}
int L3DiskRawTrackListCtrl::CompareNum(DiskD88Disk *disk, int i1, int i2, int dir)
{
	return (i1 - i2) * dir;
}

//////////////////////////////////////////////////////////////////////
//
// 左パネルのトラックリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRawTrack, L3DiskRawTrackListCtrl)
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, L3DiskRawTrack::OnListActivated)
	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, L3DiskRawTrack::OnListItemSelected)
	EVT_DATAVIEW_ITEM_BEGIN_DRAG(wxID_ANY, L3DiskRawTrack::OnBeginDrag)

	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, L3DiskRawTrack::OnListContextMenu)
#else
	EVT_LIST_ITEM_SELECTED(wxID_ANY, L3DiskRawTrack::OnListItemSelected)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, L3DiskRawTrack::OnListActivated)
	EVT_LIST_COL_CLICK(wxID_ANY, L3DiskRawTrack::OnColumnClick)

	EVT_LIST_BEGIN_DRAG(wxID_ANY, L3DiskRawTrack::OnBeginDrag)

	EVT_CONTEXT_MENU(L3DiskRawTrack::OnContextMenu)
#endif
	EVT_CHAR(L3DiskRawTrack::OnChar)

	EVT_MENU(IDM_EXPORT_TRACK, L3DiskRawTrack::OnExportTrack)
	EVT_MENU(IDM_IMPORT_TRACK, L3DiskRawTrack::OnImportTrack)

	EVT_MENU(IDM_INVERT_DATA, L3DiskRawTrack::OnChangeInvertData)
	EVT_MENU(IDM_REVERSE_SIDE, L3DiskRawTrack::OnChangeReverseSide)

	EVT_MENU(IDM_MODIFY_ID_H_DISK, L3DiskRawTrack::OnModifyIDonDisk)
	EVT_MENU(IDM_MODIFY_ID_N_DISK, L3DiskRawTrack::OnModifyIDonDisk)
	EVT_MENU(IDM_MODIFY_DENSITY_DISK, L3DiskRawTrack::OnModifyDensityOnDisk)

	EVT_MENU(IDM_MODIFY_ID_C_TRACK, L3DiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_H_TRACK, L3DiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_N_TRACK, L3DiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_DENSITY_TRACK, L3DiskRawTrack::OnModifyDensityOnTrack)
	EVT_MENU(IDM_MODIFY_SECTORS_TRACK, L3DiskRawTrack::OnModifySectorsOnTrack)
	EVT_MENU(IDM_MODIFY_SIZE_TRACK, L3DiskRawTrack::OnModifySectorSizeOnTrack)

	EVT_MENU(IDM_APPEND_TRACK, L3DiskRawTrack::OnAppendTrack)
	EVT_MENU(IDM_DELETE_TRACKS_BELOW, L3DiskRawTrack::OnDeleteTracksBelow)

	EVT_MENU(IDM_PROPERTY_TRACK, L3DiskRawTrack::OnPropertyTrack)
wxEND_EVENT_TABLE()

L3DiskRawTrack::L3DiskRawTrack(L3DiskFrame *parentframe, L3DiskRawPanel *parentwindow)
       : L3DiskRawTrackListCtrl(parentframe, parentwindow, this, wxID_ANY)
{
	parent   = parentwindow;
	frame    = parentframe;

	disk = NULL;
	side_number = -1;

	wxFont font;
	frame->GetDefaultListFont(font);
	SetFont(font);

	// popup menu
	menuPopup = new wxMenu;
	wxMenu *sm = new wxMenu;
		sm->AppendCheckItem(IDM_INVERT_DATA,  _("Invert datas."));
		sm->AppendCheckItem(IDM_REVERSE_SIDE, _("Descend side number order."));
	menuPopup->AppendSubMenu(sm, _("Behavior When In/Out"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_EXPORT_TRACK, _("&Export Track..."));
	menuPopup->Append(IDM_IMPORT_TRACK, _("&Import..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MODIFY_ID_H_DISK, _("Modify All H On This Disk"));
	menuPopup->Append(IDM_MODIFY_ID_N_DISK, _("Modify All N On This Disk"));
	menuPopup->Append(IDM_MODIFY_DENSITY_DISK, _("Modify All Density On This Disk"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MODIFY_ID_C_TRACK, _("Modify All C On This Track"));
	menuPopup->Append(IDM_MODIFY_ID_H_TRACK, _("Modify All H On This Track"));
	menuPopup->Append(IDM_MODIFY_ID_N_TRACK, _("Modify All N On This Track"));
	menuPopup->Append(IDM_MODIFY_DENSITY_TRACK, _("Modify All Density On This Track"));
	menuPopup->Append(IDM_MODIFY_SECTORS_TRACK, _("Modify All Num Of Sectors On This Track"));
	menuPopup->Append(IDM_MODIFY_SIZE_TRACK, _("Modify All Sector Size On This Track"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_APPEND_TRACK, _("Append New Track"));
	menuPopup->Append(IDM_DELETE_TRACKS_BELOW, _("Delete All Tracks Below Current Track"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY_TRACK, _("&Property"));
}

L3DiskRawTrack::~L3DiskRawTrack()
{
	delete menuPopup;
}

/// トラックリストを選択
void L3DiskRawTrack::OnListItemSelected(L3RawTrackListEvent& event)
{
	if (!disk) return;

	SelectData();
}

/// トラックリストをダブルクリック
void L3DiskRawTrack::OnListActivated(L3RawTrackListEvent &event)
{
	ShowTrackAttr();
}

/// トラックリスト右クリック
void L3DiskRawTrack::OnListContextMenu(L3RawTrackListEvent& event)
{
	ShowPopupMenu();
}

/// トラックリスト右クリック
void L3DiskRawTrack::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// カラムをクリック
void L3DiskRawTrack::OnColumnClick(L3RawTrackListEvent &event)
{
	int col = event.GetColumn();
#ifdef USE_LIST_CTRL_ON_TRACK_LIST
	SortDataItems(disk, side_number, col);
#endif
}

/// トラックをエクスポート選択
void L3DiskRawTrack::OnExportTrack(wxCommandEvent& event)
{
	ShowExportTrackDialog();
}

/// トラックにインポート選択
void L3DiskRawTrack::OnImportTrack(wxCommandEvent& event)
{
	ShowImportTrackDialog();
}

/// データを反転するチェック選択
void L3DiskRawTrack::OnChangeInvertData(wxCommandEvent& event)
{
	parent->InvertData(event.IsChecked());
}

/// サイドを逆転するチェック選択
void L3DiskRawTrack::OnChangeReverseSide(wxCommandEvent& event)
{
	parent->ReverseSide(event.IsChecked());
}

/// セクタリストからドラッグ開始
void L3DiskRawTrack::OnBeginDrag(L3RawTrackListEvent& event)
{
	DragDataSourceForExternal();
}

/// ディスク上のID一括変更選択
void L3DiskRawTrack::OnModifyIDonDisk(wxCommandEvent& event)
{
	ModifyIDonDisk(event.GetId() - IDM_MODIFY_ID_C_DISK);
}

/// トラックのID一括変更選択
void L3DiskRawTrack::OnModifyIDonTrack(wxCommandEvent& event)
{
	parent->ModifyIDonTrack(event.GetId() - IDM_MODIFY_ID_C_TRACK);
}

/// ディスク上の密度一括変更選択
void L3DiskRawTrack::OnModifyDensityOnDisk(wxCommandEvent& event)
{
	ModifyDensityOnDisk();
}

/// トラックの密度一括変更選択
void L3DiskRawTrack::OnModifyDensityOnTrack(wxCommandEvent& event)
{
	parent->ModifyDensityOnTrack();
}

/// トラックのセクタ数を一括変更選択
void L3DiskRawTrack::OnModifySectorsOnTrack(wxCommandEvent& event)
{
	parent->ModifySectorsOnTrack();
}

/// トラックのセクタサイズを一括変更
void L3DiskRawTrack::OnModifySectorSizeOnTrack(wxCommandEvent& event)
{
	parent->ModifySectorSizeOnTrack();
}

/// 新規トラックを追加選択
void L3DiskRawTrack::OnAppendTrack(wxCommandEvent& event)
{
	AppendTrack();
}

/// 現在のトラック以下を削除選択
void L3DiskRawTrack::OnDeleteTracksBelow(wxCommandEvent& event)
{
	DeleteTracks();
}

/// トラックプロパティ選択
void L3DiskRawTrack::OnPropertyTrack(wxCommandEvent& event)
{
	ShowTrackAttr();
}

/// キー押下
void L3DiskRawTrack::OnChar(wxKeyEvent& event)
{
	switch(event.GetKeyCode()) {
//	case WXK_RETURN:
//		// Enter    ダブルクリックと同じ
//		ShowTrackAttr();
//		break;
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

/// 選択
void L3DiskRawTrack::SelectData()
{
	int num = GetListSelectedNum();
	wxUint32 offset = disk->GetOffset(num);
	DiskD88Track *trk = NULL;
	if (offset > 0) {
		trk = disk->GetTrackByOffset(offset);
	} else {
		trk = disk->GetTrack(num);
	}
	if (trk) {
		parent->SetSectorListData(trk);
	} else {
		parent->ClearSectorListData();
	}
}

/// トラックリストをセット
/// @param[in] newdisk    表示対象ディスク
/// @param[in] newsidenum 表示対象ディスクのサイド番号
void L3DiskRawTrack::SetTracks(DiskD88Disk *newdisk, int newsidenum)
{
	if (!newdisk) return;

	disk = newdisk;
	side_number = newsidenum;

	SetTracks();

	// セクタリストはクリア
	UnselectListItem(GetListSelection());
	parent->ClearSectorListData();
}

/// トラックリストを再セット
void L3DiskRawTrack::RefreshTracks()
{
	L3RawTrackListItem sel_pos = GetListSelection();
	L3RawTrackListItem foc_pos = GetListFocusedItem();
	SetTracks();
	if (foc_pos >= 0) FocusListItem(foc_pos);
	if (sel_pos >= 0) SelectListItem(sel_pos);
}

/// トラックリストを再セット
void L3DiskRawTrack::SetTracks()
{
	if (!disk) return;

	int sides = disk->GetSidesPerDisk();
	int row = 0;
	int row_count = (int)GetItemCount();

	int max_pos = disk->GetTracksPerSide() * sides;
	if (max_pos < DISKD88_MAX_TRACKS) max_pos = DISKD88_MAX_TRACKS;
	
	for(int pos = (side_number >= 0 ? side_number : 0); pos < max_pos; pos+=(side_number >= 0 ? sides : 1)) {
		if (row < row_count) {
			UpdateListData(disk, pos, row, pos);
		} else {
			InsertListData(disk, pos, row, pos);
		}
		row++;
	}
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
	// 余分な行は消す
	for(int idx = row; idx < row_count; idx++) {
		DeleteItem((unsigned)row);
	}
#else
#ifndef USE_VIRTUAL_ON_LIST_CTRL
	// 余分な行は消す
	for(int idx = row; idx < row_count; idx++) {
		DeleteItem(row);
	}
#else
	SetItemCount(row);
#endif
#endif

#ifdef USE_LIST_CTRL_ON_TRACK_LIST
#ifdef USE_VIRTUAL_ON_LIST_CTRL
	SetItemCount(row);
#endif
	// ソート
	SortDataItems(disk, side_number, -1);
#endif
}

/// トラックリストをクリア
void L3DiskRawTrack::ClearTracks()
{
	DeleteAllListItems();

	disk = NULL;
	side_number = -1;

	// セクタリストクリア
	parent->ClearSectorListData();
}

/// トラックリスト上のポップアップメニュー表示
void L3DiskRawTrack::ShowPopupMenu()
{
	if (!menuPopup) return;
	int num = 0;
	wxUint32 offset = 0;

	menuPopup->Check(IDM_INVERT_DATA, parent->InvertData());
	menuPopup->Check(IDM_REVERSE_SIDE, parent->ReverseSide());

	bool opened = (disk != NULL);
	menuPopup->Enable(IDM_EXPORT_TRACK, opened);
	menuPopup->Enable(IDM_IMPORT_TRACK, opened);

	menuPopup->Enable(IDM_MODIFY_ID_H_DISK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_N_DISK, opened);
	menuPopup->Enable(IDM_MODIFY_DENSITY_DISK, opened);

	menuPopup->Enable(IDM_APPEND_TRACK, opened);

	menuPopup->Enable(IDM_PROPERTY_TRACK, opened);

	if (opened) {
		num = GetListSelectedNum();
		offset = disk->GetOffset(num);
	}

	opened = (opened && offset > 0);
	menuPopup->Enable(IDM_MODIFY_ID_C_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_H_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_N_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_DENSITY_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_SECTORS_TRACK, opened);

	menuPopup->Enable(IDM_DELETE_TRACKS_BELOW, opened);

	PopupMenu(menuPopup);
}

/// ドラッグする 外部へドロップ場合
bool L3DiskRawTrack::DragDataSourceForExternal()
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

//#ifdef __WXMSW__
//		// macでは別プロセスで動くようなのでここで削除しない。
//		ReleaseFileObject(tmp_dir_name);
//#endif
	}
	return sts;
}

// クリップボードへコピー
bool L3DiskRawTrack::CopyToClipboard()
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
bool L3DiskRawTrack::CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object)
{
	int selected_count = GetListSelectedItemCount();
	if (selected_count <= 0) return false;

	L3DiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	if (!app->MakeTempDir(tmp_dir_name)) {
		return false;
	}

	int cnt = 0;
	L3RawTrackListItems sels;
	GetListSelections(sels);
	for(size_t row = 0; row < sels.Count(); row++) {
		DiskD88Track *track = GetTrack(sels[row]);
		if (!track) continue;

		int st_sec, ed_sec;
		if (!GetFirstAndLastSectorNumOnTrack(track, st_sec, ed_sec)) continue;

		wxString filename = parent->MakeFileName(
			track->GetTrackNumber(), track->GetSideNumber(), st_sec,
			track->GetTrackNumber(), track->GetSideNumber(), ed_sec
		);

		// ファイルパスを作成
		wxFileName file_path(tmp_dir_name, filename);

		bool sts = ExportTrackDataFile(file_path.GetFullPath(),
			track->GetTrackNumber(), track->GetSideNumber(), st_sec,
			track->GetTrackNumber(), track->GetSideNumber(), ed_sec,
			parent->InvertData(), parent->ReverseSide()
		);
		if (sts) {
			// ファイルリストに追加
			file_object.AddFile(file_path.GetFullPath());
			cnt++;
		}
	}

	return (cnt > 0);
}

// ファイルリストを解放（DnD, クリップボード用）
void L3DiskRawTrack::ReleaseFileObject(const wxString &tmp_dir_name)
{
	L3DiskApp *app = &wxGetApp();

	// テンポラリディレクトリを削除
	app->RemoveTempDir(tmp_dir_name);
}

/// クリップボードからペースト
bool L3DiskRawTrack::PasteFromClipboard()
{
	// Read some text
	wxFileDataObject file_object;

	if (wxTheClipboard->Open()) {
		if (wxTheClipboard->IsSupported( wxDF_FILENAME )) {
			wxTheClipboard->GetData( file_object );
		}
		wxTheClipboard->Close();
	}

	wxArrayString file_names = file_object.GetFilenames();
	if (file_names.Count() != 1) return false;

	bool sts = true;
	for(size_t n = 0; n < file_names.Count(); n++) {
		sts &= ShowImportTrackRangeDialog(file_names.Item(n));
	}

	return sts;
}

/// トラックをエクスポート ダイアログ表示
bool L3DiskRawTrack::ShowExportTrackDialog()
{
	if (!disk) return false;

	DiskD88Track *track = GetSelectedTrack();
	DiskD88Sector *sector = NULL;
	if (!GetFirstSectorOnTrack(&track, &sector)) {
		return false;
	}

	wxString caption = _("Export data from track");

	RawExpBox dlg(this, wxID_ANY, caption, disk, side_number
		, track->GetTrackNumber(), track->GetSideNumber(), sector->GetSectorNumber()
		, -1, -1, -1
		, parent->InvertData(), parent->ReverseSide()
	);
	int sts = dlg.ShowModal();
	if (sts != wxID_OK) return false;

	wxString filename = parent->MakeFileName(
		dlg.GetTrackNumber(0), dlg.GetSideNumber(0), dlg.GetSectorNumber(0),
		dlg.GetTrackNumber(1), dlg.GetSideNumber(1), dlg.GetSectorNumber(1));

	L3DiskFileDialog fdlg(
		caption,
		frame->GetIniExportFilePath(),
		filename,
		_("All files (*.*)|*.*"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	sts = fdlg.ShowModal();
	wxString path = fdlg.GetPath();

	if (sts == wxID_OK) {
		return ExportTrackDataFile(path
		, dlg.GetTrackNumber(0), dlg.GetSideNumber(0), dlg.GetSectorNumber(0)
		, dlg.GetTrackNumber(1), dlg.GetSideNumber(1), dlg.GetSectorNumber(1)
		, dlg.InvertData(), dlg.ReverseSide()
		);
	} else {
		return false;
	}
}

/// 指定したファイルにトラックデータをエクスポート
/// @param [in] path ファイルパス
/// @param [in] st_trk 開始トラック番号
/// @param [in] st_sid 開始サイド番号
/// @param [in] st_sec 開始セクタ番号
/// @param [in] ed_trk 終了トラック番号
/// @param [in] ed_sid 終了サイド番号
/// @param [in] ed_sec 終了セクタ番号
/// @param [in] inv_data データを反転する
/// @param [in] rev_side サイドを逆転する（降順）
/// @return true:成功 / false:エラー
bool L3DiskRawTrack::ExportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec, bool inv_data, bool rev_side)
{
	// エクスポート元パスを覚えておく
	frame->SetIniExportFilePath(path);

	if (!disk) return false;

	int trk = st_trk;
	int sid = st_sid;
	int sec = st_sec;
	int ssid = side_number >= 0 ? side_number : 0;
	int esid = side_number >= 0 ? side_number : (disk->GetSidesPerDisk() - 1);
	int esec = disk->GetSectorsPerTrack();

	wxFile outfile(path, wxFile::write);
	if (!outfile.IsOpened()) return false;

	int sid_dir = 1;
	if (rev_side) {
		// 開始サイドと終了サイドを入れ替え
		int swp = esid;
		esid = ssid;
		ssid = swp;
		sid_dir = -1;
	}
	esid += sid_dir;

	Utils::TempData tbuf;
	bool eof = false;
	for(;trk <= ed_trk && !eof; trk++) {
		if (trk == ed_trk) {
			// 最終トラックなら終了サイドまで
			esid = ed_sid;
			esid += sid_dir;
		}
		for(; sid != esid && !eof; sid += sid_dir) {
			DiskD88Track *track = disk->GetTrack(trk, sid);
			if (track) {
				if (trk == ed_trk && sid == ed_sid) {
					// 最終トラック＆最終サイドなら終了セクタまで
					esec = ed_sec;
				}
				for(; sec <= esec && !eof; sec++) {
					DiskD88Sector *sector = track->GetSector(sec);
					if (sector) {
						size_t bufsize = sector->GetSectorSize();
						wxUint8 *buf = sector->GetSectorBuffer();

						tbuf.SetData(buf, bufsize, inv_data);
						if (outfile.Write(tbuf.GetData(), tbuf.GetSize()) == 0) {
							eof = true;
						}
					}
				}
				sec = 1;
			}
		}
		sid = ssid;
	}

	return !eof;
}

/// トラックにインポート ダイアログ表示
bool L3DiskRawTrack::ShowImportTrackDialog()
{
	if (!disk) return false;

	wxString caption = _("Import data to track");

	L3DiskFileDialog fdlg(
		caption,
		frame->GetIniExportFilePath(),
		wxEmptyString,
		_("All files (*.*)|*.*"),
		wxFD_OPEN);
	int sts = fdlg.ShowModal();
	if (sts != wxID_OK) return false;

	wxString path = fdlg.GetPath();

	return ShowImportTrackRangeDialog(path);
}

/// 指定したファイルのファイル名にある数値から指定したトラックにインポートする
/// @param [in] path ファイルパス
/// @param [in] st_trk 開始トラック番号
/// @param [in] st_sid 開始サイド番号
/// @param [in] st_sec 開始セクタ番号
/// @return true:成功 / false:エラー
bool L3DiskRawTrack::ShowImportTrackRangeDialog(const wxString &path, int st_trk, int st_sid, int st_sec)
{
	int ed_trk = st_trk;
	int ed_sid = st_sid;
	int ed_sec = st_sec;

	wxString caption = _("Import data to the disk");

	wxRegEx re("([0-9][0-9])-([0-9][0-9])-([0-9][0-9])--([0-9][0-9])-([0-9][0-9])-([0-9][0-9])");
	if (re.Matches(path)) {
		wxString sval;
		long lval;
		sval = re.GetMatch(path, 1); sval.ToLong(&lval);
		st_trk = (int)lval;
		sval = re.GetMatch(path, 2); sval.ToLong(&lval);
		st_sid = (int)lval;
		sval = re.GetMatch(path, 3); sval.ToLong(&lval);
		st_sec = (int)lval;
		sval = re.GetMatch(path, 4); sval.ToLong(&lval);
		ed_trk = (int)lval;
		sval = re.GetMatch(path, 5); sval.ToLong(&lval);
		ed_sid = (int)lval;
		sval = re.GetMatch(path, 6); sval.ToLong(&lval);
		ed_sec = (int)lval;

	} else if (st_trk < 0) {
		DiskD88Track *track = GetSelectedTrack();
		if (!track) track = GetFirstTrack();
		if (!GetFirstAndLastSectorNumOnTrack(track, st_sec, ed_sec)) {
			return false;
		}
		st_trk = track->GetTrackNumber();
		st_sid = track->GetSideNumber();
		ed_trk = st_trk;
		ed_sid = st_sid;
	}

	RawExpBox dlg(this, wxID_ANY, caption, disk, side_number
		, st_trk, st_sid, st_sec
		, ed_trk, ed_sid, ed_sec
		, parent->InvertData(), parent->ReverseSide()
	);
	int sts = dlg.ShowModal();
	if (sts != wxID_OK) {
		return false;
	}

	return ImportTrackDataFile(path
	, dlg.GetTrackNumber(0), dlg.GetSideNumber(0), dlg.GetSectorNumber(0)
	, dlg.GetTrackNumber(1), dlg.GetSideNumber(1), dlg.GetSectorNumber(1)
	, dlg.InvertData(), dlg.ReverseSide()
	);
}

/// 指定したファイルから指定した範囲にトラックデータをインポート
/// @param [in] path ファイルパス
/// @param [in] st_trk 開始トラック番号
/// @param [in] st_sid 開始サイド番号
/// @param [in] st_sec 開始セクタ番号
/// @param [in] ed_trk 終了トラック番号
/// @param [in] ed_sid 終了サイド番号
/// @param [in] ed_sec 終了セクタ番号
/// @param [in] inv_data データを反転する
/// @param [in] rev_side サイドを逆転する（降順）
/// @return true:成功 / false:エラー
bool L3DiskRawTrack::ImportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec, bool inv_data, bool rev_side)
{
	// エクスポート元パスを覚えておく
	frame->SetIniExportFilePath(path);

	if (!disk) return false;

	int trk = st_trk;
	int sid = st_sid;
	int sec = st_sec;
	int ssid = side_number >= 0 ? side_number : 0;
	int esid = side_number >= 0 ? side_number : (disk->GetSidesPerDisk() - 1);
	int esec = disk->GetSectorsPerTrack();

	wxFile infile(path, wxFile::read);
	if (!infile.IsOpened()) return false;

	int sid_dir = 1;
	if (rev_side) {
		// 開始サイドと終了サイドを入れ替え
		int swp = esid;
		esid = ssid;
		ssid = swp;
		sid_dir = -1;
	}
	esid += sid_dir;

	bool eof = false;
	for(;trk <= ed_trk && !eof; trk++) {
		if (trk == ed_trk) {
			// 最終トラックなら終了サイドまで
			esid = ed_sid;
			esid += sid_dir;
		}
		for(; sid != esid && !eof; sid += sid_dir) {
			DiskD88Track *track = disk->GetTrack(trk, sid);
			if (track) {
				if (trk == ed_trk && sid == ed_sid) {
					// 最終トラック＆最終サイドなら終了セクタまで
					esec = ed_sec;
				}
				for(; sec <= esec && !eof; sec++) {
					DiskD88Sector *sector = track->GetSector(sec);
					if (sector) {
						size_t bufsize = sector->GetSectorSize();
						wxUint8 *buf = sector->GetSectorBuffer();

						infile.Read((void *)buf, bufsize);
						if (inv_data) {
							mem_invert(buf, bufsize);
						}

						if (infile.Eof()) {
							// ファイル終わり
							eof = true;
						}
					}
				}
				sec = 1;
			}
		}
		sid = ssid;
	}

	return true;
}

/// ディスク全体のIDを変更
/// @param [in] type_num  1:ID H  3:ID N
void L3DiskRawTrack::ModifyIDonDisk(int type_num)
{
	if (!disk) return;

	DiskD88Track *track = NULL;
	DiskD88Sector *sector = NULL;
	if (!GetFirstSectorOnTrack(&track, &sector)) {
		return;
	}

	int value = 0;
	int maxvalue = 1;
	switch(type_num) {
	case 1:
		value = sector->GetIDH();
		break;
	case 3:
		value = sector->GetIDN();
		maxvalue = DiskD88Sector::ConvSecSizeToIDN(sector->GetSectorBufferSize());
		break;
	}

	RawParamBox dlg(this, wxID_ANY, type_num, value, maxvalue);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		int newvalue = dlg.GetValue();
		if (value != newvalue) {
			DiskD88Tracks *tracks = disk->GetTracks();
			if (tracks) {
				for(size_t num=0; num < tracks->Count(); num++) {
					track = tracks->Item(num);
					if (!track) continue;
					if (side_number >= 0) {
						if (side_number != track->GetSideNumber()) continue;
					}
					switch(type_num) {
					case 1:
						track->SetAllIDH(newvalue);
						break;
					case 3:
						track->SetAllIDN(newvalue);
						break;
					}
				}
			}
			// セクタリストクリア
			parent->ClearSectorListData();
		}
	}
}

/// ディスク上の密度を一括変更
void L3DiskRawTrack::ModifyDensityOnDisk()
{
	if (!disk) return;

	DiskD88Track *track = NULL;
	DiskD88Sector *sector = NULL;
	if (!GetFirstSectorOnTrack(&track, &sector)) {
		return;
	}

	bool sdensity = sector->IsSingleDensity();

	DensityParamBox dlg(this, wxID_ANY, sdensity);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		bool newsdensity = dlg.IsSingleDensity();
		if (sdensity != newsdensity) {
			DiskD88Tracks *tracks = disk->GetTracks();
			if (tracks) {
				for(size_t num=0; num < tracks->Count(); num++) {
					track = tracks->Item(num);
					if (!track) continue;
					if (side_number >= 0) {
						if (side_number != track->GetSideNumber()) continue;
					}
					track->SetAllSingleDensity(newsdensity);
				}
			}
			// セクタリストクリア
			parent->ClearSectorListData();
		}
	}
}

/// トラック情報を表示
void L3DiskRawTrack::ShowTrackAttr()
{
	if (!disk) return;

	int num = GetListSelectedNum();
	if (num < 0) return;

	wxUint32 offset = disk->GetOffset(num);

	RawTrackBox dlg(this, wxID_ANY, num, offset, disk);
	dlg.ShowModal();
}

/// トラックを追加
void L3DiskRawTrack::AppendTrack()
{
	if (!disk) return;

	int disk_number = -1;
	int side_number = -1;

	if (disk->IsReversible()) {
		// AB面ありで選択位置がA,B面どちらか
		frame->GetDiskListSelectedPos(disk_number, side_number);
	}

	disk->AddNewTrack(side_number);

	// 画面更新
	parent->RefreshAllData();
}

/// トラックを削除
void L3DiskRawTrack::DeleteTracks()
{
	if (!disk) return;

	int disk_number = -1;
	int side_number = -1;
	int num = GetListSelectedNum();
	if (num < 0) return;

	if (disk->IsReversible()) {
		// AB面ありで選択位置がA,B面どちらか
		frame->GetDiskListSelectedPos(disk_number, side_number);
	}

	int ans = wxYES;
	wxString msg = wxString::Format(_("Do you really want to delete tracks?"));
	ans = wxMessageBox(msg, _("Delete Tracks"), wxYES_NO);
	if (ans == wxYES) {
		disk->DeleteTracks(num, -1, side_number);

		// 画面更新
//		parent->RefreshAllData();
		RefreshTracks();
		// セクタリストはクリア
		parent->ClearSectorListData();
	}
}

/// 選択行のトラックを返す
DiskD88Track *L3DiskRawTrack::GetSelectedTrack()
{
	return GetTrack(GetListSelection());
}

/// 指定行のトラックを返す
DiskD88Track *L3DiskRawTrack::GetTrack(const L3RawTrackListItem &row)
{
	if (!disk) return NULL;

	DiskD88Track *track = NULL;
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
	if (!row.IsOk()) return NULL;
#else
	if (row == wxNOT_FOUND) return NULL;
#endif
	int num = (int)GetItemData(row);
	wxUint32 offset = disk->GetOffset(num);
	if (offset > 0) {
		track = disk->GetTrackByOffset(offset);
	}
	return track;
}

/// 最初のトラックを返す
DiskD88Track *L3DiskRawTrack::GetFirstTrack()
{
	if (!disk) return NULL;
	DiskD88Track *track = NULL;
	for(int num=0; track == NULL && num < DISKD88_MAX_TRACKS; num++) {
		track = disk->GetTrack(num, side_number >= 0 ? side_number : 0);
	}
	return track;
}

/// トラックのセクタ１を得る
/// @param [in,out] track  トラック NULLのときは最初のトラックのセクタ１
/// @param [out]    sector セクタ１
bool L3DiskRawTrack::GetFirstSectorOnTrack(DiskD88Track **track, DiskD88Sector **sector)
{
	if (!track || !sector) return false;
	if (!(*track)) *track = GetFirstTrack();
	if (*track) *sector = (*track)->GetSector(1);
	if (!(*sector)) {
		*track = NULL;
		return false;
	}
	return true;
}

/// トラックの開始セクタ番号と終了セクタ番号を得る
bool L3DiskRawTrack::GetFirstAndLastSectorNumOnTrack(const DiskD88Track *track, int &start_sector, int &end_sector)
{
	if (!track) return false;
	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors || sectors->Count() == 0) return false;
	start_sector = 0xffff;
	end_sector = 0;
	for(size_t r = 0; r < sectors->Count(); r++) {
		DiskD88Sector *sector = sectors->Item(r);
		if (!sector) continue;
		if (start_sector > sector->GetSectorNumber()) start_sector = sector->GetSectorNumber(); 
		if (end_sector < sector->GetSectorNumber()) end_sector = sector->GetSectorNumber(); 
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
L3DiskRawSectorListStoreModel::L3DiskRawSectorListStoreModel(wxWindow *parent)
{
	ctrl = (L3DiskRawSector *)parent;
}

bool L3DiskRawSectorListStoreModel::IsEnabledByRow(unsigned int row, unsigned int col) const
{
    return true;
}

int L3DiskRawSectorListStoreModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const
{
	DiskD88Sectors *sectors = ctrl->GetSectors();
	if (!sectors) return 0;

	int idx = -1;
	if (!ctrl->FindColumn(col, &idx)) return 0;

	int cmp = 0;
	int i1 = (int)GetItemData(item1);
	int i2 = (int)GetItemData(item2);
	switch(idx) {
	case SECTORCOL_ID_R:
		cmp = L3DiskRawSectorListCtrl::CompareIDR(sectors, i1, i2, ascending ? 1 : -1);
		break;
	case SECTORCOL_NUM:
		cmp = L3DiskRawSectorListCtrl::CompareNum(sectors, i1, i2, ascending ? 1 : -1);
		break;
	default:
		break;
	}
	return cmp;
}
#endif

//////////////////////////////////////////////////////////////////////
//
// セクタリストコントロール
//
L3DiskRawSectorListCtrl::L3DiskRawSectorListCtrl(L3DiskFrame *parentframe, wxWindow *parent, L3DiskRawSector *sub, wxWindowID id)
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	: L3CDListCtrl(
		parentframe, parent, id,
		gL3DiskRawSectorColumnDefs,
		NULL,
		wxDV_MULTIPLE,
		new L3DiskRawSectorListStoreModel(sub)
	)
#else
	: L3CListCtrl(
		parentframe, parent, id,
		gL3DiskRawSectorColumnDefs,
		-1, -1,
		NULL,
		0
	)
#endif
{
#if 0
	AssignListIcons(icons_for_seclist);
#endif
}

/// リストデータを設定
void L3DiskRawSectorListCtrl::SetListData(DiskD88Sector *sector, int row, L3RawSectorListValue *values)
{
	values[SECTORCOL_NUM].Set(row, wxString::Format(wxT("%d"), row));
	values[SECTORCOL_ID_C].Set(row, wxString::Format(wxT("%d"), (int)sector->GetIDC()));
	values[SECTORCOL_ID_H].Set(row, wxString::Format(wxT("%d"), (int)sector->GetIDH()));
	values[SECTORCOL_ID_R].Set(row, wxString::Format(wxT("%d"), (int)sector->GetIDR()));
	values[SECTORCOL_ID_N].Set(row, wxString::Format(wxT("%d"), (int)sector->GetIDN()));
	values[SECTORCOL_DELETED].Set(row, sector->IsDeleted() ? wxT("*") : wxEmptyString);
	values[SECTORCOL_SINGLE].Set(row, sector->IsSingleDensity() ? wxT("*") : wxEmptyString);
	values[SECTORCOL_SECTORS].Set(row, wxString::Format(wxT("%d"), (int)sector->GetSectorsPerTrack()));
	values[SECTORCOL_SIZE].Set(row, wxString::Format(wxT("%d"), (int)sector->GetSectorBufferSize()));
	values[SECTORCOL_STATUS].Set(row, wxString::Format(wxT("%x"), (int)sector->GetSectorStatus()));
}

/// リストにデータを挿入
void L3DiskRawSectorListCtrl::InsertListData(DiskD88Sector *sector, int row, int idx)
{
	L3RawSectorListValue values[SECTORCOL_END];

	SetListData(sector, row, values);

	InsertListItem(row, values, SECTORCOL_END, (wxUIntPtr)idx);
}

/// リストデータを更新
void L3DiskRawSectorListCtrl::UpdateListData(DiskD88Sector *sector, int row, int idx)
{
	L3RawSectorListValue values[SECTORCOL_END];

	SetListData(sector, row, values);

	UpdateListItem(row, values, SECTORCOL_END, (wxUIntPtr)idx);
}

#ifdef USE_LIST_CTRL_ON_SECTOR_LIST
//
//
//

/// ソート用アイテム
struct st_sector_list_sort_exp {
	DiskD88Sectors *sectors;
	int (*cmpfunc)(DiskD88Sectors *sectors, int i1, int i2, int dir);
	int dir;
};

/// アイテムをソート
void L3DiskRawSectorListCtrl::SortDataItems(DiskD88Track *track, int col)
{
	struct st_sector_list_sort_exp exp;

	// ソート対象か
	int idx;
	bool match_col;
	exp.dir = SelectColumnSortDir(col, idx, match_col);

	DiskD88Sectors *sectors = NULL;
	if (track) {
		sectors = track->GetSectors();
	}

	// 番号かID Rの時のみソート
	if (col >= 0 && match_col) {
		if (sectors) {
			// ソート
			exp.sectors = sectors;
			switch(idx) {
			case SECTORCOL_ID_R:
				exp.cmpfunc = &CompareIDR;
				break;
			case SECTORCOL_NUM:
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

/// ソート用コールバック
int wxCALLBACK L3DiskRawSectorListCtrl::Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata)
{
	struct st_sector_list_sort_exp *exp = (struct st_sector_list_sort_exp *)sortdata;

	int cmp = exp->cmpfunc != NULL ? exp->cmpfunc(exp->sectors, (int)item1, (int)item2, exp->dir) : 0;
	if (cmp == 0) cmp = ((int)item1 - (int)item2);
	return cmp;
}
#endif

/// ID Rでソート
int L3DiskRawSectorListCtrl::CompareIDR(DiskD88Sectors *sectors, int i1, int i2, int dir)
{
	return (sectors->Item(i1)->GetIDR() - sectors->Item(i2)->GetIDR()) * dir;
}
int L3DiskRawSectorListCtrl::CompareNum(DiskD88Sectors *sectors, int i1, int i2, int dir)
{
	return (i1 - i2) * dir;
}

//////////////////////////////////////////////////////////////////////
//
// 右パネルのセクタリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRawSector, L3DiskRawSectorListCtrl)
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, L3DiskRawSector::OnItemContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, L3DiskRawSector::OnItemActivated)
	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, L3DiskRawSector::OnSelectionChanged)
	EVT_DATAVIEW_ITEM_BEGIN_DRAG(wxID_ANY, L3DiskRawSector::OnBeginDrag)
#else
	EVT_CONTEXT_MENU(L3DiskRawSector::OnContextMenu)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, L3DiskRawSector::OnItemActivated)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, L3DiskRawSector::OnSelectionChanged)
	EVT_LIST_COL_CLICK(wxID_ANY, L3DiskRawSector::OnColumnClick)
	EVT_LIST_BEGIN_DRAG(wxID_ANY, L3DiskRawSector::OnBeginDrag)
#endif
	EVT_CHAR(L3DiskRawSector::OnChar)

	EVT_MENU(IDM_EXPORT_FILE, L3DiskRawSector::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, L3DiskRawSector::OnImportFile)

	EVT_MENU(IDM_INVERT_DATA, L3DiskRawSector::OnChangeInvertData)

	EVT_MENU(IDM_MODIFY_ID_C_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_H_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_N_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_DENSITY_TRACK, L3DiskRawSector::OnModifyDensityOnTrack)
	EVT_MENU(IDM_MODIFY_SECTORS_TRACK, L3DiskRawSector::OnModifySectorsOnTrack)
	EVT_MENU(IDM_MODIFY_SIZE_TRACK, L3DiskRawSector::OnModifySectorSizeOnTrack)
	EVT_MENU(IDM_APPEND_SECTOR, L3DiskRawSector::OnAppendSector)
	EVT_MENU(IDM_DELETE_SECTOR, L3DiskRawSector::OnDeleteSector)
	EVT_MENU(IDM_DELETE_SECTORS_BELOW, L3DiskRawSector::OnDeleteSectorsOnTrack)
	EVT_MENU(IDM_EDIT_SECTOR, L3DiskRawSector::OnEditSector)

	EVT_MENU(IDM_PROPERTY_SECTOR, L3DiskRawSector::OnPropertySector)
wxEND_EVENT_TABLE()

L3DiskRawSector::L3DiskRawSector(L3DiskFrame *parentframe, L3DiskRawPanel *parentwindow)
	: L3DiskRawSectorListCtrl(parentframe, parentwindow, this, wxID_ANY)
{
	initialized = false;
	parent   = parentwindow;
	frame    = parentframe;

	track = NULL;

	wxFont font;
	frame->GetDefaultListFont(font);
	SetFont(font);

	// popup menu
	menuPopup = new wxMenu;
	wxMenu *sm = new wxMenu;
		sm->AppendCheckItem(IDM_INVERT_DATA,  _("Invert datas."));
	menuPopup->AppendSubMenu(sm, _("Behavior When In/Out"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_EXPORT_FILE, _("&Export Sector..."));
	menuPopup->Append(IDM_IMPORT_FILE, _("&Import..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MODIFY_ID_C_TRACK, _("Modify All C On This Track"));
	menuPopup->Append(IDM_MODIFY_ID_H_TRACK, _("Modify All H On This Track"));
	menuPopup->Append(IDM_MODIFY_ID_N_TRACK, _("Modify All N On This Track"));
	menuPopup->Append(IDM_MODIFY_DENSITY_TRACK, _("Modify All Density On This Track"));
	menuPopup->Append(IDM_MODIFY_SECTORS_TRACK, _("Modify All Num Of Sectors On This Track"));
	menuPopup->Append(IDM_MODIFY_SIZE_TRACK, _("Modify All Sector Size On This Track"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_APPEND_SECTOR, _("Append New Sector"));
	menuPopup->Append(IDM_DELETE_SECTOR, _("Delete Current Sector"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_DELETE_SECTORS_BELOW, _("Delete Sectors More Than Current Sector Number"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_EDIT_SECTOR, _("Edit Current Sector"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY_SECTOR, _("&Property"));

	initialized = true;
}

L3DiskRawSector::~L3DiskRawSector()
{
	delete menuPopup;
}


/// セクタリスト選択
void L3DiskRawSector::OnSelectionChanged(L3RawSectorListEvent& event)
{
	if (!initialized) return;

	DiskD88Sector *sector = GetSelectedSector();
	if (!sector) {
		// 非選択
		UnselectItem();
		return;
	}

	// 選択
	SelectItem(sector);
}

/// セクタリストからドラッグ開始
void L3DiskRawSector::OnBeginDrag(L3RawSectorListEvent& event)
{
	DragDataSourceForExternal();
}

/// セクタリスト右クリック
void L3DiskRawSector::OnItemContextMenu(L3RawSectorListEvent& event)
{
	ShowPopupMenu();
}

/// 右クリック
void L3DiskRawSector::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// セクタリスト ダブルクリック
void L3DiskRawSector::OnItemActivated(L3RawSectorListEvent& event)
{
	ShowSectorAttr();
}

/// セクタリスト カラムをクリック
void L3DiskRawSector::OnColumnClick(L3RawSectorListEvent& event)
{
	int col = event.GetColumn();
#ifdef USE_LIST_CTRL_ON_SECTOR_LIST
	SortDataItems(track, col);
#endif
}

/// セクタリスト エクスポート選択
void L3DiskRawSector::OnExportFile(wxCommandEvent& event)
{
	ShowExportDataFileDialog();
}

/// セクタリスト インポート選択
void L3DiskRawSector::OnImportFile(wxCommandEvent& event)
{
	ShowImportDataFileDialog();
}

/// データを反転するチェック選択
void L3DiskRawSector::OnChangeInvertData(wxCommandEvent& event)
{
	parent->InvertData(event.IsChecked());
}

/// トラック上のID一括変更選択
void L3DiskRawSector::OnModifyIDonTrack(wxCommandEvent& event)
{
	ModifyIDonTrack(event.GetId() - IDM_MODIFY_ID_C_TRACK);
}

/// トラック上の密度一括変更選択
void L3DiskRawSector::OnModifyDensityOnTrack(wxCommandEvent& event)
{
	ModifyDensityOnTrack();
}

/// トラック上のセクタ数一括変更選択
void L3DiskRawSector::OnModifySectorsOnTrack(wxCommandEvent& event)
{
	ModifySectorsOnTrack();
}

/// トラック上のセクタサイズ一括変更選択
void L3DiskRawSector::OnModifySectorSizeOnTrack(wxCommandEvent& event)
{
	ModifySectorSizeOnTrack();
}

/// セクタ追加選択
void L3DiskRawSector::OnAppendSector(wxCommandEvent& event)
{
	ShowAppendSectorDialog();
}

/// セクタ削除選択
void L3DiskRawSector::OnDeleteSector(wxCommandEvent& event)
{
	DeleteSector();
}

/// トラック上のセクタ一括削除選択
void L3DiskRawSector::OnDeleteSectorsOnTrack(wxCommandEvent& event)
{
	DeleteSectorsOnTrack();
}

/// セクタ編集選択
void L3DiskRawSector::OnEditSector(wxCommandEvent& event)
{
	EditSector();
}

/// セクタプロパティ選択
void L3DiskRawSector::OnPropertySector(wxCommandEvent& event)
{
	ShowSectorAttr();
}

/// セクタリスト上でキー押下
void L3DiskRawSector::OnChar(wxKeyEvent& event)
{
	switch(event.GetKeyCode()) {
	case WXK_RETURN:
		// Enter    ダブルクリックと同じ
		ShowSectorAttr();
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

/// ポップアップメニュー表示
void L3DiskRawSector::ShowPopupMenu()
{
	if (!menuPopup) return;

	menuPopup->Check(IDM_INVERT_DATA, parent->InvertData());

	bool opened = (track != NULL);
	menuPopup->Enable(IDM_IMPORT_FILE, opened);
	menuPopup->Enable(IDM_MODIFY_ID_C_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_H_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_N_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_DENSITY_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_SECTORS_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_SIZE_TRACK, opened);
	menuPopup->Enable(IDM_APPEND_SECTOR, opened);

	int cnt = GetListSelectedItemCount();
	opened = (opened && (cnt > 0));
	menuPopup->Enable(IDM_EXPORT_FILE, opened);
	menuPopup->Enable(IDM_DELETE_SECTOR, opened && (cnt == 1));
	menuPopup->Enable(IDM_DELETE_SECTORS_BELOW, opened && (cnt == 1));
	menuPopup->Enable(IDM_PROPERTY_SECTOR, opened && (cnt == 1));

	PopupMenu(menuPopup);
}

// セクタリスト選択
void L3DiskRawSector::SelectItem(DiskD88Sector *sector)
{
	// ダンプリストをセット
	frame->SetBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize());

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);

}

// セクタリスト非選択
void L3DiskRawSector::UnselectItem()
{
	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// セクタリストにデータをセット
void L3DiskRawSector::SetSectors(DiskD88Track *newtrack)
{
	track = newtrack;

	RefreshSectors();
}

/// セクタリストを返す
DiskD88Sectors *L3DiskRawSector::GetSectors() const
{
	if (track) {
		return track->GetSectors();
	} else {
		return NULL;
	}
}

/// セクタリストをリフレッシュ
void L3DiskRawSector::RefreshSectors()
{
	if (!track) return;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return;

	int row = 0;
	int row_count = (int)GetItemCount();

	for (size_t i=0; i<sectors->Count(); i++) {
		DiskD88Sector *sector = sectors->Item(i);
		if (row < row_count) {
			UpdateListData(sector, row, (int)i);
		} else {
			InsertListData(sector, row, (int)i);
		}
		row++;
	}
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	// 余分な行は消す
	for(int idx = row; idx < row_count; idx++) {
		DeleteItem((unsigned)row);
	}
#else
#ifndef USE_VIRTUAL_ON_LIST_CTRL
	// 余分な行は消す
	for(int idx = row; idx < row_count; idx++) {
		DeleteItem(row);
	}
#else
	SetItemCount(row);
#endif
#endif

#ifdef USE_LIST_CTRL_ON_SECTOR_LIST
#ifdef USE_VIRTUAL_ON_LIST_CTRL
	SetItemCount(row);
#endif
	// ソート
	SortDataItems(track, -1);
#endif

	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// セクタリストをクリア
void L3DiskRawSector::ClearSectors()
{
	DeleteAllListItems();

	track = NULL;

	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// 選択しているセクタを返す
DiskD88Sector *L3DiskRawSector::GetSelectedSector(int *pos)
{
	if (!track) return NULL;

	int idx = GetListSelectedNum();
	if (idx == wxNOT_FOUND) return NULL;

	if (pos) *pos = idx;

	DiskD88Sector *sector = track->GetSectorByIndex(idx);
	return sector;
}

/// セクタを返す
DiskD88Sector *L3DiskRawSector::GetSector(const L3RawSectorListItem &item)
{
	if (!track) return NULL;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return NULL;

#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	int row = ItemToRow(item);
	if (row == wxNOT_FOUND) return NULL;
#else
	int row = (int)GetItemData(item);
#endif

	DiskD88Sector *sector = sectors->Item(row);
	if (!sector) return NULL;

	return sector;
}

/// ドラッグする 外部へドロップ場合
bool L3DiskRawSector::DragDataSourceForExternal()
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
	}
	return sts;
}

// クリップボードへコピー
bool L3DiskRawSector::CopyToClipboard()
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
bool L3DiskRawSector::CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object)
{
	L3RawSectorListItems selected_items;
	int selected_count = GetListSelections(selected_items);
	if (selected_count <= 0) return false;

	L3DiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	if (!app->MakeTempDir(tmp_dir_name)) {
		return false;
	}

	int cnt = 0;
	for(int n = 0; n < selected_count; n++) {
		DiskD88Sector *sector = GetSector(selected_items.Item(n));
		if (!sector) continue;

		wxString filename = parent->MakeFileName(sector);

		// ファイルパスを作成
		wxFileName file_path(tmp_dir_name, filename);

		bool sts = ExportDataFile(file_path.GetFullPath(), sector);
		if (sts) {
			// ファイルリストに追加
			file_object.AddFile(file_path.GetFullPath());
			cnt++;
		}
	}

	return (cnt > 0);
}

// ファイルリストを解放（DnD, クリップボード用）
void L3DiskRawSector::ReleaseFileObject(const wxString &tmp_dir_name)
{
	L3DiskApp *app = &wxGetApp();

	// テンポラリディレクトリを削除
	app->RemoveTempDir(tmp_dir_name);
}

/// クリップボードからペースト
bool L3DiskRawSector::PasteFromClipboard()
{
	return parent->PasteFromClipboard();
}

/// エクスポートダイアログ表示
bool L3DiskRawSector::ShowExportDataFileDialog()
{
	L3RawSectorListItems selected_items;
	int selected_count = GetListSelections(selected_items);

	bool sts = true;
	if (selected_count == 1) {
		// 単一行 指定
		DiskD88Sector *sector = GetSector(selected_items.Item(0));
		if (!sector) return false;

		wxString filename = parent->MakeFileName(sector);

		L3DiskFileDialog dlg(
			_("Export data from sector"),
			frame->GetIniExportFilePath(),
			filename,
			_("All files (*.*)|*.*"),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		int rc = dlg.ShowModal();
		wxString path = dlg.GetPath();

		if (rc == wxID_OK) {
			sts = ExportDataFile(path, sector);
		}

	} else {
		// 複数行 指定
		L3DiskDirDialog dlg(
			_("Export each datas from selected sector"),
			frame->GetIniExportFilePath());

		int rc = dlg.ShowModal();
		if (rc != wxID_OK) {
			return false;
		}

		wxString dir_path = dlg.GetPath();
		for(int n = 0; n < selected_count; n++) {
			DiskD88Sector *sector = GetSector(selected_items.Item(n));
			if (!sector) continue;

			// ファイルパスを作成
			wxFileName file_path(dir_path, parent->MakeFileName(sector));

			sts &= ExportDataFile(file_path.GetFullPath(), sector);
		}
	}

	return sts;
}

/// 指定したファイルにセクタのデータをエクスポート
bool L3DiskRawSector::ExportDataFile(const wxString &path, DiskD88Sector *sector)
{
	// エクスポート元パスを覚えておく
	frame->SetIniExportFilePath(path);

	if (!sector) return false;

	size_t bufsize = sector->GetSectorBufferSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	if (buf == NULL || bufsize <= 0) return false;

	wxFile outfile(path, wxFile::write);
	if (!outfile.IsOpened()) return false;

	Utils::TempData tbuf;
	tbuf.SetData(buf, bufsize, parent->InvertData());
	outfile.Write(tbuf.GetData(), tbuf.GetSize());

	return true;
}

/// インポートダイアログ表示
bool L3DiskRawSector::ShowImportDataFileDialog()
{
	L3DiskFileDialog dlg(
		_("Import data to sector"),
		frame->GetIniExportFilePath(),
		wxEmptyString,
		_("All files (*.*)|*.*"),
		wxFD_OPEN);

	int dlgsts = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (dlgsts != wxID_OK) {
		return false;
	}

	DiskD88Sector *sector = GetSelectedSector();
	int st_trk = (track ? track->GetTrackNumber() : -1);
	int st_sid = (track ? track->GetSideNumber() : 0);
	int st_sec = (sector ? sector->GetSectorNumber() : 1);

	return parent->ShowImportTrackRangeDialog(path, st_trk, st_sid, st_sec);
}

/// セクタ情報プロパティダイアログ表示
bool L3DiskRawSector::ShowSectorAttr()
{
	DiskD88Sector *sector = GetSelectedSector();
	if (!sector) return false;

	RawSectorBox dlg(this, wxID_ANY, _("Sector Information")
		, sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetIDN()
		, sector->GetSectorsPerTrack()
		, sector->IsDeleted(), sector->IsSingleDensity(), sector->GetSectorStatus()
	);

	int rc = dlg.ShowModal();
	if (rc != wxID_OK) return false;

	rc = wxNO;
	int new_size = DiskD88Sector::ConvIDNToSecSize(dlg.GetIdN());
	if (sector->GetSectorBufferSize() < new_size) {
		wxString msg = wxString::Format(_("Need expand the buffer size to %d bytes. Are you sure to do it?"), new_size);
		rc = wxMessageBox(msg, _("Expand Sector Size"), wxICON_WARNING | wxYES_NO);
		if (rc == wxYES) {
			// 大きくしたときバッファサイズも大きくする
			sector->ModifySectorSize(new_size);

			// トラックのサイズを再計算&オフセットを再計算する
			track->ShrinkAndCalcOffsets(false);
		} else {
			return false;
		}
	}

	sector->SetIDC((wxUint8)dlg.GetIdC());
	sector->SetIDH((wxUint8)dlg.GetIdH());
	sector->SetIDR((wxUint8)dlg.GetIdR());
	sector->SetIDN((wxUint8)dlg.GetIdN());
	sector->SetSectorsPerTrack((wxUint16)dlg.GetSectorNums());
	sector->SetDeletedMark(dlg.GetDeletedMark());
	sector->SetSingleDensity(dlg.GetSingleDensity());
	sector->SetSectorStatus(dlg.GetStatus());

	if (rc == wxYES) {
		parent->RefreshAllData();
	} else {
		RefreshSectors();
	}

	return true;
}

/// トラック上のIDを一括変更
/// @param [in] type_num  RawParamBox::TYPE_IDC / TYPE_IDH / TYPE_IDN / TYPE_NUM_OF_SECTORS
void L3DiskRawSector::ModifyIDonTrack(int type_num)
{
	if (!track) return;

	DiskD88Sector *sector = track->GetSectorByIndex(0);
	if (!sector) return;

	int value = 0;
	int maxvalue = 82;
	switch(type_num) {
		case RawParamBox::TYPE_IDC:
			value = sector->GetIDC();
			break;
		case RawParamBox::TYPE_IDH:
			value = sector->GetIDH();
			maxvalue = 1;
			break;
		case RawParamBox::TYPE_IDN:
			value = sector->GetIDN();
			maxvalue = DiskD88Sector::ConvSecSizeToIDN(sector->GetSectorBufferSize());
			break;
		case RawParamBox::TYPE_NUM_OF_SECTORS:
			value = sector->GetSectorsPerTrack();
			break;
	}

	RawParamBox dlg(this, wxID_ANY, type_num, value, maxvalue);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		int newvalue = dlg.GetValue();
		if (value != newvalue) {
			
			switch(type_num) {
				case RawParamBox::TYPE_IDC:
					track->SetAllIDC(newvalue);
					break;
				case RawParamBox::TYPE_IDH:
					track->SetAllIDH(newvalue);
					break;
				case RawParamBox::TYPE_IDN:
					track->SetAllIDN(newvalue);
					break;
				case RawParamBox::TYPE_NUM_OF_SECTORS:
					track->SetAllSectorsPerTrack(newvalue);
					break;
			}

			// リストを更新
			RefreshSectors();
		}
	}
}

/// トラック上の密度を一括変更
void L3DiskRawSector::ModifyDensityOnTrack()
{
	if (!track) return;

	DiskD88Sector *sector = track->GetSectorByIndex(0);
	if (!sector) return;

	bool sdensity = sector->IsSingleDensity();

	DensityParamBox dlg(this, wxID_ANY, sdensity);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		bool newsdensity = dlg.IsSingleDensity();
		if (sdensity != newsdensity) {

			track->SetAllSingleDensity(newsdensity);

			// リストを更新
			RefreshSectors();
		}
	}
}

/// トラック上のセクタ数を一括変更
void L3DiskRawSector::ModifySectorsOnTrack()
{
	ModifyIDonTrack(4);
}

/// トラック上のセクタサイズを一括変更
void L3DiskRawSector::ModifySectorSizeOnTrack()
{
	if (!track) return;

	DiskD88Sector *sector = track->GetSectorByIndex(0);
	if (!sector) return;

	int value = sector->GetSectorSize();
	int maxvalue = 2048;

	RawParamBox dlg(this, wxID_ANY, RawParamBox::TYPE_SECTOR_SIZE, value, maxvalue);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		int newvalue = dlg.GetValue();
		track->SetAllSectorSize(newvalue);

		// 画面更新
		frame->UpdateDataOnWindow(true);
	}
}

/// セクタを追加ダイアログを表示
void L3DiskRawSector::ShowAppendSectorDialog()
{
	if (!track) return;

	DiskD88Sector *sector = track->GetSectorByIndex(0);
	if (!sector) return;

	int new_sec_num = track->GetMaxSectorNumber() + 1;

	RawSectorBox dlg(this, wxID_ANY, _("Add Sector")
		, sector->GetIDC(), sector->GetIDH(), new_sec_num, sector->GetIDN()
		, 1
		, sector->IsDeleted(), sector->IsSingleDensity(), sector->GetSectorStatus()
		, SECTORBOX_HIDE_SECTOR_NUMS
	);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		track->AddNewSector(
			dlg.GetIdC(),
			dlg.GetIdH(),
			dlg.GetIdR(),
			DiskD88Sector::ConvIDNToSecSize(dlg.GetIdN()),
			sector->IsSingleDensity(),
			dlg.GetStatus()
		);

		// 画面更新
		parent->RefreshAllData();
	}
}

/// セクタを削除
void L3DiskRawSector::DeleteSector()
{
	int pos = 0;
	DiskD88Sector *sector = GetSelectedSector(&pos);
	if (!sector) return;

	int ans = wxYES;
	wxString msg = wxString::Format(_("Do you really want to delete current sector?"));
	ans = wxMessageBox(msg, _("Delete Sector"), wxYES_NO);
	if (ans == wxYES) {
		track->DeleteSectorByIndex(pos);

		// 画面更新
		parent->RefreshAllData();
	}
}

/// セクタを編集
void L3DiskRawSector::EditSector()
{
	int pos = 0;
	DiskD88Sector *sector = GetSelectedSector(&pos);
	if (!sector) return;

	wxString binary_editer = gConfig.GetBinaryEditer();
	if (binary_editer.IsEmpty()) return;

	size_t bufsize = sector->GetSectorBufferSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	if (buf == NULL || bufsize <= 0) return;

	// データを反転して出力するか
	bool inverted = false;
//	int sts = wxMessageBox(_("Do you want to invert data?"), _("Output Sector Data"), wxYES | wxNO | wxNO_DEFAULT);
//	if (sts == wxYES) {
//		inverted = true;
//	}
	inverted = parent->InvertData();

	L3DiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	wxString tmp_dir_name;
	if (!app->MakeTempDir(tmp_dir_name)) {
		return;
	}

	// セクタデータを出力
	int trk = sector->GetIDC();
	int sid = sector->GetIDH();
	int sec = sector->GetIDR();
	wxString tmp_file_name;
	tmp_file_name = wxString::Format(wxT("sector_%d_%d_%d.dat"), trk, sid, sec);

	wxFileName tmp_path(tmp_dir_name, tmp_file_name);

	wxFile outfile(tmp_path.GetFullPath(), wxFile::write);
	if (!outfile.IsOpened()) return;
	if (inverted) mem_invert(buf, bufsize);
	outfile.Write((const void *)buf, bufsize);
	outfile.Close();
	if (inverted) mem_invert(buf, bufsize);

	// エディタを起動
	binary_editer += wxT(" \"");
	binary_editer += tmp_path.GetFullPath();
	binary_editer += wxT("\"");

	wxProcess *process = NULL;
	long psts = wxExecute(binary_editer, wxEXEC_SYNC, process);
	// エディタ終了
	if (psts < 0) {
		// コマンド起動失敗
		return;
	}

	// ファイルを読み込む
	wxFile infile(tmp_path.GetFullPath(), wxFile::read);
	if (!infile.IsOpened()) return;
	if (inverted) mem_invert(buf, bufsize);
	infile.Read((void *)buf, bufsize);
	infile.Close();
	if (inverted) mem_invert(buf, bufsize);

//	wxRemove(tmp_path.GetFullPath());
}

/// トラック上のセクタを一括削除
void L3DiskRawSector::DeleteSectorsOnTrack()
{
	DiskD88Sector *sector = GetSelectedSector();
	if (!sector) return;

	int ans = wxYES;
	wxString msg = wxString::Format(_("Do you really want to delete sectors?"));
	ans = wxMessageBox(msg, _("Delete Sectors"), wxYES_NO);
	if (ans == wxYES) {
		track->DeleteSectors(sector->GetSectorNumber(), -1);

		// 画面更新
		parent->RefreshAllData();
	}
}
