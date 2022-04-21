/// @file main.cpp
///
/// @brief 本体
///
#include "main.h"
#include <wx/cmdline.h>
#include <wx/filename.h>
#include "charcodes.h"
#include "diskparambox.h"
// icon
#include "res/l3diskex.xpm"
#include "res/fd_5inch_16_1.xpm"
#include "res/fd_5inch_16_2.xpm"
#include "res/fd_5inch_16_new.xpm"
#include "res/fd_5inch_16_open.xpm"
#include "res/fd_5inch_16_add.xpm"
#include "res/fd_5inch_16_delete.xpm"
#include "res/fd_5inch_16_export.xpm"
#include "res/fd_5inch_16_import.xpm"
#include "res/fileicon_delete.xpm"

#define L3DISK_TRANS \
	_("can't open file '%s'") \
	_("can't create file '%s'") \
	_("can't close file descriptor %d") \
	_("can't read from file descriptor %d") \
	_("can't write to file descriptor %d") \
	_("can't flush file descriptor %d") \
	_("can't seek on file descriptor %d") \
	_("can't get seek position on file descriptor %d")

wxIMPLEMENT_APP(L3DiskApp);

L3DiskApp::L3DiskApp() : mLocale(wxLANGUAGE_DEFAULT)
{
	frame = NULL;
}

bool L3DiskApp::OnInit()
{
	SetAppPath();
	SetAppName(_T("l3diskex"));

	// load ini file
	mConfig.Load(ini_path + GetAppName() + _T(".ini"));

	// set locale search path and catalog name
	mLocale.AddCatalogLookupPathPrefix(res_path + _T("lang"));
	mLocale.AddCatalogLookupPathPrefix(_T("lang"));
	mLocale.AddCatalog(_T("l3diskex"));

	if (!wxApp::OnInit()) {
		return false;
	}

	// load xml
	if (!gDiskTypes.Load(res_path + wxT("data/"))) {
		wxMessageBox(_("Cannot load disk types data file."), _("Error"), wxOK);
		return false;
	}
	if (!gDiskBasicTypes.Load(res_path + wxT("data/"))) {
		wxMessageBox(_("Cannot load disk basic types data file."), _("Error"), wxOK);
		return false;
	}
	if (!gCharCodes.Load(res_path + wxT("data/"))) {
		wxMessageBox(_("Cannot load char codes data file."), _("Error"), wxOK);
		return false;
	}

	frame = new L3DiskFrame(GetAppName(), wxSize(960, 600));
	frame->Show(true);
	SetTopWindow(frame);

	if (!frame->Init(in_file)) {
		return false;
	}

	return true;
}

#define OPTION_VERBOSE "verbose"

void L3DiskApp::OnInitCmdLine(wxCmdLineParser &parser)
{
	// the standard command line options
	static const wxCmdLineEntryDesc cmdLineDesc[] = {
		{
			wxCMD_LINE_SWITCH, "h", "help",
			"show this help message",
			wxCMD_LINE_VAL_NONE,
			wxCMD_LINE_OPTION_HELP
		},

#if wxUSE_LOG
		{
			wxCMD_LINE_SWITCH, NULL, OPTION_VERBOSE,
			"generate verbose log messages",
			wxCMD_LINE_VAL_NONE,
			0x0
		},
#endif // wxUSE_LOG
	    {
			wxCMD_LINE_PARAM, NULL, NULL,
			"input file",
			wxCMD_LINE_VAL_STRING,
			wxCMD_LINE_PARAM_OPTIONAL
		},

		// terminator
		wxCMD_LINE_DESC_END
	};

	parser.SetDesc(cmdLineDesc);
}

bool L3DiskApp::OnCmdLineParsed(wxCmdLineParser &parser)
{
#if wxUSE_LOG
	if ( parser.Found(OPTION_VERBOSE) ) {
		wxLog::SetVerbose(true);
	}
#endif // wxUSE_LOG
	if (parser.GetParamCount() > 0) {
		in_file = parser.GetParam(0);
	}
	return true;
}

int L3DiskApp::OnExit()
{
	// save ini file
	mConfig.Save();

	return 0;
}

void L3DiskApp::MacOpenFile(const wxString &fileName)
{
	if (frame) {
		frame->OpenDataFile(fileName);
	}
}

void L3DiskApp::SetAppPath()
{
	app_path = wxFileName::FileName(argv[0]).GetPath(wxPATH_GET_SEPARATOR);
#ifdef __WXOSX__
	if (app_path.Find(_T("MacOS")) >= 0) {
		wxFileName file = wxFileName::FileName(app_path+"../../");
		file.Normalize();
		ini_path = file.GetPath(wxPATH_GET_SEPARATOR);
		res_path = ini_path+_T("Contents/Resources/");
	} else
#endif
	{
		ini_path = app_path;
		res_path = app_path;
	}
}

const wxString &L3DiskApp::GetAppPath()
{
	return app_path;
}

const wxString &L3DiskApp::GetIniPath()
{
	return ini_path;
}

const wxString &L3DiskApp::GetResPath()
{
	return res_path;
}

Config *L3DiskApp::GetConfig()
{
	return &mConfig;
}

//
// Frame
//
const int IDT_TOOLBAR = 500;
static const long TOOLBAR_STYLE = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT;

// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskFrame, wxFrame)
	// menu event
	EVT_CLOSE(L3DiskFrame::OnClose)

	EVT_MENU(wxID_EXIT,  L3DiskFrame::OnQuit)
	EVT_MENU(wxID_ABOUT, L3DiskFrame::OnAbout)

	EVT_MENU(IDM_NEW_FILE, L3DiskFrame::OnCreateFile)
	EVT_MENU(IDM_OPEN_FILE, L3DiskFrame::OnOpenFile)

	EVT_MENU(IDM_CLOSE_FILE, L3DiskFrame::OnCloseFile)
	EVT_MENU(IDM_SAVEAS_FILE, L3DiskFrame::OnSaveAsFile)
	EVT_MENU(IDM_SAVE_DISK, L3DiskFrame::OnSaveDisk)

	EVT_MENU(IDM_ADD_DISK, L3DiskFrame::OnAddDiskFromFile)
	EVT_MENU(IDM_ADD_DISK_NEW, L3DiskFrame::OnAddNewDisk)
	EVT_MENU(IDM_ADD_DISK_FROM_FILE, L3DiskFrame::OnAddDiskFromFile)

	EVT_MENU(IDM_REPLACE_DISK_FROM_FILE, L3DiskFrame::OnReplaceDisk)

	EVT_MENU(IDM_DELETE_DISK_FROM_FILE, L3DiskFrame::OnDeleteDiskFromFile)
	EVT_MENU(IDM_RENAME_DISK, L3DiskFrame::OnRenameDisk)

	EVT_MENU_RANGE(IDM_RECENT_FILE_0, IDM_RECENT_FILE_9, L3DiskFrame::OnOpenRecentFile)

	EVT_MENU(IDM_EXPORT_DISK, L3DiskFrame::OnExportFileFromDisk)
	EVT_MENU(IDM_IMPORT_DISK, L3DiskFrame::OnImportFileToDisk)
	EVT_MENU(IDM_DELETE_DISK, L3DiskFrame::OnDeleteFileFromDisk)
	EVT_MENU(IDM_RENAME_FILE_ON_DISK, L3DiskFrame::OnRenameFileOnDisk)
	EVT_MENU(IDM_INITIALIZE_DISK, L3DiskFrame::OnInitializeDisk)
	EVT_MENU(IDM_FORMAT_DISK, L3DiskFrame::OnFormatDisk)
	EVT_MENU(IDM_PROPERTY_DISK, L3DiskFrame::OnPropertyOnDisk)

	EVT_MENU(IDM_BASIC_MODE, L3DiskFrame::OnBasicMode)
	EVT_MENU(IDM_RAWDISK_MODE, L3DiskFrame::OnRawDiskMode)

#ifdef USE_MENU_OPEN
	EVT_MENU_OPEN(L3DiskFrame::OnMenuOpen)
#endif
wxEND_EVENT_TABLE()

// 翻訳用
#define DIALOG_BUTTON_STRING _("OK"),_("Cancel")
#define APPLE_MENU_STRING _("Hide l3diskex"),_("Hide Others"),_("Show All"),_("Quit l3diskex"),_("Services"),_("Preferences…")

L3DiskFrame::L3DiskFrame(const wxString& title, const wxSize& size)
       : wxFrame(NULL, -1, title, wxDefaultPosition, size)
{
	// config
	ini = wxGetApp().GetConfig();

	// icon
#ifdef __WXMSW__
	SetIcon(wxIcon(_T("l3diskex")));
#elif defined(__WXGTK__) || defined(__WXMOTIF__)
	SetIcon(wxIcon(l3diskex_xpm));
#endif

	// menu
	menuFile = new wxMenu;
	menuDisk = new wxMenu;
	menuMode = new wxMenu;
	menuHelp = new wxMenu;
	wxMenu *sm;

	// file menu
	menuFile->Append( IDM_NEW_FILE, _("&New...") );
	menuFile->Append( IDM_OPEN_FILE, _("&Open...") );
	menuFile->AppendSeparator();
	menuFile->Append( IDM_CLOSE_FILE, _("&Close") );
	menuFile->Append( IDM_SAVEAS_FILE, _("Save &As...") );
	menuFile->Append( IDM_SAVE_DISK, _("Save A Disk...") );
	menuFile->AppendSeparator();
		sm = new wxMenu();
		sm->Append( IDM_ADD_DISK_NEW, _("&New Disk...") );
		sm->Append( IDM_ADD_DISK_FROM_FILE, _("From &File...") );
	menuFile->AppendSubMenu(sm, _("Add D&isk") );
	menuFile->AppendSeparator();
	menuFile->Append( IDM_REPLACE_DISK_FROM_FILE, _("R&eplace Disk Data...") );
	menuFile->AppendSeparator();
	menuFile->Append( IDM_DELETE_DISK_FROM_FILE, _("&Delete Disk...") );
	menuFile->Append( IDM_RENAME_DISK, _("&Rename Disk") );
	menuFile->AppendSeparator();
	menuFile->Append( IDM_INITIALIZE_DISK, _("I&nitialize...") );
	menuFile->Append( IDM_FORMAT_DISK, _("&Format For BASIC...") );
	menuFile->AppendSeparator();
		menuRecentFiles = new wxMenu();
		UpdateMenuRecentFiles();
	menuFile->AppendSubMenu(menuRecentFiles, _("&Reccent Files") );
	menuFile->AppendSeparator();
	menuFile->Append( wxID_EXIT, _("E&xit") );
	// disk menu
	menuDisk->Append( IDM_EXPORT_DISK, _("&Export...") );
	menuDisk->Append( IDM_IMPORT_DISK, _("&Import...") );
	menuDisk->AppendSeparator();
	menuDisk->Append( IDM_DELETE_DISK, _("&Delete...") );
	menuDisk->Append( IDM_RENAME_FILE_ON_DISK, _("&Rename File") );
	menuDisk->AppendSeparator();
	menuDisk->Append( IDM_PROPERTY_DISK, _("&Property") );
	// mode menu
	menuMode->AppendRadioItem( IDM_BASIC_MODE, _("BASIC Mode") );
	menuMode->AppendRadioItem( IDM_RAWDISK_MODE, _("Raw Disk Mode") );
	// help menu
	menuHelp->Append( wxID_ABOUT, _("&About...") );

	// menu bar
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, _("&File") );
	menuBar->Append( menuDisk, _("&Disk") );
	menuBar->Append( menuMode, _("&Mode") );
	menuBar->Append( menuHelp, _("&Help") );

	SetMenuBar( menuBar );

	// tool bar
    RecreateToolbar();

	// splitter window
	panel = new L3DiskPanel(this);
}

L3DiskFrame::~L3DiskFrame()
{
}

bool L3DiskFrame::Init(const wxString &in_file)
{
	if (!in_file.IsEmpty()) {
		OpenDataFile(in_file);
	}
	UpdateMenuFile();
	UpdateMenuDisk();
	UpdateToolBar();
	return true;
}

