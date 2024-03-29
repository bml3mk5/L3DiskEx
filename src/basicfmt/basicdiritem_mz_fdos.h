/// @file basicdiritem_mz_fdos.h
///
/// @brief disk basic directory item for MZ Floppy DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_MZ_FDOS_H
#define BASICDIRITEM_MZ_FDOS_H

#include "basicdiritem_mz_base.h"


//////////////////////////////////////////////////////////////////////

enum enTypeNameMZFDOS {
	TYPE_NAME_MZ_FDOS_UNKNOWN = 0,
	TYPE_NAME_MZ_FDOS_OBJ,
	TYPE_NAME_MZ_FDOS_BTX,
	TYPE_NAME_MZ_FDOS_DAT,
	TYPE_NAME_MZ_FDOS_ASC,
	TYPE_NAME_MZ_FDOS_RB,
	TYPE_NAME_MZ_FDOS_FTN,
	TYPE_NAME_MZ_FDOS_LIB,
	TYPE_NAME_MZ_FDOS_PAS,
	TYPE_NAME_MZ_FDOS_TEM,
	TYPE_NAME_MZ_FDOS_SYS,
	TYPE_NAME_MZ_FDOS_GR,
	TYPE_NAME_MZ_FDOS_GRH,
	TYPE_NAME_MZ_FDOS_END

};
/// @brief MZ Floppy DOS 属性
enum en_file_type_mz_fdos {
	FILETYPE_MZ_FDOS_UNKNOWN = 0,
	FILETYPE_MZ_FDOS_OBJ = 0x1,
	FILETYPE_MZ_FDOS_BTX = 0x2,
	FILETYPE_MZ_FDOS_DAT = 0x3,
	FILETYPE_MZ_FDOS_ASC = 0x4,
	FILETYPE_MZ_FDOS_RB  = 0x5,
	FILETYPE_MZ_FDOS_FTN = 0x6,
	FILETYPE_MZ_FDOS_LIB = 0x7,
	FILETYPE_MZ_FDOS_PAS = 0x8,
	FILETYPE_MZ_FDOS_TEM = 0x9,
	FILETYPE_MZ_FDOS_SYS = 0xa,
	FILETYPE_MZ_FDOS_GR  = 0xb,
	FILETYPE_MZ_FDOS_GRH = 0xc,
};

#pragma pack(1)
/// @brief FDOSチェイン情報
typedef struct st_mz_fdos_chain {
	wxUint16 sectors;
	wxUint8  map[1];	// resizable
} mz_fdos_chain_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

/// @brief FDOSチェイン情報アクセス
class DiskBasicDirItemMZFDOSChain
{
private:
	DiskBasic		*basic;
	wxUint32		secs_per_track;
	DiskImageSector	*sector;
	mz_fdos_chain_t	*chain;
	bool			chain_ownmake;
	wxUint32		map_size;
public:
	DiskBasicDirItemMZFDOSChain();
	~DiskBasicDirItemMZFDOSChain();
#ifdef COPYABLE_DIRITEM
	/// @brief 代入
	DiskBasicDirItemMZFDOSChain &operator=(const DiskBasicDirItemMZFDOSChain &src);
	/// @brief 複製
	void Dup(const DiskBasicDirItemMZFDOSChain &src);
#endif
	/// @brief ポインタをセット
	void Set(DiskBasic *n_basic, DiskImageSector *n_sector, mz_fdos_chain_t *n_chain);
	/// @brief メモリ確保
	void Alloc();
	/// @brief クリア
	void Clear();
	/// @brief 有効か
	bool IsValid() const { return (chain != NULL); }
	/// @brief セクタ位置の使用状態を返す
	bool IsUsedSector(int sector_pos) const;
	/// @brief セクタ数を返す
	wxUint16 GetSectors() const;
	/// @brief セクタ位置の使用状態を設定
	void UsedSector(int sector_pos, bool val);
	/// @brief セクタ数を設定
	void SetSectors(wxUint16 val);

	void SetSectorsPerTrack(wxUint32 val) { secs_per_track = val; }
	void SetMapSize(wxUint32 val) { map_size = val; }
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム MZ Floppy DOS
class DiskBasicDirItemMZFDOS : public DiskBasicDirItemMZBase
{
private:
	DiskBasicDirItemMZFDOS() : DiskBasicDirItemMZBase() {}
	DiskBasicDirItemMZFDOS(const DiskBasicDirItemMZFDOS &src) : DiskBasicDirItemMZBase(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_mz_fdos_t> m_data;

	/// @brief チェイン情報
	DiskBasicDirItemMZFDOSChain chain;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 属性を変換
	int				ConvToNativeType(int file_type) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				ConvFileType1Pos(int native_type) const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int				CalcFileTypeFromPos(int pos) const;
	/// @brief 属性1を得る
	int				GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief 属性2を得る
	int				GetFileType2InAttrDialog(const IntNameBox *parent) const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief データ内にファイルサイズをセット
	void			SetFileSizeBase(int val);
	/// @brief データ内のファイルサイズを返す
	int				GetFileSizeBase() const;
	/// @brief グループ取得計算前処理
	void			PreCalcAllGroups(int &calc_flags, wxUint32 &group_num, int &remain, int &sec_size, void **user_data);
	/// @brief グループ取得計算中処理
	void			CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data);

public:
	DiskBasicDirItemMZFDOS(DiskBasic *basic);
	DiskBasicDirItemMZFDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemMZFDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

//	/// @brief 削除
//	bool			Delete();
	/// @brief ファイル名に設定できない文字を文字列にして返す
	wxString		InvalidateChars() const;
	/// @brief ファイル名に付随する拡張属性を設定
	virtual void	SetOptionalName(int val);
	/// @brief ファイル名に付随する拡張属性を返す
	virtual int		GetOptionalName() const;

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);
	/// @brief グループ数をセット
	void			SetGroupSize(int val);
	/// @brief グループ数を返す
	int				GetGroupSize() const;

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 追加のグループ番号をセット
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る
	virtual void	GetExtraGroups(wxArrayInt &arr) const;
	/// @brief データのあるグループ番号をセット
	void			SetDataGroup(wxUint32 val);
	/// @brief データのあるグループ番号を返す
	wxUint32		GetDataGroup() const;

	/// @brief アイテムが日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	virtual bool	HasCreateDate() const { return true; }
	virtual bool	HasCreateTime() const { return false; }
	/// @brief アイテムの時間設定を無視することができるか
	virtual enDateTime CanIgnoreDateTime() const { return DATETIME_ALL; }
	/// @brief 日付を返す
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 日付をセット
	virtual void	SetFileCreateDate(const TM &tm);

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

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief エントリデータの不明部分を設定
	void			SetUnknownData();

	/// @brief データをチェインする必要があるか（非連続データか）
	bool			NeedChainInData() const;
	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	virtual bool	PreImportDataFile(wxString &filename);

	/// @brief シーケンス番号
	void			AssignSeqNumber();

	/// @brief チェイン情報にセクタをセット
	virtual void	SetChainSector(DiskImageSector *sector, wxUint8 *data, const DiskBasicDirItem *pitem = NULL);
	/// @brief チェイン情報にセクタをセット
	void			SetChainUsedSector(int sector_pos, bool val);

	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief ファイル名から拡張属性を決定する
	virtual int		ConvOptionalNameFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	virtual bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	/// @brief ファイル名に付随する拡張属性をセットする
	virtual int		GetOptionalNameInAttrDialog(const IntNameBox *parent);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* BASICDIRITEM_MZ_FDOS_H */
