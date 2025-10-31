/// @file basictype_hfs.cpp
///
/// @brief disk basic type for HFS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_hfs.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "basicdiritem_hfs.h"
#include "../utils.h"
#include "../logging.h"


//////////////////////////////////////////////////////////////////////
//
// HFS の処理
//
DiskBasicTypeHFS::DiskBasicTypeHFS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	p_hfs_mdb = NULL;
}

/// エリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeHFS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// MDB area
	DiskImageSector *sector = basic->GetSectorFromSectorPos(2);
	if (!sector) {
		return -1.0;
	}

	p_hfs_mdb = (hfs_mdb_t *)sector->GetSectorBuffer();
	if (!p_hfs_mdb) {
		return -1.0;
	}
	// HFS signature
	if (p_hfs_mdb->sig[0] != 'B' || p_hfs_mdb->sig[1] != 'D') {
		return -1.0;
	}
	// block number of the volume bitmap
	wxUint32 blk_vol = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drVBMSt);

	// The location of the first allocation block in the volume
	wxUint32 blk_fst = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drAlBlSt);

	// ボリュームビットマップ
	bitmap.Empty();
	for(int sec = (int)blk_vol; sec < (int)blk_fst; sec++) {
		sector = basic->GetSectorFromSectorPos(sec);
		if (!sector) {
			return -1.0;
		}
		wxUint8 *buffer = sector->GetSectorBuffer();
		if (!buffer) {
			return -1.0;
		}

		bitmap.AddBuffer(buffer, sector->GetSectorBufferSize());
	}

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeHFS::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 0.0;

	double valid_ratio = 1.0;

	// Boot area
	DiskImageSector *sector = basic->GetSectorFromSectorPos(0);
	if (!sector) {
		return -1.0;
	}

	hfs_boot_blk_hdr_t *boot = (hfs_boot_blk_hdr_t *)sector->GetSectorBuffer();
	if (!boot) {
		return -1.0;
	}
	if (boot->sig[0] != 'L' || boot->sig[1] != 'K') {
		valid_ratio *= 0.8;
	}

	// MDB area
	sector = basic->GetSectorFromSectorPos(2);
	if (!sector) {
		return -1.0;
	}

	p_hfs_mdb = (hfs_mdb_t *)sector->GetSectorBuffer();
	if (!p_hfs_mdb) {
		return -1.0;
	}
	// HFS signature
	if (p_hfs_mdb->sig[0] != 'B' || p_hfs_mdb->sig[1] != 'D') {
		return -1.0;
	}
	// block number of the volume bitmap
	wxUint32 blk_vol = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drVBMSt);
	if (blk_vol != 3) {
		valid_ratio -= 0.5;
	}
	// The allocation block size (in bytes)
	wxUint32 blk_siz = wxUINT32_SWAP_ON_LE(p_hfs_mdb->drAlBlkSiz);
	if (blk_siz & 0x1ff) {
		valid_ratio -= 0.5;
	}
	// The location of the first allocation block in the volume
	wxUint32 blk_fst = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drAlBlSt);
	if (blk_fst < blk_vol) {
		valid_ratio -= 0.5;
	}
	// Number of allocation blocks in the volume
	wxUint32 num_blks = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drNmAlBlks);

	// 各パラメータ
	int secs_per_grp = (int)blk_siz / 512;

	basic->SetReservedSectors(blk_fst);
	basic->SetSectorsPerGroup(secs_per_grp);
	// 最終データ位置
	basic->SetFatEndGroup(num_blks);

	// カタログファイル
	wxUint32 ctx_sta = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drCTExtRec.d[0].start);
	wxUint32 ctx_siz = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drCTExtRec.d[0].count);

	// カタログファイルの先頭
	basic->SetDirStartSector(secs_per_grp * ctx_sta + blk_fst);
	// カタログファイルの末尾
	basic->SetDirEndSector(secs_per_grp * (ctx_sta + ctx_siz) + blk_fst - 1);

	// 拡張オーバーフローファイル
	wxUint32 xtx_sta = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drXTExtRec.d[0].start);
	wxUint32 xtx_siz = wxUINT16_SWAP_ON_LE(p_hfs_mdb->drXTExtRec.d[0].count);

	// 拡張オーバーフローファイルの先頭
	basic->SetVariousParam(wxT("ExtStartSector"), (long)(secs_per_grp * xtx_sta + blk_fst));
	// 拡張オーバーフローファイルの末尾
	basic->SetVariousParam(wxT("ExtEndSector"), (long)(secs_per_grp * (xtx_sta + xtx_siz) + blk_fst - 1));

	return valid_ratio;
}