void L3DiskFrame::RecreateToolbar()
{
	// delete and recreate the toolbar
	wxToolBarBase *toolBar = GetToolBar();
	long style = toolBar ? toolBar->GetWindowStyle() : TOOLBAR_STYLE;
	delete toolBar;

	SetToolBar(NULL);

	style &= ~(wxTB_HORIZONTAL | wxTB_VERTICAL | wxTB_BOTTOM | wxTB_RIGHT | wxTB_HORZ_LAYOUT);
	style |= wxTB_TOP;
	style &= ~wxTB_NO_TOOLTIPS;
	style |= wxTB_HORZ_LAYOUT;
	style |= wxTB_TEXT;

	toolBar = CreateToolBar(style, IDT_TOOLBAR);

	PopulateToolbar(toolBar);
}

#define INIT_TOOL_BMP(bmp) \
	toolBarBitmaps[bmp] = wxBitmap(bmp##_xpm)

void L3DiskFrame::PopulateToolbar(wxToolBarBase* toolBar)
{
	// Set up toolbar
	enum {
		fd_5inch_16_new,
		fd_5inch_16_open,
		fd_5inch_16_1,
		fd_5inch_16_add,
		fd_5inch_16_delete,
		fd_5inch_16_export,
		fd_5inch_16_import,
		fileicon_delete,
		Tool_Max
	};

	wxBitmap toolBarBitmaps[Tool_Max];

	INIT_TOOL_BMP(fd_5inch_16_new);
	INIT_TOOL_BMP(fd_5inch_16_open);
	INIT_TOOL_BMP(fd_5inch_16_1);
	INIT_TOOL_BMP(fd_5inch_16_add);
	INIT_TOOL_BMP(fd_5inch_16_delete);
	INIT_TOOL_BMP(fd_5inch_16_export);
	INIT_TOOL_BMP(fd_5inch_16_import);
	INIT_TOOL_BMP(fileicon_delete);

	int w = toolBarBitmaps[fd_5inch_16_new].GetWidth(),
		h = toolBarBitmaps[fd_5inch_16_new].GetHeight();

#if 0
    if ( !m_smallToolbar )
    {
        w *= 2;
        h *= 2;

        for ( size_t n = Tool_new; n < WXSIZEOF(toolBarBitmaps); n++ )
        {
            toolBarBitmaps[n] =
                wxBitmap(toolBarBitmaps[n].ConvertToImage().Scale(w, h));
        }
    }
#endif

	toolBar->SetToolBitmapSize(wxSize(w, h));

	toolBar->AddTool(IDM_NEW_FILE, _("New"),
		toolBarBitmaps[fd_5inch_16_new], wxNullBitmap, wxITEM_NORMAL,
		_("New disk image"));
	toolBar->AddTool(IDM_OPEN_FILE, _("Open"),
		toolBarBitmaps[fd_5inch_16_open], wxNullBitmap, wxITEM_NORMAL,
		_("Open disk image"));
	toolBar->AddTool(IDM_SAVEAS_FILE, _("Save As"),
		toolBarBitmaps[fd_5inch_16_1], wxNullBitmap, wxITEM_NORMAL,
		_("Save disk image"));
	toolBar->AddTool(IDM_ADD_DISK, _("Add"),
		toolBarBitmaps[fd_5inch_16_add], wxNullBitmap, wxITEM_DROPDOWN,
		_("Add a disk on disk image"));
	wxMenu* sm = new wxMenu;
	sm->Append( IDM_ADD_DISK_NEW, _("&New Disk...") );
	sm->Append( IDM_ADD_DISK_FROM_FILE, _("From &File...") );
	toolBar->SetDropdownMenu(IDM_ADD_DISK, sm);
//	toolBar->AddTool(IDM_DELETE_DISK_FROM_FILE, _("Delete"),
//		toolBarBitmaps[fd_5inch_16_delete], wxNullBitmap, wxITEM_NORMAL,
//		_("Delete a disk from disk image"));
	toolBar->AddSeparator();
	toolBar->AddTool(IDM_EXPORT_DISK, _("Export"),
		toolBarBitmaps[fd_5inch_16_export], wxNullBitmap, wxITEM_NORMAL,
		_("Export a file from the disk"));
	toolBar->AddTool(IDM_IMPORT_DISK, _("Import"),
		toolBarBitmaps[fd_5inch_16_import], wxNullBitmap, wxITEM_NORMAL,
		_("Import a file to the disk"));
//	toolBar->AddTool(IDM_DELETE_DISK, _("Delete file"),
//		toolBarBitmaps[fileicon_delete], wxNullBitmap, wxITEM_NORMAL,
//		_("Delete a file from the disk"));

#if 0
	toolBar->AddStretchableSpace();
	toolBar->AddTool(wxID_HELP, wxT("Help"), toolBarBitmaps[Tool_help], wxT("Help button"), wxITEM_CHECK);
#endif

	// after adding the buttons to the toolbar, must call Realize() to reflect
	// the changes
	toolBar->Realize();
	int m_rows = 1;
	toolBar->SetRows(toolBar->IsVertical() ? (int)toolBar->GetToolsCount() / m_rows : m_rows);
}

#ifdef USE_MENU_OPEN
/// メニュー更新
void L3DiskFrame::OnMenuOpen(wxMenuEvent& event)
{
	wxMenu *menu = event.GetMenu();

	if (menu == NULL) return;

	if (menu == menuFile) {	// File...
		UpdateMenuFile();
	} else if (menu == menuDisk) { // Disk...
		UpdateMenuDisk();
	}
}
#endif

/// ドロップされたファイルを開く
void L3DiskFrame::OpenDroppedFile(wxString &path)
{
 	if (!CloseDataFile()) return;
	OpenDataFile(path);
}

void L3DiskFrame::OnClose(wxCloseEvent& event)
{
	if (!CloseDataFile(!event.CanVeto())) {
		event.Veto();
		return;
	}
	event.Skip();
}

void L3DiskFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(false);
}

void L3DiskFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	L3DiskAbout(this, wxID_ANY).ShowModal();
}

void L3DiskFrame::OnCreateFile(wxCommandEvent& WXUNUSED(event))
{
	ShowCreateFileDialog();
}

void L3DiskFrame::OnOpenFile(wxCommandEvent& WXUNUSED(event))
{
	ShowOpenFileDialog();
}

void L3DiskFrame::OnOpenRecentFile(wxCommandEvent& event)
{
	wxMenuItem *item = menuRecentFiles->FindItem(event.GetId());
	if (!item) return;
	wxFileName path = item->GetItemLabel();
	if (!CloseDataFile()) return;
	OpenDataFile(path.GetFullPath());
}

