/// @file uimainframe.h
///
/// @brief メインフレーム
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef UIMAINFRAME_H
#define UIMAINFRAME_H

/* #define USE_MENU_OPEN	*/

#include "../common.h"
#include "uimainprocess.h"
#include <wx/string.h>
#include <wx/filename.h>
#include <wx/timer.h>
#include "uicommon.h"
#include "../config.h"
#include "../utils.h"


class MyMenu;

class UiDiskApp;
class UiDiskFrame;
class UiDiskPanel;
class UiDiskDiskAttr;
class UiDiskList;
class UiDiskFileList;
class UiDiskRPanel;
class UiDiskRBPanel;
class UiDiskFileDialog;
class UiDiskDirDialog;
class UiDiskFileDropTarget;
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


//////////////////////////////////////////////////////////////////////

/// @brief ステータスカウンター
///
/// ステータスバーなどの表示時間を管理
class StatusCounter
{
private:
	int m_current;
	int m_count;
	int m_using;
	wxString m_message;
public:
	StatusCounter();
	~StatusCounter();

	void Clear();
	void Start(int count, const wxString &message);
	void Append(int count);
	void Increase();
	void Finish(const wxString &message);
	int Current() const { return m_current; }
	int Count() const { return m_count; }
	bool IsIdle() const { return (m_using == 0); }
	bool IsFinished() const { return (m_using == 3); }
	wxString GetCurrentMessage() const;
};

/// @brief ステータスカウンター
///
/// ステータスバーなどの表示時間を管理
class StatusCounters
{
private:
	enum {
		StatusCountersMax = 3
	};
	StatusCounter	m_sc[StatusCountersMax];
	wxTimer			m_delay;
public:
	StatusCounters();
	~StatusCounters();
	StatusCounter &Item(int idx);

	void Clear();
	int  Start(int count, const wxString &message);
	void Append(int idx, int count);
	void Increase(int idx);
	void Finish(int idx, const wxString &message, wxEvtHandler *owner);
	int Current(int idx) const;
	int Count(int idx) const;
	wxString GetCurrentMessage(int idx) const;

	enum {
		IDT_STATUS_COUNTER = 555
	};
};

//////////////////////////////////////////////////////////////////////

/// メインFrame
class UiDiskFrame: public UiDiskProcess
{
private:
	// gui
	MyMenu *menuFile;
	MyMenu *menuRecentFiles;
	MyMenu *menuData;
	MyMenu *menuMode;
	MyMenu *menuView;
	MyMenu *menuHelp;

	UiDiskPanel *panel;
	UiDiskBinDumpFrame *bindump_frame;
	UiDiskFatAreaFrame *fatarea_frame;

	/// DISKイメージ
	DiskImage *p_image;

	StatusCounters stat_counters;

	Utils::StopWatch m_sw_export;	///< エクスポート時のストップウォッチ
	Utils::StopWatch m_sw_import;	///< インポート時のストップウォッチ

	/// パネル全体を返す
	UiDiskPanel *GetPanel() { return panel; }

	/// メニューの作成
	void MakeMenu();

#ifdef USE_MENU_OPEN
	void UpdateMenuFile();
	void UpdateMenuDisk();
#endif

	/// ツールバーの再生成
	void RecreateToolbar();
	/// ツールバーの構築
	void PopulateToolbar(wxToolBar* toolBar);

	/// ステータスバーの再生成
	void RecreateStatusbar();

public:

    UiDiskFrame(const wxString& title, const wxSize& size);
	~UiDiskFrame();

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

	/// メニュー 初期化選択
	void OnInitializeDisk(wxCommandEvent& event);
	/// メニュー フォーマット選択
	void OnFormatDisk(wxCommandEvent& event);

	/// メニュー エクスポート選択
	void OnExportDataFromDisk(wxCommandEvent& event);
	/// メニュー インポート選択
	void OnImportDataToDisk(wxCommandEvent& event);
	/// メニュー 削除選択
	void OnDeleteDataFromDisk(wxCommandEvent& event);
	/// メニュー リネーム選択
	void OnRenameDataOnDisk(wxCommandEvent& event);
	/// メニュー コピー選択
	void OnCopyDataFromDisk(wxCommandEvent& event);
	/// メニュー ペースト選択
	void OnPasteDataToDisk(wxCommandEvent& event);
	/// メニュー ディレクトリ作成選択
	void OnMakeDirectoryOnDisk(wxCommandEvent& event);
	/// メニュー ファイル編集選択
	void OnEditFileOnDisk(wxCommandEvent& event);
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
	/// ファイルリストの列選択
	void OnChangeColumnsOfFileList(wxCommandEvent& event);
	/// フォント変更選択
	void OnChangeFont(wxCommandEvent& event);

