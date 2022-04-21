/// @file basicdiritem_n88.h
///
/// @brief disk basic directory item for N88-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_N88_H_
#define _BASICDIRITEM_N88_H_

#include "basicdiritem_fat8.h"


//////////////////////////////////////////////////////////////////////

// N88-BASIC

extern const char *gTypeNameN88_1[];
enum en_type_name_n88_1 {
	TYPE_NAME_N88_ASCII = 0,
	TYPE_NAME_N88_BINARY = 1,
	TYPE_NAME_N88_MACHINE = 2,
	TYPE_NAME_N88_RANDOM = 3,
};
enum en_file_type_n88 {
	FILETYPE_N88_ASCII = 0x00,
	FILETYPE_N88_BINARY = 0x80,
	FILETYPE_N88_MACHINE = 0x01
};

extern const char *gTypeNameN88_2[];
enum en_type_name_n88_2 {
	TYPE_NAME_N88_READ_ONLY = 0,
	TYPE_NAME_N88_READ_WRITE = 1,
	TYPE_NAME_N88_ENCRYPTED = 2
};
enum en_data_type_mask_n88 {
	DATATYPE_MASK_N88_READ_ONLY = 0x10,
	DATATYPE_MASK_N88_READ_WRITE = 0x40,
	DATATYPE_MASK_N88_ENCRYPTED = 0x20
};

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicDirItemN88

@brief ディレクトリ１アイテム N88-BASIC

@li m_external_attr : ランダムアクセスファイルの時 1

*/
class DiskBasicDirItemN88 : public DiskBasicDirItemFAT8
{
private:
	DiskBasicDirItemN88() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemN88(const DiskBasicDirItemN88 &src) : DiskBasicDirItemFAT8(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_n88_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性を変換
	int				ConvFileType1(int file_type) const;
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int			   ConvFileType1Pos(int t1) const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int				CalcFileTypeFromPos(int pos) const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief ファイル内部のアドレスを取り出す
	void			TakeAddressesInFile();

public:
	DiskBasicDirItemN88(DiskBasic *basic);
	DiskBasicDirItemN88(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemN88(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief 削除
	virtual bool	Delete();

	/// @brief ENDマークがあるか(一度も使用していないか)
	virtual bool	HasEndMark();
	/// @brief 次のアイテムにENDマークを入れる
	virtual void	SetEndMark(DiskBasicDirItem *next_item);

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;


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

	/// @brief アイテムが実行アドレスを持っているか
	virtual bool	HasExecuteAddress() const { return false; }

	/// @brief ファイル名から属性を決定する
	virtual int		ConvFileTypeFromFileName(const wxString &filename) const;
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_N88_H_ */
