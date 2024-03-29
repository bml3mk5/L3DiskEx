/// @file basictype_prodos.cpp
///
/// @brief disk basic type for Apple ProDOS 8
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_prodos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "basicdiritem_prodos.h"
#include "../utils.h"
#include "../logging.h"


//////////////////////////////////////////////////////////////////////
//
// Apple ProDOS ビットマップ
//
ProDOSBitmap::ProDOSBitmap()
	: DiskBasicBitMLMap()
{
	m_group_num = (wxUint32)-1;
}

/// ポインタをセット
void ProDOSBitmap::AddBitmap(DiskImageSector *sector)
{
	DiskBasicBitMLMap::AddBuffer(
		sector->GetSectorBuffer(),
		sector->GetSectorSize()
	);
}

/// 指定位置のビットを変更する
/// @param[in] group_num 位置
/// @param[in] use セットする場合true
void ProDOSBitmap::Modify(wxUint32 group_num, bool use)
{
	DiskBasicBitMLMap::Modify(group_num, !use);	// 逆転
}

/// 指定位置が空いているか
/// @param[in] group_num 位置
/// @return 空いている場合 true
bool ProDOSBitmap::IsFree(wxUint32 group_num) const
{
	return DiskBasicBitMLMap::IsSet(group_num);	// 逆転
}

//////////////////////////////////////////////////////////////////////
//
// Apple ProDOS 8 / 16 の処理
//
DiskBasicTypeProDOS::DiskBasicTypeProDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	volume = NULL;

	/// ProDOS 8のときは、セクタ→ブロックマップを作成
	if (basic->GetTracksPerSideOnBasic() <= 40) {
		// ProDOS 8 
		sector_skew.Create(basic, basic->GetSectorsPerTrackOnBasic());
	}
}

DiskBasicTypeProDOS::~DiskBasicTypeProDOS()
{
}

/// エリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeProDOS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// ビットマップ
	wxUint32 group_num = bitmap.GetMyGroupNumber();
	int st_pos = GetStartSectorFromGroup(group_num);
	int ed_pos = GetEndSectorFromGroup(group_num, 0, st_pos, 0, 0);
	for(int sec = st_pos; sec <= ed_pos; sec++) {
		DiskImageSector *sector = basic->GetSectorFromSectorPos(sec);
		if (!sector) {
			return -1.0;
		}
		bitmap.AddBitmap(sector);
	}
	basic->SetSectorsPerFat((int)bitmap.Count());

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeProDOS::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 0;

	double valid_ratio = 1.0;

	// 可変数セクタなのでトラックごとのセクタ数を集計
	sector_map.Create(basic);

	// セクタ数の合計
	int calc_total_blocks = sector_map.GetTotalSectors() / basic->GetSectorsPerGroup();
	basic->SetFatEndGroup(calc_total_blocks - 1);

	// ボリュームディレクトリ
	DiskImageSector *sector = basic->GetSectorFromGroup(basic->GetDirStartSector() / basic->GetSectorsPerGroup());
	if (!sector) {
		return -1.0;
	}
	directory_prodos_t *vol = (directory_prodos_t *)sector->GetSectorBuffer(4);
	if (vol->v.entry_len != (int)sizeof(directory_prodos_t)) {
		return -1.0;
	}

//	// ブロック内のファイルエントリ数
//	int entries = vol->v.entries_per_block;
//	// ファイルエントリ数
//	int file_count = wxUINT16_SWAP_ON_BE(vol->v.file_count);
//	// ボリュームディレクトリで使用できるブロック数
//	int dir_blocks = (file_count + 1) / entries;

//	basic->SetDirEndSector((basic->GetDirStartSector() + dir_blocks - 1) * basic->GetSectorsPerGroup());

	int stor_total_blocks = wxUINT16_SWAP_ON_BE(vol->v.total_blocks);

	if (stor_total_blocks > calc_total_blocks) {
		valid_ratio -= 0.5;
	}

	basic->SetFatEndGroup(stor_total_blocks - 1);

	bitmap.SetMyGroupNumber(wxUINT16_SWAP_ON_BE(vol->v.bitmap_pointer));

	volume = vol;

	return valid_ratio;
}


/// Allocation Mapの開始位置を得る（ダイアログ用）
void DiskBasicTypeProDOS::GetStartNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	wxUint32 group_num = bitmap.GetMyGroupNumber();
	int st_pos = GetStartSectorFromGroup(group_num);
	GetNumFromSectorPos(st_pos, track_num, side_num, sector_num);
}

