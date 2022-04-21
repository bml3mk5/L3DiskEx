/// @file basictype.cpp
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "common.h"
#include "basictype.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "logging.h"


//
//
//
DiskBasicTempData::DiskBasicTempData()
{
	memset(data, 0, sizeof(data));
	size = 0;
}

void DiskBasicTempData::SetData(const wxUint8 *data, size_t len, bool invert)
{
	this->SetSize(len);
	memcpy(this->data, data, this->size);
	if (invert) {
		mem_invert(this->data, this->size);
	}
}

void DiskBasicTempData::InvertData(bool invert)
{
	if (invert) {
		mem_invert(data, size);
	}
}

//
// DISK BASIC 個別の処理テンプレート
//

/// コンストラクタ
/// @param [in] basic DISK BASIC
/// @param [in] fat   DISK BASIC FAT情報
/// @param [in] dir   DISK BASIC ディレクトリ情報
DiskBasicType::DiskBasicType(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
{
	this->basic = basic;
	this->fat = fat;
	this->dir = dir;

	this->data_start_group = 0;
}
/// デストラクタ
DiskBasicType::~DiskBasicType()
{
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicType::SetGroupNumber(wxUint32 num, wxUint32 val)
{
}

/// FAT位置を返す
/// @param [in] num グループ番号(0...)
/// @return グループ番号
wxUint32 DiskBasicType::GetGroupNumber(wxUint32 num) const
{
	return 0;
}

/// 使用しているグループ番号か
/// @param [in] num グループ番号(0...)
/// @return true 使用している
bool DiskBasicType::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
/// @param [in] num        グループ番号(0...)
/// @param [in] sector_pos セクタ位置
/// @return 次のグループ番号 / INVALID_GROUP_NUMBER 空きなし
wxUint32 DiskBasicType::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return 0;
}

/// 空きFAT位置を返す
/// @return INVALID_GROUP_NUMBER 空きなし
wxUint32 DiskBasicType::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	// 若い番号順に検索
	for(wxUint32 num = 0; num <= basic->GetFatEndGroup(); num++) {
		wxUint32 gnum = GetGroupNumber(num);
		if (gnum == basic->GetGroupUnusedCode()) {
			new_num = num;
			break;
		}
	}
	return new_num;
}

/// 次の空きFAT位置を返す
/// @param [in] curr_group グループ番号(0...)
/// @return INVALID_GROUP_NUMBER 空きなし
wxUint32 DiskBasicType::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	// 同じトラックでグループが連続するように検索
	int secs_per_grp = basic->GetSectorsPerGroup();
	int secs_per_trk = basic->GetSectorsPerTrackOnBasic();
	int sides = basic->GetSidesPerDiskOnBasic();

	int sed = secs_per_trk * sides / secs_per_grp;
	int group_max = (basic->GetFatEndGroup() / sed) + 1;
	int group_manage = managed_start_group / sed;
	int group_start = curr_group / sed;
	int group_end;
	int dir;
	int sst = 0; // curr_group % sed; // なるべく同じトラックを優先して埋める
	bool found = false;

	// 管理エリアより大きいグループは+方向、小さいグループなら-方向に検索
	dir = (group_start >= group_manage ? 1 : -1);

	for(int i=0; i<2; i++) {
		group_end = (dir > 0 ? group_max : -1);
		for(int g = group_start; g != group_end; g += dir) {
			for(int s = sst; s < sed; s++) {
				wxUint32 num = g * sed + s;
				if (num > basic->GetFatEndGroup()) {
					break;
				}
				wxUint32 gnum = GetGroupNumber(num);
//				myLog.SetDebug("DiskBasicType::GetNextEmptyGroupNumber num:0x%03x gnum:0x%03x", num, gnum);
				if (gnum == basic->GetGroupUnusedCode()) {	// 0xff
					new_num = num;
					found = true;
					break;
				}
			}
			if (found) break;
			sst = 0;
		}
		if (found) break;
		dir = -dir;
		group_start = group_manage;
	}
	return new_num;
}

/// FATエリアをチェック
/// @return false エラーあり
bool DiskBasicType::CheckFat()
{
	bool valid = true;

	wxUint32 end = basic->GetFatEndGroup() < 0xff ? basic->GetFatEndGroup() : 0xff;
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
	for(wxUint32 pos = 0; pos <= end; pos++) {
		if (tbl[pos] > 4) {
			valid = false;
			break;
		}
	}
	delete [] tbl;

	return valid;
}

