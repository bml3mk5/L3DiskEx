/// @file uirawdisk.h
///
/// @brief ディスクID一覧
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UIRAWDISK_H_
#define _UIRAWDISK_H_

#include "uicommon.h"
#include <wx/string.h>
#include <wx/splitter.h>
#include "../diskd88.h"


#ifndef USE_LIST_CTRL_ON_TRACK_LIST
#include "uicdlistctrl.h"
#else
#include "uiclistctrl.h"
#endif
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
#include "uicdlistctrl.h"
#else
#include "uiclistctrl.h"
#endif

class wxFileDataObject;
class L3DiskFrame;
class L3DiskRawTrack;
class L3DiskRawSector;

//////////////////////////////////////////////////////////////////////

enum en_track_list_columns {
	TRACKCOL_NUM = 0,
	TRACKCOL_TRACK,
	TRACKCOL_SIDE,
	TRACKCOL_SECS,
	TRACKCOL_OFFSET,
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
	TRACKCOL_DUMMY,
#endif
	TRACKCOL_END
};

extern const struct st_list_columns gL3DiskRawTrackColumnDefs[];

enum en_sector_list_columns {
	SECTORCOL_NUM = 0,
	SECTORCOL_ID_C,
	SECTORCOL_ID_H,
	SECTORCOL_ID_R,
	SECTORCOL_ID_N,
	SECTORCOL_DELETED,
	SECTORCOL_SINGLE,
	SECTORCOL_SECTORS,
	SECTORCOL_SIZE,
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
	SECTORCOL_DUMMY,
#endif
	SECTORCOL_END
};

extern const struct st_list_columns gL3DiskRawSectorColumnDefs[];

//////////////////////////////////////////////////////////////////////

/// 分割ウィンドウ
class L3DiskRawPanel : public wxSplitterWindow
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	L3DiskRawTrack *lpanel;
	L3DiskRawSector *rpanel;

	bool invert_data;	///< インポート・エクスポート時にデータを反転するか
	bool reverse_side;	///< インポート・エクスポート時にサイド番号を降順で行うか

public:
	L3DiskRawPanel(L3DiskFrame *parentframe, wxWindow *parentwindow);

	L3DiskRawTrack *GetLPanel() { return lpanel; }
	L3DiskRawSector *GetRPanel() { return rpanel; }

	/// トラックリストにデータを設定する
	void SetTrackListData(DiskD88Disk *disk, int side_num);
	/// トラックリストをクリアする
	void ClearTrackListData();
	/// トラックリストを再描画する
	void RefreshTrackListData();
	/// トラックリストが存在するか
	bool TrackListExists() const;
	/// トラックリストの選択行を返す
	int GetTrackListSelectedRow() const;

	/// セクタリストにデータを設定する
	void SetSectorListData(DiskD88Track *track);
	/// セクタリストをクリアする
	void ClearSectorListData();
	/// セクタリストの選択行を返す
	int GetSectorListSelectedRow() const;

	/// トラックリストとセクタリストを更新
	void RefreshAllData();

	/// クリップボードヘコピー
	bool CopyToClipboard();
	/// クリップボードからペースト
	bool PasteFromClipboard();

	/// エクスポートダイアログ表示
	bool ShowExportDataDialog();
	/// インポートダイアログ表示
	bool ShowImportDataDialog();
	/// データを削除ダイアログ表示
	bool ShowDeleteDataDialog();

	/// トラックへインポートダイアログ（トラックの範囲指定）表示
	bool ShowImportTrackRangeDialog(const wxString &path, int st_trk = -1, int st_sid = 0, int st_sec = 1);

	/// セクタからエクスポートダイアログ表示
	bool ShowExportDataFileDialog();
	/// セクタへインポートダイアログ表示
	bool ShowImportDataFileDialog();
	/// トラックのID一括変更
	void ModifyIDonTrack(int type_num);
	/// トラックの密度一括変更
	void ModifyDensityOnTrack();
	/// トラックのセクタ数一括変更
	void ModifySectorsOnTrack();
	/// トラックのセクタサイズ一括変更
	void ModifySectorSizeOnTrack();

	/// トラック or セクタのプロパティダイアログ表示
	bool ShowRawDiskAttr();
	/// セクタのプロパティダイアログ表示
	bool ShowSectorAttr();

	/// ファイル名
	wxString MakeFileName(DiskD88Sector *sector);
	/// ファイル名
	wxString MakeFileName(int st_c, int st_h, int st_r, int ed_c, int ed_h, int ed_r);

	/// フォントをセット
	void SetListFont(const wxFont &font);

	/// インポート・エクスポート時にデータを反転するか
	bool InvertData() const { return invert_data; }
	/// インポート・エクスポート時にデータを反転するか
	void InvertData(bool val) { invert_data = val; }
	/// インポート・エクスポート時にサイド番号を降順で行うか
	bool ReverseSide() const { return reverse_side; }
	/// インポート・エクスポート時にサイド番号を降順で行うか
	void ReverseSide(bool val) { reverse_side = val; }

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRawPanel);
};

