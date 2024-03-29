/// @file basicdiritem_frost.h
///
/// @brief disk basic directory item for Frost-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_FROST_H
#define BASICDIRITEM_FROST_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

// Frost-DOS

#define FROST_GROUP_SIZE	512

//extern const char *gTypeNameFROST_1[];
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

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Frost-DOS
class DiskBasicDirItemFROST : public DiskBasicDirItem
{
private:
	DiskBasicDirItemFROST() : DiskBasicDirItem() {}
	DiskBasicDirItemFROST(const DiskBasicDirItemFROST &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_frost_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性１の文字列
	wxString ConvFileType1Str(int t1) const;
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief グループを追加する
	void			AddGroups(wxUint32 group_num, wxUint32 next_group, DiskBasicGroups &group_items);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				ConvFileType1Pos(int t1) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType2Pos() const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemFROST(DiskBasic *basic);
	DiskBasicDirItemFROST(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemFROST(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

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

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }
	/// @brief アイテムが実行アドレスを持っているか
	virtual bool	HasExecuteAddress() const { return false; }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	virtual bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	virtual bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* BASICDIRITEM_FROST_H */
