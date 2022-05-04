/// @file basictype_amiga.cpp
///
/// @brief disk basic type for Amiga DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_amiga.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "basicdiritem_amiga.h"
#include "../utils.h"
#include "../logging.h"


//////////////////////////////////////////////////////////////////////
//
// AMIGA ビットマップ １つ
//
AmigaOneBitmap::AmigaOneBitmap()
{
	m_block_num = 0;
	m_block_size = 0;
	m_map = NULL;
}

/// @param[in] block_num  ブロック番号
/// @param[in] map        マップのあるバッファ
/// @param[in] block_size バッファサイズ
AmigaOneBitmap::AmigaOneBitmap(wxUint32 block_num, void *map, int block_size)
{
	m_block_num = block_num;
	m_block_size = block_size;
	m_map = (amiga_bitmap_block_t *)map;
}

AmigaOneBitmap::~AmigaOneBitmap()
{
}

/// 指定位置のビットを変更する
/// @param[in] block_num  ブロック番号(2..)
/// @param[in] use セットする場合true
void AmigaOneBitmap::Modify(wxUint32 block_num, bool use)
{
	int pos = block_num >> 5;
	int bit = block_num & 0x1f;
	wxUint32 dat = (1 << bit);
	dat = wxUINT32_SWAP_ON_LE(dat);
	if (use) {
		m_map->map[pos] &= ~dat;
	} else {
		m_map->map[pos] |= dat;
	}
}

/// 指定位置が空いているか
/// @param[in] block_num  ブロック番号(2..)
/// @return 空いている場合 true
bool AmigaOneBitmap::IsFree(wxUint32 block_num) const
{
	int pos = block_num >> 5;
	int bit = block_num & 0x1f;
	wxUint32 dat = (1 << bit);
	dat = wxUINT32_SWAP_ON_LE(dat);
	return ((m_map->map[pos] & dat) != 0);
}

/// 指定ブロックまですべて未使用にする
/// @param[in] block_num 最終ブロック番号
void AmigaOneBitmap::FreeAll(wxUint32 block_num)
{
	if (block_num >= GetBlockNums()) {
		block_num = GetBlockNums() - 1;
	}
	int pos = block_num >> 5;
	int bit = block_num & 0x1f;
	for(int p=0; p<pos; p++) {
		m_map->map[p] = 0xffffffff;
	}
	wxUint32 dat = ((1 << (bit + 1)) - 1);
	dat = wxUINT32_SWAP_ON_LE(dat);
	m_map->map[pos] = dat;
}

/// ブロック数を返す
wxUint32 AmigaOneBitmap::GetBlockNums() const
{
	return (wxUint32)((m_block_size - 4) << 3);
}

/// チェックサムの更新
void AmigaOneBitmap::UpdateCheckSum()
{
	DiskBasicDirItemAmiga::CalcCheckSum(m_map, m_block_size, m_map->check_sum);
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfAmigaBitmap);

//////////////////////////////////////////////////////////////////////
//
// AMIGA ビットマップ
//
AmigaBitmap::AmigaBitmap()
	: ArrayOfAmigaBitmap()
{
}

AmigaBitmap::~AmigaBitmap()
{
}

/// ビットマップを追加
/// @param[in] block_num  ブロック番号
/// @param[in] map        マップのあるバッファ
/// @param[in] block_size バッファサイズ
void AmigaBitmap::AddBitmap(wxUint32 block_num, void *map, int block_size)
{
	Add(AmigaOneBitmap(block_num, map, block_size));
}

/// 指定位置のビットを変更する
/// @param[in] block_num  ブロック番号(2..)
/// @param[in] use セットする場合true
void AmigaBitmap::Modify(wxUint32 block_num, bool use)
{
	if (block_num < 2) return;

	block_num -= 2;
	for(size_t i=0; i<Count(); i++) {
		AmigaOneBitmap *item = &Item(i);
		if (block_num < item->GetBlockNums()) {
			item->Modify(block_num, use);
			break;
		}
		block_num -= item->GetBlockNums();
	}
}

/// 指定位置が空いているか
/// @param[in] block_num  ブロック番号(2..)
/// @return 空いている場合 true
bool AmigaBitmap::IsFree(wxUint32 block_num) const
{
	if (block_num < 2) return false;

	block_num -= 2;
	for(size_t i=0; i<Count(); i++) {
		AmigaOneBitmap *item = &Item(i);
		if (block_num < item->GetBlockNums()) {
			return item->IsFree(block_num);
		}
		block_num -= item->GetBlockNums();
	}
	return false;
}

/// 指定ブロックまですべて未使用にする
/// @param[in] block_num 最終ブロック番号
void AmigaBitmap::FreeAll(wxUint32 block_num)
{
	if (block_num < 2) return;

	block_num -= 2;
	for(size_t i=0; i<Count(); i++) {
		AmigaOneBitmap *item = &Item(i);
		if (block_num < item->GetBlockNums()) {
			item->FreeAll(block_num);
		} else {
			item->FreeAll((wxUint32)-1);
		}
		block_num -= item->GetBlockNums();
	}
}

