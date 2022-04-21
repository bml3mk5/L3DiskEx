/// @file basictype_flex.cpp
///
/// @brief disk basic type for FLEX
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_flex.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "basicdiritem_flex.h"
#include "../utils.h"
#include "../logging.h"


#pragma pack(1)
/// FLEX FSM
struct st_flex_fsm {
	wxUint8 track;
	wxUint8 sector;
	wxUint8 count;
};
#pragma pack()

//
//
//
DiskBasicTypeFLEX::DiskBasicTypeFLEX(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	flex_sir = NULL;

	if (basic->GetGroupsPerTrack() <= 0) {
		basic->SetGroupsPerTrack(basic->GetGroupsPerSector() * basic->GetSectorsPerTrackOnBasic());
	}
}

/// 論理セクタ番号からセクタ内の位置を得る
int DiskBasicTypeFLEX::SecBufOfs(int sector_number) const
{
	int pos = (sector_number - 1) % basic->GetGroupsPerSector();
	return pos * basic->GetSectorSize() / basic->GetGroupsPerSector();
}

/// 論理セクタサイズ
int DiskBasicTypeFLEX::LogSecSiz(int sector_size) const
{
	return sector_size / basic->GetGroupsPerSector();
}

/// エリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFLEX::CheckFat(bool is_formatting)
{
	// SIR area
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(2);
	if (!sector) {
		return -1.0;
	}
	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer(SecBufOfs(2 + 1));
	if (!flex) {
		return -1.0;
	}

	for(size_t i=0; i<sizeof(flex->reserved0); i++) {
		if (flex->reserved0[i]) {
			return -1.0;
		}
	}
	if (flex->max_track == 0 || flex->max_sector == 0) {
		return -1.0;
	}

	// 最終グループ番号
	basic->SetFatEndGroup((flex->max_track + 1) * flex->max_sector - 1);

	flex_sir = flex;

	double valid_ratio = 1.0;

	// DIRエリアのチェインをチェック
	int dir_cnt = 0;
	int dir_sta_lsec = basic->GetDirStartSector();	// logical
	int dir_end_lsec = basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetGroupsPerSector();	// logical

	sector = basic->GetSectorFromSectorPos(dir_sta_lsec-1);
	int secofs = SecBufOfs(dir_sta_lsec);
	for(int lsec_pos = dir_sta_lsec; lsec_pos <= dir_end_lsec; lsec_pos++) {
		if (!sector) {
			valid_ratio = -1.0;
			break;
		}

		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(secofs);
		dir_cnt++;

		if (p->next_track == 0 && p->next_sector == 0) {
			break;
		}

		sector = basic->GetSectorFromSectorPos(GetSectorPosFromNumS(p->next_track, p->next_sector));
//		sector = basic->GetSector(p->next_track, PhySecNum(p->next_sector));
		secofs = SecBufOfs(p->next_sector);
	}
	// DIRエリアの最終セクタ
	int dir_end = basic->GetDirStartSector() + dir_cnt - 1;
	basic->SetDirEndSector(dir_end);	// logical

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFLEX::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 0;

	if (!flex_sir) {
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(2);
		flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer(SecBufOfs(2 + 1));
		flex_sir = flex;
	}

	myLog.SetInfo("FLEX: sir.max_track: %d", (int)flex_sir->max_track);
	if (flex_sir->max_track > 0) {
		basic->SetTracksPerSideOnBasic(flex_sir->max_track + 1);
	}
	myLog.SetInfo("FLEX: sir.max_sector: %d", (int)flex_sir->max_sector);
	if (flex_sir->max_sector > 0) {
		basic->SetSectorsPerTrackOnBasic(flex_sir->max_sector / basic->GetSidesPerDiskOnBasic() / basic->GetGroupsPerSector());
	}

	return 1.0;
}

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
bool DiskBasicTypeFLEX::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	bool valid = true;

	group_items.Empty();

	// ディレクトリのチェインをたどる
	size_t dir_size = 0;
	int limit = basic->GetFatEndGroup() + 1;
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;
	int div_num = 0;
	int div_nums = 1;
	// 開始セクタ
	DiskD88Sector *sector = basic->GetManagedSector(start_sector - 1, &trk_num, &sid_num, NULL, &div_num, &div_nums);
	while(limit >= 0) {
		if (!sector) {
			valid = false;
			break;
		}
		sec_num = sector->GetSectorNumber();

		wxUint8 *buffer = sector->GetSectorBuffer(SecBufOfs(div_num + 1)); 
		if (!buffer) {
			valid = false;
			break;
		}

		group_items.Add(0, 0, trk_num, sid_num, sec_num, sec_num, div_num, div_nums);

		dir_size += LogSecSiz(sector->GetSectorSize());

		flex_ptr_t *p = (flex_ptr_t *)buffer;

		// 次のセクタなし
		if (p->next_track == 0 && p->next_sector == 0) {
			break;
		}

		limit--;

		// 次のセクタを得る
		sector = basic->GetSectorFromSectorPos(GetSectorPosFromNumS(p->next_track, p->next_sector), trk_num, sid_num, &div_num, &div_nums);
	}
	group_items.SetSize(dir_size);

	// 最終セクタ番号を更新
	int sec_pos = GetSectorPosFromNum(trk_num, sid_num, sec_num);
	sec_pos -= (basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic());
	basic->SetDirEndSector(sec_pos + 1);	// logical

	if (limit < 0) {
		valid = false;
	}
	return valid;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeFLEX::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup() / basic->GetGroupsPerSector();
}