/// Allocation Mapの終了位置を得る（ダイアログ用）
void DiskBasicTypeProDOS::GetEndNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	wxUint32 group_num = bitmap.GetMyGroupNumber();
	int st_pos = GetStartSectorFromGroup(group_num);
	int ed_pos = GetEndSectorFromGroup(group_num, 0, st_pos, 0, 0);
	GetNumFromSectorPos(ed_pos, track_num, side_num, sector_num);
}

/// タイトル名（ダイアログ用）
wxString DiskBasicTypeProDOS::GetTitleForFat() const
{
	return _("Allocation Map");
}

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicTypeProDOS::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool sts = DiskBasicType::AssignRootDirectory(start_sector, end_sector, group_items, dir_item);

	// ボリュームヘッダの内容をコピーする
	DiskBasicGroupItem *gitem = &group_items.Item(0);
	DiskImageSector *sector = basic->GetSector(gitem->track, gitem->side, gitem->sector_start);
	directory_t *vol = (directory_t *)sector->GetSectorBuffer(4);
	dir_item->CopyData(vol);
	// ディレクトリ属性にしておく
	dir_item->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_DIRECTORY_MASK);
	// ブロック番号を設定
	dir_item->SetStartGroup(0, (wxUint32)(basic->GetDirStartSector() / basic->GetSectorsPerGroup()));

	return sts;
}

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
bool DiskBasicTypeProDOS::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	bool valid = true;

	group_items.Empty();

	// ディレクトリのチェインをたどる
	size_t dir_size = 0;
	int limit = basic->GetDirEndSector() - basic->GetDirStartSector() + 1;
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;

	// 開始セクタ
	int sector_pos = basic->GetDirStartSector();
//	voldir.Empty();

	while(valid && limit >= 0) {
		prodos_dir_ptr_t next;
		next.next_block = 0;

		wxUint32 group_num = (wxUint32)(sector_pos / basic->GetSectorsPerGroup());
//		voldir.Add((int)group_num);

		for(int ss = 0; ss < basic->GetSectorsPerGroup(); ss++) {
			DiskImageSector *sector = basic->GetSectorFromSectorPos(sector_pos, trk_num, sid_num);
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
			if (ss == 0) {
				// 次のブロックへのポインタを保持
				memcpy(&next, buffer, sizeof(prodos_dir_ptr_t));
			}

			group_items.Add(group_num, 0, trk_num, sid_num, sec_num, sec_num);

			dir_size += sector->GetSectorSize();
			sector_pos++;
		}

		// 次のセクタなし
		if (next.next_block == 0) {
			break;
		}

		sector_pos = (int)(wxUINT16_SWAP_ON_BE(next.next_block) * basic->GetSectorsPerGroup());

		limit--;
	}
	group_items.SetSize(dir_size);

	if (limit < 0) {
		valid = false;
	}

	return valid;
}

/// ディレクトリエリアのサイズに達したらアサイン終了するか
/// @param[in,out] pos         ディレクトリの位置
/// @param[in,out] size        ディレクトリのセクタサイズ
/// @param[in,out] size_remain ディレクトリの残りサイズ
/// @retval  0 : 終了しない
/// @retval -1 : このブロックでのアサイン終了 次のブロックへ
int DiskBasicTypeProDOS::FinishAssigningDirectory(int &pos, int &size, int &size_remain) const
{
	// サイズに達したら以降は未使用とする
	int blk_size = basic->GetSectorSize() * basic->GetSectorsPerGroup();
	return ((size_remain % blk_size) < (int)sizeof(directory_prodos_t) ? -1 : 0);
}

