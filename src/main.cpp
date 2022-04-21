/// @file main.cpp
///
/// @brief 本体
///
#include "main.h"
#include <wx/cmdline.h>
#include <wx/filename.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/toolbar.h>
#include <wx/dir.h>
#include "charcodes.h"
#include "diskparambox.h"
#include "basicfmt.h"
#include "uirpanel.h"
#include "uidisklist.h"
#include "uidiskattr.h"
#include "uifilelist.h"
#include "uirawdisk.h"
#include "uibindump.h"
#include "uifatarea.h"
#include "fileparam.h"
#include "fileselbox.h"
#include "diskresult.h"
#include "logging.h"
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
#include "res/foldericon_open.xpm"
#include "res/foldericon_close.xpm"
#include "res/fileicon_normal.xpm"
#include "res/fileicon_delete.xpm"
#include "res/labelicon_normal.xpm"

#define L3DISK_TRANS \
	_("can't open file '%s'") \
	_("can't create file '%s'") \
	_("can't close file descriptor %d") \
	_("can't read from file descriptor %d") \
	_("can't write to file descriptor %d") \
	_("can't flush file descriptor %d") \
	_("can't seek on file descriptor %d") \
	_("can't get seek position on file descriptor %d")

#define MOD_COUNT_MAX 20

wxIMPLEMENT_APP(L3DiskApp);

wxBEGIN_EVENT_TABLE(L3DiskApp, wxApp)
	EVT_IDLE(L3DiskApp::OnAppIdle)
wxEND_EVENT_TABLE()

L3DiskApp::L3DiskApp() : mLocale(wxLANGUAGE_DEFAULT)
{
	frame = NULL;
	mod_keys = 0;
	mod_cnt = 0;
}

/// 初期処理
bool L3DiskApp::OnInit()
{
	SetAppPath();
	SetAppName(_T("l3diskex"));

	// log file
	myLog.Open(ini_path, GetAppName(), _T(".log"));

	// load ini file
	mConfig.Load(ini_path + GetAppName() + _T(".ini"));

	// set locale search path and catalog name
	mLocale.AddCatalogLookupPathPrefix(res_path + _T("lang"));
	mLocale.AddCatalogLookupPathPrefix(_T("lang"));
	mLocale.AddCatalog(_T("l3diskex"));

	if (!wxApp::OnInit()) {
		return false;
	}

	wxString locale_name = mLocale.IsLoaded(_T("l3diskex")) ? mLocale.GetCanonicalName() : wxT("");

	// load xml
	if (!gDiskTypes.Load(res_path + wxT("data/"), locale_name)) {
		wxMessageBox(_("Cannot load disk types data file."), _("Error"), wxOK);
		return false;
	}
	if (!gDiskBasicTemplates.Load(res_path + wxT("data/"), locale_name)) {
		wxMessageBox(_("Cannot load disk basic types data file."), _("Error"), wxOK);
		return false;
	}
	if (!gCharCodeMaps.Load(res_path + wxT("data/"))) {
		wxMessageBox(_("Cannot load char codes data file."), _("Error"), wxOK);
		return false;
	}
	if (!gFileTypes.Load(res_path + wxT("data/"), locale_name)) {
		wxMessageBox(_("Cannot load file types data file."), _("Error"), wxOK);
		return false;
	}

	frame = new L3DiskFrame(GetAppName(), wxSize(1000, 600));
	frame->Show(true);
	SetTopWindow(frame);

	if (!frame->Init(in_file)) {
		return false;
	}

	return true;
}

#define OPTION_VERBOSE "verbose"

/// コマンドラインの解析
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

/// コマンドラインの解析完了
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

/// 終了処理
int L3DiskApp::OnExit()
{
	// save ini file
	mConfig.Save();
	// remove temp directories
	RemoveTempDirs();

	return 0;
}

/// アイドル時の処理
void L3DiskApp::OnAppIdle(wxIdleEvent& event)
{
	if (mod_cnt > 0) {
		mod_cnt--;
	}
	// 一定時間、キー入力がなければ修飾キー押下状態をクリア
	if (mod_cnt == 0) {
		mod_keys = 0;
	}
}

/// イベント強制取得
int L3DiskApp::FilterEvent(wxEvent& event)
{
	const wxEventType t = event.GetEventType();
	if (t == wxEVT_KEY_DOWN || t == wxEVT_KEY_UP) {
		mod_keys = ((wxKeyEvent &)event).GetModifiers();
		mod_cnt = MOD_COUNT_MAX;
	}
	// Continue processing the event normally as well.
	return Event_Skip;
}

/// ファイルを開く(Mac用)
void L3DiskApp::MacOpenFile(const wxString &fileName)
{
	if (frame) {
		frame->PreOpenDataFile(fileName);
	}
}

/// アプリケーションのパスを設定
void L3DiskApp::SetAppPath()
{
	app_path = wxFileName::FileName(argv[0]).GetPath(wxPATH_GET_SEPARATOR);
#ifdef __WXOSX__
	if (app_path.Find(_T("MacOS")) >= 0) {
		wxFileName file = wxFileName::FileName(app_path+"../../../");
		file.Normalize();
		ini_path = file.GetPath(wxPATH_GET_SEPARATOR);
		file = wxFileName::FileName(app_path+"../../Contents/Resources/");
		file.Normalize();
		res_path = file.GetPath(wxPATH_GET_SEPARATOR);
	} else
#endif
	{
		ini_path = app_path;
		res_path = app_path;
	}
}

/// アプリケーションのパスを返す
const wxString &L3DiskApp::GetAppPath()
{
	return app_path;
}

/// 設定ファイルのあるパスを返す
const wxString &L3DiskApp::GetIniPath()
{
	return ini_path;
}

/// リソースファイルのあるパスを返す
const wxString &L3DiskApp::GetResPath()
{
	return res_path;
}

/// 設定データへのポインタを返す
Config *L3DiskApp::GetConfig()
{
	return &mConfig;
}

