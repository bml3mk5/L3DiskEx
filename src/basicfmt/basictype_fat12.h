/// @file basictype_fat12.h
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_FAT12_H
#define BASICTYPE_FAT12_H

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
	//@}

	/// @name directory
	//@{
	//@}

	/// @name format
	//@{
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	//@}
};

#endif /* BASICTYPE_FAT12_H */
