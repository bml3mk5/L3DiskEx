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
	DiskBasicDirItemFAT8() : DiskBasicDirItem() {}
	DiskBasicDirItemFAT8(const DiskBasicDirItemFAT8 &src) : DiskBasicDirItem(src) {}

	/// 使用しているアイテムか
	bool			CheckUsed(bool unuse);

	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int			    GetFileType1Pos() const;
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int			    GetFileType2Pos() const;
	/// インポート時ダイアログ表示前にファイルの属性を設定
	virtual void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemFAT8(DiskBasic *basic);
	DiskBasicDirItemFAT8(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemFAT8(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// 属性を設定
	void			SetFileAttr(const DiskBasicFileType &file_type);

	/// 属性を返す
	DiskBasicFileType GetFileAttr() const;

//	/// リストの位置から属性を返す(プロパティダイアログ用)
//	int				CalcFileTypeFromPos(int pos1, int pos2);
	/// 属性の文字列を返す(ファイル一覧画面表示用)
	wxString 		GetFileAttrStr() const;

	/// ファイルサイズとグループ数を計算する
	virtual void	CalcFileSize();
	/// 指定ディレクトリのすべてのグループを取得
	virtual void	GetAllGroups(DiskBasicGroups &group_items);


	// ダイアログ用

#define ATTR_DIALOG_IDC_RADIO_TYPE1 51
#define ATTR_DIALOG_IDC_RADIO_TYPE2 52

	/// ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
};

//
//

/// ディレクトリ１アイテム FAT8ビット(F-BASIC, L3 1S) 
class DiskBasicDirItemFAT8F : public DiskBasicDirItemFAT8
{
protected:
	DiskBasicDirItemFAT8F() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemFAT8F(const DiskBasicDirItemFAT8F &src) : DiskBasicDirItemFAT8(src) {}

	/// ディレクトリアイテムのチェック
	bool 			Check(bool &last);

	/// ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(size_t &size, size_t &len) const;
//	/// ファイル名を格納するバッファサイズを返す
//	virtual int		GetFileNameSize(bool *invert = NULL) const;
	/// 属性１を返す
	virtual int		GetFileType1() const;
	/// 属性２を返す
	virtual int		GetFileType2() const;
	/// 属性３を返す
	virtual int		GetFileType3() const;
	/// 属性１のセット
	virtual void	SetFileType1(int val);
	/// 属性２のセット
	virtual void	SetFileType2(int val);
	/// 属性３のセット
	virtual void	SetFileType3(int val);

public:
	DiskBasicDirItemFAT8F(DiskBasic *basic);
	DiskBasicDirItemFAT8F(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemFAT8F(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;

	/// 最初のグループ番号をセット
	virtual void	SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup() const;

	/// ファイルサイズをセット
	virtual void 	SetFileSize(int val);

	/// ファイルの終端コードをチェックする必要があるか
	virtual bool 	NeedCheckEofCode();
	/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);
};

#endif /* _BASICDIRITEM_FAT8_H_ */
