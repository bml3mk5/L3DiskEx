/// @file basictype_os9.cpp
///
/// @brief disk basic type for OS-9
///
#include "basictype_os9.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_os9.h"
#include "logging.h"

//
//
//
DiskBasicTypeOS9::DiskBasicTypeOS9(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	os9_ident = NULL;
}

/// エリアをチェック
bool DiskBasicTypeOS9::CheckFat()
{
	bool valid = true;

	// Ident
	DiskD88Sector *sector = basic->GetManagedSector(0);
	os9_ident = (os9_ident_t *)sector->GetSectorBuffer();

	if (GET_OS9_LSN(os9_ident->DD_TOT) == 0) {
		valid = false;
	}
	if (os9_ident->DD_SPT == 0) {
		valid = false;
	}

	return valid;
}

/// ディスクから各パラメータを取得
/// @retval  1 警告あり
/// @retval  0 正常
/// @retval -1 エラー
int DiskBasicTypeOS9::ParseParamOnDisk(DiskD88Disk *disk)
{
	int valid = 0;

	// Ident
	DiskD88Sector *sector = basic->GetManagedSector(0);
	os9_ident = (os9_ident_t *)sector->GetSectorBuffer();
	if (!os9_ident) {
		return -1;
	}

	int ival;

	// total groups
	ival = GET_OS9_LSN(os9_ident->DD_TOT);
	if (ival < 1) {
		return -1;
	}
	ival--;
	basic->SetFatEndGroup(ival);
//	end_group = (wxUint32)ival;

	// sectors per track
	ival = wxUINT16_SWAP_ON_LE(os9_ident->DD_SPT);
	if (ival == 0) {
		return -1;
	}
	if (ival > basic->GetSectorsPerTrack()) {
		valid = 1;
	} else {
		basic->SetSectorsPerTrackOnBasic(ival);
	}

	// sectors per group(cluster)
	ival = wxUINT16_SWAP_ON_LE(os9_ident->DD_BIT);
	if (ival == 0) {
		return -1;
	}
	basic->SetSectorsPerGroup(ival);
//	secs_per_group = ival;

	// root directory
	wxUint32 dir_fd_lsn = GET_OS9_LSN(os9_ident->DD_DIR);
	if (dir_fd_lsn > basic->GetFatEndGroup()) {
		return -1;
	}
	sector = basic->GetManagedSector(dir_fd_lsn);
	if (!sector) {
		return -1;
	}
	directory_os9_fd_t *fdd = (directory_os9_fd_t *)sector->GetSectorBuffer();

	for(int i = 0; i < 48; i++) {
		wxUint32 start_lsn = GET_OS9_LSN(fdd->FD_SEG[i].LSN);
		wxUint32 end_lsn = wxUINT16_SWAP_ON_LE(fdd->FD_SEG[i].SIZ);
		if (start_lsn == 0 && end_lsn == 0) {
			break;
		}
		end_lsn = end_lsn * basic->GetSectorsPerGroup() + start_lsn;

		if (i == 0) basic->SetDirStartSector(start_lsn);
		basic->SetDirEndSector(end_lsn);
	}

	return valid;
}

//

/// ルートディレクトリのチェック
bool DiskBasicTypeOS9::CheckRootDirectory(int start_sector, int end_sector, bool is_formatting)
{
	// フォーマット中はチェックしない
	if (is_formatting) return true;

	bool valid = true;
	bool last = false;

	// root directory
	wxUint32 dir_fd_lsn = GET_OS9_LSN(os9_ident->DD_DIR);
	DiskD88Sector *sector = basic->GetManagedSector(dir_fd_lsn);
	if (!sector) {
		return false;
	}
	directory_os9_fd_t *fdd = (directory_os9_fd_t *)sector->GetSectorBuffer();
	if (!fdd) {
		return false;
	}

	for(int i = 0; i < 48 && valid && !last; i++) {
		wxUint32 start_lsn = GET_OS9_LSN(fdd->FD_SEG[i].LSN);
		wxUint32 end_lsn = wxUINT16_SWAP_ON_LE(fdd->FD_SEG[i].SIZ);
		if (start_lsn == 0 && end_lsn == 0) {
			break;
		}
		end_lsn = end_lsn * basic->GetSectorsPerGroup() + start_lsn;

		for(wxUint32 lsn = start_lsn; lsn < end_lsn; lsn++) {
			sector = basic->GetSectorFromGroup(lsn);
			if (!sector) {
				valid = false;
				break;
			}

			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				valid = false;
				break;
			}
	
			int remain = sector->GetSectorSize();

			// ディレクトリのチェック
			while(valid && !last && remain > 0) {
				DiskBasicDirItem *nitem = dir->NewItem(sector, buffer);
				valid = nitem->Check(last);
				remain -= nitem->GetDataSize();
				buffer += nitem->GetDataSize();
				delete nitem;
			}
		}
	}
	return valid;
}

