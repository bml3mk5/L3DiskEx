/// @file uimainframe.cpp
///
/// @brief メインフレーム
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uimainframe.h"
#include "../main.h"
#include <wx/toolbar.h>
//#include <wx/cmdline.h>
//#include <wx/filename.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/dir.h>
#include "../charcodes.h"
#include "diskparambox.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basicdiritem.h"
#include "mymenu.h"
#include "uidisklist.h"
#include "uirpanel.h"
#include "uidiskattr.h"
#include "uifilelist.h"
#include "uirawdisk.h"
#include "uibindump.h"
#include "uifatarea.h"
#include "fileselbox.h"
#include "diskreplacebox.h"
#include "fontminibox.h"
#include "configbox.h"
#include "../diskimg/diskd88.h"
#include "../diskimg/diskwriter.h"
#include "../diskimg/diskresult.h"
#include "../diskimg/fileparam.h"
#include "../logging.h"
#include "../version.h"
// icon
#include "../res/fd_5inch_16_1.xpm"
#include "../res/fd_5inch_16_2.xpm"
#include "../res/fd_5inch_16_new.xpm"
#include "../res/fd_5inch_16_open.xpm"
#include "../res/fd_5inch_16_add.xpm"
#include "../res/fd_5inch_16_delete.xpm"
#include "../res/fd_5inch_16_export.xpm"
#include "../res/fd_5inch_16_import.xpm"
#include "../res/foldericon_open.xpm"
#include "../res/foldericon_close.xpm"
#include "../res/fileicon_normal.xpm"
#include "../res/fileicon_delete.xpm"
#include "../res/fileicon_hidden.xpm"
#include "../res/labelicon_normal.xpm"
//#include "../res/triangle_down.xpm"
//#include "../res/triangle_up.xpm"

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
wxBEGIN_EVENT_TABLE(UiDiskFrame, wxFrame)
	// menu event
	EVT_CLOSE(UiDiskFrame::OnClose)

	EVT_MENU(wxID_EXIT,  UiDiskFrame::OnQuit)
	EVT_MENU(wxID_ABOUT, UiDiskFrame::OnAbout)

	EVT_MENU(IDM_NEW_FILE, UiDiskFrame::OnCreateFile)
	EVT_MENU(IDM_OPEN_FILE, UiDiskFrame::OnOpenFile)

	EVT_MENU(IDM_CLOSE_FILE, UiDiskFrame::OnCloseFile)
	EVT_MENU(IDM_SAVEAS_FILE, UiDiskFrame::OnSaveAsFile)
	EVT_MENU(IDM_SAVE_DISK, UiDiskFrame::OnSaveDisk)

	EVT_MENU(IDM_ADD_DISK, UiDiskFrame::OnAddDiskFromFile)
	EVT_MENU(IDM_ADD_DISK_NEW, UiDiskFrame::OnAddNewDisk)
	EVT_MENU(IDM_ADD_DISK_FROM_FILE, UiDiskFrame::OnAddDiskFromFile)

	EVT_MENU(IDM_REPLACE_DISK_FROM_FILE, UiDiskFrame::OnReplaceDisk)

	EVT_MENU(IDM_DELETE_DISK_FROM_FILE, UiDiskFrame::OnDeleteDiskFromFile)
	EVT_MENU(IDM_RENAME_DISK, UiDiskFrame::OnRenameDisk)

	EVT_MENU_RANGE(IDM_RECENT_FILE_0, IDM_RECENT_FILE_0 + MAX_RECENT_FILES - 1, UiDiskFrame::OnOpenRecentFile)

	EVT_MENU(IDM_INITIALIZE_DISK, UiDiskFrame::OnInitializeDisk)
	EVT_MENU(IDM_FORMAT_DISK, UiDiskFrame::OnFormatDisk)

	EVT_MENU(IDM_EXPORT_DATA, UiDiskFrame::OnExportDataFromDisk)
	EVT_MENU(IDM_IMPORT_DATA, UiDiskFrame::OnImportDataToDisk)
	EVT_MENU(IDM_DELETE_DATA, UiDiskFrame::OnDeleteDataFromDisk)
	EVT_MENU(IDM_RENAME_DATA_ON_DISK, UiDiskFrame::OnRenameDataOnDisk)
	EVT_MENU(IDM_COPY_DATA, UiDiskFrame::OnCopyDataFromDisk)
	EVT_MENU(IDM_PASTE_DATA, UiDiskFrame::OnPasteDataToDisk)
	EVT_MENU(IDM_MAKE_DIRECTORY_ON_DISK, UiDiskFrame::OnMakeDirectoryOnDisk)
	EVT_MENU(IDM_PROPERTY_DATA, UiDiskFrame::OnPropertyOnDisk)

	EVT_MENU(IDM_BASIC_MODE, UiDiskFrame::OnBasicMode)
	EVT_MENU(IDM_RAWDISK_MODE, UiDiskFrame::OnRawDiskMode)
	EVT_MENU_RANGE(IDM_CHAR_0, IDM_CHAR_0 + 10, UiDiskFrame::OnChangeCharCode)
	EVT_MENU(IDM_TRIM_DATA, UiDiskFrame::OnTrimData)
	EVT_MENU(IDM_SHOW_DELFILE, UiDiskFrame::OnShowDeletedFile)
	EVT_MENU(IDM_CONFIGURE, UiDiskFrame::OnConfigure)

	EVT_MENU(IDM_WINDOW_BINDUMP, UiDiskFrame::OnOpenBinDump)
	EVT_MENU(IDM_WINDOW_FATAREA, UiDiskFrame::OnOpenFatArea)
	EVT_MENU(IDM_FILELIST_COLUMN, UiDiskFrame::OnChangeColumnsOfFileList)
	EVT_MENU(IDM_CHANGE_FONT, UiDiskFrame::OnChangeFont)

	EVT_TIMER(StatusCounters::IDT_STATUS_COUNTER, UiDiskFrame::OnTimerStatusCounter)

#ifdef USE_MENU_OPEN
	EVT_MENU_OPEN(UiDiskFrame::OnMenuOpen)
#endif
wxEND_EVENT_TABLE()

// 翻訳用
#define DIALOG_BUTTON_STRING _("OK"),_("Cancel")
#define APPLE_MENU_STRING _("Hide l3diskex"),_("Hide Others"),_("Show All"),_("Quit l3diskex"),_("Services"),_("Preferences…"),_("Minimize"),_("Zoom"),_("Bring All to Front")

UiDiskFrame::UiDiskFrame(const wxString& title, const wxSize& size)
#if defined(__WXOSX__)
	: UiDiskProcess(NULL, -1, title, wxDefaultPosition, wxDefaultSize)
#else
	: UiDiskProcess(NULL, -1, title, wxDefaultPosition, size)
#endif
{
#if defined(__WXOSX__)
	SetClientSize(size);
#endif

	p_image = new DiskD88;

	// icon
#ifdef __WXMSW__
	SetIcon(wxIcon(_T(APPLICATION_NAME)));
#elif defined(__WXGTK__) || defined(__WXMOTIF__)
	SetIcon(wxIcon(APPLICATION_XPMICON_NAME));
#endif

	// menu
	MakeMenu();

	// status bar
	RecreateStatusbar();

	// tool bar
    RecreateToolbar();

	// splitter window
	panel = new UiDiskPanel(this);

	bindump_frame = NULL;
	fatarea_frame = NULL;
}

UiDiskFrame::~UiDiskFrame()
{
	// フレーム
#if defined(__WXOSX__)
	wxSize sz = GetClientSize();
#else
	wxSize sz = GetSize();
#endif
	gConfig.SetWindowWidth(sz.GetWidth());
	gConfig.SetWindowHeight(sz.GetHeight());

	delete p_image;
}