	/// メニュー 設定ダイアログ選択
	void OnConfigure(wxCommandEvent& event);

	/// ステータスカウンター終了タイマー
	void OnTimerStatusCounter(wxTimerEvent& event);

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
	void UpdateMenuDiskList(UiDiskList *list);
	/// メニューのファイル項目を更新
	void UpdateMenuFileList(UiDiskFileList *list);
	/// メニューの生ディスク項目を更新
	void UpdateMenuRawDisk(UiDiskRawPanel *rawpanel);
	/// メニューのモード項目を更新
	void UpdateMenuMode();
#endif
	/// 最近使用したファイル一覧を更新
	void UpdateMenuRecentFiles();

	/// ツールバーを更新
	void UpdateToolBar();

	/// ツールバーのディスクリスト項目を更新
	void UpdateToolBarDiskList(UiDiskList *list);
	/// メニューとツールバーのディスクリスト項目を更新
	void UpdateMenuAndToolBarDiskList(UiDiskList *list);
	/// ツールバーのファイルリスト項目を更新
	void UpdateToolBarFileList(UiDiskFileList *list);
	/// ツールバーの生ディスク項目を更新
	void UpdateToolBarRawDisk(UiDiskRawPanel *rawpanel);

	/// メニューとツールバーのファイルリスト項目を更新
	void UpdateMenuAndToolBarFileList(UiDiskFileList *list);
	/// メニューとツールバーの生ディスク項目を更新
	void UpdateMenuAndToolBarRawDisk(UiDiskRawPanel *rawpanel);

	/// ウィンドウ上のデータを更新
	void UpdateDataOnWindow(bool keep);
	/// ウィンドウ上のデータを更新 タイトルバーにファイルパスを表示
	void UpdateDataOnWindow(const wxString &path, bool keep);

	/// 保存後のウィンドウ上のデータを更新
	void UpdateSavedDataOnWindow(const wxString &path);
	/// ウィンドウ上のファイルパスを更新
	void UpdateFilePathOnWindow(const wxString &path);

	/// キャラクターコード選択
	void ChangeCharCode(const wxString &name);
	/// キャラクターコードを返す
	const wxString &GetCharCode() const;
	// キャラクターコード設定
	void SetDefaultCharCode();
	/// フォント変更ダイアログ
	void ShowListFontDialog();
	/// リストウィンドウのフォント変更
	void SetListFont(const wxFont &font);
	/// リストウィンドウのデフォルトフォントを得る
	void GetDefaultListFont(wxFont &font) const;

	/// ファイルリストの列を変更
	void ChangeColumnsOfFileList();

	/// 選択しているModeメニュー BASICかRAW DISKか
	int GetSelectedMode();

	/// 全パネルにデータをセットする（ディスク選択時）
	void SetDataOnDisk(DiskImageDisk *disk, int side_number, bool refresh_list);
	/// 全パネルのデータをクリアする
	void ClearAllData();
	/// 全パネルのデータをクリアしてRAW DISKパネルだけデータをセット
	void ClearAllAndSetRawData(DiskImageDisk *disk, int side_number);

	/// ウィンドウにタイトル名を設定
	wxString MakeTitleName(const wxString &path);
	/// ウィンドウにタイトル名を返す
	wxString GetFileName();