/// ルートディレクトリをアサイン
bool DiskBasicTypeOS9::AssignRootDirectory(int start_sector, int end_sector)
{
	int index_number = 0;
	bool unuse = false;

	// root directory
	wxUint32 dir_fd_lsn = GET_OS9_LSN(os9_ident->DD_DIR);
	DiskD88Sector *sector = basic->GetManagedSector(dir_fd_lsn);
	if (!sector) {
		return false;
	}
	directory_os9_fd_t *fdd = (directory_os9_fd_t *)sector->GetSectorBuffer();
	if (!fdd) {
		return false;
	}

	for(int i = 0; i < 48; i++) {
		wxUint32 start_lsn = GET_OS9_LSN(fdd->FD_SEG[i].LSN);
		wxUint32 end_lsn = wxUINT16_SWAP_ON_LE(fdd->FD_SEG[i].SIZ);
		if (start_lsn == 0 && end_lsn == 0) {
			break;
		}
		end_lsn = end_lsn * basic->GetSectorsPerGroup() + start_lsn;

		for(wxUint32 lsn = start_lsn; lsn < end_lsn; lsn++) {
			int trk_num = 0;
			int sid_num = 0;
			sector = basic->GetSectorFromGroup(lsn, trk_num, sid_num);
			if (!sector) {
				break;
			}

			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				break;
			}

			int pos = 0;
			int size = sector->GetSectorSize();

			buffer += basic->GetDirStartPosOnSector();
			size -= basic->GetDirStartPosOnSector();

			while(pos < size) {
				DiskBasicDirItem *item = dir->AssignItem(index_number, trk_num, sid_num, sector, pos, buffer, unuse);
				pos += item->GetDataSize();
				buffer += item->GetDataSize();
				index_number++;
			}
		}
	}

	return true;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeOS9::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
void DiskBasicTypeOS9::CalcDiskFreeSize(bool wrote)
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	fat_availability.Empty();

	DiskD88Sector *sector = NULL;

	// Allocation Mapを調べる
	wxUint32 map_start_lsn = wxUINT32_SWAP_ON_LE(os9_ident->DD_MapLSN) + 1;
	wxUint32 map_bytes = wxUINT16_SWAP_ON_LE(os9_ident->DD_MAP);
	wxUint32 bytes = 0;
	wxUint32 lsn = 0;
	for(wxUint32 map_lsn = map_start_lsn; map_lsn <= 32 && bytes < map_bytes; map_lsn++) {
		sector = basic->GetSectorFromGroup(map_lsn);
		if (!sector) {
			// error
			break;
		}
		wxUint8 *buf = sector->GetSectorBuffer();
		int size = sector->GetSectorSize();
		for(int pos = 0; pos < size && lsn <= basic->GetFatEndGroup() && bytes < map_bytes; pos++) {
			for(int bit = 0; bit < 8 && lsn <= basic->GetFatEndGroup() && bytes < map_bytes; bit++) {
				bool used = ((buf[pos] & (0x80 >> bit)) != 0);
				if (!used) {
					fat_availability.Add(FAT_AVAIL_FREE);
					fsize += (basic->GetSectorsPerGroup() * basic->GetSectorSize());
					grps++;
				} else {
					fat_availability.Add(FAT_AVAIL_USED);
				}
				lsn++;
			}
			bytes++;
		}
	}

	free_disk_size = (int)fsize;
	free_groups = (int)grps;
}

