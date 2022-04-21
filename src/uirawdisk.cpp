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
#include "main.h"
#include "rawtrackbox.h"
#include "rawsectorbox.h"
#include "rawparambox.h"
#include "rawexpbox.h"
#include "utils.h"


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

	// fit size on parent window
	wxSize sz = parentwindow->GetClientSize();
	SetSize(sz);

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	// control panel
	lpanel = new L3DiskRawTrack(frame, this);
	rpanel = new L3DiskRawSector(frame, this);
	SplitVertically(lpanel, rpanel, 200);

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
	return (int)lpanel->GetFirstSelected();
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

/// クリップボードからペースト
bool L3DiskRawPanel::PasteFromClipboard()
{
	return lpanel->PasteFromClipboard();
}

/// トラックからエクスポートダイアログ表示
bool L3DiskRawPanel::ShowExportTrackDialog()
{
	return lpanel->ShowExportTrackDialog();
}

/// トラックへインポートダイアログ表示
bool L3DiskRawPanel::ShowImportTrackDialog()
{
	return lpanel->ShowImportTrackDialog();
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

/// ファイル名
wxString L3DiskRawPanel::MakeFileName(DiskD88Sector *sector)
{
	return MakeFileName(
		sector->GetIDC(), sector->GetIDH(), sector->GetIDR(),
		sector->GetIDC(), sector->GetIDH(), sector->GetIDR()
	);
}

/// ファイル名
wxString L3DiskRawPanel::MakeFileName(int st_c, int st_h, int st_r, int ed_c, int ed_h, int ed_r)
{
	return wxString::Format(wxT("%02d-%02d-%02d--%02d-%02d-%02d.bin"),
		st_c, st_h, st_r, ed_c, ed_h, ed_r
	);
}

/// フォントをセット
void L3DiskRawPanel::SetListFont(const wxFont &font)
{
	lpanel->SetFont(font);
	lpanel->Refresh();
	rpanel->SetFont(font);
	rpanel->Refresh();
}

//
// 左パネルのトラックリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRawTrack, wxListView)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, L3DiskRawTrack::OnListItemSelected)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, L3DiskRawTrack::OnListActivated)

	EVT_LIST_BEGIN_DRAG(wxID_ANY, L3DiskRawTrack::OnBeginDrag)

	EVT_CHAR(L3DiskRawTrack::OnChar)

	EVT_CONTEXT_MENU(L3DiskRawTrack::OnContextMenu)

	EVT_MENU(IDM_EXPORT_TRACK, L3DiskRawTrack::OnExportTrack)
	EVT_MENU(IDM_IMPORT_TRACK, L3DiskRawTrack::OnImportTrack)

	EVT_MENU(IDM_MODIFY_ID_H_DISK, L3DiskRawTrack::OnModifyIDonDisk)
	EVT_MENU(IDM_MODIFY_ID_N_DISK, L3DiskRawTrack::OnModifyIDonDisk)
	EVT_MENU(IDM_MODIFY_DENSITY_DISK, L3DiskRawTrack::OnModifyDensityOnDisk)

	EVT_MENU(IDM_MODIFY_ID_C_TRACK, L3DiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_H_TRACK, L3DiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_N_TRACK, L3DiskRawTrack::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_DENSITY_TRACK, L3DiskRawTrack::OnModifyDensityOnTrack)
	EVT_MENU(IDM_MODIFY_SECTORS_TRACK, L3DiskRawTrack::OnModifySectorsOnTrack)
	EVT_MENU(IDM_MODIFY_SIZE_TRACK, L3DiskRawTrack::OnModifySectorSizeOnTrack)

	EVT_MENU(IDM_DELETE_TRACKS_BELOW, L3DiskRawTrack::OnDeleteTracksBelow)

	EVT_MENU(IDM_PROPERTY_TRACK, L3DiskRawTrack::OnPropertyTrack)
wxEND_EVENT_TABLE()

L3DiskRawTrack::L3DiskRawTrack(L3DiskFrame *parentframe, L3DiskRawPanel *parentwindow)
       : wxListView(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL)
{
	parent   = parentwindow;
	frame    = parentframe;

	disk = NULL;
	side_number = -1;

	AppendColumn(_("Num"), wxLIST_FORMAT_RIGHT, 42);
	AppendColumn(_("Track"), wxLIST_FORMAT_RIGHT, 32);
	AppendColumn(_("Side"), wxLIST_FORMAT_RIGHT, 32);
	AppendColumn(_("Offset"), wxLIST_FORMAT_RIGHT, 60);

	wxFont font;
	frame->GetDefaultListFont(font);
	SetFont(font);

	// popup menu
	menuPopup = new wxMenu;
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
	menuPopup->Append(IDM_DELETE_TRACKS_BELOW, _("Delete All Tracks Below Current Track"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY_TRACK, _("&Property"));
}

L3DiskRawTrack::~L3DiskRawTrack()
{
	delete menuPopup;
}

/// トラックリストを選択
void L3DiskRawTrack::OnListItemSelected(wxListEvent& event)
{
	if (!disk) return;

	int row = (int)event.GetIndex();
	SelectData(row);
}

/// トラックリストをダブルクリック
void L3DiskRawTrack::OnListActivated(wxListEvent &event)
{
	ShowTrackAttr();
}

/// トラックリスト右クリック
void L3DiskRawTrack::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
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

/// セクタリストからドラッグ開始
void L3DiskRawTrack::OnBeginDrag(wxListEvent& event)
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
void L3DiskRawTrack::SelectData(int row)
{
	int num = (int)GetItemData(row);
	wxUint32 offset = disk->GetOffset(num);
	if (offset > 0) {
		parent->SetSectorListData(disk->GetTrackByOffset(offset));
	} else {
		parent->ClearSectorListData();
	}
}

/// トラックリストをセット
void L3DiskRawTrack::SetTracks(DiskD88Disk *newdisk, int newsidenum)
{
	if (!newdisk) return;

	disk = newdisk;
	side_number = newsidenum;

	SetTracks();

	// セクタリストはクリア
	parent->ClearSectorListData();
}

/// トラックリストを再セット
void L3DiskRawTrack::RefreshTracks()
{
	long sel_pos = GetFirstSelected();
	long foc_pos = GetFocusedItem();
	SetTracks();
	if (foc_pos >= 0) Focus(foc_pos);
	if (sel_pos >= 0) Select(sel_pos);
}

/// トラックリストを再セット
void L3DiskRawTrack::SetTracks()
{
	DeleteAllItems();

	if (!disk) return;

	int sides = disk->GetSidesPerDisk();
	for(int pos=(side_number >= 0 ? side_number : 0), row=0; pos < DISKD88_MAX_TRACKS; pos+=(side_number >= 0 ? sides : 1)) {
		wxUint32 offset = disk->GetOffset(pos);
		DiskD88Track *trk = disk->GetTrackByOffset(offset);
		int trk_num = -1;
		int sid_num = -1;
//		if (newsidenum >= 0) {
//			if (!trk || newsidenum != trk->GetSideNumber()) continue;
//		}
		if (trk) {
			trk_num = trk->GetTrackNumber();
			sid_num = trk->GetSideNumber();
		}
		InsertItem(row, wxString::Format(wxT("%d"), pos));
		SetItem(row, 1, trk_num >= 0 ? wxString::Format(wxT("%d"), trk_num) : wxT("--"));
		SetItem(row, 2, sid_num >= 0 ? wxString::Format(wxT("%d"), sid_num) : wxT("--"));
		SetItem(row, 3, wxString::Format(wxT("%x"), offset));
		SetItemData(row, pos);
		row++;
	}
}

/// トラックリストをクリア
void L3DiskRawTrack::ClearTracks()
{
	DeleteAllItems();

	disk = NULL;
	side_number = -1;

	// セクタリストクリア
	parent->ClearSectorListData();
}

/// トラックリスト上のポップアップメニュー表示
void L3DiskRawTrack::ShowPopupMenu()
{
	if (!menuPopup) return;
	int row = 0;
	int num = 0;
	wxUint32 offset = 0;

	bool opened = (disk != NULL);
	menuPopup->Enable(IDM_EXPORT_TRACK, opened);
	menuPopup->Enable(IDM_IMPORT_TRACK, opened);

	menuPopup->Enable(IDM_MODIFY_ID_H_DISK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_N_DISK, opened);
	menuPopup->Enable(IDM_MODIFY_DENSITY_DISK, opened);

	menuPopup->Enable(IDM_PROPERTY_TRACK, opened);

	if (opened) {
		row = (int)GetFirstSelected();
		num = (int)GetItemData(row);
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

#ifdef __WXMSW__
		// macでは別プロセスで動くようなのでここで削除しない。
		ReleaseFileObject(tmp_dir_name);
#endif
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
	int selected_count = GetSelectedItemCount();
	if (selected_count <= 0) return false;

	L3DiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	if (!app->MakeTempDir(tmp_dir_name)) {
		return false;
	}

	int cnt = 0;
	long row = GetFirstSelected();
	do {
		DiskD88Track *track = GetTrack(row);
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
			track->GetTrackNumber(), track->GetSideNumber(), ed_sec
		);
		if (sts) {
			// ファイルリストに追加
			file_object.AddFile(file_path.GetFullPath());
			cnt++;
		}
	} while((row = GetNextSelected(row)) != wxNOT_FOUND);

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

	RawExpBox dlg(this, wxID_ANY, caption, disk, side_number, track->GetTrackNumber(), track->GetSideNumber(), sector->GetSectorNumber());
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
		, dlg.GetTrackNumber(1), dlg.GetSideNumber(1), dlg.GetSectorNumber(1));
	} else {
		return false;
	}
}