//////////////////////////////////////////////////////////////////////

#ifndef USE_LIST_CTRL_ON_TRACK_LIST

class L3DiskRawTrack;

/// トラックリストの挙動を設定
class L3DiskRawTrackListStoreModel : public wxDataViewListStore
{
private:
	L3DiskRawTrack *ctrl;

public:
	L3DiskRawTrackListStoreModel(wxWindow *parent);

	void SetControl(L3DiskRawTrack *n_ctrl) { ctrl = n_ctrl; }
	int  Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const;
};

#define L3RawTrackListColumn	wxDataViewColumn*
#define L3RawTrackListItem		wxDataViewItem
#define L3RawTrackListItems		wxDataViewItemArray
#define L3RawTrackListEvent		wxDataViewEvent
#define L3RawTrackListValue		L3CDListValue
#else
#define L3RawTrackListColumn	long
#define L3RawTrackListItem		long
#define L3RawTrackListItems		wxArrayLong
#define L3RawTrackListEvent		wxListEvent
#define L3RawTrackListValue		L3CListValue
#endif

//////////////////////////////////////////////////////////////////////

/// トラックリストコントロール
#ifndef USE_LIST_CTRL_ON_TRACK_LIST
class L3DiskRawTrackListCtrl : public L3CDListCtrl
#else
class L3DiskRawTrackListCtrl : public L3CListCtrl
#endif
{
public:
	L3DiskRawTrackListCtrl(L3DiskFrame *parentframe, wxWindow *parent, L3DiskRawTrack *sub, wxWindowID id);
	/// リストデータを設定
	void SetListData(DiskD88Disk *disk, int pos, int row, int idx, L3RawTrackListValue *values);
	/// リストにデータを挿入
	void InsertListData(DiskD88Disk *disk, int pos, int row, int idx);
	/// リストデータを更新
	void UpdateListData(DiskD88Disk *disk, int pos, int row, int idx);

#ifdef USE_LIST_CTRL_ON_TRACK_LIST
	/// アイテムをソート
	void SortDataItems(DiskD88Disk *disk, int side_number, int col);
	/// ソート用コールバック
	static int wxCALLBACK Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata);
#endif
	static int CompareOffset(DiskD88Disk *disk, int i1, int i2, int dir);
	static int CompareNum(DiskD88Disk *disk, int i1, int i2, int dir);
};

//////////////////////////////////////////////////////////////////////

/// 左パネルのトラックリスト
class L3DiskRawTrack : public L3DiskRawTrackListCtrl
{
private:
	L3DiskRawPanel *parent;
	L3DiskFrame *frame;

	DiskD88Disk *disk;
	int side_number;

	wxMenu *menuPopup;

	/// ファイルリスト作成（DnD, クリップボード用）
	bool CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object);
	/// ファイルリストを解放（DnD, クリップボード用）
	void ReleaseFileObject(const wxString &tmp_dir_name);

