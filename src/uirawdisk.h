/// @file uirawdisk.h
///
/// @brief ディスクID一覧
///
#ifndef _UIRAWDISK_H_
#define _UIRAWDISK_H_

#include "common.h"
#include <wx/string.h>
#include <wx/splitter.h>
#include <wx/listctrl.h>
#include <wx/dataview.h>
#include <wx/clntdata.h>
#include "diskd88.h"

class L3DiskFrame;
class L3DiskRawTrack;
class L3DiskRawSector;

/// 分割ウィンドウ
class L3DiskRawPanel : public wxSplitterWindow
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	L3DiskRawTrack *lpanel;
	L3DiskRawSector *rpanel;

public:
	L3DiskRawPanel(L3DiskFrame *parentframe, wxWindow *parentwindow);

	L3DiskRawTrack *GetLPanel() { return lpanel; }
	L3DiskRawSector *GetRPanel() { return rpanel; }

	/// トラックリストにデータを設定する
	void SetTrackListData(DiskD88Disk *disk, int side_num);
	/// トラックリストをクリアする
	void ClearTrackListData();
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

	/// クリップボードからペースト
	bool PasteFromClipboard();
	/// トラックからエクスポートダイアログ表示
	bool ShowExportTrackDialog();
	/// トラックへインポートダイアログ表示
	bool ShowImportTrackDialog();
	/// トラックへインポートダイアログ（トラックの範囲指定）表示
	bool ShowImportTrackRangeDialog(const wxString &path, int st_trk = -1, int st_sid = 0, int st_sec = 1);

	/// セクタからエクスポートダイアログ表示
	bool ShowExportDataFileDialog();
	/// セクタへインポートダイアログ表示
	bool ShowImportDataFileDialog();
	/// セクタのプロパティダイアログ表示
	bool ShowSectorAttr();
	/// トラックのID一括変更
	void ModifyIDonTrack(int type_num);
	/// トラックの密度一括変更
	void ModifyDensityOnTrack();
	/// トラックのセクタ数一括変更
	void ModifySectorsOnTrack();
	/// トラックのセクタサイズ一括変更
	void ModifySectorSizeOnTrack();

	/// ファイル名
	wxString MakeFileName(DiskD88Sector *sector);
	/// ファイル名
	wxString MakeFileName(int st_c, int st_h, int st_r, int ed_c, int ed_h, int ed_r);

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRawPanel);
};

/// 左パネルのトラックリスト
class L3DiskRawTrack : public wxListView
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
	void OnListItemSelected(wxListEvent& event);
	/// トラックリスト右クリック
	void OnContextMenu(wxContextMenuEvent& event);
	/// トラックをエクスポート選択
	void OnExportTrack(wxCommandEvent& event);
	/// トラックにインポート選択
	void OnImportTrack(wxCommandEvent& event);
	/// トラックリストからドラッグ開始
	void OnBeginDrag(wxListEvent& event);
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
	/// 現在のトラック以下を削除選択
	void OnDeleteTracksBelow(wxCommandEvent& event);
	/// キー押下
	void OnChar(wxKeyEvent& event);
	//@}

	/// トラックリストをセット
	void SetData(DiskD88Disk *newdisk, int newsidenum);
	/// トラックリストをクリア
	void ClearData();

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
	bool ExportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec);
	/// トラックにインポート ダイアログ表示
	bool ShowImportTrackDialog();
	/// 指定したファイルのファイル名にある数値から指定したトラックにインポートする
	bool ShowImportTrackRangeDialog(const wxString &path, int st_trk = -1, int st_sid = 0, int st_sec = 1);
	/// 指定したファイルから指定した範囲にトラックデータをインポート
	bool ImportTrackDataFile(const wxString &path, int st_trk, int st_sid, int st_sec, int ed_trk, int ed_sid, int ed_sec);
	/// ディスク全体のIDを変更
	void ModifyIDonDisk(int type_num);
	/// ディスク上の密度を一括変更
	void ModifyDensityOnDisk();

	/// ディスクを返す
	DiskD88Disk *GetDisk() const { return disk; }
	/// 選択行のトラックを返す
	DiskD88Track *GetSelectedTrack();
	/// 指定行のトラックを返す
	DiskD88Track *GetTrack(long row);
	/// 最初のトラックを返す
	DiskD88Track *GetFirstTrack();
	/// トラックのセクタ１を得る
	bool GetFirstSectorOnTrack(DiskD88Track **track, DiskD88Sector **sector);
	/// トラックの開始セクタ番号と終了セクタ番号を得る
	bool GetFirstAndLastSectorNumOnTrack(const DiskD88Track *track, int &start_sector, int &end_sector);

	enum {
		IDM_EXPORT_TRACK = 1,
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
		IDM_DELETE_TRACKS_BELOW,
	};

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRawTrack);
};