/// フレーム部の初期処理
bool UiDiskFrame::Init(const wxString &in_file)
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
void UiDiskFrame::RecreateToolbar()
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
void UiDiskFrame::PopulateToolbar(wxToolBar* toolBar)
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
	MyMenu* sm = new MyMenu;
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
void UiDiskFrame::RecreateStatusbar()
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
void UiDiskFrame::OnMenuOpen(wxMenuEvent& event)
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
void UiDiskFrame::OpenDroppedFile(const wxString &path)
{
 	if (!CloseDataFile()) return;
	PreOpenDataFile(path);
}

////////////////////////////////////////
//
// イベントプロシージャ
//

/// ウィンドウを閉じたとき
void UiDiskFrame::OnClose(wxCloseEvent& event)
{
	if (!CloseDataFile(!event.CanVeto())) {
		event.Veto();
		return;
	}
	event.Skip();
}

/// メニュー 終了選択
void UiDiskFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close(false);
}

/// メニュー Aboutダイアログ表示選択
void UiDiskFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	UiDiskAbout(this, wxID_ANY).ShowModal();
}

/// メニュー 新規作成選択
void UiDiskFrame::OnCreateFile(wxCommandEvent& WXUNUSED(event))
{
	ShowCreateFileDialog();
}

/// メニュー 開く選択
void UiDiskFrame::OnOpenFile(wxCommandEvent& WXUNUSED(event))
{
	ShowOpenFileDialog();
}

/// メニュー 最近使用したファイル開く選択
void UiDiskFrame::OnOpenRecentFile(wxCommandEvent& event)
{
	wxMenuItem *item = menuRecentFiles->FindItem(event.GetId());
	if (!item) return;
	wxFileName path = item->GetItemLabel();
	if (!CloseDataFile()) return;
	PreOpenDataFile(path.GetFullPath());
}

/// メニュー 閉じる選択
void UiDiskFrame::OnCloseFile(wxCommandEvent& WXUNUSED(event))
{
	CloseDataFile();
}
/// メニュー 名前を付けて保存選択
void UiDiskFrame::OnSaveAsFile(wxCommandEvent& WXUNUSED(event))
{
	ShowSaveFileDialog();
}
/// メニュー ディスク1枚を保存選択
void UiDiskFrame::OnSaveDisk(wxCommandEvent& WXUNUSED(event))
{
	UiDiskList *list = GetDiskListPanel();
	if (!list) return;
	list->ShowSaveDiskDialog();
}
/// メニュー ディスクを新規に追加選択
void UiDiskFrame::OnAddNewDisk(wxCommandEvent& WXUNUSED(event))
{
	ShowAddNewDiskDialog();
}
/// メニュー ディスクをファイルから追加選択
void UiDiskFrame::OnAddDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	ShowAddFileDialog();
}
/// メニュー ディスクを置換選択
void UiDiskFrame::OnReplaceDisk(wxCommandEvent& WXUNUSED(event))
{
	UiDiskList *list = GetDiskListPanel();
	if (!list) return;
	list->ReplaceDisk();
//	ShowReplaceDiskDialog(list->GetSelectedDiskNumber(), list->GetSelectedDiskSide());
}
/// メニュー ファイルからディスクを削除選択
void UiDiskFrame::OnDeleteDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	DeleteDisk();
}
/// メニュー ディスク名を変更選択
void UiDiskFrame::OnRenameDisk(wxCommandEvent& WXUNUSED(event))
{
	RenameDisk();
}
/// メニュー 初期化選択
void UiDiskFrame::OnInitializeDisk(wxCommandEvent& WXUNUSED(event))
{
	InitializeDisk();
}
/// メニュー フォーマット選択
void UiDiskFrame::OnFormatDisk(wxCommandEvent& WXUNUSED(event))
{
	FormatDisk();
}
/// メニュー エクスポート選択
void UiDiskFrame::OnExportDataFromDisk(wxCommandEvent& WXUNUSED(event))
{
	ExportDataFromDisk();
}
/// メニュー インポート選択
void UiDiskFrame::OnImportDataToDisk(wxCommandEvent& WXUNUSED(event))
{
	ImportDataToDisk();
}
/// メニュー 削除選択
void UiDiskFrame::OnDeleteDataFromDisk(wxCommandEvent& WXUNUSED(event))
{
	DeleteDataFromDisk();
}
/// メニュー リネーム選択
void UiDiskFrame::OnRenameDataOnDisk(wxCommandEvent& WXUNUSED(event))
{
	RenameDataOnDisk();
}
/// メニュー コピー選択
void UiDiskFrame::OnCopyDataFromDisk(wxCommandEvent& WXUNUSED(event))
{
	CopyDataFromDisk();
}
/// メニュー ペースト選択
void UiDiskFrame::OnPasteDataToDisk(wxCommandEvent& WXUNUSED(event))
{
	PasteDataToDisk();
}
/// メニュー ディレクトリ作成選択
void UiDiskFrame::OnMakeDirectoryOnDisk(wxCommandEvent& WXUNUSED(event))
{
	MakeDirectoryOnDisk();
}
/// メニュー プロパティ選択
void UiDiskFrame::OnPropertyOnDisk(wxCommandEvent& WXUNUSED(event))
{
	PropertyOnDisk();
}

/// メニュー 設定ダイアログ選択
void UiDiskFrame::OnConfigure(wxCommandEvent& WXUNUSED(event))
{
	ShowConfigureDialog();
}

/// ファイルモード選択
void UiDiskFrame::OnBasicMode(wxCommandEvent& event)
{
	ChangeRPanel(0, NULL);
}

/// Rawディスクモード選択
void UiDiskFrame::OnRawDiskMode(wxCommandEvent& event)
{
	ChangeRPanel(1, NULL);
}

/// キャラクターコード選択
void UiDiskFrame::OnChangeCharCode(wxCommandEvent& event)
{
	int sel = event.GetId() - IDM_CHAR_0;
	wxString name = gCharCodeChoices.GetItemName(wxT("main"), (size_t)sel);
	ChangeCharCode(name);
}

/// 未使用データを切り落とすか
void UiDiskFrame::OnTrimData(wxCommandEvent& event)
{
	gConfig.TrimUnusedData(event.IsChecked());
}

/// 削除ファイルを表示するか
void UiDiskFrame::OnShowDeletedFile(wxCommandEvent& event)
{
	gConfig.ShowDeletedFile(event.IsChecked());
	// ファイルリストを更新
	SetFileListData();
}

/// ダンプウィンドウ選択
void UiDiskFrame::OnOpenBinDump(wxCommandEvent& event)
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
void UiDiskFrame::OnOpenFatArea(wxCommandEvent& event)
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
void UiDiskFrame::OnChangeColumnsOfFileList(wxCommandEvent& event)
{
	ChangeColumnsOfFileList();
}

/// フォント変更選択
void UiDiskFrame::OnChangeFont(wxCommandEvent& event)
{
	ShowListFontDialog();
}

/// ステータスカウンター終了タイマー
void UiDiskFrame::OnTimerStatusCounter(wxTimerEvent& event)
{
	ClearStatusCounter();
}

////////////////////////////////////////
//
// ウィンドウ操作
//