/// ブロック数を返す
wxUint32 AmigaBitmap::GetBlockNums() const
{
	wxUint32 block_nums = 0;
	for(size_t i=0; i<Count(); i++) {
		AmigaOneBitmap *item = &Item(i);
		block_nums += item->GetBlockNums();
	}
	return block_nums;
}

/// チェックサムの更新
void AmigaBitmap::UpdateCheckSum()
{
	for(size_t i=0; i<Count(); i++) {
		AmigaOneBitmap *item = &Item(i);
		item->UpdateCheckSum();
	}
}

//////////////////////////////////////////////////////////////////////
//
// Amiga DOS の処理
//
DiskBasicTypeAmiga::DiskBasicTypeAmiga(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	m_root.block_num = 0;
	m_root.pre = NULL;
	m_root.post = NULL;
}

DiskBasicTypeAmiga::~DiskBasicTypeAmiga()
{
}

/// エリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeAmiga::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeAmiga::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 0;

	double valid_ratio = 1.0;

	//
	// boot block
	//
	DiskD88Sector *sector = basic->GetSectorFromGroup(0);
	if (!sector) {
		return -1.0;
	}
	amiga_boot_block_t *bb = (amiga_boot_block_t *)sector->GetSectorBuffer();
	if (!bb) {
		return -1.0;
	}

	// チェック
	if (memcmp(bb->type, "DOS", 3) != 0 && memcmp(bb->type, "KICK", 4) != 0) {
		return -1.0;
	}
	// Fast File System か
	bool disk_is_fast = (bb->type[3] != 'K' && (bb->type[3] & 1) != 0);
	bool param_is_fast = basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM));
	if (disk_is_fast != param_is_fast) {
		return -1.0;
	}
	basic->SetVariousParam(wxT(KEY_INTERNATIONAL), (bool)((bb->type[3] & 6) == 2 || (bb->type[3] & 6) == 4));

	//
	// root block
	//
	m_root.block_num = wxUINT32_SWAP_ON_LE(bb->root_block);
	if (m_root.block_num < 2 || m_root.block_num > basic->GetFatEndGroup()) {
		// ブート領域かディスクをオーバしている
		if (valid_ratio >= 0.0) valid_ratio *= 0.8;
		m_root.block_num = (basic->GetManagedTrackNumber() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic() + basic->GetDirStartSector() - 1);
	}
	int root_track = 0;
	int root_side = 0;
	sector = basic->GetSectorFromGroup(m_root.block_num, root_track, root_side);
	if (!sector) {
		return -1.0;
	}
	m_root.pre = (amiga_block_pre_t *)sector->GetSectorBuffer();
	if (!m_root.pre) {
		return -1.0;
	}
	// hash_tableはセクタサイズ（ブロックサイズ）で可変
	int offset = basic->GetSectorSize() - (int)sizeof(amiga_root_block_post_t);
	if (offset < 0) {
		return -1.0;
	}
	m_root.post = (amiga_block_post_t *)sector->GetSectorBuffer(offset);
	if (!m_root.post) {
		return -1.0;
	}
	basic->SetManagedTrackNumber(root_track);
	basic->SetDirStartSector(sector->GetSectorNumber());

	//
	// ビットマップ
	//
	if (m_root.post->r.bm_flag == (wxUint32)-1) {
		for(int i=0; i<25; i++) {
			wxUint32 num = wxUINT32_SWAP_ON_LE(m_root.post->r.bm_pages[i]);
			if (num < 2) {
				continue;
			}
			sector = basic->GetSectorFromGroup(num);
			if (!sector) {
				// Why?
				valid_ratio = 0.2;
				break;
			}
			m_bitmap.AddBitmap(num, sector->GetSectorBuffer(), sector->GetSectorSize());
		}
	} else {
		valid_ratio = 0.2;
	}
	basic->SetSectorsPerFat((int)m_bitmap.Count());
	basic->SetFatEndGroup(basic->GetSidesPerDiskOnBasic() * basic->GetTracksPerSideOnBasic() * basic->GetSectorsPerTrackOnBasic() - 1);

	//
	// ディレクトリエリア
	//

	// root block の hash table を追跡
	int max_ht = (int)wxUINT32_SWAP_ON_LE(m_root.pre->table_size);
	int calc_max_ht = (basic->GetSectorSize() - (int)sizeof(amiga_block_pre_t) - (int)sizeof(amiga_block_post_t) + 4) / 4;
	if (max_ht > calc_max_ht) {
		valid_ratio = -1.0;
		max_ht = calc_max_ht;
	}

	for(int ht = 0; ht < max_ht; ht++) {
		wxUint32 num = wxUINT32_SWAP_ON_LE(m_root.pre->u.table[ht]);
		if (num > 0 && num < 2) {
			// boot領域にある？
			if (valid_ratio > 0.0) valid_ratio *= 0.5;
		} else if (num > basic->GetFatEndGroup()) {
			// 範囲外
			if (valid_ratio > 0.0) valid_ratio *= 0.5;
		}
	}

	return valid_ratio;
}


/// Allocation Mapの開始位置を得る（ダイアログ用）
/// @param[out] track_num  トラック番号
/// @param[out] side_num   サイド番号
/// @param[out] sector_num セクタ番号
void DiskBasicTypeAmiga::GetStartNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	// ビットマップのあるセクタ
	if (m_bitmap.Count() > 0) {
		const AmigaOneBitmap *item = &m_bitmap.Item(0);
		GetNumFromSectorPos(item->GetBlockNumber(), track_num, side_num, sector_num);
	}
}