/// セクタをディレクトリとして初期化
/// @param [in]     group_items 確保したセクタリスト
/// @param [in,out] file_size   サイズ ディレクトリを拡張した時は既存サイズに加算
/// @param [in,out] size_remain 残りサイズ
/// @param [in,out] errinfo     エラー情報
/// @return 0:正常 <0:エラー 
int DiskBasicTypeProDOS::InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &size_remain, DiskBasicError &errinfo)
{
	file_size = (int)group_items.Count() * basic->GetSectorSize();
	size_remain = 0;

	return 0;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeProDOS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
void DiskBasicTypeProDOS::CalcDiskFreeSize(bool wrote)
{
//	wxUint32 fsize = 0;
//	wxUint32 grps = 0;

	fat_availability.Empty();

	// BITMAP table
	for(wxUint32 grp = 0; grp <= basic->GetFatEndGroup(); grp++) {
		if (grp <= 2) {
			fat_availability.Add(FAT_AVAIL_SYSTEM, 0, 0);
		} else if (grp == bitmap.GetMyGroupNumber()) {
			fat_availability.Add(FAT_AVAIL_SYSTEM, 0, 0);
		} else if (bitmap.IsFree(grp)) {
			fat_availability.Add(FAT_AVAIL_FREE, basic->GetSectorSize() * basic->GetSectorsPerGroup(), 1);
		} else {
			fat_availability.Add(FAT_AVAIL_USED, 0, 0);
		}
	}
	// Volume directory
	DiskBasicDirItem *root = dir->GetRootItem();
	if (root) {
		const DiskBasicGroups *root_groups = &root->GetGroups();
		for(size_t i=0; i<root_groups->Count(); i++) {
			fat_availability.Set(root_groups->Item(i).group, FAT_AVAIL_SYSTEM);
		}
	}
//	free_disk_size = (int)fsize;
//	free_groups = (int)grps;
}

/// グループ番号を使用済みにする
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeProDOS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	bitmap.Modify(num, val != 0);
}

/// グループ番号を得る
wxUint32 DiskBasicTypeProDOS::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeProDOS::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeProDOS::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeProDOS::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	for(wxUint32 grp = 3; grp <= basic->GetFatEndGroup(); grp++) {
		if (bitmap.IsFree(grp)) {
			new_num = grp;
			break;
		}
	}

	return new_num;
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeProDOS::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	// 次の空き位置候補
	return GetEmptyGroupNumber();
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeProDOS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	// チェインセクタをクリア
	nitem->ClearChainSector();

	return true;
}