/// メニューの作成
void UiDiskFrame::MakeMenu()
{
	menuFile = new MyMenu;
	menuData = new MyMenu;
	menuMode = new MyMenu;
	menuView = new MyMenu;
	menuHelp = new MyMenu;
	MyMenu *sm;

	// file menu
	menuFile->Append( IDM_NEW_FILE, _("&New...\tCTRL+N") );
	menuFile->Append( IDM_OPEN_FILE, _("&Open...\tCTRL+O") );
	menuFile->AppendSeparator();
	menuFile->Append( IDM_CLOSE_FILE, _("&Close") );
	menuFile->Append( IDM_SAVEAS_FILE, _("Save &As...\tCTRL+S") );
	menuFile->Append( IDM_SAVE_DISK, _("Save A Disk...") );
	menuFile->AppendSeparator();
		sm = new MyMenu();
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
		menuRecentFiles = new MyMenu();
		UpdateMenuRecentFiles();
	menuFile->AppendSubMenu(menuRecentFiles, _("&Reccent Files") );
	menuFile->AppendSeparator();
	menuFile->Append( wxID_EXIT, _("E&xit\tALT+F4") );
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
		sm = new MyMenu();
		const CharCodeChoice *choice = gCharCodeChoices.Find(wxT("main"));
		if (choice) {
			for(size_t i=0; i<choice->Count(); i++) {
				const CharCodeMap *map = choice->Item(i);
				sm->AppendRadioItem( IDM_CHAR_0 + (int)i, map->GetDescription() );
			}
		}
	menuMode->AppendSubMenu(sm, _("&Charactor Code") );
	menuMode->AppendSeparator();
		sm = new MyMenu();
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
	MyMenuBar *menuBar = new MyMenuBar;
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
void UiDiskFrame::UpdateMenuFile()
{
	bool opened = (p_image->GetFile() != NULL);
	menuFile->Enable(IDM_CLOSE_FILE, opened);
	menuFile->Enable(IDM_ADD_DISK_NEW, opened);
	menuFile->Enable(IDM_ADD_DISK_FROM_FILE, opened);

	opened = (opened && p_image->CountDisks() > 0);
	menuFile->Enable(IDM_SAVEAS_FILE, opened);

	UiDiskList *list = GetDiskListPanel();
	if (list) {
		UpdateMenuDiskList(list);
	}
}

/// ディスクメニューの更新
void UiDiskFrame::UpdateMenuDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		UpdateMenuFileList(list);
		return;
	}

	UiDiskRawPanel *rawpl = GetDiskRawPanel();
	if (rawpl) {
		UpdateMenuRawDisk(rawpl);
		return;
	}
}

/// モードメニューの更新
void UiDiskFrame::UpdateMenuMode()
{
	int sel = gCharCodeChoices.IndexOf(wxT("main"), gConfig.GetCharCode());
	wxMenuItem *mitem = menuMode->FindItem(IDM_CHAR_0 + sel);
	if (mitem) mitem->Check(true);
	menuMode->Check(IDM_TRIM_DATA, gConfig.IsTrimUnusedData());
	menuMode->Check(IDM_SHOW_DELFILE, gConfig.IsShownDeletedFile());
}

/// 最近使用したファイル一覧を更新
void UiDiskFrame::UpdateMenuRecentFiles()
{
	// メニューを更新
	wxArrayString names = gConfig.GetRecentFiles();
	for(int i=0; i<MAX_RECENT_FILES && i<(int)names.Count(); i++) {
		if (menuRecentFiles->FindItem(IDM_RECENT_FILE_0 + i)) menuRecentFiles->Delete(IDM_RECENT_FILE_0 + i);
		menuRecentFiles->Append(IDM_RECENT_FILE_0 + i, names[i]);
	}
}

/// ツールバーを更新
void UiDiskFrame::UpdateToolBar()
{
	wxToolBar *toolBar = GetToolBar();
	if (!toolBar) return;

	bool opened = (p_image->GetFile() != NULL);
//	toolBar->EnableTool(IDM_CLOSE_FILE, opened);
	toolBar->EnableTool(IDM_ADD_DISK, opened);
	toolBar->EnableTool(IDM_ADD_DISK_NEW, opened);
	toolBar->EnableTool(IDM_ADD_DISK_FROM_FILE, opened);

	opened = (opened && p_image->CountDisks() > 0);
	toolBar->EnableTool(IDM_SAVEAS_FILE, opened);

	UiDiskList *dlist = GetDiskListPanel();
	if (dlist) {
		UpdateToolBarDiskList(dlist);
	}

	UiDiskFileList *flist = GetFileListPanel();
	if (flist) {
		UpdateToolBarFileList(flist);
		return;
	}

	UiDiskRawPanel *rawpl = GetDiskRawPanel();
	if (rawpl) {
		UpdateToolBarRawDisk(rawpl);
		return;
	}
}

/// メニューのディスク項目を更新
void UiDiskFrame::UpdateMenuDiskList(UiDiskList *list)
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
void UiDiskFrame::UpdateToolBarDiskList(UiDiskList *list)
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
void UiDiskFrame::UpdateMenuAndToolBarDiskList(UiDiskList *list)
{
	UpdateMenuDiskList(list);
	UpdateToolBarDiskList(list);
}

