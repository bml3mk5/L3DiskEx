/// @file main.cpp
///
/// @brief 本体
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "main.h"
#include <wx/cmdline.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/toolbar.h>
#include <wx/dir.h>
#include <wx/sysopt.h>
#include <wx/arrstr.h>
#include "ui/uimainframe.h"
#include "ui/uirpanel.h"
#include "ui/uidisklist.h"
#include "ui/uirawdisk.h"
#include "diskimg/diskparam.h"
#include "diskimg/fileparam.h"
#include "basicfmt/basictemplate.h"
#include "diskimg/diskimage.h"
#include "logging.h"
#include "version.h"
// icon
#include "res/l3diskex.xpm"

#define MYDISK_TRANS \
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

wxIMPLEMENT_APP(UiDiskApp);

wxBEGIN_EVENT_TABLE(UiDiskApp, wxApp)
#ifdef CAPTURE_MOD_KEY_ON_APP
	EVT_IDLE(UiDiskApp::OnAppIdle)
#endif
wxEND_EVENT_TABLE()

UiDiskApp::UiDiskApp()
{
	frame = NULL;
#ifdef CAPTURE_MOD_KEY_ON_APP
	mod_keys = 0;
	mod_cnt = 0;
#endif
}

/// 初期処理
bool UiDiskApp::OnInit()
{
	SetAppPath();
	SetAppName(_T(APPLICATION_NAME));

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
		mLocale.AddCatalog(_T(APPLICATION_NAME));
	}
	if (mLocale.IsLoaded(_T(APPLICATION_NAME))) {
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
	if (!gFileTypes.Load(res_path + wxT("data/"), locale_name, errmsgs)) {
		wxMessageBox(_("Cannot load file types data file."), _("Error"), wxOK);
		return false;
	}

	int w = gConfig.GetWindowWidth();
	int h = gConfig.GetWindowHeight();
	frame = new UiDiskFrame(GetAppName(), wxSize(w, h));
	frame->Show(true);
	SetTopWindow(frame);

	if (!frame->Init(in_file)) {
		return false;
	}

	return true;
}

#define OPTION_VERBOSE "verbose"

/// コマンドラインの解析
void UiDiskApp::OnInitCmdLine(wxCmdLineParser &parser)
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
bool UiDiskApp::OnCmdLineParsed(wxCmdLineParser &parser)
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
int UiDiskApp::OnExit()
{
	// save ini file
	gConfig.Save();
	// remove temp directories
	RemoveTempDirs();

	return 0;
}

#ifdef CAPTURE_MOD_KEY_ON_APP
/// アイドル時の処理
void UiDiskApp::OnAppIdle(wxIdleEvent& event)
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
int UiDiskApp::FilterEvent(wxEvent& event)
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
void UiDiskApp::MacOpenFile(const wxString &fileName)
{
	if (frame) {
		if (!frame->CloseDataFile()) return;
		frame->PreOpenDataFile(fileName);
	}
}

/// ファイルを開く(Mac用)
void UiDiskApp::MacOpenFiles(const wxArrayString &fileNames)
{
	if (frame) {
		wxString fileName = fileNames.Item(0);
		if (!frame->CloseDataFile()) return;
		frame->PreOpenDataFile(fileName);
	}
}

/// アプリケーションのパスを設定
void UiDiskApp::SetAppPath()
{
	app_path = wxFileName::FileName(argv[0]).GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
#ifdef __WXOSX__
	if (app_path.Find(_T("MacOS")) >= 0) {
		wxFileName file = wxFileName::FileName(app_path+"../../../");
		file.Normalize(wxPATH_NORM_ALL);
		ini_path = file.GetPath(wxPATH_GET_SEPARATOR);
		file = wxFileName::FileName(app_path+"../../Contents/Resources/");
		file.Normalize(wxPATH_NORM_ALL);
		res_path = file.GetPath(wxPATH_GET_SEPARATOR);
	} else
#endif
	{
		ini_path = app_path;
		res_path = app_path;
	}
}

/// アプリケーションのパスを返す
const wxString &UiDiskApp::GetAppPath()
{
	return app_path;
}

/// 設定ファイルのあるパスを返す
const wxString &UiDiskApp::GetIniPath()
{
	return ini_path;
}

/// リソースファイルのあるパスを返す
const wxString &UiDiskApp::GetResPath()
{
	return res_path;
}

/// テンポラリディレクトリを作成する
bool UiDiskApp::MakeTempDir(wxString &tmp_dir_path)
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
void UiDiskApp::RemoveTempDir(const wxString &tmp_dir_path)
{
	RemoveTempDir(tmp_dir_path, 0);

	// リストからも削除
	tmp_dirs.Remove(tmp_dir_path);
}

/// テンポラリディレクトリを削除する
void UiDiskApp::RemoveTempDir(const wxString &tmp_dir_path, int depth)
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
void UiDiskApp::RemoveTempDirs()
{
	wxArrayString dirs = tmp_dirs;
	for(size_t n = 0; n < dirs.Count(); n++) {
		RemoveTempDir(dirs.Item(n));
	}
}

//////////////////////////////////////////////////////////////////////

// ドラッグアンドドロップ時のフォーマットID
wxDataFormat *UiDiskPanelDataFormat = NULL;

//
// メインパネルは分割ウィンドウ
//
// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

UiDiskPanel::UiDiskPanel(UiDiskFrame *parent)
                : wxSplitterWindow(parent, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_BORDER | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
    frame = parent;

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	// control panel
	lpanel = new UiDiskList(frame, this);
	rpanel = new UiDiskRPanel(frame, this, frame->GetSelectedMode());
	
	SplitVertically(lpanel, rpanel, 200);

	SetMinimumPaneSize(10);

	SetDropTarget(new UiDiskPanelDropTarget(parent, this));
}

