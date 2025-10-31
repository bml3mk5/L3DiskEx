/// @file basicdiritem_hfs.h
///
/// @brief disk basic directory item for HFS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_HFS_H
#define BASICDIRITEM_HFS_H

#include "basicdiritem.h"
#include "basictype_hfs.h"


//////////////////////////////////////////////////////////////////////

/// @brief HFS属性位置
enum en_type_name_hfs {
	TYPE_NAME_HFS_DIR = 0,
	TYPE_NAME_HFS_FILE_DATA = 1,
	TYPE_NAME_HFS_FILE_RES = 2
};
/// @brief HFS属性値
enum en_file_type_hfs {
	FILETYPE_HFS_INVALID = 0,
	FILETYPE_HFS_DIR  = 1,			// directory record
	FILETYPE_HFS_FILE = 2,			// file record
	FILETYPE_HFS_DIR_THREAD = 3,	// directory thread record
	FILETYPE_HFS_FILE_THREAD = 4	// file thread record
};
/// @brief HFS属性値2
enum en_file_type_hfs_2 {
	FILETYPE_HFS_DATA_FORK = 0,		// data fork
	FILETYPE_HFS_RES_FORK = 0xff	// resource fork
};

enum en_hfs_node_types {
   ndIndxNode     =  0x00, // index node
   ndHdrNode      =  0x01, // header node
   ndMapNode      =  0x02, // map node
   ndLeafNode     =  0xFF, // leaf node
};

//////////////////////////////////////////////////////////////////////

#pragma pack(1)
// Finder File Information
typedef struct st_hfs_f_file_info {
	wxUint32 type;		// OSType file type
	wxUint32 creator;	// OSType file creator
	wxUint16 flags;		// finder flags
	struct {
		wxInt16	v;
		wxInt16 h;
	} location;			// file's location
	wxInt16	reserved;
} hfs_f_file_info_t;

// Finder Folder Information
typedef struct st_hfs_f_dir_info {
	struct {
		wxInt16	top;
		wxInt16	left;
		wxInt16	bottom;
		wxInt16	right;
	} rect;				// folder's window rectangle
	wxUint16	flags;	// finder flags
	struct {
		wxInt16	v;
		wxInt16	h;
	} location;			// folder's location
	wxInt16	reserved;
} hfs_f_dir_info_t;

// Finder Information
typedef struct st_hfs_fndr_info {
	wxUint8 reserved[16];
} hfs_fndr_info_t;

// B*-tree Node Descriptor
typedef struct st_hfs_node_descriptor {
	wxUint32 fLink;			// forward link
	wxUint32 bLink;			// backward link
	wxUint8  type;			// node type
	wxUint8  height;			// node level
	wxUint16 numRecs;		// number of records in node
	wxUint16 reserved;		// reserved
} hfs_node_descriptor_t;

// B*-tree Header
typedef struct st_hfs_bt_hdr_rec {
	wxUint16 treeDepth;		// current depth of tree
	wxUint32 rootNode;		// node number of root node
	wxUint32 leafRecs;		// number of leaf records in tree
	wxUint32 firstLeafNode;	// number of first leaf node
	wxUint32 lastLeafNode;	// number of last leaf node
	wxUint16 nodeSize;		// size of a node
	wxUint16 maxKeyLen;		// maximum length of a key}
	wxUint32 totalNodes;		// total number of nodes in tree
	wxUint32 freeNodes;		// number of free nodes
	wxUint8  reserved[76];	// reserved
} hfs_bt_hdr_rec_t;

// Catalog Key Record
typedef struct st_hfs_cat_key_rec {
	wxUint8	 keyLength;		// key length
	wxUint8	 reserved;		// reserved
	wxUint32 parentID;		// parent directory ID
	wxUint8  nodeName[32];	// catalog node name
} hfs_cat_key_rec_t;

