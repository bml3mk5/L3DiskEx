/// @file uimainprocess.cpp
///
/// @brief ディスク＆ファイル操作
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uimainprocess.h"
#include <wx/dir.h>
#include <wx/dnd.h>
#include "uidisklist.h"
#include "uifilelist.h"
#include "intnamebox.h"
#include "intnamevalid.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basicdiritem.h"
#include "../diskimg/diskimage.h"
#include "../config.h"
#include "../utils.h"

//
// ディスク＆ファイル操作
//
UiDiskProcess::UiDiskProcess(wxWindow *parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(parent, id, title, pos, size)
{
	m_unique_number = 0;
}

/// 指定したファイルをインポート
/// @param [in]     paths     ファイルパスのリスト
/// @param [in,out] dir_basic 保存先のOS
/// @param [in,out] dir_item  保存先ディレクトリアイテム
/// @param [in]     confirm   ディレクトリを含む場合に確認ダイアログを表示するか 
/// @param [in]     start_msg 開始メッセージ
/// @param [in]     end_msg   終了メッセージ
/// @return true:OK false:Error
bool UiDiskProcess::ImportDataFiles(const wxArrayString &paths, DiskBasic *dir_basic, DiskBasicDirItem *dir_item, bool confirm, const wxString &start_msg, const wxString &end_msg)
{
	if (!dir_basic) {
		return false;
	}

	if (confirm) {
		// 確認ダイアログを表示する
		wxString name = dir_item->GetParent() ? dir_item->GetFileNameStr() : _("root directory");
		if (name.IsEmpty()) name = _("this directory");
		wxString msg = wxString::Format(_("Are you sure to import to %s?"), name);
		if (wxMessageBox(msg, start_msg, wxYES_NO) != wxYES) {
			return true;
		}
	}

	StartImportCounter(0, start_msg);

	int sts = 0;
	for(size_t n = 0; n < paths.Count() && sts >= 0; n++) {
		wxFileName file_path(paths.Item(n));
		wxString data_dir = file_path.GetPath();
		wxArrayString names;
		names.Add(file_path.GetFullName());
		file_path.RemoveLastDir();
		file_path.AppendDir(wxT("Attrs"));
		wxString attr_dir = file_path.GetPath();
		sts |= ImportDataFiles(data_dir, attr_dir, names, dir_basic, dir_item, 0);
	}

	FinishImportCounter(end_msg);

	// ディレクトリの表示は更新が必要
	dir_item->ValidDirectory(false);
	// 右パネルのリストを更新
	UiDiskFileList *file_list = GetFileListPanel();
	if (file_list) file_list->RefreshFiles();
	// 左パネルのツリーを更新
	UiDiskList *disk_list = GetDiskListPanel();
	if (disk_list) disk_list->RefreshAllDirectoryNodes(dir_basic->GetDisk(), dir_basic->GetSelectedSide(), dir_item);
	if (sts != 0) {
		dir_basic->ShowErrorMessage();
	}
	return (sts >= 0);
}

/// 指定したファイルをインポート
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param [in] data_dir      データフォルダ
/// @param [in] attr_dir      属性フォルダ
/// @param [in] names         ファイル名のリスト
/// @param [in,out] dir_basic 保存先のOS
/// @param [in,out] dir_item  保存先ディレクトリアイテム
/// @param [in] depth         深さ
/// @retval  1 警告あり
/// @retval  0 正常
/// @retval -1 エラー
int UiDiskProcess::ImportDataFiles(const wxString &data_dir, const wxString &attr_dir, const wxArrayString &names, DiskBasic *dir_basic, DiskBasicDirItem *dir_item, int depth)
{
	if (depth > gConfig.GetDirDepth()) {
		dir_basic->GetErrinfo().SetError(DiskBasicError::ERR_PATH_TOO_DEEP);
		return false;
	}

	int sts = 0;
	size_t count = names.Count();
	AppendImportCounter((int)count);

	for(size_t n = 0; n < count && sts >= 0; n++) {
		BeginBusyCursorImportCounterIfNeed();
		IncreaseImportCounter();

		wxString name = names.Item(n);
		wxString full_data_path = wxFileName(data_dir, name).GetFullPath();
		wxString full_attr_path = wxFileName(attr_dir, name, wxT("xml")).GetFullPath();
		// ファイル名を変換
		name = Utils::DecodeFileName(name);

		if (wxFileName::DirExists(full_data_path)) {
			// フォルダの場合

			// 新規ディレクトリ作成
			DiskBasicDirItem *new_dir_item = NULL;
			sts = MakeDirectory(dir_basic, dir_item, name, _("Import Directory"), &new_dir_item);
			if (sts < 0) {
				break;
			}

			// フォルダ内のファイルリスト
			wxArrayString sub_names;
			wxDir dir(full_data_path);
			wxString sub_name;
			bool valid = dir.GetFirst(&sub_name, wxEmptyString);
			while(valid) {
				sub_names.Add(sub_name);
				valid = dir.GetNext(&sub_name);
			}
			if (sub_names.Count() == 0) {
				continue;
			}

			// 新規ディレクトリに一時的に移動して初期化などを行う
			// TODO
			DiskBasicDirItem *cur_item = dir_basic->GetCurrentDirectory();
			if (!dir_basic->ChangeDirectory(new_dir_item)) {
				sts = -1;
				break;
			}
			dir_basic->ChangeDirectory(cur_item);

			// 再帰的にインポート
			sts |= ImportDataFiles(full_data_path, full_attr_path, sub_names, dir_basic, new_dir_item, depth + 1);

		} else {
			// ファイルの場合
			sts |= ImportDataFile(full_data_path, full_attr_path, name, dir_basic, dir_item);

		}
	}

	return sts;
}

/// 指定したファイルを上書きでインポート
/// @param [in] item          保存するファイルのディレクトリアイテム（属性などを持っている）
/// @param [in] path          保存するデータファイルパス
/// @param [in,out] dir_basic 保存先のOS
/// @param [in,out] dir_item  保存先ディレクトリアイテム
/// @param [in] start_msg     開始メッセージ
/// @param [in] end_msg       終了メッセージ
/// @return true:OK false:Error
bool UiDiskProcess::ImportDataFile(const DiskBasicDirItem *item, const wxString &path, DiskBasic *dir_basic, DiskBasicDirItem *dir_item, const wxString &start_msg, const wxString &end_msg)
{
	if (!dir_basic) {
		return false;
	}

	// 仮ディレクトリアイテムを作成
	DiskBasicDirItem *pitem = dir_basic->CreateDirItem();
	pitem->CopyItem(*item);

	StartImportCounter(1, start_msg);

	bool valid = dir_basic->SaveFile(path, dir_item, pitem);

	IncreaseImportCounter();
	FinishImportCounter(end_msg);

	// ディレクトリの表示は更新が必要
	dir_item->ValidDirectory(false);
	// 右パネルのリストを更新
	UiDiskFileList *file_list = GetFileListPanel();
	if (file_list) file_list->RefreshFiles();
	// 左パネルのツリーを更新
	UiDiskList *disk_list = GetDiskListPanel();
	if (disk_list) disk_list->RefreshAllDirectoryNodes(dir_basic->GetDisk(), dir_basic->GetSelectedSide(), dir_item);

	delete pitem;
	return valid;
}

/// 指定したファイルをインポート
/// @param [in]     full_data_path データファイルパス
/// @param [in]     full_attr_path 属性ファイルパス
/// @param [in]     file_name      ファイル名
/// @param [in,out] dir_basic      保存先のOS
/// @param [in,out] dir_item       保存先ディレクトリアイテム
/// @retval  1 警告あり処理継続
/// @retval  0 正常
/// @retval -1 エラー継続不可
int UiDiskProcess::ImportDataFile(const wxString &full_data_path, const wxString &full_attr_path, const wxString &file_name, DiskBasic *dir_basic, DiskBasicDirItem *dir_item)
{
	if (!dir_basic) {
		return -1;
	}

	if (!dir_basic->IsFormatted()) {
		return -1;
	}

	// ディスクの残りサイズのチェックと入力ファイルのサイズを得る
	int file_size = 0;
	if (!dir_basic->CheckFile(full_data_path, &file_size)) {
		return -1;
	}

	// 外部からインポートのスタイル
	int style = INTNAME_NEW_FILE | INTNAME_SHOW_TEXT | INTNAME_SHOW_ATTR | INTNAME_SPECIFY_FILE_NAME | INTNAME_SHOW_SKIP_DIALOG;

	int sts = 0;
	DiskBasicDirItem *temp_item = dir_basic->CreateDirItem();

	// ファイル情報があれば読み込む
	wxString filename = file_name;
	DiskBasicDirItemAttr date_time;
	if (temp_item->ReadFileAttrFromXml(full_attr_path, &date_time)) {
		// ファイル名
		filename = temp_item->GetFileNameStr();
		// 内部からインポートに変更
		style = INTNAME_IMPORT_INTERNAL | INTNAME_SHOW_TEXT | INTNAME_SHOW_ATTR
			| INTNAME_SPECIFY_CDATE_TIME | INTNAME_SPECIFY_MDATE_TIME | INTNAME_SPECIFY_ADATE_TIME
			| INTNAME_SHOW_SKIP_DIALOG;
		// コピーできるか
		if (!temp_item->IsCopyable()) {
			// エラーにはしない
			sts = 1;
		}
	} else {
		// ファイルから日付を得る
		temp_item->ReadFileDateTime(full_data_path, date_time);
		style |= INTNAME_SPECIFY_CDATE_TIME | INTNAME_SPECIFY_MDATE_TIME | INTNAME_SPECIFY_ADATE_TIME;
	}
	if (sts == 0) {
		// ダイアログ表示
		int ans = ShowIntNameBoxAndCheckSameFile(dir_basic, dir_item, temp_item, filename, file_size, date_time, style);
		if (ans == wxYES) {
			// ディスク内にセーブする
			DiskBasicDirItem *madeitem = NULL;
			bool valid = dir_basic->SaveFile(full_data_path, dir_item, temp_item, &madeitem);
			if (!valid) {
				sts = -1;
			}
		} else {
			sts = -1;
		}
	}
	delete temp_item;
	return sts;
}

/// 指定したファイルにエクスポート
/// @param[in] dir_basic    抽出元のOS
/// @param[in] item         抽出したいディレクトリアイテム
/// @param[in] path         ファイルパス
/// @param[in] start_msg    開始メッセージ
/// @param[in] end_msg      終了メッセージ
/// @return true:OK false:Error
bool UiDiskProcess::ExportDataFile(DiskBasic *dir_basic, DiskBasicDirItem *item, const wxString &path, const wxString &start_msg, const wxString &end_msg)
{
	if (!dir_basic) return false;

	StartExportCounter(1, start_msg);

	// ロード
	bool valid = dir_basic->LoadFile(item, path);
	// 日付を反映
	if (valid) {
		item->WriteFileDateTime(path);
	}

	IncreaseExportCounter();
	FinishExportCounter(end_msg);

	if (!valid) {
		dir_basic->ShowErrorMessage();
	}
	return valid;
}

/// 指定したフォルダにエクスポート
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param [in]     dir_basic   抽出元のOS
/// @param [in]     dir_items   選択したリスト
/// @param [in]     data_dir    データファイル出力先フォルダ
/// @param [in]     attr_dir    属性ファイル出力先フォルダ
/// @param [in,out] file_object ファイルオブジェクト
/// @param [in]     depth       深さ
/// @retval  1 警告あり
/// @retval  0 正常
/// @retval -1 エラー
int UiDiskProcess::ExportDataFiles(DiskBasic *dir_basic, DiskBasicDirItems *dir_items, const wxString &data_dir, const wxString &attr_dir, wxFileDataObject *file_object, int depth)
{
	if (!dir_items) return 0;

	if (depth > gConfig.GetDirDepth()) {
		return -1;
	}

	int sts = 0;
	// エクスポート可能なファイルを選択
	size_t count = dir_items->Count();
	DiskBasicDirItems valid_items;
	for(size_t n = 0; n < count && sts >= 0; n++) {
		DiskBasicDirItem *item = dir_items->Item(n);
		if (!item) {
			continue;
		}
		// 未使用は不可
		if (!item->IsUsed()) {
			continue;
		}
		// ロード不可
		if (!item->IsLoadable()) {
			continue;
		}
		valid_items.Add(item);
	}

	count = valid_items.Count();

	AppendExportCounter((int)count);

	bool attr_exists = !attr_dir.IsEmpty();
	for(size_t n = 0; n < count && sts >= 0; n++) {
		IncreaseExportCounter();
		BeginBusyCursorExportCounterIfNeed();

		DiskBasicDirItem *item = valid_items.Item(n);

		wxString native_name = item->GetFileNameStrForExport();
		// エクスポートする前の処理（ファイル名を変更するか）
		if (!item->PreExportDataFile(native_name)) {
			sts = -1;
			break;
		}
		if (native_name.IsEmpty()) {
			continue;
		}
		// ファイル名に設定できない文字をエスケープ
		wxString file_name = Utils::EncodeFileName(native_name);
		// フルパスを作成
		wxString full_data_name = wxFileName(data_dir, file_name).GetFullPath();
		wxString full_attr_name = attr_exists ? wxFileName(attr_dir, file_name, wxT("xml")).GetFullPath() : attr_dir;
		if (full_data_name.Length() > 255 || full_attr_name.Length() > 255) {
			// パスが長すぎる
			sts = 1;
			dir_basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
			dir_basic->GetErrinfo().SetError(DiskBasicError::ERR_PATH_TOO_DEEP);
			continue;
		}
		if (item->IsDirectory()) {
			// ディレクトリの場合
			// ディレクトリをアサイン
			bool valid = dir_basic->AssignDirectory(item);
			if (!valid) {
				sts = 1;
				dir_basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
				continue;
			}
			// データサブフォルダを作成
			if (wxFileName::FileExists(full_data_name) || wxFileName::DirExists(full_data_name)) {
				// 既にある
				sts = 1;
				dir_basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
				dir_basic->GetErrinfo().SetError(DiskBasicError::ERR_FILE_ALREADY_EXIST);
				continue;
			}
			if (!wxMkdir(full_data_name)) {
				sts = 1;
				dir_basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
				continue;
			}
			if (attr_exists) {
				// 属性サブフォルダを作成
				if (!wxMkdir(full_attr_name)) {
					sts = 1;
					dir_basic->GetErrinfo().SetError(DiskBasicError::ERRV_CANNOT_EXPORT, native_name.wc_str());
					continue;
				}
			}
			// 再帰的にエクスポート
			sts |= ExportDataFiles(dir_basic, item->GetChildren(), full_data_name, full_attr_name, file_object, depth + 1);
		} else {
			// ファイルの場合
			bool rc = dir_basic->LoadFile(item, full_data_name);
			sts |= (rc ? 0 : -1);
			// 日付を反映
			if (rc) {
				item->WriteFileDateTime(full_data_name);
			}
			// 属性情報をXMLで出力
			if (attr_exists) {
				item->WriteFileAttrToXml(full_attr_name);
			}
		}

		// ファイルオブジェクトを追加(DnD用)
		// トップレベルのみ追加
		if (depth == 0 && file_object != NULL && sts >= 0) {
			file_object->AddFile(full_data_name);
		}
	}
	return sts;
}

/// 指定したファイルを削除
/// @param[in]     dir_basic BASIC
/// @param[in,out] dst_item  削除対象アイテム
/// @return 0:OK >0:Warning <0:Error
int UiDiskProcess::DeleteDataFile(DiskBasic *dir_basic, DiskBasicDirItem *dst_item)
{
	if (!dst_item) return -1;

	int sts = dir_basic->IsDeletableFile(dst_item);
	if (sts == 0) {
		bool is_directory = dst_item->IsDirectory();
		wxString filename = dst_item->GetFileNameStr();
		wxString msg = wxString::Format(_("Do you really want to delete '%s'?"), filename);
		int ans = wxMessageBox(msg, is_directory ? _("Delete a directory") : _("Delete a file"), wxYES_NO);
		if (ans != wxYES) {
			return -1;
		}
		DiskBasicDirItems dst_items;
		dst_items.Add(dst_item);
		sts = DeleteDataFiles(dir_basic, dst_items, 0, NULL);

		// リスト更新
		UiDiskList *disk_list = GetDiskListPanel();
		if (disk_list) disk_list->DeleteDirectoryNode(dir_basic->GetDisk(), dst_item);
		UiDiskFileList *file_list = GetFileListPanel();
		if (file_list) file_list->RefreshFiles();
	}
	if (sts != 0) {
		dir_basic->ShowErrorMessage();
	}
	return sts;
}

/// 指定したファイルを一括削除（再帰的）
/// @attention 再帰的に呼ばれる。 This function is called recursively.
/// @param[in]     dir_basic       BASIC
/// @param[in,out] items           削除対象アイテムリスト
/// @param[in]     depth           深さ
/// @param[in,out] dir_items       サブディレクトリアイテムリスト
/// @return 0:OK >0:Warning <0:Error
int UiDiskProcess::DeleteDataFiles(DiskBasic *dir_basic, DiskBasicDirItems &items, int depth, DiskBasicDirItems *dir_items)
{
	if (depth > gConfig.GetDirDepth()) {
		return 1;
	}

	int sts = 0;
	// 機種によってはアイテムをリストから削除するので予めリストをコピー
	DiskBasicDirItems tmp_items = items;
	size_t tmp_count = (int)items.Count();
	for(size_t n = 0; n < tmp_count && sts >= 0; n++) {
		DiskBasicDirItem *item = tmp_items.Item(n);
		if (!item) {
			continue;
		}
		if (!item->IsUsed()) {
			continue;
		}
		// 削除できるか
		if (!item->IsDeletable()) {
			continue;
		}
		bool is_directory = item->IsDirectory();
		if (is_directory) {
			// アサイン
			dir_basic->AssignDirectory(item);
			// ディレクトリのときは先にディレクトリ内ファイルを削除
			DiskBasicDirItems *sitems = item->GetChildren();
			if (sitems) {
				int ssts = DeleteDataFiles(dir_basic, *sitems, depth + 1, NULL);
				if (ssts == 0 && dir_items) {
					dir_items->Add(item);
				}
				sts |= (ssts != 0 ? -1 : 0);
			}
		}
		if (sts >= 0) {
			// 削除
			bool ssts = dir_basic->DeleteFile(item, false);
			sts |= (ssts ? 0 : -1);
		}
	}
	return sts;
}

/// ディレクトリを作成できるか
/// @return true:できる false:できない
bool UiDiskProcess::CanMakeDirectory(DiskBasic *dir_basic) const
{
	return dir_basic ? dir_basic->CanMakeDirectory() : false;
}

/// ディレクトリ作成
/// ディレクトリ名が重複する時にダイアログを表示
/// @param[in,out] dir_basic 作成先のOS
/// @param[in,out] dir_item  作成先のディレクトリ
/// @param[in]     name      ディレクトリ名
/// @param[in]     title     ダイアログのタイトル
/// @param[out]    nitem     作成したディレクトリアイテム
/// @return 1:同じ名前がある -1:その他エラー
int UiDiskProcess::MakeDirectory(DiskBasic *dir_basic, DiskBasicDirItem *dir_item, const wxString &name, const wxString &title, DiskBasicDirItem **nitem)
{
	int sts = 1;
	wxString dir_name = name;
	{
		// 必要なら名前を変更
		DiskBasicDirItem *pre_item = dir_basic->CreateDirItem();
		if (!pre_item->PreImportDataFile(dir_name)) {
			sts = -1;
		}
		delete pre_item;
	}
	while (sts > 0) {
		sts = dir_basic->MakeDirectory(dir_item, dir_name, gConfig.DoesIgnoreDateTime(), nitem);
		if (sts == 1) {
			// 同じ名前があるのでダイアログ表示
			dir_basic->ClearErrorMessage();
			wxString msgs = _("The same file name or directory already exists.");
			msgs += wxT("\n");
			msgs += _("Please rename this.");
			DiskBasicDirItem *temp_item = dir_basic->CreateDirItem();

			IntNameBox dlg(this, this, wxID_ANY, title
				, msgs
				, dir_basic, temp_item, dir_name, dir_name, 0, NULL
				, INTNAME_NEW_FILE | INTNAME_SHOW_TEXT | INTNAME_SPECIFY_FILE_NAME);

			int ans = dlg.ShowModal();
			if (ans != wxID_OK) {
				sts = -1;
			} else {
				dlg.GetInternalName(dir_name);
				if (temp_item->CanIgnoreDateTime()) {
					gConfig.IgnoreDateTime(dlg.DoesIgnoreDateTime(gConfig.DoesIgnoreDateTime()));
				}
			}
			delete temp_item;
		}
	}

	return sts;
}

/// ディレクトリをアサインする
/// @param [in] dir_basic 現在のOS
/// @param [in] dir_item ディレクトリのアイテム
/// @return true:OK false:Error
bool UiDiskProcess::AssignDirectory(DiskBasic *dir_basic, DiskBasicDirItem *dir_item)
{
	if (!dir_basic) return false;

	bool sts = dir_basic->AssignDirectory(dir_item);
	if (sts) {
		// リスト更新
		UiDiskList *disk_list = GetDiskListPanel();
		if (disk_list) disk_list->SelectDirectoryNode(dir_basic->GetDisk(), dir_item);
	}
	return sts;
}

/// ディレクトリを移動する
/// @param [in] dir_basic 現在のOS
/// @param [in] dir_item 移動先ディレクトリのアイテム
/// @param [in] refresh_list ファイルリストを更新するか
/// @return true:OK false:Error
bool UiDiskProcess::ChangeDirectory(DiskBasic *dir_basic, DiskBasicDirItem *dir_item, bool refresh_list)
{
	if (!dir_basic) return false;

	bool sts = dir_basic->ChangeDirectory(dir_item);
	if (sts) {
		// リスト更新
		if (refresh_list) {
			UiDiskFileList *file_list = GetFileListPanel();
			if (file_list) file_list->SetFiles();
		}
		UiDiskList *disk_list = GetDiskListPanel();
		if (disk_list) disk_list->SelectDirectoryNode(dir_basic->GetDisk(), dir_item);
	}
	return sts;
}

/// ディレクトリを削除する
/// @param [in] dir_basic 現在のOS
/// @param [in] dir_item  削除するディレクトリのアイテム
/// @return true:OK false:Error
bool UiDiskProcess::DeleteDirectory(DiskBasic *dir_basic, DiskBasicDirItem *dir_item)
{
	if (!dir_basic || !dir_item) return false;

	DiskBasicDirItem *parent = dir_item->GetParent();
	if (parent) {
		// 親ディレクトリの表示は更新が必要
		parent->ValidDirectory(false);
	}
	int sts = DeleteDataFile(dir_basic, dir_item);

	return (sts != 0);
}

/// ファイル名ダイアログ表示と同じファイル名が存在する際のメッセージダイアログ表示
/// @param [in] dir_basic 現在のOS
/// @param [in] dir_item  現在のディレクトリ
/// @param [in] temp_item ディレクトリアイテム
/// @param [in] file_name ファイルパス
/// @param [in] file_size ファイルサイズ
/// @param [in] date_time 日時
/// @param [in] style     スタイル(IntNameBoxShowFlags)
/// @retval wxYES
/// @retval wxCANCEL
int UiDiskProcess::ShowIntNameBoxAndCheckSameFile(DiskBasic *dir_basic, DiskBasicDirItem *dir_item, DiskBasicDirItem *temp_item, const wxString &file_name, int file_size, DiskBasicDirItemAttr &date_time, int style)
{
	int ans = wxNO;
	bool skip_dlg = gConfig.IsSkipImportDialog();
	IntNameBox *dlg = NULL;
	wxString int_name = file_name;

	// ファイルパスからファイル名を生成
	if (style & INTNAME_NEW_FILE) {
		// 外部からインポート時
		if (!temp_item->PreImportDataFile(int_name)) {
			// エラー
			ans = wxCANCEL;
		}
	}
	while (ans != wxYES && ans != wxCANCEL) {
		if (!skip_dlg) {
			// ファイル名ダイアログを表示
			if (!dlg) dlg = new IntNameBox(this, this, wxID_ANY, _("Import File"), wxT(""), dir_basic, temp_item, file_name, int_name, file_size, &date_time, style);
			int dlgsts = dlg->ShowModal();

			RestartImportCounter();

			if (dlgsts == wxID_OK) {
				dlg->GetInternalName(int_name);

				// ダイアログで指定したファイル名や属性値をアイテムに反映
				if (!SetDirItemFromIntNameDialog(temp_item, *dlg, dir_basic, true)) {
					ans = wxCANCEL;
					break;
				}

				// ファイルサイズのチェック
				int limit = 0;
				if (!temp_item->IsFileValidSize(dlg, file_size, &limit)) {
					wxString msg = wxString::Format(_("File size is larger than %d bytes, do you want to continue?"), limit);
					ans = wxMessageBox(msg, _("File is too large"), wxYES_NO | wxCANCEL);
					if (ans == wxNO) continue;
					else if (ans == wxCANCEL) break;
				} else {
					ans = wxYES;
				}
			} else {
				ans = wxCANCEL;
				break;
			}
		} else {
			// ダイアログを表示しないとき
			if (style & INTNAME_NEW_FILE) {
				// 外部からインポート時でダイアログなし
				// ファイル名が適正か
				IntNameValidator vali(temp_item, _("file name"), dir_basic->GetValidFileName());
				if (!vali.Validate(this, int_name)) {
					// ファイル名が不適切
					skip_dlg = false;
					ans = wxNO;
					continue;
				}
				// 属性をファイル名から判定してアイテムに反映
				if (!SetDirItemFromIntNameParam(temp_item, file_name, int_name, date_time, dir_basic, true)) {
					ans = wxCANCEL;
					break;
				}
			} else {
				// 内部からインポート時
				bool ignore_datetime = gConfig.DoesIgnoreDateTime();
				DiskBasicDirItem::enDateTime ignore_type = temp_item->CanIgnoreDateTime();
				if (!(ignore_datetime && (ignore_type & DiskBasicDirItem::DATETIME_CREATE) != 0)) {
					temp_item->SetFileCreateDateTime(date_time.GetCreateDateTime());
				}
				if (!(ignore_datetime && (ignore_type & DiskBasicDirItem::DATETIME_MODIFY) != 0)) {
					temp_item->SetFileModifyDateTime(date_time.GetModifyDateTime());
				}
				if (!(ignore_datetime && (ignore_type & DiskBasicDirItem::DATETIME_ACCESS) != 0)) {
					temp_item->SetFileAccessDateTime(date_time.GetAccessDateTime());
				}
			}
			ans = wxYES;
		}

		if (ans == wxYES) {
			// ファイル名重複チェック
			int sts = dir_basic->IsFileNameDuplicated(dir_item, temp_item);
			if (sts < 0) {
				// 既に存在します 上書き不可
				skip_dlg = false;
				wxString msg = wxString::Format(_("File '%s' already exists and cannot overwrite, please rename it."), temp_item->GetFileNameStr());
				ans = wxMessageBox(msg, _("File exists"), wxOK | wxCANCEL);
				if (ans == wxOK) continue;
				else break;
			} else if (sts == 1) {
				// 上書き確認ダイアログ
				skip_dlg = false;
				wxString msg = wxString::Format(_("File '%s' already exists, do you really want to overwrite it?"), temp_item->GetFileNameStr());
				ans = wxMessageBox(msg, _("File exists"), wxYES_NO | wxCANCEL);
				if (ans == wxNO) continue;
				else if (ans == wxCANCEL) break;
			} else {
				ans = wxYES;
			}
		} else {
			ans = wxCANCEL;
			break;
		}
	}

	delete dlg;
	return ans;
}

/// ファイル名ダイアログの内容を反映させる
/// @param [in] item   ディレクトリアイテム
/// @param [in] dlg    ファイル名ダイアログ
/// @param [in] basic  BASIC
/// @param [in] rename ファイル名を変更できるか
/// @return true:OK false:Error
bool UiDiskProcess::SetDirItemFromIntNameDialog(DiskBasicDirItem *item, IntNameBox &dlg, DiskBasic *basic, bool rename)
{
	DiskBasicDirItemAttr attr;

	// パラメータを設定に反映
	gConfig.SkipImportDialog(dlg.IsSkipDialog(gConfig.IsSkipImportDialog()));
	if (item->CanIgnoreDateTime()) {
		gConfig.IgnoreDateTime(dlg.DoesIgnoreDateTime(gConfig.DoesIgnoreDateTime()));
	}

	// 属性をアイテムに反映
	wxString newname;

	dlg.GetInternalName(newname);
	attr.Renameable(rename);
	attr.SetFileName(newname, item->GetOptionalNameInAttrDialog(&dlg));

	attr.IgnoreDateTime(gConfig.DoesIgnoreDateTime());
	
	attr.SetCreateDateTime(dlg.GetCreateDateTime());
	attr.SetModifyDateTime(dlg.GetModifyDateTime());
	attr.SetAccessDateTime(dlg.GetAccessDateTime());

	attr.SetStartAddress(dlg.GetStartAddress());
	attr.SetEndAddress(dlg.GetEndAddress());
	attr.SetExecuteAddress(dlg.GetExecuteAddress());

	// 機種依存の属性をアイテムに反映
	bool sts = item->SetAttrInAttrDialog(&dlg, attr, basic->GetErrinfo());

	if (sts) {
		// 必要なら属性値を加工する
		sts = item->ProcessAttr(attr, basic->GetErrinfo());
	}
	if (sts) {
		// 属性を更新
		sts = basic->ChangeAttr(item, attr);
	}

	return sts;
}

/// ファイル名を反映させる
/// @param [in] item      ディレクトリアイテム
/// @param [in] file_path ファイルパス
/// @param [in] intname   内部ファイル名
/// @param [in] date_time 日時
/// @param [in] basic     BASIC
/// @param [in] rename    ファイル名を変更できるか
/// @return true:OK false:Error
bool UiDiskProcess::SetDirItemFromIntNameParam(DiskBasicDirItem *item, const wxString &file_path, const wxString &intname, DiskBasicDirItemAttr &date_time, DiskBasic *basic, bool rename)
{
	DiskBasicDirItemAttr attr;

	// 属性をアイテムに反映
	wxString newname;

	attr.Renameable(rename);
	attr.SetFileName(intname, item->ConvOptionalNameFromFileName(file_path));

	attr.IgnoreDateTime(gConfig.DoesIgnoreDateTime());
	attr.SetCreateDateTime(date_time.GetCreateDateTime());
	attr.SetModifyDateTime(date_time.GetModifyDateTime());
	attr.SetAccessDateTime(date_time.GetAccessDateTime());

	// ファイル名から属性を設定
	attr.SetFileAttr(basic->GetFormatTypeNumber(), item->ConvFileTypeFromFileName(file_path), item->ConvOriginalTypeFromFileName(file_path));

	bool sts = true;
	if (sts) {
		// 必要なら属性値を加工する
		sts = item->ProcessAttr(attr, basic->GetErrinfo());
	}
	if (sts) {
		// 属性を更新
		sts = basic->ChangeAttr(item, attr);
	}
	return sts;
}