UiDiskPanel::~UiDiskPanel()
{
}

/// 外部からのDnD
/// @param[in] x         ドロップした位置X
/// @param[in] y         ドロップした位置Y
/// @param[in] filenames ドロップしたファイル名一覧
/// @return ドロップ成功/失敗
bool UiDiskPanel::ProcessDroppedFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	if (filenames.Count() == 0) return false;

	int drop_type = 0;

	// 分割位置
	int pos_x = GetSashPosition();
	bool disk_is_empty = (frame->GetDiskImage().CountDisks() == 0);
	bool include_dir = false;
	// 一覧にディレクトリが存在するか
	for(size_t i=0; i<filenames.Count(); i++) {
		if (wxFileName::DirExists(filenames.Item(i))) {
			include_dir = true;
			break;
		}
	}

	if (disk_is_empty) {
		// ディスクイメージを開いていない
		drop_type = 2;
	} else if (x < pos_x) {
		// 右側のツリーにドロップしている
		if (lpanel->HasNodeAtPoint(x, y)) {
			// ツリー内部のディレクトリなどにドロップしている
			drop_type = 1;
		} else {
			// ツリー外
			drop_type = 2;
		}
	}

	bool sts = false;
	switch(drop_type) {
	case 0:
		{
			// ファイルをインポート
			UiDiskFileList *file_list = rpanel->GetFileListPanel();
			if (file_list) {
				sts = file_list->DropDataFiles(this, x, y, filenames, include_dir);
			}
			UiDiskRawPanel *raw_panel = rpanel->GetRawPanel();
			if (raw_panel) {
				for(int n = 0; n < (int)filenames.Count(); n++) {
					wxString filename = filenames.Item(n);
					sts = raw_panel->ShowImportTrackRangeDialog(filename);
				}
			}
		}
		break;
	case 1:
		// 指定フォルダにファイルをインポート
		if (lpanel) {
			sts = lpanel->DropDataFiles(this, x, y, filenames, include_dir);
		}
		break;
	case 2:
		// ディスクイメージ１つだけ開く
		if (!include_dir) {
			frame->OpenDroppedFile(filenames.Item(0));
		}
		break;
	default:
		// nothing to do
		break;
	}
	return sts;
}

//////////////////////////////////////////////////////////////////////
//
// File Drag and Drop
//
UiDiskPanelDropTarget::UiDiskPanelDropTarget(UiDiskFrame *parentframe, UiDiskPanel *parentwindow)
	: wxDropTarget()
{
	parent = parentwindow;
	frame = parentframe;

#ifdef USE_DATA_OBJECT_COMPOSITE
	wxDataObjectComposite* dataobj = new wxDataObjectComposite();

	dataobj->Add(new wxFileDataObject());
	SetDataObject(dataobj);
#else
	SetDataObject(new wxFileDataObject());
#endif
}

wxDragResult UiDiskPanelDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	if ( !GetData() ) return wxDragNone;
#ifdef USE_DATA_OBJECT_COMPOSITE
	bool sts = false;
	wxDataObjectComposite *comobj = (wxDataObjectComposite *)GetDataObject();
	if (comobj) {
		wxDataFormat fmt = comobj->GetReceivedFormat();
		if (fmt.GetType() == wxDF_FILENAME) {
			// エクスプローラからのDnD
			wxFileDataObject *dobj = (wxFileDataObject *)comobj->GetObject(fmt);
			sts = parent->ProcessDroppedFiles(x, y, dobj->GetFilenames());
		}
	}
#else
	bool sts = false;
	wxFileDataObject *dobj = (wxFileDataObject *)GetDataObject();
	if (dobj) {
		wxArrayString filenames;
#ifdef __WXOSX__
		size_t cnt = dobj->GetFilenames().Count();
		if (cnt == 1) {
			// ファイルパスが改行で区切られている可能性がある
			wxString str = dobj->GetFilenames().Item(0);
			filenames = wxSplit(str.Trim(), 0x0d);
		} else {
			filenames = dobj->GetFilenames();
		}
#else
		filenames = dobj->GetFilenames();
#endif
		sts = parent->ProcessDroppedFiles(x, y, filenames);
	}
#endif
	return (sts ? def : wxDragError);
}

//////////////////////////////////////////////////////////////////////
//
// File Open Dialog
//
UiDiskOpenFileDialog::UiDiskOpenFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard, long style)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wildcard, style | wxFD_OPEN)
{
}

//////////////////////////////////////////////////////////////////////
//
// File Save Dialog
//
UiDiskSaveFileDialog::UiDiskSaveFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT)
{
#if defined(__WXOSX__)
	wxSystemOptions::SetOption(wxOSX_FILEDIALOG_ALWAYS_SHOW_TYPES, 1);
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Dir Dialog
//
UiDiskDirDialog::UiDiskDirDialog(const wxString& message, const wxString& defaultDir, long style)
            : wxDirDialog(NULL, message, defaultDir, style)
{
}

//////////////////////////////////////////////////////////////////////
//
// About dialog
//
UiDiskAbout::UiDiskAbout(wxWindow* parent, wxWindowID id)
	: wxDialog(parent, id, _("About..."), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrLeft   = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrRight  = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrMain   = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *szrAll    = new wxBoxSizer(wxVERTICAL);

	szrLeft->Add(new wxStaticBitmap(this, wxID_ANY,
		wxBitmap(APPLICATION_XPMICON_NAME), wxDefaultPosition, wxSize(64, 64))
		, flags);

	wxString str = _T(APPLICATION_FULLNAME);
	str += _T(", Version ");
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