void L3DiskFrame::OnCloseFile(wxCommandEvent& WXUNUSED(event))
{
	CloseDataFile();
}
/// 名前を付けて保存
void L3DiskFrame::OnSaveAsFile(wxCommandEvent& WXUNUSED(event))
{
	ShowSaveFileDialog();
}
/// ディスク1枚を保存
void L3DiskFrame::OnSaveDisk(wxCommandEvent& WXUNUSED(event))
{
	L3DiskList *list = GetDiskListPanel();
	if (!list) return;
	ShowSaveDiskDialog(list->GetSelectedDiskNumber(), list->GetSelectedDiskSide());
}
/// ディスクを新規に追加
void L3DiskFrame::OnAddNewDisk(wxCommandEvent& WXUNUSED(event))
{
	ShowAddNewDiskDialog();
}
/// ディスクをファイルから追加
void L3DiskFrame::OnAddDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	ShowAddFileDialog();
}
/// ディスクを置換
void L3DiskFrame::OnReplaceDisk(wxCommandEvent& WXUNUSED(event))
{
	L3DiskList *list = GetDiskListPanel();
	if (!list) return;
	ShowReplaceDiskDialog(list->GetSelectedDiskNumber(), list->GetSelectedDiskSide());
}
/// ファイルからディスクを削除
void L3DiskFrame::OnDeleteDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	DeleteDisk();
}
/// ディスク名を変更
void L3DiskFrame::OnRenameDisk(wxCommandEvent& WXUNUSED(event))
{
	RenameDisk();
}

/// エクスポート
void L3DiskFrame::OnExportFileFromDisk(wxCommandEvent& WXUNUSED(event))
{
	ExportFileFromDisk();
}
/// インポート
void L3DiskFrame::OnImportFileToDisk(wxCommandEvent& WXUNUSED(event))
{
	ImportFileToDisk();
}
/// 削除
void L3DiskFrame::OnDeleteFileFromDisk(wxCommandEvent& WXUNUSED(event))
{
	DeleteFileFromDisk();
}
/// リネーム
void L3DiskFrame::OnRenameFileOnDisk(wxCommandEvent& WXUNUSED(event))
{
	RenameFileOnDisk();
}
/// 初期化
void L3DiskFrame::OnInitializeDisk(wxCommandEvent& WXUNUSED(event))
{
	InitializeDisk();
}
/// フォーマット
void L3DiskFrame::OnFormatDisk(wxCommandEvent& WXUNUSED(event))
{
	FormatDisk();
}
/// プロパティ
void L3DiskFrame::OnPropertyOnDisk(wxCommandEvent& WXUNUSED(event))
{
	PropertyOnDisk();
}

/// 設定ダイアログ
void L3DiskFrame::OnConfigure(wxCommandEvent& WXUNUSED(event))
{

}

/// ファイルモード選択
void L3DiskFrame::OnBasicMode(wxCommandEvent& event)
{
	ChangeRPanel(0);
}

/// Rawディスクモード選択
void L3DiskFrame::OnRawDiskMode(wxCommandEvent& event)
{
	ChangeRPanel(1);
}

/// 右パネルのデータウィンドウを変更
void L3DiskFrame::ChangeRPanel(int num)
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) rpanel->ChangePanel(num);
	L3DiskList *lpanel = panel->GetLPanel();
	if (lpanel) lpanel->ReSelect();

	UpdateMenuDisk();
}

/// 選択しているModeメニュー
int L3DiskFrame::GetSelectedMode()
{
	if (!menuMode) return 0;

	if (menuMode->FindItem(IDM_BASIC_MODE)->IsChecked()) {
		return 0;
	} else if (menuMode->FindItem(IDM_RAWDISK_MODE)->IsChecked()) {
		return 1;
	}
	return 0;
}

L3DiskList *L3DiskFrame::GetLPanel()
{
	return panel->GetLPanel();
}
L3DiskList *L3DiskFrame::GetDiskListPanel()
{
	return panel->GetLPanel();
}
void L3DiskFrame::SetLPanelData()
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->SetFileName();
}
void L3DiskFrame::ClearLPanelData()
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->ClearFileName();
}
//bool L3DiskFrame::IsDiskListSelectedDiskImage()
//{
//	L3DiskList *lpanel = GetDiskListPanel();
//	if (lpanel) return lpanel->IsSelectedDiskImage();
//	else return false;
//}
bool L3DiskFrame::IsDiskListSelectedDisk()
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) return lpanel->IsSelectedDisk();
	else return false;
}

L3DiskRPanel *L3DiskFrame::GetRPanel()
{
	return panel->GetRPanel();
}
void L3DiskFrame::ClearRPanelData()
{
	ClearDiskAttrData();
	ClearFileListData();
	ClearRawPanelData();
	ClearBinDumpData();
}

/// ディスク属性パネル
L3DiskDiskAttr *L3DiskFrame::GetDiskAttrPanel()
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetDiskAttrPanel();
	else return NULL;
}
void L3DiskFrame::SetDiskAttrData(DiskD88Disk *disk)
{
	L3DiskDiskAttr *dapanel = GetDiskAttrPanel();
	if (dapanel) dapanel->SetAttr(disk);
}
void L3DiskFrame::ClearDiskAttrData()
{
	L3DiskDiskAttr *dapanel = GetDiskAttrPanel();
	if (dapanel) dapanel->ClearData();
}

/// ファイルリストパネル
L3DiskFileList *L3DiskFrame::GetFileListPanel()
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetFileListPanel();
	else return NULL;
}
void L3DiskFrame::SetFileListData(DiskD88Disk *disk, int side_num)
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->SetFiles(disk, side_num);
}
void L3DiskFrame::ClearFileListData()
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->ClearFiles();
}
//int L3DiskFrame::GetFileListSelectedRow()
//{
//	L3DiskRPanel *rpanel = panel->GetRPanel();
//	if (rpanel) return rpanel->GetFileListSelectedRow();
//	else return -2;
//}

