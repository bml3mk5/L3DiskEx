/// @file main.cpp
///
/// @brief 本体
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "main.h"
#include <wx/cmdline.h>
#include <wx/filename.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/toolbar.h>
#include <wx/dir.h>
#include "charcodes.h"
#include "ui/diskparambox.h"
#include "basicfmt/basicfmt.h"
#include "basicfmt/basicdiritem.h"
#include "ui/uirpanel.h"
#include "ui/uidisklist.h"
#include "ui/uidiskattr.h"
#include "ui/uifilelist.h"
#include "ui/uirawdisk.h"
#include "ui/uibindump.h"
#include "ui/uifatarea.h"
#include "diskimg/fileparam.h"
#include "ui/fileselbox.h"
#include "diskimg/diskwriter.h"
#include "diskimg/diskresult.h"
#include "ui/diskreplacebox.h"
#include "ui/fontminibox.h"
#include "ui/configbox.h"
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
#include "res/fileicon_hidden.xpm"
#include "res/labelicon_normal.xpm"
//#include "res/triangle_down.xpm"
//#include "res/triangle_up.xpm"

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

//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_APP(L3DiskApp);

wxBEGIN_EVENT_TABLE(L3DiskApp, wxApp)
#ifdef CAPTURE_MOD_KEY_ON_APP
	EVT_IDLE(L3DiskApp::OnAppIdle)
#endif
wxEND_EVENT_TABLE()

L3DiskApp::L3DiskApp()
{
	frame = NULL;
#ifdef CAPTURE_MOD_KEY_ON_APP
	mod_keys = 0;
	mod_cnt = 0;
#endif
}

/// 初期処理
bool L3DiskApp::OnInit()
{
	SetAppPath();
	SetAppName(_T("l3diskex"));

	if (!wxApp::OnInit()) {
		return false;
	}

	// log file
	myLog.Open(ini_path, GetAppName(), _T(".log"));

	// load ini file
	gConfig.Load(ini_path + GetAppName() + _T(".ini"));

	// set locale search path and catalog name

	wxString locale_name = gConfig.GetLanguage();
	int lang_num = 0;
	if (locale_name.IsEmpty()) {
		lang_num = wxLocale::GetSystemLanguage();
	} else {
		const wxLanguageInfo * const lang = wxLocale::FindLanguageInfo(locale_name);
		if (lang) {
			lang_num = lang->Language;
		} else {
			lang_num = wxLANGUAGE_UNKNOWN;
		}
	}
	if (mLocale.Init(lang_num, wxLOCALE_LOAD_DEFAULT)) {
		mLocale.AddCatalogLookupPathPrefix(res_path + _T("lang"));
		mLocale.AddCatalogLookupPathPrefix(_T("lang"));
		mLocale.AddCatalog(_T("l3diskex"));
	}
	if (mLocale.IsLoaded(_T("l3diskex"))) {
		locale_name = mLocale.GetCanonicalName();
	} else {
		locale_name = wxT("");
	}

	// load xml
	wxString errmsgs;
	if (!gDiskTemplates.Load(res_path + wxT("data/"), locale_name, errmsgs)) {
		errmsgs += wxT("\n");
		errmsgs += _("Cannot load disk types data file.");
		wxMessageBox(errmsgs, _("Error"), wxOK);
		return false;
	}
	if (!gDiskBasicTemplates.Load(res_path + wxT("data/"), locale_name, errmsgs)) {
		errmsgs += wxT("\n");
		errmsgs += _("Cannot load disk basic types data file.");
		wxMessageBox(errmsgs, _("Error"), wxOK);
		return false;
	}
	if (!CharCodes::Load(res_path + wxT("data/"), locale_name, errmsgs)) {
		wxMessageBox(_("Cannot load char codes data file."), _("Error"), wxOK);
		return false;
	}
	if (!gFileTypes.Load(res_path + wxT("data/"), locale_name)) {
		wxMessageBox(_("Cannot load file types data file."), _("Error"), wxOK);
		return false;
	}

	int w = gConfig.GetWindowWidth();
	int h = gConfig.GetWindowHeight();
	frame = new L3DiskFrame(GetAppName(), wxSize(w, h));
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
	gConfig.Save();
	// remove temp directories
	RemoveTempDirs();

	return 0;
}

#ifdef CAPTURE_MOD_KEY_ON_APP
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
#endif

/// ファイルを開く(Mac用)
void L3DiskApp::MacOpenFile(const wxString &fileName)
{
	if (frame) {
		if (!frame->CloseDataFile()) return;
		frame->PreOpenDataFile(fileName);
	}
}

