/// @file main.h
///
/// @brief 本体
///
/// @author Copyright (c) Sasaji. All rights reserved.
///
/** @mainpage L3 Disk Explorer

    @section notes NOTES

    This application needs library to build:
    wxWidgets version 3.0 or later.
*/
#ifndef _L3DISKEX_MAIN_H_
#define _L3DISKEX_MAIN_H_

/* #define USE_MENU_OPEN	*/

#include "common.h"
#include <wx/app.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/dirdlg.h>
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/dnd.h>
#include <wx/fontdlg.h>
#include <wx/splitter.h>
#include <wx/ffile.h>
#include "diskd88.h"
#include "config.h"


class L3DiskApp;
class L3DiskFrame;
class L3DiskPanel;
class L3DiskDiskAttr;
class L3DiskList;
class L3DiskFileList;
class L3DiskRPanel;
class L3DiskRBPanel;
class L3DiskFileDialog;
class L3DiskDirDialog;
class L3DiskFileDropTarget;
class L3DiskRawPanel;
class L3DiskBinDumpFrame;
class L3DiskFatAreaFrame;

class DiskBasic;
class DiskBasics;
class DiskBasicGroups;
class DiskBasicDirItem;
class DiskBasicDirItems;

class CharTypeBox;

class ConfigBox;

//////////////////////////////////////////////////////////////////////

/// アプリトップ
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

#ifdef CAPTURE_MOD_KEY_ON_APP
	int		mod_keys;	///< 修飾キー押下を記憶
	int		mod_cnt;
#endif

	wxArrayString tmp_dirs;

	/// アプリケーションのパスを設定
	void	SetAppPath();
public:
	L3DiskApp();
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
	/// アプリケーションのパスを返す
	const wxString &GetAppPath();
	/// 設定ファイルのあるパスを返す
	const wxString &GetIniPath();
	/// リソースファイルのあるパスを返す
	const wxString &GetResPath();
	/// 設定データへのポインタを返す
	Config *GetConfig();
	/// テンポラリディレクトリを作成する
	bool	MakeTempDir(wxString &tmp_dir_path);
	/// テンポラリディレクトリを削除する
	void	RemoveTempDir(const wxString &tmp_dir_path);
	/// テンポラリディレクトリをすべて削除する
	void	RemoveTempDirs();

	wxDECLARE_EVENT_TABLE();
};

wxDECLARE_APP(L3DiskApp);

//////////////////////////////////////////////////////////////////////

/// メインFrame
class L3DiskFrame: public wxFrame
{
private:
	// gui
	wxMenu *menuFile;
	wxMenu *menuRecentFiles;
	wxMenu *menuDisk;
	wxMenu *menuMode;
	wxMenu *menuWindow;
	wxMenu *menuHelp;

	L3DiskPanel *panel;
	L3DiskBinDumpFrame *bindump_frame;
	L3DiskFatAreaFrame *fatarea_frame;

	Config *ini;

	/// DISKイメージ
	DiskD88 d88;

	int unique_number;

	/// パネル全体を返す
	L3DiskPanel *GetPanel() { return panel; }

#ifdef USE_MENU_OPEN
	void UpdateMenuFile();
	void UpdateMenuDisk();
#endif

	/// ツールバーの再生成
	void RecreateToolbar();
	/// ツールバーの構築
	void PopulateToolbar(wxToolBar* toolBar);

public:

    L3DiskFrame(const wxString& title, const wxSize& size);
	~L3DiskFrame();

	/// フレーム部の初期処理
	bool Init(const wxString &in_file);

	/// @name event procedures
	//@{
	/// ウィンドウを閉じたとき
	void OnClose(wxCloseEvent& event);

	/// メニュー 終了選択
	void OnQuit(wxCommandEvent& event);
	/// メニュー Aboutダイアログ表示選択
	void OnAbout(wxCommandEvent& event);