public:
	L3DiskRawTrack(L3DiskFrame *parentframe, L3DiskRawPanel *parentwindow);
	~L3DiskRawTrack();

	/// @name event procedures
	//@{
	/// トラックリストを選択
	void OnListItemSelected(L3RawTrackListEvent& event);
	/// トラックリストをダブルクリック
	void OnListActivated(L3RawTrackListEvent& event);
	/// トラックリスト右クリック
	void OnListContextMenu(L3RawTrackListEvent& event);
	/// トラックリスト右クリック
	void OnContextMenu(wxContextMenuEvent& event);
	/// カラムをクリック
	void OnColumnClick(L3RawTrackListEvent &event);
	/// トラックをエクスポート選択
	void OnExportTrack(wxCommandEvent& event);
	/// トラックにインポート選択
	void OnImportTrack(wxCommandEvent& event);
	/// データを反転するチェック選択
	void OnChangeInvertData(wxCommandEvent& event);
	/// サイドを逆転するチェック選択
	void OnChangeReverseSide(wxCommandEvent& event);
	/// トラックリストからドラッグ開始
	void OnBeginDrag(L3RawTrackListEvent& event);
	/// ディスク上のID一括変更選択
	void OnModifyIDonDisk(wxCommandEvent& event);
	/// トラックのID一括変更選択
	void OnModifyIDonTrack(wxCommandEvent& event);
	/// ディスク上の密度一括変更選択
	void OnModifyDensityOnDisk(wxCommandEvent& event);
	/// トラックの密度一括変更選択
	void OnModifyDensityOnTrack(wxCommandEvent& event);
	/// トラックのセクタ数を一括変更選択
	void OnModifySectorsOnTrack(wxCommandEvent& event);
	/// トラックのセクタサイズを一括変更
	void OnModifySectorSizeOnTrack(wxCommandEvent& event);
	/// 新規トラックを追加選択
	void OnAppendTrack(wxCommandEvent& event);
	/// 現在のトラック以下を削除選択
	void OnDeleteTracksBelow(wxCommandEvent& event);
	/// トラックプロパティ選択
	void OnPropertyTrack(wxCommandEvent& event);
	/// キー押下
	void OnChar(wxKeyEvent& event);
	//@}

	/// 選択
	void SelectData();
	/// トラックリストをセット
	void SetTracks(DiskD88Disk *newdisk, int newsidenum);
	/// トラックリストをセット
	void SetTracks();
	/// トラックリストを再セット
	void RefreshTracks();
	/// トラックリストをクリア
	void ClearTracks();

	/// トラックリスト上のポップアップメニュー表示
	void ShowPopupMenu();

	/// ファイルリストをドラッグ
	bool DragDataSourceForExternal();
	/// クリップボードへコピー
	bool CopyToClipboard();
	/// クリップボードからペースト
	bool PasteFromClipboard();

	/// トラックをエクスポート ダイアログ表示
	bool ShowExportTrackDialog();
	/// 指定したファイルにトラックデータをエクスポート
	bool ExportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec, bool inv_data, bool rev_side);
	/// トラックにインポート ダイアログ表示
	bool ShowImportTrackDialog();
	/// 指定したファイルのファイル名にある数値から指定したトラックにインポートする
	bool ShowImportTrackRangeDialog(const wxString &path, int st_trk = -1, int st_sid = 0, int st_sec = 1);
	/// 指定したファイルから指定した範囲にトラックデータをインポート
	bool ImportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec, bool inv_data, bool rev_side);
	/// ディスク全体のIDを変更
	void ModifyIDonDisk(int type_num);
	/// ディスク上の密度を一括変更
	void ModifyDensityOnDisk();
	/// トラック情報を表示
	void ShowTrackAttr();

	/// トラックを追加
	void AppendTrack();
	/// トラックを削除
	void DeleteTracks();

	/// ディスクを返す
	DiskD88Disk *GetDisk() const { return disk; }
	/// 選択行のトラックを返す
	DiskD88Track *GetSelectedTrack();
	/// 指定行のトラックを返す
	DiskD88Track *GetTrack(const L3RawTrackListItem &row);
	/// 最初のトラックを返す
	DiskD88Track *GetFirstTrack();
	/// トラックのセクタ１を得る
	bool GetFirstSectorOnTrack(DiskD88Track **track, DiskD88Sector **sector);
	/// トラックの開始セクタ番号と終了セクタ番号を得る
	bool GetFirstAndLastSectorNumOnTrack(const DiskD88Track *track, int &start_sector, int &end_sector);

	enum {
		IDM_MENU_CHECK = 1,
		IDM_INVERT_DATA,
		IDM_REVERSE_SIDE,
		IDM_EXPORT_TRACK,
		IDM_IMPORT_TRACK,
		IDM_MODIFY_ID_C_DISK,
		IDM_MODIFY_ID_H_DISK,
		IDM_MODIFY_ID_R_DISK,
		IDM_MODIFY_ID_N_DISK,
		IDM_MODIFY_DENSITY_DISK,
		IDM_MODIFY_ID_C_TRACK,
		IDM_MODIFY_ID_H_TRACK,
		IDM_MODIFY_ID_R_TRACK,
		IDM_MODIFY_ID_N_TRACK,
		IDM_MODIFY_DENSITY_TRACK,
		IDM_MODIFY_SECTORS_TRACK,
		IDM_MODIFY_SIZE_TRACK,
		IDM_APPEND_TRACK,
		IDM_DELETE_TRACKS_BELOW,
		IDM_PROPERTY_TRACK,
	};

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRawTrack);
};

