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

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeFAT12::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	if (istream) {
		// ベリファイ時のみEOFが自動で付加されることがある
		if (item->NeedCheckEofCode()) {
			// 終端コードの1つ前までを出力
			wxUint8 eof_code = basic->GetTextTerminateCode();
			int len = remain_size - 1;
			if (sector_buffer[len] == eof_code) {
				remain_size = len;
			}
		}
	}
	return remain_size;
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