	/// 指定したウィンドウからの位置を得る
	static void GetPositionFromBaseWindow(wxWindow *base, wxWindow *target, int &x, int &y);
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
	void SaveDataFile(const wxString &path, const wxString &file_format, const wxString &ext);
	/// ディスクをファイルに保存ダイアログ（指定ディスク）
	void ShowSaveDiskDialog(int disk_number, int side_number, bool each_sides);
	/// 指定したファイルに保存（指定ディスク）
	void SaveDataDisk(int disk_number, int side_number, const wxString &path, const wxString &file_format, const wxString &ext);
	/// ディスクイメージ置換ダイアログ
	void ShowReplaceDiskDialog(int disk_number, int side_number, const wxString &subcaption);
	/// 拡張子でファイル種別を判別する 置換時
	bool PreReplaceDisk(int disk_number, int side_number, const wxString &path);
	/// 指定したディスクイメージ置換
	void ReplaceDisk(int disk_number, int side_number, const wxString &path, DiskImageDisk *src_disk, int src_side_number, DiskImageDisk *tag_disk);
	/// ディスクをファイルから削除
	void DeleteDisk();
	/// ディスク名を変更
	void RenameDisk();
	/// ディスクパラメータを表示/変更
	void ShowDiskAttr();
	/// ディスクからデータをエクスポート
	void ExportDataFromDisk();
	/// ディスクにデータをインポート
	void ImportDataToDisk();
	/// ディスクからデータを削除
	void DeleteDataFromDisk();
	/// ディスクのデータファイル名を変更
	void RenameDataOnDisk();
	/// ディスクのデータをコピー
	void CopyDataFromDisk();
	/// ディスクにデータをペースト
	void PasteDataToDisk();
	/// ディスクにディレクトリを作成
	void MakeDirectoryOnDisk();
	/// ファイル編集
	void EditFileOnDisk(enEditorTypes editor_type);
	/// ファイルのプロパティ
	void PropertyOnDisk();
	/// ディスクを初期化
	void InitializeDisk();
	/// ディスクをDISK BASIC用に論理フォーマット
	void FormatDisk();
	/// DISK BASIC用に論理フォーマットできるか
	bool IsFormattableDisk();
	/// ドロップされたファイルを開く
	void OpenDroppedFile(const wxString &path);
	/// BASIC情報ダイアログ
	void ShowBasicAttr();
	/// DISK BASICが使用できるか
	bool CanUseBasicDisk();

	/// エクスポート用の一時フォルダを作成
	bool CreateTemporaryFolder(wxString &tmp_dir_name, wxString &tmp_data_path, wxString &tmp_attr_path) const;
	//@}

	/// @name 左パネル 全般
	//@{
	/// 左パネルを返す
	UiDiskList *GetLPanel();
	//@}

	/// @name 左パネルのディスクツリー
	//@{
	/// 左パネルのディスクツリーを返す
	UiDiskList *GetDiskListPanel();
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
	/// 左パネルのディスクツリーにルートディレクトリを設定
	void RefreshRootDirectoryNodeOnDiskList(DiskImageDisk *disk, int side_number);
	/// 左パネルのディスクツリーにディレクトリを設定
	void RefreshDirectoryNodeOnDiskList(DiskImageDisk *disk, DiskBasicDirItem *dir_item);
	/// 左パネルの全てのディレクトリツリーを更新
	void RefreshAllDirectoryNodesOnDiskList(DiskImageDisk *disk, int side_number, DiskBasicDirItem *dir_item);
	/// 左パネルのディスクツリーのディレクトリを選択
	void SelectDirectoryNodeOnDiskList(DiskImageDisk *disk, DiskBasicDirItem *dir_item);
	/// 左パネルのディスクツリーのディレクトリノードを削除
	void DeleteDirectoryNodeOnDiskList(DiskImageDisk *disk, DiskBasicDirItem *dir_item);
	/// 左パネルのディスクツリーのディレクトリを一括削除
	void DeleteDirectoryNodesOnDiskList(DiskImageDisk *disk, DiskBasicDirItems &dir_items);
	/// 左パネルのディスクツリーのディレクトリ名を再設定
	void RefreshDiskListDirectoryName(DiskImageDisk *disk);
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
	UiDiskRPanel *GetRPanel();
	/// 右パネルのデータウィンドウを変更 ファイルリスト/RAWディスク
	void ChangeRPanel(int num, const DiskBasicParam *param);
	/// 右パネルのすべてのコントロール内のデータをクリア
	void ClearRPanelData();
	//@}

	/// @name 右上パネルのディスク属性
	//@{
	/// 右上パネルのディスク属性パネルを返す
	UiDiskDiskAttr *GetDiskAttrPanel();
	/// 右上パネルのディスク属性にデータを設定する
	void SetDiskAttrData(DiskImageDisk *disk);
	/// 右上パネルのディスク属性をクリア
	void ClearDiskAttrData();
	//@}

	/// @name 右下パネルのファイルリスト
	//@{
	/// 右下パネルのファイルリストパネルを返す
	UiDiskFileList *GetFileListPanel(bool inst = false);
	/// 右下パネルのファイルリストにDISK BASICをアタッチ
	void AttachDiskBasicOnFileList(DiskImageDisk *disk, int side_num);
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
	UiDiskRawPanel *GetDiskRawPanel(bool inst = false);
	/// 右下パネルのRAWディスクパネルにデータを設定する
	void SetRawPanelData(DiskImageDisk *disk, int side_num);
	/// 右下パネルのRAWディスクパネルをクリア
	void ClearRawPanelData();
	/// 右下パネルのRAWディスクパネルにデータを再設定する
	void RefreshRawPanelData();
	//@}

