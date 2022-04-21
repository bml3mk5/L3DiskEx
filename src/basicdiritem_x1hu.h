/// @file basicdiritem_x1hu.h
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
	TYPE_NAME_X1HU_BASIC,
	TYPE_NAME_X1HU_ASCII,
	TYPE_NAME_X1HU_SWORD,
	TYPE_NAME_X1HU_RANDOM,
	TYPE_NAME_X1HU_DIRECTORY,
	TYPE_NAME_X1HU_END
};
enum en_file_type_mask_x1hu {
	FILETYPE_X1HU_BINARY = 0x01,
	FILETYPE_X1HU_BASIC = 0x02,
	FILETYPE_X1HU_ASCII = 0x04,
	FILETYPE_X1HU_DIRECTORY = 0x80,
};
#define FILETYPE_X1HU_MASK (FILETYPE_X1HU_BINARY | FILETYPE_X1HU_BASIC | FILETYPE_X1HU_ASCII | FILETYPE_X1HU_DIRECTORY)

enum en_external_type_x1 {
	EXTERNAL_X1_DEFAULT = 0,
	EXTERNAL_X1_RANDOM,
	EXTERNAL_X1_SWORD
};

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

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性２を返す
	int		GetFileType2() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 属性２のセット
	void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief 属性を変換
	int		ConvToNativeType(int file_type, int val) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int		GetFileType1Pos(int native_type) const;
//	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
//	int		GetFileType1Pos() const;
//	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
//	int		GetFileType2Pos() const;
	/// @brief 属性1を得る
	int		GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief 属性2を得る
	int		GetFileType2InAttrDialog(const IntNameBox *parent) const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int		CalcFileTypeFromPos(int pos) const;

	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);

public:
	DiskBasicDirItemX1HU(DiskBasic *basic);
	DiskBasicDirItemX1HU(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemX1HU(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

//	/// @brief ファイル名が一致するか
//	bool	IsSameFileName(const DiskBasicFileName &filename) const;

	/// @brief 属性を設定
	void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief ディレクトリを初期化 未使用にする
	void	InitialData();

	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	void	SetFileSize(int val);
	/// @brief ファイルサイズを返す
	int		GetFileSize() const;
	/// @brief ファイルサイズとグループ数を計算する
	void	CalcFileUnitSize(int fileunit_num);

	/// @brief 指定ディレクトリのすべてのグループを取得
	void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief 最初のグループ番号をセット
	void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief アイテムが日時を持っているか
	bool	HasDateTime() const { return true; }
	bool	HasDate() const { return true; }
	bool	HasTime() const { return true; }
	/// @brief アイテムの時間設定を無視することができるか
	bool	CanIgnoreDateTime() const { return true; }
	/// @brief 日付を返す
	void	GetFileDate(struct tm *tm) const;
	/// @brief 時間を返す
	void	GetFileTime(struct tm *tm) const;
	/// @brief 日付を返す
	wxString GetFileDateStr() const;
	/// @brief 時間を返す
	wxString GetFileTimeStr() const;
	/// @brief 日付をセット
	void	SetFileDate(const struct tm *tm);
	/// @brief 時間をセット
	void	SetFileTime(const struct tm *tm);

	/// @brief アイテムがアドレスを持っているか
	bool	HasAddress() const { return true; }
	/// @brief 開始アドレスを返す
	int		GetStartAddress() const;
	/// @brief 実行アドレスを返す
	int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	void	SetStartAddress(int val);
	/// @brief 実行アドレスをセット
	void	SetExecuteAddress(int val);

	/// @brief ディレクトリアイテムのサイズ
	size_t	GetDataSize() const;

	/// @brief ファイルの終端コードをチェックする必要があるか
	bool	NeedCheckEofCode();
	/// @brief ファイルの終端コードを返す
	wxUint8	GetEofCode() const;
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

//	/// @brief データをエクスポートする前に必要な処理
//	bool	PreExportDataFile(wxString &filename);
//	/// @brief データをインポートする前に必要な処理
//	bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	int		ConvOriginalTypeFromFileName(const wxString &filename) const;


	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief ダイアログ内の値を設定
	void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
	/// @brief 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	//@}
};

#endif /* _BASICDIRITEM_X1HU_H_ */
