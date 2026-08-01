/// @file main.cpp
///
/// @brief 本体
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "main.h"
#include <wx/cmdline.h>
#include <wx/dir.h>
#include "ui/uimainframe.h"
#include "diskimg/diskparam.h"
#include "diskimg/fileparam.h"
#include "basicfmt/basictemplate.h"
#include "diskimg/diskimage.h"
#include "logging.h"
#include "version.h"
// icon
#include "res/l3diskex.xpm"

#define MOD_COUNT_MAX 20

//////////////////////////////////////////////////////////////////////

wxIMPLEMENT_APP(UiDiskApp);

wxBEGIN_EVENT_TABLE(UiDiskApp, wxApp)
#ifdef CAPTURE_MOD_KEY_ON_APP
	EVT_IDLE(UiDiskApp::OnAppIdle)
#endif /* CAPTURE_MOD_KEY_ON_APP */
wxEND_EVENT_TABLE()

UiDiskApp::UiDiskApp()
{
	frame = NULL;
#ifdef CAPTURE_MOD_KEY_ON_APP
	mod_keys = 0;
	mod_cnt = 0;
#endif /* CAPTURE_MOD_KEY_ON_APP */
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
	wxArrayString errmsgs;
	if (!gDiskTemplates.Load(res_path + wxT("data/"), locale_name, errmsgs)) {
		errmsgs.Add(_("Cannot load disk types data file."));
	}
	if (!gDiskBasicTemplates.Load(res_path + wxT("data/"), locale_name, errmsgs)) {
		errmsgs.Add(_("Cannot load disk basic types data file."));
	}
	if (!CharCodes::Load(res_path + wxT("data/"), locale_name, errmsgs)) {
		errmsgs.Add(_("Cannot load char codes data file."));
	}
	if (!gFileTypes.Load(res_path + wxT("data/"), locale_name, errmsgs)) {
		errmsgs.Add(_("Cannot load file types data file."));
	}
	if (errmsgs.Count() > 0) {
		myLog.SetMessage(MyLogging::MyLog_Error, errmsgs);
		ResultInfo::ShowMessage(-1, errmsgs);
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
#ifdef __WXOSX__
		// Xcodeのオプションは無視する
		{
			wxCMD_LINE_OPTION, "NSDocumentRevisionsDebugMode", NULL,
			NULL,
			wxCMD_LINE_VAL_STRING,
			wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_HIDDEN
		},
#endif
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