/// FATの開始位置を得る
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetStartNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	int sec_pos = basic->GetFatStartSector() - 1;
	DiskD88Track *track = NULL;
	if (sec_pos >= 0) {
		if (basic->GetFatSideNumber() >= 0) sec_pos += basic->GetFatSideNumber() * basic->GetSectorsPerTrackOnBasic();
		track = basic->GetManagedTrack(sec_pos, &side_num, &sector_num);
	}
	if (track) {
// GetManagedTrack内で反転する
//		side_num = basic->GetReversedSideNumber(side_num);
		track_num = track->GetTrackNumber();
	} else {
		track_num = -1;
	}
}

/// FATの終了位置を得る
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetEndNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	int sec_sta = basic->GetFatStartSector() - 1;
	int sec_pos = sec_sta + basic->GetSectorsPerFat() * basic->GetNumberOfFats() - 1;
	DiskD88Track *track = NULL;
	if (sec_sta >= 0) {
		if (basic->GetFatSideNumber() >= 0) sec_pos += basic->GetFatSideNumber() * basic->GetSectorsPerTrackOnBasic();
		track = basic->GetManagedTrack(sec_pos, &side_num, &sector_num);
	}
	if (track) {
// GetManagedTrack内で反転する
//		side_num = basic->GetReversedSideNumber(side_num);
		track_num = track->GetTrackNumber();
	} else {
		track_num = -1;
	}
}

/// 管理エリアのトラック番号からグループ番号を計算
/// @return グループ番号
wxUint32 DiskBasicType::CalcManagedStartGroup()
{
	int trk = basic->GetManagedTrackNumber();
	int sid = basic->GetFatSideNumber();
	if (sid < 0) sid = 0;
	int sides = basic->GetSidesPerDiskOnBasic();
	int secs_per_grp = basic->GetSectorsPerGroup();
	int secs_per_trk = basic->GetSectorsPerTrackOnBasic();
	managed_start_group = (trk * sides + sid) * secs_per_trk / secs_per_grp;
	return managed_start_group;
}

//

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
/// @return true
bool DiskBasicType::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	group_items.Empty();
	size_t dir_size = 0;
	for(int sec_pos = start_sector - 1; sec_pos <= end_sector - 1; sec_pos++) {
		int trk_num = 0;
		int sid_num = 0;
		int sec_num = 1;
		DiskD88Sector *sector = basic->GetManagedSector(sec_pos, &trk_num, &sid_num, &sec_num);
		if (!sector) continue;
		group_items.Add(sec_pos, 0, trk_num, sid_num, sec_num, sec_num);
		dir_size += sector->GetSectorSize();
	}
	group_items.SetSize(dir_size);

	return true;
}

/// ルートディレクトリのチェック
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in]     is_formatting フォーマット中か
/// @return true / false
bool DiskBasicType::CheckRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, bool is_formatting)
{
	// フォーマット中はチェックしない
	if (is_formatting) return true;

	bool valid = CalcGroupsOnRootDirectory(start_sector, end_sector, group_items);

	if (valid) valid = CheckDirectory(true, group_items);

	return valid;
}

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicType::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	CalcGroupsOnRootDirectory(start_sector, end_sector, group_items);

	return AssignDirectory(true, group_items, dir_item);
}

/// ディレクトリのチェック
/// @param [in]     is_root     ルートか
/// @param [in]    group_items  セクタリスト
/// @return true / false エラーあり
bool DiskBasicType::CheckDirectory(bool is_root, const DiskBasicGroups &group_items)
{
	bool valid = true;
	bool last = false;

	int index_number = 0;
	DiskBasicDirItem *nitem = dir->NewItem(NULL, NULL);
	for(size_t idx = 0; idx < group_items.Count(); idx++) {
		const DiskBasicGroupItem *gitem = group_items.ItemPtr(idx);
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		DiskD88Track *track = basic->GetTrack(trk_num, sid_num);
		if (!track) {
			valid = false;
			break;
		}
		for(int sec_num = gitem->sector_start; sec_num <= gitem->sector_end; sec_num++) {
			DiskD88Sector *sector = track->GetSector(sec_num);
//			nitem->SetSector(sector);
			if (!sector) {
				valid = false;
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				valid = false;
				break;
			}

			int pos = 0;
			int size = sector->GetSectorSize();

			if (idx == 0 && sec_num == gitem->sector_start) {
				// ディレクトリエリア先頭をスキップする位置
				if (is_root) {
					buffer += basic->GetDirStartPosOnRoot();
					pos    += basic->GetDirStartPosOnRoot();
				} else {
					buffer += basic->GetDirStartPos();
					pos    += basic->GetDirStartPos();
				}
			}

			// ディレクトリエリア各セクタの先頭をスキップする位置
			buffer += basic->GetDirStartPosOnSector();
			pos    += basic->GetDirStartPosOnSector();

			// ディレクトリのチェック
			while(valid && !last && pos < size) {
				nitem->SetDataPtr(index_number, track->GetTrackNumber(), track->GetSideNumber(), sector, pos, buffer);
				valid = nitem->Check(last);
				pos    += nitem->GetDataSize();
				buffer += nitem->GetDataSize();
				index_number++;
			}
		}
	}
	delete nitem;
	return valid;
}

