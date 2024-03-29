/// @file uimainprocess.h
///
/// @brief ディスク＆ファイル操作
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef UIMAINPROCESS_H
#define UIMAINPROCESS_H

#include "../common.h"
#include <wx/frame.h>
#include <wx/string.h>
#include <wx/dynarray.h>

class wxFileDataObject;

class UiDiskList;
class UiDiskFileList;
class UiDiskBinDumpFrame;
class IntNameBox;

class DiskBasic;
class DiskBasicDirItem;
class DiskBasicDirItems;
class DiskBasicDirItemAttr;

class DiskImageDisk;

/// ディスク＆ファイル操作
class UiDiskProcess : public wxFrame
{
protected:
	int m_unique_number;

	/// 指定したファイルをインポート
	int  ImportDataFiles(const wxString &data_dir, const wxString &attr_dir, const wxArrayString &names, DiskBasic *dir_basic, DiskBasicDirItem *dir_item, int depth);
	/// 指定したファイルをインポート
	int  ImportDataFile(const wxString &full_data_path, const wxString &full_attr_path, const wxString &file_name, DiskBasic *dir_basic, DiskBasicDirItem *dir_item);

	/// ファイル名ダイアログ表示と同じファイル名が存在する際のメッセージダイアログ表示
	int  ShowIntNameBoxAndCheckSameFile(DiskBasic *dir_basic, DiskBasicDirItem *dir_item, DiskBasicDirItem *temp_item, const wxString &file_name, int file_size, DiskBasicDirItemAttr &date_time, int style);

public:
	UiDiskProcess(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size);

	/// 指定したファイルをインポート
	bool ImportDataFile(const DiskBasicDirItem *item, const wxString &path, DiskBasic *dir_basic, DiskBasicDirItem *dir_item, const wxString &start_msg, const wxString &end_msg);
	/// 指定したファイルをインポート
	bool ImportDataFiles(const wxArrayString &paths, DiskBasic *dir_basic, DiskBasicDirItem *dir_item, bool included_dir, const wxString &start_msg, const wxString &end_msg);

	/// 指定したファイルにエクスポート
	bool ExportDataFile(DiskBasic *dir_basic, DiskBasicDirItem *item, const wxString &path, const wxString &start_msg, const wxString &end_msg);
	/// 指定したフォルダにエクスポート
	int  ExportDataFiles(DiskBasic *dir_basic, DiskBasicDirItems *dir_items, const wxString &data_dir, const wxString &attr_dir, wxFileDataObject *file_object, int depth);

	/// 指定したファイルを削除
	int  DeleteDataFile(DiskBasic *dir_basic, DiskBasicDirItem *dst_item);
	/// 指定したファイルを一括削除
	int  DeleteDataFiles(DiskBasic *dir_basic, DiskBasicDirItems &items, int depth, DiskBasicDirItems *dir_items);

	/// ディレクトリを作成できるか
	bool CanMakeDirectory(DiskBasic *dir_basic) const;
	/// ディレクトリ作成
	int  MakeDirectory(DiskBasic *dir_basic, DiskBasicDirItem *dir_item, const wxString &name, const wxString &title, DiskBasicDirItem **nitem);
	/// ディレクトリをアサイン
	bool AssignDirectory(DiskBasic *dir_basic, DiskBasicDirItem *dir_item);
	/// ディレクトリを移動
	bool ChangeDirectory(DiskBasic *dir_basic, DiskBasicDirItem *dir_item, bool refresh_list);
	/// ディレクトリを削除する
	bool DeleteDirectory(DiskBasic *dir_basic, DiskBasicDirItem *dir_item);

	/// ファイル名ダイアログの内容を反映させる
	bool SetDirItemFromIntNameDialog(DiskBasicDirItem *item, IntNameBox &dlg, DiskBasic *basic, bool rename);
	/// ファイル名を反映させる
	bool SetDirItemFromIntNameParam(DiskBasicDirItem *item, const wxString &file_path, const wxString &intname, DiskBasicDirItemAttr &date_time, DiskBasic *basic, bool rename);

	/// リストウィンドウのデフォルトフォントを得る
	virtual void GetDefaultListFont(wxFont &font) const {}

	/// @name 左パネルのディスクツリー
	//@{
	/// 左パネルのディスクツリーを返す
	virtual UiDiskList *GetDiskListPanel() { return NULL; }
	//@}

	/// @name 右下パネルのファイルリスト
	//@{
	/// 右下パネルのファイルリストパネルを返す
	virtual UiDiskFileList *GetFileListPanel(bool inst = false) { return NULL; }
	//@}

	/// ダンプウィンドウを返す
	virtual UiDiskBinDumpFrame *GetBinDumpFrame() const { return NULL; }

	/// @name ステータスカウンター
	//@{
	virtual void StartExportCounter(int count, const wxString &message) {}
	virtual void AppendExportCounter(int count) {}
	virtual void IncreaseExportCounter() {}
	virtual void BeginBusyCursorExportCounterIfNeed() {}
	virtual void FinishExportCounter(const wxString &message) {}

	virtual void StartImportCounter(int count, const wxString &message) {}
	virtual void AppendImportCounter(int count) {}
	virtual void IncreaseImportCounter() {}
	virtual void BeginBusyCursorImportCounterIfNeed() {}
	virtual void FinishImportCounter(const wxString &message) {}
	virtual void RestartImportCounter() {}
	//@}

	/// @name プロパティ
	//@{
	/// ユニーク番号
	int  GetUniqueNumber() const { return m_unique_number; }
	/// ユニーク番号を＋１
	void IncreaseUniqueNumber() { m_unique_number++; }
	//@}
};

#endif /* UIMAINPROCESS_H */

