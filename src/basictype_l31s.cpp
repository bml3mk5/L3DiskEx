/// @file basictype_l31s.cpp
///
/// @brief disk basic fat type for L3 BASIC 1S
///
#include "basictype_l31s.h"
#include "basicfmt.h"

//
//
//
DiskBasicTypeL31S::DiskBasicTypeL31S(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT8F(basic, fat, dir)
{
}
