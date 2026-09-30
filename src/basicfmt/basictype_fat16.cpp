/// @file basictype_fat16.cpp
///
/// @brief disk basic fat type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_fat16.h"
#include "basicfmt.h"
#include "basicdiritem.h"


//
//
//
DiskBasicTypeFAT16::DiskBasicTypeFAT16(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT12(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFAT16::CheckFat(bool is_formatting)
{
	// 重複チェック
	double valid_ratio = CheckFatDuplicated(is_formatting, 2, 0x1fff);

	return valid_ratio;
}

/// 残りディスクサイズを計算
void DiskBasicTypeFAT16::CalcDiskFreeSize(bool wrote)
{
	CalcDiskFreeSizeBase(wrote, 2, 0xfff8);
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeFAT16::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	fat->GetDiskBasicFatArea()->SetData16LE(num, val);
}
/// FAT位置を返す
/// @param [in] num グループ番号(0...)
wxUint32 DiskBasicTypeFAT16::GetGroupNumber(wxUint32 num) const
{
	return fat->GetDiskBasicFatArea()->GetData16LE(0, num);
}

//
//
//
DiskBasicTypeFAT16BE::DiskBasicTypeFAT16BE(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT16(basic, fat, dir)
{
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeFAT16BE::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	fat->GetDiskBasicFatArea()->SetData16BE(num, val);
}
/// FAT位置を返す
/// @param [in] num グループ番号(0...)
wxUint32 DiskBasicTypeFAT16BE::GetGroupNumber(wxUint32 num) const
{
	return fat->GetDiskBasicFatArea()->GetData16BE(0, num);
}