/// セクタリストの各アイテム
class L3DiskRawSectorItem
{
public:
	wxString filename;
	wxString attribute;
	int size;
	int groups;
public:
	L3DiskRawSectorItem(const wxString &newname, const wxString &newattr, int newsize, int newgrps);
	~L3DiskRawSectorItem() {}
};

WX_DECLARE_OBJARRAY(L3DiskRawSectorItem, L3DiskRawSectorItems);

/// セクタリストの挙動を設定
class L3DiskRawListStoreDerivedModel : public wxDataViewListStore
{
public:
    virtual bool IsEnabledByRow(unsigned int row, unsigned int col) const;
};

/// 右パネルのセクタリスト
class L3DiskRawSector : public wxDataViewListCtrl
{
private:
	L3DiskRawPanel *parent;
	L3DiskFrame *frame;

	wxMenu *menuPopup;

	DiskD88Track *track;

	wxDataViewItem selected_item;

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
	void OnDataViewItemContextMenu(wxDataViewEvent& event);
	/// セクタリスト ダブルクリック
	void OnDataViewItemActivated(wxDataViewEvent& event);
	/// セクタリスト選択
	void OnSelectionChanged(wxDataViewEvent& event);
	/// セクタリストからドラッグ開始
	void OnBeginDrag(wxDataViewEvent& event);
	/// セクタリスト エクスポート選択
	void OnExportFile(wxCommandEvent& event);
	/// セクタリスト インポート選択
	void OnImportFile(wxCommandEvent& event);
	/// トラック上のID一括変更選択
	void OnModifyIDonTrack(wxCommandEvent& event);
	/// トラック上の密度一括変更選択
	void OnModifyDensityOnTrack(wxCommandEvent& event);
	/// トラック上のセクタ数一括変更選択
	void OnModifySectorsOnTrack(wxCommandEvent& event);
	/// トラック上のセクタサイズ一括変更選択
	void OnModifySectorSizeOnTrack(wxCommandEvent& event);
	/// トラック上のセクタ一括削除選択
	void OnDeleteSectorsOnTrack(wxCommandEvent& event);
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
	/// セクタリストをリフレッシュ
	void RefreshSectors();
	/// セクタリストをクリア
	void ClearSectors();
	/// 選択しているセクタを返す
	DiskD88Sector *GetSelectedSector();
	/// セクタを返す
	DiskD88Sector *GetSector(const wxDataViewItem &item);

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
//	/// 指定したファイルからセクタにデータをインポート
//	bool ImportDataFile(const wxString &path);
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
	/// トラック上のセクタを一括削除
	void DeleteSectorsOnTrack();

	enum {
		IDM_EXPORT_FILE = 1,
		IDM_IMPORT_FILE,
		IDM_MODIFY_ID_C_TRACK,
		IDM_MODIFY_ID_H_TRACK,
		IDM_MODIFY_ID_R_TRACK,
		IDM_MODIFY_ID_N_TRACK,
		IDM_MODIFY_DENSITY_TRACK,
		IDM_MODIFY_SECTORS_TRACK,
		IDM_MODIFY_SIZE_TRACK,
		IDM_DELETE_SECTORS_BELOW,
		IDM_PROPERTY_SECTOR,
	};

	wxDECLARE_EVENT_TABLE();
};

#endif /* _UIRAWDISK_H_ */

