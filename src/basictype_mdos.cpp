/// @file basictype_mdos.cpp
///
/// @brief disk basic type for MDOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_mdos.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_mdos.h"
#include "charcodes.h"


//
//
//
DiskBasicTypeMDOS::DiskBasicTypeMDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT16(basic, fat, dir)
{
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMDOS::ParseParamOnDisk(bool is_formatting)
{
	// グループ数
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		basic->SetFatEndGroup(end_group - 1);
	}
	return 1.0;
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMDOS::CheckFat(bool is_formatting)
{
	// 重複チェック
	double valid_ratio = CheckFatDuplicated(is_formatting, 1, 0x1fff);

	if (valid_ratio < 0.0) return valid_ratio;

	// FATの最初はシステム
	for(wxUint32 pos = 0; pos <= 1; pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		if (gnum != basic->GetGroupSystemCode()) {
			valid_ratio = -1.0;
			break;
		}
	}

	return valid_ratio;
}

/// 使用可能なディスクサイズを得る
/// @param [out] disk_size  ディスクサイズ
/// @param [out] group_size グループ数
void DiskBasicTypeMDOS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	DiskBasicType::GetUsableDiskSize(disk_size, group_size);
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeMDOS::GetStartSectorFromGroup(wxUint32 group_num)
{
	return DiskBasicType::GetStartSectorFromGroup(group_num);
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeMDOS::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return DiskBasicType::GetEndSectorFromGroup(group_num, next_group, sector_start, sector_size, remain_size);
}

/// データ領域の開始セクタを計算
int DiskBasicTypeMDOS::CalcDataStartSectorPos()
{
	return 0;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeMDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// FAT
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(basic->GetFatStartSector() - 1);
	if (sector) {
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));
		// トラック0は予約
		sector->Fill(0xee,
			basic->GetSectorsPerTrackOnBasic()
			+ basic->GetSectorsPerFat()
			+ basic->GetDirEndSector() - basic->GetDirStartSector() + 1,
			0);
	}

	return true;
}

//
// for data access
//

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファイサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeMDOS::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	if (item->NeedCheckEofCode()) {
		// 終端コード($00)の1つ前までを出力
		wxUint8 eof_code = basic->InvertUint8(basic->GetTextTerminateCode());
		for(int len = 0; len < remain_size; len++) {
			if (sector_buffer[len] == eof_code) {
				remain_size = len;
				break;
			}
		}
	}
	return remain_size;
}

//
// for delete
//

/// 指定したグループ番号のFAT領域を削除する
/// @param [in] group_num グループ番号
void DiskBasicTypeMDOS::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}