/// Allocation Mapの終了位置を得る（ダイアログ用）
/// @param[out] track_num  トラック番号
/// @param[out] side_num   サイド番号
/// @param[out] sector_num セクタ番号
void DiskBasicTypeAmiga::GetEndNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	// ビットマップのあるセクタ
	if (m_bitmap.Count() > 0) {
		const AmigaOneBitmap *item = &m_bitmap.Last();
		GetNumFromSectorPos(item->GetBlockNumber(), track_num, side_num, sector_num);
	}
}

/// タイトル名（ダイアログ用）
wxString DiskBasicTypeAmiga::GetTitleForFat() const
{
	return _("Allocation Map");
}

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
bool DiskBasicTypeAmiga::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	group_items.Empty();

	if (!m_root.pre) {
		return false;
	}
	bool valid = true;

	// root block の hash table をたどる
	int limit = (int)basic->GetFatEndGroup() + 1;

	int max_blks = (int)wxUINT32_SWAP_ON_LE(m_root.pre->table_size);

	valid = DiskBasicDirItemAmiga::GetDirectoryGroups(basic, m_root.pre->u.table, max_blks, limit, &group_items);

	return valid;
}

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicTypeAmiga::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool sts = DiskBasicType::AssignRootDirectory(start_sector, end_sector, group_items, dir_item);
	if (dir_item) {
		int trk = 0;
		int sid = 0;
		DiskD88Sector *sector = basic->GetSectorFromGroup(m_root.block_num, trk, sid);
		dir_item->SetDataPtr(0, NULL, sector, 0, sector->GetSectorBuffer());
	}
	DiskBasicDirItemAmiga::RenumberInDirectory(basic, dir_item->GetChildren());
	return sts;
}

/// ディレクトリをアサイン
/// @param [in]     is_root      ルートか
/// @param [in]     group_items  セクタリスト
/// @param [in,out] dir_item     ディレクトリアイテム
/// @return true / false
bool DiskBasicTypeAmiga::AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool sts = DiskBasicType::AssignDirectory(is_root, group_items, dir_item);
	DiskBasicDirItemAmiga::RenumberInDirectory(basic, dir_item->GetChildren());
	return sts;
}

/// セクタをディレクトリとして初期化
/// @param [in]     group_items 確保したセクタリスト
/// @param [in,out] file_size   サイズ ディレクトリを拡張した時は既存サイズに加算
/// @param [in,out] size_remain 残りサイズ
/// @param [in,out] errinfo     エラー情報
/// @return 0:正常 <0:エラー 
int DiskBasicTypeAmiga::InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &size_remain, DiskBasicError &errinfo)
{
	size_remain = 0;

	return 0;
}

/// ディレクトリエリアのサイズに達したらアサイン終了するか
/// @param[in,out] pos         ディレクトリの位置
/// @param[in,out] size        ディレクトリのセクタサイズ
/// @param[in,out] size_remain ディレクトリの残りサイズ
/// @retval  0 : 終了しない
/// @retval  1 : 強制的に未使用とする アサインは継続
/// @retval -1 : 現グループでアサイン終了。次のグループから継続
/// @retval -2 : 強制的にアサイン終了する
int DiskBasicTypeAmiga::FinishAssigningDirectory(int &pos, int &size, int &size_remain) const
{
	// ディレクトリエントリは１セクタ分のヘッダになるので最初だけアサイン
	if (pos > 0) {
		size_remain += size;
		return -1;
	}
	return 0;
}

/// 使用可能なディスクサイズを得る
/// @param[out] disk_size  ディスクサイズ
/// @param[out] group_size グループ数
void DiskBasicTypeAmiga::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() - 1;	// exclude boot blocks
	group_size -= (int)m_bitmap.Count();		// exclude bitmap blocks
	disk_size = group_size * basic->GetSectorSize();
}

/// 残りディスクサイズを計算
/// @param[in] wrote 書き込んだ後か
void DiskBasicTypeAmiga::CalcDiskFreeSize(bool wrote)
{
	fat_availability.Empty();

	int block_size = basic->GetSectorSize();
	if (!basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM))) {
		block_size -= (int)(sizeof(amiga_file_data_pre_t) - 1);
	}

	// BITMAP table
	for(wxUint32 num = 0; num <= basic->GetFatEndGroup(); num++) {
		if (num < 2) {
			fat_availability.Add(FAT_AVAIL_SYSTEM, 0, 0);
		} else if (m_bitmap.IsFree(num)) {
			fat_availability.Add(FAT_AVAIL_FREE, block_size, 1);
		} else {
			fat_availability.Add(FAT_AVAIL_USED, 0, 0);
		}
	}

	fat_availability.Item(m_root.block_num) = FAT_AVAIL_SYSTEM;

	for(size_t i=0; i<m_bitmap.Count(); i++) {
		const AmigaOneBitmap *item = &m_bitmap.Item(i);
		fat_availability.Item(item->GetBlockNumber()) = FAT_AVAIL_SYSTEM;
	}