/// アプリケーションのパスを設定
void L3DiskApp::SetAppPath()
{
	app_path = wxFileName::FileName(argv[0]).GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
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

/// テンポラリディレクトリを作成する
bool L3DiskApp::MakeTempDir(wxString &tmp_dir_path)
{
	wxFileName file_path = wxFileName(gConfig.GetTemporaryFolder(), wxGetApp().GetAppName());

	tmp_dir_path = wxFileName::CreateTempFileName(file_path.GetFullPath());
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
	RemoveTempDir(tmp_dir_path, 0);

	// リストからも削除
	tmp_dirs.Remove(tmp_dir_path);
}

/// テンポラリディレクトリを削除する
void L3DiskApp::RemoveTempDir(const wxString &tmp_dir_path, int depth)
{
	if (tmp_dir_path.IsEmpty()) return;
	if (depth > 20) return;

	// テンポラリ内のファイル削除
	wxDir dir(tmp_dir_path);
	wxString file_name;
	bool sts = dir.GetFirst(&file_name, wxEmptyString);
	while(sts) {
		wxFileName file_path(tmp_dir_path, file_name);
		wxString full_name = file_path.GetFullPath();
		if (wxFileName::DirExists(full_name)) {
			RemoveTempDir(full_name, depth + 1);
		} else {
			wxRemoveFile(full_name);
		}
		sts = dir.GetNext(&file_name);
	}
	dir.Close();

	// テンポラリディレクトリを削除
	wxDir::Remove(tmp_dir_path);
}

/// テンポラリディレクトリをすべて削除する
void L3DiskApp::RemoveTempDirs()
{
	wxArrayString dirs = tmp_dirs;
	for(size_t n = 0; n < dirs.Count(); n++) {
		RemoveTempDir(dirs.Item(n));
	}
}

//////////////////////////////////////////////////////////////////////
//
// Status Counter
//
StatusCounter::StatusCounter()
{
	m_current = 0;
	m_count = 0;
	m_using = 0;
}
StatusCounter::~StatusCounter()
{
}
void StatusCounter::Clear()
{
	m_current = 0;
	m_count = 0;
	m_using = 0;
	m_message.Empty();
}
void StatusCounter::Start(int count, const wxString &message)
{
	m_current = 0;
	m_count = count;
	m_using = 1;
	m_message = message;
}
void StatusCounter::Append(int count)
{
	m_count += count;
}
void StatusCounter::Increase()
{
	m_current++;
}
void StatusCounter::Finish(const wxString &message)
{
	m_message = message;
	m_using = 3;
}
wxString StatusCounter::GetCurrentMessage() const
{
	wxString str = wxString::Format(wxT("%d/%d "), m_current, m_count);
	str += m_message;
	return str;
}

StatusCounters::StatusCounters()
{
}
StatusCounters::~StatusCounters()
{
}
StatusCounter &StatusCounters::Item(int idx)
{
	if (idx < 0 || idx >= StatusCountersMax) idx = 0;
	return m_sc[idx];
}
void StatusCounters::Clear()
{
	for(int idx = 0; idx < StatusCountersMax; idx++) {
		StatusCounter *sc = &m_sc[idx];
		if (sc->IsFinished()) {
			sc->Clear();
		}
	}
}
int StatusCounters::Start(int count, const wxString &message)
{
	int decide = -1;
	for(int idx = 0; idx < StatusCountersMax; idx++) {
		StatusCounter *sc = &m_sc[idx];
		if (sc->IsIdle()) {
			sc->Start(count, message);
			m_delay.Stop();
			Clear();
			decide = idx;
			break;
		}
	}
	return decide;
}
void StatusCounters::Append(int idx, int count)
{
	if (idx < 0 || idx >= StatusCountersMax) idx = 0;
	m_sc[idx].Append(count);
}
void StatusCounters::Increase(int idx)
{
	if (idx < 0 || idx >= StatusCountersMax) idx = 0;
	m_sc[idx].Increase();
}
void StatusCounters::Finish(int idx, const wxString &message, wxEvtHandler *owner)
{
	if (idx < 0 || idx >= StatusCountersMax) idx = 0;
	m_sc[idx].Finish(message);
	m_delay.SetOwner(owner, IDT_STATUS_COUNTER);
	m_delay.StartOnce(5000);
}
int StatusCounters::Current(int idx) const
{
	if (idx < 0 || idx >= StatusCountersMax) idx = 0;
	return m_sc[idx].Current();
}
int StatusCounters::Count(int idx) const
{
	if (idx < 0 || idx >= StatusCountersMax) idx = 0;
	return m_sc[idx].Count();
}
wxString StatusCounters::GetCurrentMessage(int idx) const
{
	if (idx < 0 || idx >= StatusCountersMax) idx = 0;
	return m_sc[idx].GetCurrentMessage();
}

//////////////////////////////////////////////////////////////////////
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

	EVT_MENU(IDM_INITIALIZE_DISK, L3DiskFrame::OnInitializeDisk)
	EVT_MENU(IDM_FORMAT_DISK, L3DiskFrame::OnFormatDisk)

	EVT_MENU(IDM_EXPORT_DATA, L3DiskFrame::OnExportDataFromDisk)
	EVT_MENU(IDM_IMPORT_DATA, L3DiskFrame::OnImportDataToDisk)
	EVT_MENU(IDM_DELETE_DATA, L3DiskFrame::OnDeleteDataFromDisk)
	EVT_MENU(IDM_RENAME_DATA_ON_DISK, L3DiskFrame::OnRenameDataOnDisk)
	EVT_MENU(IDM_COPY_DATA, L3DiskFrame::OnCopyDataFromDisk)
	EVT_MENU(IDM_PASTE_DATA, L3DiskFrame::OnPasteDataToDisk)
	EVT_MENU(IDM_MAKE_DIRECTORY_ON_DISK, L3DiskFrame::OnMakeDirectoryOnDisk)
	EVT_MENU(IDM_PROPERTY_DATA, L3DiskFrame::OnPropertyOnDisk)

	EVT_MENU(IDM_BASIC_MODE, L3DiskFrame::OnBasicMode)
	EVT_MENU(IDM_RAWDISK_MODE, L3DiskFrame::OnRawDiskMode)
	EVT_MENU_RANGE(IDM_CHAR_0, IDM_CHAR_0 + 10, L3DiskFrame::OnChangeCharCode)
	EVT_MENU(IDM_TRIM_DATA, L3DiskFrame::OnTrimData)
	EVT_MENU(IDM_SHOW_DELFILE, L3DiskFrame::OnShowDeletedFile)
	EVT_MENU(IDM_CONFIGURE, L3DiskFrame::OnConfigure)

	EVT_MENU(IDM_WINDOW_BINDUMP, L3DiskFrame::OnOpenBinDump)
	EVT_MENU(IDM_WINDOW_FATAREA, L3DiskFrame::OnOpenFatArea)
	EVT_MENU(IDM_FILELIST_COLUMN, L3DiskFrame::OnChangeColumnsOfFileList)
	EVT_MENU(IDM_CHANGE_FONT, L3DiskFrame::OnChangeFont)

	EVT_TIMER(StatusCounters::IDT_STATUS_COUNTER, L3DiskFrame::OnTimerStatusCounter)

#ifdef USE_MENU_OPEN
	EVT_MENU_OPEN(L3DiskFrame::OnMenuOpen)
#endif
wxEND_EVENT_TABLE()

// 翻訳用
#define DIALOG_BUTTON_STRING _("OK"),_("Cancel")
#define APPLE_MENU_STRING _("Hide l3diskex"),_("Hide Others"),_("Show All"),_("Quit l3diskex"),_("Services"),_("Preferences…"),_("Minimize"),_("Zoom"),_("Bring All to Front")

L3DiskFrame::L3DiskFrame(const wxString& title, const wxSize& size)
#if defined(__WXOSX__)
	: wxFrame(NULL, -1, title, wxDefaultPosition, wxDefaultSize)
#else
	: wxFrame(NULL, -1, title, wxDefaultPosition, size)
#endif
{
#if defined(__WXOSX__)
	SetClientSize(size);
#endif

	unique_number = 0;

	// icon
#ifdef __WXMSW__
	SetIcon(wxIcon(_T("l3diskex")));
#elif defined(__WXGTK__) || defined(__WXMOTIF__)
	SetIcon(wxIcon(l3diskex_xpm));
#endif

	// menu
	MakeMenu();

	// status bar
	RecreateStatusbar();

	// tool bar
    RecreateToolbar();

	// splitter window
	panel = new L3DiskPanel(this);

	bindump_frame = NULL;
	fatarea_frame = NULL;
}

L3DiskFrame::~L3DiskFrame()
{
	// フレーム
#if defined(__WXOSX__)
	wxSize sz = GetClientSize();
#else
	wxSize sz = GetSize();
#endif
	gConfig.SetWindowWidth(sz.GetWidth());
	gConfig.SetWindowHeight(sz.GetHeight());
}

/// フレーム部の初期処理
bool L3DiskFrame::Init(const wxString &in_file)
{
	bool valid = false;

	if (!in_file.IsEmpty()) {
		valid = PreOpenDataFile(in_file);
	}
	if (!valid) {
		// 起動時にファイルを開いたときはもう更新している
		UpdateMenuFile();
		UpdateMenuDisk();
		UpdateMenuMode();
		UpdateToolBar();
	}
	if (panel) {
		// キャラクターコードマップ番号設定
		SetDefaultCharCode();
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
	toolBar->AddTool(IDM_EXPORT_DATA, _("Export"),
		toolBarBitmaps[fd_5inch_16_export], wxNullBitmap, wxITEM_NORMAL,
		_("Export a file from the disk"));
	toolBar->AddTool(IDM_IMPORT_DATA, _("Import"),
		toolBarBitmaps[fd_5inch_16_import], wxNullBitmap, wxITEM_NORMAL,
		_("Import a file to the disk"));
//	toolBar->AddTool(IDM_DELETE_DISK, _("Delete file"),
//		toolBarBitmaps[fileicon_delete], wxNullBitmap, wxITEM_NORMAL,
//		_("Delete a file from the disk"));

	// after adding the buttons to the toolbar, must call Realize() to reflect
	// the changes
	toolBar->Realize();
	int m_rows = 1;
	toolBar->SetRows(toolBar->IsVertical() ? (int)toolBar->GetToolsCount() / m_rows : m_rows);
}

/// ステータスバーの再生成
void L3DiskFrame::RecreateStatusbar()
{
	// delete and recreate the statusbar
	wxStatusBar *statBar = GetStatusBar();
	delete statBar;
	SetStatusBar(NULL);

	long style = wxSTB_DEFAULT_STYLE;

	statBar = new wxStatusBar(this, wxID_ANY, style);
	statBar->SetFieldsCount(2);

    SetStatusBar(statBar);

	PositionStatusBar();
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
	} else if (menu == menuMode) { // Mode...
		UpdateMenuMode();
	}
}
#endif

/// ドロップされたファイルを開く
void L3DiskFrame::OpenDroppedFile(const wxString &path)
{
 	if (!CloseDataFile()) return;
	PreOpenDataFile(path);
}

////////////////////////////////////////
//
// イベントプロシージャ
//

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
	list->ShowSaveDiskDialog();
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
	list->ReplaceDisk();
//	ShowReplaceDiskDialog(list->GetSelectedDiskNumber(), list->GetSelectedDiskSide());
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
/// メニュー エクスポート選択
void L3DiskFrame::OnExportDataFromDisk(wxCommandEvent& WXUNUSED(event))
{
	ExportDataFromDisk();
}
/// メニュー インポート選択
void L3DiskFrame::OnImportDataToDisk(wxCommandEvent& WXUNUSED(event))
{
	ImportDataToDisk();
}
/// メニュー 削除選択
void L3DiskFrame::OnDeleteDataFromDisk(wxCommandEvent& WXUNUSED(event))
{
	DeleteDataFromDisk();
}
/// メニュー リネーム選択
void L3DiskFrame::OnRenameDataOnDisk(wxCommandEvent& WXUNUSED(event))
{
	RenameDataOnDisk();
}
/// メニュー コピー選択
void L3DiskFrame::OnCopyDataFromDisk(wxCommandEvent& WXUNUSED(event))
{
	CopyDataFromDisk();
}
/// メニュー ペースト選択
void L3DiskFrame::OnPasteDataToDisk(wxCommandEvent& WXUNUSED(event))
{
	PasteDataToDisk();
}
/// メニュー ディレクトリ作成選択
void L3DiskFrame::OnMakeDirectoryOnDisk(wxCommandEvent& WXUNUSED(event))
{
	MakeDirectoryOnDisk();
}
/// メニュー プロパティ選択
void L3DiskFrame::OnPropertyOnDisk(wxCommandEvent& WXUNUSED(event))
{
	PropertyOnDisk();
}

/// メニュー 設定ダイアログ選択
void L3DiskFrame::OnConfigure(wxCommandEvent& WXUNUSED(event))
{
	ShowConfigureDialog();
}

/// ファイルモード選択
void L3DiskFrame::OnBasicMode(wxCommandEvent& event)
{
	ChangeRPanel(0, NULL);
}

/// Rawディスクモード選択
void L3DiskFrame::OnRawDiskMode(wxCommandEvent& event)
{
	ChangeRPanel(1, NULL);
}

/// キャラクターコード選択
void L3DiskFrame::OnChangeCharCode(wxCommandEvent& event)
{
	int sel = event.GetId() - IDM_CHAR_0;
	wxString name = gCharCodeChoices.GetItemName(wxT("main"), (size_t)sel);
	ChangeCharCode(name);
}

/// 未使用データを切り落とすか
void L3DiskFrame::OnTrimData(wxCommandEvent& event)
{
	gConfig.TrimUnusedData(event.IsChecked());
}

/// 削除ファイルを表示するか
void L3DiskFrame::OnShowDeletedFile(wxCommandEvent& event)
{
	gConfig.ShowDeletedFile(event.IsChecked());
	// ファイルリストを更新
	SetFileListData();
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

/// 使用状況ウィンドウ選択
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

/// ファイルリストの列選択
void L3DiskFrame::OnChangeColumnsOfFileList(wxCommandEvent& event)
{
	ChangeColumnsOfFileList();
}

/// フォント変更選択
void L3DiskFrame::OnChangeFont(wxCommandEvent& event)
{
	ShowListFontDialog();
}

/// ステータスカウンター終了タイマー
void L3DiskFrame::OnTimerStatusCounter(wxTimerEvent& event)
{
	ClearStatusCounter();
}

////////////////////////////////////////
//
// ウィンドウ操作
//

/// メニューの作成
void L3DiskFrame::MakeMenu()
{
	menuFile = new wxMenu;
	menuData = new wxMenu;
	menuMode = new wxMenu;
	menuView = new wxMenu;
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
	// data menu
	menuData->Append( IDM_EXPORT_DATA, _("&Export...") );
	menuData->Append( IDM_IMPORT_DATA, _("&Import...") );
	menuData->AppendSeparator();
	menuData->Append( IDM_DELETE_DATA, _("&Delete...") );
	menuData->Append( IDM_RENAME_DATA_ON_DISK, _("Rena&me File") );
	menuData->AppendSeparator();
	menuData->Append(IDM_COPY_DATA, _("&Copy"));
	menuData->Append(IDM_PASTE_DATA, _("&Paste..."));
	menuData->AppendSeparator();
	menuData->Append( IDM_MAKE_DIRECTORY_ON_DISK, _("Make Directory(&F)...") );
	menuData->AppendSeparator();
	menuData->Append( IDM_PROPERTY_DATA, _("&Property") );
	// mode menu
	menuMode->AppendRadioItem( IDM_BASIC_MODE, _("BASIC Mode") );
	menuMode->AppendRadioItem( IDM_RAWDISK_MODE, _("Raw Disk Mode") );
	menuMode->AppendSeparator();
		sm = new wxMenu();
		const CharCodeChoice *choice = gCharCodeChoices.Find(wxT("main"));
		if (choice) {
			for(size_t i=0; i<choice->Count(); i++) {
				const CharCodeMap *map = choice->Item(i);
				sm->AppendRadioItem( IDM_CHAR_0 + (int)i, map->GetDescription() );
			}
		}
	menuMode->AppendSubMenu(sm, _("&Charactor Code") );
	menuMode->AppendSeparator();
		sm = new wxMenu();
		sm->AppendCheckItem( IDM_TRIM_DATA, _("Trim unused data when save the disk image.") );
		sm->AppendCheckItem( IDM_SHOW_DELFILE, _("Show deleted and hidden files on the file list.") );
	menuMode->AppendSubMenu(sm, _("Quick &Settings") );
	menuMode->AppendSeparator();
	menuMode->Append( IDM_CONFIGURE, _("C&onfigure...") );
	// view menu
	menuView->AppendCheckItem( IDM_WINDOW_BINDUMP, _("&Dump Window") );
	menuView->AppendCheckItem( IDM_WINDOW_FATAREA, _("&Availability Window") );
	menuView->AppendSeparator();
	menuView->Append( IDM_FILELIST_COLUMN, _("Columns of File &List...") );
	menuView->AppendSeparator();
	menuView->Append( IDM_CHANGE_FONT, _("&Font...") );
#if defined(__WXOSX__) && wxCHECK_VERSION(3,1,2)
	// view system menu on mac os x
	menuView->AppendSeparator();
#endif
	// help menu
	menuHelp->Append( wxID_ABOUT, _("&About...") );

	// menu bar
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, _("&File") );
	menuBar->Append( menuData, _("&Data") );
	menuBar->Append( menuMode, _("&Mode") );
	menuBar->Append( menuView, _("&View") );
#if defined(__WXOSX__) && wxCHECK_VERSION(3,1,2)
	// window system menu on mac os x
	menuBar->Append( new wxMenu(), _("&Window") );
#endif
	menuBar->Append( menuHelp, _("&Help") );

	SetMenuBar( menuBar );
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

/// モードメニューの更新
void L3DiskFrame::UpdateMenuMode()
{
	int sel = gCharCodeChoices.IndexOf(wxT("main"), gConfig.GetCharCode());
	wxMenuItem *mitem = menuMode->FindItem(IDM_CHAR_0 + sel);
	if (mitem) mitem->Check(true);
	menuMode->Check(IDM_TRIM_DATA, gConfig.IsTrimUnusedData());
	menuMode->Check(IDM_SHOW_DELFILE, gConfig.IsShownDeletedFile());
}

/// 最近使用したファイル一覧を更新
void L3DiskFrame::UpdateMenuRecentFiles()
{
	// メニューを更新
	wxArrayString names = gConfig.GetRecentFiles();
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
	menuFile->Enable(IDM_FORMAT_DISK, opened && IsFormattableDisk());
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
	L3DiskList *lpanel = GetLPanel();
	menuData->Enable(IDM_PROPERTY_DATA, (lpanel->IsSelectedDiskImage()));

	bool opened = (list != NULL && list->CanUseBasicDisk());
	menuFile->Enable(IDM_FORMAT_DISK, opened);

	menuData->Enable(IDM_MAKE_DIRECTORY_ON_DISK, opened && list->CanMakeDirectory());

	opened = (opened && list->IsAssignedBasicDisk());
	menuData->Enable(IDM_IMPORT_DATA, opened);
	menuData->Enable(IDM_PASTE_DATA, opened);

	int	cnt = list->GetListSelectedItemCount();
	opened = (opened && cnt > 0);
	menuData->Enable(IDM_EXPORT_DATA, opened);
	menuData->Enable(IDM_DELETE_DATA, opened);
	menuData->Enable(IDM_COPY_DATA, opened);
	opened = (opened && cnt == 1);
	menuData->Enable(IDM_RENAME_DATA_ON_DISK, opened);
}

/// ツールバーのファイルリスト項目を更新
void L3DiskFrame::UpdateToolBarFileList(L3DiskFileList *list)
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (list && list->IsFormattedBasicDisk());
	toolBar->EnableTool(IDM_IMPORT_DATA, opened);

	int	cnt = list->GetListSelectedItemCount();
	opened = (opened && cnt > 0);
	toolBar->EnableTool(IDM_EXPORT_DATA, opened);
	toolBar->EnableTool(IDM_DELETE_DATA, opened);
//	toolBar->EnableTool(IDM_RENAME_FILE_ON_DISK, opened);
//	toolBar->EnableTool(IDM_PROPERTY_DISK, opened);
}

/// メニューとツールバーのファイルリスト項目を更新
void L3DiskFrame::UpdateMenuAndToolBarFileList(L3DiskFileList *list)
{
	UpdateMenuFileList(list);
	UpdateToolBarFileList(list);
}

/// メニューの生ディスク項目を更新
void L3DiskFrame::UpdateMenuRawDisk(L3DiskRawPanel *rawpanel)
{
	L3DiskList *lpanel = GetLPanel();
	menuData->Enable(IDM_PROPERTY_DATA, (lpanel->IsSelectedDiskImage()));

	bool opened = (rawpanel != NULL);
	opened = (opened && rawpanel->TrackListExists());
	menuData->Enable(IDM_EXPORT_DATA, opened);
	menuData->Enable(IDM_IMPORT_DATA, opened);
	menuData->Enable(IDM_COPY_DATA, opened);
	menuData->Enable(IDM_PASTE_DATA, opened);
	menuData->Enable(IDM_DELETE_DATA, opened);

	menuData->Enable(IDM_RENAME_DATA_ON_DISK, false);
	menuData->Enable(IDM_MAKE_DIRECTORY_ON_DISK, false);
}

/// ツールバーの生ディスク項目を更新
void L3DiskFrame::UpdateToolBarRawDisk(L3DiskRawPanel *rawpanel)
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (rawpanel != NULL);
	opened = (opened && rawpanel->TrackListExists());
	toolBar->EnableTool(IDM_EXPORT_DATA, opened);
	toolBar->EnableTool(IDM_IMPORT_DATA, opened);
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
	ClearRPanelData();
	SetDiskListData(GetFileName());

	if (keep) {
		// set position
		SetDiskListPos(dl_num, dl_sid);
	}

#ifndef USE_MENU_OPEN
	// update menu
	UpdateMenuFile();
	UpdateMenuMode();
#endif
}

