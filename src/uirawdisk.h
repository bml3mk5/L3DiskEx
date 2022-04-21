/// @file uirawdisk.h
///
/// @brief ディスクID一覧
///
#ifndef _UIRAWDISK_H_
#define _UIRAWDISK_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/dataview.h>
#include <wx/clntdata.h>
#include "diskd88.h"

class L3DiskFrame;
class L3DiskRawTrack;
class L3DiskRawSector;

/// 分割ウィンドウ
class L3DiskRawPanel : public wxSplitterWindow
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	L3DiskRawTrack *lpanel;
	L3DiskRawSector *rpanel;

public:
	L3DiskRawPanel(L3DiskFrame *parentframe, wxWindow *parentwindow);

	// event handlers

	L3DiskRawTrack *GetLPanel() { return lpanel; }
	L3DiskRawSector *GetRPanel() { return rpanel; }

	void SetTrackListData(DiskD88Disk *disk, int side_num);
	void ClearTrackListData();
	bool TrackListExists() const;
	int GetTrackListSelectedRow() const;
	void SetSectorListData(DiskD88Track *track);
	void ClearSectorListData();
	int GetSectorListSelectedRow() const;

	bool ShowExportTrackDialog();
	bool ShowImportTrackDialog();

	bool ShowExportDataFileDialog();
	bool ShowImportDataFileDialog();
	bool ShowSectorAttr();
	void ModifyIDonTrack(int type_num);
	void ModifyDensityOnTrack();

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRawPanel);
};

/// 左パネルのトラックリスト
class L3DiskRawTrack : public wxListView
{
private:
	L3DiskRawPanel *parent;
	L3DiskFrame *frame;

	DiskD88Disk *disk;
	int side_number;

	wxMenu *menuPopup;

public:
	L3DiskRawTrack(L3DiskFrame *parentframe, L3DiskRawPanel *parentwindow);
	~L3DiskRawTrack();

	void OnListItemSelected(wxListEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnExportTrack(wxCommandEvent& event);
	void OnImportTrack(wxCommandEvent& event);
	void OnModifyIDonDisk(wxCommandEvent& event);
	void OnModifyIDonTrack(wxCommandEvent& event);
	void OnModifyDensityOnDisk(wxCommandEvent& event);
	void OnModifyDensityOnTrack(wxCommandEvent& event);

	void SetData(DiskD88Disk *newdisk, int newsidenum);
	void ClearData();
	void ShowPopupMenu();
	bool ShowExportTrackDialog();
	bool ExportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec);
	bool ShowImportTrackDialog();
	bool ImportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec);
	void ModifyIDonDisk(int type_num);
	void ModifyDensityOnDisk();

	DiskD88Disk *GetDisk() const { return disk; }
	DiskD88Track *GetSelectedTrack();

	enum {
		IDM_EXPORT_TRACK = 1,
		IDM_IMPORT_TRACK,
		IDM_MODIFY_ID_C_DISK,
		IDM_MODIFY_ID_H_DISK,
		IDM_MODIFY_ID_R_DISK,
		IDM_MODIFY_ID_N_DISK,
		IDM_MODIFY_DENSITY_DISK,
		IDM_MODIFY_ID_C_TRACK,
		IDM_MODIFY_ID_H_TRACK,
		IDM_MODIFY_ID_R_TRACK,
		IDM_MODIFY_ID_N_TRACK,
		IDM_MODIFY_DENSITY_TRACK,
	};

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRawTrack);
};


/// セクタリスト
class L3DiskRawSectorItem
{
public:
	wxString filename;
	wxString attribute;
	int size;
	int groups;
public:
	L3DiskRawSectorItem(const wxString &newname, const wxString &newattr, int newsize, int newgrps);
	~L3DiskRawSectorItem() {}
};

WX_DECLARE_OBJARRAY(L3DiskRawSectorItem, L3DiskRawSectorItems);

///
class L3DiskRawListStoreDerivedModel : public wxDataViewListStore
{
public:
    virtual bool IsEnabledByRow(unsigned int row, unsigned int col) const;
};
/// 右パネルのセクタリスト
class L3DiskRawSector : public wxDataViewListCtrl
{
private:
	L3DiskRawPanel *parent;
	L3DiskFrame *frame;

	wxMenu *menuPopup;

	DiskD88Track *track;

	wxDataViewItem selected_item;

public:
	L3DiskRawSector(L3DiskFrame *parentframe, L3DiskRawPanel *parent);
	~L3DiskRawSector();

	/// @name event procedures
	//@{
	void OnDataViewItemContextMenu(wxDataViewEvent& event);
	void OnDataViewItemActivated(wxDataViewEvent& event);
	void OnSelectionChanged(wxDataViewEvent& event);
	void OnExportFile(wxCommandEvent& event);
	void OnImportFile(wxCommandEvent& event);
	void OnModifyIDonTrack(wxCommandEvent& event);
	void OnModifyDensityOnTrack(wxCommandEvent& event);
	void OnPropertySector(wxCommandEvent& event);
	//@}

	void ShowPopupMenu();

	void SelectItem(DiskD88Sector *sector);
	void UnselectItem();

	void SetSectors(DiskD88Track *newtrack);
	void RefreshSectors();
	void ClearSectors();

	bool ShowExportDataFileDialog();
	bool ExportDataFile(const wxString &path);
	bool ShowImportDataFileDialog();
	bool ImportDataFile(const wxString &path);
	bool ShowSectorAttr();
	void ModifyIDonTrack(int type_num);
	void ModifyDensityOnTrack();

	enum {
		IDM_EXPORT_FILE = 1,
		IDM_IMPORT_FILE,
		IDM_MODIFY_ID_C_TRACK,
		IDM_MODIFY_ID_H_TRACK,
		IDM_MODIFY_ID_R_TRACK,
		IDM_MODIFY_ID_N_TRACK,
		IDM_MODIFY_DENSITY_TRACK,
		IDM_PROPERTY_SECTOR,
	};

	wxDECLARE_EVENT_TABLE();
};

#endif /* _UIRAWDISK_H_ */