	/// メニュー 新規作成選択
	void OnCreateFile(wxCommandEvent& event);
	/// メニュー 開く選択
	void OnOpenFile(wxCommandEvent& event);
	/// メニュー 最近使用したファイル開く選択
	void OnOpenRecentFile(wxCommandEvent& event);
	/// メニュー 閉じる選択
	void OnCloseFile(wxCommandEvent& event);

	/// メニュー 名前を付けて保存選択
	void OnSaveAsFile(wxCommandEvent& event);
	/// メニュー ディスク1枚を保存選択
	void OnSaveDisk(wxCommandEvent& event);

	/// メニュー ディスクを新規に追加選択
	void OnAddNewDisk(wxCommandEvent& event);
	/// メニュー ディスクをファイルから追加選択
	void OnAddDiskFromFile(wxCommandEvent& event);

	/// メニュー ディスクを置換選択
	void OnReplaceDisk(wxCommandEvent& event);

	/// メニュー ファイルからディスクを削除選択
	void OnDeleteDiskFromFile(wxCommandEvent& event);
	/// メニュー ディスク名を変更選択
	void OnRenameDisk(wxCommandEvent& event);

	/// メニュー エクスポート選択
	void OnExportFileFromDisk(wxCommandEvent& event);
	/// メニュー インポート選択
	void OnImportFileToDisk(wxCommandEvent& event);
	/// メニュー 削除選択
	void OnDeleteFileFromDisk(wxCommandEvent& event);
	/// メニュー リネーム選択
	void OnRenameFileOnDisk(wxCommandEvent& event);
	/// メニュー 初期化選択
	void OnInitializeDisk(wxCommandEvent& event);
	/// メニュー フォーマット選択
	void OnFormatDisk(wxCommandEvent& event);
	/// メニュー プロパティ選択
	void OnPropertyOnDisk(wxCommandEvent& event);

	/// ファイルモード選択
	void OnBasicMode(wxCommandEvent& event);
	/// Rawディスクモード選択
	void OnRawDiskMode(wxCommandEvent& event);
	/// キャラクターコード選択
	void OnChangeCharCode(wxCommandEvent& event);
	/// 未使用データを切り落とすか
	void OnTrimData(wxCommandEvent& event);
	/// 削除ファイルを表示するか
	void OnShowDeletedFile(wxCommandEvent& event);

	/// ダンプウィンドウ選択
	void OnOpenBinDump(wxCommandEvent& event);
	/// 使用状況ウィンドウ選択
	void OnOpenFatArea(wxCommandEvent& event);
	/// フォント変更選択
	void OnChangeFont(wxCommandEvent& event);

	/// メニュー 設定ダイアログ選択
	void OnConfigure(wxCommandEvent& event);

#ifdef USE_MENU_OPEN
	void OnMenuOpen(wxMenuEvent& event);
#endif
	//@}

	/// @name ウィンドウ操作
	//@{
#ifndef USE_MENU_OPEN
	/// メニューのファイル項目を更新
	void UpdateMenuFile();
	/// メニューのディスク項目を更新
	void UpdateMenuDisk();
	/// メニューのディスク項目を更新
	void UpdateMenuDiskList(L3DiskList *list);
	/// メニューのファイル項目を更新
	void UpdateMenuFileList(L3DiskFileList *list);
	/// メニューの生ディスク項目を更新
	void UpdateMenuRawDisk(L3DiskRawPanel *rawpanel);
	/// メニューのモード項目を更新
	void UpdateMenuMode();
#endif
	/// 最近使用したファイル一覧を更新
	void UpdateMenuRecentFiles();

	/// ツールバーを更新
	void UpdateToolBar();

	/// ツールバーのディスクリスト項目を更新
	void UpdateToolBarDiskList(L3DiskList *list);
	/// メニューとツールバーのディスクリスト項目を更新
	void UpdateMenuAndToolBarDiskList(L3DiskList *list);
	/// ツールバーのファイルリスト項目を更新
	void UpdateToolBarFileList(L3DiskFileList *list);
	/// ツールバーの生ディスク項目を更新
	void UpdateToolBarRawDisk(L3DiskRawPanel *rawpanel);

