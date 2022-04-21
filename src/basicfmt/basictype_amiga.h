/// @file basictype_amiga.h
///
/// @brief disk basic type for Amiga DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_AMIGA_H_
#define _BASICTYPE_AMIGA_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"
#include "basicdiritem_amiga.h"


//////////////////////////////////////////////////////////////////////

#pragma pack(1)
/// @brief Amiga Boot Block (all Big Endien)
typedef struct st_amiga_boot_block {
	wxUint8  type[4];			///< "DOS", byte4 is type: bit0 0:OFS 1:FFS
	wxUint32 check_sum;
	wxUint32 root_block;		///< number of root block
	wxUint8  program[500];		///< boot program
} amiga_boot_block_t;

/// @brief Amiga Bitmap Block (all Big Endien)
typedef struct st_amiga_bitmap_block {
	wxUint32 check_sum;
	wxUint32 map[1];			///< block size - 4
} amiga_bitmap_block_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

/// @brief AMIGA ビットマップ １つ
class AmigaOneBitmap
{
private:
	wxUint32 m_block_num;
	int      m_block_size;
	amiga_bitmap_block_t *m_map;

public:
	AmigaOneBitmap();
	AmigaOneBitmap(wxUint32 block_num, void *map, int block_size);
	~AmigaOneBitmap();

	/// @brief 指定位置のビットを変更する
	void Modify(wxUint32 block_num, bool use);
	/// @brief 指定位置が空いているか
	bool IsFree(wxUint32 block_num) const;
	/// @brief 指定ブロックまですべて未使用にする
	void FreeAll(wxUint32 block_num);
	/// @brief ブロック番号をセット
	void SetBlockNumber(wxUint32 val) { m_block_num = val; }
	/// @brief ブロック番号を返す
	wxUint32 GetBlockNumber() const { return m_block_num; }
	/// @brief ブロックサイズをセット
	void SetBlockSize(int val) { m_block_size = val; }
	/// @brief ブロックサイズを返す
	int GetBlockSize() const { return m_block_size; }
	/// @brief ブロック数を返す
	wxUint32 GetBlockNums() const;
	/// @brief チェックサムの更新
	void UpdateCheckSum();
};

WX_DECLARE_OBJARRAY(AmigaOneBitmap, ArrayOfAmigaBitmap);

/// @brief AMIGA ビットマップ AmigaOneBitmapの配列
class AmigaBitmap : public ArrayOfAmigaBitmap
{
public:
	AmigaBitmap();
	~AmigaBitmap();

	/// @brief ビットマップを追加
	void AddBitmap(wxUint32 block_num, void *map, int block_size);
	/// @brief 指定位置のビットを変更する
	void Modify(wxUint32 block_num, bool use);
	/// @brief 指定位置が空いているか
	bool IsFree(wxUint32 block_num) const;
	/// @brief 指定ブロックまですべて未使用にする
	void FreeAll(wxUint32 block_num);
	/// @brief ブロック数を返す
	wxUint32 GetBlockNums() const;
	/// @brief チェックサムの更新
	void UpdateCheckSum();
};

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicTypeAmiga

@brief Amiga DOS の処理

DiskBasicParam
@li FastFileSystem FFSかどうか(bool)

*/
class DiskBasicTypeAmiga : public DiskBasicType
{
private:
	DiskBasicTypeAmiga() : DiskBasicType() {}
	DiskBasicTypeAmiga(const DiskBasicType &src) : DiskBasicType(src) {}

	directory_amiga_t m_root;				///< Root Block
	AmigaBitmap       m_bitmap;				///< Bitmap Blocks

	/// @brief 空きFAT位置を返す
	wxUint32		GetEmptyGroupNumberM();
//	/// @brief 次の空きFAT位置を返す
//	wxUint32		GetDirNextEmptyGroupNumber(wxUint32 curr_group);
//	/// @brief 最終グループをつなげる
//	int				ChainLastGroup(wxUint32 group_num, int remain);
	/// @brief ファイル名からハッシュ番号を生成する
	int				CreateHashNumberFromName(wxUint8 *name, size_t size);
//	/// @brief ディレクトリアイテムを入れる
//	void			InsertDirItem(DiskBasicDirItem *item);
	
public:
	DiskBasicTypeAmiga(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	~DiskBasicTypeAmiga();

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	virtual void	SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FATオフセットを返す
	virtual wxUint32 GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループ番号か
	virtual bool	IsUsedGroupNumber(wxUint32 num);
	/// @brief 次のグループ番号を得る
	virtual wxUint32 GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// @brief 空きFAT位置を返す
	virtual wxUint32 GetEmptyGroupNumber();
	/// @brief 次の空きFAT位置を返す
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	/// @brief Allocation Mapの開始位置を得る（ダイアログ用）
	virtual void	GetStartNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief Allocation Mapの終了位置を得る（ダイアログ用）
	virtual void	GetEndNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief "Allocation Map"タイトル名（ダイアログ用）
	virtual wxString GetTitleForFat() const;
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリのセクタリストを計算
	virtual bool	CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	/// @brief ルートディレクトリをアサイン
	virtual bool	AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief ディレクトリをアサイン
	virtual bool	AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief セクタをディレクトリとして初期化
	virtual int		InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &sizeremain, DiskBasicError &errinfo);
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	virtual int		FinishAssigningDirectory(int &pos, int &size, int &size_remain) const;
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	virtual void	GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
	/// @brief ファイルをセーブする前の準備を行う
	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief グループをつなげる
	virtual int		ChainGroups(wxUint32 group_num, wxUint32 append_group_num);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// @brief 未使用のディレクトリアイテムを返す
	virtual DiskBasicDirItem *GetEmptyDirectoryItem(DiskBasicDirItem *parent, DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return true; }
	/// @brief サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// @brief フォーマット時セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	/// @brief Bootブロックのチェックサムを計算
	static wxUint32	CalcCheckSumOnBootBlock(wxUint32 sum, const wxUint8 *data, size_t size);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief データの読み込み/比較処理
	virtual int		AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	/// @brief ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// @brief 指定したサイズが十分書き込めるか
	virtual bool	IsEnoughFileSize(int size) const;
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	/// @brief データの書き込み終了後の処理
	virtual void	AdditionalProcessOnSavedFile(DiskBasicDirItem *item);
	/// @brief ファイル名変更後の処理
	virtual void	AdditionalProcessOnRenamedFile(DiskBasicDirItem *item);
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
	/// @brief ファイル削除後の処理
	virtual bool	AdditionalProcessOnDeletedFile(DiskBasicDirItem *item);
	/// @brief ディレクトリアイテムを削除する
	void			ReleaseDirectoryItem(DiskBasicDirItem *item);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	/// @brief ルートのチェックサムを計算
	void			UpdateCheckSumOnRoot();
	/// @brief ルートの更新日時をセット
	void			SetModifyDateTime(const TM &tm);
	/// @brief ルートのボリューム日時をセット
	void			SetVolumeDateTime(const TM &tm);
	/// @brief ルートの作成日時をセット
	void			SetCreateDateTime(const TM &tm);
	//@}
};

#endif /* _BASICTYPE_AMIGA_H_ */