/// テンポラリディレクトリを作成する
bool L3DiskApp::MakeTempDir(wxString &tmp_dir_path)
{
	tmp_dir_path = wxFileName::CreateTempFileName(wxGetApp().GetAppName());
	// 上記でファイルができてしまうので削除
	if (wxFileExists(tmp_dir_path)) {
		wxRemoveFile(tmp_dir_path);
	}
	// ディレクトリを作成
	if (!wxDir::Make(tmp_dir_path)) {
		return false;
	}
	// ファイルパスは保存
	tmp_dirs.Add(tmp_dir_path);
	return true;
}

/// テンポラリディレクトリを削除する
void L3DiskApp::RemoveTempDir(const wxString &tmp_dir_path)
{
	if (tmp_dir_path.IsEmpty()) return;

	// テンポラリ内のファイル削除
	wxDir dir(tmp_dir_path);
	wxString file_name;
	bool sts = dir.GetFirst(&file_name, wxEmptyString, wxDIR_FILES );
	while(sts) {
		wxFileName file_path(tmp_dir_path, file_name);
		wxRemoveFile(file_path.GetFullPath());
		sts = dir.GetNext(&file_name);
	}
	dir.Close();

	// テンポラリディレクトリを削除
	wxDir::Remove(tmp_dir_path);

	// リストからも削除
	tmp_dirs.Remove(tmp_dir_path);
}

/// テンポラリディレクトリをすべて削除する
void L3DiskApp::RemoveTempDirs()
{
	wxArrayString dirs = tmp_dirs;
	for(size_t n = 0; n < dirs.Count(); n++) {
		RemoveTempDir(dirs.Item(n));
	}
}

//
// Frame
//
static const int IDT_TOOLBAR = 500;
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

	EVT_MENU_RANGE(IDM_RECENT_FILE_0, IDM_RECENT_FILE_0 + MAX_RECENT_FILES - 1, L3DiskFrame::OnOpenRecentFile)

	EVT_MENU(IDM_EXPORT_DISK, L3DiskFrame::OnExportFileFromDisk)
	EVT_MENU(IDM_IMPORT_DISK, L3DiskFrame::OnImportFileToDisk)
	EVT_MENU(IDM_DELETE_DISK, L3DiskFrame::OnDeleteFileFromDisk)
	EVT_MENU(IDM_RENAME_FILE_ON_DISK, L3DiskFrame::OnRenameFileOnDisk)
	EVT_MENU(IDM_INITIALIZE_DISK, L3DiskFrame::OnInitializeDisk)
	EVT_MENU(IDM_FORMAT_DISK, L3DiskFrame::OnFormatDisk)
	EVT_MENU(IDM_PROPERTY_DISK, L3DiskFrame::OnPropertyOnDisk)

	EVT_MENU(IDM_BASIC_MODE, L3DiskFrame::OnBasicMode)
	EVT_MENU(IDM_RAWDISK_MODE, L3DiskFrame::OnRawDiskMode)

	EVT_MENU(IDM_WINDOW_BINDUMP, L3DiskFrame::OnOpenBinDump)
	EVT_MENU(IDM_WINDOW_FATAREA, L3DiskFrame::OnOpenFatArea)

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
	menuWindow = new wxMenu;
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
	menuDisk->Append( IDM_MAKE_DIRECTORY_ON_DISK, _("&Make Directory...") );
	menuDisk->AppendSeparator();
	menuDisk->Append( IDM_PROPERTY_DISK, _("&Property") );
	// mode menu
	menuMode->AppendRadioItem( IDM_BASIC_MODE, _("BASIC Mode") );
	menuMode->AppendRadioItem( IDM_RAWDISK_MODE, _("Raw Disk Mode") );
	// window menu
	menuWindow->AppendCheckItem( IDM_WINDOW_BINDUMP, _("&Dump Window") );
	menuWindow->AppendCheckItem( IDM_WINDOW_FATAREA, _("&FAT Area Window") );
	// help menu
	menuHelp->Append( wxID_ABOUT, _("&About...") );

	// menu bar
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, _("&File") );
	menuBar->Append( menuDisk, _("&Disk") );
	menuBar->Append( menuMode, _("&Mode") );
	menuBar->Append( menuWindow, _("&Window") );
	menuBar->Append( menuHelp, _("&Help") );

	SetMenuBar( menuBar );

	// tool bar
    RecreateToolbar();

	// splitter window
	panel = new L3DiskPanel(this);

	bindump_frame = NULL;
	fatarea_frame = NULL;
}

L3DiskFrame::~L3DiskFrame()
{
}

/// フレーム部の初期処理
bool L3DiskFrame::Init(const wxString &in_file)
{
	bool valid = false;

	if (!in_file.IsEmpty()) {
		valid = OpenDataFile(in_file, wxEmptyString, wxEmptyString);
	}
	if (!valid) {
		// 起動時にファイルを開いたときはもう更新している
		UpdateMenuFile();
		UpdateMenuDisk();
		UpdateToolBar();
	}
	return true;
}

/// ツールバーの再生成
void L3DiskFrame::RecreateToolbar()
{
	// delete and recreate the toolbar
	wxToolBar *toolBar = GetToolBar();
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

/// ツールバーの構築
void L3DiskFrame::PopulateToolbar(wxToolBar* toolBar)
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
void L3DiskFrame::OpenDroppedFile(const wxString &path)
{
 	if (!CloseDataFile()) return;
	OpenDataFile(path, wxEmptyString, wxEmptyString);
}

/// ウィンドウを閉じたとき
void L3DiskFrame::OnClose(wxCloseEvent& event)
{
	if (!CloseDataFile(!event.CanVeto())) {
		event.Veto();
		return;
	}
	event.Skip();
}

/// メニュー 終了選択
void L3DiskFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(false);
}

/// メニュー Aboutダイアログ表示選択
void L3DiskFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	L3DiskAbout(this, wxID_ANY).ShowModal();
}

