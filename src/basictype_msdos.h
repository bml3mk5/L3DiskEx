/// @file basictype_msdos.h
///
/// @brief disk basic fat type for MS-DOS
///
#ifndef _BASICTYPE_MSDOS_H_
#define _BASICTYPE_MSDOS_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat12.h"

/// MS-DOSの処理
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
	bool	AdditionalProcessOnFormatted();
	//@}
};

#endif /* _BASICTYPE_MSDOS_H_ */
