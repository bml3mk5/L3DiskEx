/// @file basictype_s1os.cpp
///
/// @brief disk basic type for S1-OS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_s1os.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdiritem_s1os.h"
#include "basicdir.h"
#include "../logging.h"


//
//
//
DiskBasicTypeS1OS::DiskBasicTypeS1OS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
//	m_root_pos = 0;
	p_volume = NULL;
	m_soft_interleave = 1;
	memset(m_skew_map, 0, sizeof(m_skew_map));
	memset(m_skew_rmap, 0, sizeof(m_skew_rmap));
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeS1OS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// Volume情報はトラック0, セクタ2
	DiskImageSector *sector = basic->GetSector(0, 2);
	if (!sector) {
		return -1.0;
	}
	p_volume = (s1os_volume_info_t *)sector->GetSectorBuffer();

	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 0;
	// ルートディレクトリがあるか
	DiskBasicDirItemS1OS::GetCHR(basic, &p_volume->root_start, trk_num, sid_num, sec_num);
	if (!basic->GetSector(trk_num, sid_num, sec_num)) {
		return -1.0;
	}

	// データの開始
	data_start_group = GetSectorPosFromNum(trk_num, sid_num, sec_num);
	basic->SetManagedTrackNumber(trk_num);
	basic->SetDirStartSector(data_start_group + 1);

	// スキューマップの数値をチェック
	sector = basic->GetSector(0, 3);
	if (!sector) {
		return -1.0;
	}
	s1os_skew_bitmap_t *skew = (s1os_skew_bitmap_t *)sector->GetSectorBuffer();
	int sectors = basic->GetSectorsPerTrack();
	for(int sec_pos = 0; sec_pos < sectors; sec_pos++) {
		wxUint16 num = basic->OrderUint16(skew->skew[sec_pos]);
		if (num > (wxUint16)sectors || num > MAX_SECTORS) {
			myLog.SetError(wxT("skew map error: number is out of range (pos:%d num:0x%x)"), sec_pos, num);
			valid_ratio =  -1.0;
			break;
		}
		m_skew_map[sec_pos] = (wxUint8)(num - 1);
		m_skew_rmap[num - 1] = (wxUint8)sec_pos;
	}
	if (sectors >= 2) {
		m_soft_interleave = m_skew_map[1] - m_skew_map[0];
	}
	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param[in] is_formatting フォーマット中か
double DiskBasicTypeS1OS::ParseParamOnDisk(bool is_formatting)
{
	// 使用していないパラメータをクリアしておく
	basic->SetDirTerminateCode((wxUint16)-1);
	basic->SetDirTrimmingCode(0);
	basic->SetDirSpaceCode(0);
	basic->SetNumberOfFats(0);

	basic->SetDirStartPosOnSector((int)sizeof(s1os_chain_t));

	// ディスクサイズから計算
	wxUint32 sectors = basic->GetSidesPerDiskOnBasic() * basic->GetTracksPerSideOnBasic() * basic->GetSectorsPerTrackOnBasic();
	basic->SetFatEndGroup(sectors - 1);
	basic->SetGroupSystemCode(sectors);
	basic->SetGroupFinalCode(sectors);

	// ビットマップ
	m_bitmap.Empty();
	for(int sec_num = 3; sec_num <= 5; sec_num++) {
		DiskImageSector *sector = basic->GetSector(0, sec_num);
		if (!sector) {
			return -1.0;
		}
		if (sec_num == 3) {
			s1os_skew_bitmap_t *skew_bmp = (s1os_skew_bitmap_t *)sector->GetSectorBuffer();
			m_bitmap.AddBuffer(&skew_bmp->bitmap, sector->GetSectorSize() - (int)sizeof(skew_bmp->skew));
		} else {
			m_bitmap.AddBuffer(sector->GetSectorBuffer(), sector->GetSectorSize());
		}
	}
	return 1.0;
}

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
/// @return true
bool DiskBasicTypeS1OS::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	group_items.Empty();

	// Volume情報
	if (!p_volume) return false;

	// ルートディレクトリのあるセクタ
	CountGroups(&p_volume->root_size, &p_volume->root_start, group_items);

	if (group_items.Count() > 0) {
		basic->SetDirEndSector(group_items.Last().group + 1);
	} else {
		basic->SetDirEndSector(0);
	}