/// 残りディスクサイズを計算
void DiskBasicTypeFLEX::CalcDiskFreeSize(bool wrote)
{
	int fsize = 0;
	int grps = 0;

//	myLog.SetDebug("DiskBasicTypeFLEX::CalcDiskFreeSize");

	fat_availability.Empty();

	fat_availability.SetCount(basic->GetFatEndGroup() + 1, FAT_AVAIL_USED);

	// SIR area
//	DiskD88Disk *disk = basic->GetDisk();
	DiskD88Sector *sector = NULL;
//	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer();

	int track_num  = flex_sir->free_start_track;
	int lsector_num = flex_sir->free_start_sector;
	int limit = basic->GetFatEndGroup() + 1;
	int div_num = 0;
	int div_nums = 1;
	while((track_num != 0 || lsector_num != 0) && limit >= 0) {
		int sector_pos = GetSectorPosFromNumS(track_num, lsector_num);
		sector = basic->GetSectorFromSectorPos(sector_pos, &div_num, &div_nums);
//		sector = basic->GetSector(track_num, sector_num);
		if (!sector) {
			// error
			break;
		}
		if (sector_pos < (int)fat_availability.Count()) {
			if (fat_availability.Get(sector_pos) == FAT_AVAIL_FREE) {
				// 既に空きエリアにしているのに同じセクタにきている
				// 無限ループしている？
				break;
			}
			fat_availability.Set(sector_pos, FAT_AVAIL_FREE);
		}

		// セクタ先頭4バイトは除く
		fsize += (LogSecSiz(sector->GetSectorSize()) - 4);
		grps++;

//		myLog.SetDebug("trk:%d sec:%d size:%d", track_num, sector_num, fsize);

		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
		track_num = p->next_track;
		lsector_num = p->next_sector;
		limit--;
	}

	// ディレクトリエントリのグループ
	const DiskBasicDirItems *items = dir->GetCurrentItems();
	if (items) {
		for(size_t idx = 0; idx < items->Count(); idx++) {
			DiskBasicDirItem *item = items->Item(idx);
			if (!item || !item->IsUsed()) continue;

			// 最後のグループ
			size_t gcnt = item->GetGroupCount();
			if (gcnt > 0) {
				wxUint32 gnum = item->GetGroup(gcnt-1)->group;
				if (gnum <= basic->GetFatEndGroup()) {
					fat_availability.Set(gnum, FAT_AVAIL_USED_LAST);
				}
			}
		}
	}

	fat_availability.SetFreeSize(fsize);
	fat_availability.SetFreeGroups(grps);
}

/// シーケンス番号を設定
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeFLEX::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	int div_num = 0;
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(num, &div_num);
	if (!sector) {
		// why?
		return;
	}
	flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
	if (!p) {
		// why?
		return;
	}
	p->seq_num = wxUINT16_SWAP_ON_LE(val);
}

