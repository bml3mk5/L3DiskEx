/// @file basicdiritem_amiga.h
///
/// @brief disk basic directory item for Amiga DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_AMIGA_H_
#define _BASICDIRITEM_AMIGA_H_

#include "basicdiritem.h"
#include <wx/dynarray.h>


//////////////////////////////////////////////////////////////////////

#pragma pack(1)
/// @brief Amiga Hash Chain (last 16bytes on each block (sector))
typedef struct st_amiga_hash_chain {
	wxUint32 hash_chain;		///< next entry with same hash
	wxUint32 parent;
	wxUint32 extension;
	wxUint32 sec_type;
} amiga_hash_chain_t;

/// @brief Amiga File Data Block (all Big Endien)
typedef union st_amiga_file_data_pre {
	struct st_ofs {
		wxUint32 type;				///< starting block (value: 8) 
		wxUint32 header_key;		///< file header block
		wxUint32 seq_num;			///< data block seq number 
		wxUint32 data_size;
		wxUint32 next_data;			///< next data block
		wxUint32 check_sum;
		wxUint8  data[1];
	} o;
	struct st_ffs {
		wxUint8  data[1];
	} f;
} amiga_file_data_pre_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

/// @brief Amiga ユーザデータに渡すチェイン情報
class AmigaChain : public DiskBasicGroupUserData
{
public:
	int       m_idx;			///< ハッシュテーブル内の位置
	wxUint32 *p_prev_chain;		///< ハッシュチェインのあるポインタ(前)
	wxUint32 *p_next_chain;		///< ハッシュチェインのあるポインタ(次)
public:
	AmigaChain();
	AmigaChain(int idx, wxUint32 *prev_chain, wxUint32 *next_chain);
	virtual DiskBasicGroupUserData *Clone() const { return new AmigaChain(*this); }
	AmigaChain &operator=(const DiskBasicGroupUserData &);
};

//////////////////////////////////////////////////////////////////////

#define KEY_FAST_FILE_SYSTEM "FastFileSystem"
#define KEY_INTERNATIONAL    "International"

/// @brief AMIGA属性値0
enum en_file_type_mask_amiga_0 {
	FILETYPE_MASK_AMIGA_HEADER = 2,
	FILETYPE_MASK_AMIGA_DATA = 8,
	FILETYPE_MASK_AMIGA_LIST = 16,
	FILETYPE_MASK_AMIGA_DIRCACHE = 33,
};

/// @brief AMIGA属性位置0
enum en_type_name_amiga_0 {
	TYPE_NAME_AMIGA_HEADER = 0,
	TYPE_NAME_AMIGA_DATA,
	TYPE_NAME_AMIGA_LIST,
	TYPE_NAME_AMIGA_DIRCACHE,
};

/// @brief AMIGA属性値１
enum en_file_type_mask_amiga_1 {
	FILETYPE_MASK_AMIGA_ROOT = 1,
	FILETYPE_MASK_AMIGA_FILE = -3,
	FILETYPE_MASK_AMIGA_USERDIR = 2,
	FILETYPE_MASK_AMIGA_LINKFILE = -4,
	FILETYPE_MASK_AMIGA_LINKDIR = 4,
	FILETYPE_MASK_AMIGA_SOFTLINK = 3,
};

/// @brief AMIGA属性位置１
enum en_type_name_amiga_1 {
	TYPE_NAME_AMIGA_FILE = 0,
	TYPE_NAME_AMIGA_USERDIR,
	TYPE_NAME_AMIGA_LINKFILE,
	TYPE_NAME_AMIGA_LINKDIR,
	TYPE_NAME_AMIGA_SOFTLINK,
};

