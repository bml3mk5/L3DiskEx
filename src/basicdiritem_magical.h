/// @file basicdiritem_magical.h
///
/// @brief disk basic directory item for Magical DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_MAGICAL_H_
#define _BASICDIRITEM_MAGICAL_H_

#include "basicdiritem_xdos.h"


// Magical DOS

#define MAGICAL_GROUP_SIZE	512

extern const char *gTypeNameMAGICAL_1[];
enum en_type_name_magical_1 {
	TYPE_NAME_MAGICAL_SYS = 0,
	TYPE_NAME_MAGICAL_BAS,
	TYPE_NAME_MAGICAL_OBJ,
	TYPE_NAME_MAGICAL_ASC,
	TYPE_NAME_MAGICAL_DIR,
	TYPE_NAME_MAGICAL_CDT,
	TYPE_NAME_MAGICAL_PDT,
	TYPE_NAME_MAGICAL_GRA,
	TYPE_NAME_MAGICAL_GAK,
	TYPE_NAME_MAGICAL_SBA,
	TYPE_NAME_MAGICAL_SOB,
	TYPE_NAME_MAGICAL_REP,
	TYPE_NAME_MAGICAL_MDT,
	TYPE_NAME_MAGICAL_ARC,
	TYPE_NAME_MAGICAL_KTY,
	TYPE_NAME_MAGICAL_CGP,
	TYPE_NAME_MAGICAL_BGM,
	TYPE_NAME_MAGICAL_UNKNOWN,
};
enum en_file_type_magical {
	FILETYPE_MAGICAL_SYS = 0x01,
	FILETYPE_MAGICAL_BAS = 0x22,
	FILETYPE_MAGICAL_OBJ = 0x03,
	FILETYPE_MAGICAL_ASC = 0x44,
	FILETYPE_MAGICAL_DIR = 0x05,
	FILETYPE_MAGICAL_CDT = 0x06,
	FILETYPE_MAGICAL_PDT = 0x07,
	FILETYPE_MAGICAL_GRA = 0x08,
	FILETYPE_MAGICAL_GAK = 0x49,
	FILETYPE_MAGICAL_SBA = 0x2a,
	FILETYPE_MAGICAL_SOB = 0x0b,
	FILETYPE_MAGICAL_REP = 0x4c,
	FILETYPE_MAGICAL_MDT = 0x0d,
	FILETYPE_MAGICAL_ARC = 0x4e,
	FILETYPE_MAGICAL_KTY = 0x4f,
	FILETYPE_MAGICAL_CGP = 0x50,
	FILETYPE_MAGICAL_BGM = 0x51,
	FILETYPE_MAGICAL_UNKNOWN = 0x80,
};


extern const char *gTypeNameMAGICAL_2[];
enum en_type_name_magical_2 {
	TYPE_NAME_MAGICAL_READONLY,
	TYPE_NAME_MAGICAL_HIDDEN,
	TYPE_NAME_MAGICAL_SYSTEM,
	TYPE_NAME_MAGICAL_SUPER,
};
enum en_data_type_magical {
	DATATYPE_MAGICAL_MASK_b = 0x00,
	DATATYPE_MAGICAL_MASK_r = 0x01,
	DATATYPE_MAGICAL_MASK_g = 0x02,
	DATATYPE_MAGICAL_MASK_m = 0x03,
	DATATYPE_MAGICAL_MASK_A = 0x04,
	DATATYPE_MAGICAL_MASK_B = 0x05,
	DATATYPE_MAGICAL_MASK_C = 0x06,
	DATATYPE_MAGICAL_MASK_D = 0x07,
	DATATYPE_MAGICAL_MASK_E = 0x08,
	DATATYPE_MAGICAL_MASK_F = 0x09,
	DATATYPE_MAGICAL_MASK_G = 0x0a,
	DATATYPE_MAGICAL_MASK_H = 0x0b,
	DATATYPE_MAGICAL_MASK_I = 0x0c,
	DATATYPE_MAGICAL_MASK_J = 0x0d,
	DATATYPE_MAGICAL_MASK_K = 0x0e,
	DATATYPE_MAGICAL_MASK_L = 0x0f,
	DATATYPE_MAGICAL_MASK_READONLY = 0x10,
	DATATYPE_MAGICAL_MASK_HIDDEN = 0x20,
	DATATYPE_MAGICAL_MASK_SYSTEM = 0x40,
	DATATYPE_MAGICAL_MASK_SUPER = 0x80
};

/// ディレクトリ１アイテム Magical DOS
class DiskBasicDirItemMAGICAL : public DiskBasicDirItemXDOS
{
private:
	DiskBasicDirItemMAGICAL() : DiskBasicDirItemXDOS() {}
	DiskBasicDirItemMAGICAL(const DiskBasicDirItemMAGICAL &src) : DiskBasicDirItemXDOS(src) {}

	wxUint8 *sdata[3];	///< セクタ内部へのポインタ (16bytes)

	/// @brief ディレクトリエントリを確保
	bool	AllocateItem();

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 属性１の文字列
	wxString ConvFileType1Str(int t1) const;
	/// @brief 属性２を返す
	int		GetFileType2() const;
	/// @brief 属性２のセット
	void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief ファイル名を設定
	void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名と拡張子を得る
	void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int		ConvFileType1Pos(int t1) const;

	/// @brief 日付を変換
	static void		ConvDateToTm(wxUint16 date, struct tm *tm);
	/// @brief 時間を変換
	static void		ConvTimeToTm(wxUint16 time, struct tm *tm);
	/// @brief 日付に変換
	static wxUint16	ConvTmToDate(const struct tm *tm);
	/// @brief 時間に変換
	static wxUint16	ConvTmToTime(const struct tm *tm);

public:
	DiskBasicDirItemMAGICAL(DiskBasic *basic);
	DiskBasicDirItemMAGICAL(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemMAGICAL(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief アイテムへのポインタを設定
	void	SetDataPtr(int n_num, int n_track, int n_side, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);
	/// @brief アイテムを削除できるか
	bool	IsDeletable() const;
	/// @brief 削除
	bool	Delete(wxUint8 code);

	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief 属性を設定
	void	SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	void	SetFileSize(int val);
	/// @brief ファイルサイズを返す
	int		GetFileSize() const;

	/// @brief 最初のグループ番号をセット
	void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 追加のグループ番号をセット チェインセクタへセット
	void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す チェインセクタを返す
	wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る
	void	GetExtraGroups(wxArrayInt &arr) const;

	/// @brief 日付を得る
	void	GetFileDate(struct tm *tm) const;
	/// @brief 時間を得る
	void	GetFileTime(struct tm *tm) const;
	/// @brief 日付をセット
	void	SetFileDate(const struct tm *tm);
	/// @brief 時間をセット
	void	SetFileTime(const struct tm *tm);
	/// @brief 日付のタイトル名（ダイアログ用）
	wxString GetFileDateTimeTitle() const;

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

	/// @brief アイテムをコピー
	bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	void	ClearData();
	/// @brief ディレクトリを初期化 未使用にする
	void	InitialData();

	/// @brief データをエクスポートする前に必要な処理
	bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	int		ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent) {}
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	//@}
};

#endif /* _BASICDIRITEM_MAGICAL_H_ */
