/// @file basicdiritem_trsdos.h
///
/// @brief disk basic directory item for Tandy TRSDOS 2.x / 1.3
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_TRSDOS_H_
#define _BASICDIRITEM_TRSDOS_H_

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

/// @brief TRSDOS属性位置
enum en_type_name_trsdos_1 {
	TYPE_NAME_TRSDOS_INVISIBLE,
	TYPE_NAME_TRSDOS_SYSTEM,
	TYPE_NAME_TRSDOS_OVERFLOW,
};

/// @brief TRSDOS属性値
enum en_file_type_mask_trsdos_1 {
	FILETYPE_MASK_TRSDOS_ACCESS = 0x07,
	FILETYPE_MASK_TRSDOS_INVISIBLE = 0x08,
	FILETYPE_MASK_TRSDOS_INUSE = 0x10,
	FILETYPE_MASK_TRSDOS_SYSTEM = 0x40,
	FILETYPE_MASK_TRSDOS_OVERFLOW = 0x80,
};

/// @brief TRSDOS属性位置
enum en_type_name_trsdos_2 {
	TYPE_NAME_2_TRSDOS_SYS,
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム TRSDOS Base
///
/// @sa DiskBasicTypeTRSDOS
class DiskBasicDirItemTRSDOS : public DiskBasicDirItem
{
protected:
	DiskBasicDirItemTRSDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemTRSDOS(const DiskBasicDirItemTRSDOS &src) : DiskBasicDirItem(src) {}

	/// @brief 次のエントリ(overflowアリの場合)
	DiskBasicDirItemTRSDOS *next_item;

	/// @brief HITの位置
	int m_position_in_hit;

//	/// @brief ディレクトリエントリを確保
//	bool			AllocateItem(const SectorParam *next);

//	/// @brief ファイル名を設定
//	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
//	/// @brief ファイル名を得る
//	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
//	/// @brief 属性２を返す
//	virtual int	GetFileType2() const;
//	/// @brief 属性２のセット
//	virtual void	SetFileType2(int val);
//	/// @brief 属性３を返す
//	virtual int	GetFileType3() const;
//	/// @brief 属性３のセット
//	virtual void	SetFileType3(int val);
//	/// @brief AUX_TYPEを返す
//	int				GetAuxType() const;
//	/// @brief AUX_TYPEのセット
//	void			SetAuxType(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

//	/// @brief 使用ブロック数を返す
//	int				GetBlocksUsed() const;
//	/// @brief 使用ブロック数をセット
//	void			SetBlocksUsed(int val);

	/// @brief ファイル内部のアドレスを取り出す
	void			TakeAddressesInFile(DiskBasicGroups &group_items);

	/// @brief 属性からリストの位置を返す
	int				ConvFileType1Pos(int type1) const;
//	/// @brief 属性からリストの位置を返す
//	int				ConvFileType2Pos(int type2) const;

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemTRSDOS(DiskBasic *basic);
	DiskBasicDirItemTRSDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemTRSDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief 削除
	virtual bool	Delete();

//	/// @brief ファイル名＋拡張子のサイズ
//	virtual int		GetFileNameStrSize() const;

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief HITの位置をセット
	void			SetPositionInHIT(wxUint8 val) { m_position_in_hit = val;}
	/// @brief HITの位置を返す
	wxUint8			GetPositionInHIT() const { return m_position_in_hit; }
	/// @brief 次のアイテムをセット
	void			SetNextItem(DiskBasicDirItem *val) { next_item = (DiskBasicDirItemTRSDOS *)val; }
	/// @brief 次のアイテムを返す
	DiskBasicDirItemTRSDOS *GetNextItem() { return next_item; }

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString	GetFileAttrStr() const;

	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
//	/// @brief ディレクトリヘッダのあるグループ番号をセット(機種依存)(HEADER_POINTER)
//	virtual void	SetParentGroup(wxUint32 val);
//	/// @brief ディレクトリヘッダのあるグループ番号を返す(機種依存)(HEADER_POINTER)
//	virtual wxUint32 GetParentGroup() const;

	/// @brief Overflowをセット
	virtual void	SetOverflow(wxUint8 val) {}
	/// @brief Overflowを返す
	virtual wxUint8	GetOverflow() const { return 0; }

	/// @brief GAPのGranule番号をセット
	virtual void	SetGranulesOnGap(int pos, wxUint32 val, wxUint32 cnt) {}
	/// @brief GAPのGranule番号をクリア
	virtual void	ClearGranulesOnGap(int pos, wxUint32 track, wxUint32 granule) {}
	/// @brief GAPのGranule番号を返す
	virtual wxUint32 GetGranulesOnGap(int pos, wxUint32 *cnt = 0) const { return 0; }

	/// @brief 新規ファイルとして設定
	virtual void	SetAsNewFile() {}
	/// @brief Overflowファイルとして設定
	virtual void	SetAsOverflowFile(wxUint8 position_in_hit, wxUint8 hash_code) {}
	/// @brief "BOOT/SYS"として設定
	virtual void	SetAsBootSysEntry() {}
	/// @brief "DIR/SYS"として設定
	virtual void	SetAsDirSysEntry() {}