/// 使用状態を設定する
/// @param [in] num グループ番号(0...)
/// @param [in] val 1:使用中 0:空きにする
void DiskBasicTypeOS9::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	if (num > basic->GetFatEndGroup()) return;

	// Allocation Mapを調べる
	wxUint32 map_start_lsn = wxUINT32_SWAP_ON_LE(os9_ident->DD_MapLSN) + 1;
	wxUint32 map_lsn = ((num >> 3) / basic->GetSectorSize()) + map_start_lsn;
	int lsn_pos = ((num >> 3) % basic->GetSectorSize());
	int lsn_bit = (num & 7);

	DiskD88Sector *sector = basic->GetSectorFromGroup(map_lsn);
	wxUint8 *map_buf = sector->GetSectorBuffer();

	if (val) {
		map_buf[lsn_pos] |= (0x80 >> lsn_bit);
	} else {
		map_buf[lsn_pos] &= ~(0x80 >> lsn_bit);
	}
}

/// グループ番号を得る
wxUint32 DiskBasicTypeOS9::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeOS9::IsUsedGroupNumber(wxUint32 num)
{
	if (num > basic->GetFatEndGroup()) return false;

	// Allocation Mapを調べる
	wxUint32 map_start_lsn = wxUINT32_SWAP_ON_LE(os9_ident->DD_MapLSN) + 1;
	wxUint32 map_lsn = ((num >> 3) / basic->GetSectorSize()) + map_start_lsn;
	int lsn_pos = ((num >> 3) % basic->GetSectorSize());
	int lsn_bit = (num & 7);

	DiskD88Sector *sector = basic->GetSectorFromGroup(map_lsn);
	wxUint8 *map_buf = sector->GetSectorBuffer();

	return ((map_buf[lsn_pos] & (0x80 >> lsn_bit)) != 0);
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeOS9::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeOS9::GetEmptyGroupNumber()
{
	DiskD88Sector *sector = NULL;

	// Allocation Mapを調べる
	wxUint32 match_lsn = INVALID_GROUP_NUMBER;
	wxUint32 map_start_lsn = wxUINT32_SWAP_ON_LE(os9_ident->DD_MapLSN) + 1;
	wxUint32 map_bytes = wxUINT16_SWAP_ON_LE(os9_ident->DD_MAP);
	wxUint32 bytes = 0;
	wxUint32 lsn = 0;
	for(wxUint32 map_lsn = map_start_lsn; map_lsn <= 32 && bytes < map_bytes && match_lsn == INVALID_GROUP_NUMBER; map_lsn++) {
		sector = basic->GetSectorFromGroup(map_lsn);
		if (!sector) {
			// error
			break;
		}
		wxUint8 *buf = sector->GetSectorBuffer();
		int size = sector->GetSectorSize();
		for(int pos = 0; pos < size && lsn <= basic->GetFatEndGroup() && bytes < map_bytes && match_lsn == INVALID_GROUP_NUMBER; pos++) {
			for(int bit = 0; bit < 8 && lsn <= basic->GetFatEndGroup() && bytes < map_bytes && match_lsn == INVALID_GROUP_NUMBER; bit++) {
				bool used = ((buf[pos] & (0x80 >> bit)) != 0);
				if (!used) {
					match_lsn = lsn;
					break;
				}
				lsn++;
			}
			bytes++;
		}
	}
	return match_lsn;
}

/// 次の空き位置を返す 未使用
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeOS9::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	return INVALID_GROUP_NUMBER;
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream ストリームバッファ
/// @param [in]     pitem   ファイル名、属性を持っているディレクトリアイテム
/// @param [in]     nitem   確保したディレクトリアイテム
/// @param [in,out] errinfo エラー情報
bool DiskBasicTypeOS9::PrepareToSaveFile(wxInputStream &istream, const DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	// FDセクタを確保する
	wxUint32 lsn = GetEmptyGroupNumber();
	if (lsn == INVALID_GROUP_NUMBER) {
		return false;
	}
	DiskBasicDirItemOS9 *ditem = (DiskBasicDirItemOS9 *)nitem;
	DiskD88Sector *sector = basic->GetSectorFromGroup(lsn);

	DiskBasicDirItemOS9FD *fd = &ditem->GetFD();
	fd->Set(sector, (directory_os9_fd_t *)sector->GetSectorBuffer());
	fd->Clear();

	// 属性をコピー
	if (pitem) ditem->CopyItem(*pitem);

	// 開始LSNを設定
	ditem->SetStartGroup(lsn);

	// リンク数
	fd->SetLNK(1);

	// セクタを予約
	SetGroupNumber(lsn, 1);

	return true;
}

/// データサイズ分のグループを確保する
/// @param [in]  item        ディレクトリアイテム
/// @param [in]  data_size   確保するデータサイズ（バイト）
/// @param [in]  flags       新規か追加か
/// @param [out] group_items 確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeOS9::AllocateGroups(DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	DiskBasicDirItemOS9 *ditem = (DiskBasicDirItemOS9 *)item;
	DiskBasicDirItemOS9FD *fd = &ditem->GetFD();

	int seg_idx = -1;
	int start_seg_idx = seg_idx + 1;

	int file_size = (int)fd->GetSIZ();
	data_size += file_size;

	// データ用のセクタを確保する
	int rc = 0;
	wxUint32 lsn = 0;
	wxUint32 prev_lsn = 0;
	wxUint16 seg_cnt = 0;
	int limit = basic->GetFatEndGroup() + 1;
	while(file_size < data_size && limit >= 0 && rc == 0) {
		lsn = GetEmptyGroupNumber();
		if (lsn == INVALID_GROUP_NUMBER) {
			// 空きなし？
			rc = -2;
			break;
		}
		if (prev_lsn == 0 || (prev_lsn + 1) != lsn) {
			// LSNが連続していない
			seg_idx++;
			if (seg_idx >= 48) {
				// セグメント限界
				rc = -2;
				break;
			}
			fd->SetLSN(seg_idx, lsn);
			seg_cnt = 1;
			fd->SetSIZ(seg_idx, seg_cnt);
		} else {
			// LSNが連続しているなら、同じセグメントでセクタ数を増やす
			seg_cnt++;
			fd->SetSIZ(seg_idx, seg_cnt);
		}
		// セクタを予約
		SetGroupNumber(lsn, 1);
		// グループ追加
		basic->GetNumsFromGroup(lsn, 0, basic->GetSectorSize(), 0, group_items);
		// LSNを保持
		prev_lsn = lsn;

		if (file_size + basic->GetSectorSize() > data_size) {
			file_size = data_size;
		} else {
			file_size += basic->GetSectorSize();
		}
		// ファイルサイズ
		fd->SetSIZ(file_size);

		limit--;
	}
	if (limit < 0) {
		rc = -2;
	}

	// エラーの場合、確保したエリアを開放
	if (rc < 0) {
		for(int idx = start_seg_idx; idx < 48; idx++) {
			wxUint32 seg_lsn = fd->GetLSN(idx);
			int seg_siz = fd->GetSIZ(idx);
			if (seg_lsn == 0 && seg_siz == 0) {
				break;
			}
			for(int siz = 0; siz < seg_siz; siz++) {
				// セクタを未使用にする
				SetGroupNumber(seg_lsn, 0);
				seg_lsn++;
			}
			fd->SetLSN(idx, 0);
			fd->SetSIZ(idx, 0);
		}
	}

	return rc;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeOS9::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeOS9::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return group_num;
}

