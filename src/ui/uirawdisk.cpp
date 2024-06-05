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
#include "mymenu.h"
#include "../main.h"
#include "uimainframe.h"
#include "rawtrackbox.h"
#include "rawsectorbox.h"
#include "rawparambox.h"
#include "rawexpbox.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////

const struct st_list_columns gUiDiskRawTrackColumnDefs[] = {
	{ "Num",		wxTRANSLATE("Num"),				false,	42,	wxALIGN_RIGHT,	true },
	{ "Track",		wxTRANSLATE("Track"),			false,	32,	wxALIGN_RIGHT,	false },
	{ "Side",		wxTRANSLATE("Side"),			false,	32,	wxALIGN_RIGHT,	false },
	{ "Sectors",	wxTRANSLATE("NumOfSectors"),	false,	40,	wxALIGN_RIGHT,	false },
	{ "Offset",		wxTRANSLATE("Offset"),			false,	60,	wxALIGN_RIGHT,	true },
	{ NULL,			NULL,							false,	 0,	wxALIGN_LEFT,	false }
};

const struct st_list_columns gUiDiskRawSectorColumnDefs[] = {
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
wxBEGIN_EVENT_TABLE(UiDiskRawPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

UiDiskRawPanel::UiDiskRawPanel(UiDiskFrame *parentframe, wxWindow *parentwindow)
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
	lpanel = new UiDiskRawTrack(frame, this);
	rpanel = new UiDiskRawSector(frame, this);
	SplitVertically(lpanel, rpanel, 236);

	SetMinimumPaneSize(10);
}

/// トラックリストにデータを設定する
void UiDiskRawPanel::SetTrackListData(DiskImageDisk *disk, int side_num)
{
	lpanel->SetTracks(disk, side_num);
	frame->UpdateMenuAndToolBarRawDisk(this);
}

/// トラックリストをクリアする
void UiDiskRawPanel::ClearTrackListData()
{
	lpanel->ClearTracks();
	frame->UpdateMenuAndToolBarRawDisk(this);
}

/// トラックリストを再描画する
void UiDiskRawPanel::RefreshTrackListData()
{
	lpanel->RefreshTracks();
}

/// トラックリストが存在するか
bool UiDiskRawPanel::TrackListExists() const
{
	return (lpanel->GetDisk() != NULL);
}

/// トラックリストの選択行を返す
int UiDiskRawPanel::GetTrackListSelectedRow() const
{
	return (int)lpanel->GetListSelectedRow();
}

/// セクタリストにデータを設定する
void UiDiskRawPanel::SetSectorListData(DiskImageTrack *track)
{
	rpanel->SetSectors(track);
	frame->UpdateMenuAndToolBarRawDisk(this);
}

/// セクタリストをクリアする
void UiDiskRawPanel::ClearSectorListData()
{
	rpanel->ClearSectors();
	frame->UpdateMenuAndToolBarRawDisk(this);
}

/// セクタリストの選択行を返す
int UiDiskRawPanel::GetSectorListSelectedRow() const
{
	return rpanel->GetListSelectedRow();
}

/// トラックリストとセクタリストを更新
void UiDiskRawPanel::RefreshAllData()
{
	lpanel->RefreshTracks();
	rpanel->RefreshSectors();
}

/// クリップボードヘコピー
bool UiDiskRawPanel::CopyToClipboard()
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
bool UiDiskRawPanel::PasteFromClipboard()
{
	// トラック側
	return lpanel->PasteFromClipboard();
}

/// エクスポートダイアログ表示
bool UiDiskRawPanel::ShowExportDataDialog()
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
bool UiDiskRawPanel::ShowImportDataDialog()
{
	// トラック側のダイアログを表示
	return lpanel->ShowImportTrackDialog();
}

/// データを削除ダイアログ表示
bool UiDiskRawPanel::ShowDeleteDataDialog()
{
	wxWindow *fwin = wxWindow::FindFocus();
	if (fwin == lpanel) {
	} else if (fwin == rpanel) {
		rpanel->DeleteSector();
	}
	return true;
}

/// トラックへインポートダイアログ（トラックの範囲指定）表示
bool UiDiskRawPanel::ShowImportTrackRangeDialog(const wxString &path, int st_trk, int st_sid, int st_sec)
{
	return lpanel->ShowImportTrackRangeDialog(path, st_trk, st_sid, st_sec);
}

/// セクタからエクスポートダイアログ表示
bool UiDiskRawPanel::ShowExportDataFileDialog()
{
	return rpanel->ShowExportDataFileDialog();
}

/// セクタへインポートダイアログ表示
bool UiDiskRawPanel::ShowImportDataFileDialog()
{
	return rpanel->ShowImportDataFileDialog();
}

/// トラックのID一括変更
void UiDiskRawPanel::ModifyIDonTrack(int type_num)
{
	rpanel->ModifyIDonTrack(type_num);
}

/// トラックの密度一括変更
void UiDiskRawPanel::ModifyDensityOnTrack()
{
	rpanel->ModifyDensityOnTrack();
}

/// トラックのセクタ数一括変更
void UiDiskRawPanel::ModifySectorsOnTrack()
{
	rpanel->ModifySectorsOnTrack();
}

/// トラックのセクタサイズ一括変更
void UiDiskRawPanel::ModifySectorSizeOnTrack()
{
	rpanel->ModifySectorSizeOnTrack();
}

/// トラック or セクタのプロパティダイアログ表示
bool UiDiskRawPanel::ShowRawDiskAttr()
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
bool UiDiskRawPanel::ShowSectorAttr()
{
	return rpanel->ShowSectorAttr();
}

/// セクタを編集
void UiDiskRawPanel::EditSector()
{
	return rpanel->EditSector();
}

/// ファイル名を生成
/// @param[in] sector セクタ
/// @return ファイル名
wxString UiDiskRawPanel::MakeFileName(DiskImageSector *sector)
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
wxString UiDiskRawPanel::MakeFileName(int st_c, int st_h, int st_r, int ed_c, int ed_h, int ed_r)
{
	return wxString::Format(wxT("%02d-%02d-%02d--%02d-%02d-%02d.bin"),
		st_c, st_h, st_r, ed_c, ed_h, ed_r
	);
}

/// フォントをセット
/// @param[in] font フォントデータ
void UiDiskRawPanel::SetListFont(const wxFont &font)
{
	lpanel->SetFont(font);
	lpanel->Refresh();
	rpanel->SetFont(font);
	rpanel->Refresh();
}

/// 次のサイドへ
void UiDiskRawPanel::IncreaseSide()
{
	lpanel->IncreaseSide();
}

/// 前のサイドへ
void UiDiskRawPanel::DecreaseSide()
{
	lpanel->DecreaseSide();
}

//////////////////////////////////////////////////////////////////////
//
//
//
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
UiDiskRawTrackListStoreModel::UiDiskRawTrackListStoreModel(wxWindow *parent)
	: wxDataViewListStore()
{
	ctrl = (UiDiskRawTrack *)parent;
}

/// トラックリストのソート
int UiDiskRawTrackListStoreModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const
{
	DiskImageDisk *disk = ctrl->GetDisk();
	if (!disk) return 0;

	int idx = -1;
	if (!ctrl->FindColumn(col, &idx)) return 0;

	int cmp = 0;
	int i1 = (int)GetItemData(item1);
	int i2 = (int)GetItemData(item2);
	switch(idx) {
	case TRACKCOL_OFFSET:
		cmp = UiDiskRawTrackListCtrl::CompareOffset(disk, i1, i2, ascending ? 1 : -1);
		break;
	case TRACKCOL_NUM:
		cmp = UiDiskRawTrackListCtrl::CompareNum(disk, i1, i2, ascending ? 1 : -1);
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
UiDiskRawTrackListCtrl::UiDiskRawTrackListCtrl(UiDiskFrame *parentframe, wxWindow *parent, UiDiskRawTrack *sub, wxWindowID id)
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
	: MyCDListCtrl(
		parentframe, parent, id,
		gUiDiskRawTrackColumnDefs,
		NULL,
		wxDV_SINGLE,
		new UiDiskRawTrackListStoreModel(sub)
	)
#else
	: MyCListCtrl(
		parentframe, parent, id,
		gUiDiskRawTrackColumnDefs,
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
void UiDiskRawTrackListCtrl::SetListData(DiskImageDisk *disk, int pos, int row, int idx, MyRawTrackListValue *values)
{
	wxUint32 offset = disk->GetOffset(pos);
	DiskImageTrack *trk = NULL;
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
void UiDiskRawTrackListCtrl::InsertListData(DiskImageDisk *disk, int pos, int row, int idx)
{
	MyRawTrackListValue values[TRACKCOL_END];

	SetListData(disk, pos, row, idx, values);

	InsertListItem(row, values, TRACKCOL_END, (wxUIntPtr)idx);
}

/// リストデータを更新
/// @param[in] disk ディスク
/// @param[in] pos  ディスク内のトラック位置(0～)
/// @param[in] row  リストの行位置
/// @param[in] idx  リスト内の通し番号
void UiDiskRawTrackListCtrl::UpdateListData(DiskImageDisk *disk, int pos, int row, int idx)
{
	MyRawTrackListValue values[TRACKCOL_END];

	SetListData(disk, pos, row, idx, values);

	UpdateListItem(row, values, TRACKCOL_END, (wxUIntPtr)idx);
}

#ifdef USE_LIST_CTRL_ON_TRACK_LIST
//
//
//

/// ソート用アイテム
struct st_track_list_sort_exp {
	DiskImageDisk *disk;
	int (*cmpfunc)(DiskImageDisk *disk, int i1, int i2, int dir);
	int dir;
};

/// アイテムをソート
/// @param[in] disk        ディスク
/// @param[in] side_number サイド番号
/// @param[in] col         リストの列位置
void UiDiskRawTrackListCtrl::SortDataItems(DiskImageDisk *disk, int side_number, int col)
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
int wxCALLBACK UiDiskRawTrackListCtrl::Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata)
{
	struct st_track_list_sort_exp *exp = (struct st_track_list_sort_exp *)sortdata;

	int cmp = exp->cmpfunc != NULL ? exp->cmpfunc(exp->disk, (int)item1, (int)item2, exp->dir) : 0;
	if (cmp == 0) cmp = ((int)item1 - (int)item2);
	return cmp;
}
#endif

int UiDiskRawTrackListCtrl::CompareOffset(DiskImageDisk *disk, int i1, int i2, int dir)
{
	return (disk->GetOffset(i1) - disk->GetOffset(i2)) * dir;
}
int UiDiskRawTrackListCtrl::CompareNum(DiskImageDisk *disk, int i1, int i2, int dir)
{
	return (i1 - i2) * dir;
}

//////////////////////////////////////////////////////////////////////
//
// 左パネルのトラックリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskRawTrack, UiDiskRawTrackListCtrl)
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, UiDiskRawTrack::OnListActivated)
	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, UiDiskRawTrack::OnListItemSelected)
	EVT_DATAVIEW_ITEM_BEGIN_DRAG(wxID_ANY, UiDiskRawTrack::OnBeginDrag)

	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, UiDiskRawTrack::OnListContextMenu)
#else
	EVT_LIST_ITEM_SELECTED(wxID_ANY, UiDiskRawTrack::OnListItemSelected)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, UiDiskRawTrack::OnListActivated)
	EVT_LIST_COL_CLICK(wxID_ANY, UiDiskRawTrack::OnColumnClick)

	EVT_LIST_BEGIN_DRAG(wxID_ANY, UiDiskRawTrack::OnBeginDrag)

	EVT_CONTEXT_MENU(UiDiskRawTrack::OnContextMenu)
