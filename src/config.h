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
#include "ui/uifilelist.h"


#define MAX_RECENT_FILES 20

class wxFileConfig;

/// カラム設定テンプレート
class ColumnParams
{
protected:
	wxArrayInt		mWidth;	///< リストの各カラムの幅
	wxArrayInt		mPos;	///< リストの各カラムの位置
	int				mStart;
	int				mEnd;
	const struct st_list_columns *pList;

public:
	ColumnParams(const struct st_list_columns *list, int pos_start, int pos_end);
	virtual ~ColumnParams() {}

	virtual int		Count() const { return (int)mWidth.Count(); }
	virtual void	SetWidth(int id, int val) { mWidth[id] = val; }
	virtual int		GetWidth(int id) const { return mWidth[id]; }
	virtual void	SetPos(int id, int val) { mPos[id] = val; }
	virtual int		GetPos(int id) const { return mPos[id]; }
	virtual void	Load(wxFileConfig *ini, const wxString &prefix);
	virtual void	Save(wxFileConfig *ini, const wxString &prefix);
};

/// ファイルリストのカラム設定
class FileColumnParams : public ColumnParams
{
public:
	FileColumnParams();
};

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
	bool		mCurrentDateExport;	///< エクスポート時に現在日時を設定するか
	bool		mDecideAttrImport;	///< インポート時に拡張子で属性を決定したら拡張子を削除するか
	bool		mSkipImportDialog;	///< インポートダイアログを抑制するか
	bool		mIgnoreDateTime;	///< インポートやプロパティ変更時に日時を無視するか
	bool		mCurrentDateImport;	///< インポート時に現在日時を設定するか
	bool		mShowInterDirItem;	///< プロパティで内部データをリストで表示するか
	bool		mCheckSideNumber;	///< オープン時サイド番号をチェックするか
	int			mDirDepth;			///< 一度に処理できるディレクトリの深さ
	int			mWindowWidth;		///< ウィンドウ幅
	int			mWindowHeight;		///< ウィンドウ高さ
	wxString	mTemporaryFolder;	///< テンポラリフォルダのパス
	wxString	mBinaryEditor;		///< バイナリエディタのパス
	wxString	mTextEditor;		///< テキストエディタのパス
	wxString	mLanguage;			///< 言語
	FileColumnParams mFileColumn;	///< ファイルリストの各カラムの設定
	int			mLPanelWidth;		///< 左パネル（ツリー）の幅
	int			mTrkPanelWidth;		///< トラックパネルの幅

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
	void			SetCurrentDateExport(bool val) { mCurrentDateExport = val; }
	bool			IsSetCurrentDateExport() const { return mCurrentDateExport; }
	void			DecideAttrImport(bool val) { mDecideAttrImport = val; }
	bool			IsDecideAttrImport() const { return mDecideAttrImport; }
	void			SkipImportDialog(bool val) { mSkipImportDialog = val; }
	bool			IsSkipImportDialog() const { return mSkipImportDialog; }
	void			IgnoreDateTime(bool val) { mIgnoreDateTime = val; }
	bool			DoesIgnoreDateTime() const { return mIgnoreDateTime; }
	void			SetCurrentDateImport(bool val) { mCurrentDateImport = val; }
	bool			IsSetCurrentDateImport() const { return mCurrentDateImport; }
	void			ShowInterDirItem(bool val) { mShowInterDirItem = val; }
	bool			DoesShowInterDirItem() const { return mShowInterDirItem; }
	void			CheckSideNumber(bool val) { mCheckSideNumber = val; }
	bool			DoesCheckSideNumber() const { return mCheckSideNumber; }
	void			SetDirDepth(int val) { mDirDepth = val; }
	int				GetDirDepth() const { return mDirDepth; }
	void			SetWindowWidth(int val) { mWindowWidth = val; }
	int				GetWindowWidth() const { return mWindowWidth; }
	void			SetWindowHeight(int val) { mWindowHeight = val; }
	int				GetWindowHeight() const { return mWindowHeight; }
	void			SetTemporaryFolder(const wxString &val);
	const wxString &GetTemporaryFolder() const { return mTemporaryFolder; }
	void			ClearTemporaryFolder() { mTemporaryFolder.Empty(); }
	void			SetBinaryEditor(const wxString &val);
	const wxString &GetBinaryEditor() const { return mBinaryEditor; }
	void			SetTextEditor(const wxString &val);
	const wxString &GetTextEditor() const { return mTextEditor; }
	void			SetLanguage(const wxString &val) { mLanguage = val; }
	const wxString &GetLanguage() const { return mLanguage; }
	FileColumnParams *GetFileColumnParams() { return &mFileColumn; }
	void			SetLPanelWidth(int val) { mLPanelWidth = val; }
	int				GetLPanelWidth() const { return mLPanelWidth; }
	void			SetTrackPanelWidth(int val) { mTrkPanelWidth = val; }
	int				GetTrackPanelWidth() const { return mTrkPanelWidth; }
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

	static int FromPercentage(int percent, int base);
	static int ToPercentage(int num, int base);
};

extern Config gConfig;

#endif /* _CONFIG_H_ */
