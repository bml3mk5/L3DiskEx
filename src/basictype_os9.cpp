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
}

/// エリアをチェック
bool DiskBasicTypeOS9::CheckFat()
{
	bool valid = true;

	// Ident
	DiskD88Sector *sector = basic->GetDisk()->GetSector(1, 0, 1);
	os9_ident_t *os9 = (os9_ident_t *)sector->GetSectorBuffer();

	if (GET_OS9_LSN(os9->DD_TOT) == 0) {
		valid = false;
	}
	if (os9->DD_SPT == 0) {
		valid = false;
	}

	return valid;
}

/// ディスクから各パラメータを取得
bool DiskBasicTypeOS9::ParseParamOnDisk(DiskD88Disk *disk)
{
	// Ident
	DiskD88Sector *sector = basic->GetDisk()->GetSector(1, 0, 1);
	os9_ident_t *os9 = (os9_ident_t *)sector->GetSectorBuffer();
	if (!os9) {
		return false;
	}

	int ival;

	// total groups
	ival = GET_OS9_LSN(os9->DD_TOT);
	if (ival < 1) {
		return false;
	}
	ival--;
	basic->SetFatEndGroup(ival);
	end_group = (wxUint32)ival;

	// sectors per track
	ival = wxUINT16_SWAP_ON_LE(os9->DD_SPT);
	if (ival == 0) {
		return false;
	}
	if (ival > basic->GetSectorsPerTrack()) {
		return false;
	}
	basic->SetSectorsPerTrackOnBasic(ival);

	// sectors per group(cluster)
	ival = wxUINT16_SWAP_ON_LE(os9->DD_BIT);
	if (ival == 0) {
		return false;
	}
	basic->SetSectorsPerGroup(ival);
	secs_per_group = ival;

	// root directory
	wxUint32 dir_fd_lsn = GET_OS9_LSN(os9->DD_DIR);
	if (dir_fd_lsn > end_group) {
		return false;
	}
	sector = basic->GetDisk()->GetSector(1, 0, dir_fd_lsn + 1);
	if (!sector) {
		return false;
	}
	directory_os9_fd_t *fdd = (directory_os9_fd_t *)sector->GetSectorBuffer();

	wxUint32 dir_start_lsn = GET_OS9_LSN(fdd->FD_SEG[0].LSN);

	basic->SetDirStartSector(dir_start_lsn + CalcDataStartSectorPos() + 1);
	basic->SetDirEndSector(dir_start_lsn + CalcDataStartSectorPos() + 2);

	return true;
}

//

/// ルートディレクトリのチェック
bool DiskBasicTypeOS9::CheckRootDirectory(int start_sector, int end_sector)
{
	bool valid = true;
	bool last = false;

	// Ident
	DiskD88Sector *sector = basic->GetDisk()->GetSector(1, 0, 1);
	os9_ident_t *os9 = (os9_ident_t *)sector->GetSectorBuffer();

	// root directory
	wxUint32 dir_fd_lsn = GET_OS9_LSN(os9->DD_DIR);
	sector = basic->GetDisk()->GetSector(1, 0, dir_fd_lsn + 1);
	directory_os9_fd_t *fdd = (directory_os9_fd_t *)sector->GetSectorBuffer();
	if (!fdd) {
		return false;
	}

	for(int i = 0; i < 48 && valid && !last; i++) {
		wxUint32 start_lsn = GET_OS9_LSN(fdd->FD_SEG[i].LSN);
		if (start_lsn == 0) {
			break;
		}

		wxUint32 end_lsn = wxUINT16_SWAP_ON_LE(fdd->FD_SEG[i].SIZ);
		end_lsn = end_lsn * secs_per_group + start_lsn;

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

	// Ident
	DiskD88Sector *sector = basic->GetDisk()->GetSector(1, 0, 1);
	os9_ident_t *os9 = (os9_ident_t *)sector->GetSectorBuffer();

	// root directory
	wxUint32 dir_fd_lsn = GET_OS9_LSN(os9->DD_DIR);
	sector = basic->GetDisk()->GetSector(1, 0, dir_fd_lsn + 1);
	directory_os9_fd_t *fdd = (directory_os9_fd_t *)sector->GetSectorBuffer();
	if (!fdd) {
		return false;
	}

	for(int i = 0; i < 48; i++) {
		wxUint32 start_lsn = GET_OS9_LSN(fdd->FD_SEG[i].LSN);
		if (start_lsn == 0) {
			break;
		}

		wxUint32 end_lsn = wxUINT16_SWAP_ON_LE(fdd->FD_SEG[i].SIZ);
		end_lsn = end_lsn * secs_per_group + start_lsn;

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

/// 残りディスクサイズを計算
void DiskBasicTypeOS9::CalcDiskFreeSize()
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	fat_availability.Empty();

	// Ident
	DiskD88Sector *sector = basic->GetDisk()->GetSector(1, 0, 1);
	os9_ident_t *os9 = (os9_ident_t *)sector->GetSectorBuffer();

	wxUint32 map_bytes = wxUINT16_SWAP_ON_LE(os9->DD_MAP);
	wxUint32 bytes = 0;
	wxUint32 lsn = 0;
	for(wxUint32 map_lsn = 1; map_lsn <= 32 && bytes < map_bytes; map_lsn++) {
		sector = basic->GetSectorFromGroup(map_lsn);
		if (!sector) {
			// error
			break;
		}
		wxUint8 *buf = sector->GetSectorBuffer();
		int size = sector->GetSectorSize();
		for(int pos = 0; pos < size && lsn <= end_group && bytes < map_bytes; pos++) {
			for(int bit = 0; bit < 8 && lsn <= end_group && bytes < map_bytes; bit++) {
				bool used = ((buf[pos] & (0x80 >> bit)) != 0);
				if (!used) {
					fat_availability.Add(FAT_AVAIL_FREE);
					fsize += (secs_per_group * sector_size);
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

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeOS9::SetGroupNumber(wxUint32 num, wxUint32 val)
{
}

/// グループ番号を得る
wxUint32 DiskBasicTypeOS9::GetGroupNumber(wxUint32 num)
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeOS9::IsUsedGroupNumber(wxUint32 num)
{
	return true;
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
	return INVALID_GROUP_NUMBER;
}

/// 次の空き位置を返す 未使用
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeOS9::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	return INVALID_GROUP_NUMBER;
}

/// データサイズ分のグループを確保する
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeOS9::AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items)
{
	return -1;
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
	return basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic();
}

/// ルートディレクトリか
bool DiskBasicTypeOS9::IsRootDirectory(wxUint32 group_num)
{
	return ((group_num + CalcDataStartSectorPos() + 1) <= (wxUint32)basic->GetDirStartSector());
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeOS9::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item, wxUint32 parent_group_num)
{
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeOS9::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
void DiskBasicTypeOS9::AdditionalProcessOnFormatted()
{
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
int DiskBasicTypeOS9::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	return 0;
}

/// データの書き込み終了後の処理
void DiskBasicTypeOS9::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
}

/// ファイル名変更後の処理
void DiskBasicTypeOS9::AdditionalProcessOnRenamedFile(DiskBasicDirItem *item)
{
}

/// FAT領域を削除する
void DiskBasicTypeOS9::DeleteGroupNumber(wxUint32 group_num)
{
}
