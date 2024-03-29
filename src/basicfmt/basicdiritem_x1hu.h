/// @file basicdiritem_x1hu.h
///
/// @brief disk basic directory item for X1 Hu-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_X1HU_H
#define BASICDIRITEM_X1HU_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

/// @brief X1 Hu-BASIC 属性1位置
enum en_type_name_x1hu_1 {
	TYPE_NAME_X1HU_BINARY = 0,
	TYPE_NAME_X1HU_BASIC,
	TYPE_NAME_X1HU_ASCII,
	TYPE_NAME_X1HU_SWORD,
	TYPE_NAME_X1HU_RANDOM,
	TYPE_NAME_X1HU_DIRECTORY,
	TYPE_NAME_X1HU_END
};
/// @brief X1 Hu-BASIC 属性1値
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

/// @brief X1 Hu-BASIC 属性2位置
enum en_type_name_x1hu_2 {
	TYPE_NAME_X1HU_HIDDEN = 0,
	TYPE_NAME_X1HU_READ_WRITE,
	TYPE_NAME_X1HU_READ_ONLY,
	TYPE_NAME_X1HU_PASSWORD
};
/// @brief X1 Hu-BASIC 属性2値
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

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム X1 Hu-BASIC
///
/// @li DefaultAsciiType アスキーファイル(Hu-BASIC or S-OS)
class DiskBasicDirItemX1HU : public DiskBasicDirItem
{
private:
	DiskBasicDirItemX1HU() : DiskBasicDirItem() {}
	DiskBasicDirItemX1HU(const DiskBasicDirItemX1HU &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_x1_hu_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 属性を変換
	int				ConvToNativeType(int file_type, int val) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType1Pos(int native_type) const;
	/// @brief 属性1を得る
	int				GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief 属性2を得る
	int				GetFileType2InAttrDialog(const IntNameBox *parent) const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int				CalcFileTypeFromPos(int pos) const;

	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);

public:
	DiskBasicDirItemX1HU(DiskBasic *basic);
	DiskBasicDirItemX1HU(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemX1HU(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief 削除
	virtual bool	Delete();

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
	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);

	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief アイテムが日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	virtual bool	HasCreateDate() const { return true; }
	virtual bool	HasCreateTime() const { return true; }
	/// @brief アイテムの時間設定を無視することができるか
	virtual enDateTime CanIgnoreDateTime() const { return DATETIME_ALL; }
	/// @brief 日付を返す
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 時間を返す
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 時間を返す
	virtual wxString GetFileCreateTimeStr() const;
	/// @brief 日付をセット
	virtual void	SetFileCreateDate(const TM &tm);
	/// @brief 時間をセット
	virtual void	SetFileCreateTime(const TM &tm);

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }
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

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
	/// @brief ファイルの終端コードを返す
	virtual wxUint8 GetEofCode() const;
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;


	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief ダイアログ内の値を設定
	virtual void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* BASICDIRITEM_X1HU_H */
