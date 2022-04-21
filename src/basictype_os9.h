﻿/// @file basictype_os9.h
///
/// @brief disk basic type for OS-9
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_OS9_H_
#define _BASICTYPE_OS9_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype.h"


#pragma pack(1)
/// OS-9 Ident LSN = 0(track1, sector1)
typedef struct st_os9_ident {
	os9_lsn_t	DD_TOT;	///< total lsn
	wxUint8		DD_TKS;	///< sectors per track
	wxUint16	DD_MAP;	///< allocation map length
	wxUint16	DD_BIT;	///< sectors per group
	os9_lsn_t	DD_DIR;	///< rootdir lsn
	wxUint16	DD_OWN;	///< owner id
	wxUint8		DD_ATT;	///< disk attr
	wxUint16	DD_DSK;	///< disk ident
	wxUint8		DD_FMT;	///< format, density, number of sides
	wxUint16	DD_SPT;	///< sector per track
	wxUint16	reserved1;
	os9_lsn_t	DD_BT;	///< bootstrap lsn
	wxUint16	DD_BSZ;	///< bootstrap size (in bytes)
	os9_date_t	DD_DAT;	///< creation date
	wxUint8		DD_NAM[32];	///< volume label
	wxUint8		DD_OPT[32];	///< option
	wxUint8		reserved2;
	wxUint32	DD_SYNC;	///< media inegrity code
	wxUint32	DD_MapLSN;	///< bitmap starting sector number
	wxUint16	DD_LSNSize;	///< media logical sector size
	wxUint16	DD_VersID;	///< version id
} os9_ident_t;
#pragma pack()

//
class OS9AllocBuffer
{
private:
	int		 size;
	wxUint8	*buffer;

public:
	OS9AllocBuffer();
	OS9AllocBuffer(wxUint8 *newbuf, int newsize);
	~OS9AllocBuffer() {}
	wxUint8 *GetBuffer() { return buffer; }
	int GetSize() const { return size; }
	void SetBit(wxUint32 pos, wxUint32 bit, bool val);
	bool IsBitSet(wxUint32 pos, wxUint32 bit) const;
};

WX_DECLARE_OBJARRAY(OS9AllocBuffer, ArrayOS9AllocBuffer);

/// OS-9 Allocation Map
class OS9AllocMap : public ArrayOS9AllocBuffer
{
private:
	wxUint32 map_bytes;	///< ビットマップサイズ(bytes)
	wxUint32 map_start_lsn;	///< 開始LSN
	wxUint32 end_lsn;	///< 最終LSN
	wxUint32 sector_size;	///< セクタサイズ
	int secs_per_bit;	///< 1ビット当たりのセクタ数

	bool GetPosInMap(wxUint32 lsn, size_t &idx, wxUint32 &pos, wxUint32 &bit) const;

public:
	OS9AllocMap();
	~OS9AllocMap();
	bool AllocMap(DiskBasic *basic, wxUint32 n_map_start_lsn, wxUint32 n_map_bytes);
	void MakeAvailable(DiskBasicAvailabillity &fat, wxUint32 &grps, wxUint32 &fsize);
	void SetLSN(wxUint32 lsn, bool val);
	bool IsUsedLSN(wxUint32 lsn) const;
	wxUint32 FindEmpty() const;
};

/** @class DiskBasicTypeOS9

@brief OS-9の処理

DiskBasicParam
@li SubDirGroupSize : サブディレクトリの初期グループ(LSN)数
@li GroupWidth      : ビットマップ1ビットのセクタ数(DD_BIT)

*/
class DiskBasicTypeOS9 : public DiskBasicType
{
private:
	os9_ident_t *os9_ident;	///< Identification Sector

	OS9AllocMap alloc_map;	///< Allocation Map

	DiskBasicTypeOS9() : DiskBasicType() {}
	DiskBasicTypeOS9(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeOS9(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FATオフセットを返す
	wxUint32	GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループ番号か
	bool		IsUsedGroupNumber(wxUint32 num);
	/// @brief 次のグループ番号を得る
	wxUint32	GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// @brief 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
	/// @brief 次の空きFAT位置を返す 未使用
	wxUint32	GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	double 		CheckFat(bool is_formatting);
	/// @brief ルートディレクトリをアサイン
	bool		AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	double		ParseParamOnDisk(bool is_formatting);
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリのセクタリストを計算
	bool		CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	/// @brief ディレクトリが空か
	bool		IsEmptyDirectory(bool is_root, const DiskBasicGroups &group_items);
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	int			FinishAssigningDirectory(int size) const;
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	void		GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	void		CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
	/// @brief データサイズ分のグループを確保する
	int			AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);

	/// @brief グループ番号から開始セクタ番号を得る
	int			GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	int			GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// @brief データ領域の開始セクタを計算
	int			CalcDataStartSectorPos();
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	bool		IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	bool		CanMakeDirectory() const { return true; }
	/// @brief ルートディレクトリのサイズを拡張できるか
	bool		CanExpandRootDirectory() const { return true; }
	/// @brief サブディレクトリのサイズを拡張できるか
	bool		CanExpandDirectory() const { return true; }
	/// @brief サブディレクトリを作成する前の準備を行う
	bool		PrepareToMakeDirectory(DiskBasicDirItem *item);
	/// @brief サブディレクトリを作成した後の個別処理
	void		AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを指定コードで埋める
	void		FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// @brief セクタデータを埋めた後の個別処理
	bool		AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	//@}

	/// @name save / write
	//@{
	/// @brief ファイルをセーブする前の準備を行う
	bool		PrepareToSaveFile(wxInputStream &istream, int &file_size, const DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データの書き込み終了後の処理
	void		AdditionalProcessOnSavedFile(DiskBasicDirItem *item);
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	void		DeleteGroupNumber(wxUint32 group_num);
	/// @brief ファイル削除後の処理
	bool		AdditionalProcessOnDeletedFile(DiskBasicDirItem *item);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	void		GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	void		SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_OS9_H_ */