#if 0
/// トラックマップマスクを設定
void DiskBasicTypeHFS::SetTrackMapMask(wxUint32 val)
{
//	apledos_vtoc->track_bit_mask[0] = wxUINT16_SWAP_ON_LE(val & 0xffff);
//	val >>= 16;
//	apledos_vtoc->track_bit_mask[1] = wxUINT16_SWAP_ON_LE(val & 0xffff);
}

/// トラックマップのビットを変更
void DiskBasicTypeHFS::ModifyTrackMap(int track_num, int sector_num, bool use)
{
	wxUint32 map = GetTrackMap(track_num);
	if (use) {
		map &= ~(1 << sector_num);
	} else {
		map |= (1 << sector_num);
	}
	SetTrackMap(track_num, map);
}

/// 空いているか
bool DiskBasicTypeHFS::IsFreeTrackMap(int track_num, int sector_num) const
{
	wxUint32 map = GetTrackMap(track_num);
	return ((map & (1 << sector_num)) != 0);
}

/// トラックマップを設定
void DiskBasicTypeHFS::SetTrackMap(int track_num, wxUint32 val)
{
//	val <<= (16 - basic->GetSectorsPerTrackOnBasic());
//	apledos_vtoc->track_map[track_num][0] = wxUINT16_SWAP_ON_LE(val & 0xffff);
//	val >>= 16;
//	apledos_vtoc->track_map[track_num][1] = wxUINT16_SWAP_ON_LE(val & 0xffff);
}

/// トラックマップを得る
wxUint32 DiskBasicTypeHFS::GetTrackMap(int track_num) const
{
//	wxUint32 val = wxUINT16_SWAP_ON_LE(apledos_vtoc->track_map[track_num][1]);
//	val <<= 16;
//	val |= wxUINT16_SWAP_ON_LE(apledos_vtoc->track_map[track_num][0]);
//	val >>= (16 - basic->GetSectorsPerTrackOnBasic());
	return 0;
}
#endif

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
bool DiskBasicTypeHFS::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	bool valid = true;

	group_items.Empty();

	size_t dir_size = 0;
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;
	for(int sec = start_sector; sec < end_sector; sec++) {
		DiskImageSector *sector = basic->GetSectorFromSectorPos(sec, &trk_num, &sid_num);
		if (!sector) {
			valid = false;
			break;
		}

		sec_num = sector->GetSectorNumber();

		wxUint8 *buffer = sector->GetSectorBuffer(); 
		if (!buffer) {
			valid = false;
			break;
		}
		group_items.Add(0, 0, trk_num, sid_num, sec_num, sec_num);

		dir_size += sector->GetSectorSize();
	}

	group_items.SetSize(dir_size);

	return valid;
}

