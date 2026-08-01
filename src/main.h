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
#include <wx/string.h>
#include <wx/dynarray.h>
#include "config.h"
#include "utils.h"

class UiDiskFrame;

// icon
extern const char * l3diskex_xpm[];

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
#endif /* CAPTURE_MOD_KEY_ON_APP */

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
#endif /* CAPTURE_MOD_KEY_ON_APP */
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

#endif /* L3DISKEX_MAIN_H */
