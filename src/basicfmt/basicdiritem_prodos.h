/// @file basicdiritem_prodos.h
///
/// @brief disk basic directory item for Apple ProDOS 8 / 16
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_PRODOS_H_
#define _BASICDIRITEM_PRODOS_H_

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

/// @brief Apple ProDOS属性名
extern const char *gTypeNameProDOS1[];
/// @brief Apple ProDOS属性位置 STORAGE_TYPE
enum en_type_name_prodos_1 {
	TYPE_NAME_PRODOS_FILE    = 0,
	TYPE_NAME_PRODOS_SUBDIR,
	TYPE_NAME_PRODOS_VOLUME,
};
/// @brief Apple ProDOS属性値 STORAGE_TYPE
enum en_file_type_mask_prodos_1 {
	FILETYPE_MASK_PRODOS_DELETED = 0x0,
	FILETYPE_MASK_PRODOS_SEEDING = 0x1,
	FILETYPE_MASK_PRODOS_SAPLING = 0x2,
	FILETYPE_MASK_PRODOS_TREE    = 0x3,
	FILETYPE_MASK_PRODOS_SUBDIR  = 0xd,
	FILETYPE_MASK_PRODOS_SUBVOL  = 0xe,
	FILETYPE_MASK_PRODOS_VOLUME  = 0xf,
};
/// @brief Apple ProDOS属性位置 FILE_TYPE
enum en_type_name_prodos_2 {
	TYPE_NAME_PRODOS_NOT,
	TYPE_NAME_PRODOS_BAD,
	TYPE_NAME_PRODOS_TXT,
	TYPE_NAME_PRODOS_BIN,
	TYPE_NAME_PRODOS_DIR,
	TYPE_NAME_PRODOS_ADB,
	TYPE_NAME_PRODOS_AWP,
	TYPE_NAME_PRODOS_ASP,
	TYPE_NAME_PRODOS_PAS,
	TYPE_NAME_PRODOS_CMD,
	TYPE_NAME_PRODOS_INT,
	TYPE_NAME_PRODOS_IVR,
	TYPE_NAME_PRODOS_BAS,
	TYPE_NAME_PRODOS_VAR,
	TYPE_NAME_PRODOS_REL,
	TYPE_NAME_PRODOS_SYS,
	TYPE_NAME_PRODOS_END,
};
/// @brief Apple ProDOS属性値 FILE_TYPE
enum en_file_type_mask_prodos_2 {
	FILETYPE_MASK_PRODOS_NOT = 0x00,
	FILETYPE_MASK_PRODOS_BAD = 0x01,
	FILETYPE_MASK_PRODOS_TXT = 0x04,
	FILETYPE_MASK_PRODOS_BIN = 0x06,
	FILETYPE_MASK_PRODOS_DIR = 0x0f,
	FILETYPE_MASK_PRODOS_ADB = 0x19,
	FILETYPE_MASK_PRODOS_AWP = 0x1a,
	FILETYPE_MASK_PRODOS_ASP = 0x1b,
	FILETYPE_MASK_PRODOS_PAS = 0xef,
	FILETYPE_MASK_PRODOS_CMD = 0xf0,
	FILETYPE_MASK_PRODOS_INT = 0xfa,
	FILETYPE_MASK_PRODOS_IVR = 0xfb,
	FILETYPE_MASK_PRODOS_BAS = 0xfc,
	FILETYPE_MASK_PRODOS_VAR = 0xfd,
	FILETYPE_MASK_PRODOS_REL = 0xfe,
	FILETYPE_MASK_PRODOS_SYS = 0xff,
};
/// @brief Apple ProDOS属性値 ACCESS
enum en_file_type_mask_prodos_3 {
	FILETYPE_MASK_PRODOS_DESTROY = 0x80,
	FILETYPE_MASK_PRODOS_RENAME = 0x40,
	FILETYPE_MASK_PRODOS_CHANGE = 0x20,
	FILETYPE_MASK_PRODOS_WRITE = 0x02,
	FILETYPE_MASK_PRODOS_READ = 0x01,
	FILETYPE_MASK_PRODOS_ACCESS_ALL = 0xe3,
};

