/// @file basicdiritem_fat8.h
///
/// @brief disk basic directory item
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FAT8_H_
#define _BASICDIRITEM_FAT8_H_

#include "basicdiritem.h"


/// L3 BASIC
/// F-BASIC
/// タイプ1 0...BASIC 1...DATA 2...MACHINE
extern const char *gTypeName1[];
enum en_type_name_1 {
	TYPE_NAME_1_BASIC	= 0,
	TYPE_NAME_1_DATA	= 1,
	TYPE_NAME_1_MACHINE	= 2,
	TYPE_NAME_1_UNKNOWN = 3
};

/// タイプ2 0...Binary 1...Ascii 2...Random Access
extern const char *gTypeName2[];
enum en_type_name_2 {
	TYPE_NAME_2_BINARY	= 0,
	TYPE_NAME_2_ASCII	= 1,
	TYPE_NAME_2_RANDOM	= 2
};

/// ディレクトリ１アイテム FAT8ビット
class DiskBasicDirItemFAT8 : public DiskBasicDirItem
{
protected:
	int m_start_address;	///< ファイル内部で持っている開始アドレス
	int m_end_address;		///< ファイル内部で持っている終了アドレス
	int m_exec_address;		///< ファイル内部で持っている実行アドレス

	DiskBasicDirItemFAT8() : DiskBasicDirItem() {}
	DiskBasicDirItemFAT8(const DiskBasicDirItemFAT8 &src) : DiskBasicDirItem(src) {}

	/// @brief 使用しているアイテムか
	bool			CheckUsed(bool unuse);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int			    GetFileType1Pos() const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int			    GetFileType2Pos() const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	virtual void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief ファイル内部のアドレスを取り出す
	virtual void	TakeAddressesInFile();

	/// @brief ファイル名に拡張子を付ける
	virtual wxString AddExtension(int file_type_1, const wxString &name) const { return name; }

public:
	DiskBasicDirItemFAT8(DiskBasic *basic);
	DiskBasicDirItemFAT8(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemFAT8(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief 属性を設定
	void			SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString 		GetFileAttrStr() const;

	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);

	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }
	/// @brief アドレスを編集できるか
	virtual bool	IsAddressEditable() const { return false; }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 終了アドレスを返す
	virtual int		GetEndAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;

	/// @brief ファイル名から属性を決定する
	virtual int		ConvFileTypeFromFileName(const wxString &filename) const;

	// ダイアログ用

#define ATTR_DIALOG_IDC_RADIO_TYPE1 51
#define ATTR_DIALOG_IDC_RADIO_TYPE2 52

	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
};

//
//

/// ディレクトリ１アイテム FAT8ビット(F-BASIC, L3 1S) 
class DiskBasicDirItemFAT8F : public DiskBasicDirItemFAT8
{
protected:
	DiskBasicDirItemFAT8F() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemFAT8F(const DiskBasicDirItemFAT8F &src) : DiskBasicDirItemFAT8(src) {}

	/// @brief ディレクトリアイテムのチェック
	bool 			Check(bool &last);

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性３を返す
	virtual int		GetFileType3() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 属性３のセット
	virtual void	SetFileType3(int val);

public:
	DiskBasicDirItemFAT8F(DiskBasic *basic);
	DiskBasicDirItemFAT8F(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemFAT8F(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief ファイルサイズをセット
	virtual void 	SetFileSize(int val);

	/// @brief インポート時などのダイアログを出す前にファイルパスから内部ファイル名を生成する
	virtual bool	PreImportDataFile(wxString &filename);

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool 	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);
};

#endif /* _BASICDIRITEM_FAT8_H_ */
