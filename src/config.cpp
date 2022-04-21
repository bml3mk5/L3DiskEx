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
	mDecideAttrImport = true;
	mSkipImportDialog = false;
	mIgnoreDateTime = false;
#ifdef _DEBUG
	mShowInterDirItem = true;
#else
	mShowInterDirItem = false;
#endif
	mWindowWidth = 1000;
	mWindowHeight = 600;
	mTemporaryFolder.Empty();
	mBinaryEditer.Empty();
	mLanguage.Empty();
	for(int id=LISTCOL_NAME; id<LISTCOL_END; id++) {
		mListColumnWidth[id]=-1;
		mListColumnPos[id]=id;
	}
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
	// 10を超える分は消す
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

void Params::SetBinaryEditer(const wxString &val)
{
	mBinaryEditer = wxFileName::FileName(val).GetFullPath();
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
	// インポート時に拡張子で属性を決定したら拡張子を削除するか
	ini->Read(wxT("DeleteExtensionWhenImport"), &mDecideAttrImport);
	// インポートやプロパティ変更時に日時を無視するか
	ini->Read(wxT("IgnoreDateTime"), &mIgnoreDateTime);
	// プロパティで内部データをリストで表示するか
	ini->Read(wxT("ShowInterDirItem"), &mShowInterDirItem);
	// ウィンドウ幅
	ini->Read(wxT("WindowWidth"), &mWindowWidth);
	// ウィンドウ高さ
	ini->Read(wxT("WindowHeight"), &mWindowHeight);
	// テンポラリフォルダのパス
	ini->Read(wxT("TemporaryFolder"), &mTemporaryFolder);
	// バイナリエディタのパス
	ini->Read(wxT("BinaryEditer"), &mBinaryEditer);
	// 言語
	ini->Read(wxT("Language"), &mLanguage);
	// リストのカラム幅
	for(int id=LISTCOL_NAME; id<LISTCOL_END; id++) {
		wxString key = wxT("ListColumn");
		key += gL3DiskFileListColumnDefs[id].name;
		key += wxT("Width");
		ini->Read(key, &mListColumnWidth[id]);
	}
	// リストのカラム位置
	for(int id=LISTCOL_NAME; id<LISTCOL_END; id++) {
		wxString key = wxT("ListColumn");
		key += gL3DiskFileListColumnDefs[id].name;
		key += wxT("Pos");
		ini->Read(key, &mListColumnPos[id]);
	}

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
	// インポート時に拡張子で属性を決定したら拡張子を削除するか
	ini->Write(wxT("DeleteExtensionWhenImport"), mDecideAttrImport);
	// インポートやプロパティ変更時に日時を無視するか
	ini->Write(wxT("IgnoreDateTime"), mIgnoreDateTime);
	// プロパティで内部データをリストで表示するか
	ini->Write(wxT("ShowInterDirItem"), mShowInterDirItem);
	// ウィンドウ幅
	ini->Write(wxT("WindowWidth"), mWindowWidth);
	// ウィンドウ高さ
	ini->Write(wxT("WindowHeight"), mWindowHeight);
	// テンポラリフォルダのパス
	ini->Write(wxT("TemporaryFolder"), mTemporaryFolder);
	// バイナリエディタのパス
	ini->Write(wxT("BinaryEditer"), mBinaryEditer);
	// 言語
	ini->Write(wxT("Language"), mLanguage);
	// リストのカラム幅
	for(int id=LISTCOL_NAME; id<LISTCOL_END; id++) {
		wxString key = wxT("ListColumn");
		key += gL3DiskFileListColumnDefs[id].name;
		key += wxT("Width");
		ini->Write(key, mListColumnWidth[id]);
	}
	// リストのカラム位置
	for(int id=LISTCOL_NAME; id<LISTCOL_END; id++) {
		wxString key = wxT("ListColumn");
		key += gL3DiskFileListColumnDefs[id].name;
		key += wxT("Pos");
		ini->Write(key, mListColumnPos[id]);
	}

	// write
	delete ini;
}
