/// @file basictype_fat12.cpp
///
/// @brief disk basic fat type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_fat12.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "../logging.h"


//
//
//
DiskBasicTypeFAT12::DiskBasicTypeFAT12(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFATBase(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFAT12::CheckFat(bool is_formatting)
{
	// 重複チェック
	double valid_ratio = CheckFatDuplicated(is_formatting, 2, 0x1ff);

	return valid_ratio;
}

/// 残りディスクサイズを計算
void DiskBasicTypeFAT12::CalcDiskFreeSize(bool wrote)
{
	CalcDiskFreeSizeBase(wrote, 2, 0xff8);
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeFAT12::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	fat->GetDiskBasicFatArea()->SetData12LE(num, val);
}

/// FAT位置を返す
/// @param [in] num グループ番号(0...)
wxUint32 DiskBasicTypeFAT12::GetGroupNumber(wxUint32 num) const
{
	return fat->GetDiskBasicFatArea()->GetData12LE(0, num);
}
