/// @file basictype_mz_base.cpp
///
/// @brief disk basic type for MZ Base
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_mz_base.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "../logging.h"


//
//
//
DiskBasicTypeMZBase::DiskBasicTypeMZBase(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeMZBase::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1 - data_start_group;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
/// @param [in] wrote 書込み操作を行った後か
void DiskBasicTypeMZBase::CalcDiskFreeSize(bool wrote)
{
	int used = 0;
	fat_availability.Empty();

	// 使用済みかチェック
	int grps = 0;
	int fsts;
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup(); gnum++) {
		if (gnum < data_start_group) {
			used++;
			fsts = FAT_AVAIL_SYSTEM;
		} else if (!IsUsedGroupNumber(gnum)) {
			grps++;
			fsts = FAT_AVAIL_FREE;
		} else {
			used++;
			fsts = FAT_AVAIL_USED;
		}
		fat_availability.Add(fsts, 0, 0);
	}

	// ディレクトリエントリのグループ
	const DiskBasicDirItems *items = dir->GetCurrentItems();
	if (items) {
		for(size_t idx = 0; idx < items->Count(); idx++) {
			DiskBasicDirItem *item = items->Item(idx);
			if (!item || !item->IsUsed()) continue;

			// グループ番号のマップを調べる
			size_t gcnt = item->GetGroupCount();
			if (gcnt > 0) {
				const DiskBasicGroupItem *gitem = item->GetGroup(gcnt - 1);
				wxUint32 gnum = gitem->group;
				if (gnum <= basic->GetFatEndGroup()) {
					fat_availability.Set(gnum, FAT_AVAIL_USED_LAST);
				}
			}
		}
	}

	int fsize = grps * basic->GetSectorsPerGroup() * basic->GetSectorSize();

	fat_availability.SetFreeSize(fsize);
	fat_availability.SetFreeGroups(grps);
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeMZBase::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	if (num > basic->GetFatEndGroup()) {
		return;
	}

	int pos = (int)num - (int)data_start_group;
	if (pos < 0) {
		return;
	}

	int mask = 0;
	CalcUsedGroupPos(num, pos, mask);

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return;
	}
	// FATには未使用使用テーブルがある
	fatbuf->Bit((wxUint32)pos, (wxUint8)mask, val != 0, basic->IsDataInverted());
}

wxUint32 DiskBasicTypeMZBase::GetGroupNumber(wxUint32 num) const
{
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return INVALID_GROUP_NUMBER;
	}
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeMZBase::IsUsedGroupNumber(wxUint32 num)
{
	bool exist = false;

	if (num > basic->GetFatEndGroup()) {
		return false;
	}

	int pos = (int)num - (int)data_start_group;
	if (pos < 0) {
		// システムエリアは使用済み
		return true;
	}

	int mask = 0;
	CalcUsedGroupPos(num, pos, mask);

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return true;
	}
	// FATには未使用使用テーブルがある
	exist = fatbuf->BitTest((wxUint32)pos, (wxUint8)mask, basic->IsDataInverted());
	return exist;
}

/// 使用しているグループの位置を得る
void DiskBasicTypeMZBase::CalcUsedGroupPos(wxUint32 num, int &pos, int &mask)
{
	mask = 1 << (pos & 7);
	pos = (pos >> 3) + 6;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeMZBase::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return new_num;
	}
	// 空き位置をさがす
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup(); gnum++) {
		if (gnum >= data_start_group && !IsUsedGroupNumber(gnum)) {
			new_num = gnum;
			break;
		}
	}
	return new_num;
}

/// 次の空き位置を返す 未使用
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeMZBase::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	return INVALID_GROUP_NUMBER;
}

/// 未使用が連続している位置をさがす
wxUint32 DiskBasicTypeMZBase::FindContinuousArea(wxUint32 group_size, wxUint32 &group_start)
{
	// 未使用が連続している位置をさがす
	wxUint32 cnt = 0;
	for(wxUint32 gnum = GetGroupNumber(0); gnum <= basic->GetFatEndGroup() && cnt < group_size; gnum++) {
		if (!IsUsedGroupNumber(gnum)) {
			if (cnt == 0) {
				group_start = gnum;
			}
			cnt++;
		} else {
			cnt = 0;
		}
	}
	return cnt;
}

/// グループを確保して使用中にする
int DiskBasicTypeMZBase::AllocateGroupsSub(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, DiskBasicGroups &group_items, int &file_size, int &groups)
{
	int rc = 0;
	wxUint32 group_num = group_start;

	int limit = basic->GetFatEndGroup() + 1;
	while(remain > 0 && limit >= 0) {
		// 使用しているか
		bool used_group = IsUsedGroupNumber(group_num);
		if (!used_group) {
			// 使用済みにする
			basic->GetNumsFromGroup(group_num, 0, sec_size, remain, group_items);
			SetGroupNumber(group_num, 1);
			file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			groups++;

			remain -= (sec_size * basic->GetSectorsPerGroup());
		}
		// 次のグループ
		group_num++;
		limit--;
	}
	if (group_num > basic->GetFatEndGroup()) {
		// ファイルがオーバフローしている
		rc = -2;
	} else if (limit < 0) {
		// 無限ループ？
		rc = -2;
	}
	return rc;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeMZBase::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num * basic->GetSectorsPerGroup();
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeMZBase::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return sector_start + basic->GetSectorsPerGroup() - 1;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeMZBase::FillSector(DiskImageTrack *track, DiskImageSector *sector)
{
	sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFormat()));
}

/// FAT領域を削除する
void DiskBasicTypeMZBase::DeleteGroupNumber(wxUint32 group_num)
{
	// FATを未使用にする
	SetGroupNumber(group_num, 0);
}