//	DiskBasicDirItem *root_item = dir->GetRootItem();
//	if (root_item) {
//		root_item->SetFileSize(DiskBasicDirItemS1OS::ConvToFileSize(basic, &p_volume->root_size));
//	}
	return true;
}

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicTypeS1OS::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	// Volume情報はトラック0, セクタ2
	DiskImageSector *sector = basic->GetSector(0, 2);
	if (!sector) {
		return false;
	}
	dir_item->SetDataPtr(0, NULL, sector, 8, sector->GetSectorBuffer(8));
	return DiskBasicType::AssignRootDirectory(start_sector, end_sector, group_items, dir_item);
}

#if 0
/// ディレクトリをアサイン
/// @param [in]     is_root      ルートか
/// @param [in]     group_items  セクタリスト
/// @param [in,out] dir_item     ディレクトリアイテム
/// @return true / false
bool DiskBasicTypeS1OS::AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	return DiskBasicType::AssignDirectory(is_root, group_items, dir_item);
}
#endif

/// セクタをディレクトリとして初期化
/// @param [in]     group_items 確保したセクタリスト
/// @param [in,out] file_size   サイズ
/// @param [in,out] size_remain 残りサイズ
/// @param [in,out] errinfo     エラー情報
/// @return 0:正常 <0:エラー 
int DiskBasicTypeS1OS::InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &size_remain, DiskBasicError &errinfo)
{
	int rc = 0;

	DiskBasicDirItem *newitem = basic->CreateDirItem(NULL, 0, NULL);
	int dir_size = (int)newitem->GetDataSize();	// 1アイテムのサイズ
	int index_number = 0;
	int pos = 0;
	int finish = 0;
//	wxUint32 prev_grp_num = (wxUint32)-1;
	myLog.SetDebug(wxT("A: file_size:%d dir_size:%d"),
		file_size, dir_size);
	for(size_t idx = 0; idx < group_items.Count() && finish >= -1 && rc >= 0; idx++) {
		const DiskBasicGroupItem *gitem = &group_items.Item(idx);
		wxUint32 grp_num = gitem->group;
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		DiskImageTrack *track = basic->GetTrack(trk_num, sid_num);
		if (!track) {
			// トラックがない！
			errinfo.SetError(DiskBasicError::ERRV_NO_TRACK, grp_num, trk_num, sid_num);
			rc = -2;
			break;
		}

		const DiskBasicGroupItem *next_gitem = idx + 1 < group_items.Count() ? group_items.ItemPtr(idx + 1) : NULL;

		for(int sec_num = gitem->sector_start; sec_num <= gitem->sector_end && finish >= -1 && rc >= 0; sec_num++) {
			DiskImageSector *sector = basic->GetSector(trk_num, sid_num, sec_num);
			if (!sector) {
				// セクタがない！
				errinfo.SetError(DiskBasicError::ERRV_NO_SECTOR, grp_num, trk_num, sid_num, sec_num);
				rc = -2;
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				// セクタがない！
				errinfo.SetError(DiskBasicError::ERRV_NO_SECTOR, grp_num, trk_num, sid_num, sec_num);
				rc = -2;
				break;
			}

			int size = sector->GetSectorSize();

			SectorParam next_sec;
			if (sec_num < gitem->sector_end) {
				next_sec.SetSectorParam(trk_num, sid_num, sec_num + 1, -1);
			} else {
				if (next_gitem) {
					next_sec.SetSectorParam(next_gitem->track, next_gitem->side, next_gitem->sector_start, -1);
				} else {
					next_sec.SetSectorParam(trk_num, sid_num, -1, -1);
				}
			}

			// ディレクトリ拡張の場合ファイルサイズからオフセットを計算する
			if (idx == 0 && file_size > 0) {
				int data_size = size - basic->GetDirStartPosOnSector();
				int overflow = file_size + dir_size;
				overflow %= data_size;

				buffer += overflow;
				pos    += overflow;
				size_remain -= overflow;
			}

			// ディレクトリエリア各セクタの先頭をスキップする位置
			buffer += basic->GetDirStartPosOnSector();
			pos    += basic->GetDirStartPosOnSector();
			size_remain -= basic->GetDirStartPosOnSector();

			while(pos < size) {
				myLog.SetDebug(wxT("A: C:%d H:%d R:%d pos:%d remain:%d"),
					trk_num, sid_num, sec_num,
					pos, size_remain);
				finish = FinishAssigningDirectory(pos, size, size_remain);
				if (finish < 0) {
					// 終了
					break;
				}
				// ディスク内に書き込む
				newitem->SetDataPtr(index_number, gitem, sector, pos, buffer, &next_sec);
				// 初期値を入れる
				newitem->InitialData();

				pos    += dir_size;
				buffer += dir_size;
//				file_size  += dir_size;
				size_remain -= dir_size;
				index_number++;
			}

			pos -= size;
			pos = AdjustPositionAssigningDirectory(pos);
		}
	}

	delete newitem;

	return rc;
}