/// ディレクトリのチェック
/// @param [in]     is_root     ルートか
/// @param [in]    group_items  セクタリスト
/// @return <0.0 エラーあり 1.0:正常 使用しているディレクトリアイテムの有効度
double DiskBasicTypeHFS::CheckDirectory(bool is_root, const DiskBasicGroups &group_items)
{
	bool valid = true;
	bool last = false;
	int  n_used_items = 0;
	double n_normals = 0.0;

	int index_number = 0;

	DiskBasicDirItem *nitem = dir->NewItem(NULL, 0, NULL);

	size_t end_idx = group_items.Count();

	for(size_t idx = 0; idx < end_idx; idx++) {
		const DiskBasicGroupItem *gitem = group_items.ItemPtr(idx);
//		wxUint32 grp_num = gitem->group;
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		int sec_num = gitem->sector_start;

		DiskImageSector *sector = basic->GetSector(trk_num, sid_num, sec_num);
		if (!sector) {
			valid = false;
			break;
		}
		wxUint8 *buffer = sector->GetSectorBuffer();
		if (!buffer) {
			valid = false;
			break;
		}

		hfs_node_descriptor_t *node = (hfs_node_descriptor_t *)buffer;
		if (node->type == ndHdrNode) {
			// ヘッダノードは常に最初
			if (idx != 0) {
				valid = false;
				break;
			}
			hfs_bt_hdr_rec_t *header = (hfs_bt_hdr_rec_t *)&buffer[0xe];

			end_idx = (size_t)wxUINT32_SWAP_ON_LE(header->totalNodes) - wxUINT32_SWAP_ON_LE(header->freeNodes);
			continue;

		} else if (node->type != ndLeafNode) {
			// リーフノード以外は無視する
			continue;
		}

		// リーフノード

		// ノード内のレコード数
		int num_recs = wxUINT16_SWAP_ON_LE(node->numRecs);

		for(int rec = 0; rec < num_recs; rec++) {
			// セクタ末尾のレコードの位置を取得
			wxUint32 rpos = (wxUint32)buffer[510 - 2 * rec] * 256 + buffer[511 - 2 * rec]; 
			if (rpos < 0xe || rpos >= (wxUint32)sector->GetSectorSize()) {
				break;
			}
			int pos = (int)rpos;

			// レコードのキー部分
			hfs_cat_key_rec_t* rec_key = (hfs_cat_key_rec_t *)&buffer[rpos];
			if (rec_key->keyLength > 37) {
				// キー長すぎる
				continue;
			}
			rpos += rec_key->keyLength;
			rpos++;
			if (rpos & 1) {
				// ワード境界に合わせる
				rpos++;
			}
			// レコードのデータ部分
			hfs_cat_data_rec_t *rec_dat = (hfs_cat_data_rec_t *)&buffer[rpos];
			if(rec_dat->recType != FILETYPE_HFS_DIR && rec_dat->recType != FILETYPE_HFS_FILE) {
				// スレッドは無視
				continue;
			}
			// ID
			int id = index_number;
			switch(rec_dat->recType) {
			case FILETYPE_HFS_DIR:
				id = wxUINT32_SWAP_ON_LE(rec_dat->dir.id);
				break;
			case FILETYPE_HFS_FILE:
				id = wxUINT32_SWAP_ON_LE(rec_dat->file.id);
				break;
			}

			nitem->SetDataPtr(id, gitem, sector, pos, (wxUint8 *)rec_key);
			valid = nitem->Check(last);
			if (valid) {
				if (nitem->CheckUsed(false)) {
					n_normals += nitem->NormalCodesInFileName();
					n_used_items++;
				}
			}

			index_number++;
		}
	}

	double valid_ratio = 0.0;
	if (!valid) {
		valid_ratio = -1.0;
	} else if (n_used_items > 0) {
		valid_ratio = n_normals / (double)n_used_items;
	}

	delete nitem;

	return valid_ratio;
}