/// ディスクパネル
L3DiskRawPanel *L3DiskFrame::GetDiskRawPanel()
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetRawPanel();
	else return NULL;
}
void L3DiskFrame::SetRawPanelData(DiskD88Disk *disk, int side_num)
{
	L3DiskRawPanel *rawpanel = GetDiskRawPanel();
	if (rawpanel) rawpanel->SetTrackListData(disk, side_num);
}
void L3DiskFrame::ClearRawPanelData()
{
	L3DiskRawPanel *rawpanel = GetDiskRawPanel();
	if (rawpanel) rawpanel->ClearTrackListData();
}
//int L3DiskFrame::GetRawPanelSelectedRow()
//{
//	L3DiskRPanel *rpanel = panel->GetRPanel();
//	if (rpanel) return rpanel->GetRawPanelSelectedRow();
//	else return -2;
//}

/// バイナルダンプ
L3DiskBinDump *L3DiskFrame::GetBinDumpPanel()
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetBinDumpPanel();
	else return NULL;
}
void L3DiskFrame::SetBinDumpData(const wxUint8 *buf, size_t len)
{
	L3DiskBinDump *binpanel = GetBinDumpPanel();
	if (binpanel) binpanel->SetDatas(buf, len);
}
void L3DiskFrame::ClearBinDumpData()
{
	L3DiskBinDump *binpanel = GetBinDumpPanel();
	if (binpanel) binpanel->ClearDatas();
}

/// ファイルメニューの更新
void L3DiskFrame::UpdateMenuFile()
{
	bool opened = (d88.GetFile() != NULL);
	menuFile->Enable(IDM_CLOSE_FILE, opened);
	menuFile->Enable(IDM_ADD_DISK_NEW, opened);
	menuFile->Enable(IDM_ADD_DISK_FROM_FILE, opened);

	opened = (opened && d88.CountDisks() > 0);
	menuFile->Enable(IDM_SAVEAS_FILE, opened);

	L3DiskList *list = GetDiskListPanel();
	if (list) {
		UpdateMenuDiskList(list);
	}
}

/// ディスクメニューの更新
void L3DiskFrame::UpdateMenuDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		UpdateMenuFileList(list);
		return;
	}

	L3DiskRawPanel *rawpl = GetDiskRawPanel();
	if (rawpl) {
		UpdateMenuRawDisk(rawpl);
		return;
	}
}

/// 最近使用したファイル一覧を更新
void L3DiskFrame::UpdateMenuRecentFiles()
{
	// メニューを更新
	wxArrayString names;
	ini->GetRecentFiles(names);
	for(int i=0; i<10 && i<(int)names.Count(); i++) {
		if (menuRecentFiles->FindItem(IDM_RECENT_FILE_0 + i)) menuRecentFiles->Delete(IDM_RECENT_FILE_0 + i);
		menuRecentFiles->Append(IDM_RECENT_FILE_0 + i, names[i]);
	}
}

void L3DiskFrame::UpdateToolBar()
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (d88.GetFile() != NULL);
//	toolBar->EnableTool(IDM_CLOSE_FILE, opened);
	toolBar->EnableTool(IDM_ADD_DISK, opened);
	toolBar->EnableTool(IDM_ADD_DISK_NEW, opened);
	toolBar->EnableTool(IDM_ADD_DISK_FROM_FILE, opened);

	opened = (opened && d88.CountDisks() > 0);
	toolBar->EnableTool(IDM_SAVEAS_FILE, opened);

	L3DiskList *dlist = GetDiskListPanel();
	if (dlist) {
		UpdateToolBarDiskList(dlist);
	}

	L3DiskFileList *flist = GetFileListPanel();
	if (flist) {
		UpdateToolBarFileList(flist);
		return;
	}

	L3DiskRawPanel *rawpl = GetDiskRawPanel();
	if (rawpl) {
		UpdateToolBarRawDisk(rawpl);
		return;
	}
}

void L3DiskFrame::UpdateMenuDiskList(L3DiskList *list)
{
	bool opened = (list != NULL && list->IsSelectedDiskImage());
	menuFile->Enable(IDM_REPLACE_DISK_FROM_FILE, opened);
	menuFile->Enable(IDM_SAVE_DISK, opened);
	menuFile->Enable(IDM_DELETE_DISK_FROM_FILE, opened);
	menuFile->Enable(IDM_RENAME_DISK, opened);
	menuFile->Enable(IDM_INITIALIZE_DISK, opened);
}

void L3DiskFrame::UpdateToolBarDiskList(L3DiskList *list)
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (list != NULL && list->IsSelectedDiskImage());
	toolBar->EnableTool(IDM_SAVE_DISK, opened);
	toolBar->EnableTool(IDM_DELETE_DISK_FROM_FILE, opened);
//	toolBar->EnableTool(IDM_RENAME_DISK, opened);
//	toolBar->EnableTool(IDM_INITIALIZE_DISK, opened);
}

void L3DiskFrame::UpdateMenuAndToolBarDiskList(L3DiskList *list)
{
	UpdateMenuDiskList(list);
	UpdateToolBarDiskList(list);
}

void L3DiskFrame::UpdateMenuFileList(L3DiskFileList *list)
{
	bool opened = (list != NULL && list->CanUseBasicDisk());
	menuFile->Enable(IDM_FORMAT_DISK, opened);

	opened = (opened && list->IsFormattedBasicDisk());
	menuDisk->Enable(IDM_IMPORT_DISK, opened);

	int	pos = list->GetSelectedRow();
	opened = (opened && pos >= 0);
	menuDisk->Enable(IDM_EXPORT_DISK, opened);
	menuDisk->Enable(IDM_DELETE_DISK, opened);
	menuDisk->Enable(IDM_RENAME_FILE_ON_DISK, opened);
	menuDisk->Enable(IDM_PROPERTY_DISK, opened);
}

void L3DiskFrame::UpdateToolBarFileList(L3DiskFileList *list)
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (list && list->IsFormattedBasicDisk());
	toolBar->EnableTool(IDM_IMPORT_DISK, opened);

	int pos = list->GetSelectedRow();
	opened = (opened && pos >= 0);
	toolBar->EnableTool(IDM_EXPORT_DISK, opened);
	toolBar->EnableTool(IDM_DELETE_DISK, opened);
//	toolBar->EnableTool(IDM_RENAME_FILE_ON_DISK, opened);
//	toolBar->EnableTool(IDM_PROPERTY_DISK, opened);
}

void L3DiskFrame::UpdateMenuAndToolBarFileList(L3DiskFileList *list)
{
	UpdateMenuFileList(list);
	UpdateToolBarFileList(list);
}

