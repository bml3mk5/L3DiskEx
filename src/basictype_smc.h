/// @file basictype_smc.h
///
/// @brief disk basic type for SMC-777 Sony Filer
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_SMC_H_
#define _BASICTYPE_SMC_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_cpm.h"


/** @class DiskBasicTypeSMC

@brief SMC-777 Sony Filerの処理

*/
class DiskBasicTypeSMC : public DiskBasicTypeCPM
{
protected:
	DiskBasicTypeSMC() : DiskBasicTypeCPM() {}
	DiskBasicTypeSMC(const DiskBasicType &src) : DiskBasicTypeCPM(src) {}

	wxUint8 *vtop_map;	///< 仮想インターリーブ 仮想セクタ番号 -> 物理セクタ番号
	wxUint8 *ptov_map;	///< 仮想インターリーブ 物理セクタ番号 -> 仮想セクタ番号

public:
	DiskBasicTypeSMC(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeSMC();

	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
	virtual void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
};

#endif /* _BASICTYPE_SMC_H_ */