//	free_disk_size = (int)fat_availability.GetFreeSize();
//	free_groups = (int)fat_availability.GetFreeGroups();
}

/// グループ番号を使用済みにする
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeAmiga::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	m_bitmap.Modify(num, val != 0);
}

/// グループ番号を得る
wxUint32 DiskBasicTypeAmiga::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeAmiga::IsUsedGroupNumber(wxUint32 num)
{
	return !m_bitmap.IsFree(num);
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeAmiga::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeAmiga::GetEmptyGroupNumber()
{
	return GetEmptyGroupNumberM();
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeAmiga::GetEmptyGroupNumberM()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	int num_of_secs = basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	int sta_trk = 0;
	int end_trk = 0;
	int ndir = 1;

	for(int i=0; i<2; i++) {
		switch(i) {
		case 0:
			// 外側へ検索
			sta_trk = basic->GetManagedTrackNumber();
			end_trk = basic->GetTracksPerSideOnBasic() + basic->GetTrackNumberBaseOnDisk();
			ndir = 1;
			break;
		case 1:
			// 内側へ検索
			sta_trk = basic->GetManagedTrackNumber() - 1;
			end_trk = basic->GetTrackNumberBaseOnDisk() - 1;
			ndir = -1;
			break;
		}

		for(int trk_num = sta_trk; trk_num != end_trk && new_num == INVALID_GROUP_NUMBER; trk_num += ndir) {
			for(int sec_num = 0; sec_num < num_of_secs; sec_num++) {
				wxUint32 num = GetSectorPosFromNumS(trk_num, sec_num + basic->GetSectorNumberBase());
				if (m_bitmap.IsFree(num)) {
					new_num = num;
					break;
				}
			}
		}
	}
	return new_num;
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeAmiga::GetNextEmptyGroupNumber(wxUint32 curr_group)
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

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeAmiga::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	return true;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeAmiga::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
//	myLog.SetDebug("DiskBasicTypeAmiga::AllocateGroups {");

	int file_size = 0;
	int groups = 0;

	int rc = 0;

	// ディレクトリ新規作成の時は何もしない
	if (flags == ALLOCATE_GROUPS_NEW && item->IsDirectory()) {
		return 0;
	}

	int block_size = basic->GetSectorSize();
	if (!basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM))) {
		// OFSなら24バイト減らす
		block_size -= (int)(sizeof(amiga_file_data_pre_t) - 1);
	}
	int remain = data_size;
	int limit = basic->GetFatEndGroup() + 1;
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	wxUint32 prev_group_num = 0;
	int prev_remain = 0;

	DiskBasicDirItemAmiga *aitem = (DiskBasicDirItemAmiga *)item;
	int block_nums = aitem->GetDataBlockNums();
	int block_idx = block_nums - 1;
	int extension = -1;
	wxUint32 header_block_num = aitem->GetStartGroup(fileunit_num);

	while(remain > 0 && limit >= 0 && rc >= 0) {
		if (block_idx < 0) {
			// データテーブルがいっぱいになったので
			// extensionブロックを新たに確保する
			wxUint32 ex_num = GetEmptyGroupNumber();
			if (ex_num == INVALID_GROUP_NUMBER) {
				// 空きなし
				rc = -2;
				break;
			}
			DiskD88Sector *sector = basic->GetSectorFromGroup(ex_num);
			if (!sector) {
				rc = -2;
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				rc = -2;
				break;
			}
			// 使用済みにする
			SetGroupNumber(ex_num, 1);

			// extensionブロックへのリンクを作成
			aitem->SetExtension(ex_num);
			aitem->SetHighSeq(block_nums);

			// aitem切替
			if (extension >= 0) {
				delete aitem;
			}
			aitem = (DiskBasicDirItemAmiga *)dir->NewItem(sector, extension, buffer);

			sector->Fill(0);
			aitem->InitForExtensionBlock(header_block_num);
			block_nums = aitem->GetDataBlockNums();
			block_idx = block_nums - 1;

			extension++;
		}

		// 空きをさがす
		group_num = GetEmptyGroupNumber();
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			rc = groups > 0 ? -2 : -1;
			break;
		}

		// 使用済みにする
		if (prev_group_num > 0) {
			basic->GetNumsFromGroup(prev_group_num, group_num, basic->GetSectorSize(), prev_remain, group_items);
		}
		prev_group_num = group_num;
		prev_remain = remain;

		SetGroupNumber(group_num, 1);
		aitem->SetDataBlock(block_idx, group_num);

		file_size += block_size;
		groups++;
		remain -= block_size;
		limit--;

		block_idx--;
	}
	if (prev_group_num > 0) {
		basic->GetNumsFromGroup(prev_group_num, 0, basic->GetSectorSize(), prev_remain, group_items);
	}

	aitem->SetHighSeq(block_nums - block_idx - 1);

//	if (groups > 0) {
//		// 最終セクタは残りサイズを設定
//		remain += block_size;
//		ChainLastGroup(group_num, remain);
//	}

	if (limit < 0) {
		// 無限ループ？
		rc = groups > 0 ? -2 : -1;
	}

	// aitem切替
	if (extension >= 0) {
		delete aitem;
	}

//	myLog.SetDebug("rc: %d }", rc);
	return rc;
}

