/// @file config.h
///
/// @brief 設定ファイル入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
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
	int			mCharCode;			///< キャラクターコードマップ番号
	wxString	mListFontName;		///< リストウィンドウのフォント名
	int			mListFontSize;		///< リストウィンドウのフォントサイズ
	wxString	mDumpFontName;		///< ダンプウィンドウのフォント名
	int			mDumpFontSize;		///< ダンプウィンドウのフォントサイズ
	bool		mTrimUnusedData;	///< 未使用データを切り落とすか

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
	void			SetCharCode(int val) { mCharCode = val; }
	int				GetCharCode() const { return mCharCode; }
	void			SetListFontName(const wxString &val) { mListFontName = val; }
	const wxString &GetListFontName() const { return mListFontName; }
	void			SetListFontSize(int val) { mListFontSize = val; }
	int				GetListFontSize() const { return mListFontSize; }
	void			SetDumpFontName(const wxString &val) { mDumpFontName = val; }
	const wxString &GetDumpFontName() const { return mDumpFontName; }
	void			SetDumpFontSize(int val) { mDumpFontSize = val; }
	int				GetDumpFontSize() const { return mDumpFontSize; }
	void			TrimUnusedData(bool val) { mTrimUnusedData = val; }
	bool			IsTrimUnusedData() const { return mTrimUnusedData; }
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