/// ディレクトリが空か
/// @param [in]     is_root     ルートか
/// @param [in]    group_items  セクタリスト
/// @return true ファイルなし / false 空ではない or エラーあり
bool DiskBasicType::IsEmptyDirectory(bool is_root, const DiskBasicGroups &group_items)
{
	bool valid = true;
	bool last = false;

	int index_number = 0;
	DiskBasicDirItem *nitem = dir->NewItem(NULL, NULL);
	for(size_t idx = 0; idx < group_items.Count(); idx++) {
		const DiskBasicGroupItem *gitem = group_items.ItemPtr(idx);
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		DiskD88Track *track = basic->GetTrack(trk_num, sid_num);
		if (!track) {
			valid = false;
			break;
		}
		for(int sec_num = gitem->sector_start; sec_num <= gitem->sector_end && valid && !last; sec_num++) {
			DiskD88Sector *sector = track->GetSector(sec_num);
//			nitem->SetSector(sector);
			if (!sector) {
				valid = false;
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				valid = false;
				break;
			}

			int pos = 0;
			int size = sector->GetSectorSize();

			if (idx == 0 && sec_num == gitem->sector_start) {
				// ディレクトリエリア先頭をスキップする位置
				if (is_root) {
					buffer += basic->GetDirStartPosOnRoot();
					pos    += basic->GetDirStartPosOnRoot();
				} else {
					buffer += basic->GetDirStartPos();
					pos    += basic->GetDirStartPos();
				}
			}

			// ディレクトリエリア各セクタの先頭をスキップする位置
			buffer += basic->GetDirStartPosOnSector();
			pos    += basic->GetDirStartPosOnSector();

			// ディレクトリにファイルがないかのチェック
			while(valid && !last && pos < size) {
				nitem->SetDataPtr(index_number, track->GetTrackNumber(), track->GetSideNumber(), sector, pos, buffer);
				if (nitem->IsNormalFile()) {
					valid = !nitem->CheckUsed(last);
				}
				pos    += nitem->GetDataSize();
				buffer += nitem->GetDataSize();
				index_number++;
			}
		}
	}
	delete nitem;
	return valid;
}

/// ディレクトリをアサイン
/// @param [in]     is_root      ルートか
/// @param [in]     group_items  セクタリスト
/// @param [in,out] dir_item     ディレクトリアイテム
/// @return true
bool DiskBasicType::AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	int index_number = 0;
	bool unuse = false;
	int size_remain = (int)group_items.GetSize();
	for(size_t idx = 0; idx < group_items.Count(); idx++) {
		const DiskBasicGroupItem *gitem = group_items.ItemPtr(idx);
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		DiskD88Track *track = basic->GetTrack(trk_num, sid_num);
		if (!track) {
			continue;
		}
		for(int sec_num = gitem->sector_start; sec_num <= gitem->sector_end; sec_num++) {
			DiskD88Sector *sector = track->GetSector(sec_num);
			if (!sector) continue;

			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) continue;

			int pos = 0;
			int size = sector->GetSectorSize();

			if (idx == 0 && sec_num == gitem->sector_start) {
				// ディレクトリエリア先頭をスキップする位置
				if (is_root) {
					buffer += basic->GetDirStartPosOnRoot();
					pos    += basic->GetDirStartPosOnRoot();
				} else {
					buffer += basic->GetDirStartPos();
					pos    += basic->GetDirStartPos();
				}
			}

			// ディレクトリエリア各セクタの先頭をスキップする位置
			buffer += basic->GetDirStartPosOnSector();
			pos    += basic->GetDirStartPosOnSector();

			while(pos < size) {
//				DiskBasicDirItem *nitem = dir->AssignItem(index_number, track->GetTrackNumber(), track->GetSideNumber(), sector, pos, buffer, unuse);
				DiskBasicDirItem *nitem = dir->NewItem(index_number, track->GetTrackNumber(), track->GetSideNumber(), sector, pos, buffer, unuse);
				// サイズに達したら以降のエントリは未使用とする
				if (FinishAssigningDirectory(size_remain)) {
					nitem->Used(false);
				}
				// 親ディレクトリを設定
				nitem->SetParent(dir_item);
				// 子ディレクトリに追加
				dir_item->AddChild(nitem);

				pos    += nitem->GetDataSize();
				buffer += nitem->GetDataSize();
				size_remain -= nitem->GetDataSize();
				index_number++;
			}
		}
	}

	return true;
}

