/// @file basictype_fat_base.h
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_FAT_BASE_H_
#define _BASICTYPE_FAT_BASE_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


/** @class DiskBasicTypeFATBase

@brief FATの処理

*/
class DiskBasicTypeFATBase : public DiskBasicType
{
protected:
	DiskBasicTypeFATBase() : DiskBasicType() {}
	DiskBasicTypeFATBase(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeFATBase(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeFATBase() {}

	/// @name access to FAT area
	//@{
	/// @brief 次の空きFAT位置を返す
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアの重複チェック
	double 			CheckFatDuplicated(bool is_formatting, wxUint32 start_group, wxUint32 max_group);
	/// @brief 管理エリアのトラック番号からグループ番号を計算
	virtual wxUint32 CalcManagedStartGroup();
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	virtual void	GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	void			CalcDiskFreeSizeBase(bool wrote, wxUint32 start_group, wxUint32 used_group);
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);
	/// @brief データ領域の開始セクタを計算
	virtual int		CalcDataStartSectorPos();
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを指定コードで埋める
	virtual void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
	//@}

	/// @name save / write
	//@{
	/// @brief グループ確保時に最後のグループ番号を計算する
	virtual wxUint32 CalcLastGroupNumber(wxUint32 group_num, int &size_remain);
	//@}
};

#endif /* _BASICTYPE_FAT_BASE_H_ */