// Fork Size
typedef struct st_hfs_cat_size {
	wxUint16 StartBlock;		// first alloc. blk. of data fork
	wxUint32 LogicalSize;		// logical EOF of data fork
	wxUint32 PhysicalSize;		// physical EOF of data fork
} hfs_cat_size_t;

// Catalog File Record
typedef struct st_hfs_cat_file_rec {
	wxUint8	recType;		// record type
	wxUint8	reserved0;		// reserved
	wxUint8  flags;					// file flags
	wxUint8	 fileType;				// file type
	hfs_f_file_info_t userInfo;		// Finder information
	wxUint32 id;					// file ID
	hfs_cat_size_t datF;			// data fork
	hfs_cat_size_t resF;			// resource fork
	wxUint32 createDate;			// date and time of creation
	wxUint32 modifyDate;			// date and time of last modification
	wxUint32 backupDate;			// date and time of last backup
	hfs_fndr_info_t finderInfo;		// additional Finder information
	wxUint16 clumpSize;				// file clump size
	hfs_ext_data_rec_t datExts;		// first data fork extent record
	hfs_ext_data_rec_t resExts;		// first resource fork extent record
	wxUint32 reserved;				// reserved
} hfs_cat_file_rec_t;

// Directory Record
typedef struct st_hfs_cat_dir_rec {
	wxUint8	recType;		// record type
	wxUint8	reserved0;		// reserved
	wxUint16 flags;					// directory flags
	wxUint16 valence;				// directory valence
	wxUint32 id;					// directory ID
	wxUint32 createDate;			// date and time of creation
	wxUint32 modifyDate;			// date and time of last modification
	wxUint32 backupDate;			// date and time of last backup
	hfs_f_dir_info_t UsrInfo;		// Finder information
	hfs_fndr_info_t finderInfo;		// additional Finder information
	wxUint32 reserved[4];			// reserved
} hfs_cat_dir_rec_t;

// Directory/File Thread Record
typedef struct st_hfs_cat_thread_rec {
	wxUint8	recType;		// record type
	wxUint8	reserved0;		// reserved
	wxUint32 reserved[2];			// reserved
	wxUint32 parentID;				// parent ID for this directory/file
	wxUint8  nodeName[32];			// name of this directory/file
} hfs_cat_thread_rec_t;

// Catalog Data Records
typedef union st_hfs_cat_data_rec {
	struct {
		wxUint8	recType;		// record type
		wxUint8	reserved0;		// reserved
	};
	hfs_cat_dir_rec_t    dir;		// direcotry record
	hfs_cat_file_rec_t   file;		// file record
	hfs_cat_thread_rec_t thread;	// thread record
} hfs_cat_data_rec_t;

// Extend Overflow Key Record
typedef struct st_hfs_ext_key_rec {
	wxUint8	 keyLength;		// key length
	wxUint8	 forkType;		// fork type
	wxUint32 id;			// file id
	wxUint16 start;			// starting file allocation block
} hfs_ext_key_rec_t;
#pragma pack()

typedef struct st_hfs_ptr {
	hfs_cat_key_rec_t  *key;
	hfs_cat_data_rec_t *data;
} hfs_catalog_t;


//////////////////////////////////////////////////////////////////////

//WX_DEFINE_ARRAY(hfs_chain_t *, ArrayOfAppleDOSChain);

//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////

