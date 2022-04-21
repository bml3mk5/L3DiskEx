/// @file basictype_l31s.h
///
/// @brief disk basic fat type
///
#ifndef _BASICTYPE_L31S_H_
#define _BASICTYPE_L31S_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat8.h"

/// LEVEL-3 BASIC 1Sの処理
class DiskBasicTypeL31S : public DiskBasicTypeFAT8F
{
private:
	DiskBasicTypeL31S() : DiskBasicTypeFAT8F() {}
	DiskBasicTypeL31S(const DiskBasicType &src) : DiskBasicTypeFAT8F(src) {}
public:
	DiskBasicTypeL31S(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
};

#endif /* _BASICTYPE_L31S_H_ */