/// 指定したファイルにトラックデータをエクスポート
bool L3DiskRawTrack::ExportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec)
{
	frame->SetIniExportFilePath(path);

	if (!disk) return false;

	int trk = st_trk;
	int sid = st_sid;
	int sec = st_sec;
	int esid = side_number >= 0 ? side_number : (disk->GetSidesPerDisk() - 1);
	int esec = disk->GetSectorsPerTrack();

	wxFile outfile(path, wxFile::write);
	if (!outfile.IsOpened()) return false;

	for(;trk <= ed_trk; trk++) {
		if (trk == ed_trk) {
			// 最終トラックなら終了サイドまで
			esid = ed_sid;
		}
		for(;sid <= esid; sid++) {
			DiskD88Track *track = disk->GetTrack(trk, sid);
			if (track) {
				if (trk == ed_trk && sid == ed_sid) {
					// 最終トラック＆最終サイドなら終了セクタまで
					esec = ed_sec;
				}
				for(;sec <= esec; sec++) {
					DiskD88Sector *sector = track->GetSector(sec);
					if (sector) {
						size_t bufsize = sector->GetSectorSize();
						wxUint8 *buf = sector->GetSectorBuffer();

						outfile.Write((const void *)buf, bufsize);
					}
				}
				sec = 1;
			}
		}
		sid = side_number >= 0 ? side_number : 0;
	}

	return true;
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

	RawExpBox dlg(this, wxID_ANY, caption, disk, side_number, st_trk, st_sid, st_sec, ed_trk, ed_sid, ed_sec);
	int sts = dlg.ShowModal();
	if (sts != wxID_OK) {
		return false;
	}

	return ImportTrackDataFile(path
	, dlg.GetTrackNumber(0), dlg.GetSideNumber(0), dlg.GetSectorNumber(0)
	, dlg.GetTrackNumber(1), dlg.GetSideNumber(1), dlg.GetSectorNumber(1));
}

