/// @file basicdiritem_c1541.h
///
/// @brief disk basic directory item for Commodore 1541
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_C1541_H
#define BASICDIRITEM_C1541_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

#define C1541_START_TRACK_OFFSET  (0)
#define C1541_START_SECTOR_OFFSET (0)

/// @brief C1541属性値
enum en_file_type_mask_c1541 {
	FILETYPE_MASK_C1541_DEL = 0x80,
	FILETYPE_MASK_C1541_SEQ = 0x81,
	FILETYPE_MASK_C1541_PRG = 0x82,
	FILETYPE_MASK_C1541_USR = 0x83,
	FILETYPE_MASK_C1541_REL = 0x84,
};

/// @brief C1541属性位置
enum en_type_name_c1541 {
	TYPE_NAME_C1541_DEL = 0,
	TYPE_NAME_C1541_SEQ,
	TYPE_NAME_C1541_PRG,
	TYPE_NAME_C1541_USR,
	TYPE_NAME_C1541_REL,
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Commodore 1541
class DiskBasicDirItemC1541 : public DiskBasicDirItem
{
private:
	DiskBasicDirItemC1541() : DiskBasicDirItem() {}
	DiskBasicDirItemC1541(const DiskBasicDirItemC1541 &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_c1541_t> m_data;

	DiskBasicGroups m_ss_groups;		///< RELファイルの時、サイドセクタ分

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
	/// @brief 属性１を返す
	virtual int	GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);

	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 共通属性を個別属性に変換
	static int		ConvToFileType1(int ftype);
	/// @brief 個別属性を共通属性に変換
	static int		ConvFromFileType1(int type1);
	/// @brief 属性からリストの位置を返す
	int				ConvFileType1Pos(int type1) const;

	/// @brief ブロック数をセット
	void			SetBlocks(wxUint16 val);
	/// @brief ブロック数を返す
	wxUint16		GetBlocks() const;

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemC1541(DiskBasic *basic);
	DiskBasicDirItemC1541(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemC1541(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

	/// @brief 削除
	virtual bool	Delete();

	/// @brief ファイル名＋拡張子のサイズ
	virtual int		GetFileNameStrSize() const;

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
	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief サイドセクタのあるグループ番号をセット(機種依存)
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief サイドセクタのあるグループ番号を返す(機種依存)
	virtual wxUint32 GetExtraGroup() const;
	/// @brief サイドセクタのグループリストをセット(機種依存)
	virtual void	SetExtraGroups(const DiskBasicGroups &grps);
	/// @brief サイドセクタのグループ番号を得る(機種依存)
	virtual void	GetExtraGroups(wxArrayInt &arr) const;
	/// @brief サイドセクタのグループリストを返す(機種依存)
	virtual void	GetExtraGroups(DiskBasicGroups &grps) const;

	/// @brief レコード長をセット(REL file)
	void			SetRecordLength(int val);
	/// @brief レコード長を返す(REL file)
	int				GetRecordLength() const;

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);
	
	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief データをインポートする前に必要な処理
	virtual bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @brief アイテムの属するセクタを変更済みにする
	virtual void	SetModify();

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
//	/// @brief ダイアログ入力後のファイル名チェック
//	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
//	/// @brief その他の属性値を設定する
//	virtual void	SetOptionalAttr(DiskBasicDirItemAttr &attr);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* BASICDIRITEM_C1541_H */
