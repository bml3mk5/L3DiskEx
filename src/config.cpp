/// @file config.cpp
///
/// @brief 設定ファイル入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "config.h"
#include <wx/filename.h>
#include <wx/fileconf.h>


Config gConfig;


ColumnParams::ColumnParams(const struct st_list_columns *list, int pos_start, int pos_end)
{
	mStart = pos_start;
	mEnd = pos_end;
	pList = list;
	int size = mEnd - mStart;
	mWidth.Alloc(size);
	mPos.Alloc(size);
	for (int idx = 0; idx<size; idx++) {
		mWidth.Add(-1);
		mPos.Add(idx + mStart);
	}
}

void ColumnParams::Load(wxFileConfig *ini, const wxString &prefix)
{
	// リストのカラム幅
	int size = mEnd - mStart;
	for (int idx = 0; idx < size; idx++) {
		int id = idx + mStart;
		wxString key = prefix;
		key += pList[id].name;
		key += wxT("Width");
		int val = GetWidth(idx);
		ini->Read(key, &val);
		SetWidth(idx, val);
	}
	// リストのカラム位置
	for (int idx = 0; idx < size; idx++) {
		int id = idx + mStart;
		wxString key = prefix;
		key += pList[id].name;
		key += wxT("Pos");
		int val = GetPos(idx);
		ini->Read(key, &val);
		SetPos(idx, val);
	}
}

void ColumnParams::Save(wxFileConfig *ini, const wxString &prefix)
{
	// リストのカラム幅
	int size = mEnd - mStart;
	for (int idx = 0; idx < size; idx++) {
		int id = idx + mStart;
		wxString key = prefix;
		key += pList[id].name;
		key += wxT("Width");
		ini->Write(key, GetWidth(idx));
	}
	// リストのカラム位置
	for (int idx = 0; idx < size; idx++) {
		int id = idx + mStart;
		wxString key = prefix;
		key += pList[id].name;
		key += wxT("Pos");
		ini->Write(key, GetPos(idx));
	}
}

FileColumnParams::FileColumnParams()
	: ColumnParams(gUiDiskFileListColumnDefs, LISTCOL_NAME, LISTCOL_END)
{
}


Params::Params()
{
	// default value
	mFilePath = wxT("");
	mExportFilePath = wxT("");
	mRecentFiles.Empty();
	mCharCode.Empty();
	mListFontName.Empty();
	mListFontSize = 0;
	mDumpFontName.Empty();
	mDumpFontSize = 0;
	mTrimUnusedData = true;
	mShowDeletedFile = false;
	mAddExtExport = true;
	mCurrentDateExport = false;
	mDecideAttrImport = true;
	mSkipImportDialog = false;
	mIgnoreDateTime = false;
#ifdef _DEBUG
	mShowInterDirItem = true;
#else
	mShowInterDirItem = false;
#endif
	mCheckSideNumber = false;
	mDirDepth = 20;
	mWindowWidth = 1000;
	mWindowHeight = 600;
	mTemporaryFolder.Empty();
	mBinaryEditor.Empty();
	mTextEditor.Empty();
	mLanguage.Empty();
	mLPanelWidth = mWindowWidth * 20 / 100;	// 20%
	mTrkPanelWidth = mWindowWidth * 23 / 100;	// 23%
}

void Params::SetFilePath(const wxString &val)
{
	mFilePath = wxFileName::FileName(val).GetPath();
}

void Params::SetExportFilePath(const wxString &val, bool is_dir)
{
	if (is_dir) {
		mExportFilePath = wxFileName::FileName(val).GetFullPath();
	} else {
		mExportFilePath = wxFileName::FileName(val).GetPath();
	}
}

const wxString &Params::GetExportFilePath() const
{
	if (mExportFilePath.IsEmpty()) {
		return mFilePath;
	} else {
		return mExportFilePath;
	}
}

void Params::AddRecentFile(const wxString &val)
{
	wxFileName fpath = wxFileName::FileName(val);
	mFilePath = fpath.GetPath();
	// 同じファイルがあるか
	int pos = mRecentFiles.Index(fpath.GetFullPath());
	if (pos >= 0) {
		// 消す
		mRecentFiles.RemoveAt(pos);
	}
	// 追加
	mRecentFiles.Insert(fpath.GetFullPath(), 0);
	// MAX_RECENT_FILESを超える分は消す
	if (mRecentFiles.Count() > MAX_RECENT_FILES) {
		mRecentFiles.RemoveAt(MAX_RECENT_FILES);
	}
}