//////////////////////////////////////////////////////////////////////

#pragma pack(1)
/// @brief Apple ProDOS ディレクトリブロックのチェイン
typedef struct st_prodos_dir_ptr {
	wxUint16 prev_block;
	wxUint16 next_block;
} prodos_dir_ptr_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

/// @brief Apple ProDOS インデックス１つ
class ProDOSOneIndex
{
private:
	wxUint8 *m_buf[2];
	int		 m_size;
	wxUint32 m_group_num;
public:
	ProDOSOneIndex();
	/// @brief セクタバッファを割当て
	void AttachBuffer(DiskBasic *basic, wxUint32 group_num, int st_pos);
	/// @brief バッファをセット
	void SetBuffer(int idx, wxUint8 *buffer) { m_buf[idx] = buffer; }
	/// @brief 指定位置のブロック番号を得る
	wxUint16 GetGroupNumber(int pos) const;
	/// @brief 指定位置のブロック番号をセット
	void SetGroupNumber(int pos, wxUint16 val);
	/// @brief サイズを得る
	int GetSize() const { return m_size; }
	/// @brief 自分のブロック番号を得る
	wxUint32 GetMyGroupNumber() const { return m_group_num; }
};

//////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(ProDOSOneIndex, ArrayOfProDOSIndex);

//////////////////////////////////////////////////////////////////////

/// @brief Apple ProDOS インデックス ProDOSOneIndex の配列
class ProDOSIndex : public ArrayOfProDOSIndex
{
private:
	DiskBasic			*m_basic;