#if 0
class DiskBasicDirDataHFS
{
private:
	/// @brief 元レコードへのポインタ
	hfs_catalog_t	m_data;
	/// @brief ファイル属性 (フォーク種類 + ファイル・ディレクトリ)
	int				m_type;

public:
	DiskBasicDirDataHFS();
	~DiskBasicDirDataHFS();
	/// @brief レコードのキー部分のポインタをセット
	void	SetKeyPtr(void *key);
	/// @brief レコードのデータ部分のポインタをセット
	void	SetDataPtr(void *data);
	/// @brief データを返す
	const hfs_catalog_t &Data() const;
	/// @brief 有効か
	bool	IsValid() const;
	/// @brief 属性１を返す
	int		GetType1() const;
	/// @brief 属性１のセット
	void	SetType1(int val);
};
#endif

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム HFS
class DiskBasicDirItemHFS : public DiskBasicDirItem
{
private:
	DiskBasicDirItemHFS() : DiskBasicDirItem() {}
	DiskBasicDirItemHFS(const DiskBasicDirItemHFS &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<hfs_cat_key_rec_t>	 m_key;
	DiskBasicDirData<hfs_cat_data_rec_t> m_data;
	/// @brief ファイル属性 (フォーク種類 + ファイル・ディレクトリ)
	int				m_type;

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
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief セクタカウントを返す(機種依存)
	void			SetSectorCount(wxUint16 val);
	/// @brief セクタカウントを返す(機種依存)
	wxUint16		GetSectorCount() const;

//	/// @brief ファイル内部のアドレスを取り出す
//	void			TakeAddressesInFile(DiskBasicGroups &group_items);

	/// @brief 共通属性を個別属性に変換
	static int		ConvToFileType1(int ftype);
	/// @brief 個別属性を共通属性に変換
	static int		ConvFromFileType1(int type1);
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				ConvFileType1Pos(int type1) const;

	/// @brief ディレクトリのすべてのグループを取得
	void			GetUnitGroupsDir(int fileunit_num, const hfs_cat_dir_rec_t *dir, DiskBasicGroups &group_items);
	/// @brief ファイルのすべてのグループを取得
	void			GetUnitGroupsFile(int fileunit_num, int fork_type, const hfs_cat_file_rec_t *file, const hfs_cat_size_t *fsize, const hfs_ext_data_rec_t *exts, DiskBasicGroups &group_items);
	/// @brief ファイルの拡張オーバフローグループを取得
	void			GetUnitGroupsFileExt(int fileunit_num, int fork_type, const hfs_cat_file_rec_t *file, const hfs_cat_size_t *fsize, const hfs_ext_data_rec_t *exts, DiskBasicGroups &group_items);
	/// @brief 拡張レコードからグループを取得
	void			GetGroupsFromExtDataRec(const hfs_ext_data_rec_t *exts, DiskBasicGroups &group_items);

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1);

public:
	DiskBasicDirItemHFS(DiskBasic *basic);
	DiskBasicDirItemHFS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemHFS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief 削除
	virtual bool	Delete();
	/// @brief アイテムをロード・エクスポートできるか
	virtual bool	IsLoadable() const;
	/// @brief アイテムをコピー(内部でDnD)できるか
	virtual bool	IsCopyable() const;
	/// @brief アイテムを上書きできるか
	virtual bool	IsOverWritable() const;

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
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
	/// @brief 追加のグループ番号を返す(機種依存)
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る(機種依存)
	virtual void	GetExtraGroups(wxArrayInt &arr) const;
	/// @brief チェイン用のセクタをクリア(機種依存)
	virtual void	ClearChainSector(const DiskBasicDirItem *pitem = NULL);
	/// @brief チェイン用のセクタをセット(機種依存)
	virtual void	SetChainSector(DiskImageSector *sector, wxUint32 gnum, wxUint8 *data, const DiskBasicDirItem *pitem = NULL);
	/// @brief チェイン用のセクタにグループ番号をセット(機種依存)
	virtual void	AddChainGroupNumber(int idx, wxUint32 val);

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);
	
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

	/// @brief 最初のトラック番号をセット
	void			SetStartTrack(wxUint8 val);
	/// @brief 最初のセクタ番号をセット
	void			SetStartSector(wxUint8 val);
	/// @brief 最初のトラック番号を返す
	wxUint8			GetStartTrack() const;
	/// @brief 最初のセクタ番号を返す
	wxUint8			GetStartSector() const;

	/// @brief アイテムが作成日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	/// @brief アイテムが作成日付を持っているか
	virtual bool	HasCreateDate() const { return true; }
	/// @brief アイテムが作成時間を持っているか
	virtual bool	HasCreateTime() const { return true; }
	/// @brief アイテムが変更日時を持っているか
	virtual bool	HasModifyDateTime() const { return true; }
	/// @brief アイテムが変更日付を持っているか
	virtual bool	HasModifyDate() const { return true; }
	/// @brief アイテムが変更時間を持っているか
	virtual bool	HasModifyTime() const { return true; }
	/// @brief アイテムがアクセス日時を持っているか(HFSでは最終バックアップ日時)
	virtual bool	HasAccessDateTime() const { return true; }
	/// @brief アイテムがアクセス日付を持っているか(HFSでは最終バックアップ日時)
	virtual bool	HasAccessDate() const { return true; }
	/// @brief アイテムがアクセス時間を持っているか(HFSでは最終バックアップ日時)
	virtual bool	HasAccessTime() const { return true; }

	/// @brief 作成日付を得る
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 作成時間を得る
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 作成日時を得る
	virtual void	GetFileCreateDateTime(TM &tm) const;
	/// @brief 作成日時を返す
	virtual TM		GetFileCreateDateTime() const;
	/// @brief 作成日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 作成時間を返す
	virtual wxString GetFileCreateTimeStr() const;