/// 保存後のウィンドウ上のデータを更新
void L3DiskFrame::UpdateSavedDataOnWindow(const wxString &path)
{
	// 左パネルのパスを更新
	UpdateFilePathOnWindow(path);
	// Rawパネルを更新
	RefreshRawPanelData();
}

/// ウィンドウ上のファイルパスを更新
void L3DiskFrame::UpdateFilePathOnWindow(const wxString &path)
{
	if (!path.IsEmpty()) {
		wxString title = wxGetApp().GetAppName();
		title += wxT(" - ");
		title += path;
		SetTitle(title);
	}

	SetDiskListFilePath(GetFileName());
}

/// キャラクターコード選択
void L3DiskFrame::ChangeCharCode(const wxString &name)
{
	if (GetCharCode() == name) return;

	d88.SetCharCode(name);

	L3DiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->ChangeCharCode(name);
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->ChangeCharCode(name);

	gConfig.SetCharCode(name);

	UpdateMenuMode();
}

/// キャラクターコード番号を返す
const wxString &L3DiskFrame::GetCharCode() const
{
	return gConfig.GetCharCode();
}

/// キャラクターコード番号設定
void L3DiskFrame::SetDefaultCharCode()
{
	wxString name = gConfig.GetCharCode();

	L3DiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->ChangeCharCode(name);
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->ChangeCharCode(name);
}

