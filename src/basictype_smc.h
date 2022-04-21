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
public:
	DiskBasicTypeSMC(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
};

#endif /* _BASICTYPE_SMC_H_ */