/// ルートディレクトリの開始位置を得る
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetStartNumOnRootDirectory(int &track_num, int &side_num, int &sector_num)
{
	DiskD88Track *track = basic->GetManagedTrack(basic->GetDirStartSector() - 1, &side_num, &sector_num);
	if (track) {
		track_num = track->GetTrackNumber();
	}
}

/// ルートディレクトリの終了位置を得る
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetEndNumOnRootDirectory(int &track_num, int &side_num, int &sector_num)
{
	DiskD88Track *track = basic->GetManagedTrack(basic->GetDirEndSector() - 1, &side_num, &sector_num);
	if (track) {
		track_num = track->GetTrackNumber();
	}
}

//

/// 使用可能なディスクサイズを得る
/// @param [out] disk_size  ディスクサイズ
/// @param [out] group_size グループ数
void DiskBasicType::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = 0;
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		if (gnum != basic->GetGroupSystemCode()) group_size++;
	}
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
void DiskBasicType::CalcDiskFreeSize(bool wrote)
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	fat_availability.Empty();

	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		int fsts = FAT_AVAIL_USED;
		if (gnum == basic->GetGroupUnusedCode()) {
			fsize += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			grps++;
			fsts = FAT_AVAIL_FREE;
		} else if (gnum == basic->GetGroupSystemCode()) {
			fsts = FAT_AVAIL_SYSTEM;
		} else if (gnum >= basic->GetGroupFinalCode()) {
			fsts = FAT_AVAIL_USED_LAST;
		}
		fat_availability.Add(fsts);
	}
	free_disk_size = (int)fsize;
	free_groups = (int)grps;
}

/// 残りディスクサイズをクリア
void DiskBasicType::ClearDiskFreeSize()
{
	free_disk_size = -1;
	free_groups = -1;
	fat_availability.Empty();
}

/// FATの空き状況を配列で返す
/// @param [out] offset オフセット
/// @param [out] arr    空き状況を入れた配列
void DiskBasicType::GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const
{
	*offset = 0;
	*arr = &fat_availability;
}

//

/// データサイズ分のグループを確保する
/// @param [in]  item        ディレクトリアイテム
/// @param [in]  data_size   確保するデータサイズ（バイト）
/// @param [in]  flags       新規か追加か
/// @param [out] group_items 確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicType::AllocateGroups(DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
//	myLog.SetDebug("DiskBasicType::AllocateGroups {");

//	int file_size = data_size;
	int groups = 0; 

	// FAT
	int  rc = 0;
	bool first_group = (flags == ALLOCATE_GROUPS_NEW);
	int sizeremain = data_size;

	int bytes_per_group = basic->GetSectorsPerGroup() * basic->GetSectorSize();
	wxUint32 group_num = GetEmptyGroupNumber();
	int limit = basic->GetFatEndGroup() + 1;
	while(rc >= 0 && limit >= 0 && sizeremain > 0) {
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			rc = first_group ? -1 : -2;
			break;
		}
		// 位置を予約
		SetGroupNumber(group_num, basic->GetGroupFinalCode());

		// グループ番号の書き込み
		if (first_group) {
			item->SetStartGroup(group_num);
			first_group = false;
		}

		// 次の空きグループをさがす
		wxUint32 next_group_num = GetNextEmptyGroupNumber(group_num);

//		myLog.SetDebug("  group_num:0x%03x next:0x%03x", group_num, next_group_num);

		// 次の空きがない場合 or 残りサイズがこのグループで収まる場合
		if (next_group_num == INVALID_GROUP_NUMBER || sizeremain <= bytes_per_group) {
			// 最後のグループ番号
			next_group_num = CalcLastGroupNumber(next_group_num, sizeremain);
		}
		
		basic->GetNumsFromGroup(group_num, next_group_num, basic->GetSectorSize(), sizeremain, group_items);

		// グループ番号設定
		SetGroupNumber(group_num, next_group_num);

//		prev_gnum = gnum;
		group_num = next_group_num;

		sizeremain -= bytes_per_group;
		groups++;

		limit--;
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = first_group ? -1 : -2;
	}

	if (rc < 0) {
		// グループを削除
		DeleteGroups(group_items);
		rc = -1;
	}
