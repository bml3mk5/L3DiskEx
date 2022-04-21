﻿/// @file basicdiritem_x1hu.h
///
/// @brief disk basic directory item for X1 Hu-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_X1HU_H_
#define _BASICDIRITEM_X1HU_H_

#include "basicdiritem.h"


/// X1 Hu-BASIC 属性名1
extern const char *gTypeNameX1HU_1[];
enum en_type_name_x1hu_1 {
	TYPE_NAME_X1HU_BINARY = 0,
	TYPE_NAME_X1HU_BASIC = 1,
	TYPE_NAME_X1HU_ASCII = 2,
	TYPE_NAME_X1HU_DIRECTORY = 3,
	TYPE_NAME_X1HU_RANDOM = 4,
};
enum en_file_type_mask_x1hu {
	FILETYPE_X1HU_BINARY = 0x01,
	FILETYPE_X1HU_BASIC = 0x02,
	FILETYPE_X1HU_ASCII = 0x04,
	FILETYPE_X1HU_DIRECTORY = 0x80,
};
#define FILETYPE_X1HU_MASK (FILETYPE_X1HU_BINARY | FILETYPE_X1HU_BASIC | FILETYPE_X1HU_ASCII | FILETYPE_X1HU_DIRECTORY)

/// X1 Hu-BASIC 属性名2
extern const char *gTypeNameX1HU_2[];
enum en_type_name_x1hu_2 {
	TYPE_NAME_X1HU_HIDDEN = 0,
	TYPE_NAME_X1HU_READ_WRITE = 1,
	TYPE_NAME_X1HU_READ_ONLY = 2,
	TYPE_NAME_X1HU_PASSWORD = 3
};
enum en_data_type_mask_x1hu {
	DATATYPE_X1HU_HIDDEN = 0x10,
	DATATYPE_X1HU_READ_WRITE = 0x20,
	DATATYPE_X1HU_READ_ONLY = 0x40,
	DATATYPE_X1HU_RESERVED = 0x08,
};
#define DATATYPE_X1HU_MASK (DATATYPE_X1HU_RESERVED | DATATYPE_X1HU_HIDDEN | DATATYPE_X1HU_READ_WRITE | DATATYPE_X1HU_READ_ONLY)

#define	DATATYPE_X1HU_PASSWORD_NONE 0x20
#define DATATYPE_X1HU_PASSWORD_MASK 0xff
//#define DATATYPE_X1HU_PASSWORD_POS  20

/// ディレクトリ１アイテム X1 Hu-BASIC
class DiskBasicDirItemX1HU : public DiskBasicDirItem
{
private:
	DiskBasicDirItemX1HU() : DiskBasicDirItem() {}
	DiskBasicDirItemX1HU(const DiskBasicDirItemX1HU &src) : DiskBasicDirItem(src) {}

	/// ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// ファイル名を格納するバッファサイズを返す
	int		GetFileNameSize(bool *invert = NULL) const; 
	/// 拡張子を格納するバッファサイズを返す
	int		GetFileExtSize(bool *invert = NULL) const;
	/// 属性１を返す
	int		GetFileType1() const;
	/// 属性２を返す
	int		GetFileType2() const;
	/// 属性１のセット
	void	SetFileType1(int val);
	/// 属性２のセット
	void	SetFileType2(int val);
	/// 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int		GetFileType1Pos() const;
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int		GetFileType2Pos() const;
	/// 属性1を得る
	int		GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// 属性2を得る
	int		GetFileType2InAttrDialog(const IntNameBox *parent) const;
	/// リストの位置から属性を返す(プロパティダイアログ用)
	int		CalcFileTypeFromPos(int pos);
	/// インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemX1HU(DiskBasic *basic);
	DiskBasicDirItemX1HU(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemX1HU(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool			Check(bool &last);

	/// ファイル名が一致するか
	bool			IsSameFileName(const DiskBasicFileName &filename) const;

	/// 属性を設定
	void			SetFileAttr(const DiskBasicFileType &file_type);
	/// ディレクトリを初期化 未使用にする
	void			InitialData();

	/// 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// 属性の文字列を返す(ファイル一覧画面表示用)
	wxString		GetFileAttrStr() const;

	/// ファイルサイズをセット
	void			SetFileSize(int val);
	/// ファイルサイズとグループ数を計算する
	void			CalcFileSize();

	/// 指定ディレクトリのすべてのグループを取得
	void			GetAllGroups(DiskBasicGroups &group_items);

	/// 最初のグループ番号をセット
	void			SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	wxUint32		GetStartGroup() const;

	/// アイテムが日時を持っているか
	bool			HasDateTime() const { return true; }
	bool			HasDate() const { return true; }
	bool			HasTime() const { return true; }
	/// アイテムの時間設定を無視することができるか
	bool			CanIgnoreDateTime() const { return true; }
	/// 日付を返す
	void			GetFileDate(struct tm *tm) const;
	/// 時間を返す
	void			GetFileTime(struct tm *tm) const;
	/// 日付を返す
	wxString		GetFileDateStr() const;
	/// 時間を返す
	wxString		GetFileTimeStr() const;
	/// 日付をセット
	void			SetFileDate(const struct tm *tm);
	/// 時間をセット
	void			SetFileTime(const struct tm *tm);

	/// アイテムがアドレスを持っているか
	bool			HasAddress() const { return true; }
	/// 開始アドレスを返す
	int				GetStartAddress() const;
	/// 実行アドレスを返す
	int				GetExecuteAddress() const;
	/// 開始アドレスをセット
	void			SetStartAddress(int val);
	/// 実行アドレスをセット
	void			SetExecuteAddress(int val);

	/// ディレクトリアイテムのサイズ
	size_t			GetDataSize() const;

	/// ファイルの終端コードをチェックする必要があるか
	bool			NeedCheckEofCode();
//	/// データをエクスポートする前に必要な処理
//	bool			PreExportDataFile(wxString &filename);
	/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	int				RecalcFileSizeOnSave(wxInputStream *istream, int file_size);


	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// ダイアログ内の値を設定
	void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
	/// 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
	/// ファイルサイズが適正か
	bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	//@}
};

#endif /* _BASICDIRITEM_X1HU_H_ */