/// メニューのファイルリスト項目を更新
void UiDiskFrame::UpdateMenuFileList(UiDiskFileList *list)
{
	UiDiskList *lpanel = GetLPanel();
	menuData->Enable(IDM_PROPERTY_DATA, (lpanel->IsSelectedDiskImage()));

	bool opened = (list != NULL && list->CanUseBasicDisk());
	menuFile->Enable(IDM_FORMAT_DISK, opened);

	menuData->Enable(IDM_MAKE_DIRECTORY_ON_DISK, opened && CanMakeDirectory(list->GetDiskBasic()));

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
void UiDiskFrame::UpdateToolBarFileList(UiDiskFileList *list)
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
void UiDiskFrame::UpdateMenuAndToolBarFileList(UiDiskFileList *list)
{
	UpdateMenuFileList(list);
	UpdateToolBarFileList(list);
}

/// メニューの生ディスク項目を更新
void UiDiskFrame::UpdateMenuRawDisk(UiDiskRawPanel *rawpanel)
{
	UiDiskList *lpanel = GetLPanel();
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
void UiDiskFrame::UpdateToolBarRawDisk(UiDiskRawPanel *rawpanel)
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
void UiDiskFrame::UpdateMenuAndToolBarRawDisk(UiDiskRawPanel *rawpanel)
{
	UpdateMenuRawDisk(rawpanel);
	UpdateToolBarRawDisk(rawpanel);
}

/// ウィンドウ上のデータを更新 タイトルバーにファイルパスを表示
void UiDiskFrame::UpdateDataOnWindow(const wxString &path, bool keep)
{
	// update window
	if (!path.IsEmpty()) {
		wxString title = wxGetApp().GetAppName() + wxT(" - ") + path;
		SetTitle(title);
	}
	UpdateDataOnWindow(keep);
}

/// ウィンドウ上のデータを更新
void UiDiskFrame::UpdateDataOnWindow(bool keep)
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
void UiDiskFrame::UpdateSavedDataOnWindow(const wxString &path)
{
	// 左パネルのパスを更新
	UpdateFilePathOnWindow(path);
	// Rawパネルを更新
	RefreshRawPanelData();
}

/// ウィンドウ上のファイルパスを更新
void UiDiskFrame::UpdateFilePathOnWindow(const wxString &path)
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
void UiDiskFrame::ChangeCharCode(const wxString &name)
{
	if (GetCharCode() == name) return;

	p_image->SetCharCode(name);

	UiDiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->ChangeCharCode(name);
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->ChangeCharCode(name);

	gConfig.SetCharCode(name);

	UpdateMenuMode();
}

/// キャラクターコード番号を返す
const wxString &UiDiskFrame::GetCharCode() const
{
	return gConfig.GetCharCode();
}

/// キャラクターコード番号設定
void UiDiskFrame::SetDefaultCharCode()
{
	wxString name = gConfig.GetCharCode();

	UiDiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->ChangeCharCode(name);
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->ChangeCharCode(name);
}

/// フォント変更ダイアログ
void UiDiskFrame::ShowListFontDialog()
{
	UiDiskList *rlist = GetDiskListPanel();
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
void UiDiskFrame::SetListFont(const wxFont &font)
{
	UiDiskList *rlist = GetDiskListPanel();
	if (rlist) rlist->SetListFont(font);

	UiDiskRPanel *llist = GetRPanel();
	if (llist) llist->SetListFont(font);
}

/// リストウィンドウのフォント設定
void UiDiskFrame::GetDefaultListFont(wxFont &font) const
{
	wxFont def_font = GetFont();

	wxString name = gConfig.GetListFontName();
	if (name.IsEmpty()) name = def_font.GetFaceName();
	int size = gConfig.GetListFontSize();
	if (size == 0) size = def_font.GetPointSize();

	font = wxFont(size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, name);
}

/// ファイルリストの列を変更
void UiDiskFrame::ChangeColumnsOfFileList()
{
	UiDiskFileList *llist = GetFileListPanel(true);
	if (llist) llist->ShowListColumnDialog();
}

/// 選択しているModeメニュー BASICかRAW DISKか
/// @retval 0 BASIC
/// @retval 1 RAW DISK
int UiDiskFrame::GetSelectedMode()
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
void UiDiskFrame::SetDataOnDisk(DiskImageDisk *disk, int side_number, bool refresh_list)
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
void UiDiskFrame::ClearAllData()
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
void UiDiskFrame::ClearAllAndSetRawData(DiskImageDisk *disk, int side_number)
{
	ClearFileListData();
	SetRawPanelData(disk, side_number);
	ClearBinDumpData();
	ClearFatAreaData();
	UpdateMenuAndToolBarDiskList(GetLPanel());
}

/// タイトル名を設定
wxString UiDiskFrame::MakeTitleName(const wxString &path)
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
wxString UiDiskFrame::GetFileName()
{
	return MakeTitleName(p_image->GetFileName());
}

/// 指定したウィンドウからの位置を得る
/// @param[in]  base   基準となるウィンドウ
/// @param[in]  target 対象ウィンドウ
/// @param[out] x      X座標 
/// @param[out] y      Y座標 
void UiDiskFrame::GetPositionFromBaseWindow(wxWindow *base, wxWindow *target, int &x, int &y)
{
	int cx = 0;
	int cy = 0;
	x = 0;
	y = 0;
	while(target && target != base) {
		target->GetPosition(&cx, &cy);
		x += cx;
		y += cy;
		target = target->GetParent();
	}
}

////////////////////////////////////////
//
// ディスク操作
//

/// 新規作成ダイアログ
void UiDiskFrame::ShowCreateFileDialog()
{
	if (!CloseDataFile()) return;

	DiskParamBox dlg(this, wxID_ANY, *p_image, DiskParamBox::CREATE_NEW_DISK, 0, NULL, NULL, NULL, DiskParamBox::SHOW_ALL);

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
void UiDiskFrame::CreateDataFile(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint)
{
	wxString path = MakeTitleName(wxEmptyString);

	// create disk
	int rc = p_image->Create(diskname, param, write_protect, basic_hint);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(path, false);
	}
	if (rc != 0) {
		// message
		p_image->ShowErrorMessage();
	}
	IncreaseUniqueNumber();

	UpdateToolBar();
}
/// ディスク新規追加ダイアログ
void UiDiskFrame::ShowAddNewDiskDialog()
{
	if (!p_image->GetFile()) return;

	DiskParamBox dlg(this, wxID_ANY, *p_image, DiskParamBox::ADD_NEW_DISK, p_image->GetDiskTypeNumber(0), NULL, NULL, NULL, DiskParamBox::SHOW_ALL);

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
void UiDiskFrame::AddNewDisk(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint)
{
	if (!p_image->GetFile()) return;

	// add a disk
	int rc = p_image->Add(diskname, param, write_protect, basic_hint);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString, false);
	}
	if (rc != 0) {
		// message
		p_image->ShowErrorMessage();
	}
}
/// オープンダイアログ
void UiDiskFrame::ShowOpenFileDialog()
{
	if (!CloseDataFile()) return;

	UiDiskOpenFileDialog dlg(
		_("Open File"),
		GetIniRecentPath(),
		wxEmptyString,
		gFileTypes.GetWildcardForLoad());

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
bool UiDiskFrame::PreOpenDataFile(const wxString &path)
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
int UiDiskFrame::CheckOpeningDataFile(const wxString &path, const wxString &ext, wxString &file_format, DiskParam &param_hint)
{
	DiskParamPtrs	n_disk_params;	// パラメータ候補
	DiskParam		n_manual_param;	// 手動設定時のパラメータ
	int rc = 1;

	const FileParam *fitem = gFileTypes.FindExt(ext);
	if (!fitem) {
		// 不明の拡張子なのでファイル種類を選択してもらう
		rc = ShowFileSelectDialog(path, file_format) ? 1 : 0;
	}
	if (rc == 1) {
		// 既知の拡張子ならファイルをチェックする
		rc = p_image->Check(path, file_format, n_disk_params, n_manual_param);
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
		p_image->ShowErrorMessage();
	}
	return rc;
}
/// 指定したディスクイメージを開く
/// @param [in] path        ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] param_hint  ディスクパラメータヒント(plain時のみ)
bool UiDiskFrame::OpenDataFile(const wxString &path, const wxString &file_format, const DiskParam &param_hint)
{
	bool valid = false;

	// set recent file path
	SetIniRecentPath(path);

	// open disk
	int rc = p_image->Open(path, file_format, param_hint);
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
		p_image->ShowErrorMessage();
	}

	IncreaseUniqueNumber();

	UpdateToolBar();

	return valid;
}

/// ファイル追加ダイアログ
void UiDiskFrame::ShowAddFileDialog()
{
	UiDiskOpenFileDialog dlg(
		_("Add File"),
		GetIniRecentPath(),
		wxEmptyString,
		gFileTypes.GetWildcardForLoad());

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
bool UiDiskFrame::PreAddDiskFile(const wxString &path)
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
void UiDiskFrame::AddDiskFile(const wxString &path, const wxString &file_format, const DiskParam &param_hint)
{
	// set recent file path
	SetIniRecentPath(path);

	// open disk
	int	rc = p_image->Add(path, file_format, param_hint);
	if (rc >= 0) {
		// update window
		UpdateDataOnWindow(wxEmptyString, false);
	}

	if (rc != 0) {
		// message
		p_image->ShowErrorMessage();
	}
}

/// ファイル種類選択ダイアログ
/// @param [in]  path        ファイルパス
/// @param [out] file_format 選択したファイルの形式名("d88","plain"など)
/// @return true OKボタンを押した
bool UiDiskFrame::ShowFileSelectDialog(const wxString &path, wxString &file_format)
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
bool UiDiskFrame::ShowParamSelectDialog(const wxString &path, const DiskParamPtrs &disk_params, const DiskParam *manual_param, DiskParam &param_hint)
{
	// パラメータを選択
	DiskParamBox dlg(this, wxID_ANY, *p_image, DiskParamBox::SELECT_DISK_TYPE, 0, NULL, &disk_params, manual_param, DiskParamBox::SHOW_TEMPLATE);
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
bool UiDiskFrame::CloseDataFile(bool force)
{
	// 変更されているか
	if (!force && p_image->IsModified()) {
		int ans = wxMessageBox(_("This file is modified. Do you want to save it?"), _("Close File"), wxYES_NO | wxCANCEL | wxICON_INFORMATION);  
		if (ans == wxCANCEL) {
			return false;
		}
		if (ans == wxYES) {
			ShowSaveFileDialog();
		}
	}

	IncreaseUniqueNumber();
	
	// プロパティウィンドウを閉じる
	CloseAllFileAttr();

	// update window
	wxString title = wxGetApp().GetAppName();
	SetTitle(title);

	// update panel
	ClearRPanelData();
	ClearDiskListData();

	//
	p_image->Close();

#ifndef USE_MENU_OPEN
	// update menu
	UpdateMenuFile();
	UpdateMenuMode();
#endif
	UpdateToolBar();

	return true;
}
/// 保存ダイアログ
void UiDiskFrame::ShowSaveFileDialog()
{
	// オープン時または前回保存時のフォーマットを取得
	wxString format_type = p_image->GetFormatType();
	wxString extension = p_image->GetFileExt();

	if (!DiskWriter::SupportedFormat(format_type)) {
		// d88 is default
		extension = wxT("d88");
	}

	wxString filename = p_image->GetFileNameBase();
	if (!extension.IsEmpty()) {
		filename += wxT(".");
		filename += extension;
	}

	// ダイアログ表示
	UiDiskSaveFileDialog dlg(
		_("Save File"),
		p_image->GetPath(),
		filename,
		gFileTypes.GetWildcardForSave(format_type, extension));

	int rc = dlg.ShowModal();

	if (rc == wxID_OK) {
		// 選択した形式、拡張子で保存
		wxString path = dlg.GetPath();
		gFileTypes.GetFormatByIndexForSave(dlg.GetFilterIndex(), format_type);
		gFileTypes.GetExtByIndexForSave(dlg.GetFilterIndex(), extension);
		SaveDataFile(path, format_type, extension);
	}
}
/// 指定したファイルに保存
/// @param [in] path        ファイルパス
/// @param [in] file_format 保存フォーマット形式
/// @param [in] ext         拡張子("d88"など)
void UiDiskFrame::SaveDataFile(const wxString &path, const wxString &file_format, const wxString &ext)
{
	// set recent file path
	SetIniRecentPath(path);

	int rc;

	// validate disk
	rc = p_image->CanSave(file_format);
	if (rc != 0) {
		rc = p_image->ShowErrWarnMessage();
		if (rc < 0) return;
	}

	// save disk
	rc = p_image->Save(path, file_format,
		DiskWriteOptions(
			gConfig.IsTrimUnusedData()
		)
	);
	if (rc >= 0) {
		// store the state saved format and extension
		p_image->SetFormatType(file_format);
		p_image->SetFileExt(ext);
		// update window
		UpdateSavedDataOnWindow(path);
	}
	if (rc != 0) {
		// message
		p_image->ShowErrorMessage();
	}
}
/// ディスクをファイルに保存ダイアログ（指定ディスク）
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] each_sides  OSが各面で異なる
void UiDiskFrame::ShowSaveDiskDialog(int disk_number, int side_number, bool each_sides)
{
	// ファイル名生成
	wxString filename = p_image->GetDiskName(disk_number, true);
	if (filename.IsEmpty()) {
		filename = p_image->GetFileNameBase();
		filename += wxString::Format(wxT("_%02d"), disk_number);
	}
	if (side_number >= 0) {
		filename += wxT("_");
		filename += Utils::GetSideNumStr(side_number, each_sides);
	}

	// オープン時または前回保存時のフォーマットを取得
	wxString format_type = p_image->GetFormatType();
	wxString extension = p_image->GetFileExt();

	if (!DiskWriter::SupportedFormat(format_type)) {
		// d88 is default
		extension = wxT("d88");
	}

	if (!extension.IsEmpty()) {
		filename += wxT(".");
		filename += extension;
	}

	// ダイアログ表示
	wxString title = _("Save Disk");
	title += Utils::GetSideStr(side_number, each_sides);
	UiDiskSaveFileDialog dlg(
		title,
		GetIniRecentPath(),
		filename,
		gFileTypes.GetWildcardForSave(format_type, extension));

	int rc = dlg.ShowModal();

	if (rc == wxID_OK) {
		// 選択した形式、拡張子で保存
		wxString path = dlg.GetPath();
		gFileTypes.GetFormatByIndexForSave(dlg.GetFilterIndex(), format_type);
		gFileTypes.GetExtByIndexForSave(dlg.GetFilterIndex(), extension);
		SaveDataDisk(disk_number, side_number, path, format_type, extension);
	}
}
/// 指定したファイルに保存（指定ディスク）
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] path        ファイルパス
/// @param [in] file_format 保存フォーマット形式
/// @param [in] ext         拡張子("d88"など)
void UiDiskFrame::SaveDataDisk(int disk_number, int side_number, const wxString &path, const wxString &file_format, const wxString &ext)
{
	// set recent file path
	SetIniRecentPath(path);

	int rc;

	// validate disk
	rc = p_image->CanSave(file_format);
	if (rc != 0) {
		rc = p_image->ShowErrWarnMessage();
		if (rc < 0) return;
	}

	// save disk
	rc = p_image->SaveDisk(disk_number, side_number, path, file_format,
		DiskWriteOptions(
			gConfig.IsTrimUnusedData()
		)
	);
	if (rc >= 0) {
		// store the state saved format and extension
		p_image->SetFormatType(file_format);
		p_image->SetFileExt(ext);
		// RAWパネルだけ更新
		RefreshRawPanelData();
	}
	if (rc != 0) {
		// message
		p_image->ShowErrorMessage();
	}
}
/// ディスクイメージ置換ダイアログ
/// @param [in] disk_number 置き換え対象ディスク番号
/// @param [in] side_number 置き換え対象ディスクのサイド番号
/// @param [in] subcaption  サブキャプション
void UiDiskFrame::ShowReplaceDiskDialog(int disk_number, int side_number, const wxString &subcaption)
{
	wxString title = _("Replace Disk Data");
	if (!subcaption.IsEmpty()) {
		title += wxT(" (");
		title += subcaption;
		title += wxT(")");
	}
	UiDiskOpenFileDialog dlg(
		title,
		GetIniRecentPath(),
		wxEmptyString,
		gFileTypes.GetWildcardForLoad());

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
bool UiDiskFrame::PreReplaceDisk(int disk_number, int side_number, const wxString &path)
{
	wxFileName file_path(path);
	wxString   file_format;
	DiskParam  param_hint;

	int rc = CheckOpeningDataFile(path, file_path.GetExt(), file_format, param_hint);
	if (rc < 0) {
		// エラー終了
		return false;
	}

	DiskImageFile *src_file = p_image->NewImageFile();
	DiskImageDisk *tag_disk = NULL;
	rc = p_image->ParseForReplace(disk_number, side_number, path, file_format, param_hint, *src_file, tag_disk);
	if (rc < 0) {
		// エラー終了
		p_image->ShowErrorMessage();
		delete src_file;
		return false;
	}

	DiskReplaceBox dlg(this, wxID_ANY, side_number, *src_file, *tag_disk);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		// 選択したディスク
		DiskImageDisk *src_disk = src_file->GetDisk(dlg.GetSelectedDiskNumber());
		// 置き換え
		ReplaceDisk(disk_number, side_number, path, src_disk, dlg.GetSelectedSideNumber(), tag_disk);
	}
	delete src_file;
	return true;
}

/// 指定したディスクイメージ置換
/// @param [in] disk_number     置き換え対象ディスク番号
/// @param [in] side_number     置き換え対象ディスクのサイド番号(両面なら-1)
/// @param [in] path            置き換え元イメージファイルパス
/// @param [in] src_disk        置き換え元ディスク
/// @param [in] src_side_number 置き換え元ディスクのサイド番号
/// @param [in] tag_disk        置き換え対象ディスク
void UiDiskFrame::ReplaceDisk(int disk_number, int side_number, const wxString &path, DiskImageDisk *src_disk, int src_side_number, DiskImageDisk *tag_disk)
{
	// set recent file path
	SetIniRecentPath(path);

	// open disk
	int rc = p_image->ReplaceDisk(disk_number, side_number, src_disk, src_side_number, tag_disk);
	if (rc >= 0) {
		// update window
		ClearRPanelData();
		p_image->ClearDiskBasicParseAndAssign(disk_number, side_number);
		RefreshDiskListOnSelectedDisk();
//		ReSelectDiskList();
//		UpdateDataOnWindow(wxEmptyString, true);
	}
	if (rc != 0) {
		// message
		p_image->ShowErrorMessage();
	}
}

/// ディスクをファイルから削除
void UiDiskFrame::DeleteDisk()
{
	UiDiskList *list = GetLPanel();
	if (list) {
		list->DeleteDisk();
		return;
	}
}
/// ディスク名を変更
void UiDiskFrame::RenameDisk()
{
	UiDiskList *list = GetLPanel();
	if (list) {
		list->RenameDisk();
		return;
	}
}
/// ディスクパラメータを表示/変更
void UiDiskFrame::ShowDiskAttr()
{
	UiDiskList *list = GetLPanel();
	if (list) {
		list->ShowDiskAttr();
		return;
	}
}
/// ディスクからファイルをエクスポート
void UiDiskFrame::ExportDataFromDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowExportDataFileDialog();
		return;
	}
	UiDiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowExportDataDialog();
		return;
	}
}
/// ディスクにファイルをインポート
void UiDiskFrame::ImportDataToDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowImportDataFileDialog();
		return;
	}
	UiDiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowImportDataDialog();
		return;
	}
}
/// ディスクからファイルを削除
void UiDiskFrame::DeleteDataFromDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->DeleteDataFile();
		return;
	}
	UiDiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowDeleteDataDialog();
		return;
	}
}
/// ディスクのファイル名を変更
void UiDiskFrame::RenameDataOnDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->StartEditingFileName();
		return;
	}
}
/// ディスクのデータをコピー
void UiDiskFrame::CopyDataFromDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->CopyToClipboard();
		return;
	}
	UiDiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->CopyToClipboard();
		return;
	}
}
/// ディスクにデータをペースト
void UiDiskFrame::PasteDataToDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->PasteFromClipboard();
		return;
	}
	UiDiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->PasteFromClipboard();
		return;
	}
}
/// ディスクにディレクトリを作成
void UiDiskFrame::MakeDirectoryOnDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowMakeDirectoryDialog();
		return;
	}
}
/// ファイルのプロパティ
void UiDiskFrame::PropertyOnDisk()
{
	wxWindow *fwin = wxWindow::FindFocus();
	UiDiskList *lpanel = GetLPanel();
	if (fwin == lpanel) {
		lpanel->ShowDiskAttr();
		return;
	}
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowFileAttr();
		return;
	}
	UiDiskRawPanel *panel = GetDiskRawPanel();
	if (panel) {
		panel->ShowRawDiskAttr();
		return;
	}
}
/// ディスクを初期化
void UiDiskFrame::InitializeDisk()
{
	UiDiskList *list = GetLPanel();
	if (list) {
		list->InitializeDisk();
		return;
	}
}
/// ディスクをDISK BASIC用に論理フォーマット
void UiDiskFrame::FormatDisk()
{
	UiDiskList *list = GetDiskListPanel();
	if (list) {
		list->FormatDisk();
		return;
	}
}