/// チェインセクタを確保する
/// @param [in]  idx          チェイン番号
/// @param [in]  item         ディレクトリアイテム
/// @return 確保したグループ番号 / エラー時 INVALID_GROUP_NUMBER
wxUint32 DiskBasicTypeProDOS::AllocChainSector(int idx, DiskBasicDirItem *item)
{
	wxUint32 gnum = GetEmptyGroupNumber();
	if (gnum == INVALID_GROUP_NUMBER) {
		return INVALID_GROUP_NUMBER;
	}
	// セクタ
	int st_pos = GetStartSectorFromGroup(gnum);
	int ed_pos = GetEndSectorFromGroup(gnum, 0, st_pos, 0, 0);
	for(int sec = st_pos; sec <= ed_pos; sec++) {
		DiskImageSector *sector = basic->GetSectorFromSectorPos(sec);
		if (!sector) {
			return INVALID_GROUP_NUMBER;
		}
		sector->Fill(0);
	}

	// チェイン情報にセクタをセット
	item->SetChainSector(gnum, st_pos, NULL);

	// 開始グループを設定
	if (idx == 0) {
		item->SetStartGroup(0, gnum, 1);
	}
	// セクタを予約
	SetGroupNumber(gnum, 1);

	return gnum;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeProDOS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
//	myLog.SetDebug("DiskBasicTypeProDOS::AllocateGroups {");

//	int file_size = 0;
	int groups = 0;

	int rc = 0;
	int sec_size = basic->GetSectorSize();
	int block_size = sec_size * basic->GetSectorsPerGroup();
	int remain = data_size;
	int limit = basic->GetFatEndGroup() + 1;
	int chain_idx = 0;

	DiskBasicFileType attr = item->GetFileAttr();
	int stype = (attr.GetOrigin() >> 16) & 0xff;
	if (stype == FILETYPE_MASK_PRODOS_SUBDIR) {
		// サブディレクトリ
		while(remain > 0 && limit >= 0) {
			// 空きをさがす
			wxUint32 group_num = GetEmptyGroupNumber();
			if (group_num == INVALID_GROUP_NUMBER) {
				// 空きなし
				rc = groups > 0 ? -2 : -1;
				return rc;
			}

			// 使用済みにする
			basic->GetNumsFromGroup(group_num, 0, sec_size, remain, group_items);
			SetGroupNumber(group_num, 1);

			if (chain_idx == 0 && flags != ALLOCATE_GROUPS_APPEND) {
				item->SetStartGroup(0, group_num, 1);
			}
			chain_idx++;

//			file_size += block_size;
			groups++;
			remain -= block_size;
			limit--;
		}
		if (rc == 0 && flags == ALLOCATE_GROUPS_APPEND) {
			// 追加のときはチェインをつなぐ
			if (group_items.Count() > 0) {
				rc = ChainDirectoryGroups(item, group_items);
			}
		}
	} else if (stype == FILETYPE_MASK_PRODOS_SAPLING) {
		// 131Kバイト未満はインデックス１つ
		if (AllocChainSector(0, item) == INVALID_GROUP_NUMBER) {
			return -1;
		}
		while(remain > 0 && limit >= 0) {
			// 空きをさがす
			wxUint32 group_num = GetEmptyGroupNumber();
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

//			file_size += block_size;
			groups++;
			remain -= block_size;
			limit--;
		}
	} else if (stype == FILETYPE_MASK_PRODOS_TREE){
		// 131Kバイト以上 ツリー
		int chain_pidx = 0;
		chain_idx = 256;
		// チェインセクタを確保
		if (AllocChainSector(0, item) == INVALID_GROUP_NUMBER) {
			return -1;
		}
		chain_pidx++;
		while(remain > 0 && limit >= 0) {
			// チェインセクタを確保
			if ((chain_idx % 256) == 0) {
				wxUint32 cgnum = AllocChainSector(chain_pidx, item);
				if (cgnum == INVALID_GROUP_NUMBER) {
					return -1;
				}
				// ルートチェインセクタと結びつける
				item->AddChainGroupNumber(chain_pidx, cgnum);
				chain_pidx++;
			}
			// 空きをさがす
			wxUint32 group_num = GetEmptyGroupNumber();
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

//			file_size += block_size;
			groups++;
			remain -= block_size;
			limit--;
		}
	} else {
		//　512バイト以下
		while(remain > 0 && limit >= 0) {
			// 空きをさがす
			wxUint32 group_num = GetEmptyGroupNumber();
			if (group_num == INVALID_GROUP_NUMBER) {
				// 空きなし
				rc = groups > 0 ? -2 : -1;
				return rc;
			}

			// 使用済みにする
			basic->GetNumsFromGroup(group_num, 0, sec_size, remain, group_items);
			SetGroupNumber(group_num, 1);

			if (chain_idx == 0) {
				item->SetStartGroup(0, group_num, 1);
			}
			chain_idx++;

//			file_size += block_size;
			groups++;
			remain -= block_size;
			limit--;
		}
	}

	if (limit < 0) {
		// 無限ループ？
		rc = -2;
	}

//	myLog.SetDebug("rc: %d }", rc);
	return rc;
}

/// グループをつなげる
/// @return 0 正常
int DiskBasicTypeProDOS::ChainDirectoryGroups(DiskBasicDirItem *item, DiskBasicGroups &group_items)
{
	DiskBasicGroups orig_group_items;
	item->GetAllGroups(orig_group_items);
	orig_group_items.Add(group_items);
	group_items = orig_group_items;

	// ディレクトリチェインを再作成
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	wxUint32 prev_group_num = INVALID_GROUP_NUMBER;
	prodos_dir_ptr_t *prev = NULL;
	for(size_t i=0; i<group_items.Count(); i++) {
		DiskBasicGroupItem *item = &group_items.Item(i);
		if (item->group != group_num) {
			group_num = item->group;
			DiskImageSector *sector = basic->GetSectorFromGroup(group_num);
			prodos_dir_ptr_t *curr = (prodos_dir_ptr_t *)sector->GetSectorBuffer();

			curr->prev_block = prev_group_num != INVALID_GROUP_NUMBER ? prev_group_num : 0;
			curr->prev_block = wxUINT16_SWAP_ON_BE(curr->prev_block);

			if (prev) {
				prev->next_block = group_num;
				prev->next_block = wxUINT16_SWAP_ON_BE(prev->next_block);
			}

			prev = curr;
			prev_group_num = group_num;
		}
	}

	return 0;
}

#if 0
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
int DiskBasicTypeProDOS::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	int size = sector_size < remain_size ? sector_size : remain_size;

	if (ostream) {
		// 書き出し
		temp.SetData(sector_buffer, size, basic->IsDataInverted());
		ostream->Write(temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read(temp.GetData(), temp.GetSize());
		temp.InvertData(basic->IsDataInverted());

		if (memcmp(temp.GetData(), sector_buffer, size) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}
#endif

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeProDOS::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	return remain_size;
}

/// グループ番号からセクタ番号を得る
/// @param [in] group_num グループ番号
int DiskBasicTypeProDOS::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num * basic->GetSectorsPerGroup();
}

