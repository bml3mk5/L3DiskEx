/// @file main.h
///
/// @brief 本体
///
#ifndef _L3DISKEX_MAIN_H_
#define _L3DISKEX_MAIN_H_

/* #define USE_MENU_OPEN	*/

#include "common.h"
#include <wx/wx.h>
#include <wx/dynarray.h>
#include <wx/dnd.h>
#include <wx/fontdlg.h>
#include <wx/splitter.h>
#include "config.h"
#include "diskd88.h"
#include "basicfmt.h"
#include "uidisklist.h"
#include "uidiskattr.h"
#include "uifilelist.h"
#include "uirawdisk.h"
#include "uibindump.h"

// icon
extern const char *fd_5inch_16_1_xpm[];
extern const char *fd_5inch_16_2_xpm[];

class L3DiskApp;
class L3DiskFrame;
class L3DiskPanel;
class L3DiskList;
class L3DiskRPanel;
class L3DiskRBPanel;
class L3DiskFileDialog;
class L3DiskFileDropTarget;

class CharTypeBox;

class ConfigBox;

/// メインWindow
class L3DiskApp: public wxApp
{
private:
	wxString app_path;
	wxString ini_path;
	wxString res_path;
	wxLocale mLocale;
	Config   mConfig;

	L3DiskFrame *frame;
	wxString in_file;

	void SetAppPath();
public:
	L3DiskApp();
	bool OnInit();
	void OnInitCmdLine(wxCmdLineParser &parser);
	bool OnCmdLineParsed(wxCmdLineParser &parser);
	int  OnExit();
	void MacOpenFile(const wxString &fileName);
	const wxString &GetAppPath();
	const wxString &GetIniPath();
	const wxString &GetResPath();

	Config *GetConfig();
};

wxDECLARE_APP(L3DiskApp);

/// メインFrame
class L3DiskFrame: public wxFrame
{
private:
	// gui
	wxMenu *menuFile;
	wxMenu *menuRecentFiles;
	wxMenu *menuDisk;
	wxMenu *menuMode;
	wxMenu *menuHelp;

	L3DiskPanel *panel;

	Config *ini;

	//
	DiskD88 d88;

	void ChangeRPanel(int num);

	L3DiskPanel *GetPanel() { return panel; }

	L3DiskList *GetLPanel();
	void SetLPanelData();

	L3DiskRPanel *GetRPanel();

#ifdef USE_MENU_OPEN
	void UpdateMenuFile();
	void UpdateMenuDisk();
#endif

	void RecreateToolbar();
	void PopulateToolbar(wxToolBarBase* toolBar);

public:

    L3DiskFrame(const wxString& title, const wxSize& size);
	~L3DiskFrame();

	bool Init(const wxString &in_file);

	/// @name event procedures
	//@{
	void OnClose(wxCloseEvent& event);