/// ディレクトリをアサイン
/// @param [in]     is_root      ルートか
/// @param [in]     group_items  セクタリスト
/// @param [in,out] dir_item     ディレクトリアイテム
/// @return true
bool DiskBasicTypeHFS::AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool valid = true;
	bool last = false;
	bool unuse = false;

	int index_number = 0;

	// 親ID
	int parent_id = 1;	// root
	if (!is_root) {
		// 子ディレクトリ
//		hfs_cat_data_rec_t *pdata = (hfs_cat_data_rec_t *)dir_item->GetData();
//		if (pdata) {
//			parent_id = wxUINT32_SWAP_ON_LE(pdata->dir.id);
//		}
		parent_id = dir_item->GetNumber();
	}

	size_t end_idx = group_items.Count();

	for(size_t idx = 0; idx < end_idx; idx++) {
		const DiskBasicGroupItem *gitem = group_items.ItemPtr(idx);
//		wxUint32 grp_num = gitem->group;
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		int sec_num = gitem->sector_start;

		DiskImageSector *sector = basic->GetSector(trk_num, sid_num, sec_num);
		if (!sector) {
			valid = false;
			break;
		}
		wxUint8 *buffer = sector->GetSectorBuffer();
		if (!buffer) {
			valid = false;
			break;
		}

		hfs_node_descriptor_t *node = (hfs_node_descriptor_t *)buffer;
		if (node->type == ndHdrNode) {
			// ヘッダノードは常に最初
			if (idx != 0) {
				valid = false;
				break;
			}
			hfs_bt_hdr_rec_t *header = (hfs_bt_hdr_rec_t *)&buffer[0xe];

			end_idx = (size_t)wxUINT32_SWAP_ON_LE(header->totalNodes) - wxUINT32_SWAP_ON_LE(header->freeNodes);
			continue;

		} else if (node->type != 0xff) {
			// リーフノード以外は無視する
			continue;
		}

		// リーフノード

		// ノード内のレコード数
		int num_recs = wxUINT16_SWAP_ON_LE(node->numRecs);

		for(int rec = 0; rec < num_recs; rec++) {
			// セクタ末尾のレコードの位置を取得
			wxUint32 rpos = (wxUint32)buffer[510 - 2 * rec] * 256 + buffer[511 - 2 * rec]; 
			if (rpos < 0xe || rpos >= (wxUint32)sector->GetSectorSize()) {
				break;
			}
			int pos = (int)rpos;

			// レコードのキー部分
			hfs_cat_key_rec_t* rec_key = (hfs_cat_key_rec_t *)&buffer[rpos];
			if (rec_key->keyLength > 37) {
				// キー長すぎる
				continue;
			}
			wxUint32 p_id = wxUINT32_SWAP_ON_LE(rec_key->parentID);
			if (p_id != (wxUint32)parent_id) {
				// 親ID一致しない
				continue;
			}

			rpos += rec_key->keyLength;
			rpos++;
			if (rpos & 1) {
				// ワード境界に合わせる
				rpos++;
			}
			// レコードのデータ部分
			hfs_cat_data_rec_t *rec_dat = (hfs_cat_data_rec_t *)&buffer[rpos];
			if(rec_dat->recType != FILETYPE_HFS_DIR && rec_dat->recType != FILETYPE_HFS_FILE) {
				// スレッドは無視
				continue;
			}
			// ID
			int id = index_number;
			switch(rec_dat->recType) {
			case FILETYPE_HFS_DIR:
				id = wxUINT32_SWAP_ON_LE(rec_dat->dir.id);

				{
					// 新規アイテムを作成
					DiskBasicDirItem *nitem = dir->NewItem(id, gitem, sector, pos, (wxUint8 *)rec_key, NULL, unuse);
					// 属性を設定
					nitem->SetFileAttr(DiskBasicFileType(FORMAT_TYPE_MACHFS, FILE_TYPE_DIRECTORY_MASK));
					// ファイルサイズの計算
					nitem->CalcFileSize();

					valid = nitem->Check(last);
					if (valid) {
						if (nitem->CheckUsed(false)) {
						}
					}

					// 親ディレクトリを設定
					nitem->SetParent(dir_item);
					// 子ディレクトリに追加
					dir_item->AddChild(nitem);

					index_number++;
				}

				break;

			case FILETYPE_HFS_FILE:
				id = wxUINT32_SWAP_ON_LE(rec_dat->file.id);

				// データフォークとリソースフォークそれぞれでアイテムを作成する
				if (rec_dat->file.datF.LogicalSize > 0) {
					// 新規アイテムを作成
					DiskBasicDirItem *nitem = dir->NewItem(id, gitem, sector, pos, (wxUint8 *)rec_key, NULL, unuse);
					// 属性を設定
					nitem->SetFileAttr(DiskBasicFileType(FORMAT_TYPE_MACHFS, FILE_TYPE_DATA_MASK));
					// ファイルサイズの計算
					nitem->CalcFileSize();

					valid = nitem->Check(last);
					if (valid) {
						if (nitem->CheckUsed(false)) {
						}
					}

					// 親ディレクトリを設定
					nitem->SetParent(dir_item);
					// 子ディレクトリに追加
					dir_item->AddChild(nitem);

					index_number++;
				}
				if (rec_dat->file.resF.LogicalSize > 0) {
					// 新規アイテムを作成
					DiskBasicDirItem *nitem = dir->NewItem(id, gitem, sector, pos, (wxUint8 *)rec_key, NULL, unuse);
					// 属性を設定
					nitem->SetFileAttr(DiskBasicFileType(FORMAT_TYPE_MACHFS, FILE_TYPE_RANDOM_MASK));
					// ファイルサイズの計算
					nitem->CalcFileSize();

					valid = nitem->Check(last);
					if (valid) {
						if (nitem->CheckUsed(false)) {
						}
					}

					// 親ディレクトリを設定
					nitem->SetParent(dir_item);
					// 子ディレクトリに追加
					dir_item->AddChild(nitem);

					index_number++;
				}

				break;
			}


		}
	}

	return true;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeHFS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	disk_size = group_size * basic->GetSectorSize();
}