/// グループ番号から最終セクタ番号を得る
/// @param [in] group_num    グループ番号
/// @param [in] next_group   未使用
/// @param [in] sector_start 未使用
/// @param [in] sector_size  未使用
/// @param [in] remain_size  未使用
int DiskBasicTypeProDOS::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return (group_num + 1) * basic->GetSectorsPerGroup() - 1;
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num 物理セクタ番号
/// @param [out] div_num    分割番号
/// @param [out] div_nums   分割数
void DiskBasicTypeProDOS::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
//	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sectors_per_track = sides_per_disk;

	// セクタ位置がどのトラックにあるか
	sector_map.GetNumFromSectorPos(sector_pos, track_num, sector_num, sectors_per_track);

//	if (selected_side >= 0) {
//		// 1S
//		track_num = sector_pos / sectors_per_track;
//		side_num = selected_side;
//	} else {
//		// 2D, 2HD
//		track_num = sector_pos / sectors_per_track / sides_per_disk;
//		side_num = (sector_pos / sectors_per_track) % sides_per_disk;
//	}
//	sector_num = (sector_pos % sectors_per_track);

	// サイド番号
	side_num = sector_num * sides_per_disk / sectors_per_track;

	// 連番でない場合
	if (numbering_sector != 1) {
		sector_num = sector_num % (sectors_per_track / sides_per_disk);
	}

	// マッピング
	sector_num = sector_skew.ToPhysical(sector_num);

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBase();

	if (div_num)  *div_num = 0;
	if (div_nums) *div_nums = 1;
}

/// 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] sector_num 論理セクタ番号
void DiskBasicTypeProDOS::GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num)
{
	int sectors_per_track = 1;

	sector_map.GetNumFromSectorPos(sector_pos, track_num, sector_num, sectors_per_track);

	// マッピング
	sector_num = sector_skew.ToPhysical(sector_num);

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBase();
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  物理セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeProDOS::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
//	int selected_side = basic->GetSelectedSide();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = 1;
//	int sector_pos;

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	// マッピング
	sector_num = sector_skew.ToLogical(sector_num);

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	int sector_pos = sector_map.GetSectorPosFromNum(track_num, sector_num, sectors_per_track);

	// 連番でない場合
	if (numbering_sector != 1) {
		sector_pos += side_num * sectors_per_track / sides_per_disk;
	}

//	if (selected_side >= 0) {
//		// 1S
//		sector_pos = track_num * sectors_per_track + sector_num;
//	} else {
//		// 2D, 2HD
//		sector_pos = track_num * sectors_per_track * sides_per_disk;
//		sector_pos += (side_num % sides_per_disk) * sectors_per_track;
//		sector_pos += sector_num;
//	}
	return sector_pos;
}

/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] sector_num 論理セクタ番号
/// @return 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeProDOS::GetSectorPosFromNumS(int track_num, int sector_num)
{
//	int selected_side = basic->GetSelectedSide();
//	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
//	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	int sectors_per_track = 1;

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	// マッピング
	sector_num = sector_skew.ToLogical(sector_num);

	sector_pos = sector_map.GetSectorPosFromNum(track_num, sector_num, sectors_per_track);

	return sector_pos;
}

/// ルートディレクトリか
bool DiskBasicTypeProDOS::IsRootDirectory(wxUint32 group_num)
{
	return false;
}

/// サブディレクトリを作成する前にディレクトリ名を編集する
bool DiskBasicTypeProDOS::RenameOnMakingDirectory(wxString &dir_name)
{
	// 名前が空は作成不可
	if (dir_name.IsEmpty()) {
		return false;
	}
	return true;
}