	//	/// @brief グループリストをセット
//	virtual void	SetExtraGroups(const DiskBasicGroups &grps);
//	/// @brief 追加のグループ番号を返す(機種依存)
//	virtual wxUint32 GetExtraGroup() const;
//	/// @brief 追加のグループ番号を得る(機種依存)
//	virtual void	GetExtraGroups(wxArrayInt &arr) const;
//	/// @brief チェイン用のセクタをクリア(機種依存)
//	virtual void	ClearChainSector(const DiskBasicDirItem *pitem = NULL);
//	/// @brief チェイン用のセクタをセット(機種依存)
//	virtual void	SetChainSector(wxUint32 num, int pos, wxUint8 *data, const DiskBasicDirItem *pitem = NULL);
//	/// @brief チェイン用のセクタにグループ番号をセット(機種依存)
//	virtual void	AddChainGroupNumber(int idx, wxUint32 val);

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
//	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
//	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);
	
//	/// @brief データをエクスポートする前に必要な処理
//	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief データをインポートする前に必要な処理
	virtual bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;

//	/// @brief ファイル数を＋１
//	void			IncreaseFileCount();
//	/// @brief ファイル数を－１
//	void			DecreaseFileCount();

	/// @brief アイテムの属するセクタを変更済みにする
	virtual void	SetModify();

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief ダイアログ内の値を設定
	virtual void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
//	/// @brief 属性を変更した際に呼ばれるコールバック
//	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
//	/// @brief ダイアログ入力後のファイル名チェック
//	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
//	/// @brief その他の属性値を設定する
//	virtual void	SetOptionalAttr(DiskBasicDirItemAttr &attr);
//	/// @brief プロパティで表示する内部データを設定
//	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム TRSDOS 2.x
///
/// @sa DiskBasicTypeTRSD23
class DiskBasicDirItemTRSD23 : public DiskBasicDirItemTRSDOS
{
protected:
	DiskBasicDirItemTRSD23() : DiskBasicDirItemTRSDOS() {}
	DiskBasicDirItemTRSD23(const DiskBasicDirItemTRSD23 &src) : DiskBasicDirItemTRSDOS(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_trsd23_t> m_data;

	/// @brief 属性１を返す
	virtual int	GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);

public:
	DiskBasicDirItemTRSD23(DiskBasic *basic);
	DiskBasicDirItemTRSD23(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemTRSD23(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

	/// @brief Overflowをセット
	virtual void	SetOverflow(wxUint8 val);
	/// @brief Overflowを返す
	virtual wxUint8	GetOverflow() const;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;

	/// @brief 新規ファイルとして設定
	virtual void	SetAsNewFile();
	/// @brief Overflowファイルとして設定
	virtual void	SetAsOverflowFile(wxUint8 position_in_hit, wxUint8 hash_code);
	/// @brief "BOOT/SYS"として設定
	virtual void	SetAsBootSysEntry();
	/// @brief "DIR/SYS"として設定
	virtual void	SetAsDirSysEntry();

	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);
	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// @brief ファイルサイズを返す
	virtual int		GetFileSize() const;

	/// @brief GAPのGranule番号をセット
	virtual void	SetGranulesOnGap(int pos, wxUint32 val, wxUint32 cnt);
	/// @brief GAPのGranule番号をクリア
	virtual void	ClearGranulesOnGap(int pos, wxUint32 track, wxUint32 granule);
	/// @brief GAPのGranule番号を返す
	virtual wxUint32 GetGranulesOnGap(int pos, wxUint32 *cnt = 0) const;

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @name プロパティダイアログ用
	//@{
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム TRSDOS 1.3
///
/// @sa DiskBasicTypeTRSD13
class DiskBasicDirItemTRSD13 : public DiskBasicDirItemTRSDOS
{
protected:
	DiskBasicDirItemTRSD13() : DiskBasicDirItemTRSDOS() {}
	DiskBasicDirItemTRSD13(const DiskBasicDirItemTRSD13 &src) : DiskBasicDirItemTRSDOS(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_trsd13_t> m_data;

	/// @brief 属性１を返す
	virtual int	GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);

public:
	DiskBasicDirItemTRSD13(DiskBasic *basic);
	DiskBasicDirItemTRSD13(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemTRSD13(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

//	/// @brief Overflowをセット
//	virtual void	SetOverflow(wxUint8 val);
//	/// @brief Overflowを返す
//	virtual wxUint8	GetOverflow() const;

	/// @brief アイテムが作成日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	/// @brief アイテムが作成日付を持っているか
	virtual bool	HasCreateDate() const { return true; }
	/// @brief 作成日付を得る
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 作成日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 作成日付をセット
	virtual void	SetFileCreateDate(const TM &tm);

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;

	/// @brief 新規ファイルとして設定
	virtual void	SetAsNewFile();
//	/// @brief Overflowファイルとして設定
//	virtual void	SetAsOverflowFile(wxUint8 position_in_hit, wxUint8 hash_code);

	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);
	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// @brief ファイルサイズを返す
	virtual int		GetFileSize() const;

	/// @brief GAPのGranule番号をセット
	virtual void	SetGranulesOnGap(int pos, wxUint32 val, wxUint32 cnt);
	/// @brief GAPのGranule番号をクリア
	virtual void	ClearGranulesOnGap(int pos, wxUint32 track, wxUint32 granule);
	/// @brief GAPのGranule番号を返す
	virtual wxUint32 GetGranulesOnGap(int pos, wxUint32 *cnt = 0) const;

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @name プロパティダイアログ用
	//@{
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_TRSDOS_H_ */