/// 指定したファイルから指定した範囲にトラックデータをインポート
/// @param [in] path ファイルパス
/// @param [in] st_trk 開始トラック番号
/// @param [in] st_sid 開始サイド番号
/// @param [in] st_sec 開始セクタ番号
/// @param [in] ed_trk 終了トラック番号
/// @param [in] ed_sid 終了サイド番号
/// @param [in] ed_sec 終了セクタ番号
/// @return true:成功 / false:エラー
bool L3DiskRawTrack::ImportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec)
{
	frame->SetIniExportFilePath(path);

	if (!disk) return false;

	int trk = st_trk;
	int sid = st_sid;
	int sec = st_sec;
	int esid = side_number >= 0 ? side_number : (disk->GetSidesPerDisk() - 1);
	int esec = disk->GetSectorsPerTrack();

	wxFile infile(path, wxFile::read);
	if (!infile.IsOpened()) return false;

	for(;trk <= ed_trk; trk++) {
		if (trk == ed_trk) {
			// 最終トラックなら終了サイドまで
			esid = ed_sid;
		}
		for(;sid <= esid; sid++) {
			DiskD88Track *track = disk->GetTrack(trk, sid);
			if (track) {
				if (trk == ed_trk && sid == ed_sid) {
					// 最終トラック＆最終サイドなら終了セクタまで
					esec = ed_sec;
				}
				for(;sec <= esec; sec++) {
					DiskD88Sector *sector = track->GetSector(sec);
					if (sector) {
						size_t bufsize = sector->GetSectorSize();
						wxUint8 *buf = sector->GetSectorBuffer();

						infile.Read((void *)buf, bufsize);

						if (infile.Eof()) {
							// ファイル終わり
							esec = 0;
							esid = 0;
							ed_trk = 0;
						}
					}
				}
				sec = 1;
			}
		}
		sid = side_number >= 0 ? side_number : 0;
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

	int row = (int)GetFirstSelected();
	if (row < 0) return;
	int num = (int)GetItemData(row);

	wxUint32 offset = disk->GetOffset(num);

	RawTrackBox dlg(this, wxID_ANY, row, offset, disk);
	dlg.ShowModal();
}

/// トラックを削除
void L3DiskRawTrack::DeleteTracks()
{
	if (!disk) return;

	int disk_number = -1;
	int side_number = -1;
	int row = (int)GetFirstSelected();
	if (row < 0) return;
	int num = (int)GetItemData(row);

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
		parent->RefreshAllData();
	}
}