//////////////////////////////////////////////////////////////////////

#ifndef USE_LIST_CTRL_ON_SECTOR_LIST

class L3DiskRawSector;

/// セクタリストの挙動を設定
class L3DiskRawSectorListStoreModel : public wxDataViewListStore
{
private:
	L3DiskRawSector *ctrl;
public:
	L3DiskRawSectorListStoreModel(wxWindow *parent);

	void SetControl(L3DiskRawSector *n_ctrl) { ctrl = n_ctrl; }

	bool IsEnabledByRow(unsigned int row, unsigned int col) const;

	int  Compare(const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int col, bool ascending) const;
};

#define L3RawSectorListColumn	wxDataViewColumn*
#define L3RawSectorListItem		wxDataViewItem
#define L3RawSectorListItems	wxDataViewItemArray
#define L3RawSectorListEvent	wxDataViewEvent
#define L3RawSectorListValue	L3CDListValue
#else
#define L3RawSectorListColumn	long
#define L3RawSectorListItem		long
#define L3RawSectorListItems	wxArrayLong
#define L3RawSectorListEvent	wxListEvent
#define L3RawSectorListValue	L3CListValue
#endif

//////////////////////////////////////////////////////////////////////

/// セクタリストコントロール
#ifndef USE_LIST_CTRL_ON_SECTOR_LIST
class L3DiskRawSectorListCtrl : public L3CDListCtrl
#else
class L3DiskRawSectorListCtrl : public L3CListCtrl
#endif
{
public:
	L3DiskRawSectorListCtrl(L3DiskFrame *parentframe, wxWindow *parent, L3DiskRawSector *sub, wxWindowID id);
	/// リストデータを設定
	void SetListData(DiskD88Sector *sector, int row, L3RawSectorListValue *values);
	/// リストにデータを挿入
	void InsertListData(DiskD88Sector *sector, int row, int idx);
	/// リストデータを更新
	void UpdateListData(DiskD88Sector *sector, int row, int idx);

#ifdef USE_LIST_CTRL_ON_SECTOR_LIST
	/// アイテムをソート
	void SortDataItems(DiskD88Track *track, int col);
	/// ソート用コールバック
	static int wxCALLBACK Compare(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortdata);
#endif
	static int CompareIDR(DiskD88Sectors *sectors, int i1, int i2, int dir);
	static int CompareNum(DiskD88Sectors *sectors, int i1, int i2, int dir);
};

//////////////////////////////////////////////////////////////////////

/// 右パネルのセクタリスト
class L3DiskRawSector : public L3DiskRawSectorListCtrl
{
private:
	L3DiskRawPanel *parent;
	L3DiskFrame *frame;

	wxMenu *menuPopup;

	DiskD88Track *track;

//	L3SectorListItem selected_item;

	bool initialized;

	/// ファイルリスト作成（DnD, クリップボード用）
	bool CreateFileObject(wxString &tmp_dir_name, wxFileDataObject &file_object);
	/// ファイルリストを解放（DnD, クリップボード用）
	void ReleaseFileObject(const wxString &tmp_dir_name);

public:
	L3DiskRawSector(L3DiskFrame *parentframe, L3DiskRawPanel *parent);
	~L3DiskRawSector();