/// グループ番号を得る
wxUint32 DiskBasicTypeFLEX::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeFLEX::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeFLEX::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeFLEX::GetEmptyGroupNumber()
{
	DiskD88Sector *sector = NULL;
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	int sta_track_num  = flex_sir->free_start_track;
	int sta_lsector_num = flex_sir->free_start_sector;
	if (sta_track_num == 0 && sta_lsector_num == 0) {
		// no free space ?
		return INVALID_GROUP_NUMBER;
	}
	int div_num = 0;
	// グループ番号を得る
	group_num = GetSectorPosFromNumS(sta_track_num, sta_lsector_num);
	sector = basic->GetSectorFromSectorPos(group_num, &div_num);
	if (!sector) {
		// no free space ?
		return INVALID_GROUP_NUMBER;
	}

	// 次のポインタをフリーセクタポインタに設定
	flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
	if (!p) {
		// why?
		return INVALID_GROUP_NUMBER;
	}
	// 空き領域の開始ポインタを更新
	flex_sir->free_start_track = p->next_track;
	flex_sir->free_start_sector = p->next_sector;
	int size = wxUINT16_SWAP_ON_LE(flex_sir->free_sector_nums);
	size--;
	if (size <= 0 || (flex_sir->free_start_track == 0 && flex_sir->free_start_sector == 0)) {
		// 空きがなくなった
		size = 0;
		flex_sir->free_start_track = 0;
		flex_sir->free_start_sector = 0;
		flex_sir->free_last_track = 0;
		flex_sir->free_last_sector = 0;
	}
	flex_sir->free_sector_nums = wxUINT16_SWAP_ON_LE(size);
	// 予約済みにする
	p->next_track = 0;
	p->next_sector = 0;

	return group_num;
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeFLEX::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	// 次の空き位置候補
	wxUint32 next_group_num = GetEmptyGroupNumber();
	if (next_group_num == INVALID_GROUP_NUMBER) {
		return INVALID_GROUP_NUMBER;
	}
	// 現在のセクタに次のセクタへのポインタをセット
	if (ChainGroups(curr_group, next_group_num) < 0) {
		return INVALID_GROUP_NUMBER;
	}

	return next_group_num;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeFLEX::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
//	myLog.SetDebug("DiskBasicTypeFLEX::AllocateGroups {");

//	int file_size = data_size;
	int groups = 0; 

	// FAT
	int  rc = 0;
	bool first_group = (flags == ALLOCATE_GROUPS_NEW);
	wxUint32 group_num = (flags == ALLOCATE_GROUPS_NEW ? INVALID_GROUP_NUMBER : item->GetLastGroup());

	// 1セクタ当たり4バイトはチェイン用のリンクポインタになるので減算
	int bytes_per_group = basic->GetSectorsPerGroup() * (LogSecSiz(basic->GetSectorSize()) - 4);
	// ランダムアクセスファイルか
	int random_file = item->GetFileAttr().GetOrigin();
	if (flags == ALLOCATE_GROUPS_NEW && random_file > 0) {
		// ランダムアクセスファイル
		// インデックス(FSM)セクタを確保
		for(int idx = 0; idx < random_file; idx++) {
			group_num = first_group ? GetEmptyGroupNumber() : GetNextEmptyGroupNumber(group_num);
			if (group_num == INVALID_GROUP_NUMBER) {
				// 空きなし
				rc = first_group ? -1 : -2;
				break;
			}
			// セクタをクリア
			int div_num = 0;
			DiskD88Sector *sector = basic->GetSectorFromGroup(group_num, &div_num);
			if (sector) {
				sector->Fill(0, LogSecSiz(basic->GetSectorSize()), SecBufOfs(div_num + 1));
			}
			// グループ番号の書き込み
			if (first_group) {
				item->SetStartGroup(fileunit_num, group_num);
				first_group = false;
			}
		}
	}

	int sizeremain = data_size;
	int limit = basic->GetFatEndGroup() + 1;
	while(rc >= 0 && limit >= 0 && sizeremain > 0) {
		group_num = first_group ? GetEmptyGroupNumber() : GetNextEmptyGroupNumber(group_num);
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			rc = first_group ? -1 : -2;
			break;
		}
		// グループ番号の書き込み
		if (first_group) {
			item->SetStartGroup(fileunit_num, group_num);
			first_group = false;
		}

//		myLog.SetDebug("  group_num:0x%03x", group_num);
		
		basic->GetNumsFromGroup(group_num, 0, LogSecSiz(basic->GetSectorSize()), sizeremain, group_items);

		// シーケンス番号設定
		SetGroupNumber(group_num, groups + 1);

		sizeremain -= bytes_per_group;
		groups++;

		limit--;
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = first_group ? -1 : -2;
	}
	if (rc == 0) {
		// 最終グループ番号
		item->SetLastGroup(group_num);

		// ランダムアクセスファイルの場合は、インデックスを作成する
		if (random_file > 0) {
			DiskBasicGroups random_groups;
			int prev_trk = -1;
			wxUint32 prev_grp = 0xfffffff;
			wxUint32 idx_count = 0;
			wxUint32 idx_start = 0;
			for(size_t i = 0; i < group_items.Count(); i++) {
				DiskBasicGroupItem *gitm = group_items.ItemPtr(i);
				if (prev_grp == gitm->group) {
					// 同じならスキップ
					continue;
				}
				if (prev_trk == gitm->track && prev_grp + 1 == gitm->group) {
					// グループ番号が連続している
					idx_count++;
				} else {
					// グループ番号が連続していない
					if (idx_count > 0) {
						random_groups.Add(idx_start, idx_count, 0, 0, 0, 0, 0, basic->GetGroupsPerSector());
					}
					idx_start = gitm->group;
					idx_count = 1;
				}
				prev_trk = gitm->track;
				prev_grp = gitm->group;
			}
			if (idx_count > 0) {
				random_groups.Add(idx_start, idx_count, 0, 0, 0, 0, 0, basic->GetGroupsPerSector());
			}
			// インデックス(FSM)セクタに書き込む
			DiskD88Sector *isector = NULL;
			idx_start = item->GetStartGroup(fileunit_num);
			size_t idx = 0;
			bool finished = false;
			for(int sec = 0; sec < random_file && !finished; sec++) {
				int div_num = 0;
				isector = basic->GetSectorFromGroup(idx_start, &div_num);
				if (!isector) break;
				wxUint8 *buf = isector->GetSectorBuffer(SecBufOfs(div_num + 1));
				if (!buf) break;
				for(int pos = 4; pos < LogSecSiz(isector->GetSectorSize()); pos += (int)sizeof(struct st_flex_fsm)) {
					DiskBasicGroupItem *ritem = random_groups.ItemPtr(idx);
					int trk_num, sec_num;
					GetNumFromSectorPosS(ritem->group, trk_num, sec_num);
					struct st_flex_fsm *fsm = (struct st_flex_fsm *)&buf[pos];
					fsm->track = (wxUint8)trk_num;
					fsm->sector = (wxUint8)sec_num;
					fsm->count = (wxUint8)(ritem->next);
					idx++;
					if (idx >= random_groups.Count()) {
						finished = true;
						break;
					}
				}
				flex_ptr_t *p = (flex_ptr_t *)buf;
				idx_start = GetSectorPosFromNumS(p->next_track, p->next_sector);
			}
		}
	} else {
		// エラー時
		// 確保した領域を削除
		DeleteGroups(group_items);
		// 空き領域をチェインする
		wxUint32 last_group = item->GetLastGroup();
		if (last_group != 0) {
			int div_num = 0;
			DiskD88Sector *sector = basic->GetSectorFromGroup(last_group, &div_num);
			if (sector) {
				flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
				if (p) {
					flex_sir->free_last_track = p->next_track;
					flex_sir->free_last_sector = p->next_sector;
					p->next_track = 0;
					p->next_sector = 0;
					p->seq_num = 0;

					RemakeChainOnFreeArea();

					rc = -1;
				}
			}
		}
	}