/// フォント変更ダイアログ
void L3DiskFrame::ShowListFontDialog()
{
	L3DiskList *rlist = GetDiskListPanel();
	if (!rlist) return;

	wxFont font = rlist->GetFont();

	FontMiniBox dlg(this, wxID_ANY, GetFont());
	wxString name = gConfig.GetListFontName();
	if (name.IsEmpty()) name = font.GetFaceName();
	int size = gConfig.GetListFontSize();
	if (size == 0) size = font.GetPointSize();
	dlg.SetFontName(name);
	dlg.SetFontSize(size);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		wxFont new_font(dlg.GetFontSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, dlg.GetFontName());

		SetListFont(new_font);

		gConfig.SetListFontName(dlg.GetFontName());
		gConfig.SetListFontSize(dlg.GetFontSize());
	}
}

/// リストウィンドウのフォント変更
void L3DiskFrame::SetListFont(const wxFont &font)
{
	L3DiskList *rlist = GetDiskListPanel();
	if (rlist) rlist->SetListFont(font);

	L3DiskRPanel *llist = GetRPanel();
	if (llist) llist->SetListFont(font);
}

/// リストウィンドウのフォント設定
void L3DiskFrame::GetDefaultListFont(wxFont &font)
{
	wxFont def_font = GetFont();

	wxString name = gConfig.GetListFontName();
	if (name.IsEmpty()) name = def_font.GetFaceName();
	int size = gConfig.GetListFontSize();
	if (size == 0) size = def_font.GetPointSize();

	font = wxFont(size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, name);
}

