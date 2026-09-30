/// @file basictype_fat16.h
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_FAT16_H
#define BASICTYPE_FAT16_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype_fat12.h"


/** @class DiskBasicTypeFAT16

@brief FAT16の処理

*/
class DiskBasicTypeFAT16 : public DiskBasicTypeFAT12
{
protected:
	DiskBasicTypeFAT16() : DiskBasicTypeFAT12() {}
	DiskBasicTypeFAT16(const DiskBasicType &src) : DiskBasicTypeFAT12(src) {}
public:
	DiskBasicTypeFAT16(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeFAT16() {}

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

	/// @name save / write
	//@{
	//@}
};

/** @class DiskBasicTypeFAT16BE

@brief FAT16の処理 (Big Endien)

*/
class DiskBasicTypeFAT16BE : public DiskBasicTypeFAT16
{
protected:
	DiskBasicTypeFAT16BE() : DiskBasicTypeFAT16() {}
	DiskBasicTypeFAT16BE(const DiskBasicType &src) : DiskBasicTypeFAT16(src) {}
public:
	DiskBasicTypeFAT16BE(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeFAT16BE() {}

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	virtual void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
	virtual wxUint32	GetGroupNumber(wxUint32 num) const;
	//@}
};

#endif /* BASICTYPE_FAT16_H */
