/// @file basictype_pa.cpp
///
/// @brief disk basic type for PASOPIA T-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_pa.h"
#include "basicfmt.h"
#include "basicdiritem.h"

//
//
//
DiskBasicTypePA::DiskBasicTypePA(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeN88(basic, fat, dir)
{
}

/// グループ番号から開始セクタ番号を得る
/// @param [in] group_num グループ番号
/// @return 開始セクタ番号
int DiskBasicTypePA::GetStartSectorFromGroup(wxUint32 group_num)
{
	// グループ（クラスタ）番号はサイド（サーフェース）優先なので、
	// セクタ番号はトラック優先になるよう変換する。
	int sides = basic->GetSidesPerDiskOnBasic();
	int grp_per_trk = basic->GetSectorsPerTrack() / basic->GetSectorsPerGroup();
	int grp_per_sid = sides * grp_per_trk;
	int ngrp = (group_num / grp_per_sid) * grp_per_sid + (group_num % sides) * grp_per_trk + ((group_num % grp_per_sid) / sides);
	return ngrp * basic->GetSectorsPerGroup();
}
