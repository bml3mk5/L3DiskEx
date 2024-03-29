/// @file main.h
///
/// @brief 本体
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef L3DISKEX_MAIN_H
#define L3DISKEX_MAIN_H

#include "common.h"
#include <wx/app.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/dnd.h>
#include <wx/fontdlg.h>
#include <wx/splitter.h>
#include "ui/uicommon.h"
#include "config.h"
#include "utils.h"

// icon
extern const char * l3diskex_xpm[];

class MyMenu;

class UiDiskApp;
class UiDiskFrame;
class UiDiskPanel;
class UiDiskDiskAttr;
class UiDiskList;
class UiDiskFileList;
class UiDiskRPanel;
class UiDiskRBPanel;
class UiDiskOpenFileDialog;
class UiDiskSaveFileDialog;
class UiDiskDirDialog;
class UiDiskFileDropTarget;
#if 0
class UiDiskRawPanel;
class UiDiskBinDumpFrame;
class UiDiskFatAreaFrame;

class DiskImage;
class DiskImageFile;
class DiskImageDisk;
class DiskParam;
class DiskParamPtrs;
class DiskBasic;
class DiskBasics;
class DiskBasicGroups;
class DiskBasicDirItem;
class DiskBasicDirItems;
class DiskBasicParam;

class CharTypeBox;

class ConfigBox;
#endif

//////////////////////////////////////////////////////////////////////

/// アプリトップ
class UiDiskApp: public wxApp
{
private:
	wxString app_path;
	wxString ini_path;
	wxString res_path;
	wxLocale mLocale;

	UiDiskFrame *frame;
	wxString in_file;

#ifdef CAPTURE_MOD_KEY_ON_APP
	int		mod_keys;	///< 修飾キー押下を記憶
	int		mod_cnt;
#endif

	wxArrayString tmp_dirs;

	/// アプリケーションのパスを設定
	void	SetAppPath();
public:
	UiDiskApp();
	/// 初期処理
	bool	OnInit();
	/// コマンドラインの解析
	void	OnInitCmdLine(wxCmdLineParser &parser);
	/// コマンドラインの解析完了
	bool	OnCmdLineParsed(wxCmdLineParser &parser);
	/// 終了処理
	int		OnExit();
#ifdef CAPTURE_MOD_KEY_ON_APP
	/// アイドル時の処理
	void	OnAppIdle(wxIdleEvent& event);
	/// イベント強制取得
	int		FilterEvent(wxEvent& event);
	/// 修飾キー押下状態を返す
	int		GetModifiers() const { return mod_keys; }
	/// 修飾キー押下状態を設定
	void	SetModifiers(int val) { mod_keys = val; }
#endif
	/// ファイルを開く(Mac用)
	void	MacOpenFile(const wxString &fileName);
	/// ファイルを開く(Mac用)
	void	MacOpenFiles(const wxArrayString &fileNames);
	/// アプリケーションのパスを返す
	const wxString &GetAppPath();
	/// 設定ファイルのあるパスを返す
	const wxString &GetIniPath();
	/// リソースファイルのあるパスを返す
	const wxString &GetResPath();
	/// テンポラリディレクトリを作成する
	bool	MakeTempDir(wxString &tmp_dir_path);
	/// テンポラリディレクトリを削除する
	void	RemoveTempDir(const wxString &tmp_dir_path);
	/// テンポラリディレクトリを削除する
	void	RemoveTempDir(const wxString &tmp_dir_path, int depth);
	/// テンポラリディレクトリをすべて削除する
	void	RemoveTempDirs();

	wxDECLARE_EVENT_TABLE();
};

wxDECLARE_APP(UiDiskApp);

//////////////////////////////////////////////////////////////////////

/// 分割ウィンドウ
class UiDiskPanel : public wxSplitterWindow
{
private:
	UiDiskFrame *frame;

	UiDiskList *lpanel;
	UiDiskRPanel *rpanel;

public:
	UiDiskPanel(UiDiskFrame *parent);
	~UiDiskPanel();

	// event handlers

	UiDiskList *GetLPanel() { return lpanel; }
	UiDiskRPanel *GetRPanel() { return rpanel; }

	bool ProcessDroppedFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(UiDiskPanel);
};

//////////////////////////////////////////////////////////////////////

/// ドラッグ＆ドロップ
class UiDiskPanelDropTarget : public wxDropTarget
{
	UiDiskPanel *parent;
    UiDiskFrame *frame;

public:
    UiDiskPanelDropTarget(UiDiskFrame *parentframe, UiDiskPanel *parentwindow);

	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
};

//////////////////////////////////////////////////////////////////////

/// ファイルオープンダイアログ
class UiDiskOpenFileDialog: public wxFileDialog
{
public:
	UiDiskOpenFileDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, const wxString& defaultFile = wxEmptyString, const wxString& wildcard = wxFileSelectorDefaultWildcardStr, long style = 0);

};

//////////////////////////////////////////////////////////////////////

/// ファイルセーブダイアログ
class UiDiskSaveFileDialog: public wxFileDialog
{
public:
	UiDiskSaveFileDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, const wxString& defaultFile = wxEmptyString, const wxString& wildcard = wxFileSelectorDefaultWildcardStr);

};

//////////////////////////////////////////////////////////////////////

/// ディレクトリダイアログ
class UiDiskDirDialog: public wxDirDialog
{
public:
	UiDiskDirDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, long style = wxDD_DEFAULT_STYLE);

};

//////////////////////////////////////////////////////////////////////

/// About dialog
class UiDiskAbout : public wxDialog
{
public:
	UiDiskAbout(wxWindow* parent, wxWindowID id);
};

#endif /* L3DISKEX_MAIN_H */

