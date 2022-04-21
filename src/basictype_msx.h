/// @file basictype_msx.h
///
/// @brief disk basic type for MSX
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_MSX_H_
#define _BASICTYPE_MSX_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat12.h"


/** @class DiskBasicTypeMSX

@brief MSX BASIC / MSX-DOSの処理

DiskBasicParam
@li MediaID : メディアID

*/
class DiskBasicTypeMSX : public DiskBasicTypeFAT12
{
private:
	DiskBasicTypeMSX() : DiskBasicTypeFAT12() {}
	DiskBasicTypeMSX(const DiskBasicType &src) : DiskBasicTypeFAT12(src) {}
public:
	DiskBasicTypeMSX(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name check / assign FAT area
	//@{
	/// ディスクから各パラメータを取得
	int		ParseParamOnDisk(DiskD88Disk *disk);
	//@}

	/// @name directory
	//@{
	/// サブディレクトリを作成できるか
	bool	CanMakeDirectory() const { return false; }
	//@}

	/// @name format
	//@{
	/// セクタデータを埋めた後の個別処理
	bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_MSX_H_ */
