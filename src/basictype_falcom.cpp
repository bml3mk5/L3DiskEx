/// @file basictype_falcom.cpp
///
/// @brief disk basic type for Falcom DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_falcom.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_falcom.h"
#include "charcodes.h"


//
//
//
DiskBasicTypeFalcom::DiskBasicTypeFalcom(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFalcom::ParseParamOnDisk(bool is_formatting)
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
double DiskBasicTypeFalcom::CheckFat(bool is_formatting)
{
	return 1.0;
}

/// 残りディスクサイズを計算
void DiskBasicTypeFalcom::CalcDiskFreeSize(bool wrote)
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;

	fat_availability.Empty();
	fat_availability.SetCount(basic->GetFatEndGroup() + 1, FAT_AVAIL_FREE);

	const DiskBasicDirItems *items = dir->GetCurrentItems();
	for(size_t idx = 0; idx < items->Count(); idx++) {
		DiskBasicDirItem *item = items->Item(idx);
		if (!item || !item->IsUsed()) continue;

		// グループ番号のマップを調べる
		const DiskBasicGroups *groups = &item->GetGroups();
		int count = (int)groups->Count();
		for(int n = 0; n < count; n++) {
			const DiskBasicGroupItem *group = groups->ItemPtr(n);
			wxUint32 gnum = group->group;
			if (gnum <= basic->GetFatEndGroup()) {
				if (n + 1 == count) {
					fat_availability.Item(gnum) = FAT_AVAIL_USED_LAST;
				} else {
					fat_availability.Item(gnum) = FAT_AVAIL_USED;
				}
			}
		}
	}

	// 空きをチェック
	wxUint32 dir_area = (basic->GetDirEndSector() / basic->GetSectorsPerGroup());
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		if (pos < dir_area) {
			// ディレクトリエリアは使用済み
			fat_availability.Item(pos) = FAT_AVAIL_SYSTEM;
		} else if (fat_availability.Item(pos) == FAT_AVAIL_FREE) {
//			fat_availability.Item(pos) = FAT_AVAIL_FREE;
			grps++;
		}
	}

	fsize = grps * basic->GetSectorSize() * basic->GetSectorsPerGroup();

	free_disk_size = (int)fsize;
	free_groups = (int)grps;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeFalcom::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
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
int DiskBasicTypeFalcom::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	return remain_size;
}

//
// for delete
//

/// 指定したグループ番号のFAT領域を削除する
/// @param [in] group_num グループ番号
void DiskBasicTypeFalcom::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}
