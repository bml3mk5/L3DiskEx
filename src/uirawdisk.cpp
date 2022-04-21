/// @file uirawdisk.cpp
///
/// @brief ディスクID一覧
///

#include "main.h"
#include "uirawdisk.h"
#include <wx/regex.h>
#include "sectorbox.h"
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
                                   wxSP_3D | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
	parent = parentwindow;
    frame = parentframe;

	// fit size on parent window
    SetSize(parentwindow->GetClientSize());

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	// control panel
	lpanel = new L3DiskRawTrack(frame, this);
	rpanel = new L3DiskRawSector(frame, this);
	SplitVertically(lpanel, rpanel, 170);

	SetMinimumPaneSize(10);
}

void L3DiskRawPanel::SetTrackListData(DiskD88Disk *disk, int side_num)
{
	lpanel->SetData(disk, side_num);
	frame->UpdateMenuAndToolBarRawDisk(this);
}

void L3DiskRawPanel::ClearTrackListData()
{
	lpanel->ClearData();
	frame->UpdateMenuAndToolBarRawDisk(this);
}
bool L3DiskRawPanel::TrackListExists() const
{
	return (lpanel->GetDisk() != NULL);
}
int L3DiskRawPanel::GetTrackListSelectedRow() const
{
	return (int)lpanel->GetFirstSelected();
}

void L3DiskRawPanel::SetSectorListData(DiskD88Track *track)
{
	rpanel->SetSectors(track);
	frame->UpdateMenuAndToolBarRawDisk(this);
}
void L3DiskRawPanel::ClearSectorListData()
{
	rpanel->ClearSectors();
	frame->UpdateMenuAndToolBarRawDisk(this);
}
int L3DiskRawPanel::GetSectorListSelectedRow() const
{
	return rpanel->GetSelectedRow();
}

bool L3DiskRawPanel::ShowExportTrackDialog()
{
	return lpanel->ShowExportTrackDialog();
}

bool L3DiskRawPanel::ShowImportTrackDialog()
{
	return lpanel->ShowImportTrackDialog();
}

bool L3DiskRawPanel::ShowExportDataFileDialog()
{
	return rpanel->ShowExportDataFileDialog();
}

bool L3DiskRawPanel::ShowImportDataFileDialog()
{
	return rpanel->ShowImportDataFileDialog();
}

bool L3DiskRawPanel::ShowSectorAttr()
{
	return rpanel->ShowSectorAttr();
}

void L3DiskRawPanel::ModifyIDonTrack(int type_num)
{
	rpanel->ModifyIDonTrack(type_num);
}

void L3DiskRawPanel::ModifyDensityOnTrack()
{
	rpanel->ModifyDensityOnTrack();
}

//
// 左パネルのトラックリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRawTrack, wxListView)
	EVT_LIST_ITEM_SELECTED(wxID_ANY, L3DiskRawTrack::OnListItemSelected)
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
wxEND_EVENT_TABLE()

