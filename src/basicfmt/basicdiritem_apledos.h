/// @file basicdiritem_apledos.h
///
/// @brief disk basic directory item for Apple DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_APLEDOS_H
#define BASICDIRITEM_APLEDOS_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

/// @brief Apple DOS属性位置
enum en_type_name_appledos {
	TYPE_NAME_APLEDOS_TEXT = 0,
	TYPE_NAME_APLEDOS_IBASIC = 1,	// Integer BASIC
	TYPE_NAME_APLEDOS_ABASIC = 2,	// Applesoft BASIC
	TYPE_NAME_APLEDOS_BINARY = 3,
	TYPE_NAME_APLEDOS_READ_ONLY = 4,
};
/// @brief Apple DOS属性値
enum en_file_type_mask_appledos {
	FILETYPE_MASK_APLEDOS_TEXT = 0x00,
	FILETYPE_MASK_APLEDOS_IBASIC = 0x01,
	FILETYPE_MASK_APLEDOS_ABASIC = 0x02,
	FILETYPE_MASK_APLEDOS_BINARY = 0x04,
	FILETYPE_MASK_APLEDOS_READ_ONLY = 0x80,
};

//////////////////////////////////////////////////////////////////////

#define APLEDOS_TRACK_LIST_MAX	122

#pragma pack(1)
/// @brief Apple DOS トラックセクタリスト情報 256bytes
typedef struct st_apledos_chain {
	apledos_ptr_t	next;
	wxUint8			reserved1[2];
	wxUint16		number;
	wxUint8			reserved2[5];
	struct {
		wxUint8		track;
		wxUint8		sector;
	} list[APLEDOS_TRACK_LIST_MAX];
} apledos_chain_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

WX_DEFINE_ARRAY(apledos_chain_t *, ArrayOfAppleDOSChain);

//////////////////////////////////////////////////////////////////////

/// @brief Apple DOS トラックセクタリストの各セクタ
///
/// @sa apledos_chain_t
class AppleDOSChains : public ArrayOfAppleDOSChain
{
private:
	bool chain_ownmake;
public:
	AppleDOSChains();
	~AppleDOSChains();

	void Clear();
	void Alloc();
};

//////////////////////////////////////////////////////////////////////

/// @brief Apple DOS トラックセクタリスト
class DiskBasicDirItemAppleDOSChain
{
private:
	DiskBasic			*basic;
	AppleDOSChains		chains;

	DiskBasicDirItemAppleDOSChain(const DiskBasicDirItemAppleDOSChain &src);

public:
	DiskBasicDirItemAppleDOSChain();
	~DiskBasicDirItemAppleDOSChain();
#ifdef COPYABLE_DIRITEM
	/// @brief 代入
	DiskBasicDirItemAppleDOSChain &operator=(const DiskBasicDirItemAppleDOSChain &src);
	/// @brief 複製
	void Dup(const DiskBasicDirItemAppleDOSChain &src);
#endif
	/// @brief BASICをセット
	void SetBasic(DiskBasic *n_basic);
	/// @brief ポインタをセット
	void Add(apledos_chain_t *n_chain);
	/// @brief メモリ確保
	void Alloc();
	/// @brief クリア
	void Clear();
	/// @brief セクタ数を返す
	int  Count() const;
	/// @brief 有効か
	bool IsValid() const { return (chains.Count() > 0); }
	/// @brief トラック＆セクタを返す
	void GetTrackAndSector(int idx, int &track, int &sector) const;
	/// @brief トラック＆セクタを設定
	void SetTrackAndSector(int idx, int track, int sector);
	/// @brief 次のセクタのあるセクタ番号を得る
	int	 GetNext(int idx) const;
	/// @brief 次のセクタのあるセクタ番号を設定
	void SetNext(int idx, int val);
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Apple DOS 3.x
class DiskBasicDirItemAppleDOS : public DiskBasicDirItem
{
private:
	DiskBasicDirItemAppleDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemAppleDOS(const DiskBasicDirItemAppleDOS &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_apledos_t> m_data;

	int m_start_address;	///< ファイル内部で持っている開始アドレス
	int m_data_length;		///< ファイル内部で持っているサイズ

	/// @brief トラック＆セクタリスト
	DiskBasicDirItemAppleDOSChain chain;

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

	/// @brief ファイル内部のアドレスを取り出す
	void			TakeAddressesInFile(DiskBasicGroups &group_items);

	/// @brief 共通属性を個別属性に変換
	static int		ConvToFileType1(int ftype);
	/// @brief 個別属性を共通属性に変換
	static int		ConvFromFileType1(int type1);
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				ConvFileType1Pos(int type1) const;

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1);

public:
	DiskBasicDirItemAppleDOS(DiskBasic *basic);
	DiskBasicDirItemAppleDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemAppleDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
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
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;

	/// @brief 最初のトラック番号をセット
	void			SetStartTrack(wxUint8 val);
	/// @brief 最初のセクタ番号をセット
	void			SetStartSector(wxUint8 val);
	/// @brief 最初のトラック番号を返す
	wxUint8			GetStartTrack() const;
	/// @brief 最初のセクタ番号を返す
	wxUint8			GetStartSector() const;

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }
	/// @brief アイテムが実行アドレスを持っているか
	virtual bool	HasExecuteAddress() const { return false; }
	/// @brief アドレスを編集できるか
	virtual bool	IsAddressEditable() const { return false; }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 終了アドレスを返す
	virtual int		GetEndAddress() const;

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

#endif /* BASICDIRITEM_APLEDOS_H */