/// 残りディスクサイズを計算
void DiskBasicTypeHFS::CalcDiskFreeSize(bool wrote)
{
	fat_availability.Empty();

	int size = basic->GetSectorSize() * basic->GetSectorsPerGroup();

	int res_grp = basic->GetReservedSectors() / basic->GetSectorsPerGroup();

	int ctx_sta = basic->GetDirStartSector() / basic->GetSectorsPerGroup() - res_grp;
	int ctx_end = basic->GetDirEndSector() / basic->GetSectorsPerGroup() - res_grp;
	int xtx_sta = basic->GetVariousIntegerParam(wxT("ExtStartSector")) / basic->GetSectorsPerGroup() - res_grp;
	int xtx_end = basic->GetVariousIntegerParam(wxT("ExtEndSector")) / basic->GetSectorsPerGroup() - res_grp;

	// check bitmap
	int grp_end = (int)basic->GetFatEndGroup();
	for(int grp = 0; grp < grp_end; grp++) {
		if (!bitmap.IsSet(grp)) {
			fat_availability.Add(FAT_AVAIL_FREE, size, 1);
		} else if ((grp >= ctx_sta && grp <= ctx_end) || (grp >= xtx_sta && grp <= xtx_end)) {
			fat_availability.Add(FAT_AVAIL_SYSTEM, 0, 0);
		} else {
			fat_availability.Add(FAT_AVAIL_USED, 0, 0);
		}
	}
	// alternate MDB
	for(int i=0; i<(2 / basic->GetSectorsPerGroup()); i++) {
		fat_availability.Add(FAT_AVAIL_SYSTEM, 0, 0);
	}
}

/// グループ番号を使用済みにする
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeHFS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
#if 0
	int track_num = 0;
	int sector_num = 0;
	GetNumFromSectorPosS(num, track_num, sector_num);
	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();
	ModifyTrackMap(track_num, sector_num, val != 0);
#endif
}