/// ディレクトリエリアのサイズに達したらアサイン終了するか
/// @param[in,out] pos         ディレクトリの位置
/// @param[in,out] size        ディレクトリのセクタサイズ
/// @param[in,out] size_remain ディレクトリの残りサイズ
/// @retval  0 : 終了しない
/// @retval  1 : 強制的に未使用とする アサインは継続
/// @retval -1 : 現グループでアサイン終了。次のグループから継続
/// @retval -2 : 強制的にアサイン終了する
int DiskBasicTypeS1OS::FinishAssigningDirectory(int &pos, int &size, int &size_remain) const
{
	return size_remain < (int)sizeof(directory_s1os_t) ? -1 : 0;
}

#if 0
/// ディレクトリアサインでセクタ毎に位置を調整する
/// @param[in] pos ディレクトリの位置
/// @return 調整後のディレクトリの位置
int DiskBasicTypeS1OS::AdjustPositionAssigningDirectory(int pos)
{
	return pos;
}
#endif

/// 番号をスキューマップを適用した後のグループ番号に変換
wxUint32 DiskBasicTypeS1OS::ToGroupNum(wxUint32 sgnum)
{
	wxUint32 secs = (wxUint32)basic->GetSectorsPerTrack();
	wxUint32 sec = sgnum % secs;
	wxUint32 trk = sgnum / secs;
	sec = m_skew_map[sec];
	return trk * secs + sec;
}

/// グループ番号をスキューマップを適用する前の番号に変換
wxUint32 DiskBasicTypeS1OS::FromGroupNum(wxUint32 group_num)
{
	wxUint32 secs = (wxUint32)basic->GetSectorsPerTrack();
	wxUint32 sec = group_num % secs;
	wxUint32 trk = group_num / secs;
	sec = m_skew_rmap[sec];
	return trk * secs + sec;
}

/// 使用可能なディスクサイズを得る
/// @param [out] disk_size  ディスクサイズ
/// @param [out] group_size グループ数
void DiskBasicTypeS1OS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1 - data_start_group;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
/// @param [in] wrote 書込み操作を行った後か
void DiskBasicTypeS1OS::CalcDiskFreeSize(bool wrote)
{
//	int used = 0;
	fat_availability.Empty();

	// 使用済みかチェック
	int grps = 0;
	int fsts;
//	wxUint32 sectors = basic->GetSectorsPerTrackOnBasic();
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup(); gnum++) {
		if (gnum < data_start_group) {
//			used++;
			fsts = FAT_AVAIL_SYSTEM;
		} else {
			wxUint32 sgnum = FromGroupNum(gnum);
			if (!IsUsedGroupNumber(sgnum)) {
				grps++;
				fsts = FAT_AVAIL_FREE;
			} else {
//				used++;
				fsts = FAT_AVAIL_USED;
			}
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

#if 0
/// 使用しているグループの位置を得る
void DiskBasicTypeS1OS::CalcUsedGroupPos(wxUint32 num, int &pos, int &mask)
{
	mask = 1 << (pos & 7);
	pos = (pos >> 3);
}
#endif

/// FAT位置をセット
/// @param [in] num グループ番号
/// @param [in] val 0 or 1
void DiskBasicTypeS1OS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	m_bitmap.Modify(num, val != 0);
}

/// FAT位置を返す
/// @param [in] num グループ番号
wxUint32 DiskBasicTypeS1OS::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// 使用しているグループ番号か
/// @param [in] num グループ番号
bool DiskBasicTypeS1OS::IsUsedGroupNumber(wxUint32 num)
{
	return m_bitmap.IsSet(num);
}

#if 0
/// 次のグループ番号を得る
/// @param [in] num グループ番号
/// @param [in] sector_pos 未使用
wxUint32 DiskBasicTypeS1OS::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return num + 1;
}

/// 空きFAT位置を返す
/// @return 
wxUint32 DiskBasicTypeS1OS::GetEmptyGroupNumber()
{
	wxUint32 found = INVALID_GROUP_NUMBER;
	for(wxUint32 grp_num = data_start_group; grp_num <= basic->GetFatEndGroup(); grp_num++) {
		if (!IsUsedGroupNumber(grp_num)) {
			found = grp_num;
			break;
		}
	}
	return found;
}
#endif