/// ファイルリストの列を変更
void L3DiskFrame::ChangeColumnsOfFileList()
{
	L3DiskFileList *llist = GetFileListPanel(true);
	if (llist) llist->ShowListColumnDialog();
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

/// 全パネルにデータをセットする（ディスク選択時）
///
/// ディスク選択orツリー展開で、ルートディレクトリをアサインする。
/// refresh_listをtrueにすればファイルリストを更新する
/// @param [in] disk         ディスク
/// @param [in] side_number  AB面ありの時、サイド番号
/// @param [in] refresh_list 右パネルのディスクを選択した時、左パネルのファイルリストを更新
void L3DiskFrame::SetDataOnDisk(DiskD88Disk *disk, int side_number, bool refresh_list)
{
	AttachDiskBasicOnFileList(disk, side_number);
	if (refresh_list) {
		ClearFatAreaData();
		SetFileListData();
	}
	SetRawPanelData(disk, side_number);
	ClearBinDumpData();
	UpdateMenuAndToolBarDiskList(GetLPanel());
}

/// 全パネルのデータをクリアする
void L3DiskFrame::ClearAllData()
{
	DetachDiskBasicOnFileList();
	ClearDiskAttrData();
	ClearFileListData();
	ClearRawPanelData();
	ClearBinDumpData();
	ClearFatAreaData();
	UpdateMenuAndToolBarDiskList(GetLPanel());
}

/// 全パネルのデータをクリアしてRAW DISKパネルだけデータをセット
void L3DiskFrame::ClearAllAndSetRawData(DiskD88Disk *disk, int side_number)
{
	ClearFileListData();
	SetRawPanelData(disk, side_number);
	ClearBinDumpData();
	ClearFatAreaData();
	UpdateMenuAndToolBarDiskList(GetLPanel());
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

////////////////////////////////////////
//
// ディスク操作
//

/// 新規作成ダイアログ
void L3DiskFrame::ShowCreateFileDialog()
{
	if (!CloseDataFile()) return;

	DiskParamBox dlg(this, wxID_ANY, DiskParamBox::CREATE_NEW_DISK, 0, NULL, NULL, NULL, DiskParamBox::SHOW_ALL);

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
	unique_number++;

	UpdateToolBar();
}
/// ディスク新規追加ダイアログ
void L3DiskFrame::ShowAddNewDiskDialog()
{
	if (!d88.GetFile()) return;

	DiskParamBox dlg(this, wxID_ANY, DiskParamBox::ADD_NEW_DISK, d88.GetDiskTypeNumber(0), NULL, NULL, NULL, DiskParamBox::SHOW_ALL);

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
		GetIniRecentPath(),
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
///
/// 拡張子からファイル種別を判別し、必要なら選択ダイアログを表示する。
/// 判別できた場合やダイアログでOKを選択したらファイルを実際に開く。
/// 
/// @param[in] path ファイルパス
bool L3DiskFrame::PreOpenDataFile(const wxString &path)
{
	wxFileName file_path(path);
	wxString   file_format;
	DiskParam  param_hint;

	int rc = CheckOpeningDataFile(path, file_path.GetExt(), file_format, param_hint);
	if (rc < 0) {
		// エラー終了
		return false;
	}
	if (rc == 0) {
		// 開く
		return OpenDataFile(path, file_format, param_hint);
	} else {
		return true;
	}
}
/// 指定したディスクイメージをチェック
///
/// 拡張子からファイル種別を判別し、必要なら選択ダイアログを表示する。
///
/// @param [in]     path        ファイルパス
/// @param [in]     ext         拡張子
/// @param [in,out] file_format ファイルの形式名("d88","plain"など)
/// @param [out]    param_hint  ディスクパラメータヒント(plain時のみ)
/// @retval  0     候補あり正常
/// @retval -1     エラー終了
/// @retval -32767 キャンセルで終了
int L3DiskFrame::CheckOpeningDataFile(const wxString &path, const wxString &ext, wxString &file_format, DiskParam &param_hint)
{
	DiskParamPtrs	n_disk_params;	// パラメータ候補
	DiskParam		n_manual_param;	// 手動設定時のパラメータ
	int rc = 1;

	FileParam *fitem = gFileTypes.FindExt(ext);
	if (!fitem) {
		// 不明の拡張子なのでファイル種類を選択してもらう
		rc = ShowFileSelectDialog(path, file_format) ? 1 : 0;
	}
	if (rc == 1) {
		// 既知の拡張子ならファイルをチェックする
		rc = d88.Check(path, file_format, n_disk_params, n_manual_param);
		int count = (int)n_disk_params.Count(); 
		if (count > 1 || rc == 1) {
			// 選択ダイアログを表示
			if (ShowParamSelectDialog(path, n_disk_params, count == 0 ? &n_manual_param : NULL, param_hint)) {
				// 選択した
				rc = 0;
			} else {
				// キャンセル
				rc = -32767;
			}
		} else if (count == 1) {
			// 候補1つ
			param_hint = *n_disk_params.Item(0);
			rc = 0;
		}
	}
	// エラーメッセージ表示
	if (rc < 0 && rc != -32767) {
		d88.ShowErrorMessage();
	}
	return rc;
}
/// 指定したディスクイメージを開く
/// @param [in] path        ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] param_hint  ディスクパラメータヒント(plain時のみ)
bool L3DiskFrame::OpenDataFile(const wxString &path, const wxString &file_format, const DiskParam &param_hint)
{
	bool valid = false;

	// set recent file path
	SetIniRecentPath(path);

	// open disk
	int rc = d88.Open(path, file_format, param_hint);
	if (rc >= 0) {
		//
		wxFileName fn(path);
		myLog.SetInfo("Opened the disk image: %s", fn.GetFullName().t_str());
		// update window
		UpdateDataOnWindow(path, false);
		valid = true;
	}

	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
	}
	unique_number++;

	UpdateToolBar();

	return valid;
}

/// ファイル追加ダイアログ
void L3DiskFrame::ShowAddFileDialog()
{
	L3DiskFileDialog dlg(
		_("Add File"),
		GetIniRecentPath(),
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
///
/// 拡張子からファイル種別を判別し、必要なら選択ダイアログを表示する。
/// 判別できた場合やダイアログでOKを選択したらファイルを実際に開く。
/// 
/// @param[in] path ファイルパス
bool L3DiskFrame::PreAddDiskFile(const wxString &path)
{
	wxFileName file_path(path);
	wxString   file_format;
	DiskParam  param_hint;

	int rc = CheckOpeningDataFile(path, file_path.GetExt(), file_format, param_hint);
	if (rc < 0) {
		// エラー終了
		return false;
	}
	if (rc == 0) {
		// 追加
		AddDiskFile(path, file_format, param_hint);
	}

	return true;
}

/// 指定したファイルを追加
/// @param [in] path        ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] param_hint  ディスクパラメータヒント(plain時のみ)
void L3DiskFrame::AddDiskFile(const wxString &path, const wxString &file_format, const DiskParam &param_hint)
{
	// set recent file path
	SetIniRecentPath(path);

	// open disk
	int	rc = d88.Add(path, file_format, param_hint);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString, false);
	}

	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
	}
}

/// ファイル種類選択ダイアログ
/// @param [in]  path        ファイルパス
/// @param [out] file_format 選択したファイルの形式名("d88","plain"など)
/// @return true OKボタンを押した
bool L3DiskFrame::ShowFileSelectDialog(const wxString &path, wxString &file_format)
{
	FileSelBox dlg(this, wxID_ANY);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		wxString name = dlg.GetFormatType();
		file_format = dlg.GetFormatType();
	}
	return (sts == wxID_OK);
}