/// メニュー 新規作成選択
void L3DiskFrame::OnCreateFile(wxCommandEvent& WXUNUSED(event))
{
	ShowCreateFileDialog();
}

/// メニュー 開く選択
void L3DiskFrame::OnOpenFile(wxCommandEvent& WXUNUSED(event))
{
	ShowOpenFileDialog();
}

/// メニュー 最近使用したファイル開く選択
void L3DiskFrame::OnOpenRecentFile(wxCommandEvent& event)
{
	wxMenuItem *item = menuRecentFiles->FindItem(event.GetId());
	if (!item) return;
	wxFileName path = item->GetItemLabel();
	if (!CloseDataFile()) return;
	PreOpenDataFile(path.GetFullPath());
}

/// メニュー 閉じる選択
void L3DiskFrame::OnCloseFile(wxCommandEvent& WXUNUSED(event))
{
	CloseDataFile();
}
/// メニュー 名前を付けて保存選択
void L3DiskFrame::OnSaveAsFile(wxCommandEvent& WXUNUSED(event))
{
	ShowSaveFileDialog();
}
/// メニュー ディスク1枚を保存選択
void L3DiskFrame::OnSaveDisk(wxCommandEvent& WXUNUSED(event))
{
	L3DiskList *list = GetDiskListPanel();
	if (!list) return;
	ShowSaveDiskDialog(list->GetSelectedDiskNumber(), list->GetSelectedDiskSide());
}
/// メニュー ディスクを新規に追加選択
void L3DiskFrame::OnAddNewDisk(wxCommandEvent& WXUNUSED(event))
{
	ShowAddNewDiskDialog();
}
/// メニュー ディスクをファイルから追加選択
void L3DiskFrame::OnAddDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	ShowAddFileDialog();
}
/// メニュー ディスクを置換選択
void L3DiskFrame::OnReplaceDisk(wxCommandEvent& WXUNUSED(event))
{
	L3DiskList *list = GetDiskListPanel();
	if (!list) return;
	ShowReplaceDiskDialog(list->GetSelectedDiskNumber(), list->GetSelectedDiskSide());
}
/// メニュー ファイルからディスクを削除選択
void L3DiskFrame::OnDeleteDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	DeleteDisk();
}
/// メニュー ディスク名を変更選択
void L3DiskFrame::OnRenameDisk(wxCommandEvent& WXUNUSED(event))
{
	RenameDisk();
}

/// メニュー エクスポート選択
void L3DiskFrame::OnExportFileFromDisk(wxCommandEvent& WXUNUSED(event))
{
	ExportFileFromDisk();
}
/// メニュー インポート選択
void L3DiskFrame::OnImportFileToDisk(wxCommandEvent& WXUNUSED(event))
{
	ImportFileToDisk();
}
/// メニュー 削除選択
void L3DiskFrame::OnDeleteFileFromDisk(wxCommandEvent& WXUNUSED(event))
{
	DeleteFileFromDisk();
}
/// メニュー リネーム選択
void L3DiskFrame::OnRenameFileOnDisk(wxCommandEvent& WXUNUSED(event))
{
	RenameFileOnDisk();
}
/// メニュー 初期化選択
void L3DiskFrame::OnInitializeDisk(wxCommandEvent& WXUNUSED(event))
{
	InitializeDisk();
}
/// メニュー フォーマット選択
void L3DiskFrame::OnFormatDisk(wxCommandEvent& WXUNUSED(event))
{
	FormatDisk();
}
/// メニュー プロパティ選択
void L3DiskFrame::OnPropertyOnDisk(wxCommandEvent& WXUNUSED(event))
{
	PropertyOnDisk();
}

/// メニュー 設定ダイアログ選択
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

/// ダンプウィンドウ選択
void L3DiskFrame::OnOpenBinDump(wxCommandEvent& event)
{
	if (!bindump_frame) {
		// ウィンドウを開く
		OpenBinDumpWindow();
	} else {
		// ウィンドウを閉じる
		CloseBinDumpWindow();
	}
}

/// FAT使用状況ウィンドウ選択
void L3DiskFrame::OnOpenFatArea(wxCommandEvent& event)
{
	if (!fatarea_frame) {
		// ウィンドウを開く
		OpenFatAreaWindow();
	} else {
		// ウィンドウを閉じる
		CloseFatAreaWindow();
	}
}

/// 右パネルのデータウィンドウを変更 ファイルリスト/RAWディスク
void L3DiskFrame::ChangeRPanel(int num)
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) rpanel->ChangePanel(num);
	L3DiskList *lpanel = panel->GetLPanel();
	if (lpanel) lpanel->ReSelect();

	UpdateMenuDisk();
}

/// 選択しているModeメニュー BASICかRAW DISKか
/// @retval 0 BASIC
/// @retval 1 RAW DISK
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

/// 左パネルを返す
L3DiskList *L3DiskFrame::GetLPanel()
{
	return panel->GetLPanel();
}
/// 左パネルのディスクリストを返す
L3DiskList *L3DiskFrame::GetDiskListPanel()
{
	return panel->GetLPanel();
}
/// 左パネルのディスクリストにデータを設定する
void L3DiskFrame::SetDiskListData()
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->SetFileName();
}
/// 左パネルのディスクリストをクリア
void L3DiskFrame::ClearDiskListData()
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
/// 左パネルのディスクリストのディスクを選択しているか
bool L3DiskFrame::IsDiskListSelectedDisk()
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) return lpanel->IsSelectedDisk();
	else return false;
}

/// 左パネルのディスクリストの選択している位置
/// @param [out] disk_number ディスク番号
/// @param [out] side_number サイド番号
void L3DiskFrame::GetDiskListSelectedPos(int &disk_number, int &side_number)
{
	L3DiskList *lpanel = GetDiskListPanel();
	int num = -1;
	int sid = -1;
	if (lpanel) {
		num = lpanel->GetSelectedDiskNumber();
		sid = lpanel->GetSelectedDiskSide();
	}
	disk_number = num;
	side_number = sid;
}

