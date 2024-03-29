/// @file basictype_cpm.cpp
///
/// @brief disk basic type for CP/M
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_cpm.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_cpm.h"
#include "../logging.h"


//
//
//
DiskBasicTypeCPM::DiskBasicTypeCPM(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	sector_skew.Create(basic, basic->GetSectorsPerTrackOnBasic());
}

DiskBasicTypeCPM::~DiskBasicTypeCPM()
{
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeCPM::ParseParamOnDisk(bool is_formatting)
{
	// 最終グループ番号
	if (basic->GetFatEndGroup() == 0) {
		wxUint32 max_group = (basic->GetTracksPerSide() - basic->GetManagedTrackNumber()) * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic() / basic->GetSectorsPerGroup() - 1;
		basic->SetFatEndGroup(max_group);
	}

	if (is_formatting) return 1.0;

	// 最終グループ番号が最大値を超えていないか？
	if((1 << (basic->GetGroupWidth() * 8)) <= (int)basic->GetFatEndGroup()) {
		return -1.0;
	}

	// セクタ０
	DiskImageSector *sector = basic->GetSector(0, 0, 1);
	if (!sector) {
		return -1.0;
	}

	// 最初のセクタに識別文字がある場合はその文字列が含まれるかで判断
	double valid_ratio = 0.5;
	int found = -1;
	wxCharBuffer istr;
	for(int i=0; i<1; i++) {
		found = -1;
		switch(i) {
		case 0:
			istr = basic->GetVariousStringParam(wxT("IdentString")).To8BitData();
			break;
		case 1:
			istr = basic->GetVariousStringParam(wxT("IPLString")).To8BitData();
			break;
		}
		if (istr.length() > 0) {
			found = sector->Find(istr.data(), istr.length());
		}
		if (found >= 0) {
			valid_ratio = 1.0;
			break;
		}
	}

	return valid_ratio;
}

/// エリアをチェック
double DiskBasicTypeCPM::CheckFat(bool is_formatting)
{
	return 1.0;
}

//

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
bool DiskBasicTypeCPM::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool sts = DiskBasicType::AssignRootDirectory(start_sector, end_sector, group_items, dir_item);

	// エクステント 同じファイル名 を関連付ける
	DiskBasicDirItems sort_items = *dir_item->GetChildren();
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
			citem->CalcFileUnitSize(0);
		}
	}

	return sts;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeCPM::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
void DiskBasicTypeCPM::CalcDiskFreeSize(bool wrote)
{
//	wxUint32 fsize = 0;
//	wxUint32 grps = 0;

	fat_availability.Empty();
	fat_availability.SetCount(basic->GetFatEndGroup() + 1, FAT_AVAIL_FREE);

	const DiskBasicDirItems *items = dir->GetCurrentItems();
	for(size_t idx = 0; idx < items->Count(); idx++) {
		DiskBasicDirItem *item = items->Item(idx);
		if (!item || !item->IsUsed()) continue;

		// グループ番号のマップを調べる
		const DiskBasicGroups *groups = &item->GetGroups();
		int count = (int)groups->Count();
		for(int n = 0; n < count; n++) {
			const DiskBasicGroupItem *group = groups->ItemPtr(n);
			wxUint32 gnum = group->group;
			if (gnum <= basic->GetFatEndGroup()) {
				if (n + 1 == count) {
					fat_availability.Set(gnum, FAT_AVAIL_USED_LAST);
				} else {
					fat_availability.Set(gnum, FAT_AVAIL_USED);
				}
			}
		}
	}

	// 空きをチェック
	int grps = 0;
	wxUint32 dir_area = ((basic->GetDirEndSector() - basic->GetDirStartSector() + 1) / basic->GetSectorsPerGroup());
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		if (pos < dir_area) {
			// ディレクトリエリアは使用済み
			fat_availability.Set(pos, FAT_AVAIL_SYSTEM);
		} else if (fat_availability.Get(pos) == FAT_AVAIL_FREE) {
//			fat_availability.Item(pos) = FAT_AVAIL_FREE;
			grps++;
		}
	}

	int fsize = grps * basic->GetSectorSize() * basic->GetSectorsPerGroup();

	fat_availability.SetFreeSize(fsize);
	fat_availability.SetFreeGroups(grps);
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeCPM::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	fat_availability.Item(num) = (val ? FAT_AVAIL_USED : FAT_AVAIL_FREE);
}