/// 選択行のトラックを返す
DiskD88Track *L3DiskRawTrack::GetSelectedTrack()
{
	return GetTrack(GetFirstSelected());
}

/// 指定行のトラックを返す
DiskD88Track *L3DiskRawTrack::GetTrack(long row)
{
	if (!disk) return NULL;

	DiskD88Track *track = NULL;
	if (row == wxNOT_FOUND) return NULL;
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

//
//
//
L3DiskRawSectorItem::L3DiskRawSectorItem(const wxString &newname, const wxString &newattr, int newsize, int newgrps)
{
	filename = newname;
	attribute = newattr;
	size = newsize;
	groups = newgrps;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(L3DiskRawSectorItems);

//
//
//
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
bool L3DiskRawListStoreDerivedModel::IsEnabledByRow(unsigned int row, unsigned int col) const
{
    return true;
}
#endif

//
// 右パネルのセクタリスト
//
// Attach Event
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
wxBEGIN_EVENT_TABLE(L3DiskRawSector, wxDataViewListCtrl)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, L3DiskRawSector::OnItemContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, L3DiskRawSector::OnItemActivated)
	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, L3DiskRawSector::OnSelectionChanged)
	EVT_DATAVIEW_ITEM_BEGIN_DRAG(wxID_ANY, L3DiskRawSector::OnBeginDrag)
#else
wxBEGIN_EVENT_TABLE(L3DiskRawSector, wxListCtrl)
	EVT_CONTEXT_MENU(L3DiskRawSector::OnContextMenu)
	EVT_LIST_ITEM_ACTIVATED(wxID_ANY, L3DiskRawSector::OnItemActivated)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, L3DiskRawSector::OnSelectionChanged)
	EVT_LIST_BEGIN_DRAG(wxID_ANY, L3DiskRawSector::OnBeginDrag)
#endif
	EVT_CHAR(L3DiskRawSector::OnChar)

	EVT_MENU(IDM_EXPORT_FILE, L3DiskRawSector::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, L3DiskRawSector::OnImportFile)

	EVT_MENU(IDM_MODIFY_ID_C_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_H_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_N_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_DENSITY_TRACK, L3DiskRawSector::OnModifyDensityOnTrack)
	EVT_MENU(IDM_MODIFY_SECTORS_TRACK, L3DiskRawSector::OnModifySectorsOnTrack)
	EVT_MENU(IDM_MODIFY_SIZE_TRACK, L3DiskRawSector::OnModifySectorSizeOnTrack)
	EVT_MENU(IDM_APPEND_SECTOR, L3DiskRawSector::OnAppendSector)
	EVT_MENU(IDM_DELETE_SECTOR, L3DiskRawSector::OnDeleteSector)
	EVT_MENU(IDM_DELETE_SECTORS_BELOW, L3DiskRawSector::OnDeleteSectorsOnTrack)

	EVT_MENU(IDM_PROPERTY_SECTOR, L3DiskRawSector::OnPropertySector)
wxEND_EVENT_TABLE()