/// 左パネルのディスクリストを選択
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
void L3DiskFrame::SetDiskListPos(int disk_number, int side_number)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->ChangeSelection(disk_number, side_number);
	}
}

//

/// 右パネルを返す
L3DiskRPanel *L3DiskFrame::GetRPanel()
{
	return panel->GetRPanel();
}
/// 右パネルのすべてのコントロール内のデータをクリア
void L3DiskFrame::ClearRPanelData()
{
	ClearDiskAttrData();
	ClearFileListData();
	ClearRawPanelData();
	ClearBinDumpData();
	ClearFatAreaData();
}

/// 右上パネルのディスク属性パネルを返す
L3DiskDiskAttr *L3DiskFrame::GetDiskAttrPanel()
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetDiskAttrPanel();
	else return NULL;
}
/// 右上パネルのディスク属性にデータを設定する
void L3DiskFrame::SetDiskAttrData(DiskD88Disk *disk)
{
	L3DiskDiskAttr *dapanel = GetDiskAttrPanel();
	if (dapanel) dapanel->SetAttr(disk);
}
/// 右上パネルのディスク属性をクリア
void L3DiskFrame::ClearDiskAttrData()
{
	L3DiskDiskAttr *dapanel = GetDiskAttrPanel();
	if (dapanel) dapanel->ClearData();
}

/// 右下パネルのファイルリストパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
L3DiskFileList *L3DiskFrame::GetFileListPanel(bool inst)
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetFileListPanel(inst);
	else return NULL;
}
/// 右下パネルのファイルリストにデータを設定する
void L3DiskFrame::SetFileListData(DiskD88Disk *disk, int side_num)
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->SetFiles(disk, side_num);
}
/// 右下パネルのファイルリストをクリア
void L3DiskFrame::ClearFileListData()
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->ClearFiles();
}
#if 0
/// 右下パネルのファイルリストの選択行を返す
int L3DiskFrame::GetFileListSelectedRow()
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetFileListSelectedRow();
	else return -2;
}
#endif

/// ファイル名属性プロパティダイアログをすべて閉じる
void L3DiskFrame::CloseAllFileAttr()
{
	L3DiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->CloseAllFileAttr();
}

/// 右下パネルのRAWディスクパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
L3DiskRawPanel *L3DiskFrame::GetDiskRawPanel(bool inst)
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetRawPanel(inst);
	else return NULL;
}
/// 右下パネルのRAWディスクパネルにデータを設定する
void L3DiskFrame::SetRawPanelData(DiskD88Disk *disk, int side_num)
{
	L3DiskRawPanel *rawpanel = GetDiskRawPanel();
	if (rawpanel) rawpanel->SetTrackListData(disk, side_num);
}
/// 右下パネルのRAWディスクパネルをクリア
void L3DiskFrame::ClearRawPanelData()
{
	L3DiskRawPanel *rawpanel = GetDiskRawPanel();
	if (rawpanel) rawpanel->ClearTrackListData();
}
#if 0
/// 右下パネルのRAWディスクパネルの選択行を返す
int L3DiskFrame::GetRawPanelSelectedRow()
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetRawPanelSelectedRow();
	else return -2;
}
#endif

#if 0
/// バイナリダンプ
L3DiskBinDump *L3DiskFrame::GetBinDumpPanel()
{
//	L3DiskRPanel *rpanel = panel->GetRPanel();
//	if (rpanel) return rpanel->GetBinDumpPanel();
//	else return NULL;
	if (bindump_frame) return bindump_frame->GetPanel();
	else return NULL;
}
#endif
/// ダンプウィンドウにデータを設定する
void L3DiskFrame::SetBinDumpData(const wxUint8 *buf, size_t len, int char_code, bool invert)
{
//	L3DiskBinDump *binpanel = GetBinDumpPanel();
//	if (binpanel) binpanel->SetDatas(buf, len, invert);
	if (bindump_frame) {
		bindump_frame->SetDataInvert(invert);
		bindump_frame->SetDataChar(char_code);
		L3DiskBinDump *binpanel = bindump_frame->GetDumpPanel();
		if (binpanel) binpanel->SetDatas(buf, len);
	}
}
/// ダンプウィンドウにデータを設定する
void L3DiskFrame::SetBinDumpData(const wxUint8 *buf, size_t len)
{
//	L3DiskBinDump *binpanel = GetBinDumpPanel();
//	if (binpanel) binpanel->SetDatas(buf, len, invert);
	if (bindump_frame) {
		L3DiskBinDump *binpanel = bindump_frame->GetDumpPanel();
		if (binpanel) binpanel->SetDatas(buf, len);
	}
}
/// ダンプウィンドウにデータを追記する
void L3DiskFrame::AppendBinDumpData(const wxUint8 *buf, size_t len, int char_code, bool invert)
{
//	L3DiskBinDump *binpanel = GetBinDumpPanel();
//	if (binpanel) binpanel->AppendDatas(buf, len, invert);
	if (bindump_frame) {
		bindump_frame->SetDataInvert(invert);
		bindump_frame->SetDataChar(char_code);
		L3DiskBinDump *binpanel = bindump_frame->GetDumpPanel();
		if (binpanel) binpanel->AppendDatas(buf, len);
	}
}
/// ダンプウィンドウにデータを追記する
void L3DiskFrame::AppendBinDumpData(const wxUint8 *buf, size_t len)
{
//	L3DiskBinDump *binpanel = GetBinDumpPanel();
//	if (binpanel) binpanel->AppendDatas(buf, len, invert);
	if (bindump_frame) {
		L3DiskBinDump *binpanel = bindump_frame->GetDumpPanel();
		if (binpanel) binpanel->AppendDatas(buf, len);
	}
}
/// ダンプウィンドウをクリア
void L3DiskFrame::ClearBinDumpData()
{
//	L3DiskBinDump *binpanel = GetBinDumpPanel();
//	if (binpanel) binpanel->ClearDatas();
	if (bindump_frame) {
		L3DiskBinDump *binpanel = bindump_frame->GetDumpPanel();
		if (binpanel) binpanel->ClearDatas();
	}
}

