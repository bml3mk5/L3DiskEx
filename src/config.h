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
#include "uifilelist.h"


#define MAX_RECENT_FILES 20

/// 設定ファイルパラメータ
class Params
{
protected:
	wxString	mFilePath;			///< ファイルパス
	wxString	mExportFilePath;	///< エクスポート先パス
	wxArrayString mRecentFiles;		///< 最近使用したファイル
	wxString	mCharCode;			///< キャラクターコード名
	wxString	mListFontName;		///< リストウィンドウのフォント名
	int			mListFontSize;		///< リストウィンドウのフォントサイズ
	wxString	mDumpFontName;		///< ダンプウィンドウのフォント名
	int			mDumpFontSize;		///< ダンプウィンドウのフォントサイズ
	bool		mTrimUnusedData;	///< 未使用データを切り落とすか
	bool		mShowDeletedFile;	///< 削除したファイルを表示するか
	bool		mAddExtExport;		///< エクスポート時に属性から拡張子を追加するか
	bool		mDecideAttrImport;	///< インポート時に拡張子で属性を決定したら拡張子を削除するか
	bool		mSkipImportDialog;	///< インポートダイアログを抑制するか
	bool		mIgnoreDateTime;	///< インポートやプロパティ変更時に日時を無視するか
	int			mWindowWidth;		///< ウィンドウ幅
	int			mWindowHeight;		///< ウィンドウ高さ
	wxString	mTemporaryFolder;	///< テンポラリフォルダのパス
	wxString	mBinaryEditer;		///< バイナリエディタのパス
	int			mListColumnWidth[LISTCOL_END];	///< ファイルリストの各カラムの幅
	int			mListColumnPos[LISTCOL_END];	///< ファイルリストの各カラムの位置

public:
	Params();
	virtual ~Params() {}

	/// @name properties
	//@{
	void			SetFilePath(const wxString &val);
	const wxString &GetFilePath() const { return mFilePath; }
	void			SetExportFilePath(const wxString &val, bool is_dir = false);
	const wxString &GetExportFilePath() const;
	void			AddRecentFile(const wxString &val);
	const wxString &GetRecentFile() const;
	const wxArrayString &GetRecentFiles() const;
	void			SetCharCode(const wxString &val) { mCharCode = val; }
	const wxString &GetCharCode() const { return mCharCode; }
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
	void			ShowDeletedFile(bool val) { mShowDeletedFile = val; }
	bool			IsShownDeletedFile() const { return mShowDeletedFile; }
	void			AddExtensionExport(bool val) { mAddExtExport = val; }
	bool			IsAddExtensionExport() const { return mAddExtExport; }
	void			DecideAttrImport(bool val) { mDecideAttrImport = val; }
	bool			IsDecideAttrImport() const { return mDecideAttrImport; }
	void			SkipImportDialog(bool val) { mSkipImportDialog = val; }
	bool			IsSkipImportDialog() const { return mSkipImportDialog; }
	void			IgnoreDateTime(bool val) { mIgnoreDateTime = val; }
	bool			DoesIgnoreDateTime() const { return mIgnoreDateTime; }
	void			SetWindowWidth(int val) { mWindowWidth = val; }
	int				GetWindowWidth() const { return mWindowWidth; }
	void			SetWindowHeight(int val) { mWindowHeight = val; }
	int				GetWindowHeight() const { return mWindowHeight; }
	void			SetTemporaryFolder(const wxString &val);
	const wxString &GetTemporaryFolder() const { return mTemporaryFolder; }
	void			ClearTemporaryFolder() { mTemporaryFolder.Empty(); }
	void			SetBinaryEditer(const wxString &val);
	const wxString &GetBinaryEditer() const { return mBinaryEditer; }
	void			SetListColumnWidth(int id, int val) { mListColumnWidth[id] = val; }
	int				GetListColumnWidth(int id) const { return mListColumnWidth[id]; }
	void			SetListColumnPos(int id, int val) { mListColumnPos[id] = val; }
	int				GetListColumnPos(int id) const { return mListColumnPos[id]; }
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

extern Config gConfig;

#endif /* _CONFIG_H_ */
