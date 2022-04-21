/// @file basicdiritem_frost.h
///
/// @brief disk basic directory item for Frost-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FROST_H_
#define _BASICDIRITEM_FROST_H_

#include "basicdiritem.h"


// Frost-DOS

#define FROST_GROUP_SIZE	512

extern const char *gTypeNameFROST_1[];
enum en_type_name_frost_1 {
	TYPE_NAME_FROST_BAS = 0,
	TYPE_NAME_FROST_BIN,
	TYPE_NAME_FROST_RGB,
	TYPE_NAME_FROST_UNKNOWN
};
enum en_file_type_frost {
	FILETYPE_FROST_BAS = 0x00,
	FILETYPE_FROST_BIN = 0x01,
	FILETYPE_FROST_RGB = 0x02
};

/// ディレクトリ１アイテム Frost-DOS
class DiskBasicDirItemFROST : public DiskBasicDirItem
{
private:
	DiskBasicDirItemFROST() : DiskBasicDirItem() {}
	DiskBasicDirItemFROST(const DiskBasicDirItemFROST &src) : DiskBasicDirItem(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 属性１の文字列
	wxString ConvFileType1Str(int t1) const;
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief グループを追加する
	void	AddGroups(wxUint32 group_num, wxUint32 next_group, DiskBasicGroups &group_items);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    ConvFileType1Pos(int t1) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType2Pos() const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemFROST(DiskBasic *basic);
	DiskBasicDirItemFROST(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemFROST(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// @brief ENDマークがあるか(一度も使用していないか)
	bool	HasEndMark();
	/// @brief 次のアイテムにENDマークを入れる
	void	SetEndMark(DiskBasicDirItem *next_item);

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

	/// @brief ファイルサイズとグループ数を計算する
	void	CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);
	/// @brief 最初のグループ番号をセット
	void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief アイテムがアドレスを持っているか
	bool	HasAddress() const { return true; }
	/// @brief アイテムが実行アドレスを持っているか
	bool	HasExecuteAddress() const { return false; }
	/// @brief 開始アドレスを返す
	int		GetStartAddress() const;
	/// @brief 開始アドレスをセット
	void	SetStartAddress(int val);

	/// @brief ディレクトリアイテムのサイズ
	size_t	GetDataSize() const;

	/// @brief ファイルの終端コードをチェックする必要があるか
	bool	NeedCheckEofCode();

	/// @brief ディレクトリをクリア ファイル新規作成時
	void	ClearData();

	/// @brief データをエクスポートする前に必要な処理
	bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	//@}
};

#endif /* _BASICDIRITEM_FROST_H_ */