/// グループをつなげる
int DiskBasicTypeAmiga::ChainGroups(wxUint32 group_num, wxUint32 append_group_num)
{
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
int DiskBasicTypeAmiga::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
//	amiga_file_data_pre_t *pre = NULL;
	const wxUint8 *buffer = sector_buffer;
	int size = sector_size;

	if (!basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM))) {
		// OFSなら24バイト減らす
//		pre = (amiga_file_data_pre_t *)buffer;
		size -= (int)(sizeof(amiga_file_data_pre_t) - 1);
		buffer += (sizeof(amiga_file_data_pre_t) - 1);
	}
	if (remain_size < size) {
		size = remain_size;
	}

	if (ostream) {
		// 書き出し
		temp.SetData(buffer, size, basic->IsDataInverted());
		ostream->Write(temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read(temp.GetData(), temp.GetSize());
		temp.InvertData(basic->IsDataInverted());

		if (memcmp(temp.GetData(), buffer, size) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeAmiga::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	return remain_size;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeAmiga::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeAmiga::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return group_num;
}

/// ファイル名からハッシュ番号を生成する
/// @param [in,out] name ファイル名
/// @param [in,out] size サイズ
/// @return ハッシュ番号
int	DiskBasicTypeAmiga::CreateHashNumberFromName(wxUint8 *name, size_t size)
{
	int hash, l;
	int bsize = (basic->GetSectorSize() - (int)sizeof(amiga_block_pre_t) - (int)sizeof(amiga_block_post_t) + 4) / 4;
	bool is_intr = basic->GetVariousBoolParam(wxT(KEY_INTERNATIONAL));

	l = hash = (int)str_length(name, size, 0);
	for(int i=0; i<l; i++) {
		hash *= 13;
		hash += DiskBasicDirItemAmiga::Upper(name[i], is_intr);
		hash &= 0x7ff;
	}
	hash %= bsize;

	return hash;
}

/// ルートディレクトリか
bool DiskBasicTypeAmiga::IsRootDirectory(wxUint32 group_num)
{
	return (group_num == m_root.block_num);
}

/// 未使用のディレクトリアイテムを返す
/// @param [in,out] parent    ディレクトリ
/// @param [in,out] items     ディレクトリアイテム一覧
/// @param [in,out] pitem     ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out]    next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicTypeAmiga::GetEmptyDirectoryItem(DiskBasicDirItem *parent, DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	wxUint8 name[32];
	size_t len = sizeof(name);
	size_t elen = 0;

	// ファイル名からハッシュ番号を算出し、ハッシュテーブルに関連付ける
	if (parent && pitem) {
		DiskBasicFileType pattr = parent->GetFileAttr();
		if (!pattr.IsDirectory()) {
			// Why not directory?
			return match_item;
		}

		// ファイル名を得る
		pitem->GetNativeFileName(name, len, NULL, elen);
		// ハッシュ番号を計算
		int hash = CreateHashNumberFromName(name, len);

		// 新しいセクタを確保
		wxUint32 new_num = GetEmptyGroupNumber();
		if (new_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			return match_item;
		}
		// 実際にセクタがあるか
		DiskD88Sector *sector = basic->GetSectorFromGroup(new_num);
		if (!sector) {
			return match_item;
		}
		wxUint8 *buffer = sector->GetSectorBuffer();
		if (!buffer) {
			return match_item;
		}

		// ヘッダ情報をセット
		DiskBasicDirItemAmiga *apitem = (DiskBasicDirItemAmiga *)pitem;
		apitem->SetStartGroup(0, new_num);
		apitem->InitForHeaderBlock(parent->GetStartGroup(0));

		// アイテムを新規作成
		match_item = dir->NewItem(sector, 0, buffer);

		// セクタにヘッダ情報をセット
		sector->Fill(0);

		// セクタ使用中にする
		SetGroupNumber(new_num, 1);

		// 親ディレクトリをセット
		match_item->SetParent(parent);

		// ハッシュ番号を登録
		DiskBasicDirItemAmiga *aparent = (DiskBasicDirItemAmiga *)parent;
		aparent->ChainHashNumber(hash, new_num, match_item);

		// ディレクトリリストに追加する
		int limit = basic->GetFatEndGroup() + 1;
		wxUint32 *tables = aparent->GetBlockTable();
		int nums = aparent->GetDataBlockNums();

		if (!items) {
			parent->CreateChildren();
			items = parent->GetChildren();
		}
		DiskBasicDirItemAmiga::InsertItemInDirectory(basic, tables, nums, limit, items, match_item);
	}
	return match_item;
}

/// サブディレクトリを作成した後の個別処理
/// @param [in,out] item        作成したディレクトリアイテム    
/// @param [in,out] group_items グループ数
/// @param [in]     parent_item 親ディレクトリ
void DiskBasicTypeAmiga::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	// ビットマップのチェックサムを更新する
	m_bitmap.UpdateCheckSum();

	// 日時
	TM tm = TM::GetNow();
	item->SetFileModifyDateTime(tm);

	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) return;

	DiskBasicDirItemAmiga *aparent = (DiskBasicDirItemAmiga *)parent;

	// 親ディレクトリの日時を更新する（ルートディレクトリの場合も含む）
	aparent->SetFileModifyDateTime(tm);

	// ルートディレクトリのボリューム日時を更新する
	SetVolumeDateTime(tm);

	// 親ディレクトリのチェックサムを更新する
	aparent->UpdateCheckSum();

	// ルートディレクトリのチェックサムを更新する
	if (aparent->GetParent()) UpdateCheckSumOnRoot();
}

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeAmiga::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskD88Sector *sector;

	bool is_ffs = basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM));
	bool is_intr = false;	// support international
	bool is_dirc = false;	// support directory cache

	int track_num;
	int sector_num;

	//
	// Boot Block を作成
	//
	wxUint32 blk = 0;
	sector = basic->GetSectorFromGroup(blk);
	sector->Fill(0);
	amiga_boot_block_t *boot = (amiga_boot_block_t *)sector->GetSectorBuffer();
	memcpy(boot->type, "DOS", 3);
	if (is_ffs) {
		boot->type[3] |= 1;
	}
	if (is_intr) {
		boot->type[3] |= 2;
		if (is_dirc) boot->type[3] += 2;
	}

	const DiskBasicParam *default_param = gDiskBasicTemplates.FindType(wxT(""), basic->GetBasicTypeName());
	track_num = default_param->GetManagedTrackNumber();
	sector_num = default_param->GetDirStartSector();
	basic->SetManagedTrackNumber(track_num);
	basic->SetDirStartSector(sector_num);

	wxUint32 root_block = (wxUint32)GetSectorPosFromNumS(track_num, sector_num);
	boot->root_block = wxUINT32_SWAP_ON_LE(root_block);

	// ユーザディスクなのでチェックサムを計算しない