/// すべてのグループを取得
void DiskBasicTypeS1OS::CountGroups(s1os_size_t *size, s1os_ptr_t *ptr, DiskBasicGroups &group_items)
{
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 0;
	if (ptr->track == 0 && ptr->sector == 0) {
		// スタート設定がないので終り
		return;
	}
	DiskBasicDirItemS1OS::GetCHR(basic, ptr, trk_num, sid_num, sec_num);
	// 占有サイズ
	size_t max_secs = (size_t)basic->OrderUint32(size->secs);
	// 各セクタ先頭にあるチェイン情報をたどる
	// 占有サイズに関わらずチェインをたどるが
	// エラーなどで無限ループを防ぐため占有サイズ＋αで停止する。
	max_secs += 4;
	while(group_items.Count() <= max_secs) {
		DiskImageSector *sector = basic->GetSector(trk_num, sid_num, sec_num);
		if (!sector) break;

		wxUint32 group_num = GetGroupNumFromNum(trk_num, sid_num, sec_num);
		group_items.Add(group_num, 0, trk_num, sid_num, sec_num, sec_num, 0, 1);
		group_items.AddNums(1);

		// 次のセクタ番号を得る
		s1os_chain_t *chain = (s1os_chain_t *)sector->GetSectorBuffer();
		DiskBasicDirItemS1OS::GetCHR(basic, &chain->next, trk_num, sid_num, sec_num);

		if (trk_num == 0 && sec_num == 0) break;
	}
	// 渡すサイズはセクタサイズ * グループ数
	group_items.SetSize(group_items.Count() * basic->GetSectorSize());
}

#if 0
/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeS1OS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	return true;
}
#endif

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    確保するデータサイズ（バイト）
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeS1OS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int file_size = 0;
	int groups = 0;

	int rc = 0;
	int remain = 0;
	int sec_size = basic->GetSectorSize() - (int)sizeof(s1os_chain_t);

	// 必要なグループ数
	wxUint32 group_start = 1;

	if (item->IsDirectory()) {
		// ディレクトリの場合
		// 新規にディレクトリを作ったときはセクタは確保しない
		// 既存ディレクトリに新規ファイルを追加する場合は以下
		if (flags == ALLOCATE_GROUPS_APPEND) {
			int bytes = basic->OrderUint16(item->GetData()->s1os.size.bytes);
			// ディレクトリエントリが足りるかをチェック
			if (bytes == 0 || bytes + (int)(sizeof(directory_s1os_t) + sizeof(s1os_chain_t)) >= data_size) {
				// 新たにセクタを確保する必要がある
				remain = data_size - (int)sizeof(s1os_chain_t);
				group_start = data_start_group;
			}
		}
	} else {
		// ファイルの場合
		remain = data_size;
		group_start = GetGroupNumFromNum(5, 0, 1);
	}

	if (remain > 0) {
		// 新たなセクタを確保できるか探す
		wxArrayInt reserved_groups;
		bool found = FindEmptySectors(group_start, remain, sec_size, reserved_groups);
		if (!found) {
			// 十分な空きがない
			rc = -1;
			return rc;
		}
		// 最初の空きグループ番号
		group_start = reserved_groups.Item(0);
		if (flags == ALLOCATE_GROUPS_NEW || item->GetStartGroup(fileunit_num) == (wxUint32)-1) {
			// 新規orグループ未設定ならデータの開始グループ決定
			item->SetStartGroup(fileunit_num, group_start);
		}
		// 領域を確保する
		rc = AllocateSectors(item, group_start, remain, sec_size, reserved_groups, group_items, file_size, groups);
		// 確保したグループ数をセット
		item->SetGroupSize(groups);
	}
	return rc;
}

/// 未使用領域をさがす
/// @param [in]     group_start 開始グループ番号
/// @param [in]     remain      確保するデータサイズ（バイト）
/// @param [in]     sec_size    セクタサイズ(チェイン部分を除く)
/// @param [in,out] reserved_groups 予約したグループリスト
/// @return 確保できたらtrue
bool DiskBasicTypeS1OS::FindEmptySectors(wxUint32 group_start, int remain, int sec_size, wxArrayInt &reserved_groups)
{
	for(int i=0; i<2; i++) {
		wxUint32 st, ed;
		if (i == 0) {
			st = group_start;
			ed = basic->GetFatEndGroup() + 1;
		} else {
			st = data_start_group;
			ed = group_start;
		}
		for(wxUint32 sgnum = st; sgnum < ed && remain > 0; sgnum++) {
			if (IsUsedGroupNumber(sgnum)) {
				continue;
			}
			// 未使用部分のセクタが存在するかをチェック
			int trk_num = 0;
			int sid_num = 0;
			int sec_num = 0;
			wxUint32 group_num = ToGroupNum(sgnum);
			GetNumFromGroupNum(group_num, trk_num, sid_num, sec_num);
			DiskImageSector *sector = basic->GetSector(trk_num, sid_num, sec_num);
			if (!sector) {
				// セクタがない
				continue;
			}
			// 候補に追加
			reserved_groups.Add(group_num);
			remain -= (sec_size * basic->GetSectorsPerGroup());
		}
		if (remain <= 0) {
			break;
		}
	}
	return (remain <= 0);
}

