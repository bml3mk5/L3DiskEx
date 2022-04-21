/// @file basictype_smc.cpp
///
/// @brief disk basic type for SMC-777 Sony Filer
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_smc.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_cpm.h"


//
//
//
DiskBasicTypeSMC::DiskBasicTypeSMC(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeCPM(basic, fat, dir)
{
}