	/// メニューとツールバーのファイルリスト項目を更新
	void UpdateMenuAndToolBarFileList(L3DiskFileList *list);
	/// メニューとツールバーの生ディスク項目を更新
	void UpdateMenuAndToolBarRawDisk(L3DiskRawPanel *rawpanel);

	/// ウィンドウ上のデータを更新
	void UpdateDataOnWindow(bool keep);
	/// ウィンドウ上のデータを更新 タイトルバーにファイルパスを表示
	void UpdateDataOnWindow(const wxString &path, bool keep);

	/// 保存後のウィンドウ上のデータを更新
	void UpdateSavedDataOnWindow(const wxString &path);
	/// ウィンドウ上のファイルパスを更新
	void UpdateFilePathOnWindow(const wxString &path);

	/// キャラクターコード選択
	void ChangeCharCode(int sel);
	/// キャラクターコード番号を返す
	int  GetCharCode() const;
	// キャラクターコード番号設定
	void SetDefaultCharCode();
	/// フォント変更ダイアログ
	void ShowListFontDialog();
	/// リストウィンドウのフォント変更
	void SetListFont(const wxFont &font);
	/// リストウィンドウのデフォルトフォントを得る
	void GetDefaultListFont(wxFont &font);

	/// 選択しているModeメニュー BASICかRAW DISKか
	int GetSelectedMode();

	/// 全パネルにデータをセットする（ディスク選択時）
	void SetDataOnDisk(DiskD88Disk *disk, int side_number, bool refresh_list);
	/// 全パネルのデータをクリアする
	void ClearAllData();
	/// 全パネルのデータをクリアしてRAW DISKパネルだけデータをセット
	void ClearAllAndSetRawData(DiskD88Disk *disk, int side_number);

	/// ウィンドウにタイトル名を設定
	wxString MakeTitleName(const wxString &path);
	/// ウィンドウにタイトル名を返す
	wxString GetFileName();
	//@}

	/// @name ディスク操作
	//@{
	/// 新規作成ダイアログ
	void ShowCreateFileDialog();
	/// 新規作成
	void CreateDataFile(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint);
	/// ディスク新規追加ダイアログ
	void ShowAddNewDiskDialog();
	/// ディスクを追加
	void AddNewDisk(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint);
	/// オープンダイアログ
	void ShowOpenFileDialog();
	/// 拡張子でファイル種別を判別する オープン時
	bool PreOpenDataFile(const wxString &path);
	/// 指定したディスクイメージをチェック
	int  CheckOpeningDataFile(const wxString &path, const wxString &ext, wxString &file_format, DiskParam &param_hint);
	/// 指定したディスクイメージを開く
	bool OpenDataFile(const wxString &path, const wxString &file_format, const DiskParam &param_hint);
	/// ファイル追加ダイアログ
	void ShowAddFileDialog();
	/// 拡張子でファイル種別を判別する 追加時
	bool PreAddDiskFile(const wxString &path);
	/// 指定したファイルを追加
	void AddDiskFile(const wxString &path, const wxString &file_format, const DiskParam &param_hint);
	/// ファイル種類選択ダイアログ
	bool ShowFileSelectDialog(const wxString &path, wxString &file_format);
	/// ディスク種類選択ダイアログ
	bool ShowParamSelectDialog(const wxString &path, const DiskParamPtrs &disk_params, const DiskParam *manual_param, DiskParam &param_hint);
	/// ファイルを閉じる
	bool CloseDataFile(bool force = false);
	/// 保存ダイアログ
	void ShowSaveFileDialog();
	/// 指定したファイルに保存
	void SaveDataFile(const wxString &path);
	/// ディスクをファイルに保存ダイアログ（指定ディスク）
	void ShowSaveDiskDialog(int disk_number, int side_number, bool each_sides);
	/// 指定したファイルに保存（指定ディスク）
	void SaveDataDisk(int disk_number, int side_number, const wxString &path);
	/// ディスクイメージ置換ダイアログ
	void ShowReplaceDiskDialog(int disk_number, int side_number, const wxString &subcaption);
	/// 拡張子でファイル種別を判別する 置換時
	bool PreReplaceDisk(int disk_number, int side_number, const wxString &path);
	/// 指定したディスクイメージ置換
	void ReplaceDisk(int disk_number, int side_number, const wxString &path, DiskD88Disk *src_disk, DiskD88Disk *tag_disk);
	/// ディスクをファイルから削除
	void DeleteDisk();
	/// ディスク名を変更
	void RenameDisk();
	/// ディスクパラメータを表示/変更
	void ShowDiskAttr();
	/// ディスクからファイルをエクスポート
	void ExportFileFromDisk();
	/// ディスクにファイルをインポート
	void ImportFileToDisk();
	/// ディスクからファイルを削除
	void DeleteFileFromDisk();
	/// ディスクのファイル名を変更
	void RenameFileOnDisk();
	/// ファイルのプロパティ
	void PropertyOnDisk();
	/// ディスクを初期化
	void InitializeDisk();
	/// ディスクをDISK BASIC用に論理フォーマット
	void FormatDisk();
	/// ドロップされたファイルを開く
	void OpenDroppedFile(const wxString &path);
	/// BASIC情報ダイアログ
	void ShowBasicAttr();

