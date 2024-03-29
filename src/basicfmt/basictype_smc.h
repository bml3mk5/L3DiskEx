/// @file basictype_smc.h
///
/// @brief disk basic type for SMC-777 Sony Filer
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_SMC_H
#define BASICTYPE_SMC_H

#include "../common.h"
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

#endif /* BASICTYPE_SMC_H */
