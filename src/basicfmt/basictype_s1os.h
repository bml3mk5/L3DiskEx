/// @file basictype_s1os.h
///
/// @brief disk basic type for S1-OS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_S1OS_H
#define BASICTYPE_S1OS_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"

#pragma pack(1)
/// S1-OS Volume Information (track0, sector2)
typedef struct st_s1os_volume_info {
	wxUint16	header;
	wxUint8		unknown[4];
	wxUint32	wrote_sectors;
	s1os_size_t root_size;
	s1os_ptr_t	root_start;
	wxUint32	date_time;
	s1os_attr_t	attr;
	wxUint8		name_len;
	wxUint8		name[64];
} s1os_volume_info_t;

/// S1-OS Skew and Bitmap (track0, sector3, 4)
typedef struct st_s1os_skew_bitmap {
	wxUint16	skew[26];
	wxUint8		bitmap; // 26 * 2 * 77 = 4004 bits
} s1os_skew_bitmap_t;
#pragma pack()

/** @class DiskBasicTypeS1OS

@brief S1-OS の処理

DiskBasicParam 固有のパラメータ
@li	IPLString : セクタ1のIPL

*/
class DiskBasicTypeS1OS : public DiskBasicType
{
private:
	DiskBasicTypeS1OS() : DiskBasicType() {}
	DiskBasicTypeS1OS(const DiskBasicType &src) : DiskBasicType(src) {}

	enum {
		MAX_SECTORS = 32
	};

	wxUint8 m_skew_map[MAX_SECTORS]; ///< スキューマップ
	wxUint8 m_skew_rmap[MAX_SECTORS]; ///< スキューマップ逆引き
	DiskBasicBitMLMap m_bitmap;	///< 使用状況テーブル

	s1os_volume_info_t *p_volume;	///< ボリューム情報
//	wxUint32 m_root_pos;	///< ルートディレクトリのあるセクタ位置
	wxUint32 m_soft_interleave;		///< スキュ（ソフトインターリーブ）

	/// @brief 番号をスキューマップを適用した後のグループ番号に変換
	wxUint32 ToGroupNum(wxUint32 sgnum);
	/// @brief グループ番号をスキューマップを適用する前の番号に変換
	wxUint32 FromGroupNum(wxUint32 group_num);
	/// @brief グループからトラック、サイド、セクタの各番号を得る
	void	GetNumFromGroupNum(wxUint32 group_num, int &track_num, int &side_num, int &sector_num);
	/// @brief トラック、サイド、セクタ番号からグループ番号を得る
	wxUint32 GetGroupNumFromNum(int track_num, int side_num, int sector_num);
	
public:
	DiskBasicTypeS1OS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
//	/// @brief 使用しているグループの位置を得る
//	virtual void	CalcUsedGroupPos(wxUint32 num, int &pos, int &mask);
	/// @brief FAT位置をセット
	virtual void	SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
	virtual wxUint32 GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループ番号か
	virtual bool	IsUsedGroupNumber(wxUint32 num);
//	/// @brief 次のグループ番号を得る
//	virtual wxUint32 GetNextGroupNumber(wxUint32 num, int sector_pos);
//	/// @brief 空きFAT位置を返す
//	virtual wxUint32 GetEmptyGroupNumber();
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリのセクタリストを計算
	virtual bool	CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	/// @brief ルートディレクトリをアサイン
	virtual bool	AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
//	/// @brief ディレクトリをアサイン
//	virtual bool	AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief セクタをディレクトリとして初期化
	virtual int		InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &sizeremain, DiskBasicError &errinfo);
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	virtual int		FinishAssigningDirectory(int &pos, int &size, int &size_remain) const;
//	/// @brief ディレクトリアサインでセクタ毎に位置を調整する
//	virtual int     AdjustPositionAssigningDirectory(int pos);
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	virtual void	GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file chain
	//@{
	/// @brief 指定ディレクトリのすべてのグループを取得
	void			CountGroups(s1os_size_t *size, s1os_ptr_t *ptr, DiskBasicGroups &group_items);
//	/// @brief ファイルをセーブする前の準備を行う
//	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief 未使用領域をさがす
	bool			FindEmptySectors(wxUint32 group_start, int remain, int sec_size, wxArrayInt &reserved_groups);
	/// @brief グループを確保して使用中にする
	int				AllocateSectors(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, const wxArrayInt &reserved_groups, DiskBasicGroups &group_items, int &file_size, int &groups);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);

//	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
//	virtual void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
//	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
//	virtual int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return true; }
	/// @brief ルートディレクトリのサイズを拡張できるか
	virtual bool	CanExpandRootDirectory() const { return true; }
	/// @brief サブディレクトリのサイズを拡張できるか
	virtual bool	CanExpandDirectory() const { return true; }
//	/// @brief 未使用のディレクトリアイテムを返す
//	virtual DiskBasicDirItem *GetEmptyDirectoryItem(DiskBasicDirItem *parent, DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item);
	/// @brief サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	/// @brief ディレクトリ拡張後の個別処理
	virtual bool	AdditionalProcessOnExpandedDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief データの読み込み/比較処理
	virtual int		AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	//@}

	/// @name save / write
	//@{
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, DiskBasicGroupItem *group_item, DiskBasicGroupItem *next_group, int sector_end, int seq_num);
	/// @brief データの書き込み終了後の処理
	virtual void	AdditionalProcessOnSavedFile(DiskBasicDirItem *item);
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のチェインを削除する
	virtual void	DeleteGroups(const DiskBasicGroups &group_items);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* BASICTYPE_S1OS_H */