/// データ領域の開始セクタを計算
int DiskBasicTypeOS9::CalcDataStartSectorPos()
{
	return basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic();
}

/// ルートディレクトリか
bool DiskBasicTypeOS9::IsRootDirectory(wxUint32 group_num)
{
	return ((group_num + 1) <= (wxUint32)basic->GetDirStartSector());
}

/// サブディレクトリを作成する前の準備を行う
/// @param [in]  item    確保したディレクトリアイテム
bool DiskBasicTypeOS9::PrepareToMakeDirectory(DiskBasicDirItem *item)
{
	// FDセクタを確保する
	wxUint32 lsn = GetEmptyGroupNumber();
	if (lsn == INVALID_GROUP_NUMBER) {
		return false;
	}
	DiskBasicDirItemOS9 *ditem = (DiskBasicDirItemOS9 *)item;
	DiskD88Sector *sector = basic->GetSectorFromGroup(lsn);

	DiskBasicDirItemOS9FD *fd = &ditem->GetFD();
	fd->Set(sector, (directory_os9_fd_t *)sector->GetSectorBuffer());
	fd->Clear();

	// 開始LSNを設定
	ditem->SetStartGroup(lsn);
	// 日付を設定
	struct tm tm;
	wxDateTime::GetTmNow(&tm);
	ditem->SetFileDateTime(&tm);
	ditem->SetCDate(&tm);
	// セクタを予約
	SetGroupNumber(lsn, 1);

	return true;
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeOS9::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	if (group_items.Count() <= 0) return;

	// ディレクトリ属性
	item->SetFileAttr(FILE_TYPE_DIRECTORY_MASK | (
		(
			FILETYPE_MASK_OS9_PUBLIC_EXEC |
			FILETYPE_MASK_OS9_PUBLIC_WRITE |
			FILETYPE_MASK_OS9_PUBLIC_READ |
			FILETYPE_MASK_OS9_USER_EXEC |
			FILETYPE_MASK_OS9_USER_WRITE |
			FILETYPE_MASK_OS9_USER_READ
		) << FILETYPE_OS9_PERMISSION_POS)
	);

	// ファイルサイズはエントリ２つ分
	item->SetFileSize((int)item->GetDataSize() * 2);

	// カレントと親ディレクトリのエントリを作成する
	DiskBasicGroupItem *gitem = &group_items.Item(0);

	DiskD88Sector *sector = basic->GetSector(gitem->track, gitem->side, gitem->sector_start);

	wxUint8 *buf = sector->GetSectorBuffer();
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, buf);

	// 親をつくる
	newitem->ClearData();
	if (parent_item) {
		// 親がサブディレクトリ
		newitem->SetStartGroup(parent_item->GetStartGroup());
	} else {
		// 親がルート
		newitem->SetStartGroup(GET_OS9_LSN(os9_ident->DD_DIR));
	}
	newitem->SetFileNamePlain(wxT(".."));