/// DISK BASIC用に論理フォーマットできるか
bool UiDiskFrame::IsFormattableDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		return list->IsFormattableBasicDisk();
	}
	return false;
}

/// BASIC情報ダイアログ
void UiDiskFrame::ShowBasicAttr()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		list->ShowBasicAttr();
		return;
	}
}

/// DISK BASICが使用できるか
bool UiDiskFrame::CanUseBasicDisk()
{
	UiDiskFileList *list = GetFileListPanel();
	if (list) {
		return list->CanUseBasicDisk();
	}
	return false;
}

/// ファイル名属性プロパティダイアログをすべて閉じる
void UiDiskFrame::CloseAllFileAttr()
{
	UiDiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->CloseAllFileAttr();
}

/// エクスポート用の一時フォルダを作成
/// @param [in,out] tmp_dir_name 作成したいテンポラリフォルダ名
/// @param [out]    tmp_data_path 作成したフォルダ（データ用）
/// @param [out]    tmp_attr_path 作成したフォルダ（属性用）
bool UiDiskFrame::CreateTemporaryFolder(wxString &tmp_dir_name, wxString &tmp_data_path, wxString &tmp_attr_path) const
{
	UiDiskApp *app = &wxGetApp();

	// テンポラリディレクトリを作成
	if (!app->MakeTempDir(tmp_dir_name)) {
		return false;
	}
	// データディレクトリを作成
	tmp_data_path = wxFileName(tmp_dir_name, wxT("Datas")).GetFullPath();
	if (!wxMkdir(tmp_data_path)) {
		return false;
	}
	// 属性ディレクトリを作成
	tmp_attr_path = wxFileName(tmp_dir_name, wxT("Attrs")).GetFullPath();
	if (!wxMkdir(tmp_attr_path)) {
		return false;
	}

	return true;
}