//	myLog.SetDebug("rc: %d }", rc);
	return rc;
}

/// グループをつなげる
int DiskBasicTypeFLEX::ChainGroups(wxUint32 group_num, wxUint32 append_group_num)
{
	// 現在のセクタに次のセクタへのポインタをセット
	int div_num = 0;
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(group_num, &div_num);
	if (!sector) {
		// why?
		return -1;
	}
	flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
	if (!p) {
		// why?
		return -1;
	}
	int next_track_num = 0;
	int next_sector_num = 0;
	GetNumFromSectorPosS(append_group_num, next_track_num, next_sector_num);
	p->next_track = (wxUint8)next_track_num;
	p->next_sector = (wxUint8)next_sector_num;

	return 0;
}

/// データの読み込み/比較処理
/// @param [in] fileunit_num  ファイル番号
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @param [in] sector_num    セクタ番号
/// @param [in] sector_end    最終セクタ番号
/// @return >=0 : 処理したサイズ  -1:比較不一致
int DiskBasicTypeFLEX::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	const wxUint8 *buf = &sector_buffer[4];
	int size = (sector_size - 4) < remain_size ? (sector_size - 4) : remain_size;

	if (ostream) {
		// 書き出し
		temp.SetData(buf, size, basic->IsDataInverted());
		ostream->Write(temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read(temp.GetData(), temp.GetSize());
		temp.InvertData(basic->IsDataInverted());

		if (memcmp(temp.GetData(), buf, size) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeFLEX::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeFLEX::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return group_num;
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num 物理セクタ番号
/// @param [out] div_num    分割番号
/// @param [out] div_nums   分割数
void DiskBasicTypeFLEX::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int groups_per_sector = basic->GetGroupsPerSector();
	int groups_per_track = basic->GetGroupsPerTrack();

	int trksid_num = sector_pos / groups_per_track;
	if (selected_side >= 0) {
		// 1S
		track_num = trksid_num;
		side_num = selected_side;
	} else {
		// 2D, 2HD
		track_num = trksid_num / sides_per_disk;
		side_num = trksid_num % sides_per_disk;
	}
	sector_num = ((sector_pos % groups_per_track) / groups_per_sector) + 1;
	if (div_num) *div_num = ((sector_pos % groups_per_track) % groups_per_sector);

	if (numbering_sector == 1) {
		// トラックごとに連番の場合
		sector_num += (side_num * sectors_per_track);
	}

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	if (div_nums) *div_nums = groups_per_sector;
}

/// 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] sector_num 論理セクタ番号
void DiskBasicTypeFLEX::GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num)
{
	int selected_side = basic->GetSelectedSide();
	int groups_per_track = basic->GetGroupsPerTrack();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();

	if (selected_side >= 0) {
		// 1S
		track_num = sector_pos / groups_per_track;
		sector_num = (sector_pos % groups_per_track) + 1;
	} else {
		// 2D, 2HD
		track_num = sector_pos / (groups_per_track * sides_per_disk);
		sector_num = (sector_pos % (groups_per_track * sides_per_disk)) + 1;
	}
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  物理セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeFLEX::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
	int groups_per_track = basic->GetGroupsPerTrack();
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
//	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * groups_per_track;
		sector_pos += ((sector_num - 1) * div_nums + div_num);
	} else {
		// 2D, 2HD
		sector_pos = track_num * sides_per_disk * groups_per_track;
		if (numbering_sector == 1) {
			sector_pos += ((sector_num - 1) * div_nums + div_num);
		} else {
			sector_pos += (side_num % sides_per_disk) * groups_per_track;
			sector_pos += ((sector_num - 1) * div_nums + div_num);
		}
	}

	return sector_pos;
}