//	newitem->SetFileAttr(FILE_TYPE_DIRECTORY_MASK);

	// カレント
	buf += newitem->GetDataSize();
	newitem->SetDataPtr((directory_t *)buf);

	newitem->ClearData();
	newitem->SetStartGroup(item->GetStartGroup());
	newitem->SetFileNamePlain(wxT("."));
//	newitem->SetFileAttr(FILE_TYPE_DIRECTORY_MASK);

	// ディレクトリサイズを更新
	DiskBasicDirItem *dir_item = dir->FindFile(wxT("."), NULL, NULL);
	if (dir_item) {
		int dir_size = dir_item->GetFileSize() + (int)dir_item->GetDataSize();
		dir_item->SetFileSize(dir_size);
	}

	delete newitem;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeOS9::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
bool DiskBasicTypeOS9::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// Ident
	DiskD88Sector *sector = basic->GetManagedSector(0);
	if (!sector) return false;
	os9_ident = (os9_ident_t *)sector->GetSectorBuffer();
	if (!os9_ident) return false;

	int total_lsn = basic->GetFatEndGroup() + 1;

	int map_lsn = 1;
	int root_start_lsn = (basic->GetDirStartSector() - 1) / basic->GetSectorsPerGroup();
	int root_end_lsn = (basic->GetDirEndSector() - 1) / basic->GetSectorsPerGroup();
	int ival;

	//
	// OS9 Identifier をセット
	//

	sector->Fill(0);

	// total lsn
	SET_OS9_LSN(os9_ident->DD_TOT, total_lsn);
	// sectors per track
	os9_ident->DD_TKS = basic->GetSectorsPerTrackOnBasic();
	// allocation map length
	ival = (total_lsn + 7) / 8;
	os9_ident->DD_MAP = wxUINT16_SWAP_ON_LE(ival);
	// sectors per group
	ival = basic->GetSectorsPerGroup();
	os9_ident->DD_BIT = wxUINT16_SWAP_ON_LE(ival);
	// rootdir lsn
	SET_OS9_LSN(os9_ident->DD_DIR, root_start_lsn);
	// owner id
	os9_ident->DD_OWN = 0;
	// disk attr
	os9_ident->DD_ATT = 0;
	// disk ident
	os9_ident->DD_DSK = wxUINT16_SWAP_ON_LE(0);

	// format, density, number of sides
	ival = ((basic->GetSidesOnBasic() - 1) & 0x01);
	ival |= (basic->HasSingleDensity() == 1 ? 0 : 0x02);
	os9_ident->DD_FMT = (wxUint8)ival;

	// sector per track
	ival = basic->GetSectorsPerTrackOnBasic();
	os9_ident->DD_SPT = wxUINT16_SWAP_ON_LE(ival);

	// bootstrap lsn
	SET_OS9_LSN(os9_ident->DD_BT, 0);
	// bootstrap size (in bytes)
	os9_ident->DD_BSZ = wxUINT16_SWAP_ON_LE(0);

	// creation date
	struct tm tm;
	wxDateTime::GetTmNow(&tm);
	os9_ident->DD_DAT.yy = (tm.tm_year % 100);
	os9_ident->DD_DAT.mm = (tm.tm_mon + 1);
	os9_ident->DD_DAT.dd = (tm.tm_mday);
	os9_ident->DD_DAT.hh = (tm.tm_hour);
	os9_ident->DD_DAT.mi = (tm.tm_min);

	// bitmap starting sector number
	os9_ident->DD_MapLSN = wxUINT32_SWAP_ON_LE(map_lsn - 1);

	// volume label
	SetIdentifiedData(data);

	//
	// Allocation Mapを作成
	//

	for(int lsn = map_lsn; lsn < root_start_lsn; lsn++) {
		for(int sec = 0; sec < basic->GetSectorsPerGroup(); sec++) {
			sector = basic->GetManagedSector(lsn * basic->GetSectorsPerGroup() + sec);
			if (!sector) return false;
			sector->Fill(0xff);
		}
	}
	for(int lsn = root_end_lsn + 1; lsn < total_lsn; lsn++) {
		SetGroupNumber(lsn, 0);
	}

	//
	// ルートディレクトリを作成
	//

	sector = basic->GetManagedSector(root_start_lsn * basic->GetSectorsPerGroup());
	if (!sector) return false;

	DiskBasicDirItemOS9 *root_item = (DiskBasicDirItemOS9 *)dir->NewItem();
	DiskBasicDirItemOS9FD *root_fd = &root_item->GetFD();

	root_fd->Set(sector, (directory_os9_fd_t *)sector->GetSectorBuffer());
	root_fd->Clear();

	root_item->SetStartGroup(root_start_lsn);
	// 日付を設定
	root_item->SetFileDateTime(&tm);
	root_item->SetCDate(&tm);
	// セグメント設定
	root_fd->SetLSN(0, root_start_lsn + 1);
	root_fd->SetSIZ(0, root_end_lsn - root_start_lsn);
	// リンクの数
	root_fd->SetLNK(2);
	// セクタを予約
	SetGroupNumber(root_start_lsn, 1);

	for(int lsn = root_start_lsn + 1; lsn <= root_end_lsn; lsn++) {
		for(int sec = 0; sec < basic->GetSectorsPerGroup(); sec++) {
			sector = basic->GetManagedSector(lsn * basic->GetSectorsPerGroup() + sec);
			if (!sector) {
				continue;
			}
			sector->Fill(0);
		}
	}
	DiskBasicGroups group_items;
	basic->GetNumsFromGroup(root_start_lsn + 1, 0, basic->GetSectorSize(), basic->GetSectorSize(), group_items);
	AdditionalProcessOnMadeDirectory(root_item, group_items, NULL);

	delete root_item;

	return true;
}