	/// ディレクトリをアサイン
	bool AssignDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dir_item);
	/// ディレクトリを移動
	bool ChangeDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dir_item, bool refresh_list);
	/// ディレクトリを削除
	bool DeleteDirectory(DiskD88Disk *disk, int side_num, DiskBasicDirItem *dir_item);
	//@}

	/// @name 左パネル 全般
	//@{
	/// 左パネルを返す
	L3DiskList *GetLPanel();
	//@}

	/// @name 左パネルのディスクツリー
	//@{
	/// 左パネルのディスクツリーを返す
	L3DiskList *GetDiskListPanel();
	/// 左パネルのディスクツリーにデータを設定する
	void SetDiskListData(const wxString &filename);
	/// 左パネルのディスクツリーをクリア
	void ClearDiskListData();
	/// 左パネルのディスクツリーのディスクを選択しているか
	bool IsDiskListSelectedDisk();
	/// 左パネルのディスクツリーの選択している位置
	void GetDiskListSelectedPos(int &disk_number, int &side_number);
	/// 左パネルのディスクツリーを選択
	void SetDiskListPos(int disk_number, int side_number);
	/// 左パネルのディスクツリーにファイルパスを設定
	void SetDiskListFilePath(const wxString &path);
	/// 左パネルのディスクツリーにディスク名を設定
	void SetDiskListName(const wxString &name);
	/// 選択しているディスクのルートを初期化＆再選択
	void RefreshDiskListOnSelectedDisk(const DiskBasicParam *newparam = NULL);
	/// 選択しているディスクのサイドを再選択
	void RefreshDiskListOnSelectedSide(const DiskBasicParam *newparam = NULL);
	/// 左パネルのディスクツリーを再選択
	void ReSelectDiskList(const DiskBasicParam *newparam = NULL);
	//@}

	/// @name 右パネル 全般
	//@{
	/// 右パネルを返す
	L3DiskRPanel *GetRPanel();
	/// 右パネルのデータウィンドウを変更 ファイルリスト/RAWディスク
	void ChangeRPanel(int num, const DiskBasicParam *param);
	/// 右パネルのすべてのコントロール内のデータをクリア
	void ClearRPanelData();
	//@}

	/// @name 右上パネルのディスク属性
	//@{
	/// 右上パネルのディスク属性パネルを返す
	L3DiskDiskAttr *GetDiskAttrPanel();
	/// 右上パネルのディスク属性にデータを設定する
	void SetDiskAttrData(DiskD88Disk *disk);
	/// 右上パネルのディスク属性をクリア
	void ClearDiskAttrData();
	//@}

	/// @name 右下パネルのファイルリスト
	//@{
	/// 右下パネルのファイルリストパネルを返す
	L3DiskFileList *GetFileListPanel(bool inst = false);
	/// 右下パネルのファイルリストにDISK BASICをアタッチ
	void AttachDiskBasicOnFileList(DiskD88Disk *disk, int side_num);
	/// 右下パネルのファイルリストからDISK BASICをデタッチ
	void DetachDiskBasicOnFileList();
	/// 右下パネルのファイルリストにデータを設定する
	void SetFileListData();
	/// 右下パネルのファイルリストをクリア
	void ClearFileListData();
	//@}

	/// @name ファイル名属性プロパティダイアログ
	//@{
	/// ファイル名属性プロパティダイアログをすべて閉じる
	void CloseAllFileAttr();
	//@}

	/// @name 右下パネルのRAWディスクパネル
	//@{
	/// 右下パネルのRAWディスクパネルを返す
	L3DiskRawPanel *GetDiskRawPanel(bool inst = false);
	/// 右下パネルのRAWディスクパネルにデータを設定する
	void SetRawPanelData(DiskD88Disk *disk, int side_num);
	/// 右下パネルのRAWディスクパネルをクリア
	void ClearRawPanelData();
	/// 右下パネルのRAWディスクパネルにデータを再設定する
	void RefreshRawPanelData();
	//@}

	/// @name ダンプウィンドウ
	//@{
	/// ダンプウィンドウを返す
	L3DiskBinDumpFrame *GetBinDumpFrame() const { return bindump_frame; }
	/// ダンプウィンドウにデータを設定する
	void SetBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len, int char_code, bool invert);
	/// ダンプウィンドウにデータを設定する
	void SetBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	/// ダンプウィンドウにデータを追記する
	void AppendBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len, int char_code, bool invert);
	/// ダンプウィンドウにデータを追記する
	void AppendBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	/// ダンプウィンドウをクリア
	void ClearBinDumpData();

	/// ダンプウィンドウを開く
	void OpenBinDumpWindow();
	/// ダンプウィンドウを閉じる
	void CloseBinDumpWindow();
	/// ダンプウィンドウを閉じる時にウィンドウ側から呼ばれるコールバック
	void BinDumpWindowClosed();
	//@}

	/// @name 使用状況ウィンドウ
	//@{
	/// 使用状況ウィンドウを返す
	L3DiskFatAreaFrame *GetFatAreaFrame() const { return fatarea_frame; }
	/// 使用状況ウィンドウにデータを設定する
	void SetFatAreaData();
	/// 使用状況ウィンドウにデータを設定する
	void SetFatAreaData(wxUint32 offset, const wxArrayInt *arr);
	/// 使用状況ウィンドウをクリア
	void ClearFatAreaData();
	/// 使用状況ウィンドウにフォーカスさせるグループ番号を設定する
	void SetFatAreaGroup(wxUint32 group_num);
	/// 使用状況ウィンドウにフォーカスさせるグループ番号を設定する
	void SetFatAreaGroup(const DiskBasicGroups *group_items, wxUint32 extra_group_num);
	/// 使用状況ウィンドウでフォーカスしているグループ番号をクリア
	void ClearFatAreaGroup();

	/// 使用状況ウィンドウを開く
	void OpenFatAreaWindow();
	/// 使用状況ウィンドウを閉じる
	void CloseFatAreaWindow();
	/// 使用状況ウィンドウを閉じる時にウィンドウ側から呼ばれるコールバック
	void FatAreaWindowClosed();
	//@}

	/// @name 設定ファイル
	//@{
	/// 設定ファイルを返す
	Config *GetConfig() { return ini; }
	/// 最近使用したパスを取得
	const wxString &GetIniRecentPath() const;
	/// 最近使用したパスを取得(エクスポート用)
	const wxString &GetIniExportFilePath() const;
	/// 最近使用したファイルを更新（一覧も更新）
	void SetIniRecentPath(const wxString &path);
	/// 最近使用したパスを更新
	void SetIniFilePath(const wxString &path);
	/// 最近使用したパスを更新(エクスポート用)
	void SetIniExportFilePath(const wxString &path);
	/// ダンプフォントを更新
	void SetIniDumpFont(const wxFont &font);
	/// ダンプフォント名を返す
	const wxString &GetIniDumpFontName() const;
	/// ダンプフォントサイズを返す
	int GetIniDumpFontSize() const;
	//@}

	/// @name property
	//@{
	/// ディスク操作用のインスタンス
	DiskD88 &GetDiskD88() { return d88; }
	/// ユニーク番号
	int GetUniqueNumber() const { return unique_number; }
	/// ユニーク番号を＋１
	void IncrementUniqueNumber() { unique_number++; }
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
		IDM_MAKE_DIRECTORY_ON_DISK,
		IDM_INITIALIZE_DISK,
		IDM_FORMAT_DISK,
		IDM_PROPERTY_DISK,

		IDM_BASIC_MODE,
		IDM_RAWDISK_MODE,
		IDM_CHAR_ASCII,
		IDM_CHAR_SJIS,
		IDM_TRIM_DATA,
		IDM_SHOW_DELFILE,

		IDM_WINDOW_BINDUMP,
		IDM_WINDOW_FATAREA,
		IDM_CHANGE_FONT,

		IDM_CONFIGURE,

		IDD_CONFIGBOX,
		IDD_CHARTYPEBOX,
		IDD_INTNAMEBOX,

		IDM_RECENT_FILE_0 = 80,
	};

	wxDECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////////