#if 0
	// calc check sum on boot block
	wxUint32 sum = CalcCheckSumOnBootBlock(0, sector->GetSectorBuffer(), sector->GetSectorSize());

	// next sector
	sector = basic->GetSectorFromGroup(blk + 1);
	sector->Fill(0);
	sum = CalcCheckSumOnBootBlock(sum, sector->GetSectorBuffer(), sector->GetSectorSize());
	sum = ~sum;
	boot->check_sum = wxUINT32_SWAP_ON_LE(sum);
#endif

	//
	// bitmapブロックを確保
	//
	basic->SetFatEndGroup(basic->GetSidesPerDiskOnBasic() * basic->GetTracksPerSideOnBasic() * basic->GetSectorsPerTrackOnBasic() - 1);
	m_bitmap.Clear();
	blk = root_block;
	do {
		blk++;
		sector = basic->GetSectorFromGroup(blk);
		m_bitmap.AddBitmap(blk, sector->GetSectorBuffer(), sector->GetSectorSize());
	} while (m_bitmap.GetBlockNums() < basic->GetFatEndGroup() + 1);

	m_bitmap.FreeAll(basic->GetFatEndGroup());

	basic->SetSectorsPerFat((int)m_bitmap.Count());

	//
	// Root Block を作成
	//
	sector = basic->GetSectorFromGroup(root_block);
	sector->Fill(0);
	m_root.block_num = root_block;
	m_root.pre = (amiga_block_pre_t *)sector->GetSectorBuffer();
	m_root.pre->type = wxUINT32_SWAP_ON_LE(FILETYPE_MASK_AMIGA_HEADER);

	wxUint32 val = (sector->GetSectorSize() - (int)sizeof(amiga_block_pre_t) - (int)sizeof(amiga_block_post_t) + 4) / 4;
	m_root.pre->table_size = wxUINT32_SWAP_ON_LE(val);

	m_root.post = (amiga_block_post_t *)sector->GetSectorBuffer((int)sizeof(amiga_block_pre_t) + val * 4 - 4);
	m_root.post->r.bm_flag = wxUINT32_SWAP_ON_LE(-1);
	for(size_t i=0; i<m_bitmap.Count(); i++) {
		val = m_bitmap.Item(i).GetBlockNumber();
		m_root.post->r.bm_pages[i] = wxUINT32_SWAP_ON_LE(val);
		m_bitmap.Modify(val, true);
	}

	m_root.post->r.sec_type = wxUINT32_SWAP_ON_LE(FILETYPE_MASK_AMIGA_ROOT);

	if (is_ffs && is_dirc) {
		// block number to first directory cache block
		m_root.post->r.extension = wxUINT32_SWAP_ON_LE(0);
	}

	// 日時
	TM tm = TM::GetNow();
	SetCreateDateTime(tm);
	SetVolumeDateTime(tm);
	SetModifyDateTime(tm);

	// volume name
	SetIdentifiedData(data);

	// bitmapをセット
	m_bitmap.Modify(root_block, true);

	// チェックサムを計算
	m_bitmap.UpdateCheckSum();
	DiskBasicDirItemAmiga::CalcCheckSum(m_root.pre, basic->GetSectorSize(), m_root.pre->check_sum);

	return true;
}

/// Bootブロックのチェックサムを計算
wxUint32 DiskBasicTypeAmiga::CalcCheckSumOnBootBlock(wxUint32 sum, const wxUint8 *data, size_t size)
{
	for(size_t i=0; i<size; i+=4) {
		wxUint32 d = *(wxUint32 *)(&data[i]);
		d = wxUINT32_SWAP_ON_LE(d);
		wxUint32 prev = sum;
		sum += d;
		if (sum < prev) sum++; // overflow
	}
	return sum;
}