void L3DiskFrame::UpdateMenuRawDisk(L3DiskRawPanel *rawpanel)
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (rawpanel != NULL);
	opened = (opened && rawpanel->TrackListExists());
	menuDisk->Enable(IDM_EXPORT_DISK, opened);
	menuDisk->Enable(IDM_IMPORT_DISK, opened);
	menuDisk->Enable(IDM_DELETE_DISK, false);

	int pos = rawpanel->GetSectorListSelectedRow();
	opened = (opened && pos >= 0);
	menuDisk->Enable(IDM_PROPERTY_DISK, opened);
}

void L3DiskFrame::UpdateToolBarRawDisk(L3DiskRawPanel *rawpanel)
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (rawpanel != NULL);
	opened = (opened && rawpanel->TrackListExists());
	toolBar->EnableTool(IDM_EXPORT_DISK, opened);
	toolBar->EnableTool(IDM_IMPORT_DISK, opened);
//	toolBar->EnableTool(IDM_DELETE_DISK, false);

//	pos = rawpanel->GetSectorListSelectedRow();
//	opened = (opened && pos >= 0);
//	toolBar->EnableTool(IDM_PROPERTY_DISK, opened);
}

void L3DiskFrame::UpdateMenuAndToolBarRawDisk(L3DiskRawPanel *rawpanel)
{
	UpdateMenuRawDisk(rawpanel);
	UpdateToolBarRawDisk(rawpanel);
}

wxString L3DiskFrame::MakeTitleName(const wxString &path)
{
	wxString title;
	if (path.IsEmpty()) {
		title = _("(new file)");
	} else {
		title = path;
	}
	return title;
}

wxString L3DiskFrame::GetFileName()
{
	return MakeTitleName(d88.GetFileName());
}

/// 最近使用したパスを取得
const wxString &L3DiskFrame::GetRecentPath() const
{
	return ini->GetFilePath();
}

/// 最近使用したファイルを更新（一覧も更新）
void L3DiskFrame::SetRecentPath(const wxString &path)
{
	// set recent file path
	ini->AddRecentFile(path);
	UpdateMenuRecentFiles();
}

/// 最近使用したパスを更新
void L3DiskFrame::SetFilePath(const wxString &path)
{
	ini->SetFilePath(path);
}

/// ウィンドウ上のデータを更新
void L3DiskFrame::UpdateDataOnWindow(const wxString &path)
{
	// update window
	if (!path.IsEmpty()) {
		wxString title = wxGetApp().GetAppName() + wxT(" - ") + path;
		SetTitle(title);
	}

	// update panel
	SetLPanelData();
	ClearRPanelData();

#ifndef USE_MENU_OPEN
	// update menu
	UpdateMenuFile();
#endif
}

/// 新規作成ダイアログ
void L3DiskFrame::ShowCreateFileDialog()
{
	if (!CloseDataFile()) return;

	DiskParamBox dlg(this, wxID_ANY, _("Create New Disk"), 0, NULL);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		DiskParam param;
		dlg.GetParam(param);
		CreateDataFile(dlg.GetDiskName(), param, dlg.GetWriteProtect());
	}
}
/// 新規作成
void L3DiskFrame::CreateDataFile(const wxString &diskname, const DiskParam &param, bool write_protect)
{
	wxString path = MakeTitleName(wxEmptyString);

	// create disk
	int rc = d88.Create(diskname, param, write_protect);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(path);
	}
	if (rc != 0) {
		// message
		L3DiskErrorMessageBox(rc, d88.GetErrorMessage());
	}
	UpdateToolBar();
}
/// ディスク追加ダイアログ
void L3DiskFrame::ShowAddNewDiskDialog()
{
	if (!d88.GetFile()) return;

	DiskParamBox dlg(this, wxID_ANY, _("Add New Disk"), d88.GetDiskTypeNumber(0), NULL);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		DiskParam param;
		dlg.GetParam(param);
		AddNewDisk(dlg.GetDiskName(), param, dlg.GetWriteProtect());
	}
}
/// ディスクを追加
void L3DiskFrame::AddNewDisk(const wxString &diskname, const DiskParam &param, bool write_protect)
{
	if (!d88.GetFile()) return;

	// add a disk
	int rc = d88.Add(diskname, param, write_protect);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString);
	}
	if (rc != 0) {
		// message
		L3DiskErrorMessageBox(rc, d88.GetErrorMessage());
	}
}
/// オープンダイアログ
void L3DiskFrame::ShowOpenFileDialog()
{
	if (!CloseDataFile()) return;

	L3DiskFileDialog dlg(
		_("Open File"),
		GetRecentPath(),
		wxEmptyString,
		_("Supported files (*.d88)|*.d88|All files (*.*)|*.*"),
		wxFD_OPEN);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		OpenDataFile(path);
	}
}
/// 指定したファイルを開く
void L3DiskFrame::OpenDataFile(const wxString &path)
{
	// set recent file path
	SetRecentPath(path);

	// open disk
	int rc = d88.Open(path);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(path);
	}
	if (rc != 0) {
		// message
		L3DiskErrorMessageBox(rc, d88.GetErrorMessage());
	}
	UpdateToolBar();
}