////////////////////////////////////////
//
// 左パネル
//

/// 左パネルを返す
UiDiskList *UiDiskFrame::GetLPanel()
{
	return panel->GetLPanel();
}
/// 左パネルのディスクツリーを返す
UiDiskList *UiDiskFrame::GetDiskListPanel()
{
	return panel->GetLPanel();
}
/// 左パネルのディスクツリーにデータを設定する
void UiDiskFrame::SetDiskListData(const wxString &filename)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->SetFileName(filename);
}
/// 左パネルのディスクツリーをクリア
void UiDiskFrame::ClearDiskListData()
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) lpanel->ClearFileName();
}
//bool UiDiskFrame::IsDiskListSelectedDiskImage()
//{
//	UiDiskList *lpanel = GetDiskListPanel();
//	if (lpanel) return lpanel->IsSelectedDiskImage();
//	else return false;
//}
/// 左パネルのディスクツリーのディスクを選択しているか
bool UiDiskFrame::IsDiskListSelectedDisk()
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) return lpanel->IsSelectedDisk();
	else return false;
}

/// 左パネルのディスクツリーの選択している位置
/// @param [out] disk_number ディスク番号
/// @param [out] side_number サイド番号
void UiDiskFrame::GetDiskListSelectedPos(int &disk_number, int &side_number)
{
	UiDiskList *lpanel = GetDiskListPanel();
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
void UiDiskFrame::SetDiskListPos(int disk_number, int side_number)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->ChangeSelection(disk_number, side_number);
	}
}

/// 左パネルのディスクツリーにルートディレクトリを設定
void UiDiskFrame::RefreshRootDirectoryNodeOnDiskList(DiskImageDisk *disk, int side_number)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshRootDirectoryNode(disk, side_number);
	}
}

/// 左パネルのディスクツリーにディレクトリを設定
void UiDiskFrame::RefreshDirectoryNodeOnDiskList(DiskImageDisk *disk, DiskBasicDirItem *dir_item)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshDirectoryNode(disk, dir_item);
	}
}

/// 左パネルの全てのディレクトリツリーを更新
void UiDiskFrame::RefreshAllDirectoryNodesOnDiskList(DiskImageDisk *disk, int side_number, DiskBasicDirItem *dir_item)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshAllDirectoryNodes(disk, side_number, dir_item);
	}
}

/// 左パネルのディスクツリーのディレクトリを選択
void UiDiskFrame::SelectDirectoryNodeOnDiskList(DiskImageDisk *disk, DiskBasicDirItem *dir_item)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->SelectDirectoryNode(disk, dir_item);
	}
}

/// 左パネルのディスクツリーのディレクトリを削除
void UiDiskFrame::DeleteDirectoryNodeOnDiskList(DiskImageDisk *disk, DiskBasicDirItem *dir_item)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->DeleteDirectoryNode(disk, dir_item);
	}
}

/// 左パネルのディスクツリーのディレクトリを一括削除
void UiDiskFrame::DeleteDirectoryNodesOnDiskList(DiskImageDisk *disk, DiskBasicDirItems &dir_items)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->DeleteDirectoryNodes(disk, dir_items);
	}
}

/// 左パネルのディスクツリーのディレクトリ名を再設定
void UiDiskFrame::RefreshDiskListDirectoryName(DiskImageDisk *disk)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshDirectoryName(disk);
	}
}