	void OnQuit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	void OnCreateFile(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnOpenRecentFile(wxCommandEvent& event);
	void OnCloseFile(wxCommandEvent& event);

	void OnSaveAsFile(wxCommandEvent& event);
	void OnSaveDisk(wxCommandEvent& event);

	void OnAddNewDisk(wxCommandEvent& event);
	void OnAddDiskFromFile(wxCommandEvent& event);

	void OnReplaceDisk(wxCommandEvent& event);

	void OnDeleteDiskFromFile(wxCommandEvent& event);
	void OnRenameDisk(wxCommandEvent& event);

	void OnExportFileFromDisk(wxCommandEvent& event);
	void OnImportFileToDisk(wxCommandEvent& event);
	void OnDeleteFileFromDisk(wxCommandEvent& event);
	void OnRenameFileOnDisk(wxCommandEvent& event);
	void OnInitializeDisk(wxCommandEvent& event);
	void OnFormatDisk(wxCommandEvent& event);
	void OnPropertyOnDisk(wxCommandEvent& event);

	void OnBasicMode(wxCommandEvent& event);
	void OnRawDiskMode(wxCommandEvent& event);


	void OnConfigure(wxCommandEvent& event);

#ifdef USE_MENU_OPEN
	void OnMenuOpen(wxMenuEvent& event);
#endif
	//@}

	/// @name functions
	//@{
#ifndef USE_MENU_OPEN
	void UpdateMenuFile();
	void UpdateMenuDisk();
	void UpdateMenuDiskList(L3DiskList *list);
	void UpdateMenuFileList(L3DiskFileList *list);
	void UpdateMenuRawDisk(L3DiskRawPanel *rawpanel);
#endif
	void UpdateMenuRecentFiles();

	void UpdateToolBar();

	void UpdateToolBarDiskList(L3DiskList *list);
	void UpdateMenuAndToolBarDiskList(L3DiskList *list);

	void UpdateToolBarFileList(L3DiskFileList *list);
	void UpdateToolBarRawDisk(L3DiskRawPanel *rawpanel);

	void UpdateMenuAndToolBarFileList(L3DiskFileList *list);
	void UpdateMenuAndToolBarRawDisk(L3DiskRawPanel *rawpanel);

	void UpdateDataOnWindow();
	void UpdateDataOnWindow(const wxString &path);

	void ShowCreateFileDialog();
	void CreateDataFile(const wxString &diskname, const DiskParam &param, bool write_protect);
	void ShowAddNewDiskDialog();
	void AddNewDisk(const wxString &diskname, const DiskParam &param, bool write_protect);
	void ShowOpenFileDialog();
	void OpenDataFile(const wxString &path);
	void ShowAddFileDialog();
	void AddDiskFile(const wxString &path);
	bool CloseDataFile(bool force = false);
	void ShowSaveFileDialog();
	void SaveDataFile(const wxString &path);
	void ShowSaveDiskDialog(int disk_number, int side_number);
	void SaveDataDisk(int disk_number, int side_number, const wxString &path);
	void ShowReplaceDiskDialog(int disk_number, int side_number);
	void ReplaceDisk(int disk_number, int side_number, const wxString &path);
	void DeleteDisk();
	void RenameDisk();
	void ShowDiskAttr();
	void ExportFileFromDisk();
	void ImportFileToDisk();
	void DeleteFileFromDisk();
	void RenameFileOnDisk();
	void PropertyOnDisk();
	void InitializeDisk();
	void FormatDisk();
	void OpenDroppedFile(wxString &path);

	//@}

	/// @name properties
	//@{
	int GetSelectedMode();

	DiskD88 &GetDiskD88() { return d88; }

	L3DiskList *GetDiskListPanel();

	void ClearLPanelData();
//	bool IsDiskListSelectedDiskImage();
	bool IsDiskListSelectedDisk();
	void ClearRPanelData();

	L3DiskDiskAttr *GetDiskAttrPanel();
	void SetDiskAttrData(DiskD88Disk *disk);
	void ClearDiskAttrData();

	L3DiskFileList *GetFileListPanel();
	void SetFileListData(DiskD88Disk *disk, int side_num);
	void ClearFileListData();
//	int GetFileListSelectedRow();

	L3DiskRawPanel *GetDiskRawPanel();
	void SetRawPanelData(DiskD88Disk *disk, int side_num);
	void ClearRawPanelData();
//	int GetRawPanelSelectedRow();

	L3DiskBinDump *GetBinDumpPanel();
	void SetBinDumpData(const wxUint8 *buf, size_t len);
	void ClearBinDumpData();

	wxString MakeTitleName(const wxString &path);
	wxString GetFileName();

	const wxString &GetRecentPath() const;
	void SetRecentPath(const wxString &path);
	void SetFilePath(const wxString &path);
	//@}

	enum en_menu_id
	{
		// menu id
		IDM_EXIT = 1,

		IDM_NEW_FILE,
		IDM_OPEN_FILE,
		IDM_CLOSE_FILE,
		IDM_SAVEAS_FILE,
		IDM_SAVE_DISK,

		IDM_ADD_DISK,
		IDM_ADD_DISK_NEW,
		IDM_ADD_DISK_FROM_FILE,

		IDM_REPLACE_DISK_FROM_FILE,

		IDM_DELETE_DISK_FROM_FILE,
		IDM_RENAME_DISK,

		IDM_EXPORT_DISK,
		IDM_IMPORT_DISK,
		IDM_DELETE_DISK,
		IDM_RENAME_FILE_ON_DISK,
		IDM_INITIALIZE_DISK,
		IDM_FORMAT_DISK,
		IDM_PROPERTY_DISK,

		IDM_BASIC_MODE,
		IDM_RAWDISK_MODE,

		IDM_CONFIGURE,

		IDD_CONFIGBOX,
		IDD_CHARTYPEBOX,
		IDD_INTNAMEBOX,

		IDM_RECENT_FILE_0 = 90,
		IDM_RECENT_FILE_1,
		IDM_RECENT_FILE_2,
		IDM_RECENT_FILE_3,
		IDM_RECENT_FILE_4,
		IDM_RECENT_FILE_5,
		IDM_RECENT_FILE_6,
		IDM_RECENT_FILE_7,
		IDM_RECENT_FILE_8,
		IDM_RECENT_FILE_9,
	};

	wxDECLARE_EVENT_TABLE();
};

/// 分割ウィンドウ
class L3DiskPanel : public wxSplitterWindow
{
private:
	L3DiskFrame *frame;