/// ダンプウィンドウを開く
void L3DiskFrame::OpenBinDumpWindow()
{
	if (bindump_frame) return;

	// ウィンドウを開く
	bindump_frame = new L3DiskBinDumpFrame(this, _("Binary Dump"), wxSize(640, 480));
	// 位置はメインウィンドウの右側
	wxSize sz = GetSize();
	wxPoint pt = GetPosition();
	pt.x = pt.x + sz.x;
	pt.y = pt.y;
	bindump_frame->SetPosition(pt);
	bindump_frame->Show();
	bindump_frame->SetFocus();
}
/// ダンプウィンドウを閉じる
void L3DiskFrame::CloseBinDumpWindow()
{
	if (!bindump_frame) return;

	bindump_frame->Close();
	bindump_frame = NULL;
}
/// ダンプウィンドウを閉じる時にウィンドウ側から呼ばれるコールバック
void L3DiskFrame::BinDumpWindowClosed()
{
	bindump_frame = NULL;

	if (!IsBeingDeleted()) {
		wxMenuItem *mitem = menuWindow->FindItem(IDM_WINDOW_BINDUMP);
		if (mitem) {
			mitem->Check(false);
		}
	}
}

/// FAT使用状況ウィンドウにデータを設定する
void L3DiskFrame::SetFatAreaData()
{
	if (fatarea_frame) {
		wxUint32 offset = 0;
		const wxArrayInt *arr = NULL;
		L3DiskFileList *list = GetFileListPanel();
		if (list) {
			list->GetFatAvailability(&offset, &arr);
			SetFatAreaData(offset, arr);
		}
	}
}

/// FAT使用状況ウィンドウにデータを設定する
void L3DiskFrame::SetFatAreaData(wxUint32 offset, const wxArrayInt *arr)
{
	if (fatarea_frame && arr) {
		fatarea_frame->SetData(offset, arr);
	}
}
/// FAT使用状況ウィンドウをクリア
void L3DiskFrame::ClearFatAreaData()
{
	if (fatarea_frame) {
		fatarea_frame->ClearData();
	}
}
/// FAT使用状況ウィンドウにフォーカスさせるグループ番号を設定する
void L3DiskFrame::SetFatAreaGroup(wxUint32 group_num)
{
	if (fatarea_frame) {
		fatarea_frame->SetGroup(group_num);
	}
}
/// FAT使用状況ウィンドウにフォーカスさせるグループ番号を設定する
void L3DiskFrame::SetFatAreaGroup(const DiskBasicGroups *group_items, wxUint32 extra_group_num)
{
	if (fatarea_frame) {
		fatarea_frame->SetGroup(group_items, extra_group_num);
	}
}
/// FAT使用状況ウィンドウでフォーカスしているグループ番号をクリア
void L3DiskFrame::ClearFatAreaGroup()
{
	if (fatarea_frame) {
		fatarea_frame->ClearGroup();
	}
}
/// FAT使用状況ウィンドウを開く
void L3DiskFrame::OpenFatAreaWindow()
{
	if (fatarea_frame) return;

	// ウィンドウを開く
	fatarea_frame = new L3DiskFatAreaFrame(this, _("FAT Availability"), wxDefaultSize);
	// 位置はメインウィンドウの右側
	wxSize sz = GetSize();
	wxPoint pt = GetPosition();
	pt.x = pt.x + sz.x;
	pt.y = pt.y + sz.y - 64;
	fatarea_frame->SetPosition(pt);
	fatarea_frame->Show();
	fatarea_frame->SetFocus();

	SetFatAreaData();
}
/// FAT使用状況ウィンドウを閉じる
void L3DiskFrame::CloseFatAreaWindow()
{
	if (!fatarea_frame) return;

	fatarea_frame->Close();
	fatarea_frame = NULL;
}
/// FAT使用状況ウィンドウを閉じる時にウィンドウ側から呼ばれるコールバック
void L3DiskFrame::FatAreaWindowClosed()
{
	fatarea_frame = NULL;

	if (!IsBeingDeleted()) {
		wxMenuItem *mitem = menuWindow->FindItem(IDM_WINDOW_FATAREA);
		if (mitem) {
			mitem->Check(false);
		}
	}
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
	for(int i=0; i<MAX_RECENT_FILES && i<(int)names.Count(); i++) {
		if (menuRecentFiles->FindItem(IDM_RECENT_FILE_0 + i)) menuRecentFiles->Delete(IDM_RECENT_FILE_0 + i);
		menuRecentFiles->Append(IDM_RECENT_FILE_0 + i, names[i]);
	}
}

/// ツールバーを更新
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

/// メニューのディスク項目を更新
void L3DiskFrame::UpdateMenuDiskList(L3DiskList *list)
{
	bool opened = (list != NULL && list->IsSelectedDiskImage());
	menuFile->Enable(IDM_REPLACE_DISK_FROM_FILE, opened);
	menuFile->Enable(IDM_SAVE_DISK, opened);
	menuFile->Enable(IDM_DELETE_DISK_FROM_FILE, opened);
	menuFile->Enable(IDM_RENAME_DISK, opened);
	menuFile->Enable(IDM_INITIALIZE_DISK, opened);
}

/// ツールバーのディスク項目を更新
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

/// メニューとツールバーのディスク項目を更新
void L3DiskFrame::UpdateMenuAndToolBarDiskList(L3DiskList *list)
{
	UpdateMenuDiskList(list);
	UpdateToolBarDiskList(list);
}