//	myLog.SetDebug("rc: %d }", rc);

	return rc;
}

/// グループ番号から開始セクタ番号を得る
/// @param [in] group_num グループ番号
/// @return 開始セクタ番号
int DiskBasicType::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num * basic->GetSectorsPerGroup();
}

/// グループ番号から最終セクタ番号を得る
/// @param [in] group_num    グループ番号
/// @param [in] next_group   次のグループ番号
/// @param [in] sector_start 開始セクタ番号
/// @param [in] sector_size  セクタサイズ
/// @param [in] remain_size  残りデータサイズ
/// @return 最終セクタ番号
int DiskBasicType::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int sector_end = sector_start + basic->GetSectorsPerGroup() - 1;
	if (next_group >= basic->GetGroupFinalCode()) {
		// 最終グループの場合指定したセクタまで
		sector_end = sector_start + (next_group - basic->GetGroupFinalCode());
	}
	return sector_end;
}

/// データ領域の開始セクタを計算
int DiskBasicType::CalcDataStartSectorPos()
{
	return 0;
}

/// スキップするトラック番号
int DiskBasicType::CalcSkippedTrack()
{
	return 0x7fff;
}

/// 指定したグループ番号からルートディレクトリかどうかを判定する
bool DiskBasicType::IsRootDirectory(wxUint32 group_num)
{
	return true;
}

//
// for format
//

/// セクタデータを指定コードで埋める 全トラック＆セクタで呼ばれる
/// @param [in] track  トラック
/// @param [in] sector セクタ
void DiskBasicType::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// @param [in] data ダイアログで入力したデータ（ラベル名など）
bool DiskBasicType::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	return true;
}

//
// for data access (read/verify)
//

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicType::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	return remain_size;
}

/// データの読み込み/比較処理
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @param [in] sector_num    セクタ番号
/// @param [in] sector_end    最終セクタ番号
/// @return >=0 : 処理したサイズ  -1:比較不一致  -2:セクタがおかしい  
int DiskBasicType::AccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	if (remain_size <= sector_size) {
		// ファイルの最終セクタ
		sector_size = CalcDataSizeOnLastSector(item, istream, ostream, sector_buffer, sector_size, remain_size);
	}
	if (sector_size < 0) {
		// セクタなし
		return -2;
	}

	if (ostream) {
		// 書き出し
		temp.SetData(sector_buffer, sector_size, basic->IsDataInverted());
		ostream->Write(temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(sector_size);
		istream->Read(temp.GetData(), temp.GetSize());
		temp.InvertData(basic->IsDataInverted());

		if (memcmp(temp.GetData(), sector_buffer, temp.GetSize()) != 0) {
			// データが異なる
			return -1;
		}
	}
	return sector_size;
}

//
// for write
//

/// 最後のグループ番号を計算する
/// @param [in]  group_num		現在のグループ番号
/// @param [in]  size_remain	残りのデータサイズ
/// @return 最後のグループ番号
wxUint32 DiskBasicType::CalcLastGroupNumber(wxUint32 group_num, int size_remain)
{
	return group_num;
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
int DiskBasicType::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	bool need_eof_code = item->NeedCheckEofCode();

	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (need_eof_code) {
			// 最終は終端コード
			if (remain > 1) istream.Read((void *)buffer, remain - 1);
			if (remain > 0) buffer[remain - 1]=0x1a;
		} else {
			if (remain > 0) istream.Read((void *)buffer, remain);
		}
		if (size > remain) {
			// バッファの余りは0サプレス
			memset((void *)&buffer[remain], 0, size - remain);
		}
		len = remain;
	} else {
		// 継続
		istream.Read((void *)buffer, size);
		len = size;
	}

	// 反転
	basic->InvertMem(buffer, size);

	return len;
}

//
// for delete
//

/// 指定したグループ番号のFAT領域を削除する
/// @param [in] group_items グループリスト
void DiskBasicType::DeleteGroups(const DiskBasicGroups &group_items)
{
	for(size_t gidx=0; gidx<group_items.Count(); gidx++) {
		// FATエントリを削除
		DeleteGroupNumber(group_items.Item(gidx).group);
	}
}

/// 指定したグループ番号のFAT領域を削除する
/// @param [in] group_num グループ番号
void DiskBasicType::DeleteGroupNumber(wxUint32 group_num)
{
	// FATに未使用コードを設定
	SetGroupNumber(group_num, basic->GetGroupUnusedCode());
}