/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] sector_num 論理セクタ番号
/// @return 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeFLEX::GetSectorPosFromNumS(int track_num, int sector_num)
{
	int selected_side = basic->GetSelectedSide();
	int groups_per_track = basic->GetGroupsPerTrack();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * groups_per_track + sector_num - 1;
	} else {
		// 2D, 2HD
		sector_pos = track_num * groups_per_track * sides_per_disk + sector_num - 1;
	}
	return sector_pos;
}

/// ルートディレクトリか
bool DiskBasicTypeFLEX::IsRootDirectory(wxUint32 group_num)
{
	return false;
}

/// フォーマット時セクタデータを指定コードで埋める
/// @param[in] track  トラック
/// @param[in] sector セクタ
void DiskBasicTypeFLEX::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	for(int div_num = 0; div_num < basic->GetGroupsPerSector(); div_num++) { 
		sector->Fill(basic->GetFillCodeOnFormat(), LogSecSiz(basic->GetSectorSize()), SecBufOfs(div_num + 1));

		if (track->GetTrackNumber() > 0 && track->GetSideNumber() < basic->GetSidesPerDiskOnBasic()) {
			// セクタの先頭にリンクを作成
			flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
			if (p) {
				int next_track = track->GetTrackNumber();
				int next_sector = (sector->GetSectorNumber() - 1) * basic->GetGroupsPerSector() + 2 + div_num;
				if (next_sector >= (basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetGroupsPerSector() + 1)) {
					next_track++;
					next_sector = 1;
				}
				if (next_track < basic->GetTracksPerSide()) {
					p->next_track = (wxUint8)next_track;
					p->next_sector = (wxUint8)next_sector;
				} else {
					p->next_track = 0;
					p->next_sector = 0;
				}
				p->seq_num = 0;
			}
		}
	}
}

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeFLEX::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// SIR area
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(2);
	if (!sector) return false;
	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer(SecBufOfs(2 + 1));
	if (!flex) return false;

	flex_sir = flex;

	sector->Fill(0, LogSecSiz(basic->GetSectorSize()), SecBufOfs(2 + 1));

	// SIRエリアを設定

	flex_sir->free_start_track = 1;
	flex_sir->free_start_sector = 1;
	flex_sir->free_last_track = (basic->GetTracksPerSide() - 1);
	flex_sir->free_last_sector = (basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetGroupsPerSector());

	int sector_nums = (basic->GetTracksPerSide() - 1) * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetGroupsPerSector();

	flex_sir->free_sector_nums = wxUINT16_SWAP_ON_LE(sector_nums);

	flex_sir->max_track = (basic->GetTracksPerSide() - 1);
	flex_sir->max_sector = (basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetGroupsPerSector());

	TM tm;
	tm.Now();
	flex_sir->cmonth = tm.GetMonth() + 1;
	flex_sir->cday = tm.GetDay();
	flex_sir->cyear = (tm.GetYear() % 100);

	// volume name and number
	SetIdentifiedData(data);

	// DIRエリア

	int dir_sta_lsec = basic->GetDirStartSector() - 1;	// logical
	int dir_end_lsec = basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetGroupsPerSector() - 1;	// logical
	DiskD88Sector *prev_sector = NULL;
	int prev_lsec_pos = 0;
	int next_track = 0;
	int next_sector = 0;
	for(int lsec_pos = dir_sta_lsec; lsec_pos <= dir_end_lsec; lsec_pos++) {
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(lsec_pos);
		if (prev_sector) {
			flex_ptr_t *p = (flex_ptr_t *)prev_sector->GetSectorBuffer(SecBufOfs(prev_lsec_pos + 1));
			if (p) {
				next_sector = lsec_pos + 1;
				p->next_track = (wxUint8)next_track;
				p->next_sector = (wxUint8)next_sector;
				p->seq_num = 0;
			}
		}
		if (!sector) {
			// トラック0のセクタ数はほかのトラックより少ない場合がある
			continue;
		}
		prev_sector = sector;
		prev_lsec_pos = lsec_pos;
		sector->Fill(0, LogSecSiz(basic->GetSectorSize()), SecBufOfs(lsec_pos + 1));
	}
	if (prev_sector) {
		flex_ptr_t *p = (flex_ptr_t *)prev_sector->GetSectorBuffer(SecBufOfs(prev_lsec_pos + 1));
		if (p) {
			p->next_track = 0;
			p->next_sector = 0;
			p->seq_num = 0;
		}
	}

	return true;
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
/// @param [in]  seq_num		通し番号(0...)
/// @return 書き込んだバイト数
int DiskBasicTypeFLEX::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	bool need_eof_code = item->NeedCheckEofCode();

	// セクタの4バイト目から
	buffer += 4;
	size -= 4;

	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (need_eof_code) {
			// 最終は終端コード
			if (remain > 1) istream.Read((void *)buffer, remain - 1);
			if (remain > 0) buffer[remain - 1]=basic->GetTextTerminateCode();
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

/// FAT領域を削除する
void DiskBasicTypeFLEX::DeleteGroupNumber(wxUint32 group_num)
{
}

/// ファイル削除後の処理
bool DiskBasicTypeFLEX::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	directory_flex_t *d = (directory_flex_t *)item->GetData();

	DiskD88Sector *sector;
	flex_ptr_t *p;

	int start_track_num = d->start_track;
	int start_sector_num = d->start_sector;
	int last_track_num = d->last_track;
	int last_sector_num = d->last_sector;
	int div_num = 0;

	// 削除した領域を空き領域の最後につなげる

	if ((flex_sir->free_start_track == 0 && flex_sir->free_start_sector == 0)
	 || (flex_sir->free_last_track == 0 && flex_sir->free_last_sector == 0)) {
		// 空きがない
		flex_sir->free_start_track = (wxUint8)start_track_num;
		flex_sir->free_start_sector = (wxUint8)start_sector_num;
		flex_sir->free_last_track = (wxUint8)last_track_num;
		flex_sir->free_last_sector = (wxUint8)last_sector_num;
	} else {
		// チェインする
		sector = basic->GetSectorFromSectorPos(GetSectorPosFromNumS(flex_sir->free_last_track, flex_sir->free_last_sector), &div_num);
		if (!sector) return false;
		p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
		if (!p) return false;

		p->next_track = (wxUint8)start_track_num;
		p->next_sector = (wxUint8)start_sector_num;
		flex_sir->free_last_track = (wxUint8)last_track_num;
		flex_sir->free_last_sector = (wxUint8)last_sector_num;
	}

	// 空き領域のチェインを作り直す

	RemakeChainOnFreeArea();

	return true;
}