/// メニューのファイルリスト項目を更新
void L3DiskFrame::UpdateMenuFileList(L3DiskFileList *list)
{
	bool opened = (list != NULL && list->CanUseBasicDisk());
	menuFile->Enable(IDM_FORMAT_DISK, opened);

	menuDisk->Enable(IDM_MAKE_DIRECTORY_ON_DISK, opened && list->CanMakeDirectory());

	opened = (opened && list->IsFormattedBasicDisk());
	menuDisk->Enable(IDM_IMPORT_DISK, opened /* && list->IsWritableBasicFile() */);

	int	cnt = list->GetSelectedItemCount();
	opened = (opened && cnt > 0);
	menuDisk->Enable(IDM_EXPORT_DISK, opened);
	menuDisk->Enable(IDM_DELETE_DISK, opened /* && list->IsDeletableBasicFile() */);
	opened = (opened && cnt == 1);
	menuDisk->Enable(IDM_RENAME_FILE_ON_DISK, opened);
	menuDisk->Enable(IDM_PROPERTY_DISK, opened);
}

/// ツールバーのファイルリスト項目を更新
void L3DiskFrame::UpdateToolBarFileList(L3DiskFileList *list)
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (list && list->IsFormattedBasicDisk());
	toolBar->EnableTool(IDM_IMPORT_DISK, opened /* && list->IsWritableBasicFile() */);

	int	cnt = list->GetSelectedItemCount();
	opened = (opened && cnt > 0);
	toolBar->EnableTool(IDM_EXPORT_DISK, opened);
	toolBar->EnableTool(IDM_DELETE_DISK, opened);
//	toolBar->EnableTool(IDM_RENAME_FILE_ON_DISK, opened);
//	toolBar->EnableTool(IDM_PROPERTY_DISK, opened);
}

/// メニューとツールバーのファイルリスト項目を更新
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

	menuDisk->Enable(IDM_RENAME_FILE_ON_DISK, false);
}

/// ツールバーの生ディスク項目を更新
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

/// メニューとツールバーの生ディスク項目を更新
void L3DiskFrame::UpdateMenuAndToolBarRawDisk(L3DiskRawPanel *rawpanel)
{
	UpdateMenuRawDisk(rawpanel);
	UpdateToolBarRawDisk(rawpanel);
}

/// タイトル名を設定
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

/// タイトル名を返す
wxString L3DiskFrame::GetFileName()
{
	return MakeTitleName(d88.GetFileName());
}

/// 最近使用したパスを取得
const wxString &L3DiskFrame::GetRecentPath() const
{
	return ini->GetFilePath();
}

/// 最近使用したパスを取得(エクスポート用)
const wxString &L3DiskFrame::GetExportFilePath() const
{
	return ini->GetExportFilePath();
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

/// 最近使用したパスを更新(エクスポート用)
void L3DiskFrame::SetExportFilePath(const wxString &path)
{
	ini->SetExportFilePath(path);
}

/// ウィンドウ上のデータを更新 タイトルバーにファイルパスを表示
void L3DiskFrame::UpdateDataOnWindow(const wxString &path, bool keep)
{
	// update window
	if (!path.IsEmpty()) {
		wxString title = wxGetApp().GetAppName() + wxT(" - ") + path;
		SetTitle(title);
	}
	UpdateDataOnWindow(keep);
}

/// ウィンドウ上のデータを更新
void L3DiskFrame::UpdateDataOnWindow(bool keep)
{
	int dl_num = -1;
	int dl_sid = -1;
	if (keep) {
		// get current position
		GetDiskListSelectedPos(dl_num, dl_sid);

	}

	// update panel
	SetDiskListData();
	ClearRPanelData();

	if (keep) {
		// set position
		SetDiskListPos(dl_num, dl_sid);
	}

#ifndef USE_MENU_OPEN
	// update menu
	UpdateMenuFile();
#endif
}

/// 新規作成ダイアログ
void L3DiskFrame::ShowCreateFileDialog()
{
	if (!CloseDataFile()) return;

	DiskParamBox dlg(this, wxID_ANY, _("Create New Disk"), 0, NULL, true);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		DiskParam param;
		dlg.GetParam(param);
		CreateDataFile(dlg.GetDiskName(), param, dlg.IsWriteProtected(), dlg.GetCategory());
	}
}
/// 新規作成
/// @param [in] diskname      ディスク名
/// @param [in] param         ディスクパラメータ
/// @param [in] write_protect 書き込み禁止
/// @param [in] basic_hint    DISK BASIC種類のヒント
void L3DiskFrame::CreateDataFile(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint)
{
	wxString path = MakeTitleName(wxEmptyString);

	// create disk
	int rc = d88.Create(diskname, param, write_protect, basic_hint);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(path, false);
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
	}
	UpdateToolBar();
}
/// ディスク新規追加ダイアログ
void L3DiskFrame::ShowAddNewDiskDialog()
{
	if (!d88.GetFile()) return;

	DiskParamBox dlg(this, wxID_ANY, _("Add New Disk"), d88.GetDiskTypeNumber(0), NULL, true);

	int rc = dlg.ShowModal();
	if (rc == wxID_OK) {
		DiskParam param;
		dlg.GetParam(param);
		AddNewDisk(dlg.GetDiskName(), param, dlg.IsWriteProtected(), dlg.GetCategory());
	}
}
/// ディスクを追加
/// @param [in] diskname      ディスク名
/// @param [in] param         ディスクパラメータ
/// @param [in] write_protect 書き込み禁止
/// @param [in] basic_hint    DISK BASIC種類のヒント
void L3DiskFrame::AddNewDisk(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint)
{
	if (!d88.GetFile()) return;

	// add a disk
	int rc = d88.Add(diskname, param, write_protect, basic_hint);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString, false);
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
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
		gFileTypes.GetWildcardForLoad(),
		wxFD_OPEN);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		PreOpenDataFile(path);
	}
}
/// 拡張子でファイル種別を判別する オープン時
void L3DiskFrame::PreOpenDataFile(const wxString &path)
{
	wxFileName file_path(path);

	FileParam *fitem = gFileTypes.FindExt(file_path.GetExt());
	if (!fitem) {
		// 不明の拡張子
		ShowFileSelectDialogForOpen(path);
	} else {
		// 既知の拡張子
		OpenDataFile(path, wxEmptyString, wxEmptyString);
	}
}
/// 指定したディスクイメージを開く
/// @param [in] path        ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] disk_type   ディスクパラメータ種類
bool L3DiskFrame::OpenDataFile(const wxString &path, const wxString &file_format, const wxString &disk_type)
{
	bool valid = false;

	// set recent file path
	SetRecentPath(path);

	// open disk
	int rc = d88.Open(path, file_format, disk_type);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(path, false);
		valid = true;
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
	}
	UpdateToolBar();

	return valid;
}

