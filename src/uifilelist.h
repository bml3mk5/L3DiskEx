/// @file uifilelist.h
///
/// @brief BASICファイル名一覧
///
#ifndef _UIFILELIST_H_
#define _UIFILELIST_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dataview.h>
#include <wx/clntdata.h>
#include "basicfmt.h"

class L3DiskFrame;
class DiskD88Disk;
class IntNameBox;

#if 0
/// 
class L3DiskListItem
{
public:
	int file_type;
	int data_type;
public:
	L3DiskListItem(int newfiletype, int newdatatype);
	~L3DiskListItem() {}
};
#endif

///
class L3DiskFileListStoreModel : public wxDataViewListStore
{
private:
	L3DiskFrame *frame;

public:
	L3DiskFileListStoreModel(L3DiskFrame *parentframe);

	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
};

/// 右パネルのファイルリスト
class L3DiskFileList : public wxPanel
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	wxTextCtrl *text;
	wxDataViewListCtrl *list;

	wxMenu *menuPopup;

//	wxDataViewItem selected_item;

	// BASICフォーマットの情報
	DiskBasic basic;

	bool initialized;

	int ShowIntNameBoxAndCheckSameFile(IntNameBox &dlg);

public:
	L3DiskFileList(L3DiskFrame *parentframe, wxWindow *parent);
	~L3DiskFileList();

	/// @name event procedures
	//@{
	void OnSize(wxSizeEvent& event);
	void OnDataViewItemContextMenu(wxDataViewEvent& event);
	void OnDataViewItemActivated(wxDataViewEvent& event);
	void OnSelectionChanged(wxDataViewEvent& event);
	void OnFileNameEditedDone(wxDataViewEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnBeginDrag(wxDataViewEvent& event);
	void OnExportFile(wxCommandEvent& event);
	void OnImportFile(wxCommandEvent& event);
	void OnDeleteFile(wxCommandEvent& event);
	void OnRenameFile(wxCommandEvent& event);
	void OnProperty(wxCommandEvent& event);
	void OnChar(wxKeyEvent& event);
	//@}

	void ShowPopupMenu();

	void SetAttr(const wxString &val);
	void ClearAttr();

	void SetFiles(DiskD88Disk *newdisk, int newsidenum);
	void RefreshFiles();
	void ClearFiles();

	void SelectItem(DiskD88Sector *sector);
	void UnselectItem();

	bool ShowExportDataFileDialog();
	bool ExportDataFile(int dir_pos, const wxString &path);

	bool DragDataSource();

	bool ShowImportDataFileDialog();
	bool ImportDataFile(const wxString &path);
	bool ImportDataFile(const wxUint8 *buffer, size_t buflen);


	bool DeleteDataFile();

	void StartEditingFileName();
	bool RenameDataFile(wxDataViewItem listitem, const wxString &newname);

	bool ShowFileAttr();

	bool FormatDisk();

	int  GetSelectedRow() const;

	bool CanUseBasicDisk() const;
	bool IsFormattedBasicDisk() const;

	enum {
		IDM_EXPORT_FILE = 1,
		IDM_IMPORT_FILE,
		IDM_DELETE_FILE,
		IDM_RENAME_FILE,
		IDM_PROPERTY,
		IDC_TEXT_ATTR,
		IDC_VIEW_LIST,
	};

	wxDECLARE_EVENT_TABLE();
};

/// ディスクファイル ドラッグ＆ドロップ
class L3DiskFileListDropTarget : public wxDropTarget
{
	L3DiskFileList *parent;
    L3DiskFrame *frame;

public:
    L3DiskFileListDropTarget(L3DiskFrame *parentframe, L3DiskFileList *parentwindow);

	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
    bool OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames);
};


#endif /* _UIFILELIST_H_ */