L3DiskRawSector::L3DiskRawSector(L3DiskFrame *parentframe, L3DiskRawPanel *parentwindow)
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	: wxDataViewListCtrl(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_ROW_LINES | wxDV_MULTIPLE)
#else
	: wxListCtrl(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT)
#endif
{
	initialized = false;
	parent   = parentwindow;
	frame    = parentframe;

	track = NULL;

#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	L3DiskRawListStoreDerivedModel *model = new L3DiskRawListStoreDerivedModel();
	AssociateModel(model);
	model->DecRef();

	AppendTextColumn(_("C"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT);
	AppendTextColumn(_("H"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT);
	AppendTextColumn(_("R"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT);
	AppendTextColumn(_("N"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT);
//	AppendToggleColumn(_("Deleted"), wxDATAVIEW_CELL_ACTIVATABLE, 32);
//	AppendToggleColumn(_("SingleDensity"), wxDATAVIEW_CELL_ACTIVATABLE, 32);
	AppendTextColumn(_("Deleted"), wxDATAVIEW_CELL_INERT, 36, wxALIGN_CENTER);
	AppendTextColumn(_("SingleDensity"), wxDATAVIEW_CELL_INERT, 36, wxALIGN_CENTER);
	AppendTextColumn(_("NumOfSectors"), wxDATAVIEW_CELL_INERT, 72, wxALIGN_RIGHT);
	AppendTextColumn(_("Size"), wxDATAVIEW_CELL_INERT, 72, wxALIGN_RIGHT);
	AppendTextColumn(wxT(""), wxDATAVIEW_CELL_INERT, 4);
#else
	AppendColumn(_("Num"), wxLIST_FORMAT_RIGHT, 40);
	AppendColumn(_("C"), wxLIST_FORMAT_RIGHT, 40);
	AppendColumn(_("H"), wxLIST_FORMAT_RIGHT, 40);
	AppendColumn(_("R"), wxLIST_FORMAT_RIGHT, 40);
	AppendColumn(_("N"), wxLIST_FORMAT_RIGHT, 40);
	AppendColumn(_("Deleted"), wxLIST_FORMAT_CENTER, 36);
	AppendColumn(_("SingleDensity"), wxLIST_FORMAT_CENTER, 36);
	AppendColumn(_("NumOfSectors"), wxLIST_FORMAT_RIGHT, 72);
	AppendColumn(_("Size"), wxLIST_FORMAT_RIGHT, 72);
#endif

	wxFont font;
	frame->GetDefaultListFont(font);
	SetFont(font);

	// popup menu
	menuPopup = new wxMenu;
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
	menuPopup->Append(IDM_PROPERTY_SECTOR, _("&Property"));

	initialized = true;
}

L3DiskRawSector::~L3DiskRawSector()
{
	delete menuPopup;
}


/// セクタリスト選択
void L3DiskRawSector::OnSelectionChanged(L3SectorListEvent& event)
{
	if (!initialized) return;

	if (GetListSelectedRow() == wxNOT_FOUND || !track) {
		// 非選択
		UnselectItem();
		return;
	}

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) {
		// 非選択
		UnselectItem();
		return;
	}

	DiskD88Sector *sector = sectors->Item(GetListSelectedRow());
	if (!sector) {
		// 非選択
		UnselectItem();
		return;
	}

	// 選択
	SelectItem(sector);
}

/// セクタリストからドラッグ開始
void L3DiskRawSector::OnBeginDrag(L3SectorListEvent& event)
{
	DragDataSourceForExternal();
}

/// セクタリスト右クリック
void L3DiskRawSector::OnItemContextMenu(L3SectorListEvent& event)
{
	ShowPopupMenu();
}

/// 右クリック
void L3DiskRawSector::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// セクタリスト ダブルクリック
void L3DiskRawSector::OnItemActivated(L3SectorListEvent& event)
{
	ShowSectorAttr();
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

	bool opened = (track != NULL);
	menuPopup->Enable(IDM_IMPORT_FILE, opened);
	menuPopup->Enable(IDM_MODIFY_ID_C_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_H_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_N_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_DENSITY_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_SECTORS_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_SIZE_TRACK, opened);
	menuPopup->Enable(IDM_APPEND_SECTOR, opened);

	int cnt = GetListSelectedCount();
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

/// セクタリストをリフレッシュ
void L3DiskRawSector::RefreshSectors()
{
	DeleteAllItems();

	if (!track) return;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return;

#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	wxVector<wxVariant> data;
#else
	long row = 0;
#endif
	for (size_t i=0; i<sectors->Count(); i++) {
		DiskD88Sector *sector = sectors->Item(i);
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
		data.clear();
		data.push_back(wxString::Format(wxT("%d"),sector->GetIDC()));
		data.push_back(wxString::Format(wxT("%d"),sector->GetIDH()));
		data.push_back(wxString::Format(wxT("%d"),sector->GetIDR()));
		data.push_back(wxString::Format(wxT("%d"),sector->GetIDN()));
//		data.push_back(sector->GetDeletedMark());
//		data.push_back(sector->IsSingleDensity());
		data.push_back(sector->IsDeleted() ? wxT("*") : wxEmptyString);
		data.push_back(sector->IsSingleDensity() ? wxT("*") : wxEmptyString);
		data.push_back(wxString::Format(wxT("%d"),sector->GetSectorsPerTrack()));
		data.push_back(wxString::Format(wxT("%d"),sector->GetSectorBufferSize()));
		data.push_back(wxT(""));

		AppendItem( data );
#else
		int col = 1;
		InsertItem(row, wxString::Format(wxT("%d"), (int)row));
		SetItem(row, col++, wxString::Format(wxT("%d"),sector->GetIDC()));
		SetItem(row, col++, wxString::Format(wxT("%d"),sector->GetIDH()));
		SetItem(row, col++, wxString::Format(wxT("%d"),sector->GetIDR()));
		SetItem(row, col++, wxString::Format(wxT("%d"),sector->GetIDN()));
		SetItem(row, col++, sector->IsDeleted() ? wxT("*") : wxEmptyString);
		SetItem(row, col++, sector->IsSingleDensity() ? wxT("*") : wxEmptyString);
		SetItem(row, col++, wxString::Format(wxT("%d"),sector->GetSectorsPerTrack()));
		SetItem(row, col++, wxString::Format(wxT("%d"),sector->GetSectorBufferSize()));
		SetItemPtrData(row, (wxUIntPtr)i);

		row++;
#endif
	}

	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// セクタリストをクリア
void L3DiskRawSector::ClearSectors()
{
	DeleteAllItems();

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

	int row = GetListSelectedRow();
	if (row == wxNOT_FOUND) return NULL;

	if (pos) *pos = row;

	DiskD88Sector *sector = track->GetSectorByIndex(row);
	return sector;
}

/// セクタを返す
DiskD88Sector *L3DiskRawSector::GetSector(const L3SectorListItem &item)
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
	L3SectorListItems selected_items;
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
	L3SectorListItems selected_items;
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
	frame->SetIniExportFilePath(path);

	if (!sector) return false;

	size_t bufsize = sector->GetSectorBufferSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	if (buf == NULL || bufsize <= 0) return false;

	wxFile outfile(path, wxFile::write);
	if (!outfile.IsOpened()) return false;

	outfile.Write((const void *)buf, bufsize);

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
		, sector->IsDeleted(), sector->IsSingleDensity()
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
		, sector->IsDeleted(), sector->IsSingleDensity()
		, SECTORBOX_HIDE_SECTOR_NUMS
	);
	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		track->AddNewSector(
			dlg.GetIdC(),
			dlg.GetIdH(),
			dlg.GetIdR(),
			DiskD88Sector::ConvIDNToSecSize(dlg.GetIdN()),
			sector->IsSingleDensity()
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

/// 選択行を返す
int L3DiskRawSector::GetListSelectedRow() const
{
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	return GetSelectedRow();
#else
	int row = GetSelectedItemCount();
	return row == 1 ? (int)GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED) : wxNOT_FOUND;
#endif
}

/// 選択行数を返す
int L3DiskRawSector::GetListSelectedCount() const
{
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	return GetSelectedItemsCount();
#else
	return GetSelectedItemCount();
#endif
}

/// 選択行を配列で返す
int L3DiskRawSector::GetListSelections(L3SectorListItems &arr) const
{
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	return GetSelections(arr);
#else
	long item = -1;
	while((item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED)) >= 0) {
		arr.Add(item);
	}
	return (int)arr.Count();
#endif
}