/// @brief AMIGA属性値２
enum en_file_type_mask_amiga_2 {
	FILETYPE_MASK_AMIGA_U_NDEL   = 0x00000001,
	FILETYPE_MASK_AMIGA_U_NEXEC  = 0x00000002,
	FILETYPE_MASK_AMIGA_U_NWRITE = 0x00000004,
	FILETYPE_MASK_AMIGA_U_NREAD  = 0x00000008,
	FILETYPE_MASK_AMIGA_ARCHIVE  = 0x00000010,
	FILETYPE_MASK_AMIGA_PURE     = 0x00000020,
	FILETYPE_MASK_AMIGA_SCRIPT   = 0x00000040,
	FILETYPE_MASK_AMIGA_HOLD     = 0x00000080,
	FILETYPE_MASK_AMIGA_G_NDEL   = 0x00000100,
	FILETYPE_MASK_AMIGA_G_NEXEC  = 0x00000200,
	FILETYPE_MASK_AMIGA_G_NWRITE = 0x00000400,
	FILETYPE_MASK_AMIGA_G_NREAD  = 0x00000800,
	FILETYPE_MASK_AMIGA_O_NDEL   = 0x00001000,
	FILETYPE_MASK_AMIGA_O_NEXEC  = 0x00002000,
	FILETYPE_MASK_AMIGA_O_NWRITE = 0x00004000,
	FILETYPE_MASK_AMIGA_O_NREAD  = 0x00008000,
	FILETYPE_MASK_AMIGA_SETUID   = 0x80000000,
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Amiga DOS
///
/// @li FastFileSystem FFSかどうか(bool)
class DiskBasicDirItemAmiga : public DiskBasicDirItem
{
private:
	DiskBasicDirItemAmiga() : DiskBasicDirItem() {}
	DiskBasicDirItemAmiga(const DiskBasicDirItemAmiga &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_amiga_t> m_data;

	/// @brief エクステンションブロック番号
	wxArrayInt m_extension_list;

	/// @brief インポート時などで一時的に作成する際のバッファ
	amiga_block_pre_t  *m_temp_pre;
	/// @brief インポート時などで一時的に作成する際のバッファ
	amiga_block_post_t *m_temp_post;

	/// @brief チェイン情報
	AmigaChain m_chain;

	/// @brief ディレクトリ情報をアロケート
	void			AllocData(DiskD88Sector *n_sector, wxUint8 *n_data);
	/// @brief バッファをアロケート
	void			AllocTemp();
	/// @brief バッファを削除
	void			DeleteTemp();

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 属性３を返す protect
	virtual int		GetFileType3() const;
	/// @brief 属性３のセット protect
	virtual void	SetFileType3(int val);

	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 共通属性を個別属性に変換
	static int		ConvToFileType1(int ftype);
	/// @brief 個別属性を共通属性に変換
	static int		ConvFromFileType1(int type1);
	/// @brief 個別属性を共通属性に変換
	static int		ConvFromFileType2(int type2);
	/// @brief 属性１からリストの位置を返す
	int				ConvFileType1Pos(int type1) const;

	/// @brief ファイルサイズを返す
	wxUint32		GetByteSize() const;

	/// @brief ユーザＩＤを返す
	wxUint16		GetUserID() const;
	/// @brief ユーザＩＤをセット
	void			SetUserID(wxUint16 val);
	/// @brief グループＩＤを返す
	wxUint16		GetGroupID() const;
	/// @brief グループＩＤをセット
	void			SetGroupID(wxUint16 val);

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemAmiga(DiskBasic *basic);
	DiskBasicDirItemAmiga(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemAmiga(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);
	~DiskBasicDirItemAmiga();

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);

	/// @brief header用にセクタを初期化
	void			InitForHeaderBlock(wxUint32 parent_num);
	/// @brief extension用にセクタを初期化
	void			InitForExtensionBlock(wxUint32 parent_num);

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief 削除
	virtual bool	Delete();
	/// @brief ハードリンクを削除
	bool			DeleteHardLink();

	/// @brief ファイル名＋拡張子のサイズ
	virtual int		GetFileNameStrSize() const;
	/// @brief 小文字を大文字にする
	virtual void	ToUpper(wxUint8 *str, size_t size) const;
	/// @brief 小文字を大文字にする
	static wxUint8	Upper(wxUint8 ch, bool is_intr);

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
	/// @brief ファイルのすべてのグループを取得
	bool			GetFileGroups(const wxUint32 *tables, int table_nums, int limit, DiskBasicGroups *group_items, wxArrayInt *extension_list);
	/// @brief ディレクトリ内のすべてのグループを取得
	static bool		GetDirectoryGroups(DiskBasic *basic, wxUint32 *tables, int table_size, int limit, DiskBasicGroups *group_items);
	/// @brief インデックス番号をリナンバ
	static void		RenumberInDirectory(DiskBasic *basic, DiskBasicDirItems *items);
	/// @brief アイテムリストにアイテムを挿入
	static bool		InsertItemInDirectory(DiskBasic *basic, const wxUint32 *tables, int table_size, int limit, DiskBasicDirItems *items, DiskBasicDirItem *item);
	/// @brief アイテムリストからアイテムを削除
	static bool		DeleteItemInDirectory(DiskBasic *basic, wxUint32 *tables, int table_size, int limit, DiskBasicDirItems *items, DiskBasicDirItem *item);
	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief ヘッダのあるグループ番号をセット(機種依存)
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief ヘッダのあるグループ番号を返す(機種依存)
	virtual wxUint32 GetExtraGroup() const;
	/// @brief ヘッダのグループリストをセット(機種依存)
	virtual void	SetExtraGroups(const DiskBasicGroups &grps);
	/// @brief ヘッダのグループ番号を得る(機種依存)
	virtual void	GetExtraGroups(wxArrayInt &arr) const;
	/// @brief ヘッダのグループリストを返す(機種依存)
	virtual void	GetExtraGroups(DiskBasicGroups &grps) const;
	/// @brief 次のグループ番号をセット(機種依存)
	virtual void	SetNextGroup(wxUint32 val);
	/// @brief 次のグループ番号を返す(機種依存)
	virtual wxUint32 GetNextGroup() const;

	/// @brief ストアしたブロック数をセット
	void			SetHighSeq(wxUint32 val);
	/// @brief ブロックテーブルを返す
	wxUint32 *		GetBlockTable() const;
	/// @brief ブロック番号を返す
	wxUint32		GetDataBlock(int idx) const;
	/// @brief ブロック番号をセット
	void			SetDataBlock(int idx, wxUint32 val);
	/// @brief 1ヘッダで格納できるブロック数を返す
	int				GetDataBlockNums() const;
	/// @brief ハッシュテーブルにセット（必要ならチェインをたどる）
	bool			ChainHashNumber(int idx, wxUint32 val, DiskBasicDirItem *item);
	/// @brief ハッシュ番号を返す
	int				GetHashNumber() const;

	/// @brief エクステンションをセット
	void			SetExtension(wxUint32 val);
	/// @brief エクステンションを返す
	wxUint32		GetExtension() const;

	/// @brief アイテムが変更日時を持っているか
	virtual bool	HasModifyDateTime() const { return true; }
	/// @brief アイテムが変更日付を持っているか
	virtual bool	HasModifyDate() const { return true; }
	/// @brief アイテムが変更時間を持っているか
	virtual bool	HasModifyTime() const { return true; }
	/// @brief 日時を返す（ファイルリスト用）
	virtual wxString GetFileDateTimeStr() const;
	/// @brief 更新日付を返す
	virtual void	GetFileModifyDate(TM &tm) const;
	/// @brief 更新時間を返す
	virtual void	GetFileModifyTime(TM &tm) const;
	/// @brief 更新日付を返す
	virtual wxString GetFileModifyDateStr() const;
	/// @brief 更新時間を返す
	virtual wxString GetFileModifyTimeStr() const;
	/// @brief 更新日付をセット
	virtual void	SetFileModifyDate(const TM &tm);
	/// @brief 更新時間をセット
	virtual void	SetFileModifyTime(const TM &tm);
	/// @brief 日付を変換
	static void		ConvDateToTm(wxUint32 days, TM &tm);
	/// @brief 時間を変換
	static void		ConvTimeToTm(wxUint32 mins, wxUint32 ticks, TM &tm);
	/// @brief 日付を変換
	static void		ConvDateFromTm(const TM &tm, wxUint32 &days);
	/// @brief 時間を変換
	static void		ConvTimeFromTm(const TM &tm, wxUint32 &mins, wxUint32 &ticks);

//	/// @brief ファイル名、属性をコピー
//	virtual void	CopyItem(const DiskBasicDirItem &src);
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
	/// @brief チェックサムを更新する
	void			UpdateCheckSumAll();
	/// @brief チェックサムを更新する
	void			UpdateCheckSum();
	/// @brief チェックサムを計算
	static wxUint32	CalcCheckSum(const void *data, int size);
	/// @brief チェックサムを計算
	static void		CalcCheckSum(const void *data, int size, wxUint32 &sum_data);

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

#endif /* _BASICDIRITEM_AMIGA_H_ */