/// ディスク種類選択ダイアログ
/// @param [in] path         ファイルパス
/// @param [in] disk_params  ディスクパラメータの候補
/// @param [in] manual_param 手動設定の初期パラメータ
/// @param [out] param_hint  選択したディスクパラメータ
/// @return true OKボタンを押した
bool L3DiskFrame::ShowParamSelectDialog(const wxString &path, const DiskParamPtrs &disk_params, const DiskParam *manual_param, DiskParam &param_hint)
{
	// パラメータを選択
	DiskParamBox dlg(this, wxID_ANY, DiskParamBox::SELECT_DISK_TYPE, 0, NULL, &disk_params, manual_param, DiskParamBox::SHOW_TEMPLATE);
	int sts = dlg.ShowModal();
	if (sts != wxID_OK) {
		return false;
	}
	dlg.GetParam(param_hint);
	return true;
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

	unique_number++;
	
	// プロパティウィンドウを閉じる
	CloseAllFileAttr();

	// update window
	wxString title = wxGetApp().GetAppName();
	SetTitle(title);

	// update panel
	ClearRPanelData();
	ClearDiskListData();

	//
	d88.Close();

#ifndef USE_MENU_OPEN
	// update menu
	UpdateMenuFile();
	UpdateMenuMode();
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
		gFileTypes.GetWildcardForSave(),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();
	FileFormat *fmt = gFileTypes.GetFilterForSave(dlg.GetFilterIndex());

	if (rc == wxID_OK) {
		SaveDataFile(path, fmt ? fmt->GetName() : wxT(""));
	}
}
/// 指定したファイルに保存
/// @param [in] path        ファイルパス
/// @param [in] file_format 保存フォーマット形式
void L3DiskFrame::SaveDataFile(const wxString &path, const wxString &file_format)
{
	// set recent file path
	SetIniRecentPath(path);

	int rc;

	// validate disk
	rc = d88.CanSave(file_format);
	if (rc != 0) {
		rc = d88.ShowErrWarnMessage();
		if (rc < 0) return;
	}

	// save disk
	rc = d88.Save(path, file_format,
		DiskWriteOptions(
			gConfig.IsTrimUnusedData()
		)
	);
	if (rc >= 0) {
		// update window
		UpdateSavedDataOnWindow(path);
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
	}
}
/// ディスクをファイルに保存ダイアログ（指定ディスク）
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] each_sides  OSが各面で異なる
void L3DiskFrame::ShowSaveDiskDialog(int disk_number, int side_number, bool each_sides)
{

	wxString filename = d88.GetDiskName(disk_number, true);
	if (filename.IsEmpty()) {
		filename = d88.GetFileNameBase();
		filename += wxString::Format(wxT("_%02d"), disk_number);
	}
	if (side_number >= 0) {
		filename += wxT("_");
		filename += Utils::GetSideNumStr(side_number, each_sides);
	}
	filename += wxT(".d88");

	wxString title = _("Save Disk");
	title += Utils::GetSideStr(side_number, each_sides);
	L3DiskFileDialog dlg(
		title,
		GetIniRecentPath(),
		filename,
		gFileTypes.GetWildcardForSave(),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int rc = dlg.ShowModal();
	wxString path = dlg.GetPath();
	FileFormat *fmt = gFileTypes.GetFilterForSave(dlg.GetFilterIndex());

	if (rc == wxID_OK) {
		SaveDataDisk(disk_number, side_number, path, fmt ? fmt->GetName() : wxT(""));
	}
}
/// 指定したファイルに保存（指定ディスク）
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] path        ファイルパス
/// @param [in] file_format 保存フォーマット形式
void L3DiskFrame::SaveDataDisk(int disk_number, int side_number, const wxString &path, const wxString &file_format)
{
	// set recent file path
	SetIniRecentPath(path);

	int rc;

	// validate disk
	rc = d88.CanSave(file_format);
	if (rc != 0) {
		rc = d88.ShowErrWarnMessage();
		if (rc < 0) return;
	}

	// save disk
	rc = d88.SaveDisk(disk_number, side_number, path, file_format,
		DiskWriteOptions(
			gConfig.IsTrimUnusedData()
		)
	);
	if (rc >= 0) {
		// RAWパネルだけ更新
		RefreshRawPanelData();
	}
	if (rc != 0) {
		// message
		d88.ShowErrorMessage();
	}
}
/// ディスクイメージ置換ダイアログ
/// @param [in] disk_number 置き換え対象ディスク番号
/// @param [in] side_number 置き換え対象ディスクのサイド番号
/// @param [in] subcaption  サブキャプション
void L3DiskFrame::ShowReplaceDiskDialog(int disk_number, int side_number, const wxString &subcaption)
{
	wxString title = _("Replace Disk Data");
	if (!subcaption.IsEmpty()) {
		title += wxT(" (");
		title += subcaption;
		title += wxT(")");
	}
	L3DiskFileDialog dlg(
		title,
		GetIniRecentPath(),
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
///
/// 拡張子からファイル種別を判別し、必要なら選択ダイアログを表示する。
/// 判別できた場合やダイアログでOKを選択したらファイルを実際に開く。
/// 
/// @param [in] disk_number 置き換え対象ディスク番号
/// @param [in] side_number 置き換え対象ディスクのサイド番号(両面なら-1)
/// @param [in] path        置き換え元イメージファイルパス
bool L3DiskFrame::PreReplaceDisk(int disk_number, int side_number, const wxString &path)
{
	wxFileName file_path(path);
	wxString   file_format;
	DiskParam  param_hint;

	int rc = CheckOpeningDataFile(path, file_path.GetExt(), file_format, param_hint);
	if (rc < 0) {
		// エラー終了
		return false;
	}

	DiskD88File src_file;
	DiskD88Disk *tag_disk = NULL;
	rc = d88.ParseForReplace(disk_number, side_number, path, file_format, param_hint, src_file, tag_disk);
	if (rc < 0) {
		// エラー終了
		d88.ShowErrorMessage();
		return false;
	}

	DiskReplaceBox dlg(this, wxID_ANY, side_number, src_file, *tag_disk);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		// 選択したディスク
		DiskD88Disk *src_disk = src_file.GetDisk(dlg.GetSelectedDiskNumber());
		// 置き換え
		ReplaceDisk(disk_number, side_number, path, src_disk, dlg.GetSelectedSideNumber(), tag_disk);
	}
	return true;
}

/// 指定したディスクイメージ置換
/// @param [in] disk_number     置き換え対象ディスク番号
/// @param [in] side_number     置き換え対象ディスクのサイド番号(両面なら-1)
/// @param [in] path            置き換え元イメージファイルパス
/// @param [in] src_disk        置き換え元ディスク
/// @param [in] src_side_number 置き換え元ディスクのサイド番号
/// @param [in] tag_disk        置き換え対象ディスク
void L3DiskFrame::ReplaceDisk(int disk_number, int side_number, const wxString &path, DiskD88Disk *src_disk, int src_side_number, DiskD88Disk *tag_disk)
{
	// set recent file path
	SetIniRecentPath(path);

	// open disk
	int rc = d88.ReplaceDisk(disk_number, side_number, src_disk, src_side_number, tag_disk);
	if (rc >= 0) {
		// update window
		ClearRPanelData();
		d88.ClearDiskBasicParseAndAssign(disk_number, side_number);
		RefreshDiskListOnSelectedDisk();
//		ReSelectDiskList();
//		UpdateDataOnWindow(wxEmptyString, true);
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
void L3DiskFrame::ExportDataFromDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowExportDataFileDialog();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowExportDataDialog();
		return;
	}
}
/// ディスクにファイルをインポート
void L3DiskFrame::ImportDataToDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowImportDataFileDialog();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowImportDataDialog();
		return;
	}
}
/// ディスクからファイルを削除
void L3DiskFrame::DeleteDataFromDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->DeleteDataFile();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowDeleteDataDialog();
		return;
	}
}
/// ディスクのファイル名を変更
void L3DiskFrame::RenameDataOnDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->StartEditingFileName();
		return;
	}
}
/// ディスクのデータをコピー
void L3DiskFrame::CopyDataFromDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->CopyToClipboard();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->CopyToClipboard();
		return;
	}
}
/// ディスクにデータをペースト
void L3DiskFrame::PasteDataToDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->PasteFromClipboard();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->PasteFromClipboard();
		return;
	}
}
/// ディスクにディレクトリを作成
void L3DiskFrame::MakeDirectoryOnDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowMakeDirectoryDialog();
		return;
	}
}
/// ファイルのプロパティ
void L3DiskFrame::PropertyOnDisk()
{
	wxWindow *fwin = wxWindow::FindFocus();
	L3DiskList *lpanel = GetLPanel();
	if (fwin == lpanel) {
		lpanel->ShowDiskAttr();
		return;
	}
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowFileAttr();
		return;
	}
	L3DiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowRawDiskAttr();
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
	L3DiskList *list = GetDiskListPanel();
	if (list) {
		list->FormatDisk();
		return;
	}
}

/// DISK BASIC用に論理フォーマットできるか
bool L3DiskFrame::IsFormattableDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		return list->IsFormattableBasicDisk();
	}
	return false;
}

/// BASIC情報ダイアログ
void L3DiskFrame::ShowBasicAttr()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowBasicAttr();
		return;
	}
}

/// DISK BASICが使用できるか
bool L3DiskFrame::CanUseBasicDisk()
{
	L3DiskFileList *list = GetFileListPanel();
	if (list) {
		return list->CanUseBasicDisk();
	}
	return false;
}

/// ディレクトリをアサイン
bool L3DiskFrame::AssignDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dir_item)
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (!listpanel) return false;
	return listpanel->AssignDirectory(disk, side_num, dir_item);
}

/// ディレクトリを移動
bool L3DiskFrame::ChangeDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dir_item, bool refresh_list)
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (!listpanel) return false;
	return listpanel->ChangeDirectory(disk, side_num, dir_item, refresh_list);
}

/// ディレクトリを削除
bool L3DiskFrame::DeleteDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dir_item)
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (!listpanel) return false;
	return listpanel->DeleteDirectory(disk, side_num, dir_item);
}

/// ファイル名属性プロパティダイアログをすべて閉じる
void L3DiskFrame::CloseAllFileAttr()
{
	L3DiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->CloseAllFileAttr();
}

////////////////////////////////////////
//
// 左パネル
//

/// 左パネルを返す
L3DiskList *L3DiskFrame::GetLPanel()
{
	return panel->GetLPanel();
}
/// 左パネルのディスクツリーを返す
L3DiskList *L3DiskFrame::GetDiskListPanel()
{
	return panel->GetLPanel();
}
/// 左パネルのディスクツリーにデータを設定する
void L3DiskFrame::SetDiskListData(const wxString &filename)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->SetFileName(filename);
}
/// 左パネルのディスクツリーをクリア
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
/// 左パネルのディスクツリーのディスクを選択しているか
bool L3DiskFrame::IsDiskListSelectedDisk()
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) return lpanel->IsSelectedDisk();
	else return false;
}