/// ファイル追加ダイアログ
void L3DiskFrame::ShowAddFileDialog()
{
	L3DiskFileDialog dlg(
		_("Add File"),
		GetRecentPath(),
		wxEmptyString,
		_("Supported files (*.d88)|*.d88|All files (*.*)|*.*"),
		wxFD_OPEN);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		AddDiskFile(path);
	}
}
/// 指定したファイルを追加
void L3DiskFrame::AddDiskFile(const wxString &path)
{
	// set recent file path
	SetRecentPath(path);

	// open disk
	int rc = d88.Add(path);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString);
	}
	if (rc != 0) {
		// message
		L3DiskErrorMessageBox(rc, d88.GetErrorMessage());
	}
}
/// ファイルを閉じる
/// @return false:キャンセルした
bool L3DiskFrame::CloseDataFile(bool force)
{
	// 変更されているか
	if (!force && d88.IsModified()) {
		int ans = wxMessageBox(_("This file is modified. Do you want to save it?"), _("Close File"), wxYES_NO | wxCANCEL | wxICON_INFORMATION);  
		if (ans == wxCANCEL) {
			return false;
		}
		if (ans == wxYES) {
			ShowSaveFileDialog();
		}
	}

	//
	d88.Close();

	// update window
	wxString title = wxGetApp().GetAppName();
	SetTitle(title);

	// update panel
	ClearLPanelData();
	ClearRPanelData();

#ifndef USE_MENU_OPEN
	// update menu
	UpdateMenuFile();
#endif
	UpdateToolBar();

	return true;
}
/// 保存ダイアログ
void L3DiskFrame::ShowSaveFileDialog()
{
	L3DiskFileDialog dlg(
		_("Save File"),
		d88.GetPath(),
		d88.GetFileName(),
		_("Supported files (*.d88)|*.d88|All files (*.*)|*.*"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		SaveDataFile(path);
	}
}
/// 指定したファイルに保存
void L3DiskFrame::SaveDataFile(const wxString &path)
{
	// set recent file path
	SetRecentPath(path);

	// save disk
	int rc = d88.Save(path);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(path);
	}
	if (rc != 0) {
		// message
		L3DiskErrorMessageBox(rc, d88.GetErrorMessage());
	}
}
/// ディスクをファイルに保存ダイアログ（指定ディスク）
void L3DiskFrame::ShowSaveDiskDialog(int disk_number, int side_number)
{

	wxString filename = d88.GetDiskName(disk_number, true);
	if (filename.IsEmpty()) {
		filename = d88.GetFileNameBase();
		filename += wxString::Format(wxT("_%02d"), disk_number);
	}
	if (side_number >= 0) filename += wxString::Format(wxT("_%c"), side_number + 0x41);
	filename += wxT(".d88");

	wxString title = _("Save Disk");
	if (side_number >= 0) title += wxString::Format(_(" (side %c)"), side_number + 0x41);
	L3DiskFileDialog dlg(
		title,
		GetRecentPath(),
		filename,
		_("Supported files (*.d88)|*.d88|All files (*.*)|*.*"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		SaveDataDisk(disk_number, side_number, path);
	}
}
/// 指定したファイルに保存（指定ディスク）
void L3DiskFrame::SaveDataDisk(int disk_number, int side_number, const wxString &path)
{
	// set recent file path
	SetRecentPath(path);

	// save disk
	int rc = d88.SaveDisk(disk_number, side_number, path);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString);
	}
	if (rc != 0) {
		// message
		L3DiskErrorMessageBox(rc, d88.GetErrorMessage());
	}
}
/// ディスクイメージ置換ダイアログ
void L3DiskFrame::ShowReplaceDiskDialog(int disk_number, int side_number)
{
	wxString title = _("Replace Disk Data");
	if (side_number >= 0) title += wxString::Format(_(" (side %c)"), side_number + 0x41);
	L3DiskFileDialog dlg(
		title,
		GetRecentPath(),
		wxEmptyString,
		_("Supported files (*.d88)|*.d88|All files (*.*)|*.*"),
		wxFD_OPEN);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		ReplaceDisk(disk_number, side_number, path);
	}
}
/// 指定したディスクイメージ置換
void L3DiskFrame::ReplaceDisk(int disk_number, int side_number, const wxString &path)
{
	// set recent file path
	SetRecentPath(path);

	// open disk
	int rc = d88.ReplaceDisk(disk_number, side_number, path);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString);
	}
	if (rc != 0) {
		// message
		L3DiskErrorMessageBox(rc, d88.GetErrorMessage());
	}
}
/// ディスクをファイルから削除
void L3DiskFrame::DeleteDisk()
{
	L3DiskList *list = GetLPanel();
	if (list) {
		list->DeleteDisk();
		return;
	}
}
/// ディスク名を変更
void L3DiskFrame::RenameDisk()
{
	L3DiskList *list = GetLPanel();
	if (list) {
		list->RenameDisk();
		return;
	}
}
/// ディスクからファイルをエクスポート
void L3DiskFrame::ExportFileFromDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowExportDataFileDialog();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowExportTrackDialog();
		return;
	}
}
/// ディスクにファイルをインポート
void L3DiskFrame::ImportFileToDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowImportDataFileDialog();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowImportTrackDialog();
		return;
	}
}
/// ディスクからファイルを削除
void L3DiskFrame::DeleteFileFromDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->DeleteDataFile();
		return;
	}
}
/// ディスクのファイル名を変更
void L3DiskFrame::RenameFileOnDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->StartEditingFileName();
		return;
	}
}
/// ファイルのプロパティ
void L3DiskFrame::PropertyOnDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowFileAttr();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowSectorAttr();
		return;
	}
}
/// ディスクを初期化
void L3DiskFrame::InitializeDisk()
{
	L3DiskList *list = GetLPanel();
	if (list) {
		list->InitializeDisk();
		return;
	}
}
/// ディスクをフォーマット
void L3DiskFrame::FormatDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->FormatDisk();
		return;
	}
}

//
// メインパネルは分割ウィンドウ
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

L3DiskPanel::L3DiskPanel(L3DiskFrame *parent)
                : wxSplitterWindow(parent, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_3D | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
    frame = parent;

	// fit size on parent window
    SetSize(parent->GetClientSize());

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	// control panel
	lpanel = new L3DiskList(frame, this);
	rpanel = new L3DiskRPanel(frame, this, frame->GetSelectedMode());
	SplitVertically(lpanel, rpanel, 200);

	SetMinimumPaneSize(10);
}

//
// 右パネル上下
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

L3DiskRPanel::L3DiskRPanel(L3DiskFrame *parentframe, wxWindow *parentwindow, int selected_window)
                : wxSplitterWindow(parentwindow, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_3D | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
	parent = parentwindow;
    frame = parentframe;

	// fit size
	SetSize(parentwindow->GetSize());

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	diskattr = new L3DiskDiskAttr(parentframe, this);
	bpanel = new L3DiskRBPanel(parentframe, this, selected_window);

	SplitHorizontally(diskattr, bpanel, 32);

	SetMinimumPaneSize(10);
}

void L3DiskRPanel::ChangePanel(int num)
{
	if (bpanel) bpanel->ChangePanel(num);
}

//void L3DiskRPanel::SetDiskAttrData(DiskD88Disk *disk)
//{
//	if (diskattr) diskattr->SetAttr(disk);
//}
//void L3DiskRPanel::ClearDiskAttrData()
//{
//	if (diskattr) diskattr->ClearData();
//}