/// グループを確保して使用中にする
/// @param [in,out] item        ディレクトリアイテム
/// @param [in]     group_start 開始グループ番号
/// @param [in]     remain      確保するデータサイズ（バイト）
/// @param [in]     sec_size    セクタサイズ(チェイン部分を除く)
/// @param [in]     reserved_groups 予約したグループリスト
/// @param [in,out] group_items 確保したグループリスト
/// @param [out]    file_size   確保したサイズ
/// @param [out]    groups      確保したグループの数
/// @return 確保できたらtrue
int DiskBasicTypeS1OS::AllocateSectors(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, const wxArrayInt &reserved_groups, DiskBasicGroups &group_items, int &file_size, int &groups)
{
	int rc = 0;
	int limit = basic->GetFatEndGroup() + 1;
	size_t count = reserved_groups.Count();
	for(size_t n = 0; n < count && remain > 0 && limit >= 0; n++) {
		wxUint32 group_num = reserved_groups.Item(n);
		wxUint32 next_group_num = 0;
		if (n < (count - 1)) {
			next_group_num = reserved_groups.Item(n + 1);
		}
		// 使用済みにする
		basic->GetNumsFromGroup(group_num, next_group_num, sec_size, remain, group_items);
		wxUint32 sgnum = FromGroupNum(group_num);
		SetGroupNumber(sgnum, 1);
		//
		file_size += (sec_size * basic->GetSectorsPerGroup());
		remain -= (sec_size * basic->GetSectorsPerGroup());
		groups++;
		limit--;
	}
	if (limit < 0) {
		// 無限ループ？
		rc = -2;
	}
	return rc;
}

/// グループ番号から開始セクタ番号を得る
/// @param [in] group_num グループ番号
/// @return 開始セクタ番号
int DiskBasicTypeS1OS::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num * basic->GetSectorsPerGroup();
}

/// グループからトラック、サイド、セクタの各番号を得る
/// @param [in]  group_num    グループ番号
/// @param [out] track_num    トラック番号
/// @param [out] side_num     サイド番号
/// @param [out] sector_num   セクタ番号
void DiskBasicTypeS1OS::GetNumFromGroupNum(wxUint32 group_num, int &track_num, int &side_num, int &sector_num)
{
	int sector_pos = GetStartSectorFromGroup(group_num);
	GetNumFromSectorPos(sector_pos, track_num, side_num, sector_num);
}

/// トラック、サイド、セクタ番号からグループ番号を得る
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  セクタ番号
/// @return グループ番号
wxUint32 DiskBasicTypeS1OS::GetGroupNumFromNum(int track_num, int side_num, int sector_num)
{
	int sector_pos = GetSectorPosFromNum(track_num, side_num, sector_num);
	wxUint32 group_num = (wxUint32)(sector_pos / basic->GetSectorsPerGroup());
	return group_num;
}

#if 0
/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos    セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num    トラック番号
/// @param [out] side_num     サイド番号
/// @param [out] sector_num   セクタ番号
/// @param [out] div_num      分割番号
/// @param [out] div_nums     分割数
void DiskBasicTypeS1OS::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
	DiskBasicType::GetNumFromSectorPos(sector_pos, track_num, side_num, sector_num, div_num, div_nums);
//	// セクタスキュ(インターリーブ)を適用 セクタ番号を変更する
//	if (sector_num > 0 && sector_num <= MAX_SECTORS) {
//		sector_num = m_skew_map[sector_num - 1] + 1;
//	}
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int DiskBasicTypeS1OS::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
//	// セクタスキュ(インターリーブ)を適用 セクタ番号を変更する
//	if (sector_num > 0 && sector_num <= MAX_SECTORS) {
//		sector_num = m_skew_rmap[sector_num - 1] + 1;
//	}
	return DiskBasicType::GetSectorPosFromNum(track_num, side_num, sector_num, div_num, div_nums);
}
#endif

/// ルートディレクトリか
/// @param [in] group_num グループ番号
/// @return true / false
bool DiskBasicTypeS1OS::IsRootDirectory(wxUint32 group_num)
{
	return (group_num == data_start_group);
}