	L3DiskList *lpanel;
	L3DiskRPanel *rpanel;

public:
	L3DiskPanel(L3DiskFrame *parent);

	// event handlers

	L3DiskList *GetLPanel() { return lpanel; }
	L3DiskRPanel *GetRPanel() { return rpanel; }

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskPanel);
};

/// 右パネル
class L3DiskRPanel : public wxSplitterWindow
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	L3DiskDiskAttr *diskattr;
	L3DiskRBPanel  *bpanel;

public:
	L3DiskRPanel(L3DiskFrame *parentframe, wxWindow *parentwindow, int selected_window);

	void ChangePanel(int num);

	L3DiskDiskAttr *GetDiskAttrPanel() const { return diskattr; }
//	void SetDiskAttrData(DiskD88Disk *disk);
//	void ClearDiskAttrData();

	L3DiskFileList *GetFileListPanel() const;
//	void SetFileListData(DiskD88Disk *disk, int side_num);
//	void ClearFileListData();
//	int GetFileListSelectedRow();

	L3DiskRawPanel *GetRawPanel() const;
//	void SetRawPanelData(DiskD88Disk *disk, int side_num);
//	void ClearRawPanelData();
//	int GetRawPanelSelectedRow();

	L3DiskBinDump *GetBinDumpPanel() const;
//	void SetBinDumpData(const wxUint8 *buf, size_t len);
//	void ClearBinDumpData();

//	void ClearRBPanelData();

//	void ClearData();

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRPanel);
};

/// 右下パネル
class L3DiskRBPanel : public wxSplitterWindow
{
private:
	L3DiskRPanel *parent;
	L3DiskFrame *frame;

	L3DiskFileList *filelist;
	L3DiskRawPanel *rawpanel;
	L3DiskBinDump  *bindump;

public:
	L3DiskRBPanel(L3DiskFrame *parentframe, L3DiskRPanel *parentwindow, int selected_window);

	void ChangePanel(int num);

	L3DiskFileList *GetFileListPanel() const { return filelist; }
//	void SetFileListData(DiskD88Disk *disk, int side_num);
//	void ClearFileListData();
//	int GetFileListSelectedRow();

	L3DiskRawPanel *GetRawPanel() const { return rawpanel; }

	L3DiskBinDump *GetBinDumpPanel() const { return bindump; }
//	void SetBinDumpData(const wxUint8 *buf, size_t len);
//	void ClearBinDumpData();

//	void ClearData();

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRBPanel);
};

/// ファイルダイアログ
class L3DiskFileDialog: public wxFileDialog
{
public:
	L3DiskFileDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, const wxString& defaultFile = wxEmptyString, const wxString& wildcard = wxFileSelectorDefaultWildcardStr, long style = wxFD_DEFAULT_STYLE);

};

/// エラーメッセージBOX
void L3DiskErrorMessageBox(int level, const wxArrayString &msgs);

/// About dialog
class L3DiskAbout : public wxDialog
{
public:
	L3DiskAbout(wxWindow* parent, wxWindowID id);
};

#endif /* _L3DISKEX_MAIN_H_ */