L3DiskFileList *L3DiskRPanel::GetFileListPanel() const
{
	if (bpanel) return bpanel->GetFileListPanel();
	else return NULL;
}

//void L3DiskRPanel::SetFileListData(DiskD88Disk *disk, int side_num)
//{
//	if (bpanel) bpanel->SetFileListData(disk, side_num);
//}
//void L3DiskRPanel::ClearFileListData()
//{
//	if (bpanel) bpanel->ClearFileListData();
//}
//int L3DiskRPanel::GetFileListSelectedRow()
//{
//	if (bpanel) return bpanel->GetFileListSelectedRow();
//	else return -2;
//}

L3DiskRawPanel *L3DiskRPanel::GetRawPanel() const
{
	if (bpanel) return bpanel->GetRawPanel();
	else return NULL;
}

//void L3DiskRPanel::SetRawPanelData(DiskD88Disk *disk, int side_num)
//{
//	L3DiskRawPanel *rawpanel = GetRawPanel();
//	if (rawpanel) rawpanel->SetDiskData(disk, side_num);
//}
//void L3DiskRPanel::ClearRawPanelData()
//{
//	L3DiskRawPanel *rawpanel = GetRawPanel();
//	if (rawpanel) rawpanel->ClearDiskData();
//}
//int L3DiskRPanel::GetRawPanelSelectedRow()
//{
//	if (bpanel) return bpanel->GetRawPanelSelectedRow();
//	else return -2;
//}

L3DiskBinDump *L3DiskRPanel::GetBinDumpPanel() const
{
	if (bpanel) return bpanel->GetBinDumpPanel();
	else return NULL;
}

//void L3DiskRPanel::SetBinDumpData(const wxUint8 *buf, size_t len)
//{
//	if (bpanel) bpanel->SetBinDumpData(buf, len);
//}
//void L3DiskRPanel::ClearBinDumpData()
//{
//	if (bpanel) bpanel->ClearBinDumpData();
//}

//void L3DiskRPanel::ClearRBPanelData()
//{
//	if (bpanel) bpanel->ClearData();
//}

//void L3DiskRPanel::ClearData()
//{
//	ClearDiskAttrData();
//	ClearRBPanelData();
//}


//
// 右下パネル
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRBPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

L3DiskRBPanel::L3DiskRBPanel(L3DiskFrame *parentframe, L3DiskRPanel *parentwindow, int selected_window)
                : wxSplitterWindow(parentwindow, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_3D | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
	parent = parentwindow;
    frame = parentframe;

	// fit size
	SetSize(parentwindow->GetSize());

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	filelist = NULL;
	rawpanel = NULL;
	bindump = new L3DiskBinDump(parentframe, this);

	switch(selected_window) {
	case 1:
		rawpanel = new L3DiskRawPanel(parentframe, this);
		SplitVertically(rawpanel, bindump, 500);
		break;
	default:
		filelist = new L3DiskFileList(parentframe, this);
		SplitVertically(filelist, bindump, 500);
		break;
	}

	SetMinimumPaneSize(10);
}

void L3DiskRBPanel::ChangePanel(int num)
{
	switch(num) {
	case 1:
		if (!rawpanel) {
			rawpanel = new L3DiskRawPanel(frame, this);
			ReplaceWindow(filelist, rawpanel);
			delete filelist;
			filelist = NULL;
		}
		break;
	default:
		if (!filelist) {
			filelist = new L3DiskFileList(frame, this);
			ReplaceWindow(rawpanel, filelist);
			delete rawpanel;
			rawpanel = NULL;
		}
		break;
	}
}

#if 0
void L3DiskRBPanel::SetFileListData(DiskD88Disk *disk, int side_num)
{
	if (filelist) filelist->SetFiles(disk, side_num);
}
void L3DiskRBPanel::ClearFileListData()
{
	if (filelist) filelist->ClearFiles();
}
//int L3DiskRBPanel::GetFileListSelectedRow()
//{
//	if (filelist) return filelist->GetSelectedRow();
//	else return -2;
//}

void L3DiskRBPanel::SetBinDumpData(const wxUint8 *buf, size_t len)
{
	if (bindump) bindump->SetDatas(buf, len);
}

void L3DiskRBPanel::ClearBinDumpData()
{
	if (bindump) bindump->ClearDatas();
}

void L3DiskRBPanel::ClearData()
{
	ClearFileListData();
//	ClearRawPanelData();
	ClearBinDumpData();
}
#endif

//
// File Dialog
//
L3DiskFileDialog::L3DiskFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard, long style)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wildcard, style)
{
}

//
//
//
void L3DiskErrorMessageBox(int level, const wxArrayString &msgs)
{
	wxString msg;
	for(size_t i=0; i<msgs.Count(); i++) {
		msg += msgs[i];
		msg += wxT("\n");
	}
	if (msg.IsEmpty()) return;

	wxString caption;
	int style = wxOK;

	if (level < 0) {
		caption = _("Error");
		style |= wxICON_HAND;
	} else if (level > 0) {
		caption = _("Warning");
		style |= wxICON_EXCLAMATION;
	}

	wxMessageBox(msg, caption, style);
}

//
// About dialog
//
L3DiskAbout::L3DiskAbout(wxWindow* parent, wxWindowID id)
	: wxDialog(parent, id, _("About..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrLeft   = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrRight  = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrMain   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *szrAll    = new wxBoxSizer(wxVERTICAL);

	szrLeft->Add(new wxStaticBitmap(this, wxID_ANY,
		wxBitmap(l3diskex_xpm), wxDefaultPosition, wxSize(64, 64))
		, flags);

	wxString str = _T("");
	str += _T("L3DiskExplorer, Version ");
	str += _T(APPLICATION_VERSION);
	str += _T(" \"");
	str += _T(PLATFORM);
	str += _T("\"\n\n");
	str	+= _T("using ");
	str += wxVERSION_STRING;
	str += _T("\n\n");
	str	+= _T(APP_COPYRIGHT);

	szrRight->Add(new wxStaticText(this, wxID_ANY, str), flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK);
	szrMain->Add(szrLeft, flags);
	szrMain->Add(szrRight, flags);
	szrAll->Add(szrMain, flags);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}