/// 空きエリアのチェインを作り直す
void DiskBasicTypeFLEX::RemakeChainOnFreeArea()
{
	DiskD88Sector *sector;
	flex_ptr_t *p;

	// 空き領域のリスト
	int free_track_num = flex_sir->free_start_track;
	int free_sector_num = flex_sir->free_start_sector;
	DiskBasicGroups group_items;
	while(free_track_num != 0 && free_sector_num != 0) {
		wxUint32 free_group_num = GetSectorPosFromNumS(free_track_num, free_sector_num);
		group_items.Add(free_group_num, 0, free_track_num, 0, free_sector_num, 0);
		int div_num = 0;
		sector = basic->GetSectorFromSectorPos(GetSectorPosFromNumS(free_track_num, free_sector_num), &div_num);
		if (!sector) break;
		p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
		if (!p) break;

		free_track_num = p->next_track;
		free_sector_num = p->next_sector;
	}

	// 空き領域をソートしてチェインを作り直す
	group_items.SortItems();
	size_t group_items_count = group_items.Count();
	for(size_t idx = 0; idx < group_items_count; idx++) {
		DiskBasicGroupItem *gitem = &group_items.Item(idx);
		int div_num = 0;
		sector = basic->GetSectorFromSectorPos(GetSectorPosFromNumS(gitem->track, gitem->sector_start), &div_num);
		if (!sector) break;
		p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
		if (!p) break;

		if (idx == 0) {
			// first
			flex_sir->free_start_track = (wxUint8)gitem->track;
			flex_sir->free_start_sector = (wxUint8)gitem->sector_start;
		}
		if ((idx + 1) != group_items_count) {
			DiskBasicGroupItem *next_gitem = &group_items.Item(idx + 1);
			p->next_track = (wxUint8)next_gitem->track;
			p->next_sector = (wxUint8)next_gitem->sector_start;
//			p->seq_num = wxUINT16_SWAP_ON_LE(idx + 1);
			p->seq_num = 0;
		} else {
			// last
			p->next_track = 0;
			p->next_sector = 0;
			p->seq_num = 0;

			flex_sir->free_last_track = (wxUint8)gitem->track;
			flex_sir->free_last_sector = (wxUint8)gitem->sector_start;
		}
	}
	flex_sir->free_sector_nums = wxUINT16_SWAP_ON_LE(group_items_count);
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeFLEX::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// volume label
	wxString vol((const char *)flex_sir->volume_label, sizeof(flex_sir->volume_label)); 
	data.SetVolumeName(vol);
	data.SetVolumeNameMaxLength(sizeof(flex_sir->volume_label));
	// volume number
	data.SetVolumeNumber(wxUINT16_SWAP_ON_LE(flex_sir->volume_number));
	// volume date
	TM tm;
	tm.SetYear(flex_sir->cyear % 100);
	if (tm.GetYear() >= 0 && tm.GetYear() < 80) tm.AddYear(100);
	tm.SetMonth(flex_sir->cmonth - 1); 
	tm.SetDay(flex_sir->cday);
	data.SetVolumeDate(Utils::FormatYMDStr(tm));
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeFLEX::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	const DiskBasicFormat *fmt = basic->GetFormatType();

	// volume label
	if (fmt->HasVolumeName()) {
		wxCharBuffer vol = data.GetVolumeName().To8BitData();
		mem_copy(vol.data(), vol.length(), 0, flex_sir->volume_label, sizeof(flex_sir->volume_label)); 
	}
	// volume number
	if (fmt->HasVolumeNumber()) {
		flex_sir->volume_number = wxUINT16_SWAP_ON_LE(data.GetVolumeNumber());
	}
}
