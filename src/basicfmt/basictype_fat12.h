/// @file basictype_fat12.h
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_FAT12_H_
#define _BASICTYPE_FAT12_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype_fat_base.h"


/** @class DiskBasicTypeFAT12

@brief FAT12の処理

*/
class DiskBasicTypeFAT12 : public DiskBasicTypeFATBase
{
protected:
	DiskBasicTypeFAT12() : DiskBasicTypeFATBase() {}
	DiskBasicTypeFAT12(const DiskBasicType &src) : DiskBasicTypeFATBase(src) {}
public:
	DiskBasicTypeFAT12(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeFAT12() {}

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	virtual void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
	virtual wxUint32	GetGroupNumber(wxUint32 num) const;
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	//@}

	/// @name disk size
	//@{
	/// @brief 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
//	/// @brief グループ番号から開始セクタ番号を得る
//	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
//	/// @brief グループ番号から最終セクタ番号を得る
//	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);
//	/// @brief データ領域の開始セクタを計算
//	virtual int		CalcDataStartSectorPos();
	//@}

	/// @name directory
	//@{
	//@}

	/// @name format
	//@{
	//@}

	/// @name save / write
	//@{
	//@}
};

#endif /* _BASICTYPE_FAT12_H_ */