/// ファイル追加ダイアログ
void L3DiskFrame::ShowAddFileDialog()
{
	L3DiskFileDialog dlg(
		_("Add File"),
		GetRecentPath(),
		wxEmptyString,
		gFileTypes.GetWildcardForLoad(),
/*		_("Supported files (*.d88;*.d77)|*.d88;*.d77|All files (*.*)|*.*"), */
		wxFD_OPEN);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		PreAddDiskFile(path);
	}
}
/// 拡張子でファイル種別を判別する 追加時
void L3DiskFrame::PreAddDiskFile(const wxString &path)
{
	wxFileName file_path(path);

	FileParam *fitem = gFileTypes.FindExt(file_path.GetExt());
	if (!fitem) {
		// 不明の拡張子
		ShowFileSelectDialogForAdd(path);
	} else {
		// 既知の拡張子
		AddDiskFile(path, wxEmptyString, wxEmptyString);
	}
}

/// 指定したファイルを追加
/// @param [in] path        ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] disk_type   ディスクパラメータ種類
void L3DiskFrame::AddDiskFile(const wxString &path, const wxString &file_format, const wxString &disk_type)
{
	// set recent file path
	SetRecentPath(path);

	// open disk
	int rc = d88.Add(path, file_format, disk_type);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString, false);
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
	}
}

/// ファイル種類選択ダイアログ オープン時
void L3DiskFrame::ShowFileSelectDialogForOpen(const wxString &path)
{
	wxString file_format, disk_type;
	if (ShowFileSelectDialog(path, file_format, disk_type)) {
		if (!CloseDataFile()) return;
		OpenDataFile(path, file_format, disk_type);
	}
}

/// ファイル種類選択ダイアログ 追加時
void L3DiskFrame::ShowFileSelectDialogForAdd(const wxString &path)
{
	wxString file_format, disk_type;
	if (ShowFileSelectDialog(path, file_format, disk_type)) {
		AddDiskFile(path, file_format, disk_type);
	}
}

/// ファイル種類選択ダイアログ
/// @param [in] path        ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] disk_type   ディスクパラメータ種類
/// @return true OKボタンを押した
bool L3DiskFrame::ShowFileSelectDialog(const wxString &path, wxString &file_format, wxString &disk_type)
{
	DiskParam disk_param;
	FileSelBox dlg1(this, wxID_ANY);
	int sts = wxID_CANCEL;
	do {
		sts = dlg1.ShowModal();
		if (sts == wxID_OK) {
			wxString name = dlg1.GetFormatType();
			if (name == wxT("plain")) {
				// パラメータを選択
				DiskParamBox dlg2(this, wxID_ANY, _("Select Disk Type"), 0, NULL, true);
				int sts2 = dlg2.ShowModal();
				if (sts2 != wxID_OK) {
					continue;
				}
				sts = sts2;
				dlg2.GetParam(disk_param);
				disk_type = disk_param.GetDiskTypeName();
			}
			file_format = dlg1.GetFormatType();
		}
	} while (sts != wxID_OK && sts != wxID_CANCEL);

	return (sts == wxID_OK);
}

/// ファイルを閉じる
/// @param [in] force 強制 (確認をしない)
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
	ClearDiskListData();
	ClearRPanelData();

#ifndef USE_MENU_OPEN
	// update menu
	UpdateMenuFile();
#endif
	UpdateToolBar();
	
	// プロパティウィンドウを閉じる
	CloseAllFileAttr();

	return true;
}
/// 保存ダイアログ
void L3DiskFrame::ShowSaveFileDialog()
{
	L3DiskFileDialog dlg(
		_("Save File"),
		d88.GetPath(),
		d88.GetFileName(),
		gFileTypes.GetWildcardForSave(),
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
		UpdateDataOnWindow(path, true);
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
	}
}
/// ディスクをファイルに保存ダイアログ（指定ディスク）
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
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
		gFileTypes.GetWildcardForSave(),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		SaveDataDisk(disk_number, side_number, path);
	}
}
/// 指定したファイルに保存（指定ディスク）
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] path        ファイルパス
void L3DiskFrame::SaveDataDisk(int disk_number, int side_number, const wxString &path)
{
	// set recent file path
	SetRecentPath(path);

	// save disk
	int rc = d88.SaveDisk(disk_number, side_number, path);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString, true);
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
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
		gFileTypes.GetWildcardForLoad(),
		wxFD_OPEN);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (rc == wxID_OK) {
		PreReplaceDisk(disk_number, side_number, path);
	}
}
/// 拡張子でファイル種別を判別する 置換時
void L3DiskFrame::PreReplaceDisk(int disk_number, int side_number, const wxString &path)
{
	wxFileName file_path(path);

	FileParam *fitem = gFileTypes.FindExt(file_path.GetExt());
	if (!fitem) {
		// 不明の拡張子
		ShowFileSelectDialogForReplace(disk_number, side_number, path);
	} else {
		// 既知の拡張子
		ReplaceDisk(disk_number, side_number, path, wxEmptyString, wxEmptyString);
	}
}
/// ファイル種類選択ダイアログ 置換時
void L3DiskFrame::ShowFileSelectDialogForReplace(int disk_number, int side_number, const wxString &path)
{
	wxString file_format, disk_type;
	if (ShowFileSelectDialog(path, file_format, disk_type)) {
		ReplaceDisk(disk_number, side_number, path, file_format, disk_type);
	}
}
/// 指定したディスクイメージ置換
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] path        ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] disk_type   ディスクパラメータ種類
void L3DiskFrame::ReplaceDisk(int disk_number, int side_number, const wxString &path, const wxString &file_format, const wxString &disk_type)
{
	// set recent file path
	SetRecentPath(path);

	// open disk
	int rc = d88.ReplaceDisk(disk_number, side_number, path, file_format, disk_type);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString, true);
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
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
/// ディスクパラメータを表示/変更
void L3DiskFrame::ShowDiskAttr()
{
	L3DiskList *list = GetLPanel();
	if (list) {
		list->ShowDiskAttr();
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
/// ディスクをDISK BASIC用に論理フォーマット
void L3DiskFrame::FormatDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->FormatDisk();
		return;
	}
}

