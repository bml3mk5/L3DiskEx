﻿/// @file config.h
///
/// @brief 設定ファイル入出力
///
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common.h"
#include <wx/wx.h>

/// 設定ファイルパラメータ
class Params
{
protected:
	wxString mFilePath;
	wxArrayString mRecentFiles;

public:
	Params();
	virtual ~Params() {}

	/// @name properties
	//@{
	void SetFilePath(const wxString &val);
	wxString &GetFilePath() { return mFilePath; }
	void AddRecentFile(const wxString &val);
	wxString &GetRecentFile();
	void GetRecentFiles(wxArrayString &vals);
	//@}
};

/// 設定ファイル入出力
class Config : public Params
{
private:
	wxString ini_file;

public:
	Config();
	~Config();
	void SetFileName(const wxString &file);
	void Load(const wxString &file);
	void Load();
	void Save();
};

#endif /* _CONFIG_H_ */