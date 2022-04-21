/// @file basicdiritem_hu68k.h
///
/// @brief disk basic directory item for Human68k
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_HU68K_H_
#define _BASICDIRITEM_HU68K_H_

#include "basicdiritem_msdos.h"


/// ディレクトリ１アイテム Human68k
class DiskBasicDirItemHU68K : public DiskBasicDirItemMSDOS
{
protected:
	DiskBasicDirItemHU68K() : DiskBasicDirItemMSDOS() {}
	DiskBasicDirItemHU68K(const DiskBasicDirItemHU68K &src) : DiskBasicDirItemMSDOS(src) {}

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
//	/// @brief ファイル名を設定
//	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
//	/// @brief ファイル名を得る
//	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
//	/// @brief 属性１を返す
//	virtual int		GetFileType1() const;
//	/// @brief 属性１のセット
//	virtual void	SetFileType1(int val);
//	/// @brief 使用しているアイテムか
//	virtual bool	CheckUsed(bool unuse);
//	/// @brief ファイル名と拡張子を得る
//	virtual void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

//	/// @brief 日付を変換
//	static void		ConvDateToTm(wxUint16 date, struct tm *tm);
//	/// @brief 時間を変換
//	static void		ConvTimeToTm(wxUint16 time, struct tm *tm);
//	/// @brief 日付に変換
//	static wxUint16	ConvTmToDate(const struct tm *tm);
//	/// @brief 時間に変換
//	static wxUint16	ConvTmToTime(const struct tm *tm);

	/// @brief ダイアログ表示前にファイルの属性を設定
	virtual void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemHU68K(DiskBasic *basic);
	DiskBasicDirItemHU68K(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemHU68K(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

//	/// @brief ディレクトリアイテムのチェック
//	virtual bool	Check(bool &last);
//	/// @brief アイテムを削除できるか
//	virtual bool	IsDeletable() const;
//	/// @brief ファイル名を編集できるか
//	virtual bool	IsFileNameEditable() const;
//	/// @brief ファイル名に設定できない文字を文字列にして返す
//	virtual wxString GetDefaultInvalidateChars() const;
//	/// @brief ファイル名は必須（空文字不可）か
//	virtual bool	IsFileNameRequired() const { return true; }
//	/// @brief 属性を設定
//	virtual void	SetFileAttr(const DiskBasicFileType &file_type);

//	/// @brief 属性を返す
//	virtual DiskBasicFileType GetFileAttr() const;

//	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
//	virtual wxString GetFileAttrStr() const;
//	/// @brief ファイルサイズをセット
//	virtual void	SetFileSize(int val);
//	/// @brief ファイルサイズを返す
//	virtual int		GetFileSize() const;
//	/// @brief ファイルサイズとグループ数を計算する
//	virtual void	CalcFileUnitSize(int fileunit_num);

//	/// @brief 指定ディレクトリのすべてのグループを取得
//	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

//	/// @brief 最初のグループ番号をセット
//	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
//	/// @brief 最初のグループ番号を返す
//	virtual wxUint32 GetStartGroup(int fileunit_num) const;

//	/// @brief アイテムが日時を持っているか
//	virtual bool 	HasDateTime() const { return true; }
//	virtual bool 	HasDate() const { return true; }
//	virtual bool 	HasTime() const { return true; }
//	/// @brief 日付を返す
//	virtual void	GetFileDate(struct tm *tm) const;
//	/// @brief 時間を返す
//	virtual void	GetFileTime(struct tm *tm) const;
//	/// @brief 日付を返す
//	virtual wxString GetFileDateStr() const;
//	/// @brief 時間を返す
//	virtual wxString GetFileTimeStr() const;
//	/// @brief 日付をセット
//	virtual void	SetFileDate(const struct tm *tm);
//	/// @brief 時間をセット
//	virtual void	SetFileTime(const struct tm *tm);
//	/// @brief 日付のタイトル名（ダイアログ用）
//	virtual wxString GetFileDateTimeTitle() const;

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;

	/// @brief ダイアログ入力前のファイル名を変換 大文字にする
	virtual void	ConvertToFileNameStr(wxString &filename) const;
	/// @brief ダイアログ入力後のファイル名文字列を変換 大文字にする
	virtual void	ConvertFromFileNameStr(wxString &filename) const;


	/// @name プロパティダイアログ用
	//@{
//	/// @brief ダイアログ内の属性部分のレイアウトを作成
//	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
//	/// @brief 機種依存の属性を設定する
//	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	//@}
};

#endif /* _BASICDIRITEM_HU68K_H_ */