#if 0
/// 未使用のディレクトリアイテムを返す
/// @param [in,out] parent    ディレクトリ
/// @param [in,out] items     ディレクトリアイテム一覧
/// @param [in,out] pitem     ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out]    next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicTypeS1OS::GetEmptyDirectoryItem(DiskBasicDirItem *parent, DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	if (items) {
		for(size_t i=0; i < items->Count(); i++) {
			DiskBasicDirItem *item = items->Item(i);
			if (!item->IsUsed()) {
				match_item = item;
				if (next_item) {
					i++;
					if (i < items->Count() && !items->Item(i)->IsUsed()) {
						*next_item = items->Item(i);
					} else {
						*next_item = NULL;
					}
				}
				break;
			}
		}
	}
	return match_item;
}
#endif

/// サブディレクトリを作成した後の個別処理
/// @param [in] item        ディレクトリアイテム
/// @param [in] group_items グループリスト
/// @param [in] parent_item 親ディレクトリアイテム
void DiskBasicTypeS1OS::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	// ディレクトリの初期サイズ
	item->SetFileSize(0);

	// 親ディレクトリのサイズを更新
	DiskBasicDirItem *dir_item = item->GetParent();
	if (dir_item) {
		int dir_size = dir->CalcSize(dir_item);
		dir_item->SetFileSize(dir_size);
	}
}

/// ディレクトリ拡張後の個別処理
/// @param [in] item        ディレクトリアイテム
/// @param [in] group_items グループリスト
/// @param [in] parent_item 親ディレクトリアイテム
/// @return true
bool DiskBasicTypeS1OS::AdditionalProcessOnExpandedDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	DiskImageSector *sector = NULL;
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 0;

	if (group_items.Count() == 0) {
		// Why?
		return false;
	}

	DiskBasicDirItemS1OS::GetCHR(basic, &item->GetData()->s1os.start, trk_num, sid_num, sec_num);
	sector = basic->GetSector(trk_num, sid_num, sec_num);

	DiskBasicGroupItem *gitem = &group_items.Item(0);
	bool found = (trk_num == gitem->track && sid_num == gitem->side && sec_num == gitem->sector_start);

	// チェインの最後に新たに確保したセクタを追加する
	int ptrk_num = 0;
	int psid_num = 0;
	int psec_num = 0;
	while(sector && !found) {
		s1os_chain_t *chain = (s1os_chain_t *)sector->GetSectorBuffer();
		ptrk_num = trk_num;
		psid_num = sid_num;
		psec_num = sec_num;
		DiskBasicDirItemS1OS::GetCHR(basic, &chain->next, trk_num, sid_num, sec_num);
		if (trk_num <= 0 && sid_num <= 0 && sec_num <= 0) {
			// 新規に追加するセクタ番号を書く
			trk_num = gitem->track;
			sid_num = gitem->side;
			sec_num = gitem->sector_start;
			DiskBasicDirItemS1OS::SetCHR(basic, trk_num, sid_num, sec_num, &chain->next);
			found = true;
		}
		sector = basic->GetSector(trk_num, sid_num, sec_num);
	}
	if (sector && found) {
		// 次のセクタのチェイン情報を更新
		s1os_chain_t *chain = (s1os_chain_t *)sector->GetSectorBuffer();
		memset(chain, 0, sizeof(s1os_chain_t));
		DiskImageSector *psector = item->GetSector();
		// ディレクトリエントリの位置を書く
		if (psector) {
			DiskBasicDirItemS1OS::SetCHR(basic, psector->GetIDC(), psector->GetIDH(), psector->GetIDR(), &chain->entry.s);
			chain->entry.top = basic->OrderUint16(item->GetPosition());
		}
		// 前のセクタの位置を書く
		DiskBasicDirItemS1OS::SetCHR(basic, ptrk_num, psid_num, psec_num, &chain->prev);
	}