const wxString &Params::GetRecentFile() const
{
	return mRecentFiles.Count() > 0 ? mRecentFiles[0] : mFilePath;
}

const wxArrayString &Params::GetRecentFiles() const
{
	return mRecentFiles;
}

void Params::SetTemporaryFolder(const wxString &val)
{
	mTemporaryFolder = wxFileName::FileName(val).GetFullPath();
}

void Params::SetBinaryEditor(const wxString &val)
{
	mBinaryEditor = wxFileName::FileName(val).GetFullPath();
}

void Params::SetTextEditor(const wxString &val)
{
	mTextEditor = wxFileName::FileName(val).GetFullPath();
}

//
//
//

Config::Config() : Params()
{
	ini_file = wxT("");
}

Config::~Config()
{
}

void Config::SetFileName(const wxString &file)
{
	ini_file = file;
}

void Config::Load()
{
	if (ini_file.IsEmpty()) return;

	int ival;

	// load ini file
	wxFileConfig *ini = new wxFileConfig(wxEmptyString,wxEmptyString,ini_file,wxEmptyString
		,wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
	// ファイルパス
	ini->Read(wxT("Path"), &mFilePath, mFilePath);
	// エクスポート先パス
	ini->Read(wxT("ExportPath"), &mExportFilePath, mExportFilePath);
	// 最近使用したファイル
	for(int i=0; i<MAX_RECENT_FILES; i++) {
		wxString sval;
		ini->Read(wxString::Format(wxT("Recent%d"), i), &sval);
		if (!sval.IsEmpty()) {
			mRecentFiles.Add(sval);
		}
	}
	// キャラクターコードマップ名
	ini->Read(wxT("CharCode"), &mCharCode);
	// リストウィンドウのフォント名
	ini->Read(wxT("ListFontName"), &mListFontName);
	// リストウィンドウのフォントサイズ
	ini->Read(wxT("ListFontSize"), &mListFontSize);
	// ダンプウィンドウのフォント名
	ini->Read(wxT("DumpFontName"), &mDumpFontName);
	// ダンプウィンドウのフォントサイズ
	ini->Read(wxT("DumpFontSize"), &mDumpFontSize);
	// 未使用データを切り落とすか
	ini->Read(wxT("TrimUnusedData"), &mTrimUnusedData);
	// 削除したファイルを表示するか
	ini->Read(wxT("ShowDeletedFile"), &mShowDeletedFile);
	// エクスポート時に属性から拡張子を追加するか
	ini->Read(wxT("AddExtensionWhenExport"), &mAddExtExport);
	// エクスポート時に現在日時を設定するか
	ini->Read(wxT("SetCurrentDateTimeWhenExport"), &mCurrentDateExport);
	// インポート時に拡張子で属性を決定したら拡張子を削除するか
	ini->Read(wxT("DeleteExtensionWhenImport"), &mDecideAttrImport);
	// インポートやプロパティ変更時に日時を無視するか
	ini->Read(wxT("IgnoreDateTime"), &mIgnoreDateTime);
	// インポート時に現在日時を設定するか
	ini->Read(wxT("SetCurrentDateTimeWhenImport"), &mCurrentDateImport);
	// プロパティで内部データをリストで表示するか
	ini->Read(wxT("ShowInterDirItem"), &mShowInterDirItem);
	// オープン時サイド番号をチェックするか
	ini->Read(wxT("CheckSideNumber"), &mCheckSideNumber);
	// 一度に処理できるディレクトリの深さ
	ival = 0;
	ini->Read(wxT("DirectoriesDepth"), &ival);
	if (ival >= 1 && ival <= 100) mDirDepth = ival;
	// ウィンドウ幅
	ini->Read(wxT("WindowWidth"), &mWindowWidth);
	// ウィンドウ高さ
	ini->Read(wxT("WindowHeight"), &mWindowHeight);
	// テンポラリフォルダのパス
	ini->Read(wxT("TemporaryFolder"), &mTemporaryFolder);
	// バイナリエディタのパス
	ini->Read(wxT("BinaryEditor"), &mBinaryEditor);
	if (mBinaryEditor.IsEmpty()) {
		ini->Read(wxT("BinaryEditer"), &mBinaryEditor);
	}
	// テキストエディタのパス
	ini->Read(wxT("TextEditor"), &mTextEditor);
	// 言語
	ini->Read(wxT("Language"), &mLanguage);
	// ファイルリストのカラム
	mFileColumn.Load(ini, wxT("ListColumn"));
	// 左パネル（ツリー）の幅
	ini->Read(wxT("LeftPanelWidth"), &mLPanelWidth);
	// トラックパネルの幅
	ini->Read(wxT("TrackPanelWidth"), &mTrkPanelWidth);

	delete ini;
}

void Config::Load(const wxString &file)
{
	SetFileName(file);
	Load();
}

void Config::Save()
{
	if (ini_file.IsEmpty()) return;

	// save ini file
	wxFileConfig *ini = new wxFileConfig(wxEmptyString,wxEmptyString,ini_file,wxEmptyString
		,wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH | wxCONFIG_USE_NO_ESCAPE_CHARACTERS);
	// ファイルパス
	ini->Write(wxT("Path"), mFilePath);
	// エクスポート先パス
	ini->Write(wxT("ExportPath"), mExportFilePath);
	// 最近使用したファイル
	for(int i=0,row=0; row<MAX_RECENT_FILES && i<(int)mRecentFiles.Count(); i++) {
		wxString sval = mRecentFiles.Item(i);
		if (sval.IsEmpty()) continue;
		ini->Write(wxString::Format(wxT("Recent%d"), row), sval);
		row++;
	}
	// キャラクターコードマップ名
	ini->Write(wxT("CharCode"), mCharCode);
	// リストウィンドウのフォント名
	ini->Write(wxT("ListFontName"), mListFontName);
	// リストウィンドウのフォントサイズ
	ini->Write(wxT("ListFontSize"), mListFontSize);
	// ダンプウィンドウのフォント名
	ini->Write(wxT("DumpFontName"), mDumpFontName);
	// ダンプウィンドウのフォントサイズ
	ini->Write(wxT("DumpFontSize"), mDumpFontSize);
	// 未使用データを切り落とすか
	ini->Write(wxT("TrimUnusedData"), mTrimUnusedData);
	// 削除したファイルを表示するか
	ini->Write(wxT("ShowDeletedFile"), mShowDeletedFile);
	// エクスポート時に属性から拡張子を追加するか
	ini->Write(wxT("AddExtensionWhenExport"), mAddExtExport);
	// エクスポート時に現在日時を設定するか
	ini->Write(wxT("SetCurrentDateTimeWhenExport"), mCurrentDateExport);
	// インポート時に拡張子で属性を決定したら拡張子を削除するか
	ini->Write(wxT("DeleteExtensionWhenImport"), mDecideAttrImport);
	// インポートやプロパティ変更時に日時を無視するか
	ini->Write(wxT("IgnoreDateTime"), mIgnoreDateTime);
	// インポート時に現在日時を設定するか
	ini->Write(wxT("SetCurrentDateTimeWhenImport"), mCurrentDateImport);
	// プロパティで内部データをリストで表示するか
	ini->Write(wxT("ShowInterDirItem"), mShowInterDirItem);
	// オープン時サイド番号をチェックするか
	ini->Write(wxT("CheckSideNumber"), mCheckSideNumber);
	// 一度に処理できるディレクトリの深さ
	ini->Write(wxT("DirectoriesDepth"), mDirDepth);
	// ウィンドウ幅
	ini->Write(wxT("WindowWidth"), mWindowWidth);
	// ウィンドウ高さ
	ini->Write(wxT("WindowHeight"), mWindowHeight);
	// テンポラリフォルダのパス
	ini->Write(wxT("TemporaryFolder"), mTemporaryFolder);
	// バイナリエディタのパス
	ini->Write(wxT("BinaryEditor"), mBinaryEditor);
	ini->DeleteEntry(wxT("BinaryEditer"));
	// テキストエディタのパス
	ini->Write(wxT("TextEditor"), mTextEditor);
	// 言語
	ini->Write(wxT("Language"), mLanguage);
	// ファイルリストのカラム
	mFileColumn.Save(ini, wxT("ListColumn"));
	// 左パネル（ツリー）の幅
	ini->Write(wxT("LeftPanelWidth"), mLPanelWidth);
	// トラックパネルの幅
	ini->Write(wxT("TrackPanelWidth"), mTrkPanelWidth);

	// write
	delete ini;
}

int Config::FromPercentage(int percent, int base)
{
	if (percent < 5) percent = 5;
	else if (percent > 95) percent = 95;
	int num = percent * base / 100;
	return num;
}

int Config::ToPercentage(int num, int base)
{
	if (base <= 0) base = 1;
	int percent = num * 100 / base;
	if (percent < 5) percent = 5;
	else if (percent > 95) percent = 95;
	return percent;
}