/// サブディレクトリを作成した後の個別処理
/// @param[in] item        アイテム
/// @param[in] group_items グループ番号
/// @param[in] parent_item 親ディレクトリ
void DiskBasicTypeProDOS::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	if (group_items.Count() <= 0) return;

	int block_size = basic->GetSectorSize() * basic->GetSectorsPerGroup();

	DiskBasicDirItemProDOS *ditem = (DiskBasicDirItemProDOS *)item;
	DiskBasicDirItemProDOS *parent_ditem = (DiskBasicDirItemProDOS *)parent_item;

	// ディレクトリの最初のブロックをセット
	item->SetParentGroup(parent_item->GetStartGroup(0));
	// バージョンはヘッダと合わせる
	ditem->SetVersion(parent_ditem->GetVersion());

	// サブボリュームヘッダのエントリを作成する

	DiskBasicGroupItem *gitem = &group_items.Item(0);

	DiskImageSector *sector = basic->GetSector(gitem->track, gitem->side, gitem->sector_start);

	wxUint8 *buf = sector->GetSectorBuffer(4);
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, 0, buf);
	DiskBasicDirItemProDOS *newditem = (DiskBasicDirItemProDOS *)newitem;

	newitem->CopyData(item->GetData());
	newitem->SetFileAttr(FORMAT_TYPE_PRODOS, 0, FILETYPE_MASK_PRODOS_SUBVOL << 16 | 0x75 << 8 | (FILETYPE_MASK_PRODOS_ACCESS_ALL & ~FILETYPE_MASK_PRODOS_CHANGE));
	newitem->SetStartGroup(0, 0);
	newitem->SetFileSize(0);
	// バージョン
	newditem->SetVersion(parent_ditem->GetVersion());

	directory_prodos_t *vol = (directory_prodos_t *)buf;

	// エントリのサイズ
	vol->sv.entry_len = (wxUint8)sizeof(directory_prodos_t);
	// ブロック内のファイルエントリ数
	vol->sv.entries_per_block = (wxUint8)((block_size - 4) / (int)sizeof(directory_prodos_t));
	// ファイルエントリ数
	vol->sv.file_count = 0;

	int parent_start_block = (int)parent_item->GetStartGroup(0);
	int item_number = item->GetNumber();
	int parent_pointer = (item_number / (int)vol->sv.entries_per_block) + parent_start_block;
	int parent_entry = item_number % (int)vol->sv.entries_per_block;

	// 親のブロック番号
	vol->sv.parent_pointer = wxUINT16_SWAP_ON_BE(parent_pointer);
	// 親エントリ
	vol->sv.parent_entry = (wxUint8)parent_entry;

	// 親エントリのサイズ
	vol->sv.parent_entry_len = (wxUint8)sizeof(directory_prodos_t);

	delete newitem;
}

#if 0
/// フォーマット時セクタデータを指定コードで埋める
/// @param[in] track  トラック
/// @param[in] sector セクタ
void DiskBasicTypeProDOS::FillSector(DiskImageTrack *track, DiskImageSector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}
#endif

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeProDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskImageSector *sector;

	// ボリュームディレクトリをクリア
	int st_pos = basic->GetDirStartSector();
	int ed_pos = (basic->GetDirEndSector() / basic->GetSectorsPerGroup() + 1) * basic->GetSectorsPerGroup() - 1;
	for(int sec = st_pos; sec <= ed_pos; sec++) {
		sector = basic->GetSectorFromSectorPos(sec);
		if (!sector) {
			// Why?
			return false;
		}
		sector->Fill(0);
	}

	// ボリュームディレクトリのチェインを作成
	int st_blk = st_pos / basic->GetSectorsPerGroup();
	int ed_blk = ed_pos / basic->GetSectorsPerGroup();
