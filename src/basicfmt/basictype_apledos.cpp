/// @file basictype_apledos.cpp
///
/// @brief disk basic type for Apple DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_apledos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "basicdiritem_apledos.h"
#include "../utils.h"
#include "../logging.h"


//////////////////////////////////////////////////////////////////////
//
// Apple DOS 3.x の処理
//
DiskBasicTypeAppleDOS::DiskBasicTypeAppleDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	if (basic->GetGroupsPerTrack() <= 0) {
		basic->SetGroupsPerTrack(basic->GetGroupsPerSector() * basic->GetSectorsPerTrackOnBasic());
	}
}

/// エリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeAppleDOS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// VTOC area
	DiskImageSector *sector = basic->GetManagedSector(0);
	if (!sector) {
		return -1.0;
	}
	apledos_vtoc_t *vtoc = (apledos_vtoc_t *)sector->GetSectorBuffer();
	if (!vtoc) {
		return -1.0;
	}

	apledos_vtoc = vtoc;

	if (vtoc->tracks_per_disk == 0 || vtoc->sectors_per_track == 0) {
		return -1.0;
	}

	if (vtoc->dir_start_track < 3) {
		return -1.0;
	}

	// 各パラメータ
	basic->SetSectorsPerGroup(1);
	basic->SetTracksPerSideOnBasic(vtoc->tracks_per_disk);
	basic->SetSectorsPerTrackOnBasic(vtoc->sectors_per_track);
	basic->SetFatEndGroup(vtoc->tracks_per_disk * vtoc->sectors_per_track - 1);

	basic->SetManagedTrackNumber(vtoc->dir_start_track);
	basic->SetDirStartSector(vtoc->dir_start_sector);

	// ディレクトリをチェック
	// 通常セクタ15から1に向かう
//	int dir_cnt = 0;
	int dir_sta_lsec = basic->GetDirStartSector();
	int dir_end_lsec = 1;

	sector = basic->GetManagedSector(dir_sta_lsec);
	for(int lsec_pos = dir_sta_lsec; lsec_pos >= dir_end_lsec; lsec_pos--) {
		if (!sector) {
			valid_ratio = -1.0;
			break;
		}

		apledos_ptr_t *p = (apledos_ptr_t *)sector->GetSectorBuffer();
//		dir_cnt++;

		if (p->next_track == 0 && p->next_sector == 0) {
			break;
		}

		sector = basic->GetSectorFromSectorPos(
			GetSectorPosFromNumS(p->next_track + basic->GetTrackNumberBaseOnDisk(), p->next_sector + basic->GetSectorNumberBase())
		);
	}

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeAppleDOS::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 0;

	if (!apledos_vtoc) {
		DiskImageSector *sector = basic->GetManagedSector(0);
		apledos_vtoc_t *vtoc = (apledos_vtoc_t *)sector->GetSectorBuffer();
		apledos_vtoc = vtoc;

		// 各パラメータ
		basic->SetTracksPerSideOnBasic(vtoc->tracks_per_disk);
		basic->SetSectorsPerTrackOnBasic(vtoc->sectors_per_track);
		basic->SetFatEndGroup((vtoc->tracks_per_disk + 1) * vtoc->sectors_per_track - 1);

		basic->SetManagedTrackNumber(vtoc->dir_start_track);
		basic->SetDirStartSector(vtoc->dir_start_sector);
	}

//	myLog.SetInfo("APPLEDOS: vtoc.tracks_per_disk: %d", (int)apledos_vtoc->tracks_per_disk);
//	myLog.SetInfo("APPLEDOS: vtoc.sectors_per_track: %d", (int)apledos_vtoc->sectors_per_track);

	return 1.0;
}

/// トラックマップマスクを設定
void DiskBasicTypeAppleDOS::SetTrackMapMask(wxUint32 val)
{
	apledos_vtoc->track_bit_mask[0] = wxUINT16_SWAP_ON_LE(val & 0xffff);
	val >>= 16;
	apledos_vtoc->track_bit_mask[1] = wxUINT16_SWAP_ON_LE(val & 0xffff);
}

/// トラックマップのビットを変更
void DiskBasicTypeAppleDOS::ModifyTrackMap(int track_num, int sector_num, bool use)
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
bool DiskBasicTypeAppleDOS::IsFreeTrackMap(int track_num, int sector_num) const
{
	wxUint32 map = GetTrackMap(track_num);
	return ((map & (1 << sector_num)) != 0);
}

/// トラックマップを設定
void DiskBasicTypeAppleDOS::SetTrackMap(int track_num, wxUint32 val)
{
	val <<= (16 - basic->GetSectorsPerTrackOnBasic());
	apledos_vtoc->track_map[track_num][0] = wxUINT16_SWAP_ON_LE(val & 0xffff);
	val >>= 16;
	apledos_vtoc->track_map[track_num][1] = wxUINT16_SWAP_ON_LE(val & 0xffff);
}

