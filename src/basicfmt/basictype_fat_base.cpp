/// @file basictype_fat_base.cpp
///
/// @brief disk basic fat type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_fat_base.h"
#include "basicfmt.h"
#include "basicdiritem.h"


//
//
//
DiskBasicTypeFATBase::DiskBasicTypeFATBase(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeFATBase::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	// グループが連続するように検索
	int group_start = curr_group;
	int group_end = basic->GetFatEndGroup();
	bool found = false;

	for(int i=0; i<2; i++) {
		for(int g = group_start; g <= group_end; g++) {
			wxUint32 gnum = GetGroupNumber(g);
//			myLog.SetDebug("DiskBasicTypeFATBase::GetNextEmptyGroupNumber: g:%d gnum:%d", g, gnum);
			if (gnum == basic->GetGroupUnusedCode()) {
				new_num = g;
				found = true;
				break;
			}
		}
		if (found) break;
		// ないときは最初からさがす
		group_start = 2;
	}
	return new_num;
}

/// FATエリアの重複チェック
/// @param [in] is_formatting フォーマット中か
/// @param [in] start_group 重複チェックを開始するグループ番号
/// @param [in] max_group 重複チェックを行う最大グループ番号
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFATBase::CheckFatDuplicated(bool is_formatting, wxUint32 start_group, wxUint32 max_group)
{
	wxUint32 end = basic->GetFatEndGroup() < max_group ? basic->GetFatEndGroup() : max_group;
	wxUint8 *tbl = new wxUint8[end + 1];
	memset(tbl, 0, end + 1);

	// 同じグループ番号が重複しているか
	for(wxUint32 pos = 0; pos <= end; pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		if (gnum <= end) {
			tbl[gnum]++;
		}
	}
	// 同じグループ番号が重複している場合エラー
	double valid_ratio = 1.0;
	for(wxUint32 pos = start_group; pos <= end; pos++) {
		if (tbl[pos] > 4) {
			valid_ratio = -1.0;
			break;
		}
	}
	delete [] tbl;

	return valid_ratio;
}

/// 管理エリアのトラック番号からグループ番号を計算
wxUint32 DiskBasicTypeFATBase::CalcManagedStartGroup()
{
	managed_start_group = 0;
	return managed_start_group;
}

/// 使用可能なディスクサイズを得る(MS-DOS用)
void DiskBasicTypeFATBase::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() - 2;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算(MS-DOS用)
/// @param [in] wrote 書き込んだ後か
/// @param [in] start_group 開始グループ番号
/// @param [in] used_group 使用中グループ番号
void DiskBasicTypeFATBase::CalcDiskFreeSizeBase(bool wrote, wxUint32 start_group, wxUint32 used_group)
{
	fat_availability.Empty();

	// システム領域
	for(wxUint32 pos = 0; pos < start_group; pos++) {
		fat_availability.Add(FAT_AVAIL_SYSTEM, 0, 0);
	}

	// クラスタは2から始まる(MS-DOS)
	for(wxUint32 pos = start_group; pos <= basic->GetFatEndGroup(); pos++) {
		int fsize = 0;
		int grps = 0; 
		wxUint32 gnum = GetGroupNumber(pos);
		int fsts = FAT_AVAIL_USED;
		if (gnum == basic->GetGroupUnusedCode()) {
			fsize = (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			grps  = 1;
			fsts = FAT_AVAIL_FREE;
		} else if (gnum >= used_group) { // 0xff8 0xfff8 0xfffffff8
			fsts = FAT_AVAIL_USED_LAST;
		}
		fat_availability.Add(fsts, fsize, grps);
//		myLog.SetDebug("DiskBasicTypeFATBase::CalcDiskFreeSizeBase: pos:%d gnum:%d size:%d grps:%d", pos, gnum, fsize, grps);
	}

//	free_disk_size = (int)fsize;
//	free_groups = (int)grps;
}

/// グループ番号からセクタ番号を得る(MS-DOS用)
int DiskBasicTypeFATBase::GetStartSectorFromGroup(wxUint32 group_num)
{
	// 2から始まる (0,1は予約)
	if (group_num < 2) {
		return -1;
	}
	return (group_num - 2) * basic->GetSectorsPerGroup();
}

/// グループ番号から最終セクタ番号を得る(MS-DOS用)
int DiskBasicTypeFATBase::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int sector_end = sector_start + basic->GetSectorsPerGroup() - 1;
	return sector_end;
}

/// データ領域の開始セクタを計算(MS-DOS用)
int DiskBasicTypeFATBase::CalcDataStartSectorPos()
{
	return basic->GetDirEndSector();	// ディレクトリの次が0始まりで計算する
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeFATBase::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	int sector_pos = basic->CalcSectorPosFromNumForGroup(track->GetTrackNumber(), track->GetSideNumber(), sector->GetSectorNumber());
	if (sector_pos < 0) {
		// ファイル管理エリアの場合
		sector->Fill(basic->GetFillCodeOnFAT());
	} else {
		// ユーザーエリア
		sector->Fill(basic->GetFillCodeOnFormat());
	}
}

/// グループ確保時に最後のグループ番号を計算する
/// @param [in]     group_num	現在のグループ番号
/// @param [in,out] size_remain	残りのデータサイズ
/// @return 最後のグループ番号
wxUint32 DiskBasicTypeFATBase::CalcLastGroupNumber(wxUint32 group_num, int &size_remain)
{
	return (group_num != INVALID_GROUP_NUMBER ? basic->GetGroupFinalCode() : group_num);
}