	/// @name event procedures
	//@{
	/// セクタリスト右クリック
	void OnItemContextMenu(L3RawSectorListEvent& event);
	/// 右クリック
	void OnContextMenu(wxContextMenuEvent& event);
	/// セクタリスト ダブルクリック
	void OnItemActivated(L3RawSectorListEvent& event);
	/// セクタリスト カラムをクリック
	void OnColumnClick(L3RawSectorListEvent& event);
	/// セクタリスト選択
	void OnSelectionChanged(L3RawSectorListEvent& event);
	/// セクタリストからドラッグ開始
	void OnBeginDrag(L3RawSectorListEvent& event);
	/// セクタリスト エクスポート選択
	void OnExportFile(wxCommandEvent& event);
	/// セクタリスト インポート選択
	void OnImportFile(wxCommandEvent& event);
	/// データを反転するチェック選択
	void OnChangeInvertData(wxCommandEvent& event);
	/// トラック上のID一括変更選択
	void OnModifyIDonTrack(wxCommandEvent& event);
	/// トラック上の密度一括変更選択
	void OnModifyDensityOnTrack(wxCommandEvent& event);
	/// トラック上のセクタ数一括変更選択
	void OnModifySectorsOnTrack(wxCommandEvent& event);
	/// トラック上のセクタサイズ一括変更選択
	void OnModifySectorSizeOnTrack(wxCommandEvent& event);
	/// セクタ追加選択
	void OnAppendSector(wxCommandEvent& event);
	/// セクタ削除選択
	void OnDeleteSector(wxCommandEvent& event);
	/// トラック上のセクタ一括削除選択
	void OnDeleteSectorsOnTrack(wxCommandEvent& event);
	/// セクタ編集選択
	void OnEditSector(wxCommandEvent& event);
	/// セクタプロパティ選択
	void OnPropertySector(wxCommandEvent& event);
	/// セクタリスト上でキー押下
	void OnChar(wxKeyEvent& event);
	//@}

	/// ポップアップメニュー表示
	void ShowPopupMenu();

	// セクタリスト選択
	void SelectItem(DiskD88Sector *sector);
	// セクタリスト非選択
	void UnselectItem();

	/// セクタリストにデータをセット
	void SetSectors(DiskD88Track *newtrack);
	/// セクタリストを返す
	DiskD88Sectors *GetSectors() const;
	/// セクタリストをリフレッシュ
	void RefreshSectors();
	/// セクタリストをクリア
	void ClearSectors();
	/// 選択しているセクタを返す
	DiskD88Sector *GetSelectedSector(int *pos = NULL);
	/// セクタを返す
	DiskD88Sector *GetSector(const L3RawSectorListItem &item);

	/// ファイルリストをドラッグ
	bool DragDataSourceForExternal();
	/// クリップボードへコピー
	bool CopyToClipboard();
	/// クリップボードからペースト
	bool PasteFromClipboard();

	/// エクスポートダイアログ表示
	bool ShowExportDataFileDialog();
	/// 指定したファイルにセクタのデータをエクスポート
	bool ExportDataFile(const wxString &path, DiskD88Sector *sector);
	/// インポートダイアログ表示
	bool ShowImportDataFileDialog();
	/// セクタ情報プロパティダイアログ表示
	bool ShowSectorAttr();

	/// トラック上のIDを一括変更
	void ModifyIDonTrack(int type_num);
	/// トラック上の密度を一括変更
	void ModifyDensityOnTrack();
	/// トラック上のセクタ数を一括変更
	void ModifySectorsOnTrack();
	/// トラック上のセクタサイズを一括変更
	void ModifySectorSizeOnTrack();

	/// セクタを追加ダイアログを表示
	void ShowAppendSectorDialog();
	/// セクタを削除
	void DeleteSector();
	/// トラック上のセクタを一括削除
	void DeleteSectorsOnTrack();

	/// セクタを編集
	void EditSector();

	enum {
		IDM_MENU_CHECK = 1,
		IDM_INVERT_DATA,
		IDM_REVERSE_SIDE,
		IDM_EXPORT_FILE,
		IDM_IMPORT_FILE,
		IDM_MODIFY_ID_C_TRACK,
		IDM_MODIFY_ID_H_TRACK,
		IDM_MODIFY_ID_R_TRACK,
		IDM_MODIFY_ID_N_TRACK,
		IDM_MODIFY_DENSITY_TRACK,
		IDM_MODIFY_SECTORS_TRACK,
		IDM_MODIFY_SIZE_TRACK,
		IDM_APPEND_SECTOR,
		IDM_DELETE_SECTOR,
		IDM_DELETE_SECTORS_BELOW,
		IDM_EDIT_SECTOR,
		IDM_PROPERTY_SECTOR,
	};

	wxDECLARE_EVENT_TABLE();
};

#endif /* _UIRAWDISK_H_ */

