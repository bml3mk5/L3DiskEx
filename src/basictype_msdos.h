/// @file basictype_msdos.h
///
/// @brief disk basic type for MS-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_MSDOS_H_
#define _BASICTYPE_MSDOS_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat12.h"


/** @class DiskBasicTypeMSDOS

@brief MS-DOSの処理

DiskBasicParam
@li MediaID : メディアID

*/
class DiskBasicTypeMSDOS : public DiskBasicTypeFAT12
{
private:
	DiskBasicTypeMSDOS() : DiskBasicTypeFAT12() {}
	DiskBasicTypeMSDOS(const DiskBasicType &src) : DiskBasicTypeFAT12(src) {}
public:
	DiskBasicTypeMSDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name format
	//@{
	/// セクタデータを埋めた後の個別処理
	bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name property
	//@{
	/// IPLや管理エリアの属性を得る
	void		GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// IPLや管理エリアの属性をセット
	void		SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_MSDOS_H_ */