/// グループ番号を得る
wxUint32 DiskBasicTypeCPM::GetGroupNumber(wxUint32 num) const
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
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		if (fat_availability.Item(pos) == FAT_AVAIL_FREE) {
			group_num = pos;
			break;
		}
	}
	return group_num;
}

/// 次の空き位置を返す 未使用
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeCPM::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	return INVALID_GROUP_NUMBER;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    確保するデータサイズ（バイト）
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeCPM::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int rc = 0;

	DiskBasicDirItemCPM *ditem = (DiskBasicDirItemCPM *)item;
	int group_entries = ditem->GetGroupEntries();

	int start_gpos = 0;
	if (flags == ALLOCATE_GROUPS_APPEND) {
		// 追加の時、空きエントリをさがす
		while(ditem->GetGroup(start_gpos) != 0) {
			start_gpos++;
			if ((start_gpos % group_entries) == 0) {
				// グループエントリ数に達したら次のディレクトリエントリに移動
				ditem = ditem->GetNextItem();
				if (!ditem) {
					// 次がない
					break;
				}
			}
		}
	}
	if (!ditem) {
		return -1;
	}

	int gpos = start_gpos;
	int group_size = (basic->GetSectorSize() * basic->GetSectorsPerGroup());
	int remain_size = data_size;
	int file_size = 0;
	int limit = basic->GetFatEndGroup() + 1;
	while(remain_size > 0 && limit >= 0 && rc == 0) {
		wxUint32 gnum = GetEmptyGroupNumber();
		if (gnum == INVALID_GROUP_NUMBER) {
			rc = -2;
			break;
		}
		basic->GetNumsFromGroup(gnum, 0, basic->GetSectorSize(), remain_size, group_items);
		// 使用中にする
		SetGroupNumber(gnum, 1);
		// グループエントリ
		ditem->SetGroup((gpos % group_entries), gnum);

		file_size += (remain_size < group_size ? remain_size : group_size);
		// エクステント番号とレコード番号をセット
		ditem->CalcExtentAndRecordNumber(file_size);
	
		remain_size -= group_size;
		limit--;

		gpos++;
		if (remain_size > 0 && (gpos % group_entries) == 0) {
			// グループエントリ数に達したら次のディレクトリエントリに移動
			ditem = ditem->GetNextItem();
			if (!ditem) {
				// 次がない！？
				rc = -2;
				break;
			}
		}
	}
	if (limit < 0) {
		rc = -2;
	}
	if (rc < 0) {
		DeleteGroups(group_items);
		// グループエントリを削除
		ditem = (DiskBasicDirItemCPM *)item;
		gpos = 0;
		while(ditem->GetGroup(gpos) != 0) {
			if (gpos >= start_gpos) {
				ditem->SetGroup(gpos, 0);
			}
			gpos++;
			if ((gpos % group_entries) == 0) {
				// グループエントリ数に達したら次のディレクトリエントリに移動
				ditem = ditem->GetNextItem();
				if (!ditem) {
					break;
				}
			}
		}
	}
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
int DiskBasicTypeCPM::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
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
int DiskBasicTypeCPM::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num * basic->GetSectorsPerGroup();
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeCPM::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int val = sector_start;
	if (remain_size < (sector_size * basic->GetSectorsPerGroup())) {
		val += ((remain_size - 1) / sector_size);
	} else {
		val += (basic->GetSectorsPerGroup() - 1);
	}
	return val;
}

