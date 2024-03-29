/// @file basicdiritem_sdos.h
///
/// @brief disk basic directory item for S-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_SDOS_H
#define BASICDIRITEM_SDOS_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

// S-DOS

enum en_type_name_sdos_1 {
	TYPE_NAME_SDOS_BAS1 = 0,
	TYPE_NAME_SDOS_BAS2,
	TYPE_NAME_SDOS_DAT,
	TYPE_NAME_SDOS_OBJ,
	TYPE_NAME_SDOS_UNKNOWN,
};
enum en_file_type_sdos {
	FILETYPE_SDOS_BAS1 = 0x00,	// N mode
	FILETYPE_SDOS_BAS2 = 0x01,	// n88 mode (Sn88-DOS only)
	FILETYPE_SDOS_DAT = 0x02,	// except exec address
	FILETYPE_SDOS_OBJ = 0x0e,	// include exec address
	FILETYPE_SDOS_UNKNOWN = 0xff,
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム S-DOS
class DiskBasicDirItemSDOS : public DiskBasicDirItem
{
private:
	DiskBasicDirItemSDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemSDOS(const DiskBasicDirItemSDOS &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_sdos_t> m_data;

	DirItemSectorBoundary m_sdata;	///< セクタ内部へのポインタ

	/// @brief ディレクトリエントリを確保
	bool			AllocateItem(const SectorParam *next);

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int	GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief グループ数を返す
	wxUint32 GetGroupSize() const;

	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名と拡張子を得る
	virtual void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

	/// @brief グループを追加する
	void			AddGroups(wxUint32 group_num, wxUint32 next_group, DiskBasicGroups &group_items);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType1Pos() const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType2Pos() const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief 未使用領域の設定
	void			SetUnknownData();

public:
	DiskBasicDirItemSDOS(DiskBasic *basic);
	DiskBasicDirItemSDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemSDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

	/// @brief 削除
	virtual bool	Delete();
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
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val);

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

	/// @brief データをインポートする前に必要な処理
	virtual bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	virtual bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;

	/// @brief アイテムの属するセクタを変更済みにする
	virtual void	SetModify();

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

#endif /* BASICDIRITEM_SDOS_H */
