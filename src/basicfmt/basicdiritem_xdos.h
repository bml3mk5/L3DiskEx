/// @file basicdiritem_xdos.h
///
/// @brief disk basic directory item for X-DOS for X1
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_XDOS_H
#define BASICDIRITEM_XDOS_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

#define XDOS_CHAIN_SEGMENTS	170

#pragma pack(1)
/// @brief X-DOSチェイン情報 (FAM)
typedef struct st_xdos_chain {
	xdos_seg_t seg[XDOS_CHAIN_SEGMENTS];
} xdos_chain_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

/// @brief X-DOS属性値
enum en_file_type_xdos {
	FILETYPE_XDOS_NUL = 0x00,
	FILETYPE_XDOS_BIN = 0x01,
	FILETYPE_XDOS_BAS = 0x02,
	FILETYPE_XDOS_CMD = 0x03,
	FILETYPE_XDOS_ASC = 0x04,
	FILETYPE_XDOS_SUB = 0x05,
	FILETYPE_XDOS_BAT = 0x06,
	FILETYPE_XDOS_SYS = 0x07,
	FILETYPE_XDOS_DIC = 0x08,
	FILETYPE_XDOS_DIR = 0x80,
};

//////////////////////////////////////////////////////////////////////

/// @brief X-DOSチェイン情報アクセス
class DiskBasicDirItemXDOSChain
{
private:
	DiskBasic		*basic;
	DiskImageSector	*sector;
	xdos_chain_t	*chain;
	bool			chain_ownmake;
public:
	DiskBasicDirItemXDOSChain();
	~DiskBasicDirItemXDOSChain();
#ifdef COPYABLE_DIRITEM
	/// @brief 代入
	DiskBasicDirItemXDOSChain &operator=(const DiskBasicDirItemXDOSChain &src);
	/// @brief 複製
	void Dup(const DiskBasicDirItemXDOSChain &src);
#endif
	/// @brief ポインタをセット
	void Set(DiskBasic *n_basic, DiskImageSector *n_sector, xdos_chain_t *n_chain);
	/// @brief メモリ確保
	void Alloc();
	/// @brief クリア
	void Clear();
	/// @brief 有効か
	bool IsValid() const { return (chain != NULL); }
	/// @brief セグメントのセクタ番号を返す
	wxUint32 GetSectorPos(int idx) const;
	/// @brief セクタ数を返す
	int GetSectors() const;
	/// @brief セグメントのセクタ数を返す
	wxUint8 GetSectors(int idx) const;
	/// @brief セグメントにLSNを設定 すでにあればサイズを増やす
	void AddSectorPos(int idx, wxUint32 val);
	/// @brief セグメントにセクタ数を設定
	void SetSectors(int idx, wxUint8 val);
	/// @brief セグメントを得る
	bool GetSegment(int idx, wxUint32 &group_num, int &size) const;

	void SetBasic(DiskBasic *n_basic) { basic = n_basic; }
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム X-DOS Base
class DiskBasicDirItemXDOSBase : public DiskBasicDirItem
{
protected:
	DiskBasicDirItemXDOSBase() : DiskBasicDirItem() {}
	DiskBasicDirItemXDOSBase(const DiskBasicDirItemXDOSBase &src) : DiskBasicDirItem(src) {}
	DiskBasicDirItemXDOSBase(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse, bool n_inherit);

	/// @brief チェイン情報
	DiskBasicDirItemXDOSChain chain;

	/// @brief ディレクトリエントリを確保
	virtual bool	AllocateItem() { return true; }
	/// @brief ディレクトリエントリを確保
	virtual bool	AllocateItem(const SectorParam *next) { return true; }

	/// @brief チェイン情報を設定
	virtual void	AttachChain(wxUint32 group_num);

	/// @brief グループを追加する
	void			AddGroups(wxUint32 group_num, wxUint32 next_group, DiskBasicGroups &group_items);

public:
	DiskBasicDirItemXDOSBase(DiskBasic *basic);
	DiskBasicDirItemXDOSBase(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemXDOSBase(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

//	/// @brief アイテムへのポインタを設定
//	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ファイル名を編集できるか
	virtual bool	IsFileNameEditable() const;
	/// @brief アイテムをロード・エクスポートできるか
	virtual bool	IsLoadable() const;
	/// @brief アイテムをコピー(内部でDnD)できるか
	virtual bool	IsCopyable() const;
	/// @brief アイテムを上書きできるか
	virtual bool	IsOverWritable() const;

	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);
	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);
	/// @brief チェイン用のセクタをセット
	virtual void	SetChainSector(DiskImageSector *sector, wxUint8 *data, const DiskBasicDirItem *pitem = NULL);
	/// @brief チェイン用のセクタにグループ番号をセット(機種依存)
	virtual void	AddChainGroupNumber(int idx, wxUint32 val);

	/// @brief アイテムが日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	virtual bool	HasCreateDate() const { return true; }
	virtual bool	HasCreateTime() const { return true; }
	/// @brief 日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 時間を返す
	virtual wxString GetFileCreateTimeStr() const;

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム X-DOS
class DiskBasicDirItemXDOS : public DiskBasicDirItemXDOSBase
{
protected:
	DiskBasicDirItemXDOS() : DiskBasicDirItemXDOSBase() {}
	DiskBasicDirItemXDOS(const DiskBasicDirItemXDOS &src) : DiskBasicDirItemXDOSBase(src) {}
	DiskBasicDirItemXDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse, bool n_inherit);

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_xdos_t> m_data;

//	/// @brief ディレクトリエントリを確保
//	bool	AllocateItem() { return true; }

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性１の文字列
	wxString		ConvFileType1Str(int t1) const;
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType1Pos() const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType2Pos() const;
	/// @brief 属性1を得る
	int				GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief 属性2を得る
	int				GetFileType2InAttrDialog(const IntNameBox *parent) const;

	/// @brief ユーザーファイルタイプ名に変換
	static void		ConvUserFileTypeToStr(int type1, wxString &str);
	/// @brief ユーザーファイルタイプに変換
	static int		ConvStrToUserFileType(const wxString &str);

	/// @brief 日付を変換
	static void		ConvDateToTm(wxUint16 date, TM &tm);
	/// @brief 時間を変換
	static void		ConvTimeToTm(wxUint16 time, TM &tm);
	/// @brief 日付に変換
	static wxUint16	ConvTmToDate(const TM &tm);
	/// @brief 時間に変換
	static wxUint16	ConvTmToTime(const TM &tm);

public:
	DiskBasicDirItemXDOS(DiskBasic *basic);
	DiskBasicDirItemXDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemXDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
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
	/// @brief 追加のグループ番号をセット チェインセクタへセット
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す チェインセクタを返す
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る
	virtual void	GetExtraGroups(wxArrayInt &arr) const;

	/// @brief 日付を得る
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 時間を得る
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 日付をセット
	virtual void	SetFileCreateDate(const TM &tm);
	/// @brief 時間をセット
	virtual void	SetFileCreateTime(const TM &tm);
	/// @brief 日付のタイトル名（ダイアログ用）
	virtual wxString GetFileCreateDateTimeTitle() const;

	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();
	/// @brief ディレクトリを初期化 未使用にする
	virtual void	InitialData();

	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	virtual bool	PreImportDataFile(wxString &filename);
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

#endif /* BASICDIRITEM_XDOS_H */
