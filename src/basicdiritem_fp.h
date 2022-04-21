/// @file basicdiritem_fp.h
///
/// @brief disk basic directory item for C82-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FP_H_
#define _BASICDIRITEM_FP_H_

#include "basicdiritem_fat8.h"


// Casio FP-1100 C82-BASIC

/// C82-BASICタイプ変換
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

/// ディレクトリ１アイテム Casio FP-1100 C82-BASIC
class DiskBasicDirItemFP : public DiskBasicDirItemFAT8
{
private:
	DiskBasicDirItemFP() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemFP(const DiskBasicDirItemFP &src) : DiskBasicDirItemFAT8(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 属性２を返す
	int		GetFileType2() const;
	/// @brief 属性２のセット
	void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief エントリデータの未使用部分を設定
	void	SetTerminate(int val);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType1Pos() const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType2Pos() const;
//	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
//	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief ファイル内部のアドレスを取り出す
	void	TakeAddressesInFile() {}

public:
	DiskBasicDirItemFP(DiskBasic *basic);
	DiskBasicDirItemFP(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemFP(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// @brief 削除
	bool	Delete(wxUint8 code);

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

	/// @name 開始アドレス、実行アドレス
	//@{
	/// @brief アイテムがアドレスを持っているか
	bool	HasAddress() const { return true; }
	/// @brief アイテムが実行アドレスを持っているか
	bool	HasExecuteAddress() const { return HasAddress(); }
	/// @brief 開始アドレスを返す
	int		GetStartAddress() const;
	/// @brief 終了アドレスを返す
	int		GetEndAddress() const;
	/// @brief 実行アドレスを返す
	int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	void	SetStartAddress(int val);
	/// @brief 終了アドレスをセット
	void	SetEndAddress(int val);
	/// @brief 実行アドレスをセット
	void	SetExecuteAddress(int val);
	//@}

	/// @brief ディレクトリアイテムのサイズ
	size_t	GetDataSize() const;

	/// @brief ファイルの終端コードをチェックする必要があるか
	bool	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ディレクトリをクリア ファイル新規作成時
	void	ClearData();

	/// @brief インポート時などのダイアログを出す前にファイルパスから内部ファイル名を生成する
	bool	PreImportDataFile(wxString &filename);
//	/// @brief ファイル名から属性を決定する
//	int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログ用に終了アドレスを返す
	int		GetEndAddressInAttrDialog(IntNameBox *parent);
	//@}
};

#endif /* _BASICDIRITEM_FP_H_ */