L3DiskRawTrack::L3DiskRawTrack(L3DiskFrame *parentframe, L3DiskRawPanel *parentwindow)
       : wxListView(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL)
{
	parent   = parentwindow;
	frame    = parentframe;

	disk = NULL;
	side_number = -1;

	AppendColumn(_("Num"), wxLIST_FORMAT_LEFT, 38);
	AppendColumn(_("Track"), wxLIST_FORMAT_LEFT, 28);
	AppendColumn(_("Side"), wxLIST_FORMAT_LEFT, 28);
	AppendColumn(_("Offset"), wxLIST_FORMAT_RIGHT, 60);

	// popup menu
	menuPopup = new wxMenu;
	menuPopup->Append(IDM_EXPORT_TRACK, _("&Export Track..."));
	menuPopup->Append(IDM_IMPORT_TRACK, _("&Import To Track..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MODIFY_ID_H_DISK, _("Modify All H On This Disk"));
	menuPopup->Append(IDM_MODIFY_ID_N_DISK, _("Modify All N On This Disk"));
	menuPopup->Append(IDM_MODIFY_DENSITY_DISK, _("Modify All Density On This Disk"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MODIFY_ID_C_TRACK, _("Modify All C On This Track"));
	menuPopup->Append(IDM_MODIFY_ID_H_TRACK, _("Modify All H On This Track"));
	menuPopup->Append(IDM_MODIFY_ID_N_TRACK, _("Modify All N On This Track"));
	menuPopup->Append(IDM_MODIFY_DENSITY_TRACK, _("Modify All Density On This Track"));
}

L3DiskRawTrack::~L3DiskRawTrack()
{
	delete menuPopup;
}

/// 選択
void L3DiskRawTrack::OnListItemSelected(wxListEvent& event)
{
	if (!disk) return;

	int row = (int)event.GetIndex();
	int num = (int)GetItemData(row);
	wxUint32 offset = disk->GetOffset(num);
	if (offset > 0) {
		parent->SetSectorListData(disk->GetTrackByOffset(offset));
	} else {
		parent->ClearSectorListData();
	}
}

/// 右クリック
void L3DiskRawTrack::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// トラックをエクスポート
void L3DiskRawTrack::OnExportTrack(wxCommandEvent& event)
{
	ShowExportTrackDialog();
}

/// トラックにインポート
void L3DiskRawTrack::OnImportTrack(wxCommandEvent& event)
{
	ShowImportTrackDialog();
}

/// ID一括変更
void L3DiskRawTrack::OnModifyIDonDisk(wxCommandEvent& event)
{
	ModifyIDonDisk(event.GetId() - IDM_MODIFY_ID_C_DISK);
}

/// ID一括変更
void L3DiskRawTrack::OnModifyIDonTrack(wxCommandEvent& event)
{
	parent->ModifyIDonTrack(event.GetId() - IDM_MODIFY_ID_C_TRACK);
}

/// 密度一括変更
void L3DiskRawTrack::OnModifyDensityOnDisk(wxCommandEvent& event)
{
	ModifyDensityOnDisk();
}

/// 密度一括変更
void L3DiskRawTrack::OnModifyDensityOnTrack(wxCommandEvent& event)
{
	parent->ModifyDensityOnTrack();
}

/// トラックリストをセット
void L3DiskRawTrack::SetData(DiskD88Disk *newdisk, int newsidenum)
{
	if (!newdisk) return;

	disk = newdisk;
	side_number = newsidenum;

	DeleteAllItems();

	for(int num=0, row=0; num < DISKD88_MAX_TRACKS; num++) {
		wxUint32 offset = disk->GetOffset(num);
		DiskD88Track *trk = disk->GetTrackByOffset(offset);
		int trk_num = -1;
		int sid_num = -1;
		if (newsidenum >= 0) {
			if (!trk || newsidenum != trk->GetSideNumber()) continue;
		}
		if (trk) {
			trk_num = trk->GetTrackNumber();
			sid_num = trk->GetSideNumber();
		}
		InsertItem(row, wxString::Format(wxT("%d"), num));
		SetItem(row, 1, trk_num >= 0 ? wxString::Format(wxT("%d"), trk_num) : wxT("--"));
		SetItem(row, 2, sid_num >= 0 ? wxString::Format(wxT("%d"), sid_num) : wxT("--"));
		SetItem(row, 3, wxString::Format(wxT("%x"), offset));
		SetItemData(row, num);
		row++;
	}

	// セクタリストはクリア
	parent->ClearSectorListData();
}

void L3DiskRawTrack::ClearData()
{
	DeleteAllItems();

	disk = NULL;
	side_number = -1;

	// セクタリストクリア
	parent->ClearSectorListData();
}

/// ポップアップメニュー表示
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

	PopupMenu(menuPopup);
}

/// トラックをエクスポート
bool L3DiskRawTrack::ShowExportTrackDialog()
{
	if (!disk) return false;

	DiskD88Track *track = GetSelectedTrack();
	DiskD88Sector *sector = NULL;
	for(int num=0; (track == NULL || sector == NULL) && num < DISKD88_MAX_TRACKS; num++) {
		if (!track) track = disk->GetTrack(num, side_number >= 0 ? side_number : 0);
		if (track) sector = track->GetSector(1);
		if (!sector) track = NULL;
	}
	if (!sector) return false;

	wxString caption = _("Export data from track");

	RawExpBox dlg(this, wxID_ANY, caption, disk, side_number, track->GetTrackNumber(), track->GetSideNumber(), sector->GetSectorNumber());
	int sts = dlg.ShowModal();
	if (sts != wxID_OK) return false;

	wxString filename = wxString::Format(wxT("%02d-%02d-%02d--%02d-%02d-%02d.bin")
		, dlg.GetTrackNumber(0), dlg.GetSideNumber(0), dlg.GetSectorNumber(0)
		, dlg.GetTrackNumber(1), dlg.GetSideNumber(1), dlg.GetSectorNumber(1));

	L3DiskFileDialog fdlg(
		caption,
		frame->GetRecentPath(),
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
	frame->SetFilePath(path);

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

/// トラックにインポート
bool L3DiskRawTrack::ShowImportTrackDialog()
{
	if (!disk) return false;

	DiskD88Track *track = GetSelectedTrack();
	DiskD88Sector *sector = NULL;
	for(int num=0; (track == NULL || sector == NULL) && num < DISKD88_MAX_TRACKS; num++) {
		if (!track) track = disk->GetTrack(num, side_number >= 0 ? side_number : 0);
		if (track) sector = track->GetSector(1);
		if (!sector) track = NULL;
	}
	if (!sector) return false;

	wxString caption = _("Import data to track");

	L3DiskFileDialog fdlg(
		caption,
		frame->GetRecentPath(),
		wxEmptyString,
		_("All files (*.*)|*.*"),
		wxFD_OPEN);
	int sts = fdlg.ShowModal();
	if (sts != wxID_OK) return false;

	wxString path = fdlg.GetPath();

	int start_track = track->GetTrackNumber();
	int start_side = track->GetSideNumber();
	int start_sector = sector->GetSectorNumber();
	int end_track = start_track;
	int end_side = start_side;
	int end_sector = start_sector;

	wxRegEx re("([0-9][0-9])-([0-9][0-9])-([0-9][0-9])--([0-9][0-9])-([0-9][0-9])-([0-9][0-9])");
	if (re.Matches(path)) {
		wxString sval;
		long lval;
		sval = re.GetMatch(path, 1); sval.ToLong(&lval);
		start_track = (int)lval;
		sval = re.GetMatch(path, 2); sval.ToLong(&lval);
		start_side = (int)lval;
		sval = re.GetMatch(path, 3); sval.ToLong(&lval);
		start_sector = (int)lval;
		sval = re.GetMatch(path, 4); sval.ToLong(&lval);
		end_track = (int)lval;
		sval = re.GetMatch(path, 5); sval.ToLong(&lval);
		end_side = (int)lval;
		sval = re.GetMatch(path, 6); sval.ToLong(&lval);
		end_sector = (int)lval;
	}


	RawExpBox dlg(this, wxID_ANY, caption, disk, side_number, start_track, start_side, start_sector, end_track, end_side, end_sector);
	sts = dlg.ShowModal();

	if (sts == wxID_OK) {
		return ImportTrackDataFile(path
		, dlg.GetTrackNumber(0), dlg.GetSideNumber(0), dlg.GetSectorNumber(0)
		, dlg.GetTrackNumber(1), dlg.GetSideNumber(1), dlg.GetSectorNumber(1));
	} else {
		return false;
	}
}

/// 指定したファイルからトラックデータをインポート
bool L3DiskRawTrack::ImportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec)
{
	frame->SetFilePath(path);

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
void L3DiskRawTrack::ModifyIDonDisk(int type_num)
{
	if (!disk) return;

	DiskD88Track *track = NULL;
	DiskD88Sector *sector = NULL;
	for(int num=0; (track == NULL || sector == NULL) && num < DISKD88_MAX_TRACKS; num++) {
		track = disk->GetTrack(num, side_number >= 0 ? side_number : 0);
		if (track) sector = track->GetSector(1);
	}
	if (!sector) return;

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

/// 密度を一括変更
void L3DiskRawTrack::ModifyDensityOnDisk()
{
	if (!disk) return;

	DiskD88Track *track = NULL;
	DiskD88Sector *sector = NULL;
	for(int num=0; (track == NULL || sector == NULL) && num < DISKD88_MAX_TRACKS; num++) {
		track = disk->GetTrack(num, side_number >= 0 ? side_number : 0);
		if (track) sector = track->GetSector(1);
	}
	if (!sector) return;

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

/// 選択行のトラック
DiskD88Track *L3DiskRawTrack::GetSelectedTrack()
{
	if (!disk) return NULL;

	DiskD88Track *track = NULL;
	int row = (int)GetFirstSelected();
	if (row == wxNOT_FOUND) return NULL;
	int num = (int)GetItemData(row);
	wxUint32 offset = disk->GetOffset(num);
	if (offset > 0) {
		track = disk->GetTrackByOffset(offset);
	}
	return track;
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
bool L3DiskRawListStoreDerivedModel::IsEnabledByRow(unsigned int row, unsigned int col) const
{
    return true;
}

//
// 右パネルのセクタリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRawSector, wxDataViewListCtrl)
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, L3DiskRawSector::OnDataViewItemContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(wxID_ANY, L3DiskRawSector::OnDataViewItemActivated)
	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, L3DiskRawSector::OnSelectionChanged)

	EVT_MENU(IDM_EXPORT_FILE, L3DiskRawSector::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, L3DiskRawSector::OnImportFile)

	EVT_MENU(IDM_MODIFY_ID_C_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_H_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_ID_N_TRACK, L3DiskRawSector::OnModifyIDonTrack)
	EVT_MENU(IDM_MODIFY_DENSITY_TRACK, L3DiskRawSector::OnModifyDensityOnTrack)

	EVT_MENU(IDM_PROPERTY_SECTOR, L3DiskRawSector::OnPropertySector)

wxEND_EVENT_TABLE()

L3DiskRawSector::L3DiskRawSector(L3DiskFrame *parentframe, L3DiskRawPanel *parentwindow)
       : wxDataViewListCtrl(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	initialized = false;
	parent   = parentwindow;
	frame    = parentframe;

	track = NULL;

	L3DiskRawListStoreDerivedModel *model = new L3DiskRawListStoreDerivedModel();
	AssociateModel(model);
	model->DecRef();

	AppendTextColumn(_("C"), wxDATAVIEW_CELL_INERT, 28);
	AppendTextColumn(_("H"), wxDATAVIEW_CELL_INERT, 28);
	AppendTextColumn(_("R"), wxDATAVIEW_CELL_INERT, 28);
	AppendTextColumn(_("N"), wxDATAVIEW_CELL_INERT, 28);
//	AppendToggleColumn(_("Deleted"), wxDATAVIEW_CELL_ACTIVATABLE, 32);
//	AppendToggleColumn(_("SingleDensity"), wxDATAVIEW_CELL_ACTIVATABLE, 32);
	AppendTextColumn(_("Deleted"), wxDATAVIEW_CELL_INERT, 32, wxALIGN_CENTER);
	AppendTextColumn(_("SingleDensity"), wxDATAVIEW_CELL_INERT, 32, wxALIGN_CENTER);
	AppendTextColumn(_("NumOfSectors"), wxDATAVIEW_CELL_INERT, 48, wxALIGN_RIGHT);
	AppendTextColumn(_("Size"), wxDATAVIEW_CELL_INERT, 48, wxALIGN_RIGHT);

	// popup menu
	menuPopup = new wxMenu;
	menuPopup->Append(IDM_EXPORT_FILE, _("&Export Sector..."));
	menuPopup->Append(IDM_IMPORT_FILE, _("&Import To Sector..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_MODIFY_ID_C_TRACK, _("Modify All C On This Track"));
	menuPopup->Append(IDM_MODIFY_ID_H_TRACK, _("Modify All H On This Track"));
	menuPopup->Append(IDM_MODIFY_ID_N_TRACK, _("Modify All N On This Track"));
	menuPopup->Append(IDM_MODIFY_DENSITY_TRACK, _("Modify All Density On This Track"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY_SECTOR, _("&Property"));

	initialized = true;
}

L3DiskRawSector::~L3DiskRawSector()
{
	delete menuPopup;
}


/// 選択
void L3DiskRawSector::OnSelectionChanged(wxDataViewEvent& event)
{
	if (!initialized) return;

	if (GetSelectedRow() == wxNOT_FOUND || !track) {
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

	DiskD88Sector *sector = sectors->Item(GetSelectedRow());
	if (!sector) {
		// 非選択
		UnselectItem();
		return;
	}

	// 選択
	SelectItem(sector);
}

/// 右クリック
void L3DiskRawSector::OnDataViewItemContextMenu(wxDataViewEvent& event)
{
	ShowPopupMenu();
}

/// ダブルクリック
void L3DiskRawSector::OnDataViewItemActivated(wxDataViewEvent& event)
{
	ShowSectorAttr();
}

/// エクスポート
void L3DiskRawSector::OnExportFile(wxCommandEvent& event)
{
	ShowExportDataFileDialog();
}

/// インポート
void L3DiskRawSector::OnImportFile(wxCommandEvent& event)
{
	ShowImportDataFileDialog();
}

/// 一括変更
void L3DiskRawSector::OnModifyIDonTrack(wxCommandEvent& event)
{
	ModifyIDonTrack(event.GetId() - IDM_MODIFY_ID_C_TRACK);
}

/// 密度一括変更
void L3DiskRawSector::OnModifyDensityOnTrack(wxCommandEvent& event)
{
	ModifyDensityOnTrack();
}

/// プロパティ
void L3DiskRawSector::OnPropertySector(wxCommandEvent& event)
{
	ShowSectorAttr();
}

/// ポップアップメニュー表示
void L3DiskRawSector::ShowPopupMenu()
{
	if (!menuPopup) return;

	bool opened = (track != NULL);
	menuPopup->Enable(IDM_MODIFY_ID_C_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_H_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_ID_N_TRACK, opened);
	menuPopup->Enable(IDM_MODIFY_DENSITY_TRACK, opened);

	opened = (opened && GetSelectedRow() != wxNOT_FOUND);
	menuPopup->Enable(IDM_EXPORT_FILE, opened);
	menuPopup->Enable(IDM_IMPORT_FILE, opened);
	menuPopup->Enable(IDM_PROPERTY_SECTOR, opened);

	PopupMenu(menuPopup);
}


// 選択
void L3DiskRawSector::SelectItem(DiskD88Sector *sector)
{
	// ダンプリストをセット
	frame->SetBinDumpData(sector->GetSectorBuffer(), sector->GetSectorSize());

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);

}

// 非選択
void L3DiskRawSector::UnselectItem()
{
	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// セクタをリストにセット
void L3DiskRawSector::SetSectors(DiskD88Track *newtrack)
{
	track = newtrack;

	RefreshSectors();
}

// リフレッシュ
void L3DiskRawSector::RefreshSectors()
{
	if (!track) return;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return;

	DeleteAllItems();

	wxVector<wxVariant> data;
	for (size_t i=0; i<sectors->Count(); i++) {
		DiskD88Sector *sector = sectors->Item(i);
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

		AppendItem( data );
	}

	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// セクタをクリア
void L3DiskRawSector::ClearSectors()
{
	DeleteAllItems();

	track = NULL;

	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarRawDisk(parent);
}

/// エクスポートダイアログ
bool L3DiskRawSector::ShowExportDataFileDialog()
{
	if (!track) return false;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return false;

	int row = GetSelectedRow();
	if (row == wxNOT_FOUND) return false;

	DiskD88Sector *sector = sectors->Item(row);
	if (!sector) return false;

	wxString filename = wxString::Format(wxT("%02d-%02d-%02d--%02d-%02d-%02d.bin")
		, sector->GetIDC(), sector->GetIDH(), sector->GetIDR()
		, sector->GetIDC(), sector->GetIDH(), sector->GetIDR()
	);

	L3DiskFileDialog dlg(
		_("Export data from sector"),
		frame->GetRecentPath(),
		filename,
		_("All files (*.*)|*.*"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		return ExportDataFile(path);
	} else {
		return false;
	}
}

/// 指定したファイルにセクタのデータをエクスポート
bool L3DiskRawSector::ExportDataFile(const wxString &path)
{
	frame->SetFilePath(path);

	if (!track) return false;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return false;

	int row = GetSelectedRow();
	if (row == wxNOT_FOUND) return false;

	DiskD88Sector *sector = sectors->Item(row);
	if (!sector) return false;

	size_t bufsize = sector->GetSectorBufferSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	if (buf == NULL || bufsize <= 0) return false;

	wxFile outfile(path, wxFile::write);
	if (!outfile.IsOpened()) return false;

	outfile.Write((const void *)buf, bufsize);

	return true;
}

/// インポートダイアログ
bool L3DiskRawSector::ShowImportDataFileDialog()
{
	L3DiskFileDialog dlg(
		_("Import data to sector"),
		frame->GetRecentPath(),
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

/// 指定したファイルからセクタにデータをインポート
bool L3DiskRawSector::ImportDataFile(const wxString &path)
{
	frame->SetFilePath(path);

	if (!track) return false;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return false;

	int row = GetSelectedRow();
	if (row == wxNOT_FOUND) return false;

	DiskD88Sector *sector = sectors->Item(row);
	if (!sector) return false;

	size_t bufsize = sector->GetSectorBufferSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	if (buf == NULL || bufsize <= 0) return false;

	wxFile infile(path, wxFile::read);
	if (!infile.IsOpened()) return false;

	infile.Read((void *)buf, bufsize);

	sector->SetModify();

	return true;
}

/// セクタ情報ダイアログ表示
bool L3DiskRawSector::ShowSectorAttr()
{
	if (!track) return false;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return false;

	int row = GetSelectedRow();
	if (row == wxNOT_FOUND) return false;

	DiskD88Sector *sector = sectors->Item(row);
	if (!sector) return false;

	SectorBox dlg(this, wxID_ANY
		, sector->GetIDC(), sector->GetIDH(), sector->GetIDR(), sector->GetIDN()
		, sector->GetSectorsPerTrack()
		, sector->IsDeleted(), sector->IsSingleDensity()
	);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		sector->SetIDC((wxUint8)dlg.GetIdC());
		sector->SetIDH((wxUint8)dlg.GetIdH());
		sector->SetIDR((wxUint8)dlg.GetIdR());
		sector->SetIDN((wxUint8)dlg.GetIdN());
		sector->SetSectorsPerTrack((wxUint16)dlg.GetSectorNums());
		sector->SetDeletedMark(dlg.GetDeletedMark());
		sector->SetSingleDensity(dlg.GetSingleDensity());

		sector->SetModify();

		RefreshSectors();
	}
	return (rc == wxID_OK);
}

/// IDを一括変更
void L3DiskRawSector::ModifyIDonTrack(int type_num)
{
	if (!track) return;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return;

	DiskD88Sector *sector = sectors->Item(0);
	if (!sector) return;

	int value = 0;
	int maxvalue = 82;
	switch(type_num) {
		case 0:
			value = sector->GetIDC();
			break;
		case 1:
			value = sector->GetIDH();
			maxvalue = 1;
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
			
			switch(type_num) {
				case 0:
					track->SetAllIDC(newvalue);
					break;
				case 1:
					track->SetAllIDH(newvalue);
					break;
				case 3:
					track->SetAllIDN(newvalue);
					break;
			}

			// リストを更新
			RefreshSectors();
		}
	}
}

/// 密度を一括変更
void L3DiskRawSector::ModifyDensityOnTrack()
{
	if (!track) return;

	DiskD88Sectors *sectors = track->GetSectors();
	if (!sectors) return;

	DiskD88Sector *sector = sectors->Item(0);
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