//	// 拡張したのでディレクトリエントリ１つ分のサイズを加算
//	int size = item->GetFileSize();
//	size += (int)sizeof(directory_s1os_t);
//	item->SetFileSize(size);

	return true;
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
/// @return >=0 : 処理したサイズ  -1:比較不一致  -2:セクタがおかしい
int DiskBasicTypeS1OS::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	// セクタは先頭のチェイン部分を除く
	sector_size -= (int)sizeof(s1os_chain_t);
	sector_buffer += sizeof(s1os_chain_t);

	int size = (remain_size < sector_size ? remain_size : sector_size);

	if (ostream) {
		// 書き出し
		temp.SetData(sector_buffer, size, basic->IsDataInverted());

		ostream->Write((const void *)temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read((void *)temp.GetData(), temp.GetSize());
		temp.InvertData(basic->IsDataInverted());

		if (memcmp(temp.GetData(), sector_buffer, temp.GetSize()) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット後の処理
/// @param [in] data ダイアログで入力したデータ（ラベル名など）
/// @return true / false
bool DiskBasicTypeS1OS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskImageSector *sector;

	//
	// Volume エリア
	//
	sector = basic->GetSector(0, 2);
	if (!sector) {
		return false;
	}
	p_volume = (s1os_volume_info_t *)sector->GetSectorBuffer();
	p_volume->header = basic->OrderUint16(0x701c);
	memset(p_volume->unknown, 0, sizeof(p_volume->unknown));
	p_volume->wrote_sectors = 0;
	p_volume->root_size.secs = basic->OrderUint32(1);
	p_volume->root_size.bytes = 0;
	DiskBasicDirItemS1OS::SetCHR(basic, 0, 1, 1, &p_volume->root_start);
	data_start_group = GetSectorPosFromNum(0, 1, 1);
	TM tm;
	tm.Now();
	p_volume->date_time = DiskBasicDirItemS1OS::ConvToFileDateTime(basic, tm);
	p_volume->attr.k = FILETYPE_S1OS_D;
	p_volume->attr.s = FILETYPE_S1OS_N;
	p_volume->attr.p = FILETYPE_S1OS_L;
	p_volume->name_len = 0;

	SetIdentifiedData(data);

	//
	// Skewエリア
	//
	DiskBasicBitMLMap bmap;
	sector = basic->GetSector(0, 3);
	if (!sector) {
		return false;
	}
	s1os_skew_bitmap_t *skew_bmp = (s1os_skew_bitmap_t *)sector->GetSectorBuffer();
	wxUint32 count = (wxUint32)basic->GetSectorsPerTrackOnBasic();
	if (count > (wxUint32)(sizeof(skew_bmp->skew)/sizeof(wxUint16))) {
		count = (wxUint32)(sizeof(skew_bmp->skew)/sizeof(wxUint16));
	}
	wxUint32 sec_bits = 0;
	wxUint32 sec_pos = 0;
	// インターリーブを考慮してセクタ位置を設定する
	for(wxUint32 pos = 0; pos < count; pos++) {
		skew_bmp->skew[pos] = basic->OrderUint16(sec_pos + 1);
		sec_bits |= (1 << sec_pos);
		sec_pos += m_soft_interleave;
		if (sec_pos >= count) {
			sec_pos -= count;
			while(sec_bits & (1 << sec_pos)) {
				sec_pos++;
				if (sec_pos >= count) {
					break;
				}
			}
		}
	}

	//
	// Bitmapエリア
	//
	bmap.AddBuffer(&skew_bmp->bitmap, sector->GetSectorBufferSize() - sizeof(skew_bmp->skew));
	for(int sec_num = 4; sec_num <= 5; sec_num++) {
		sector = basic->GetSector(0, sec_num);
		if (!sector) {
			return false;
		}
		bmap.AddBuffer(sector->GetSectorBuffer(), sector->GetSectorSize());
	}
	// 最初からrootディレクトリまで使用中にする
	for(wxUint32 sec_pos = 0; sec_pos <= basic->GetFatEndGroup(); sec_pos++) {
		bmap.Modify(sec_pos, sec_pos <= data_start_group);
	}

	//
	// rootディレクトリ
	//
	sector = basic->GetSector(0, 1, 1);
	if (!sector) {
		return false;
	}
	// 0で初期化
	sector->Fill(0);

	return true;
}

/// データの書き込み処理
/// @param [in]	 item			ディレクトリアイテム
/// @param [in]	 istream		ストリームデータ
/// @param [out] buffer			セクタ内の書き込み先バッファ
/// @param [in]  size			書き込み先バッファサイズ
/// @param [in]  remain			残りのデータサイズ
/// @param [in]  sector_num		セクタ番号
/// @param [in]  group_item		現在のグループアイテム
/// @param [in]  next_group		次のグループアイテム
/// @param [in]  sector_end		最終セクタ番号
/// @param [in]  seq_num		通し番号(0...)
/// @return 書き込んだバイト数
int DiskBasicTypeS1OS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, DiskBasicGroupItem *group_item, DiskBasicGroupItem *next_group, int sector_end, int seq_num)
{
	size -= (int)sizeof(s1os_chain_t);
	wxUint8 *data_buff = buffer + sizeof(s1os_chain_t);
	s1os_chain_t *chain = (s1os_chain_t *)buffer;

	int len;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (remain > 0) {
			istream.Read((void *)data_buff, remain);
		}
		if (size > remain) {
			// バッファの余りは0サプレス
			memset((void *)&data_buff[remain], 0, size - remain);
		}
		len = remain;
	} else {
		// 継続
		istream.Read((void *)data_buff, size);
		len = size;
	}

	// チェイン情報を初期化
	memset(chain, 0, sizeof(s1os_chain_t));

	// 次のセクタのある位置を書く
	// ベリファイでチェインを追いかけるため仮で登録する
	if (next_group) {
		// セット
		DiskBasicDirItemS1OS::SetCHR(basic, next_group->track, next_group->side, next_group->sector_start, &chain->next);
	}
	return len;
}