//	/// @brief 作成日時を返す
//	virtual wxString GetFileCreateDateTimeStr() const;
//	/// @brief 作成日付のタイトル名（ダイアログ用）
//	virtual wxString GetFileCreateDateTimeTitle() const;
	/// @brief 変更日付を得る
	virtual void	GetFileModifyDate(TM &tm) const;
	/// @brief 変更時間を得る
	virtual void	GetFileModifyTime(TM &tm) const;
	/// @brief 変更日時を得る
	virtual void	GetFileModifyDateTime(TM &tm) const;
	/// @brief 変更日時を返す
	virtual TM		GetFileModifyDateTime() const;
	/// @brief 変更日付を返す
	virtual wxString GetFileModifyDateStr() const;
	/// @brief 変更時間を返す
	virtual wxString GetFileModifyTimeStr() const;
//	/// @brief 変更日時を返す
//	virtual wxString GetFileModifyDateTimeStr() const;
//	/// @brief 変更日付のタイトル名（ダイアログ用）
//	virtual wxString GetFileModifyDateTimeTitle() const;
	/// @brief アクセス日付を得る
	virtual void	GetFileAccessDate(TM &tm) const;
	/// @brief アクセス時間を得る
	virtual void	GetFileAccessTime(TM &tm) const;
	/// @brief アクセス日時を得る
	virtual void	GetFileAccessDateTime(TM &tm) const;
	/// @brief アクセス日時を返す
	virtual TM		GetFileAccessDateTime() const;
	/// @brief アクセス日付を返す
	virtual wxString GetFileAccessDateStr() const;
	/// @brief アクセス時間を返す
	virtual wxString GetFileAccessTimeStr() const;
//	/// @brief アクセス日時を返す
//	virtual wxString GetFileAccessDateTimeStr() const;
	/// @brief アクセス日付のタイトル名（ダイアログ用）
	virtual wxString GetFileAccessDateTimeTitle() const;

	/// @brief HFS日付をTMに変換
	static bool ConvHFSDateToTm(wxUint32 src, TM &dst);
	/// @brief HFS時間をTMに変換
	static bool ConvHFSTimeToTm(wxUint32 src, TM &dst);
	/// @brief TMをHFS日時に変換
	static bool ConvTmToHFSDateTime(const TM &src, wxUint32 &dst);

//	/// @brief アイテムがアドレスを持っているか
//	virtual bool	HasAddress() const { return false; }
//	/// @brief アイテムが実行アドレスを持っているか
//	virtual bool	HasExecuteAddress() const { return false; }
//	/// @brief アドレスを編集できるか
//	virtual bool	IsAddressEditable() const { return false; }
//	/// @brief 開始アドレスを返す
//	virtual int		GetStartAddress() const;
//	/// @brief 終了アドレスを返す
//	virtual int		GetEndAddress() const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* BASICDIRITEM_HFS_H */
