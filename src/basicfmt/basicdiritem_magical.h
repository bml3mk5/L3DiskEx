/// @file basicdiritem_magical.h
///
/// @brief disk basic directory item for Magical DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_MAGICAL_H_
#define _BASICDIRITEM_MAGICAL_H_

#include "basicdiritem_xdos.h"


//////////////////////////////////////////////////////////////////////

// Magical DOS

#define MAGICAL_GROUP_SIZE	512

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

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Magical DOS
class DiskBasicDirItemMAGICAL : public DiskBasicDirItemXDOSBase
{
private:
	DiskBasicDirItemMAGICAL() : DiskBasicDirItemXDOSBase() {}
	DiskBasicDirItemMAGICAL(const DiskBasicDirItemMAGICAL &src) : DiskBasicDirItemXDOSBase(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_magical_t> m_data;

	DirItemSectorBoundary m_sdata;	///< セクタ内部へのポインタ

	/// @brief ディレクトリエントリを確保
	bool			AllocateItem(const SectorParam *next);

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int	GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性１の文字列
	wxString ConvFileType1Str(int t1) const;
	/// @brief 属性２を返す
	virtual int	GetFileType2() const;
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名と拡張子を得る
	virtual void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int		ConvFileType1Pos(int t1) const;

	/// @brief 日付を変換
	static void		ConvDateToTm(wxUint16 date, TM &tm);
	/// @brief 時間を変換
	static void		ConvTimeToTm(wxUint16 time, TM &tm);
	/// @brief 日付に変換
	static wxUint16	ConvTmToDate(const TM &tm);
	/// @brief 時間に変換
	static wxUint16	ConvTmToTime(const TM &tm);

public:
	DiskBasicDirItemMAGICAL(DiskBasic *basic);
	DiskBasicDirItemMAGICAL(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemMAGICAL(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief 削除
	virtual bool	Delete();

	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// @brief ファイルサイズを返す
	virtual int		GetFileSize() const;

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 追加のグループ番号をセット チェインセクタへセット
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す チェインセクタを返す
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る
	virtual void	GetExtraGroups(wxArrayInt &arr) const;

	/// @brief 日付を得る
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 時間を得る
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 日付をセット
	virtual void	SetFileCreateDate(const TM &tm);
	/// @brief 時間をセット
	virtual void	SetFileCreateTime(const TM &tm);
	/// @brief 日付のタイトル名（ダイアログ用）
	virtual wxString GetFileCreateDateTimeTitle() const;

	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();
	/// @brief ディレクトリを初期化 未使用にする
	virtual void	InitialData();

	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	virtual bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @brief アイテムの属するセクタを変更済みにする
	virtual void	SetModify();

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent) {}
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_MAGICAL_H_ */
