/// @file config.cpp
///
/// @brief 設定ファイル入出力
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

	ini->Read(wxT("Path"), &mFilePath, mFilePath);
	ini->Read(wxT("ExportPath"), &mExportFilePath, mExportFilePath);
	for(int i=0; i<MAX_RECENT_FILES; i++) {
		wxString sval;
		ini->Read(wxString::Format(wxT("Recent%d"), i), &sval);
		if (!sval.IsEmpty()) {
			mRecentFiles.Add(sval);
		}
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

	ini->Write(wxT("Path"), mFilePath);
	ini->Write(wxT("ExportPath"), mExportFilePath);
	for(int i=0,row=0; row<MAX_RECENT_FILES && i<(int)mRecentFiles.Count(); i++) {
		wxString sval = mRecentFiles.Item(i);
		if (sval.IsEmpty()) continue;
		ini->Write(wxString::Format(wxT("Recent%d"), row), sval);
		row++;
	}

	// write
	delete ini;
}