	ProDOSIndex(const ProDOSIndex &src) {}

public:
	ProDOSIndex();
	~ProDOSIndex();
#ifdef COPYABLE_DIRITEM
	/// @brief 代入
	ProDOSIndex &operator=(const ProDOSIndex &src);
	/// @brief 複製
	void Dup(const ProDOSIndex &src);
#endif
	/// @brief BASICをセット
	void SetBasic(DiskBasic *basic);
	/// @brief 指定位置のブロック番号を得る
	wxUint16 GetGroupNumber(int pos) const;
	/// @brief 指定位置のブロック番号をセット
	void SetGroupNumber(int pos, wxUint16 val);
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Apple ProDOS 8 / 16
///
/// @sa DiskBasicTypeProDOS
class DiskBasicDirItemProDOS : public DiskBasicDirItem
{
private:
	DiskBasicDirItemProDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemProDOS(const DiskBasicDirItemProDOS &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_prodos_t> m_data;

	DirItemSectorBoundary m_sdata;		///< セクタ内部へのポインタ

	wxUint32 m_dir_group_num;			///< このアイテムの属するグループ番号

	/// @brief インデックス
	ProDOSIndex m_index;

	/// @brief ディレクトリエントリを確保
	bool			AllocateItem(const SectorParam *next);

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
	/// @brief 属性１を返す STORAGE_TYPE
	virtual int	GetFileType1() const;
	/// @brief 属性１のセット STORAGE_TYPE
	virtual void	SetFileType1(int val);
	/// @brief 属性２を返す FILE_TYPE
	virtual int	GetFileType2() const;
	/// @brief 属性２のセット FILE_TYPE
	virtual void	SetFileType2(int val);
	/// @brief 属性３を返す ACCESS
	virtual int	GetFileType3() const;
	/// @brief 属性３のセット ACCESS
	virtual void	SetFileType3(int val);
	/// @brief AUX_TYPEを返す
	int				GetAuxType() const;
	/// @brief AUX_TYPEのセット
	void			SetAuxType(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 使用ブロック数を返す
	int				GetBlocksUsed() const;
	/// @brief 使用ブロック数をセット
	void			SetBlocksUsed(int val);

	/// @brief ファイル内部のアドレスを取り出す
	void			TakeAddressesInFile(DiskBasicGroups &group_items);

	/// @brief 属性からリストの位置を返す
	int				ConvFileType1Pos(int type1) const;
	/// @brief 属性からリストの位置を返す
	int				ConvFileType2Pos(int type2) const;

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemProDOS(DiskBasic *basic);
	DiskBasicDirItemProDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemProDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

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

	/// @brief バージョンを返す
	int				GetVersion() const;
	/// @brief バージョンをセット
	void			SetVersion(int val);

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString	GetFileAttrStr() const;

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
	/// @brief ディレクトリヘッダのあるグループ番号をセット(機種依存)(HEADER_POINTER)
	virtual void	SetParentGroup(wxUint32 val);
	/// @brief ディレクトリヘッダのあるグループ番号を返す(機種依存)(HEADER_POINTER)
	virtual wxUint32 GetParentGroup() const;
	/// @brief 追加のグループ番号を返す(機種依存)
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る(機種依存)
	virtual void	GetExtraGroups(wxArrayInt &arr) const;
	/// @brief チェイン用のセクタをクリア(機種依存)
	virtual void	ClearChainSector(const DiskBasicDirItem *pitem = NULL);
	/// @brief チェイン用のセクタをセット(機種依存)
	virtual void	SetChainSector(wxUint32 num, int pos, wxUint8 *data, const DiskBasicDirItem *pitem = NULL);
	/// @brief チェイン用のセクタにグループ番号をセット(機種依存)
	virtual void	AddChainGroupNumber(int idx, wxUint32 val);

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
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;

	/// @brief アイテムが作成日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	/// @brief アイテムが作成日付を持っているか
	virtual bool	HasCreateDate() const { return true; }
	/// @brief アイテムが作成時間を持っているか
	virtual bool	HasCreateTime() const { return true; }
	/// @brief アイテムが更新日時を持っているか
	virtual bool	HasModifyDateTime() const;
	/// @brief アイテムが更新日付を持っているか
	virtual bool	HasModifyDate() const { return HasModifyDateTime(); }
	/// @brief アイテムが更新時間を持っているか
	virtual bool	HasModifyTime() const { return HasModifyDateTime(); }
	/// @brief アイテムの時間設定を無視することができるか
	virtual enDateTime CanIgnoreDateTime() const { return DATETIME_ALL; }
	/// @brief 作成日付を得る
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 作成時間を得る
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 作成日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 作成時間を返す
	virtual wxString GetFileCreateTimeStr() const;
	/// @brief 作成日付をセット
	virtual void	SetFileCreateDate(const TM &tm);
	/// @brief 作成時間をセット
	virtual void	SetFileCreateTime(const TM &tm);
	/// @brief 変更日付を得る
	virtual void	GetFileModifyDate(TM &tm) const;
	/// @brief 変更時間を得る
	virtual void	GetFileModifyTime(TM &tm) const;
	/// @brief 変更日付を返す
	virtual wxString GetFileModifyDateStr() const;
	/// @brief 変更時間を返す
	virtual wxString GetFileModifyTimeStr() const;
	/// @brief 変更日付をセット
	virtual void	SetFileModifyDate(const TM &tm);
	/// @brief 変更時間をセット
	virtual void	SetFileModifyTime(const TM &tm);
	/// @brief 日付を変換
	static void		ConvDateToTm(const wxUint8 *data, TM &tm);
	/// @brief 時間を変換
	static void		ConvTimeToTm(const wxUint8 *data, TM &tm);
	/// @brief 日付を変換
	static void		ConvDateFromTm(const TM &tm, wxUint8 *data);
	/// @brief 時間を変換
	static void		ConvTimeFromTm(const TM &tm, wxUint8 *data);

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return false; }
	/// @brief アイテムが実行アドレスを持っているか
	virtual bool	HasExecuteAddress() const { return false; }
	/// @brief アドレスを編集できるか
	virtual bool	IsAddressEditable() const { return false; }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 終了アドレスを返す
	virtual int		GetEndAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);

	/// @brief ファイル数を＋１
	void			IncreaseFileCount();
	/// @brief ファイル数を－１
	void			DecreaseFileCount();

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
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	/// @brief その他の属性値を設定する
	virtual void	SetAttr(DiskBasicDirItemAttr &attr);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_PRODOS_H_ */