//	voldir.Empty();
	for(int blk = st_blk; blk <= ed_blk; blk++) {
		sector = basic->GetSectorFromGroup(blk);
		prodos_dir_ptr_t *p = (prodos_dir_ptr_t *)sector->GetSectorBuffer();
		if (!p) {
			// Why?
			return false;
		}
		wxUint16 next_block = (blk != ed_blk ? blk + 1 : 0);
		wxUint16 prev_block = (blk != st_blk ? blk - 1 : 0);
		p->next_block = wxUINT16_SWAP_ON_BE(next_block);
		p->prev_block = wxUINT16_SWAP_ON_BE(prev_block);
//		voldir.Add(blk);
	}

	int block_size = basic->GetSectorSize() * basic->GetSectorsPerGroup();

	// ボリュームヘッダを作成
	sector = basic->GetSectorFromGroup(st_blk);
	directory_prodos_t *vol = (directory_prodos_t *)sector->GetSectorBuffer(4);
	// 属性
	vol->stype_and_nlen = (FILETYPE_MASK_PRODOS_VOLUME << 4);
	// 日時
	TM tm;
	tm.Now();
	DiskBasicDirItemProDOS::ConvDateFromTm(tm, vol->cdate);
	DiskBasicDirItemProDOS::ConvTimeFromTm(tm, vol->ctime);
	// アクセス
	vol->access = FILETYPE_MASK_PRODOS_ACCESS_ALL & ~FILETYPE_MASK_PRODOS_CHANGE;
	// エントリのサイズ
	vol->v.entry_len = (wxUint8)sizeof(directory_prodos_t);
	// ブロック内のファイルエントリ数
	vol->v.entries_per_block = (wxUint8)((block_size - 4) / (int)sizeof(directory_prodos_t));
	// ファイルエントリ数
	vol->v.file_count = 0;
	// ビットマップポインタ
	wxUint16 bitmap_pointer = 6;
	vol->v.bitmap_pointer = wxUINT16_SWAP_ON_BE(bitmap_pointer);
	// トータルブロック数
	wxUint16 total_blocks = (wxUint16)(basic->GetSidesPerDisk() * basic->GetTracksPerSideOnBasic() * basic->GetSectorsPerTrackOnBasic() / basic->GetSectorsPerGroup());
	vol->v.total_blocks = wxUINT16_SWAP_ON_BE(total_blocks);

	volume = vol;

	basic->SetFatEndGroup(total_blocks - 1);

	// ビットマップポインタを設定
	bitmap.Clear();
	st_pos = GetStartSectorFromGroup(bitmap_pointer);
	ed_pos = GetEndSectorFromGroup(bitmap_pointer, 0, st_pos, 0, 0);
	for(int sec = st_pos; sec <= ed_pos; sec++) {
		sector = basic->GetSectorFromSectorPos(sec);
		if (!sector) {
			// Why?
			return false;
		}
		sector->Fill(0);
		bitmap.AddBitmap(sector);
	}
	bitmap.SetMyGroupNumber(bitmap_pointer);
	for(int blk = ed_blk + 1; blk < (int)total_blocks; blk++) {
		bitmap.Modify(blk, false);
	}
	bitmap.Modify(bitmap_pointer, true);

	// volume name
	SetIdentifiedData(data);

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
int DiskBasicTypeProDOS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
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

/// データの書き込み終了後の処理
/// @param [in]	 item			ディレクトリアイテム
void DiskBasicTypeProDOS::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	DiskBasicDirItemProDOS *ditem = (DiskBasicDirItemProDOS *)item;

	// ディレクトリのヘッダにあるファイル数を＋１する
	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) {
		// Why?
		return;
	}
	DiskBasicDirItems *children = parent->GetChildren();
	if (!children) {
		// Why?
		return;
	}
	DiskBasicDirItemProDOS *vol = (DiskBasicDirItemProDOS *)children->Item(0);
	if (!vol) {
		// Why?
		return;
	}
	vol->IncreaseFileCount();
	// ディレクトリの最初のブロックをセット
	item->SetParentGroup(parent->GetStartGroup(0));
	// バージョンはヘッダと合わせる
	ditem->SetVersion(vol->GetVersion());
}

/// FAT領域を削除する
void DiskBasicTypeProDOS::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}

/// ファイル削除後の処理
bool DiskBasicTypeProDOS::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// チェインセクタを未使用にする
	item->ClearChainSector();

	// ディレクトリのヘッダにあるファイル数を－１する
	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) {
		// Why?
		return true;
	}
	DiskBasicDirItems *children = parent->GetChildren();
	if (!children) {
		// Why?
		return true;
	}
	DiskBasicDirItemProDOS *vol = (DiskBasicDirItemProDOS *)children->Item(0);
	if (!vol) {
		// Why?
		return true;
	}
	vol->DecreaseFileCount();

	return true;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeProDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// volume name
	if (volume) {
		size_t len = (volume->stype_and_nlen & 0xf);
		wxString volname(volume->name, len);
		data.SetVolumeName(volname);
		data.SetVolumeNameMaxLength(sizeof(volume->name));
	}
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeProDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	const DiskBasicFormat *fmt = basic->GetFormatType();

	// volume name
	if (volume && fmt->HasVolumeName()) {
		wxCharBuffer volname = data.GetVolumeName().To8BitData();
		size_t len = sizeof(volume->name);
		if (len > volname.length()) len = volname.length();
		memcpy(volume->name, volname.data(), len);

		volume->stype_and_nlen = (len & 0xf) | (volume->stype_and_nlen & 0xf0);
	}
}
