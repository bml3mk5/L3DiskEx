/// @file basictype_mz_base.h
///
/// @brief disk basic type for MZ Base
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_MZ_BASE_H_
#define _BASICTYPE_MZ_BASE_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


/** @class DiskBasicTypeMZBase

@brief MZ Baseの処理

*/
class DiskBasicTypeMZBase : public DiskBasicType
{
protected:
	DiskBasicTypeMZBase() : DiskBasicType() {}
	DiskBasicTypeMZBase(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeMZBase(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	virtual void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FATオフセットを返す
	virtual wxUint32	GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループ番号か
	virtual bool		IsUsedGroupNumber(wxUint32 num);
	/// @brief 使用しているグループの位置を得る
	virtual void		CalcUsedGroupPos(wxUint32 num, int &pos, int &mask);
	/// @brief 空き位置を返す
	virtual wxUint32	GetEmptyGroupNumber();
	/// @brief 次の空きFAT位置を返す 未使用
	virtual wxUint32	GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	virtual void		GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	virtual void		CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file chain
	//@{
	/// @brief 未使用が連続している位置をさがす
	virtual wxUint32	FindContinuousArea(wxUint32 group_size, wxUint32 &group_start);
	/// @brief グループを確保して使用中にする
	virtual int			AllocateGroupsSub(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, DiskBasicGroups &group_items, int &file_size, int &groups);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int			GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int			GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを指定コードで埋める
	virtual void		FillSector(DiskD88Track *track, DiskD88Sector *sector);
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void		DeleteGroupNumber(wxUint32 group_num);
	//@}
};

#endif /* _BASICTYPE_MZ_BASE_H_ */