/// グループ番号を得る
wxUint32 DiskBasicTypeHFS::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeHFS::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeHFS::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeHFS::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
#if 0
	int manage_track_num = basic->GetManagedTrackNumber();
	int sta_trk = 0;
	int end_trk = 0;
	int ndir = 1;

	for(int i=0; i<2 && new_num == INVALID_GROUP_NUMBER; i++) {
		switch(i) {
		case 0:
			// 管理エリアより大きなトラック番号から
			sta_trk = manage_track_num + 1;
			end_trk = basic->GetTracksPerSideOnBasic();
			ndir = 1;
			break;
		case 1:
			// 管理エリアより小さなトラック番号へ
			sta_trk = manage_track_num - 1;
			end_trk = 2;
			ndir = -1;
			break;
		}

		for(int sec = basic->GetSectorsPerTrackOnBasic() - 1; sec >= 0; sec--) {
			for(int trk = sta_trk; trk != end_trk && new_num == INVALID_GROUP_NUMBER; trk += ndir) {
				// 各トラックの末尾を優先して探す
				if (IsFreeTrackMap(trk, sec)) {
					// 空きあり
					new_num = trk * basic->GetSectorsPerTrackOnBasic() + sec;
					break;
				}
			}
		}
	}
#endif
	return new_num;
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeHFS::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	// 次の空き位置候補
	wxUint32 new_num = INVALID_GROUP_NUMBER;
#if 0
	int manage_track_num = basic->GetManagedTrackNumber();
	int curr_trk = curr_group / basic->GetSectorsPerTrackOnBasic();
	int sta_trk = 0;
	int end_trk = 0;
	int ndir = 1;

	for(int i=0; i<2 && new_num == INVALID_GROUP_NUMBER; i++) {
		switch(i) {
		case 0:
			// 管理エリアより大きなトラック番号から
			sta_trk = curr_trk;
			end_trk = basic->GetTracksPerSideOnBasic();
			ndir = 1;
			break;
		case 1:
			// 管理エリアより小さなトラック番号へ
			sta_trk = curr_trk;
			end_trk = 2;
			ndir = -1;
			break;
		}

		for(int trk = sta_trk; trk != end_trk && new_num == INVALID_GROUP_NUMBER; trk += ndir) {
			if (trk == manage_track_num) {
				trk += ndir;
			}
			for(int sec = basic->GetSectorsPerTrackOnBasic() - 1; sec >= 0; sec--) {
				if (IsFreeTrackMap(trk, sec)) {
					// 空きあり
					new_num = trk * basic->GetSectorsPerTrackOnBasic() + sec;
					break;
				}
			}
		}
	}
#endif
	return new_num;
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeHFS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	// チェインセクタをクリア
	nitem->ClearChainSector();

	return true;
}

#if 0
/// チェインセクタを確保する
/// @param [in] idx          チェイン番号
/// @param [in] item         ディレクトリアイテム
/// @param [in] curr_group   事前に確保したグループ番号
/// @return 確保したグループ番号 / INVALID_GROUP_NUMBER
wxUint32 DiskBasicTypeHFS::AllocChainSector(int idx, DiskBasicDirItem *item, wxUint32 curr_group)
{
	wxUint32 gnum = (idx == 0) ? GetEmptyGroupNumber() : GetNextEmptyGroupNumber(curr_group);
	if (gnum == INVALID_GROUP_NUMBER) {
		return INVALID_GROUP_NUMBER;
	}
	// セクタ
	DiskImageSector *sector = basic->GetSectorFromGroup(gnum);
	if (!sector) {
		return INVALID_GROUP_NUMBER;
	}
	wxUint8 *buf = sector->GetSectorBuffer();
	if (!buf) {
		return INVALID_GROUP_NUMBER;
	}
	sector->Fill(0);

	// チェイン情報にセクタをセット
	item->SetChainSector(sector, gnum, buf);

	// 開始グループを設定
	if (idx == 0) {
		item->SetStartGroup(0, gnum, 1);
	}

	// セクタを予約
	SetGroupNumber(gnum, 1);

	return gnum;
}
#endif

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeHFS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
#if 0
//	myLog.SetDebug("DiskBasicTypeHFS::AllocateGroups {");