/// 分割ウィンドウ
class L3DiskPanel : public wxSplitterWindow
{
private:
	L3DiskFrame *frame;

	L3DiskList *lpanel;
	L3DiskRPanel *rpanel;

public:
	L3DiskPanel(L3DiskFrame *parent);
	~L3DiskPanel();

	// event handlers

	L3DiskList *GetLPanel() { return lpanel; }
	L3DiskRPanel *GetRPanel() { return rpanel; }

	bool ProcessDroppedFile(wxCoord x, wxCoord y, const wxString &filename);
	bool ProcessDroppedFile(wxCoord x, wxCoord y, const wxUint8 *buffer, size_t buflen);
	bool ProcessDroppedFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskPanel);
};

//////////////////////////////////////////////////////////////////////

// ドラッグアンドドロップ時のフォーマットID
extern wxDataFormat *L3DiskPanelDataFormat;

/// ドラッグ＆ドロップ
class L3DiskPanelDropTarget : public wxDropTarget
{
	L3DiskPanel *parent;
    L3DiskFrame *frame;

public:
    L3DiskPanelDropTarget(L3DiskFrame *parentframe, L3DiskPanel *parentwindow);

	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
};

//////////////////////////////////////////////////////////////////////

/// ファイルダイアログ
class L3DiskFileDialog: public wxFileDialog
{
public:
	L3DiskFileDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, const wxString& defaultFile = wxEmptyString, const wxString& wildcard = wxFileSelectorDefaultWildcardStr, long style = wxFD_DEFAULT_STYLE);

};

//////////////////////////////////////////////////////////////////////

/// ディレクトリダイアログ
class L3DiskDirDialog: public wxDirDialog
{
public:
	L3DiskDirDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, long style = wxDD_DEFAULT_STYLE);

};

//////////////////////////////////////////////////////////////////////

/// About dialog
class L3DiskAbout : public wxDialog
{
public:
	L3DiskAbout(wxWindow* parent, wxWindowID id);
};

#endif /* _L3DISKEX_MAIN_H_ */