/// トラックマップを得る
wxUint32 DiskBasicTypeAppleDOS::GetTrackMap(int track_num) const
{
	wxUint32 val = wxUINT16_SWAP_ON_LE(apledos_vtoc->track_map[track_num][1]);
	val <<= 16;
	val |= wxUINT16_SWAP_ON_LE(apledos_vtoc->track_map[track_num][0]);
	val >>= (16 - basic->GetSectorsPerTrackOnBasic());
	return val;
}

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
bool DiskBasicTypeAppleDOS::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	bool valid = true;

	group_items.Empty();

	// ディレクトリのチェインをたどる
	size_t dir_size = 0;
	int limit = basic->GetFatEndGroup() + 1;
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;
	// 開始セクタ
	DiskImageSector *sector = basic->GetManagedSector(start_sector, &trk_num, &sid_num);
	while(limit >= 0) {
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

		apledos_ptr_t *p = (apledos_ptr_t *)buffer;

		// 次のセクタなし
		if (p->next_track == 0 && p->next_sector == 0) {
			break;
		}

		limit--;

		// 次のセクタを得る
		sector = basic->GetSectorFromSectorPos(
			GetSectorPosFromNumS(p->next_track + basic->GetTrackNumberBaseOnDisk(), p->next_sector + basic->GetSectorNumberBase())
			, trk_num, sid_num);
	}
	group_items.SetSize(dir_size);

	if (limit < 0) {
		valid = false;
	}
	return valid;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeAppleDOS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	disk_size = group_size * basic->GetSectorSize() / basic->GetGroupsPerSector();
}

/// 残りディスクサイズを計算
void DiskBasicTypeAppleDOS::CalcDiskFreeSize(bool wrote)
{
	fat_availability.Empty();

	// BITMAP table on VTOC area

	// get BITMAP mask (value is often invalidate)
//	wxUint32 mask = wxUINT16_SWAP_ON_LE(apledos_vtoc->track_bit_mask[1]);
//	mask <<= 16;
//	mask |= wxUINT16_SWAP_ON_LE(apledos_vtoc->track_bit_mask[0]);
	int managed_track_num = basic->GetManagedTrackNumber();

	// check BITMAP
	for(int trk = 0; trk < basic->GetTracksPerSideOnBasic(); trk++) {
		wxUint32 usemap = GetTrackMap(trk);

		for(int sec=0; sec < basic->GetSectorsPerTrackOnBasic(); sec++) {
			if (trk < 3 || trk == managed_track_num) {
				fat_availability.Add(FAT_AVAIL_SYSTEM, 0, 0);
			} else if (usemap & (1 << sec)) {
				fat_availability.Add(FAT_AVAIL_FREE, basic->GetSectorSize(), 1);
			} else {
				fat_availability.Add(FAT_AVAIL_USED, 0, 0);
			}
		}
	}

//	free_disk_size = (int)fsize;
//	free_groups = (int)grps;
}

/// グループ番号を使用済みにする
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeAppleDOS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	int track_num = 0;
	int sector_num = 0;
	GetNumFromSectorPosS(num, track_num, sector_num);
	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();
	ModifyTrackMap(track_num, sector_num, val != 0);
}

/// グループ番号を得る
wxUint32 DiskBasicTypeAppleDOS::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeAppleDOS::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeAppleDOS::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeAppleDOS::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

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

	return new_num;
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeAppleDOS::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	// 次の空き位置候補
	wxUint32 new_num = INVALID_GROUP_NUMBER;

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

	return new_num;
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeAppleDOS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	// チェインセクタをクリア
	nitem->ClearChainSector();

	return true;
}

/// チェインセクタを確保する
/// @param [in] idx          チェイン番号
/// @param [in] item         ディレクトリアイテム
/// @param [in] curr_group   事前に確保したグループ番号
/// @return 確保したグループ番号 / INVALID_GROUP_NUMBER
wxUint32 DiskBasicTypeAppleDOS::AllocChainSector(int idx, DiskBasicDirItem *item, wxUint32 curr_group)
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

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeAppleDOS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
//	myLog.SetDebug("DiskBasicTypeAppleDOS::AllocateGroups {");

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
		if ((chain_idx % APLEDOS_TRACK_LIST_MAX) == 0) {
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
}

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeAppleDOS::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
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
	return remain_size;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeAppleDOS::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeAppleDOS::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return group_num;
}

/// セクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,最初のセクタを0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num 物理セクタ番号
/// @param [out] div_num    分割番号
/// @param [out] div_nums   分割数
void DiskBasicTypeAppleDOS::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
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
void DiskBasicTypeAppleDOS::GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num)
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
int  DiskBasicTypeAppleDOS::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
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
int  DiskBasicTypeAppleDOS::GetSectorPosFromNumS(int track_num, int sector_num)
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

/// ルートディレクトリか
bool DiskBasicTypeAppleDOS::IsRootDirectory(wxUint32 group_num)
{
	return false;
}

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeAppleDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
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
int DiskBasicTypeAppleDOS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
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
void DiskBasicTypeAppleDOS::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}

/// ファイル削除後の処理
bool DiskBasicTypeAppleDOS::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// チェインセクタを未使用にする
	wxUint32 gnum = item->GetStartGroup(0);
	while (gnum != 0) {
		SetGroupNumber(gnum, 0);
		DiskImageSector *sector = basic->GetSectorFromGroup(gnum);
		if (!sector) break;
		apledos_chain_t *p = (apledos_chain_t *)sector->GetSectorBuffer();
		if (!p) break;
		gnum = GetSectorPosFromNumS(p->next.next_track + basic->GetTrackNumberBaseOnDisk(),  p->next.next_sector + basic->GetSectorNumberBase());
	}

	DiskBasicDirItemAppleDOS *ditem = (DiskBasicDirItemAppleDOS *)item;
	// ディレクトリの最初に削除コード(0xff)を入れる
	ditem->SetStartTrack(basic->GetDeleteCode());

	return true;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeAppleDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// volume number
	data.SetVolumeNumber(apledos_vtoc->volume_number);
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeAppleDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	const DiskBasicFormat *fmt = basic->GetFormatType();

	// volume number
	if (fmt->HasVolumeNumber()) {
		apledos_vtoc->volume_number = (wxUint8)data.GetVolumeNumber();
		if (apledos_vtoc->volume_number == 0) {
			apledos_vtoc->volume_number = 0xfe;	// default
		}
	}
}