//	int file_size = 0;
	int groups = 0;

	int rc = 0;
	int sec_size = basic->GetSectorSize();
	int remain = data_size;
	int limit = basic->GetFatEndGroup() + 1;
	int chain_idx = 0;
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	while(remain > 0 && limit >= 0) {
		// チェインセクタを確保
		if ((chain_idx % 1) == 0) {
			group_num = AllocChainSector(chain_idx, item, group_num);
			if (group_num == INVALID_GROUP_NUMBER) {
				// エラー
				rc = groups > 0 ? -2 : -1;
				return rc;
			}
		}

		// 空きをさがす
		group_num = GetNextEmptyGroupNumber(group_num);
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			rc = groups > 0 ? -2 : -1;
			return rc;
		}

		// 使用済みにする
		basic->GetNumsFromGroup(group_num, 0, sec_size, remain, group_items);
		SetGroupNumber(group_num, 1);

		// チェインセクタも更新
		item->AddChainGroupNumber(chain_idx, group_num);
		chain_idx++;

//		file_size += sec_size;
		groups++;

		remain -= sec_size;

		limit--;
	}
	if (limit < 0) {
		// 無限ループ？
		rc = -2;
	}

//	myLog.SetDebug("rc: %d }", rc);
	return rc;
#endif
	return -1;
}

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeHFS::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
#if 0
	// ファイルサイズはセクタサイズ境界なので要計算
	if (item->NeedCheckEofCode()) {
		// 終端コードの1つ前までを出力
		wxUint8 eof_code = basic->InvertUint8(basic->GetTextTerminateCode());
		for(int len = 0; len < remain_size; len++) {
			if (sector_buffer[len] == eof_code) {
				remain_size = len;
				break;
			}
		}
	} else {
		// 計算手段がないので残りサイズをそのまま返す
		if (istream) {
			// 比較時は、比較先のファイルサイズ
			remain_size = istream->GetLength() % sector_size;
		}
	}
#endif
	return remain_size;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeHFS::GetStartSectorFromGroup(wxUint32 group_num)
{
	int val = group_num * basic->GetSectorsPerGroup() + basic->GetReservedSectors();
	return val;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeHFS::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int val = (group_num + 1) * basic->GetSectorsPerGroup() + basic->GetReservedSectors() - 1;
	return val;
}

#if 0
/// セクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num 物理セクタ番号
/// @param [out] div_num    分割番号
/// @param [out] div_nums   分割数
void DiskBasicTypeHFS::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
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
	sector_num = ((sector_pos % groups_per_track) / groups_per_sector);
	if (div_num) *div_num = ((sector_pos % groups_per_track) % groups_per_sector);

	if (numbering_sector == 1) {
		// トラックごとに連番の場合
		sector_num += (side_num * sectors_per_track);
	}

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBase();

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	if (div_nums) *div_nums = groups_per_sector;
}

/// 論理セクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)からトラック、セクタの各番号を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @param [in] sector_pos  論理セクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] sector_num 論理セクタ番号
void DiskBasicTypeHFS::GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num)
{
	int selected_side = basic->GetSelectedSide();
	int groups_per_track = basic->GetGroupsPerTrack();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();

	if (selected_side >= 0) {
		// 1S
		track_num = sector_pos / groups_per_track;
		sector_num = (sector_pos % groups_per_track);
	} else {
		// 2D, 2HD
		track_num = sector_pos / (groups_per_track * sides_per_disk);
		sector_num = (sector_pos % (groups_per_track * sides_per_disk));
	}

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBase();
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,最初のセクタを0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  物理セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)
int  DiskBasicTypeHFS::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
	int groups_per_track = basic->GetGroupsPerTrack();
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * groups_per_track;
		sector_pos += (sector_num * div_nums + div_num);
	} else {
		// 2D, 2HD
		sector_pos = track_num * sides_per_disk * groups_per_track;
		if (numbering_sector == 1) {
			sector_pos += (sector_num * div_nums + div_num);
		} else {
			sector_pos += (side_num % sides_per_disk) * groups_per_track;
			sector_pos += (sector_num * div_nums + div_num);
		}
	}

	return sector_pos;
}

