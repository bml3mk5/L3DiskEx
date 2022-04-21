/// @file basictype_cpm.cpp
///
/// @brief disk basic type for CP/M
///
#include "basictype_cpm.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_cpm.h"
#include "logging.h"

//
//
//
DiskBasicTypeCPM::DiskBasicTypeCPM(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// エリアをチェック
bool DiskBasicTypeCPM::CheckFat()
{
	bool valid = true;

	return valid;
}

/// ディスクから各パラメータを取得
bool DiskBasicTypeCPM::ParseParamOnDisk(DiskD88Disk *disk)
{
	return true;
}

//

#if 0
/// ルートディレクトリのチェック
bool DiskBasicTypeCPM::CheckRootDirectory(int start_sector, int end_sector)
{
}
#endif

/// ルートディレクトリをアサイン
bool DiskBasicTypeCPM::AssignRootDirectory(int start_sector, int end_sector)
{
	int index_number = 0;
	bool unuse = false;
	for(int sec_pos = start_sector; sec_pos <= end_sector; sec_pos++) {
		int side_num, sec_num;
		DiskD88Track *track = basic->GetManagedTrack(sec_pos - 1, &side_num, &sec_num);
		if (!track) {
			continue;
		}
		DiskD88Sector *sector = track->GetSector(sec_num);
		if (!sector) break;

		int pos = 0;
		int size = sector->GetSectorSize();
		wxUint8 *buffer = sector->GetSectorBuffer();

		buffer += basic->GetDirStartPosOnSector();
		size -= basic->GetDirStartPosOnSector();

		while(pos < size) {
			DiskBasicDirItem *item = dir->AssignItem(index_number, track->GetTrackNumber(), side_num, sector, pos, buffer, unuse);
			pos += item->GetDataSize();
			buffer += item->GetDataSize();
			index_number++;
		}
	}

	// エクステント 同じファイル名 を関連付ける
	DiskBasicDirItems sort_items = dir->GetItems();
	sort_items.Sort(&DiskBasicDirItemCPM::Compare);
	DiskBasicDirItem *prev_item = NULL;
	for(size_t i = 0; i < sort_items.Count(); i++) {
		DiskBasicDirItem *item = sort_items.Item(i);
		if (!item->IsUsed()) continue;
		if (prev_item != NULL) {
			int cmp = DiskBasicDirItemCPM::CompareName(&item, &prev_item);
			if (cmp == 0) {
				// 一つ前と同じ名前なら、ポインタをセット
				((DiskBasicDirItemCPM *)prev_item)->SetNextItem(item);
				// このアイテムはリストに表示しない
				item->Visible(false);
			}
		}
		prev_item = item;
	}

	// ファイルサイズを計算
	for(size_t i = 0; i < sort_items.Count(); i++) {
		DiskBasicDirItemCPM *citem = (DiskBasicDirItemCPM *)sort_items.Item(i);
		if (citem->IsUsedAndVisible()) {
			citem->CalcFileSize();
		}
	}

	return true;
}

/// 残りディスクサイズを計算
void DiskBasicTypeCPM::CalcDiskFreeSize()
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	fat_availability.Empty();

	wxUint8 *used_map = new wxUint8[basic->GetFatEndGroup() + 1];
	memset(used_map, 0, basic->GetFatEndGroup() + 1);

	const DiskBasicDirItems *items = &dir->GetItems();
	for(size_t idx = 0; idx < items->Count(); idx++) {
		DiskBasicDirItemCPM *citem = (DiskBasicDirItemCPM *)items->Item(idx);
		if (!citem || !citem->IsUsed()) continue;

		// グループ番号のマップを調べる
		for(int n = 0; n < ((3 - citem->GetGroupWidth()) * 8); n++) {
			wxUint32 gnum = citem->GetGroup(n);
			if (gnum == 0) break;
			if (gnum <= (wxUint32)basic->GetFatEndGroup()) {
				used_map[gnum]++;
			}
		}
	}

	// 空きをチェック
	int dir_area = ((basic->GetDirEndSector() - basic->GetDirStartSector() + 1) / secs_per_group);
	for(int pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		if (pos < dir_area) {
			// ディレクトリエリアは使用済み
			fat_availability.Add(FAT_AVAIL_SYSTEM);
		} else if (used_map[pos] == 0) {
			fat_availability.Add(FAT_AVAIL_FREE);
			grps++;
		} else{
			fat_availability.Add(FAT_AVAIL_USED);
		}
	}

	fsize = grps * sector_size * secs_per_group;

	free_disk_size = (int)fsize;
	free_groups = (int)grps;

	delete [] used_map;
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeCPM::SetGroupNumber(wxUint32 num, wxUint32 val)
{
}

/// グループ番号を得る
wxUint32 DiskBasicTypeCPM::GetGroupNumber(wxUint32 num)
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeCPM::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeCPM::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeCPM::GetEmptyGroupNumber()
{
	return INVALID_GROUP_NUMBER;
}

/// 次の空き位置を返す 未使用
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeCPM::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	return INVALID_GROUP_NUMBER;
}

/// データサイズ分のグループを確保する
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeCPM::AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items)
{
	return -1;
}

/// ファイルの最終セクタのデータサイズを求める
int DiskBasicTypeCPM::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	int size = remain_size - 1;

	// ファイルサイズはセクタサイズ境界なので要計算
	// 終端コードは除く
	for(; size >= 0; size--) {
		if (sector_buffer[size] != 0x1a) break;
	}
	if (size < 0) {
		// 終端コードだけで埋まるのはおかしい
		size = remain_size - 1;
	}

	size++;

	return size;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeCPM::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num * secs_per_group;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeCPM::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int val = sector_start;
	if (remain_size < (sector_size * secs_per_group)) {
		val += ((remain_size - 1) / sector_size);
	} else {
		val += (secs_per_group - 1);
	}
	return val;
}

/// データ領域の開始セクタを計算
int DiskBasicTypeCPM::CalcDataStartSectorPos()
{
	return basic->GetSectorPosFromNum(basic->GetManagedTrackNumber(), basic->GetDirStartSector());
}

/// ルートディレクトリか
bool DiskBasicTypeCPM::IsRootDirectory(wxUint32 group_num)
{
	return true;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeCPM::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
void DiskBasicTypeCPM::AdditionalProcessOnFormatted()
{
}

/// データの書き込み処理
/// @param [in]	 item			ディレクトリアイテム
/// @param [in]	 istream		ストリームデータ
/// @param [out] buffer			セクタ内の書き込み先バッファ
/// @param [in]  size			書き込み先バッファサイズ
/// @param [in]  remain			残りのデータサイズ
/// @param [in]  sector_num		セクタ番号
/// @param [in]  group_num		現在のグループ番号
/// @param [in]  next_group		次のグループ番号
/// @param [in]  sector_end		最終セクタ番号
/// @return 書き込んだバイト数
int DiskBasicTypeCPM::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	return 0;
}

/// データの書き込み終了後の処理
void DiskBasicTypeCPM::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
}

/// ファイル名変更後の処理
void DiskBasicTypeCPM::AdditionalProcessOnRenamedFile(DiskBasicDirItem *item)
{
}

/// FAT領域を削除する
void DiskBasicTypeCPM::DeleteGroupNumber(wxUint32 group_num)
{
}