#endif
	EVT_CHAR(UiDiskRawTrack::OnChar)

	EVT_MENU(IDM_EXPORT_TRACK, UiDiskRawTrack::OnExportTrack)
	EVT_MENU(IDM_IMPORT_TRACK, UiDiskRawTrack::OnImportTrack)

	EVT_MENU(IDM_INVERT_DATA, UiDiskRawTrack::OnChangeInvertData)
	EVT_MENU(IDM_REVERSE_SIDE, UiDiskRawTrack::OnChangeReverseSide)

	EVT_MENU(IDM_MODIFY_ID_H_DISK, UiDiskRawTrack::OnModifyIDonDisk)
	EVT_MENU(IDM_MODIFY_ID_N_DISK, UiDiskRawTrack::OnModifyIDonDisk)
	EVT_MENU(IDM_MODIFY_DENSITY_DISK, UiDiskRawTrack::OnModifyDensityOnDisk)

	EVT_MENU(IDM_MODIFY_ID_H_EVEN_TRACKS, UiDiskRawTrack::OnModifyIDonEvenTracks)
	EVT_MENU(IDM_MODIFY_ID_H_ODD_TRACKS, UiDiskRawTrack::OnModifyIDonOddTracks)

	EVT_MENU(IDM_MODIFY_ID_C_TRACK, UiDiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_H_TRACK, UiDiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_N_TRACK, UiDiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_DENSITY_TRACK, UiDiskRawTrack::OnModifyDensityOnTrack)
	EVT_MENU(IDM_MODIFY_SECTORS_TRACK, UiDiskRawTrack::OnModifySectorsOnTrack)
	EVT_MENU(IDM_MODIFY_SIZE_TRACK, UiDiskRawTrack::OnModifySectorSizeOnTrack)

	EVT_MENU(IDM_APPEND_TRACK, UiDiskRawTrack::OnAppendTrack)
	EVT_MENU(IDM_DELETE_TRACKS_BELOW, UiDiskRawTrack::OnDeleteTracksBelow)

	EVT_MENU(IDM_PROPERTY_TRACK, UiDiskRawTrack::OnPropertyTrack)
wxEND_EVENT_TABLE()

UiDiskRawTrack::UiDiskRawTrack(UiDiskFrame *parentframe, UiDiskRawPanel *parentwindow)
       : UiDiskRawTrackListCtrl(parentframe, parentwindow, this, wxID_ANY)
{
	parent   = parentwindow;
	frame    = parentframe;

	p_disk = NULL;
	m_side_number = -1;

	wxFont font;
	frame->GetDefaultListFont(font);
	SetFont(font);

	// popup menu
	MakePopupMenu();
}

UiDiskRawTrack::~UiDiskRawTrack()
{
	delete menuPopup;
}

/// トラックリストを選択
void UiDiskRawTrack::OnListItemSelected(MyRawTrackListEvent& event)
{
	if (!p_disk) return;

	SelectData();
}