/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,最初のセクタを0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] sector_num 論理セクタ番号
/// @return 論理セクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)
int  DiskBasicTypeHFS::GetSectorPosFromNumS(int track_num, int sector_num)
{
	int selected_side = basic->GetSelectedSide();
	int groups_per_track = basic->GetGroupsPerTrack();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * groups_per_track + sector_num;
	} else {
		// 2D, 2HD
		sector_pos = track_num * groups_per_track * sides_per_disk + sector_num;
	}
	return sector_pos;
}
#endif

/// ルートディレクトリか
bool DiskBasicTypeHFS::IsRootDirectory(wxUint32 group_num)
{
	return ((group_num & 0x7fffffff) <= 1);
}

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeHFS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
#if 0
	// VTOCエリア
	DiskImageSector *sector = basic->GetManagedSector(0);
	if (!sector) return false;
	apledos_vtoc_t *vtoc = (apledos_vtoc_t *)sector->GetSectorBuffer();
	if (!vtoc) return false;

	apledos_vtoc = vtoc;

	sector->Fill(0);

	// VTOCエリアを設定

	vtoc->dir_start_track = (wxUint8)basic->GetManagedTrackNumber();
	vtoc->dir_start_sector = (wxUint8)basic->GetDirStartSector();

	vtoc->release_number = 3;

	vtoc->chain_size = APLEDOS_TRACK_LIST_MAX;

	vtoc->tracks_per_disk = (wxUint8)basic->GetTracksPerSide();
	vtoc->sectors_per_track = (wxUint8)basic->GetSectorsPerTrackOnBasic();

	wxUint32 map = 0;
	for(int sec=0; sec<basic->GetSectorsPerTrackOnBasic(); sec++) {
		map |= (1 << sec);
	}
	SetTrackMapMask(map);

	for(int trk=3; trk<basic->GetTracksPerSideOnBasic(); trk++) {
		SetTrackMap(trk, map);
	}

	// volume number
	SetIdentifiedData(data);

	// DIRエリア
	int dir_sta_lsec = basic->GetDirStartSector();
	int dir_end_lsec = 2;
	for(int lsec_pos = dir_sta_lsec; lsec_pos >= dir_end_lsec; lsec_pos--) {
		sector = basic->GetManagedSector(lsec_pos);
		if (!sector) {
			continue;
		}
		sector->Fill(0);
		apledos_ptr_t *p = (apledos_ptr_t *)sector->GetSectorBuffer();
		p->next_track = vtoc->dir_start_track;
		p->next_sector = lsec_pos - 1;
	}
#endif
	return false;
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
int DiskBasicTypeHFS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (remain > 0) istream.Read((void *)buffer, remain);
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

	return len;
}

/// FAT領域を削除する
void DiskBasicTypeHFS::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}

/// ファイル削除後の処理
bool DiskBasicTypeHFS::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// チェインセクタを未使用にする
	wxUint32 gnum = item->GetStartGroup(0);
	while (gnum != 0) {
		SetGroupNumber(gnum, 0);
		DiskImageSector *sector = basic->GetSectorFromGroup(gnum);
		if (!sector) break;
//		apledos_chain_t *p = (apledos_chain_t *)sector->GetSectorBuffer();
//		if (!p) break;
//		gnum = GetSectorPosFromNumS(p->next.next_track + basic->GetTrackNumberBaseOnDisk(),  p->next.next_sector + basic->GetSectorNumberBase());
	}

//	DiskBasicDirItemAppleDOS *ditem = (DiskBasicDirItemAppleDOS *)item;
	// ディレクトリの最初に削除コード(0xff)を入れる
//	ditem->SetStartTrack(basic->GetDeleteCode());

	return true;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeHFS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
#if 0
	// volume number
	data.SetVolumeNumber(apledos_vtoc->volume_number);
#endif
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeHFS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
#if 0
	const DiskBasicFormat *fmt = basic->GetFormatType();

	// volume number
	if (fmt->HasVolumeNumber()) {
		apledos_vtoc->volume_number = (wxUint8)data.GetVolumeNumber();
		if (apledos_vtoc->volume_number == 0) {
			apledos_vtoc->volume_number = 0xfe;	// default
		}
	}
#endif
}