/// 左パネルのディスクツリーの選択している位置
/// @param [out] disk_number ディスク番号
/// @param [out] side_number サイド番号
void L3DiskFrame::GetDiskListSelectedPos(int &disk_number, int &side_number)
{
	L3DiskList *lpanel = GetDiskListPanel();
	int num = -1;
	int sid = -1;
	if (lpanel) {
		lpanel->GetSelectedDisk(num, sid);
	}
	disk_number = num;
	side_number = sid;
}

/// 左パネルのディスクツリーを選択
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
void L3DiskFrame::SetDiskListPos(int disk_number, int side_number)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->ChangeSelection(disk_number, side_number);
	}
}

/// 左パネルのディスクツリーにルートディレクトリを設定
void L3DiskFrame::RefreshRootDirectoryNodeOnDiskList(DiskD88Disk *disk, int side_number)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshRootDirectoryNode(disk, side_number);
	}
}

/// 左パネルのディスクツリーにディレクトリを設定
void L3DiskFrame::RefreshDirectoryNodeOnDiskList(DiskD88Disk *disk, DiskBasicDirItem *dir_item)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshDirectoryNode(disk, dir_item);
	}
}

/// 左パネルの全てのディレクトリツリーを更新
void L3DiskFrame::RefreshAllDirectoryNodesOnDiskList(DiskD88Disk *disk, int side_number)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshAllDirectoryNodes(disk, side_number);
	}
}

/// 左パネルのディスクツリーのディレクトリを選択
void L3DiskFrame::SelectDirectoryNodeOnDiskList(DiskD88Disk *disk, DiskBasicDirItem *dir_item)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->SelectDirectoryNode(disk, dir_item);
	}
}

/// 左パネルのディスクツリーのディレクトリを削除
void L3DiskFrame::DeleteDirectoryNodeOnDiskList(DiskD88Disk *disk, DiskBasicDirItem *dir_item)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->DeleteDirectoryNode(disk, dir_item);
	}
}

/// 左パネルのディスクツリーのディレクトリを一括削除
void L3DiskFrame::DeleteDirectoryNodesOnDiskList(DiskD88Disk *disk, DiskBasicDirItems &dir_items)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->DeleteDirectoryNodes(disk, dir_items);
	}
}

/// 左パネルのディスクツリーのディレクトリ名を再設定
void L3DiskFrame::RefreshDiskListDirectoryName(DiskD88Disk *disk)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshDirectoryName(disk);
	}
}

/// 左パネルのディスクツリーにファイルパスを設定
void L3DiskFrame::SetDiskListFilePath(const wxString &path)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->SetFilePath(path);
	}
}

/// 左パネルのディスクツリーにディスク名を設定
void L3DiskFrame::SetDiskListName(const wxString &name)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->SetName(name);
	}
}

/// 左パネルの選択しているディスクの子供を削除
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void L3DiskFrame::RefreshDiskListOnSelectedDisk(const DiskBasicParam *newparam)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshSelectedDisk(newparam);
	}
}

/// 選択しているディスクのサイドを再選択
void L3DiskFrame::RefreshDiskListOnSelectedSide(const DiskBasicParam *newparam)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshSelectedSide(newparam);
	}
}

/// 左パネルのディスクツリーを再選択
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void L3DiskFrame::ReSelectDiskList(const DiskBasicParam *newparam)
{
	L3DiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->ReSelect(newparam);
	}
}

////////////////////////////////////////
//
// 右パネル
//

/// 右パネルを返す
L3DiskRPanel *L3DiskFrame::GetRPanel()
{
	return panel->GetRPanel();
}

/// 右パネルのデータウィンドウを変更 ファイルリスト/RAWディスク
void L3DiskFrame::ChangeRPanel(int num, const DiskBasicParam *param)
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) rpanel->ChangePanel(num);
	L3DiskList *lpanel = panel->GetLPanel();
	if (lpanel) lpanel->ReSelect(param);

	UpdateMenuDisk();
	UpdateMenuMode();
}

/// 右パネルのすべてのコントロール内のデータをクリア
void L3DiskFrame::ClearRPanelData()
{
	DetachDiskBasicOnFileList();
	ClearDiskAttrData();
	ClearFileListData();
	ClearRawPanelData();
	ClearBinDumpData();
	ClearFatAreaData();
}

////////////////////////////////////////
//
// 右上パネルのディスク属性
//

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

////////////////////////////////////////
//
// 右下パネルのファイルリスト
//

/// 右下パネルのファイルリストパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
L3DiskFileList *L3DiskFrame::GetFileListPanel(bool inst)
{
	L3DiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetFileListPanel(inst);
	else return NULL;
}
/// 右下パネルのファイルリストにDISK BASICをアタッチ
void L3DiskFrame::AttachDiskBasicOnFileList(DiskD88Disk *disk, int side_num)
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->AttachDiskBasic(disk, side_num);
}
/// 右下パネルのファイルリストからDISK BASICをデタッチ
void L3DiskFrame::DetachDiskBasicOnFileList()
{
	L3DiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->DetachDiskBasic();
}
/// 右下パネルのファイルリストにデータを設定する
void L3DiskFrame::SetFileListData()
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->SetFiles();
}
/// 右下パネルのファイルリストをクリア
void L3DiskFrame::ClearFileListData()
{
	L3DiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->ClearFiles();
}

////////////////////////////////////////
//
// 右下パネルのRAWディスクパネル
//

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
/// 右下パネルのRAWディスクパネルにデータを再設定する
void L3DiskFrame::RefreshRawPanelData()
{
	L3DiskRawPanel *rawpanel = GetDiskRawPanel();
	if (rawpanel) rawpanel->RefreshTrackListData();
}

////////////////////////////////////////
//
// ダンプウィンドウ
//

/// ダンプウィンドウにデータを設定する
void L3DiskFrame::SetBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len, const wxString &char_code, bool invert)
{
	if (bindump_frame) {
		bindump_frame->SetDataInvert(invert);
//		bindump_frame->SetDataChar(char_code);
		bindump_frame->SetDatas(trk, sid, sec, buf, len);
	}
}
/// ダンプウィンドウにデータを設定する
void L3DiskFrame::SetBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	if (bindump_frame) {
		bindump_frame->SetDatas(trk, sid, sec, buf, len);
	}
}
/// ダンプウィンドウにデータを追記する
void L3DiskFrame::AppendBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len, const wxString &char_code, bool invert)
{
	if (bindump_frame) {
		bindump_frame->SetDataInvert(invert);
//		bindump_frame->SetDataChar(char_code);
		bindump_frame->AppendDatas(trk, sid, sec, buf, len);
	}
}
/// ダンプウィンドウにデータを追記する
void L3DiskFrame::AppendBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	if (bindump_frame) {
		bindump_frame->AppendDatas(trk, sid, sec, buf, len);
	}
}
/// ダンプウィンドウをクリア
void L3DiskFrame::ClearBinDumpData()
{
	if (bindump_frame) {
		bindump_frame->ClearDatas();
	}
}

/// ダンプウィンドウを開く
void L3DiskFrame::OpenBinDumpWindow()
{
	if (bindump_frame) return;

	// ウィンドウを開く
	bindump_frame = new L3DiskBinDumpFrame(this, _("Dump View"), wxSize(640, 480));
	// 位置はメインウィンドウの右側
	wxSize sz = GetSize();
	wxPoint pt = GetPosition();
	pt.x = pt.x + sz.x;
//	pt.y = pt.y;
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
		wxMenuItem *mitem = menuView->FindItem(IDM_WINDOW_BINDUMP);
		if (mitem) {
			mitem->Check(false);
		}
	}
}

////////////////////////////////////////
//
// 使用状況ウィンドウ
//

/// 使用状況ウィンドウにデータを設定する
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

