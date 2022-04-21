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

#if 0
/// ディスクから各パラメータを取得
int DiskBasicTypeCPM::ParseParamOnDisk(DiskD88Disk *disk)
{
	return 0;
}
#endif

//

#if 0
/// ルートディレクトリのチェック
bool DiskBasicTypeCPM::CheckRootDirectory(int start_sector, int end_sector)
{
}
#endif

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
bool DiskBasicTypeCPM::AssignRootDirectory(int start_sector, int end_sector)
{
	DiskBasicType::AssignRootDirectory(start_sector, end_sector);

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
//	fat_availability.Empty();

	fat_availability.SetCount(basic->GetFatEndGroup() + 1, FAT_AVAIL_FREE);

	const DiskBasicDirItems *items = &dir->GetItems();
	for(size_t idx = 0; idx < items->Count(); idx++) {
		DiskBasicDirItemCPM *citem = (DiskBasicDirItemCPM *)items->Item(idx);
		if (!citem || !citem->IsUsed()) continue;

		// グループ番号のマップを調べる
		for(int n = 0; n < citem->GetGroupEntries(); n++) {
			wxUint32 gnum = citem->GetGroup(n);
			if (gnum == 0) break;
			if (gnum <= basic->GetFatEndGroup()) {
				fat_availability.Item(gnum) = FAT_AVAIL_USED;
			}
		}
	}

	// 空きをチェック
	int dir_area = ((basic->GetDirEndSector() - basic->GetDirStartSector() + 1) / basic->GetSectorsPerGroup());
	for(int pos = 0; pos <= (int)basic->GetFatEndGroup(); pos++) {
		if (pos < dir_area) {
			// ディレクトリエリアは使用済み
			fat_availability.Item(pos) = FAT_AVAIL_SYSTEM;
		} else if (fat_availability.Item(pos) != FAT_AVAIL_USED) {
			fat_availability.Item(pos) = FAT_AVAIL_FREE;
			grps++;
		}
	}

	fsize = grps * basic->GetSectorSize() * basic->GetSectorsPerGroup();

	free_disk_size = (int)fsize;
	free_groups = (int)grps;
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
/// @param [in]  item        ディレクトリアイテム
/// @param [in]  data_size   確保するデータサイズ（バイト）
/// @param [out] group_items 確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeCPM::AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items)
{
	int rc = 0;

	DiskBasicDirItemCPM *ditem = (DiskBasicDirItemCPM *)item;
	int group_entries = ditem->GetGroupEntries();

	int start_gpos = 0;
	// 空きエントリをさがす
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
		if ((gpos % group_entries) == 0) {
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
/// @param [in] sector_size   バッファイサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeCPM::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	int size = remain_size - 1;

	// ファイルサイズはセクタサイズ境界なので要計算

	// 終端コードは除く
	wxUint8 eof_code = basic->InvertUint8(0x1a);
	for(; size >= 0; size--) {
		if (sector_buffer[size] != eof_code) {
			break;
		}
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
bool DiskBasicTypeCPM::AdditionalProcessOnFormatted()
{
	// ディレクトリエリア
	for(int sec_pos = basic->GetDirStartSector(); sec_pos <= basic->GetDirEndSector(); sec_pos++) {
		DiskD88Sector *sector = basic->GetManagedSector(sec_pos - 1);
		if (sector) {
			sector->Fill(basic->GetFillCodeOnDir());
		}
	}
	return true;
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream ストリームバッファ
/// @param [in]     pitem   ファイル名、属性を持っているディレクトリアイテム
/// @param [in]     nitem   確保したディレクトリアイテム
/// @param [in,out] errinfo エラー情報
bool DiskBasicTypeCPM::PrepareToSaveFile(wxInputStream &istream, const DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
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
		aitem = (DiskBasicDirItemCPM *)dir->GetEmptyItemPtr(NULL);
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
/// @return 書き込んだバイト数
int DiskBasicTypeCPM::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	bool need_eof_code = item->NeedCheckEofCode();

	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (need_eof_code && ((remain % SECTOR_UNIT_CPM) != 0)) {
			// 最終は終端コード
			if (remain > 1) istream.Read((void *)buffer, remain - 1);
			if (remain > 0) buffer[remain - 1]=0x1a;
			if (SECTOR_UNIT_CPM > remain) {
				// バッファの余りは0x1aサプレス(128バイト未満)
				memset((void *)&buffer[remain], 0x1a, SECTOR_UNIT_CPM - remain);
				size = SECTOR_UNIT_CPM;
			} else if (size > remain && remain > SECTOR_UNIT_CPM) {
				// バッファの余りは0x1aサプレス
				memset((void *)&buffer[remain], 0x1a, size - remain);
			}
		} else {
			if (remain > 0) istream.Read((void *)buffer, remain);
			if (SECTOR_UNIT_CPM > remain) {
				// バッファの余りは0サプレス(128バイト未満)
				memset((void *)&buffer[remain], 0, SECTOR_UNIT_CPM - remain);
				size = SECTOR_UNIT_CPM;
			} else if (size > remain && remain > SECTOR_UNIT_CPM) {
				// バッファの余りは0サプレス
				memset((void *)&buffer[remain], 0, size - remain);
			}
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
