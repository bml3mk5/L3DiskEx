/// @file config.h
///
/// @brief 設定ファイル入出力
///
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "common.h"
#include <wx/string.h>
#include <wx/arrstr.h>

#define MAX_RECENT_FILES 20

/// 設定ファイルパラメータ
class Params
{
protected:
	wxString	mFilePath;			///< ファイルパス
	wxString	mExportFilePath;	///< エクスポート先パス
	wxArrayString mRecentFiles;		///< 最近使用したファイル

public:
	Params();
	virtual ~Params() {}

	/// @name properties
	//@{
	void			SetFilePath(const wxString &val);
	const wxString &GetFilePath() const { return mFilePath; }
	void			SetExportFilePath(const wxString &val);
	const wxString &GetExportFilePath() const;
	void			AddRecentFile(const wxString &val);
	const wxString &GetRecentFile() const;
	const wxArrayString &GetRecentFiles() const;
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