/// データの書き込み終了後の処理
/// @param [in]	item        ディレクトリアイテム
void DiskBasicTypeS1OS::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	// ファイルのチェイン情報を更新
	const DiskBasicGroups *groups = &item->GetGroups();
	size_t count = groups->Count();
	if (count > 0) {
		DiskBasicGroupItem *prev = NULL;
		// エントリのあるセクタ
		DiskImageSector *psector = item->GetSector();
		for(size_t n = 0; n < count; n++) {
			DiskBasicGroupItem *curr = &groups->Item(n);
			// 現セクタ
			DiskImageSector *sector = basic->GetSector(curr->track, curr->side, curr->sector_start);
			if (!sector) {
				// Why?
				break;
			}
			s1os_chain_t *chain = (s1os_chain_t *)sector->GetSectorBuffer();
			// ディレクトリエントリの位置を書く
			if (psector) {
				DiskBasicDirItemS1OS::SetCHR(basic, psector->GetIDC(), psector->GetIDH(), psector->GetIDR(), &chain->entry.s);
				chain->entry.top = basic->OrderUint16(item->GetPosition());
			}
			// 次のセクタ番号を書く
			if (n < (count - 1)) {
				DiskBasicGroupItem *next = &groups->Item(n+1);
				DiskBasicDirItemS1OS::SetCHR(basic, next->track, next->side, next->sector_start, &chain->next);
			}
			// 前のセクタ番号を書く
			if (prev) {
				DiskBasicDirItemS1OS::SetCHR(basic, prev->track, prev->side, prev->sector_start, &chain->prev);
			}
			prev = curr;
		}
	}

	// 親ディレクトリのサイズを更新
	DiskBasicDirItem *dir_item = item->GetParent();
	if (dir_item) {
		int dir_size = dir->CalcSize(dir_item);
		dir_item->SetFileSize(dir_size);
	}
}

/// 指定したグループ番号のFAT領域を削除する
/// @param [in] group_items グループリスト
void DiskBasicTypeS1OS::DeleteGroups(const DiskBasicGroups &group_items)
{
	for(size_t gidx=0; gidx<group_items.Count(); gidx++) {
		DiskBasicGroupItem *gitem = &group_items.Item(gidx);
		// FATエントリを削除
		wxUint32 sgnum = FromGroupNum(gitem->group);
		DeleteGroupNumber(sgnum);
		// 各セクタのチェイン情報はクリアしない
#if 0
		DiskImageSector *sector = basic->GetSector(gitem->track, gitem->side, gitem->sector_start);
		if (!sector) continue;
		s1os_chain_t *chain = (s1os_chain_t *)sector->GetSectorBuffer();
		if (!chain) continue;
		memset(chain, 0, sizeof(s1os_chain_t));
#endif
	}
}

/// IPLや管理エリアの属性を得る
/// @param [out] data ダイアログで入力したデータ（ラベル名など）
void DiskBasicTypeS1OS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// Volume情報
	if (!p_volume) return;

	data.SetVolumeName(wxString((const char *)p_volume->name, (size_t)p_volume->name_len));
	data.SetVolumeNameMaxLength(sizeof(p_volume->name));
	data.SetVolumeSkew(m_soft_interleave);
}

/// IPLや管理エリアの属性をセット
/// @param [in] data ダイアログで入力したデータ（ラベル名など）
void DiskBasicTypeS1OS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	// Volume情報
	if (!p_volume) return;

	wxString vol = data.GetVolumeName();
	size_t len = vol.Length();
	if (len > sizeof(p_volume->name)) len = sizeof(p_volume->name);
	memcpy(p_volume->name, vol.c_str(), len);
	p_volume->name_len = (wxUint8)len;
	m_soft_interleave = data.GetVolumeSkew();
}