/// 指定したサイズが十分書き込めるか
bool DiskBasicTypeAmiga::IsEnoughFileSize(int size) const
{
	int block_size = basic->GetSectorSize();
	if (!basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM))) {
		// OFSなら24バイト減らす
		block_size -= (int)(sizeof(amiga_file_data_pre_t) - 1);
	}
	int table_cnt = (basic->GetSectorSize() - (int)sizeof(amiga_block_pre_t) - (int)sizeof(amiga_block_post_t) + 4) / 4;
	// 必要なブロック数
	int data_cnt = (size + block_size - 1) / block_size;
	// ヘッダ＋extensionの数を計算
	int header_cnt = 1 + data_cnt / table_cnt;

	return (GetFreeGroupSize() >= (header_cnt + data_cnt));
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
int DiskBasicTypeAmiga::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	int len = 0;
	amiga_file_data_pre_t *pre = NULL;
//	int block_size = size;

	if (!basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM))) {
		// OFSなら24バイト減らす
		pre = (amiga_file_data_pre_t *)buffer;
		size -= (int)(sizeof(amiga_file_data_pre_t) - 1);
		buffer += (sizeof(amiga_file_data_pre_t) - 1);
	}

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
	if (pre) {
		// OFSの場合、パラメータをセット
		DiskBasicDirItemAmiga *aitem = (DiskBasicDirItemAmiga *)item;
		wxUint32 val =FILETYPE_MASK_AMIGA_DATA;
		pre->o.type = wxUINT32_SWAP_ON_LE(val);
		val = aitem->GetStartGroup(0);
		pre->o.header_key = wxUINT32_SWAP_ON_LE(val);
		val = seq_num + 1;
		pre->o.seq_num = wxUINT32_SWAP_ON_LE(val);
		val = (remain > size ? size : remain);
		pre->o.data_size = wxUINT32_SWAP_ON_LE(val);
		pre->o.next_data = wxUINT32_SWAP_ON_LE(next_group);
	}

	return len;
}

/// データの書き込み終了後の処理
/// @param [in]	 item			ディレクトリアイテム
void DiskBasicTypeAmiga::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	// ビットマップのチェックサムを更新する
	m_bitmap.UpdateCheckSum();

	DiskBasicDirItemAmiga *aitem = (DiskBasicDirItemAmiga *)item;
	aitem->UpdateCheckSumAll();

	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) return;

	DiskBasicDirItemAmiga *aparent = (DiskBasicDirItemAmiga *)parent;

	// 親ディレクトリの日時を更新する（ルートディレクトリの場合も含む）
	TM tm = TM::GetNow();
	aparent->SetFileModifyDateTime(tm);

	// ルートディレクトリのボリューム日時を更新する
	SetVolumeDateTime(tm);

	// 親ディレクトリのチェックサムを更新する
	aparent->UpdateCheckSum();

	// ルートディレクトリのチェックサムを更新する
	if (aparent->GetParent()) UpdateCheckSumOnRoot();
}

/// ファイル名変更後の処理
void DiskBasicTypeAmiga::AdditionalProcessOnRenamedFile(DiskBasicDirItem *item)
{
	wxUint8 name[32];
	size_t len = sizeof(name);
	size_t elen = 0;

	// ファイル名を得る
	item->GetNativeFileName(name, len, NULL, elen);
	// ハッシュ番号を計算
	int hash_num = CreateHashNumberFromName(name, len);

	DiskBasicDirItemAmiga *aitem = (DiskBasicDirItemAmiga *)item;
	if (hash_num == aitem->GetHashNumber()) {
		// ハッシュ番号が同じなら変更なし
		return;
	}

	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) return;

	DiskBasicDirItemAmiga *aparent = (DiskBasicDirItemAmiga *)parent;

	int limit = basic->GetFatEndGroup() + 1;
	wxUint32 *tables = aparent->GetBlockTable();
	int nums = aparent->GetDataBlockNums();

	// ディレクトリリストから削除する
	DiskBasicDirItemAmiga::DeleteItemInDirectory(basic, tables, nums, limit, parent->GetChildren(), item);

	// ハッシュ番号を登録
	wxUint32 block_num = item->GetStartGroup(0);
	aparent->ChainHashNumber(hash_num, block_num, item);

	// ディレクトリリストに追加する
	DiskBasicDirItemAmiga::InsertItemInDirectory(basic, tables, nums, limit, parent->GetChildren(), item);
}

/// FAT領域を削除する
void DiskBasicTypeAmiga::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}

