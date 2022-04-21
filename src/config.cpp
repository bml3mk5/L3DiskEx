/// @file config.cpp
///
/// @brief 設定ファイル入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "config.h"
#include <wx/filename.h>
#include <wx/fileconf.h>


Params::Params()
{
	// default value
	mFilePath = wxT("");
	mExportFilePath = wxT("");
	mRecentFiles.Empty();
	mCharCode = 0;
	mListFontName.Empty();
	mListFontSize = 0;
	mDumpFontName.Empty();
	mDumpFontSize = 0;
	mTrimUnusedData = true;
	mShowDeletedFile = false;
	mWindowWidth = 1000;
	mWindowHeight = 600;
}

void Params::SetFilePath(const wxString &val)
{
	mFilePath = wxFileName::FileName(val).GetPath();
}

void Params::SetExportFilePath(const wxString &val)
{
	mExportFilePath = wxFileName::FileName(val).GetPath();
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
	// キャラクターコードマップ番号
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
	// ウィンドウ幅
	ini->Read(wxT("WindowWidth"), &mWindowWidth);
	// ウィンドウ高さ
	ini->Read(wxT("WindowHeight"), &mWindowHeight);

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
	// キャラクターコードマップ番号
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
	// ウィンドウ幅
	ini->Write(wxT("WindowWidth"), mWindowWidth);
	// ウィンドウ高さ
	ini->Write(wxT("WindowHeight"), mWindowHeight);

	// write
	delete ini;
}
