/// @file basicdiritem_fp.h
///
/// @brief disk basic directory item for C82-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FP_H_
#define _BASICDIRITEM_FP_H_

#include "basicdiritem_fat8.h"


//////////////////////////////////////////////////////////////////////

// Casio FP-1100 C82-BASIC

/// @brief C82-BASICタイプ変換
enum en_file_type_fp {
	FILETYPE_FP_BASIC = 0x10,	// BASIC
	FILETYPE_FP_DATA = 0x04,	// DATA
	FILETYPE_FP_MACHINE = 0x0d,	// MACHINE
	FILETYPE_FP_ASCII = 0x20,	// ascii
	FILETYPE_FP_RANDOM = 0x80,	// random
};

extern const char *gTypeNameFP_2[];
enum en_type_name_fp_2 {
	TYPE_NAME_FP_READ_ONLY = 0,
	TYPE_NAME_FP_READ_WRITE,
	TYPE_NAME_FP_UNKNOWN
};
enum en_data_type_mask_fp {
	DATATYPE_MASK_FP_READ_ONLY = 0xf0,
	DATATYPE_MASK_FP_READ_WRITE = 0x0f
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Casio FP-1100 C82-BASIC
class DiskBasicDirItemFP : public DiskBasicDirItemFAT8
{
private:
	DiskBasicDirItemFP() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemFP(const DiskBasicDirItemFP &src) : DiskBasicDirItemFAT8(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_fp_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２を返す
	virtual int	GetFileType2() const;
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief エントリデータの未使用部分を設定
	void			SetTerminate(int val);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType1Pos() const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType2Pos() const;

	/// @brief ファイル内部のアドレスを取り出す
	void			TakeAddressesInFile() {}

public:
	DiskBasicDirItemFP(DiskBasic *basic);
	DiskBasicDirItemFP(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemFP(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

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

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;

	/// @name 開始アドレス、実行アドレス
	//@{
	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }
	/// @brief アイテムが実行アドレスを持っているか
	virtual bool	HasExecuteAddress() const { return HasAddress(); }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 終了アドレスを返す
	virtual int		GetEndAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);
	/// @brief 終了アドレスをセット
	virtual void	SetEndAddress(int val);
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val);
	//@}

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief インポート時などのダイアログを出す前にファイルパスから内部ファイル名を生成する
	virtual bool	PreImportDataFile(wxString &filename);
	/// @brief 属性値を加工する
	virtual bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログ用に終了アドレスを返す
	virtual int		GetEndAddressInAttrDialog(IntNameBox *parent);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_FP_H_ */