/// 使用状況ウィンドウにデータを設定する
void L3DiskFrame::SetFatAreaData(wxUint32 offset, const wxArrayInt *arr)
{
	if (fatarea_frame && arr) {
		fatarea_frame->SetData(offset, arr);
	}
}
/// 使用状況ウィンドウをクリア
void L3DiskFrame::ClearFatAreaData()
{
	if (fatarea_frame) {
		fatarea_frame->ClearData();
	}
}
/// 使用状況ウィンドウにフォーカスさせるグループ番号を設定する
void L3DiskFrame::SetFatAreaGroup(wxUint32 group_num)
{
	if (fatarea_frame) {
		fatarea_frame->SetGroup(group_num);
	}
}
/// 使用状況ウィンドウにフォーカスさせるグループ番号を設定する
void L3DiskFrame::SetFatAreaGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums)
{
	if (fatarea_frame) {
		fatarea_frame->SetGroup(group_items, extra_group_nums);
	}
}
/// 使用状況ウィンドウにフォーカスをはずすグループ番号を設定する
void L3DiskFrame::UnsetFatAreaGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums)
{
	if (fatarea_frame) {
		fatarea_frame->UnsetGroup(group_items, extra_group_nums);
	}
}
/// 使用状況ウィンドウでフォーカスしているグループ番号をクリア
void L3DiskFrame::ClearFatAreaGroup()
{
	if (fatarea_frame) {
		fatarea_frame->ClearGroup();
	}
}
/// 使用状況ウィンドウを開く
void L3DiskFrame::OpenFatAreaWindow()
{
	if (fatarea_frame) return;

	// ウィンドウを開く
	fatarea_frame = new L3DiskFatAreaFrame(this, _("Availability"), wxDefaultSize);
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
/// 使用状況ウィンドウを閉じる
void L3DiskFrame::CloseFatAreaWindow()
{
	if (!fatarea_frame) return;

	fatarea_frame->Close();
	fatarea_frame = NULL;
}
/// 使用状況ウィンドウを閉じる時にウィンドウ側から呼ばれるコールバック
void L3DiskFrame::FatAreaWindowClosed()
{
	fatarea_frame = NULL;

	if (!IsBeingDeleted()) {
		wxMenuItem *mitem = menuView->FindItem(IDM_WINDOW_FATAREA);
		if (mitem) {
			mitem->Check(false);
		}
	}
}

////////////////////////////////////////
//
// 設定ファイル
//

/// 最近使用したパスを取得
const wxString &L3DiskFrame::GetIniRecentPath() const
{
	return gConfig.GetFilePath();
}

/// 最近使用したパスを取得(エクスポート用)
const wxString &L3DiskFrame::GetIniExportFilePath() const
{
	return gConfig.GetExportFilePath();
}

/// 最近使用したファイルを更新（一覧も更新）
void L3DiskFrame::SetIniRecentPath(const wxString &path)
{
	// set recent file path
	gConfig.AddRecentFile(path);
	UpdateMenuRecentFiles();
}

/// 最近使用したパスを更新
void L3DiskFrame::SetIniFilePath(const wxString &path)
{
	gConfig.SetFilePath(path);
}

/// 最近使用したパスを更新(エクスポート用)
void L3DiskFrame::SetIniExportFilePath(const wxString &path, bool is_dir)
{
	gConfig.SetExportFilePath(path, is_dir);
}

/// ダンプフォントを更新
void L3DiskFrame::SetIniDumpFont(const wxFont &font)
{
	gConfig.SetDumpFontName(font.GetFaceName());
	gConfig.SetDumpFontSize(font.GetPointSize());
}

/// ダンプフォント名を返す
const wxString &L3DiskFrame::GetIniDumpFontName() const
{
	return gConfig.GetDumpFontName();
}

/// ダンプフォントサイズを返す
int L3DiskFrame::GetIniDumpFontSize() const
{
	return gConfig.GetDumpFontSize();
}

/// 設定ダイアログ表示
void L3DiskFrame::ShowConfigureDialog()
{
	ConfigBox dlg(this, wxID_ANY, &gConfig);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		dlg.CommitData();
		// メニューを更新
		UpdateMenuMode();
		// ファイルリストを更新
		SetFileListData();
	}
}

/// ステータスカウンタを開始
int L3DiskFrame::StartStatusCounter(int count, const wxString &message)
{
	return stat_counters.Start(count, message);
}
/// ステータスカウンタに母数を追加
void L3DiskFrame::AppendStatusCounter(int idx, int count)
{
	stat_counters.Append(idx, count);
}
/// ステータスカウンタの数を＋１
void L3DiskFrame::IncreaseStatusCounter(int idx)
{
	stat_counters.Increase(idx);
	SetStatusText(stat_counters.GetCurrentMessage(idx), 0);
}
/// ステータスカウンタの計数を終了
void L3DiskFrame::FinishStatusCounter(int idx, const wxString &message)
{
	stat_counters.Finish(idx, message, this);
	SetStatusText(stat_counters.GetCurrentMessage(idx), 0);
}
/// ステータスカウンタをクリア
void L3DiskFrame::ClearStatusCounter()
{
	stat_counters.Clear();
	SetStatusText(wxT(""), 0);
}

//////////////////////////////////////////////////////////////////////

// ドラッグアンドドロップ時のフォーマットID
wxDataFormat *L3DiskPanelDataFormat = NULL;

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

	SetDropTarget(new L3DiskPanelDropTarget(parent, this));
}

L3DiskPanel::~L3DiskPanel()
{
}

/// 外部からのDnD
bool L3DiskPanel::ProcessDroppedFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	if (filenames.Count() == 0) return false;

	bool is_disk_file = false;

	// 分割位置
	int pos_x = GetSashPosition();
	bool disk_is_empty = (frame->GetDiskD88().CountDisks() == 0);

	if (disk_is_empty || x < pos_x) {
		is_disk_file = true;
	}
	bool sts = false;
	if (is_disk_file) {
		// ディスクイメージ１つだけ開く
		frame->OpenDroppedFile(filenames.Item(0));
	} else {
		// ファイルをインポート
		L3DiskFileList *file_list = rpanel->GetFileListPanel();
		if (file_list) {
			sts = file_list->DropDataFiles(filenames);
		}
		L3DiskRawPanel *raw_panel = rpanel->GetRawPanel();
		if (raw_panel) {
			for(int n = 0; n < (int)filenames.Count(); n++) {
				wxString filename = filenames.Item(n);
				sts = raw_panel->ShowImportTrackRangeDialog(filename);
			}
		}
	}
	return sts;
}

//////////////////////////////////////////////////////////////////////
//
// File Drag and Drop
//
L3DiskPanelDropTarget::L3DiskPanelDropTarget(L3DiskFrame *parentframe, L3DiskPanel *parentwindow)
	: wxDropTarget()
{
	parent = parentwindow;
	frame = parentframe;

	wxDataObjectComposite* dataobj = new wxDataObjectComposite();

	dataobj->Add(new wxFileDataObject());
	SetDataObject(dataobj);
}

wxDragResult L3DiskPanelDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	if ( !GetData() ) return wxDragNone;
	bool sts = false;
	wxDataObjectComposite *comobj = (wxDataObjectComposite *)GetDataObject();
	if (comobj) {
		wxDataFormat fmt = comobj->GetReceivedFormat();
//		wxDataFormat fmt = comobj->GetPreferredFormat();
		if (fmt.GetType() == wxDF_FILENAME) {
			// エクスプローラからのDnD
			wxFileDataObject *dobj = (wxFileDataObject *)comobj->GetObject(fmt);
			sts = parent->ProcessDroppedFiles(x, y, dobj->GetFilenames());
		}
	}
	return (sts ? def : wxDragError);
}

//////////////////////////////////////////////////////////////////////
//
// File Dialog
//
L3DiskFileDialog::L3DiskFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard, long style)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wildcard, style)
{
}

//////////////////////////////////////////////////////////////////////
//
// Dir Dialog
//
L3DiskDirDialog::L3DiskDirDialog(const wxString& message, const wxString& defaultDir, long style)
            : wxDirDialog(NULL, message, defaultDir, style)
{
}

//////////////////////////////////////////////////////////////////////
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
#ifdef _DEBUG
	str += _T("(Debug Version)\n\n");
#endif
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