	/// @name ダンプウィンドウ
	//@{
	/// ダンプウィンドウを返す
	UiDiskBinDumpFrame *GetBinDumpFrame() const { return bindump_frame; }
	/// ダンプウィンドウにデータを設定する
	void SetBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len, const wxString &char_code, bool invert);
	/// ダンプウィンドウにデータを設定する
	void SetBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	/// ダンプウィンドウにデータを追記する
	void AppendBinDumpData(int trk, int sid, int sec, const wxUint8 *buf, size_t len, const wxString &char_code, bool invert);
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
	UiDiskFatAreaFrame *GetFatAreaFrame() const { return fatarea_frame; }
	/// 使用状況ウィンドウにデータを設定する
	void SetFatAreaData();
	/// 使用状況ウィンドウにデータを設定する
	void SetFatAreaData(wxUint32 offset, const wxArrayInt *arr);
	/// 使用状況ウィンドウをクリア
	void ClearFatAreaData();
	/// 使用状況ウィンドウにフォーカスさせるグループ番号を設定する
	void SetFatAreaGroup(wxUint32 group_num);
	/// 使用状況ウィンドウにフォーカスさせるグループ番号を設定する
	void SetFatAreaGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums);
	/// 使用状況ウィンドウにフォーカスをはずすグループ番号を設定する
	void UnsetFatAreaGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums);
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
	/// 最近使用したパスを取得
	const wxString &GetIniRecentPath() const;
	/// 最近使用したパスを取得(エクスポート用)
	const wxString &GetIniExportFilePath() const;
	/// 最近使用したファイルを更新（一覧も更新）
	void SetIniRecentPath(const wxString &path);
	/// 最近使用したパスを更新
	void SetIniFilePath(const wxString &path);
	/// 最近使用したパスを更新(エクスポート用)
	void SetIniExportFilePath(const wxString &path, bool is_dir = false);
	/// ダンプフォントを更新
	void SetIniDumpFont(const wxFont &font);
	/// ダンプフォント名を返す
	const wxString &GetIniDumpFontName() const;
	/// ダンプフォントサイズを返す
	int GetIniDumpFontSize() const;

	/// 設定ダイアログ表示
	void ShowConfigureDialog();
	//@}

	/// @name ステータスカウンター
	//@{
	int  StartStatusCounter(int count, const wxString &message);
	void AppendStatusCounter(int idx, int count);
	void IncreaseStatusCounter(int idx);
	void FinishStatusCounter(int idx, const wxString &message);
	void ClearStatusCounter();

	void StartExportCounter(int count, const wxString &message);
	void AppendExportCounter(int count);
	void IncreaseExportCounter();
	void BeginBusyCursorExportCounterIfNeed();
	void FinishExportCounter(const wxString &message);

	void StartImportCounter(int count, const wxString &message);
	void AppendImportCounter(int count);
	void IncreaseImportCounter();
	void BeginBusyCursorImportCounterIfNeed();
	void FinishImportCounter(const wxString &message);
	void RestartImportCounter();
	//@}

	/// @name その他のUI
	//@{
	/// @brief 指定ファイルを引数にして外部エディタを起動する
	static bool OpenFileWithEditor(enEditorTypes editor_type, const wxFileName &file);
	//@}

	/// @name property
	//@{
	/// ディスク操作用のインスタンス
	DiskImage &GetDiskImage() { return *p_image; }
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

		IDM_INITIALIZE_DISK,
		IDM_FORMAT_DISK,

		IDM_EXPORT_DATA,
		IDM_IMPORT_DATA,
		IDM_DELETE_DATA,
		IDM_RENAME_DATA_ON_DISK,
		IDM_DUPLICATE_DATA,
		IDM_COPY_DATA,
		IDM_PASTE_DATA,
		IDM_MAKE_DIRECTORY_ON_DISK,
		IDM_EDIT_FILE_BINARY,
		IDM_EDIT_FILE_TEXT,
		IDM_PROPERTY_DATA,

		IDM_BASIC_MODE,
		IDM_RAWDISK_MODE,
		IDM_TRIM_DATA,
		IDM_SHOW_DELFILE,

		IDM_WINDOW_BINDUMP,
		IDM_WINDOW_FATAREA,
		IDM_FILELIST_COLUMN,
		IDM_CHANGE_FONT,

		IDM_CONFIGURE,

		IDD_CONFIGBOX,
		IDD_CHARTYPEBOX,
		IDD_INTNAMEBOX,

		IDM_CHAR_0 = 50,

		IDM_RECENT_FILE_0 = 80,
	};

	wxDECLARE_EVENT_TABLE();
};

#endif /* UIMAINFRAME_H */

