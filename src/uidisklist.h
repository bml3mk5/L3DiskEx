/// @file uidisklist.h
///
/// @brief ディスクリスト
///
#ifndef _UIDISKLIST_H_
#define _UIDISKLIST_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dataview.h>
#include <wx/clntdata.h>

class L3DiskFrame;
class DiskD88File;
class DiskD88Disk;

///
class L3DiskNameString
{
public:
	wxString      name;
	wxArrayString sides;
public:
	L3DiskNameString();
	L3DiskNameString(const wxString &newname);
	~L3DiskNameString() {}
};

WX_DECLARE_OBJARRAY(L3DiskNameString, L3DiskNameStrings);

/// ディスク情報に紐づける属性
class L3DiskPositionData : public wxClientData
{
private:
	int num;
	int subnum;
	bool editable;
public:
	L3DiskPositionData(int val1, int val2, bool edt) : wxClientData() { num = val1; subnum = val2; editable = edt; }

	void SetNumber(int val1, int val2, bool edt) { num = val1; subnum = val2; editable = edt; }
	int  GetNumber() { return num; }
	int  GetSubNumber() { return subnum; }
	bool GetEditable() { return editable; }
};

///
class L3DiskTreeStoreModel : public wxDataViewTreeStore
{
private:
	L3DiskFrame *frame;
public:
	L3DiskTreeStoreModel(L3DiskFrame *parentframe);

	virtual bool IsEnabled(const wxDataViewItem &item, unsigned int col) const;
	virtual bool SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col);
};

/// 左Panel ディスク情報
class L3DiskList: public wxDataViewTreeCtrl
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	wxMenu *menuPopup;

	DiskD88Disk *disk;
	bool selected_disk;

	wxDataViewItem SetSelectedItemAtDiskImage();

public:
	L3DiskList(L3DiskFrame *parentframe, wxWindow *parent);
	~L3DiskList();

	/// @name event procedures
	//@{
//	void OnSize(wxSizeEvent& event);
	void OnDataViewItemContextMenu(wxDataViewEvent& event);
	void OnSelectionChanged(wxDataViewEvent& event);
	void OnStartEditing(wxDataViewEvent& event);
	void OnEditingDone(wxDataViewEvent& event);

	void OnSaveDisk(wxCommandEvent& event);
	void OnAddNewDisk(wxCommandEvent& event);
	void OnAddDiskFromFile(wxCommandEvent& event);
	void OnReplaceDisk(wxCommandEvent& event);
	void OnDeleteDisk(wxCommandEvent& event);
	void OnRenameDisk(wxCommandEvent& event);
	void OnInitializeDisk(wxCommandEvent& event);
	void OnFormatDisk(wxCommandEvent& event);

	void OnPropertyDisk(wxCommandEvent& event);

	void OnChar(wxKeyEvent& event);
	//@}
	/// @name functions
	//@{
	void ShowPopupMenu();

	void ReSelect();
	void ChangeSelection(wxDataViewItem &item);
	void SetFileName();
	void SetFileName(const wxString &filename, L3DiskNameStrings &disknames);
	void ClearFileName();

	bool InitializeDisk();
	void ShowSaveDiskDialog();
//	bool SaveDisk(const wxString &path);
	bool DeleteDisk();
	void RenameDisk();
	void ShowDiskAttr();
	void SetDiskName(const wxString &val);
	//@}
	/// @name properties
	//@{
	int  GetSelectedDiskNumber();
	int  GetSelectedDiskSide();
	bool IsSelectedDiskImage();
	bool IsSelectedDisk();
	bool IsSelectedDiskSide();
	//@}

	enum {
		IDM_SAVE_DISK = 1,
		IDM_ADD_DISK_NEW,
		IDM_ADD_DISK_FROM_FILE,
		IDM_REPLACE_DISK_FROM_FILE,
		IDM_DELETE_DISK_FROM_FILE,
		IDM_RENAME_DISK,
		IDM_INITIALIZE_DISK,
		IDM_FORMAT_DISK,
		IDM_PROPERTY_DISK,
	};

	wxDECLARE_EVENT_TABLE();
};

#ifndef USE_DND_ON_TOP_PANEL
/// ディスクファイル ドラッグ＆ドロップ
class L3DiskListDropTarget : public wxFileDropTarget
{
	L3DiskList  *parent;
    L3DiskFrame *frame;

public:
    L3DiskListDropTarget(L3DiskFrame *parentframe, L3DiskList *parentwindow);
    bool OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames);
};
#endif

#endif /* _UIDISKLIST_H_ */