#ifdef USE_DND_ON_TOP_PANEL
// ドラッグアンドドロップ時のフォーマットID
wxDataFormat *L3DiskPanelDataFormat = NULL;
#endif

//
// メインパネルは分割ウィンドウ
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

L3DiskPanel::L3DiskPanel(L3DiskFrame *parent)
                : wxSplitterWindow(parent, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_BORDER | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
    frame = parent;

//	// fit size on parent window
//    SetSize(parent->GetClientSize());

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	// control panel
	lpanel = new L3DiskList(frame, this);
	rpanel = new L3DiskRPanel(frame, this, frame->GetSelectedMode());
	
	SplitVertically(lpanel, rpanel, 200);

	SetMinimumPaneSize(10);

#ifdef USE_DND_ON_TOP_PANEL
	// drag and drop
	if (!L3DiskPanelDataFormat) {
		L3DiskPanelDataFormat = new wxDataFormat(wxT("L3DISKPANELDATAV2"));
	}
	SetDropTarget(new L3DiskPanelDropTarget(parent, this));
#endif
}

L3DiskPanel::~L3DiskPanel()
{
#ifdef USE_DND_ON_TOP_PANEL
	delete L3DiskPanelDataFormat;
	L3DiskPanelDataFormat = NULL;
#endif
}

/// ドロップされたファイルを処理
/// @return : ディスクイメージを処理した時true
bool L3DiskPanel::ProcessDroppedFile(wxCoord x, wxCoord y, const wxString &filename)
{
	bool is_disk_file = false;

	// 分割位置
	int pos_x = GetSashPosition();
	bool disk_is_empty = (frame->GetDiskD88().CountDisks() == 0);

	// is d88 file?
	wxFileName fname(filename);
	wxString ext = fname.GetExt().Lower();

	if ((disk_is_empty || x < pos_x) && gFileTypes.FindExt(ext)) {
		is_disk_file = true;
	}
	if (is_disk_file) {
		// ディスクイメージを開く
		frame->OpenDroppedFile(filename);
	} else if (disk_is_empty) {
		// ディスクイメージのタイプ
		frame->ShowFileSelectDialogForOpen(filename);
	} else {
		// ファイルをインポート
		L3DiskFileList *file_list = rpanel->GetFileListPanel();
		if (file_list) {
			file_list->ImportDataFile(filename);
		}
		L3DiskRawPanel *raw_panel = rpanel->GetRawPanel();
		if (raw_panel) {
			raw_panel->ShowImportTrackRangeDialog(filename);
		}
	}
	return is_disk_file;
}

bool L3DiskPanel::ProcessDroppedFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	for(int n = 0; n < (int)filenames.Count(); n++) {
		if (ProcessDroppedFile(x, y, filenames.Item(n))) {
			// d88ファイルを処理した時はここで中断
			break;
		}
	}
	return true;
}

bool L3DiskPanel::ProcessDroppedFile(wxCoord x, wxCoord y, const wxUint8 *buffer, size_t buflen)
{
	L3DiskFileList *list = rpanel->GetFileListPanel();
	if (list) {
		list->ImportDataFile(buffer, buflen);
	}
	return true;
}

#ifdef USE_DND_ON_TOP_PANEL
//
// File Drag and Drop
//
L3DiskPanelDropTarget::L3DiskPanelDropTarget(L3DiskFrame *parentframe, L3DiskPanel *parentwindow)
	: wxDropTarget()
{
	parent = parentwindow;
	frame = parentframe;

	wxDataObjectComposite* dataobj = new wxDataObjectComposite();
	// from explorer, finder etc.
	dataobj->Add(new wxFileDataObject(), true);
	// from own appli
	dataobj->Add(new wxCustomDataObject(*L3DiskPanelDataFormat));
	SetDataObject(dataobj);
}

//bool L3DiskPanelDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
//{
//	if (filenames.Count() > 0) {
//		parent->ProcessDroppedFiles(x, y, filenames);
//	}
//    return true;
//}

wxDragResult L3DiskPanelDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	if ( !GetData() ) return wxDragNone;
	bool sts = false;
	wxDataObjectComposite *comobj = (wxDataObjectComposite *)GetDataObject();
	if (comobj) {
		wxDataFormat fmt = comobj->GetReceivedFormat();
		if (fmt.GetType() == wxDF_FILENAME) {
			// エクスプローラからのDnD
			wxFileDataObject *dobj = (wxFileDataObject *)comobj->GetObject(fmt);
			sts = parent->ProcessDroppedFiles(x, y, dobj->GetFilenames());
		} else if (fmt == *L3DiskPanelDataFormat) {
			// このアプリからのDnD
			wxCustomDataObject *dobj = (wxCustomDataObject *)comobj->GetObject(fmt);
			size_t buflen = dobj->GetDataSize();
			wxUint8 *buffer = (wxUint8 *)dobj->GetData();
			sts = parent->ProcessDroppedFile(x, y, buffer, buflen);
		}
	}
	return (sts ? def : wxDragNone);
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
// Dir Dialog
//
L3DiskDirDialog::L3DiskDirDialog(const wxString& message, const wxString& defaultDir, long style)
            : wxDirDialog(NULL, message, defaultDir, style)
{
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