/// ファイル削除後の処理
bool DiskBasicTypeAmiga::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// ビットマップのチェックサムを更新する
	m_bitmap.UpdateCheckSum();

	// ディレクトリリストから削除する
	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) return false;

	DiskBasicDirItemAmiga *aparent = (DiskBasicDirItemAmiga *)parent;

	int limit = basic->GetFatEndGroup() + 1;
	wxUint32 *tables = aparent->GetBlockTable();
	int nums = aparent->GetDataBlockNums();

	DiskBasicDirItemAmiga::DeleteItemInDirectory(basic, tables, nums, limit, parent->GetChildren(), item);

	// 親ディレクトリの日時を更新する（ルートディレクトリの場合も含む）
	TM tm = TM::GetNow();
	aparent->SetFileModifyDateTime(tm);

	// ルートディレクトリのボリューム日時を更新する
	SetVolumeDateTime(tm);

	// 親ディレクトリのチェックサムを更新する
	aparent->UpdateCheckSum();

	// ルートディレクトリのチェックサムを更新する
	if (aparent->GetParent()) UpdateCheckSumOnRoot();

	return true;
}

/// リストから除いたのでディレクトリアイテムを削除する
/// @param[in,out] item ディレクトリアイテム
void DiskBasicTypeAmiga::ReleaseDirectoryItem(DiskBasicDirItem *item)
{
	delete item;
}

/// IPLや管理エリアの属性を得る
/// @param[out] data ボリューム名など
void DiskBasicTypeAmiga::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	if (!m_root.post) return;

	// volume name in root
	wxUint8 name[sizeof(m_root.post->r.disk_name) + 1];
	memset(name, 0, sizeof(name));
	size_t len = m_root.post->r.disk_name_len;
	memcpy(name, m_root.post->r.disk_name, len); 

	wxString wname;
	basic->GetCharCodes().ConvToString(name, len, wname, 0);
	data.SetVolumeName(wname);
	data.SetVolumeNameMaxLength(sizeof(m_root.post->r.disk_name));

	// volume date in root
	TM tm;
	DiskBasicDirItemAmiga::ConvDateToTm(
		wxUINT32_SWAP_ON_LE(m_root.post->r.c_days),
		tm);
	DiskBasicDirItemAmiga::ConvTimeToTm(
		wxUINT32_SWAP_ON_LE(m_root.post->r.c_mins),
		wxUINT32_SWAP_ON_LE(m_root.post->r.c_ticks),
		tm);
	wxString datetime = Utils::FormatYMDStr(tm);
	datetime += wxT(" ");
	datetime += Utils::FormatHMSStr(tm);
	data.SetVolumeDate(datetime);
}

/// IPLや管理エリアの属性をセット
/// @param[in] data ボリューム名など
void DiskBasicTypeAmiga::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	if (!m_root.post) return;

	const DiskBasicFormat *fmt = basic->GetFormatType();

	// volume name
	if (fmt->HasVolumeName()) {
		wxUint8 name[sizeof(m_root.post->r.disk_name) + 1];
		memset(name, 0, sizeof(name));
		int len = basic->GetCharCodes().ConvToChars(data.GetVolumeName(), name, sizeof(name));
		if (len >= 0) {
			memcpy(m_root.post->r.disk_name, name, len + 1);
			m_root.post->r.disk_name_len = (len & 0xff);
		}
	}
}

/// ルートのチェックサムを計算
void DiskBasicTypeAmiga::UpdateCheckSumOnRoot()
{
	DiskBasicDirItemAmiga *aroot = (DiskBasicDirItemAmiga *)dir->GetRootItem();
	aroot->UpdateCheckSum();
}

/// ルートの更新日時をセット
/// @param[in] tm 日時
void DiskBasicTypeAmiga::SetModifyDateTime(const TM &tm)
{
	wxUint32 days, mins, ticks;
	DiskBasicDirItemAmiga::ConvDateFromTm(tm, days);
	DiskBasicDirItemAmiga::ConvTimeFromTm(tm, mins, ticks);
	days = wxUINT32_SWAP_ON_LE(days);
	mins = wxUINT32_SWAP_ON_LE(mins);
	ticks = wxUINT32_SWAP_ON_LE(ticks);
	m_root.post->r.r_days = days;
	m_root.post->r.r_mins = mins;
	m_root.post->r.r_ticks = ticks;
}

/// ルートのボリューム日時をセット
/// @param[in] tm 日時
void DiskBasicTypeAmiga::SetVolumeDateTime(const TM &tm)
{
	wxUint32 days, mins, ticks;
	DiskBasicDirItemAmiga::ConvDateFromTm(tm, days);
	DiskBasicDirItemAmiga::ConvTimeFromTm(tm, mins, ticks);
	days = wxUINT32_SWAP_ON_LE(days);
	mins = wxUINT32_SWAP_ON_LE(mins);
	ticks = wxUINT32_SWAP_ON_LE(ticks);
	m_root.post->r.v_days = days;
	m_root.post->r.v_mins = mins;
	m_root.post->r.v_ticks = ticks;
}

/// ルートの作成日時をセット
/// @param[in] tm 日時
void DiskBasicTypeAmiga::SetCreateDateTime(const TM &tm)
{
	wxUint32 days, mins, ticks;
	DiskBasicDirItemAmiga::ConvDateFromTm(tm, days);
	DiskBasicDirItemAmiga::ConvTimeFromTm(tm, mins, ticks);
	days = wxUINT32_SWAP_ON_LE(days);
	mins = wxUINT32_SWAP_ON_LE(mins);
	ticks = wxUINT32_SWAP_ON_LE(ticks);
	m_root.post->r.c_days = days;
	m_root.post->r.c_mins = mins;
	m_root.post->r.c_ticks = ticks;
}