/// データの書き込み終了後の処理
void DiskBasicTypeOS9::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	// ディレクトリサイズを更新
	DiskBasicDirItem *dir_item = dir->FindFile(wxT("."), NULL, NULL);
	if (dir_item) {
		int dir_size = dir_item->GetFileSize() + (int)dir_item->GetDataSize();
		dir_item->SetFileSize(dir_size);
	}
}

#if 0
/// ファイル名変更後の処理
void DiskBasicTypeOS9::AdditionalProcessOnRenamedFile(DiskBasicDirItem *item)
{
}
#endif

/// FAT領域を削除する
void DiskBasicTypeOS9::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}

/// ファイル削除後の処理
bool DiskBasicTypeOS9::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// FDセクタを未使用にする
	SetGroupNumber(item->GetStartGroup(), 0);

	// ディレクトリサイズを更新
	DiskBasicDirItem *dir_item = dir->FindName(wxT("."), NULL, NULL);
	if (dir_item) {
		int dir_size = dir_item->GetFileSize() - (int)dir_item->GetDataSize();
		dir_item->SetFileSize(dir_size);
	}

	return true;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeOS9::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// volume label
	char buf[sizeof(os9_ident->DD_NAM) + 1];
	DiskBasicDirItemOS9::DecodeString(buf, sizeof(os9_ident->DD_NAM), os9_ident->DD_NAM, sizeof(os9_ident->DD_NAM));
	wxString vol(buf, sizeof(os9_ident->DD_NAM));
	data.SetVolumeName(vol);
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeOS9::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	// volume label
	wxCharBuffer vol = data.GetVolumeName().To8BitData();
	DiskBasicDirItemOS9::EncodeString(os9_ident->DD_NAM, sizeof(os9_ident->DD_NAM), vol, vol.length());
}