/// 左パネルのディスクツリーにファイルパスを設定
void UiDiskFrame::SetDiskListFilePath(const wxString &path)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->SetFilePath(path);
	}
}

/// 左パネルのディスクツリーにディスク名を設定
void UiDiskFrame::SetDiskListName(const wxString &name)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->SetName(name);
	}
}

/// 左パネルの選択しているディスクの子供を削除
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void UiDiskFrame::RefreshDiskListOnSelectedDisk(const DiskBasicParam *newparam)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshSelectedDisk(newparam);
	}
}

/// 選択しているディスクのサイドを再選択
void UiDiskFrame::RefreshDiskListOnSelectedSide(const DiskBasicParam *newparam)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->RefreshSelectedSide(newparam);
	}
}

/// 左パネルのディスクツリーを再選択
/// @param [in] newparam BASICパラメータ 通常NULL BASICを変更した際に設定する 
void UiDiskFrame::ReSelectDiskList(const DiskBasicParam *newparam)
{
	UiDiskList *lpanel = GetDiskListPanel();
	if (lpanel) {
		lpanel->ReSelect(newparam);
	}
}

////////////////////////////////////////
//
// 右パネル
//

/// 右パネルを返す
UiDiskRPanel *UiDiskFrame::GetRPanel()
{
	return panel->GetRPanel();
}

/// 右パネルのデータウィンドウを変更 ファイルリスト/RAWディスク
void UiDiskFrame::ChangeRPanel(int num, const DiskBasicParam *param)
{
	UiDiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) rpanel->ChangePanel(num);
	UiDiskList *lpanel = panel->GetLPanel();
	if (lpanel) lpanel->ReSelect(param);

	UpdateMenuDisk();
	UpdateMenuMode();
}

/// 右パネルのすべてのコントロール内のデータをクリア
void UiDiskFrame::ClearRPanelData()
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
UiDiskDiskAttr *UiDiskFrame::GetDiskAttrPanel()
{
	UiDiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetDiskAttrPanel();
	else return NULL;
}
/// 右上パネルのディスク属性にデータを設定する
void UiDiskFrame::SetDiskAttrData(DiskImageDisk *disk)
{
	UiDiskDiskAttr *dapanel = GetDiskAttrPanel();
	if (dapanel) dapanel->SetAttr(disk);
}
/// 右上パネルのディスク属性をクリア
void UiDiskFrame::ClearDiskAttrData()
{
	UiDiskDiskAttr *dapanel = GetDiskAttrPanel();
	if (dapanel) dapanel->ClearData();
}

////////////////////////////////////////
//
// 右下パネルのファイルリスト
//

/// 右下パネルのファイルリストパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
UiDiskFileList *UiDiskFrame::GetFileListPanel(bool inst)
{
	UiDiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetFileListPanel(inst);
	else return NULL;
}
/// 右下パネルのファイルリストにDISK BASICをアタッチ
void UiDiskFrame::AttachDiskBasicOnFileList(DiskImageDisk *disk, int side_num)
{
	UiDiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->AttachDiskBasic(disk, side_num);
}
/// 右下パネルのファイルリストからDISK BASICをデタッチ
void UiDiskFrame::DetachDiskBasicOnFileList()
{
	UiDiskFileList *listpanel = GetFileListPanel(true);
	if (listpanel) listpanel->DetachDiskBasic();
}
/// 右下パネルのファイルリストにデータを設定する
void UiDiskFrame::SetFileListData()
{
	UiDiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->SetFiles();
}
/// 右下パネルのファイルリストをクリア
void UiDiskFrame::ClearFileListData()
{
	UiDiskFileList *listpanel = GetFileListPanel();
	if (listpanel) listpanel->ClearFiles();
}

////////////////////////////////////////
//
// 右下パネルのRAWディスクパネル
//

/// 右下パネルのRAWディスクパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
UiDiskRawPanel *UiDiskFrame::GetDiskRawPanel(bool inst)
{
	UiDiskRPanel *rpanel = panel->GetRPanel();
	if (rpanel) return rpanel->GetRawPanel(inst);
	else return NULL;
}
/// 右下パネルのRAWディスクパネルにデータを設定する
void UiDiskFrame::SetRawPanelData(DiskImageDisk *disk, int side_num)
{
	UiDiskRawPanel *rawpanel = GetDiskRawPanel();
	if (rawpanel) rawpanel->SetTrackListData(disk, side_num);
}
/// 右下パネルのRAWディスクパネルをクリア
void UiDiskFrame::ClearRawPanelData()
{
	UiDiskRawPanel *rawpanel = GetDiskRawPanel();
	if (rawpanel) rawpanel->ClearTrackListData();
}
/// 右下パネルのRAWディスクパネルにデータを再設定する
void UiDiskFrame::RefreshRawPanelData()
{
	UiDiskRawPanel *rawpanel = GetDiskRawPanel();
	if (rawpanel) rawpanel->RefreshTrackListData();
}

////////////////////////////////////////
//
// ダンプウィンドウ
//

/// ダンプウィンドウにデータを設定する
void UiDiskFrame::SetBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len, const wxString &char_code, bool invert)
{
	if (bindump_frame) {
		bindump_frame->SetDataInvert(invert);
//		bindump_frame->SetDataChar(char_code);
		bindump_frame->SetDatas(trk, sid, sec, buf, len);
	}
}
/// ダンプウィンドウにデータを設定する
void UiDiskFrame::SetBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	if (bindump_frame) {
		bindump_frame->SetDatas(trk, sid, sec, buf, len);
	}
}
/// ダンプウィンドウにデータを追記する
void UiDiskFrame::AppendBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len, const wxString &char_code, bool invert)
{
	if (bindump_frame) {
		bindump_frame->SetDataInvert(invert);
//		bindump_frame->SetDataChar(char_code);
		bindump_frame->AppendDatas(trk, sid, sec, buf, len);
	}
}
/// ダンプウィンドウにデータを追記する
void UiDiskFrame::AppendBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	if (bindump_frame) {
		bindump_frame->AppendDatas(trk, sid, sec, buf, len);
	}
}
/// ダンプウィンドウをクリア
void UiDiskFrame::ClearBinDumpData()
{
	if (bindump_frame) {
		bindump_frame->ClearDatas();
	}
}

/// ダンプウィンドウを開く
void UiDiskFrame::OpenBinDumpWindow()
{
	if (bindump_frame) return;

	// ウィンドウを開く
	bindump_frame = new UiDiskBinDumpFrame(this, _("Dump View"), wxSize(640, 480));
#if 0
	// 位置はメインウィンドウの右側
	wxSize sz = GetSize();
	wxPoint pt = GetPosition();
	pt.x = pt.x + sz.x - 32;
	bindump_frame->SetPosition(pt);
#endif
	bindump_frame->Show();
	bindump_frame->SetFocus();
}
/// ダンプウィンドウを閉じる
void UiDiskFrame::CloseBinDumpWindow()
{
	if (!bindump_frame) return;

	bindump_frame->Close();
	bindump_frame = NULL;
}
/// ダンプウィンドウを閉じる時にウィンドウ側から呼ばれるコールバック
void UiDiskFrame::BinDumpWindowClosed()
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
void UiDiskFrame::SetFatAreaData()
{
	if (fatarea_frame) {
		wxUint32 offset = 0;
		const wxArrayInt *arr = NULL;
		UiDiskFileList *list = GetFileListPanel();
		if (list) {
			list->GetFatAvailability(&offset, &arr);
			SetFatAreaData(offset, arr);
		}
	}
}