/// トラックリストをダブルクリック
void UiDiskRawTrack::OnListActivated(MyRawTrackListEvent &event)
{
	ShowTrackAttr();
}

/// トラックリスト右クリック
void UiDiskRawTrack::OnListContextMenu(MyRawTrackListEvent& event)
{
	ShowPopupMenu();
}

/// トラックリスト右クリック
void UiDiskRawTrack::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// カラムをクリック
void UiDiskRawTrack::OnColumnClick(MyRawTrackListEvent &event)
{
	int col = event.GetColumn();
#ifdef USE_LIST_CTRL_ON_TRACK_LIST
	SortDataItems(p_disk, m_side_number, col);
#endif
}

/// トラックをエクスポート選択
void UiDiskRawTrack::OnExportTrack(wxCommandEvent& event)
{
	ShowExportTrackDialog();
}

/// トラックにインポート選択
void UiDiskRawTrack::OnImportTrack(wxCommandEvent& event)
{
	ShowImportTrackDialog();
}

/// データを反転するチェック選択
void UiDiskRawTrack::OnChangeInvertData(wxCommandEvent& event)
{
	parent->InvertData(event.IsChecked());
}

/// サイドを逆転するチェック選択
void UiDiskRawTrack::OnChangeReverseSide(wxCommandEvent& event)
{
	parent->ReverseSide(event.IsChecked());
}

/// セクタリストからドラッグ開始
void UiDiskRawTrack::OnBeginDrag(MyRawTrackListEvent& event)
{
	DragDataSourceForExternal();
}

/// ディスク上のID一括変更選択
void UiDiskRawTrack::OnModifyIDonDisk(wxCommandEvent& event)
{
	ModifyIDonDisk(event.GetId() - IDM_MODIFY_ID_C_DISK, -1);
}

/// 偶数トラックのID一括変更選択
void UiDiskRawTrack::OnModifyIDonEvenTracks(wxCommandEvent& event)
{
	ModifyIDonDisk(event.GetId() - IDM_MODIFY_ID_C_EVEN_TRACKS, 0);
}

/// 奇数トラックのID一括変更選択
void UiDiskRawTrack::OnModifyIDonOddTracks(wxCommandEvent& event)
{
	ModifyIDonDisk(event.GetId() - IDM_MODIFY_ID_C_ODD_TRACKS, 1);
}

/// トラックのID一括変更選択
void UiDiskRawTrack::OnModifyIDonTrack(wxCommandEvent& event)
{
	parent->ModifyIDonTrack(event.GetId() - IDM_MODIFY_ID_C_TRACK);
}

/// ディスク上の密度一括変更選択
void UiDiskRawTrack::OnModifyDensityOnDisk(wxCommandEvent& event)
{
	ModifyDensityOnDisk();
}

/// トラックの密度一括変更選択
void UiDiskRawTrack::OnModifyDensityOnTrack(wxCommandEvent& event)
{
	parent->ModifyDensityOnTrack();
}

/// トラックのセクタ数を一括変更選択
void UiDiskRawTrack::OnModifySectorsOnTrack(wxCommandEvent& event)
{
	parent->ModifySectorsOnTrack();
}

/// トラックのセクタサイズを一括変更
void UiDiskRawTrack::OnModifySectorSizeOnTrack(wxCommandEvent& event)
{
	parent->ModifySectorSizeOnTrack();
}

/// 新規トラックを追加選択
void UiDiskRawTrack::OnAppendTrack(wxCommandEvent& event)
{
	AppendTrack();
}

/// 現在のトラック以下を削除選択
void UiDiskRawTrack::OnDeleteTracksBelow(wxCommandEvent& event)
{
	DeleteTracks();
}

/// トラックプロパティ選択
void UiDiskRawTrack::OnPropertyTrack(wxCommandEvent& event)
{
	ShowTrackAttr();
}