/// データ領域の開始セクタを計算
int DiskBasicTypeCPM::CalcDataStartSectorPos()
{
	return GetSectorPosFromNumS(basic->GetManagedTrackNumber(), basic->GetDirStartSector());
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos    セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num    トラック番号
/// @param [out] side_num     サイド番号
/// @param [out] sector_num   セクタ番号
/// @param [out] div_num      分割番号
/// @param [out] div_nums     分割数
void DiskBasicTypeCPM::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();

	if (selected_side >= 0) {
		// 1S
		track_num = sector_pos / sectors_per_track;
		side_num = selected_side;
	} else {
		// 2D, 2HD
		track_num = sector_pos / sectors_per_track / sides_per_disk;
		side_num = (sector_pos / sectors_per_track) % sides_per_disk;
	}
	sector_num = (sector_pos % sectors_per_track);

	// マッピング
	sector_num = sector_skew.ToPhysical(sector_num);

	if (numbering_sector == 1) {
		// トラックごとに連番の場合
		sector_num += (side_num * sectors_per_track);
	}

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBase();

	if (div_num)  *div_num = 0;
	if (div_nums) *div_nums = 1;
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeCPM::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	// 連番の場合
	if (numbering_sector == 1) {
		sector_num = (sector_num % sectors_per_track);
	}

	// マッピング
	sector_num = sector_skew.ToLogical(sector_num);

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * sectors_per_track + sector_num;
	} else {
		// 2D, 2HD
		sector_pos = track_num * sectors_per_track * sides_per_disk;
		sector_pos += (side_num % sides_per_disk) * sectors_per_track;
		sector_pos += sector_num;
	}
	return sector_pos;
}

/// ルートディレクトリか
bool DiskBasicTypeCPM::IsRootDirectory(wxUint32 group_num)
{
	return true;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeCPM::FillSector(DiskImageTrack *track, DiskImageSector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
bool DiskBasicTypeCPM::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// ディレクトリエリア
	for(int sec_pos = basic->GetDirStartSector(); sec_pos <= basic->GetDirEndSector(); sec_pos++) {
		DiskImageSector *sector = basic->GetManagedSector(sec_pos - 1);
		if (sector) {
			sector->Fill(basic->GetFillCodeOnDir());
		}
	}
	return true;
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeCPM::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	DiskBasicDirItemCPM *ditem = (DiskBasicDirItemCPM *)nitem;
	// グループエントリ数
	int group_entries = ditem->GetGroupEntries();
	// １ディレクトリで設定できるファイルサイズを求める (32K)
	int limit_size = basic->GetSectorSize() * basic->GetSectorsPerGroup() * group_entries;

	ditem->Used(true);
	ditem->Visible(true);

	int remain_size = (int)istream.GetLength();

	DiskBasicDirItemCPM *prev_aitem = ditem;
	DiskBasicDirItemCPM *aitem = NULL;
	while (limit_size < remain_size) {
		// １ディレクトリで入りきらないので追加でディレクトリエントリを確保
		aitem = (DiskBasicDirItemCPM *)dir->GetEmptyItemOnCurrent(pitem, NULL);
		if (aitem == NULL) {
			errinfo.SetError(DiskBasicError::ERR_DIRECTORY_FULL);
			return false;
		}
		aitem->CopyData(prev_aitem->GetData());
		aitem->Used(true);
		aitem->Visible(false);

		prev_aitem->SetNextItem(aitem);

		remain_size -= limit_size;

		prev_aitem = aitem;
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
int DiskBasicTypeCPM::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	bool need_eof_code = item->NeedCheckEofCode();

	int len = 0;
	if (remain <= size) {
		// 残り少ない
		wxUint8 term = 0;
		if (remain < 0) remain = 0;
		if (remain > 0) istream.Read((void *)buffer, remain);
		if (need_eof_code && ((remain % SECTOR_UNIT_CPM) != 0)) {
			// アスキーは終端コードをサプレス
			term = basic->GetTextTerminateCode();
		}
		// 残りを128バイトで丸める
		int size128 = ((remain + SECTOR_UNIT_CPM - 1) / SECTOR_UNIT_CPM) * SECTOR_UNIT_CPM;
		if (remain < size128) {
			// バッファの余りはサプレス(128バイト境界まで)
			memset((void *)&buffer[remain], term, size128 - remain);
			size = size128;
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
void DiskBasicTypeCPM::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}