/// 使用状況ウィンドウにデータを設定する
void UiDiskFrame::SetFatAreaData(wxUint32 offset, const wxArrayInt *arr)
{
	if (fatarea_frame && arr) {
		fatarea_frame->SetData(offset, arr);
	}
}
/// 使用状況ウィンドウをクリア
void UiDiskFrame::ClearFatAreaData()
{
	if (fatarea_frame) {
		fatarea_frame->ClearData();
	}
}
/// 使用状況ウィンドウにフォーカスさせるグループ番号を設定する
void UiDiskFrame::SetFatAreaGroup(wxUint32 group_num)
{
	if (fatarea_frame) {
		fatarea_frame->SetGroup(group_num);
	}
}
/// 使用状況ウィンドウにフォーカスさせるグループ番号を設定する
void UiDiskFrame::SetFatAreaGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums)
{
	if (fatarea_frame) {
		fatarea_frame->SetGroup(group_items, extra_group_nums);
	}
}
/// 使用状況ウィンドウにフォーカスをはずすグループ番号を設定する
void UiDiskFrame::UnsetFatAreaGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums)
{
	if (fatarea_frame) {
		fatarea_frame->UnsetGroup(group_items, extra_group_nums);
	}
}
/// 使用状況ウィンドウでフォーカスしているグループ番号をクリア
void UiDiskFrame::ClearFatAreaGroup()
{
	if (fatarea_frame) {
		fatarea_frame->ClearGroup();
	}
}
/// 使用状況ウィンドウを開く
void UiDiskFrame::OpenFatAreaWindow()
{
	if (fatarea_frame) return;

	// ウィンドウを開く
	fatarea_frame = new UiDiskFatAreaFrame(this, _("Availability"), wxDefaultSize);
#if 0
	// 位置はメインウィンドウの右側
	wxSize sz = GetSize();
	wxPoint pt = GetPosition();
	wxSize csz = fatarea_frame->GetSize();
	pt.x = pt.x + sz.x - 32;
	pt.y = pt.y + sz.y - csz.y;
	if (pt.y < 0) pt.y = 0;
	fatarea_frame->SetPosition(pt);
#endif
	fatarea_frame->Show();
	fatarea_frame->SetFocus();

	SetFatAreaData();
}
/// 使用状況ウィンドウを閉じる
void UiDiskFrame::CloseFatAreaWindow()
{
	if (!fatarea_frame) return;

	fatarea_frame->Close();
	fatarea_frame = NULL;
}
/// 使用状況ウィンドウを閉じる時にウィンドウ側から呼ばれるコールバック
void UiDiskFrame::FatAreaWindowClosed()
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
const wxString &UiDiskFrame::GetIniRecentPath() const
{
	return gConfig.GetFilePath();
}

/// 最近使用したパスを取得(エクスポート用)
const wxString &UiDiskFrame::GetIniExportFilePath() const
{
	return gConfig.GetExportFilePath();
}

/// 最近使用したファイルを更新（一覧も更新）
void UiDiskFrame::SetIniRecentPath(const wxString &path)
{
	// set recent file path
	gConfig.AddRecentFile(path);
	UpdateMenuRecentFiles();
}

/// 最近使用したパスを更新
void UiDiskFrame::SetIniFilePath(const wxString &path)
{
	gConfig.SetFilePath(path);
}

/// 最近使用したパスを更新(エクスポート用)
void UiDiskFrame::SetIniExportFilePath(const wxString &path, bool is_dir)
{
	gConfig.SetExportFilePath(path, is_dir);
}

/// ダンプフォントを更新
void UiDiskFrame::SetIniDumpFont(const wxFont &font)
{
	gConfig.SetDumpFontName(font.GetFaceName());
	gConfig.SetDumpFontSize(font.GetPointSize());
}

/// ダンプフォント名を返す
const wxString &UiDiskFrame::GetIniDumpFontName() const
{
	return gConfig.GetDumpFontName();
}

/// ダンプフォントサイズを返す
int UiDiskFrame::GetIniDumpFontSize() const
{
	return gConfig.GetDumpFontSize();
}

/// 設定ダイアログ表示
void UiDiskFrame::ShowConfigureDialog()
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
int UiDiskFrame::StartStatusCounter(int count, const wxString &message)
{
	return stat_counters.Start(count, message);
}
/// ステータスカウンタに母数を追加
void UiDiskFrame::AppendStatusCounter(int idx, int count)
{
	stat_counters.Append(idx, count);
}
/// ステータスカウンタの数を＋１
void UiDiskFrame::IncreaseStatusCounter(int idx)
{
	stat_counters.Increase(idx);
	SetStatusText(stat_counters.GetCurrentMessage(idx), 0);
}
/// ステータスカウンタの計数を終了
void UiDiskFrame::FinishStatusCounter(int idx, const wxString &message)
{
	stat_counters.Finish(idx, message, this);
	SetStatusText(stat_counters.GetCurrentMessage(idx), 0);
}
/// ステータスカウンタをクリア
void UiDiskFrame::ClearStatusCounter()
{
	stat_counters.Clear();
	SetStatusText(wxT(""), 0);
}

/// エクスポート用カウンタを開始
void UiDiskFrame::StartExportCounter(int count, const wxString &message)
{
	m_sw_export.Start();
	m_sw_export.SetID(StartStatusCounter(count, message));
}
/// エクスポート用カウンタの母数を追加
void UiDiskFrame::AppendExportCounter(int count)
{
	AppendStatusCounter(m_sw_export.GetID(), count);
}
/// エクスポート用カウンタの数を＋１
void UiDiskFrame::IncreaseExportCounter()
{
	IncreaseStatusCounter(m_sw_export.GetID());
}
/// エクスポート用カウンタのアイコンを時計にする
void UiDiskFrame::BeginBusyCursorExportCounterIfNeed()
{
	if (m_sw_export.Time() > 3000) {
		m_sw_export.Busy();
	}
}
/// エクスポート用カウンタを終了
void UiDiskFrame::FinishExportCounter(const wxString &message)
{
	FinishStatusCounter(m_sw_export.GetID(), message);
	m_sw_export.Finish();
}

/// インポート用カウンタを開始
void UiDiskFrame::StartImportCounter(int count, const wxString &message)
{
	m_sw_import.Start();
	m_sw_import.SetID(StartStatusCounter(count, message));
}
/// インポート用カウンタの母数を追加
void UiDiskFrame::AppendImportCounter(int count)
{
	AppendStatusCounter(m_sw_import.GetID(), count);
}
/// インポート用カウンタの数を＋１
void UiDiskFrame::IncreaseImportCounter()
{
	IncreaseStatusCounter(m_sw_import.GetID());
}
/// インポート用カウンタのアイコンを時計にする
void UiDiskFrame::BeginBusyCursorImportCounterIfNeed()
{
	if (m_sw_import.Time() > 3000) {
		m_sw_import.Busy();
	}
}
/// インポート用カウンタを終了
void UiDiskFrame::FinishImportCounter(const wxString &message)
{
	FinishStatusCounter(m_sw_import.GetID(), message);
	m_sw_import.Finish();
}
/// インポート用カウンタを再スタート
void UiDiskFrame::RestartImportCounter()
{
	m_sw_import.Restart();
}

/// 指定ファイルを引数にして外部エディタを起動する
/// @param[in] editor_type 0:binary editor 1:text editor
/// @param[in] file
bool UiDiskFrame::OpenFileWithEditor(enEditorTypes editor_type, const wxFileName &file)
{
	// エディタのあるパスを得る
	wxString editor = editor_type == EDITOR_TYPE_BINARY ? gConfig.GetBinaryEditor() : gConfig.GetTextEditor();
	if (editor.IsEmpty()) {
		wxMessageBox(_("No path of an editor specified."), _("Edit"), wxICON_ERROR | wxOK);
		return false;
	}
	// エディタを起動
	editor += wxT(" \"");
	editor += file.GetFullPath();
	editor += wxT("\"");

	wxProcess *process = NULL;
	long psts = wxExecute(editor, wxEXEC_SYNC, process);
	// エディタ終了
	if (psts < 0) {
		// コマンド起動失敗
		return false;
	}

	return true;
}