/// キー押下
void UiDiskRawTrack::OnChar(wxKeyEvent& event)
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
void UiDiskRawTrack::SelectData()
{
	int num = GetListSelectedNum();
	wxUint32 offset = p_disk->GetOffset(num);
	DiskImageTrack *trk = NULL;
	if (offset > 0) {
		trk = p_disk->GetTrackByOffset(offset);
	} else {
		trk = p_disk->GetTrack(num);
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
void UiDiskRawTrack::SetTracks(DiskImageDisk *newdisk, int newsidenum)
{
	if (!newdisk) return;

	p_disk = newdisk;
	m_side_number = newsidenum;

	SetTracks();

	// セクタリストはクリア
	UnselectListItem(GetListSelection());
	parent->ClearSectorListData();
}

/// トラックリストを再セット
void UiDiskRawTrack::RefreshTracks()
{
	MyRawTrackListItem sel_pos = GetListSelection();
	MyRawTrackListItem foc_pos = GetListFocusedItem();
	SetTracks();
	if (foc_pos >= 0) FocusListItem(foc_pos);
	if (sel_pos >= 0) SelectListItem(sel_pos);
}

/// トラックリストを再セット
void UiDiskRawTrack::SetTracks()
{
	if (!p_disk) return;

	int sides = p_disk->GetSidesPerDisk();
	int row = 0;
	int row_count = (int)GetItemCount();

	int max_pos = p_disk->GetTracksPerSide() * sides;
	int limit_pos = p_disk->GetCreatableTracks();
	if (max_pos < limit_pos) max_pos = limit_pos;
	
	for(int pos = (m_side_number >= 0 ? m_side_number : 0); pos < max_pos; pos+=(m_side_number >= 0 ? sides : 1)) {
		if (row < row_count) {
			UpdateListData(p_disk, pos, row, pos);
		} else {
			InsertListData(p_disk, pos, row, pos);
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
	SortDataItems(p_disk, m_side_number, -1);
#endif
}

/// トラックリストをクリア
void UiDiskRawTrack::ClearTracks()
{
	DeleteAllListItems();

	p_disk = NULL;
	m_side_number = -1;

	// セクタリストクリア
	parent->ClearSectorListData();
}

/// トラックリスト上のポップアップメニュー作成
void UiDiskRawTrack::MakePopupMenu()
{
	menuPopup = new MyMenu;
	MyMenu *sm = new MyMenu;
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
	menuPopup->Append(IDM_MODIFY_ID_H_EVEN_TRACKS, _("Modify All H On Even Tracks"));
	menuPopup->Append(IDM_MODIFY_ID_H_ODD_TRACKS, _("Modify All H On Odd Tracks"));
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

/// トラックリスト上のポップアップメニュー表示
void UiDiskRawTrack::ShowPopupMenu()
{
	if (!menuPopup) return;
	int num = 0;
	wxUint32 offset = 0;

	menuPopup->Check(IDM_INVERT_DATA, parent->InvertData());
	menuPopup->Check(IDM_REVERSE_SIDE, parent->ReverseSide());

	bool opened = (p_disk != NULL);
	menuPopup->Enable(IDM_EXPORT_TRACK, opened);
	menuPopup->Enable(IDM_IMPORT_TRACK, opened);

	menuPopup->Enable(IDM_MODIFY_ID_H_DISK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_N_DISK, opened);
	menuPopup->Enable(IDM_MODIFY_DENSITY_DISK, opened);

	menuPopup->Enable(IDM_MODIFY_ID_H_EVEN_TRACKS, opened);
	menuPopup->Enable(IDM_MODIFY_ID_H_ODD_TRACKS, opened);

	menuPopup->Enable(IDM_APPEND_TRACK, opened);

	menuPopup->Enable(IDM_PROPERTY_TRACK, opened);

	if (opened) {
		num = GetListSelectedNum();
		offset = p_disk->GetOffset(num);
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
bool UiDiskRawTrack::DragDataSourceForExternal()
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
bool UiDiskRawTrack::CopyToClipboard()
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
bool UiDiskRawTrack::CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object)
{
	int selected_count = GetListSelectedItemCount();
	if (selected_count <= 0) return false;

	UiDiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	if (!app->MakeTempDir(tmp_dir_name)) {
		return false;
	}

	int cnt = 0;
	MyRawTrackListItems sels;
	GetListSelections(sels);
	for(size_t row = 0; row < sels.Count(); row++) {
		DiskImageTrack *track = GetTrack(sels[row]);
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
void UiDiskRawTrack::ReleaseFileObject(const wxString &tmp_dir_name)
{
	UiDiskApp *app = &wxGetApp();

	// テンポラリディレクトリを削除
	app->RemoveTempDir(tmp_dir_name);
}

/// クリップボードからペースト
bool UiDiskRawTrack::PasteFromClipboard()
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
bool UiDiskRawTrack::ShowExportTrackDialog()
{
	if (!p_disk) return false;

	DiskImageTrack *track = GetSelectedTrack();
//	DiskImageSector *sector = NULL;
//	if (!GetFirstSectorOnTrack(&track, &sector)) {
//		return false;
//	}
	int st_sec, ed_sec;
	if (!GetFirstAndLastSectorNumOnTrack(track, st_sec, ed_sec)) {
		return false;
	}

	wxString caption = _("Export data from track");

	RawExpBox dlg(this, wxID_ANY, caption, p_disk, m_side_number
		, track->GetTrackNumber(), track->GetSideNumber(), st_sec
		, -1, -1, ed_sec
		, parent->InvertData(), parent->ReverseSide()
	);
	int sts = dlg.ShowModal();
	if (sts != wxID_OK) return false;

	wxString filename = parent->MakeFileName(
		dlg.GetTrackNumber(0), dlg.GetSideNumber(0), dlg.GetSectorNumber(0),
		dlg.GetTrackNumber(1), dlg.GetSideNumber(1), dlg.GetSectorNumber(1));

	UiDiskSaveFileDialog fdlg(
		caption,
		frame->GetIniExportFilePath(),
		filename,
		_("All files (*.*)|*.*"));

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
bool UiDiskRawTrack::ExportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec, bool inv_data, bool rev_side)
{
	// エクスポート元パスを覚えておく
	frame->SetIniExportFilePath(path);

	if (!p_disk) return false;

	int trk = st_trk;
	int sid = st_sid;
	int sec = st_sec;
	int ssid = m_side_number >= 0 ? m_side_number : 0;
	int esid = m_side_number >= 0 ? m_side_number : (p_disk->GetSidesPerDisk() - 1);
	int esec = p_disk->GetSectorsPerTrack();

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
			DiskImageTrack *track = p_disk->GetTrack(trk, sid);
			if (track) {
				if (trk == ed_trk && sid == ed_sid) {
					// 最終トラック＆最終サイドなら終了セクタまで
					esec = ed_sec;
				}
				for(; sec <= esec && !eof; sec++) {
					DiskImageSector *sector = track->GetSector(sec);
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
bool UiDiskRawTrack::ShowImportTrackDialog()
{
	if (!p_disk) return false;

	wxString caption = _("Import data to track");

	UiDiskOpenFileDialog fdlg(
		caption,
		frame->GetIniExportFilePath(),
		wxEmptyString,
		_("All files (*.*)|*.*"));
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
bool UiDiskRawTrack::ShowImportTrackRangeDialog(const wxString &path, int st_trk, int st_sid, int st_sec)
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
		DiskImageTrack *track = GetSelectedTrack();
		if (!track) track = GetFirstTrack();
		if (!GetFirstAndLastSectorNumOnTrack(track, st_sec, ed_sec)) {
			return false;
		}
		st_trk = track->GetTrackNumber();
		st_sid = track->GetSideNumber();
		ed_trk = st_trk;
		ed_sid = st_sid;
	}

	RawExpBox dlg(this, wxID_ANY, caption, p_disk, m_side_number
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
bool UiDiskRawTrack::ImportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec, bool inv_data, bool rev_side)
{
	// エクスポート元パスを覚えておく
	frame->SetIniExportFilePath(path);

	if (!p_disk) return false;

	int trk = st_trk;
	int sid = st_sid;
	int sec = st_sec;
	int ssid = m_side_number >= 0 ? m_side_number : 0;
	int esid = m_side_number >= 0 ? m_side_number : (p_disk->GetSidesPerDisk() - 1);
	int esec = p_disk->GetSectorsPerTrack();

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
			DiskImageTrack *track = p_disk->GetTrack(trk, sid);
			if (track) {
				if (trk == ed_trk && sid == ed_sid) {
					// 最終トラック＆最終サイドなら終了セクタまで
					esec = ed_sec;
				}
				for(; sec <= esec && !eof; sec++) {
					DiskImageSector *sector = track->GetSector(sec);
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

/// ディスク全体または偶数奇数トラックのIDを一括変更
/// @param [in] type_num  1:ID H  3:ID N
/// @param [in] even_odd  -1:All Tracks 0:Even Tracks 1:Odd Tracks
void UiDiskRawTrack::ModifyIDonDisk(int type_num, int even_odd)
{
	if (!p_disk) return;

	DiskImageTrack *track = NULL;
	DiskImageSector *sector = NULL;
	if (even_odd >= 0) {
		track = p_disk->GetTrack(even_odd & 1);
	}
	if (!GetFirstSectorOnTrack(&track, &sector)) {
		return;
	}

	wxString title;
	int value = 0;
	int maxvalue = 255;
	switch(type_num) {
	case 1:
		title = _("Modify ID H on %s.");
		value = sector->GetIDH();
		break;
	case 3:
		title = _("Modify ID N on %s.");
		value = sector->GetIDN();
		maxvalue = DiskImageSector::ConvSecSizeToIDN(sector->GetSectorBufferSize());
		break;
	}
	if (even_odd >= 0) {
		if (even_odd & 1) {
			title = wxString::Format(title, _("odd tracks"));
		} else {
			title = wxString::Format(title, _("even tracks"));
		}
	} else {
		title = wxString::Format(title, _("the disk"));
	}

	RawParamBox dlg(this, wxID_ANY, title, type_num, value, maxvalue);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		int newvalue = dlg.GetValue();
		DiskImageTracks *tracks = p_disk->GetTracks();
		if (tracks) {
			size_t start = 0;
			size_t step = 1;
			if (even_odd >= 0) {
				// for even or odd tracks
				start = (even_odd & 1);
				step = 2;
			}
			for(size_t num=start; num < tracks->Count(); num+=step) {
				track = tracks->Item(num);
				if (!track) continue;
				if (m_side_number >= 0) {
					// 片面の場合
					if (m_side_number != track->GetSideNumber()) continue;
				}
				switch(type_num) {
				case 1:
					track->SetAllIDH(newvalue & 0xff);
					track->SetSideNumber(newvalue);
					break;
				case 3:
					track->SetAllIDN(newvalue & 0xff);
					break;
				}
			}
		}
		// リストを更新
		parent->RefreshAllData();
	}
}

/// ディスク上の密度を一括変更
void UiDiskRawTrack::ModifyDensityOnDisk()
{
	if (!p_disk) return;

	DiskImageTrack *track = NULL;
	DiskImageSector *sector = NULL;
	if (!GetFirstSectorOnTrack(&track, &sector)) {
		return;
	}

	bool sdensity = sector->IsSingleDensity();

	DensityParamBox dlg(this, wxID_ANY, sdensity);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		bool newsdensity = dlg.IsSingleDensity();
		if (sdensity != newsdensity) {
			DiskImageTracks *tracks = p_disk->GetTracks();
			if (tracks) {
				for(size_t num=0; num < tracks->Count(); num++) {
					track = tracks->Item(num);
					if (!track) continue;
					if (m_side_number >= 0) {
						// 片面の場合
						if (m_side_number != track->GetSideNumber()) continue;
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
void UiDiskRawTrack::ShowTrackAttr()
{
	if (!p_disk) return;

	int num = GetListSelectedNum();
	if (num < 0) return;

	wxUint32 offset = p_disk->GetOffset(num);

	RawTrackBox dlg(this, wxID_ANY, num, offset, p_disk);
	dlg.ShowModal();
}

/// トラックを追加
void UiDiskRawTrack::AppendTrack()
{
	if (!p_disk) return;

	int disk_number = -1;
	int side_number = -1;

	if (p_disk->IsReversible()) {
		// AB面ありで選択位置がA,B面どちらか
		frame->GetDiskListSelectedPos(disk_number, side_number);
	}

	p_disk->AddNewTrack(side_number);

	// 画面更新
	parent->RefreshAllData();
}

/// トラックを削除
void UiDiskRawTrack::DeleteTracks()
{
	if (!p_disk) return;

	int disk_number = -1;
	int side_number = -1;
	int num = GetListSelectedNum();
	if (num < 0) return;

	if (p_disk->IsReversible()) {
		// AB面ありで選択位置がA,B面どちらか
		frame->GetDiskListSelectedPos(disk_number, side_number);
	}

	int ans = wxYES;
	wxString msg = wxString::Format(_("Do you really want to delete tracks?"));
	ans = wxMessageBox(msg, _("Delete Tracks"), wxYES_NO);
	if (ans == wxYES) {
		p_disk->DeleteTracks(num, -1, side_number);

		// 画面更新
//		parent->RefreshAllData();
		RefreshTracks();
		// セクタリストはクリア
		parent->ClearSectorListData();
	}
}

/// 選択行のトラックを返す
DiskImageTrack *UiDiskRawTrack::GetSelectedTrack()
{
	return GetTrack(GetListSelection());
}

/// 指定行のトラックを返す
DiskImageTrack *UiDiskRawTrack::GetTrack(const MyRawTrackListItem &row)
{
	if (!p_disk) return NULL;

	DiskImageTrack *track = NULL;
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
	if (!row.IsOk()) return NULL;
#else
	if (row == wxNOT_FOUND) return NULL;
#endif
	int num = (int)GetItemData(row);
	wxUint32 offset = p_disk->GetOffset(num);
	if (offset > 0) {
		track = p_disk->GetTrackByOffset(offset);
	}
	return track;
}

/// 最初のトラックを返す
DiskImageTrack *UiDiskRawTrack::GetFirstTrack()
{
	if (!p_disk) return NULL;
	int limit = p_disk->GetCreatableTracks();
	DiskImageTrack *track = NULL;
	for(int num=0; track == NULL && num < limit; num++) {
		track = p_disk->GetTrack(num, m_side_number >= 0 ? m_side_number : 0);
	}
	return track;
}

/// トラックの最初のセクタを得る
/// @param [in,out] track  トラック NULLのときは最初のトラックのセクタ１
/// @param [out]    sector セクタ１
bool UiDiskRawTrack::GetFirstSectorOnTrack(DiskImageTrack **track, DiskImageSector **sector)
{
	if (!track || !sector) return false;
	if (!(*track)) *track = GetFirstTrack();
	if (*track) *sector = (*track)->GetSectorByIndex(0);
	if (!(*sector)) {
		*track = NULL;
		return false;
	}
	return true;
}

/// トラックの開始セクタ番号と終了セクタ番号を得る
bool UiDiskRawTrack::GetFirstAndLastSectorNumOnTrack(const DiskImageTrack *track, int &start_sector, int &end_sector)
{
	if (!track) return false;
	DiskImageSectors *sectors = track->GetSectors();
	if (!sectors || sectors->Count() == 0) return false;
	start_sector = 0xffff;
	end_sector = 0;
	for(size_t r = 0; r < sectors->Count(); r++) {
		DiskImageSector *sector = sectors->Item(r);
		if (!sector) continue;
		if (start_sector > sector->GetSectorNumber()) start_sector = sector->GetSectorNumber(); 
		if (end_sector < sector->GetSectorNumber()) end_sector = sector->GetSectorNumber(); 
	}
	return true;
}

/// 次のサイドへ
void UiDiskRawTrack::IncreaseSide()
{
	if (!p_disk) return;

	int row = GetListSelectedRow();
	if (row < 0) return;

	row++;
	if (row >= GetItemCount()) return;

	SelectListRow(row);

	SelectData();
}

/// 前のサイドへ
void UiDiskRawTrack::DecreaseSide()
{
	if (!p_disk) return;

	int row = GetListSelectedRow();
	if (row <= 0) return;
	row--;

	SelectListRow(row);

	SelectData();
}

//////////////////////////////////////////////////////////////////////
//
//
//
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
UiDiskRawSectorListStoreModel::UiDiskRawSectorListStoreModel(wxWindow *parent)
{
	ctrl = (UiDiskRawSector *)parent;
}

bool UiDiskRawSectorListStoreModel::IsEnabledByRow(unsigned int row, unsigned int col) const
{
    return true;
}

int UiDiskRawSectorListStoreModel::Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const
{
	DiskImageSectors *sectors = ctrl->GetSectors();
	if (!sectors) return 0;

	int idx = -1;
	if (!ctrl->FindColumn(col, &idx)) return 0;

	int cmp = 0;
	int i1 = (int)GetItemData(item1);
	int i2 = (int)GetItemData(item2);
	switch(idx) {
	case SECTORCOL_ID_R:
		cmp = UiDiskRawSectorListCtrl::CompareIDR(sectors, i1, i2, ascending ? 1 : -1);
		break;
	case SECTORCOL_NUM:
		cmp = UiDiskRawSectorListCtrl::CompareNum(sectors, i1, i2, ascending ? 1 : -1);
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
UiDiskRawSectorListCtrl::UiDiskRawSectorListCtrl(UiDiskFrame *parentframe, wxWindow *parent, UiDiskRawSector *sub, wxWindowID id)
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	: MyCDListCtrl(
		parentframe, parent, id,
		gUiDiskRawSectorColumnDefs,
		NULL,
		wxDV_MULTIPLE,
		new UiDiskRawSectorListStoreModel(sub)
	)
#else
	: MyCListCtrl(
		parentframe, parent, id,
		gUiDiskRawSectorColumnDefs,
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
void UiDiskRawSectorListCtrl::SetListData(DiskImageSector *sector, int row, MyRawSectorListValue *values)
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
void UiDiskRawSectorListCtrl::InsertListData(DiskImageSector *sector, int row, int idx)
{
	MyRawSectorListValue values[SECTORCOL_END];

	SetListData(sector, row, values);

	InsertListItem(row, values, SECTORCOL_END, (wxUIntPtr)idx);
}

/// リストデータを更新
void UiDiskRawSectorListCtrl::UpdateListData(DiskImageSector *sector, int row, int idx)
{
	MyRawSectorListValue values[SECTORCOL_END];

	SetListData(sector, row, values);

	UpdateListItem(row, values, SECTORCOL_END, (wxUIntPtr)idx);
}

#ifdef USE_LIST_CTRL_ON_SECTOR_LIST
//
//
//

/// ソート用アイテム
struct st_sector_list_sort_exp {
	DiskImageSectors *sectors;
	int (*cmpfunc)(DiskImageSectors *sectors, int i1, int i2, int dir);
	int dir;
};

/// アイテムをソート
void UiDiskRawSectorListCtrl::SortDataItems(DiskImageTrack *track, int col)
{
	struct st_sector_list_sort_exp exp;

	// ソート対象か
	int idx;
	bool match_col;
	exp.dir = SelectColumnSortDir(col, idx, match_col);

	DiskImageSectors *sectors = NULL;
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
int wxCALLBACK UiDiskRawSectorListCtrl::Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata)
{
	struct st_sector_list_sort_exp *exp = (struct st_sector_list_sort_exp *)sortdata;

	int cmp = exp->cmpfunc != NULL ? exp->cmpfunc(exp->sectors, (int)item1, (int)item2, exp->dir) : 0;
	if (cmp == 0) cmp = ((int)item1 - (int)item2);
	return cmp;
}
#endif

/// ID Rでソート
int UiDiskRawSectorListCtrl::CompareIDR(DiskImageSectors *sectors, int i1, int i2, int dir)
{
	return (sectors->Item(i1)->GetIDR() - sectors->Item(i2)->GetIDR()) * dir;
}
int UiDiskRawSectorListCtrl::CompareNum(DiskImageSectors *sectors, int i1, int i2, int dir)
{
	return (i1 - i2) * dir;
}

//////////////////////////////////////////////////////////////////////
//
// 右パネルのセクタリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskRawSector, UiDiskRawSectorListCtrl)
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, UiDiskRawSector::OnItemContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, UiDiskRawSector::OnItemActivated)
	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, UiDiskRawSector::OnSelectionChanged)
	EVT_DATAVIEW_ITEM_BEGIN_DRAG(wxID_ANY, UiDiskRawSector::OnBeginDrag)
#else
	EVT_CONTEXT_MENU(UiDiskRawSector::OnContextMenu)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, UiDiskRawSector::OnItemActivated)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, UiDiskRawSector::OnSelectionChanged)
	EVT_LIST_COL_CLICK(wxID_ANY, UiDiskRawSector::OnColumnClick)
	EVT_LIST_BEGIN_DRAG(wxID_ANY, UiDiskRawSector::OnBeginDrag)
#endif
	EVT_CHAR(UiDiskRawSector::OnChar)

	EVT_MENU(IDM_EXPORT_FILE, UiDiskRawSector::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, UiDiskRawSector::OnImportFile)

	EVT_MENU(IDM_INVERT_DATA, UiDiskRawSector::OnChangeInvertData)

	EVT_MENU(IDM_MODIFY_ID_C_TRACK, UiDiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_H_TRACK, UiDiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_N_TRACK, UiDiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_DENSITY_TRACK, UiDiskRawSector::OnModifyDensityOnTrack)
	EVT_MENU(IDM_MODIFY_SECTORS_TRACK, UiDiskRawSector::OnModifySectorsOnTrack)
	EVT_MENU(IDM_MODIFY_SIZE_TRACK, UiDiskRawSector::OnModifySectorSizeOnTrack)
	EVT_MENU(IDM_APPEND_SECTOR, UiDiskRawSector::OnAppendSector)
	EVT_MENU(IDM_DELETE_SECTOR, UiDiskRawSector::OnDeleteSector)
	EVT_MENU(IDM_DELETE_SECTORS_BELOW, UiDiskRawSector::OnDeleteSectorsOnTrack)
	EVT_MENU(IDM_EDIT_SECTOR, UiDiskRawSector::OnEditSector)

	EVT_MENU(IDM_PROPERTY_SECTOR, UiDiskRawSector::OnPropertySector)
wxEND_EVENT_TABLE()

UiDiskRawSector::UiDiskRawSector(UiDiskFrame *parentframe, UiDiskRawPanel *parentwindow)
	: UiDiskRawSectorListCtrl(parentframe, parentwindow, this, wxID_ANY)
{
	m_initialized = false;
	parent   = parentwindow;
	frame    = parentframe;

	p_track = NULL;

	wxFont font;
	frame->GetDefaultListFont(font);
	SetFont(font);

	// popup menu
	MakePopupMenu();

	m_initialized = true;
}

UiDiskRawSector::~UiDiskRawSector()
{
	delete menuPopup;
}


/// セクタリスト選択
void UiDiskRawSector::OnSelectionChanged(MyRawSectorListEvent& event)
{
	if (!m_initialized) return;

	DiskImageSector *sector = GetSelectedSector();
	if (!sector) {
		// 非選択
		UnselectItem();
		return;
	}

	// 選択
	SelectItem(sector);
}

/// セクタリストからドラッグ開始
void UiDiskRawSector::OnBeginDrag(MyRawSectorListEvent& event)
{
	DragDataSourceForExternal();
}

/// セクタリスト右クリック
void UiDiskRawSector::OnItemContextMenu(MyRawSectorListEvent& event)
{
	ShowPopupMenu();
}

/// 右クリック
void UiDiskRawSector::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// セクタリスト ダブルクリック
void UiDiskRawSector::OnItemActivated(MyRawSectorListEvent& event)
{
	ShowSectorAttr();
}

/// セクタリスト カラムをクリック
void UiDiskRawSector::OnColumnClick(MyRawSectorListEvent& event)
{
	int col = event.GetColumn();
#ifdef USE_LIST_CTRL_ON_SECTOR_LIST
	SortDataItems(p_track, col);
#endif
}

/// セクタリスト エクスポート選択
void UiDiskRawSector::OnExportFile(wxCommandEvent& event)
{
	ShowExportDataFileDialog();
}

/// セクタリスト インポート選択
void UiDiskRawSector::OnImportFile(wxCommandEvent& event)
{
	ShowImportDataFileDialog();
}

/// データを反転するチェック選択
void UiDiskRawSector::OnChangeInvertData(wxCommandEvent& event)
{
	parent->InvertData(event.IsChecked());
}

/// トラック上のID一括変更選択
void UiDiskRawSector::OnModifyIDonTrack(wxCommandEvent& event)
{
	ModifyIDonTrack(event.GetId() - IDM_MODIFY_ID_C_TRACK);
}

/// トラック上の密度一括変更選択
void UiDiskRawSector::OnModifyDensityOnTrack(wxCommandEvent& event)
{
	ModifyDensityOnTrack();
}

/// トラック上のセクタ数一括変更選択
void UiDiskRawSector::OnModifySectorsOnTrack(wxCommandEvent& event)
{
	ModifySectorsOnTrack();
}

/// トラック上のセクタサイズ一括変更選択
void UiDiskRawSector::OnModifySectorSizeOnTrack(wxCommandEvent& event)
{
	ModifySectorSizeOnTrack();
}

/// セクタ追加選択
void UiDiskRawSector::OnAppendSector(wxCommandEvent& event)
{
	ShowAppendSectorDialog();
}

/// セクタ削除選択
void UiDiskRawSector::OnDeleteSector(wxCommandEvent& event)
{
	DeleteSector();
}

/// トラック上のセクタ一括削除選択
void UiDiskRawSector::OnDeleteSectorsOnTrack(wxCommandEvent& event)
{
	DeleteSectorsOnTrack();
}

/// セクタ編集選択
void UiDiskRawSector::OnEditSector(wxCommandEvent& event)
{
	EditSector();
}

/// セクタプロパティ選択
void UiDiskRawSector::OnPropertySector(wxCommandEvent& event)
{
	ShowSectorAttr();
}

/// セクタリスト上でキー押下
void UiDiskRawSector::OnChar(wxKeyEvent& event)
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
	case WXK_LEFT:
		// Allow <- 前のレコードへ
		parent->DecreaseSide();
		break;
	case WXK_RIGHT:
		// Allow -> 次のレコードへ
		parent->IncreaseSide();
		break;
	default:
		event.Skip();
		break;
	}
}

/// ポップアップメニュー作成
void UiDiskRawSector::MakePopupMenu()
{
	menuPopup = new MyMenu;
	MyMenu *sm = new MyMenu;
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
}

/// ポップアップメニュー表示
void UiDiskRawSector::ShowPopupMenu()
{
	if (!menuPopup) return;

	menuPopup->Check(IDM_INVERT_DATA, parent->InvertData());

	bool opened = (p_track != NULL);
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
	menuPopup->Enable(IDM_EDIT_SECTOR, opened && (cnt == 1));
	menuPopup->Enable(IDM_PROPERTY_SECTOR, opened && (cnt == 1));

	PopupMenu(menuPopup);
}

// セクタリスト選択
void UiDiskRawSector::SelectItem(DiskImageSector *sector)
{
	// ダンプリストをセット
	frame->SetBinDumpData(sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetSectorBuffer(), sector->GetSectorSize());

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);

}

// セクタリスト非選択
void UiDiskRawSector::UnselectItem()
{
	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// セクタリストにデータをセット
void UiDiskRawSector::SetSectors(DiskImageTrack *newtrack)
{
	p_track = newtrack;

	RefreshSectors();
}

/// セクタリストを返す
DiskImageSectors *UiDiskRawSector::GetSectors() const
{
	if (p_track) {
		return p_track->GetSectors();
	} else {
		return NULL;
	}
}

/// セクタリストをリフレッシュ
void UiDiskRawSector::RefreshSectors()
{
	if (!p_track) return;

	DiskImageSectors *sectors = p_track->GetSectors();
	if (!sectors) return;

	int row = 0;
	int row_count = (int)GetItemCount();

	for (size_t i=0; i<sectors->Count(); i++) {
		DiskImageSector *sector = sectors->Item(i);
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
	SortDataItems(p_track, -1);
#endif

	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// セクタリストをクリア
void UiDiskRawSector::ClearSectors()
{
	DeleteAllListItems();

	p_track = NULL;

	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// 選択しているセクタを返す
DiskImageSector *UiDiskRawSector::GetSelectedSector(int *pos)
{
	if (!p_track) return NULL;

	int idx = GetListSelectedNum();
	if (idx == wxNOT_FOUND) return NULL;

	if (pos) *pos = idx;

	DiskImageSector *sector = p_track->GetSectorByIndex(idx);
	return sector;
}

/// セクタを返す
DiskImageSector *UiDiskRawSector::GetSector(const MyRawSectorListItem &item)
{
	if (!p_track) return NULL;

	DiskImageSectors *sectors = p_track->GetSectors();
	if (!sectors) return NULL;

#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	int row = ItemToRow(item);
	if (row == wxNOT_FOUND) return NULL;
#else
	int row = (int)GetItemData(item);
#endif

	DiskImageSector *sector = sectors->Item(row);
	if (!sector) return NULL;

	return sector;
}

/// ドラッグする 外部へドロップ場合
bool UiDiskRawSector::DragDataSourceForExternal()
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
bool UiDiskRawSector::CopyToClipboard()
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
bool UiDiskRawSector::CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object)
{
	MyRawSectorListItems selected_items;
	int selected_count = GetListSelections(selected_items);
	if (selected_count <= 0) return false;

	UiDiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	if (!app->MakeTempDir(tmp_dir_name)) {
		return false;
	}

	int cnt = 0;
	for(int n = 0; n < selected_count; n++) {
		DiskImageSector *sector = GetSector(selected_items.Item(n));
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
void UiDiskRawSector::ReleaseFileObject(const wxString &tmp_dir_name)
{
	UiDiskApp *app = &wxGetApp();

	// テンポラリディレクトリを削除
	app->RemoveTempDir(tmp_dir_name);
}

/// クリップボードからペースト
bool UiDiskRawSector::PasteFromClipboard()
{
	return parent->PasteFromClipboard();
}

/// エクスポートダイアログ表示
bool UiDiskRawSector::ShowExportDataFileDialog()
{
	MyRawSectorListItems selected_items;
	int selected_count = GetListSelections(selected_items);

	bool sts = true;
	if (selected_count == 1) {
		// 単一行 指定
		DiskImageSector *sector = GetSector(selected_items.Item(0));
		if (!sector) return false;

		wxString filename = parent->MakeFileName(sector);

		UiDiskSaveFileDialog dlg(
			_("Export data from sector"),
			frame->GetIniExportFilePath(),
			filename,
			_("All files (*.*)|*.*"));

		int rc = dlg.ShowModal();
		wxString path = dlg.GetPath();

		if (rc == wxID_OK) {
			sts = ExportDataFile(path, sector);
		}

	} else {
		// 複数行 指定
		UiDiskDirDialog dlg(
			_("Export each datas from selected sector"),
			frame->GetIniExportFilePath());

		int rc = dlg.ShowModal();
		if (rc != wxID_OK) {
			return false;
		}

		wxString dir_path = dlg.GetPath();
		for(int n = 0; n < selected_count; n++) {
			DiskImageSector *sector = GetSector(selected_items.Item(n));
			if (!sector) continue;

			// ファイルパスを作成
			wxFileName file_path(dir_path, parent->MakeFileName(sector));

			sts &= ExportDataFile(file_path.GetFullPath(), sector);
		}
	}

	return sts;
}

/// 指定したファイルにセクタのデータをエクスポート
bool UiDiskRawSector::ExportDataFile(const wxString &path, DiskImageSector *sector)
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
bool UiDiskRawSector::ShowImportDataFileDialog()
{
	UiDiskOpenFileDialog dlg(
		_("Import data to sector"),
		frame->GetIniExportFilePath(),
		wxEmptyString,
		_("All files (*.*)|*.*"));

	int dlgsts = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (dlgsts != wxID_OK) {
		return false;
	}

	DiskImageSector *sector = GetSelectedSector();
	int st_trk = (p_track ? p_track->GetTrackNumber() : -1);
	int st_sid = (p_track ? p_track->GetSideNumber() : 0);
	int st_sec = (sector ? sector->GetSectorNumber() : 1);

	return parent->ShowImportTrackRangeDialog(path, st_trk, st_sid, st_sec);
}

/// セクタ情報プロパティダイアログ表示
bool UiDiskRawSector::ShowSectorAttr()
{
	DiskImageSector *sector = GetSelectedSector();
	if (!sector) return false;

	RawSectorBox dlg(this, wxID_ANY, _("Sector Information")
		, sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetIDN()
		, sector->GetSectorsPerTrack()
		, sector->IsDeleted(), sector->IsSingleDensity(), sector->GetSectorStatus()
	);

	int rc = dlg.ShowModal();
	if (rc != wxID_OK) return false;

	rc = wxNO;
	int new_size = DiskImageSector::ConvIDNToSecSize(dlg.GetIdN());
	if (sector->GetSectorBufferSize() < new_size) {
		wxString msg = wxString::Format(_("Need expand the buffer size to %d bytes. Are you sure to do it?"), new_size);
		rc = wxMessageBox(msg, _("Expand Sector Size"), wxICON_WARNING | wxYES_NO);
		if (rc == wxYES) {
			// 大きくしたときバッファサイズも大きくする
			sector->ModifySectorSize(new_size);

			// トラックのサイズを再計算&オフセットを再計算する
			p_track->ShrinkAndCalcOffsets(false);
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
void UiDiskRawSector::ModifyIDonTrack(int type_num)
{
	if (!p_track) return;

	DiskImageSector *sector = p_track->GetSectorByIndex(0);
	if (!sector) return;

	wxString title;
	int value = 0;
	int maxvalue = 255;
	switch(type_num) {
		case RawParamBox::TYPE_IDC:
			title = _("Modify ID C on the track.");
			value = sector->GetIDC();
			break;
		case RawParamBox::TYPE_IDH:
			title = _("Modify ID H on the track.");
			value = sector->GetIDH();
			break;
		case RawParamBox::TYPE_IDN:
			title = _("Modify ID N on the track.");
			value = sector->GetIDN();
			maxvalue = DiskImageSector::ConvSecSizeToIDN(sector->GetSectorBufferSize());
			break;
		case RawParamBox::TYPE_NUM_OF_SECTORS:
			title = _("Modify number of sector on the track.");
			value = sector->GetSectorsPerTrack();
			break;
	}

	RawParamBox dlg(this, wxID_ANY, title, type_num, value, maxvalue);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		int newvalue = dlg.GetValue();
		if (value != newvalue) {
			
			switch(type_num) {
				case RawParamBox::TYPE_IDC:
					p_track->SetAllIDC(newvalue);
					p_track->SetTrackNumber(newvalue);
					break;
				case RawParamBox::TYPE_IDH:
					p_track->SetAllIDH(newvalue);
					p_track->SetSideNumber(newvalue);
					break;
				case RawParamBox::TYPE_IDN:
					p_track->SetAllIDN(newvalue);
					break;
				case RawParamBox::TYPE_NUM_OF_SECTORS:
					p_track->SetAllSectorsPerTrack(newvalue);
					break;
			}

			// リストを更新
			RefreshSectors();
		}
	}
}

/// トラック上の密度を一括変更
void UiDiskRawSector::ModifyDensityOnTrack()
{
	if (!p_track) return;

	DiskImageSector *sector = p_track->GetSectorByIndex(0);
	if (!sector) return;

	bool sdensity = sector->IsSingleDensity();

	DensityParamBox dlg(this, wxID_ANY, sdensity);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		bool newsdensity = dlg.IsSingleDensity();
		if (sdensity != newsdensity) {

			p_track->SetAllSingleDensity(newsdensity);

			// リストを更新
			RefreshSectors();
		}
	}
}

/// トラック上のセクタ数を一括変更
void UiDiskRawSector::ModifySectorsOnTrack()
{
	ModifyIDonTrack(4);
}

/// トラック上のセクタサイズを一括変更
void UiDiskRawSector::ModifySectorSizeOnTrack()
{
	if (!p_track) return;

	DiskImageSector *sector = p_track->GetSectorByIndex(0);
	if (!sector) return;

	int value = sector->GetSectorSize();
	int maxvalue = 2048;

	RawParamBox dlg(this, wxID_ANY, _("Modify sector size on the track."), RawParamBox::TYPE_SECTOR_SIZE, value, maxvalue);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		int newvalue = dlg.GetValue();
		p_track->SetAllSectorSize(newvalue);

		// 画面更新
		frame->UpdateDataOnWindow(true);
	}
}

/// セクタを追加ダイアログを表示
void UiDiskRawSector::ShowAppendSectorDialog()
{
	if (!p_track) return;

	DiskImageSector *sector = p_track->GetSectorByIndex(0);
	if (!sector) return;

	int new_sec_num = p_track->GetMaxSectorNumber() + 1;

	RawSectorBox dlg(this, wxID_ANY, _("Add Sector")
		, sector->GetIDC(), sector->GetIDH(), new_sec_num, sector->GetIDN()
		, 1
		, sector->IsDeleted(), sector->IsSingleDensity(), sector->GetSectorStatus()
		, SECTORBOX_HIDE_SECTOR_NUMS
	);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		p_track->AddNewSector(
			dlg.GetIdC(),
			dlg.GetIdH(),
			dlg.GetIdR(),
			DiskImageSector::ConvIDNToSecSize(dlg.GetIdN()),
			sector->IsSingleDensity(),
			dlg.GetStatus()
		);

		// 画面更新
		parent->RefreshAllData();
	}
}

/// セクタを削除
void UiDiskRawSector::DeleteSector()
{
	int pos = 0;
	DiskImageSector *sector = GetSelectedSector(&pos);
	if (!sector) return;

	int ans = wxYES;
	wxString msg = wxString::Format(_("Do you really want to delete current sector?"));
	ans = wxMessageBox(msg, _("Delete Sector"), wxYES_NO);
	if (ans == wxYES) {
		p_track->DeleteSectorByIndex(pos);

		// 画面更新
		parent->RefreshAllData();
	}
}

/// セクタを編集
void UiDiskRawSector::EditSector()
{
	int pos = 0;
	DiskImageSector *sector = GetSelectedSector(&pos);
	if (!sector) return;

	size_t bufsize = sector->GetSectorBufferSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	if (buf == NULL || bufsize <= 0) {
		wxMessageBox(_("No sector data exists."), _("Edit Sector"), wxICON_ERROR | wxOK);
		return;
	}

	// データを反転して出力するか
	bool inverted = false;
//	int sts = wxMessageBox(_("Do you want to invert data?"), _("Output Sector Data"), wxYES | wxNO | wxNO_DEFAULT);
//	if (sts == wxYES) {
//		inverted = true;
//	}
	inverted = parent->InvertData();

	UiDiskApp *app = &wxGetApp();

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
	if (!frame->OpenFileWithEditor(EDITOR_TYPE_BINARY, tmp_path)) {
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
void UiDiskRawSector::DeleteSectorsOnTrack()
{
	DiskImageSector *sector = GetSelectedSector();
	if (!sector) return;

	int ans = wxYES;
	wxString msg = wxString::Format(_("Do you really want to delete sectors?"));
	ans = wxMessageBox(msg, _("Delete Sectors"), wxYES_NO);
	if (ans == wxYES) {
		p_track->DeleteSectors(sector->GetSectorNumber(), -1);

		// 画面更新
		parent->RefreshAllData();
	}
}
