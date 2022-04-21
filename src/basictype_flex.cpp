/// @file basictype_flex.cpp
///
/// @brief disk basic type for FLEX
///
#include "basictype_flex.h"
#include "basicfmt.h"
#include "basicdiritem.h"
#include "logging.h"

//
//
//
DiskBasicTypeFLEX::DiskBasicTypeFLEX(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	flex_sir = NULL;
}

/// エリアをチェック
bool DiskBasicTypeFLEX::CheckFat()
{
	bool valid = true;

	// SIR area
	DiskD88Sector *sector = basic->GetDisk()->GetSector(0, 0, 3);
	if (!sector) {
		return false;
	}
	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer();
	if (!flex) {
		return false;
	}

	for(size_t i=0; i<sizeof(flex->reserved0); i++) {
		if (flex->reserved0[i]) {
			valid = false;
			break;
		}
	}
	if (flex->max_track == 0 || flex->max_sector == 0) {
		valid = false;
	}

	if (!valid) return valid;

	flex_sir = flex;

	// DIRエリアのチェインをチェック
	int dir_cnt = 0;
	int dir_sta_sec = basic->GetDirStartSector() - 1;
	int dir_end_sec = basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic() - 1;
	sector = basic->GetSectorFromSectorPos(dir_sta_sec);
	for(int sec_pos = dir_sta_sec; sec_pos <= dir_end_sec; sec_pos++) {
		if (!sector) {
			valid = false;
			break;
		}

		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
		dir_cnt++;

		if (p->next_track == 0 && p->next_sector == 0) {
			break;
		}
		sector = basic->GetSector(p->next_track, p->next_sector);
	}
	// DIRエリアの使用セクタが少ない場合
	int dir_end = basic->GetDirStartSector() + dir_cnt - 1;
	if (valid && dir_end < (dir_end_sec + 1)) {
		basic->SetDirEndSector(dir_end);
	}

	return valid;
}

/// ディスクから各パラメータを取得
/// @retval  0 正常
int DiskBasicTypeFLEX::ParseParamOnDisk(DiskD88Disk *disk)
{
	if (!flex_sir) {
		DiskD88Sector *sector = disk->GetSector(0, 0, 3);
		flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer();
		flex_sir = flex;
	}

	if (flex_sir->max_sector > 0) {
		basic->SetSectorsPerTrackOnBasic(flex_sir->max_sector / basic->GetSidesOnBasic());
	}

	return 0;
}

/// 残りディスクサイズを計算
void DiskBasicTypeFLEX::CalcDiskFreeSize()
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	fat_availability.Empty();

	fat_availability.Add(FAT_AVAIL_USED, basic->GetFatEndGroup() + 1);

	// SIR area
//	DiskD88Disk *disk = basic->GetDisk();
	DiskD88Sector *sector = NULL;
//	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer();

	int track_num  = flex_sir->free_start_track;
	int sector_num = flex_sir->free_start_sector;

	while(track_num != 0 || sector_num != 0) {
		sector = basic->GetSector(track_num, sector_num);
		if (!sector) {
			// error
			break;
		}
		int sector_pos = basic->GetSectorPosFromNum(track_num, sector_num);
		if (sector_pos < (int)fat_availability.Count()) {
			fat_availability.Item(sector_pos) = FAT_AVAIL_FREE;
		}

		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
		track_num = p->next_track;
		sector_num = p->next_sector;

		// セクタ先頭4バイトは除く
		fsize += (sector->GetSectorSize() - 4);
		grps++;
	}

	free_disk_size = (int)fsize;
	free_groups = (int)grps;
}

/// シーケンス番号を設定
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeFLEX::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(num);
	if (!sector) {
		// why?
		return;
	}
	flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
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
	int sta_sector_num = flex_sir->free_start_sector;
	if (sta_track_num == 0 && sta_sector_num == 0) {
		// no free space ?
		return INVALID_GROUP_NUMBER;
	}
	// グループ番号を得る
	sector = basic->GetSector(sta_track_num, sta_sector_num);
	if (!sector) {
		// no free space ?
		return INVALID_GROUP_NUMBER;
	}
	group_num = basic->GetSectorPosFromNum(sta_track_num, sta_sector_num);

	// 次のポインタをフリーセクタポインタに設定
	flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
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

/// 次の空き位置を返す 未使用
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
/// @param [in]  item         ディレクトリアイテム
/// @param [in]  data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [out] group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeFLEX::AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items)
{
//	myLog.SetDebug("DiskBasicTypeFLEX::AllocateGroups {");

//	int file_size = data_size;
	int groups = 0; 

	// FAT
	int  rc = 0;
	bool first_group = true;
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	int sizeremain = data_size;
	// 1セクタ当たり4バイトはチェイン用のリンクポインタになるので減算
	int bytes_per_group = basic->GetSectorsPerGroup() * (basic->GetSectorSize() - 4);
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
			item->SetStartGroup(group_num);
			first_group = false;
		}

//		myLog.SetDebug("  group_num:0x%03x", group_num);
		
		basic->GetNumsFromGroup(group_num, 0, basic->GetSectorSize(), sizeremain, group_items);

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
	} else {
		// 確保した領域を削除
		DeleteGroups(group_items);
		// 空き領域をチェインする
		wxUint32 last_group = item->GetLastGroup();
		if (last_group != 0) {
			DiskD88Sector *sector = basic->GetSectorFromGroup(last_group);
			if (sector) {
				flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
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
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(group_num);
	if (!sector) {
		// why?
		return -1;
	}
	flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
	if (!p) {
		// why?
		return -1;
	}
	int next_track_num = 0;
	int next_sector_num = 0;
	basic->GetNumFromSectorPos(append_group_num, next_track_num, next_sector_num);
	p->next_track = (wxUint8)next_track_num;
	p->next_sector = (wxUint8)next_sector_num;

	return 0;
}

/// データの読み込み/比較処理
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファイサイズ
/// @param [in] remain_size   残りサイズ
/// @param [in] sector_num    セクタ番号
/// @param [in] sector_end    最終セクタ番号
/// @return >=0 : 処理したサイズ  -1:比較不一致
int DiskBasicTypeFLEX::AccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
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

/// ルートディレクトリか
bool DiskBasicTypeFLEX::IsRootDirectory(wxUint32 group_num)
{
	return false;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeFLEX::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());

	if (track->GetTrackNumber() > 0 && track->GetSideNumber() < basic->GetSidesOnBasic()) {
		// セクタの先頭にリンクを作成
		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
		if (p) {
			int next_track = track->GetTrackNumber();
			int next_sector = sector->GetSectorNumber() + 1;
			if (next_sector >= (basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic() + 1)) {
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

/// セクタデータを埋めた後の個別処理
bool DiskBasicTypeFLEX::AdditionalProcessOnFormatted()
{
	// SIR area
	DiskD88Sector *sector = basic->GetSector(0, 0, 3);
	if (!sector) return false;
	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer();
	if (!flex) return false;

	flex_sir = flex;

	sector->Fill(0);

	// SIRエリアを設定

	flex_sir->free_start_track = 1;
	flex_sir->free_start_sector = 1;
	flex_sir->free_last_track = (basic->GetTracksPerSide() - 1);
	flex_sir->free_last_sector = (basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic());

	int sector_nums = (basic->GetTracksPerSide() - 1) * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic();

	flex_sir->free_sector_nums = wxUINT16_SWAP_ON_LE(sector_nums);

	flex_sir->max_track = (basic->GetTracksPerSide() - 1);
	flex_sir->max_sector = (basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic());

	struct tm tm;
	wxDateTime::GetTmNow(&tm);
	flex_sir->cmonth = tm.tm_mon + 1;
	flex_sir->cday = tm.tm_mday;
	flex_sir->cyear = (tm.tm_year % 100);

	// DIRエリア

	int sec_end_pos = basic->GetSectorsPerTrackOnBasic() * basic->GetSidesOnBasic() - 1;
	DiskD88Sector *prev_sector = NULL;
	int next_track = 0;
	int next_sector = 0;
	for(int sec_pos = basic->GetDirStartSector() - 1; sec_pos <= sec_end_pos; sec_pos++) {
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec_pos);
		if (prev_sector) {
			flex_ptr_t *p = (flex_ptr_t *)prev_sector->GetSectorBuffer();
			if (p) {
				next_sector = sec_pos + 1;
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
		sector->Fill(0);
	}
	if (prev_sector) {
		flex_ptr_t *p = (flex_ptr_t *)prev_sector->GetSectorBuffer();
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
/// @return 書き込んだバイト数
int DiskBasicTypeFLEX::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
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
			if (remain > 0) buffer[remain - 1]=0x1a;
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

#if 0
/// データの書き込み終了後の処理
void DiskBasicTypeFLEX::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
}

/// ファイル名変更後の処理
void DiskBasicTypeFLEX::AdditionalProcessOnRenamedFile(DiskBasicDirItem *item)
{
}
#endif

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
		sector = basic->GetSector(flex_sir->free_last_track, flex_sir->free_last_sector);
		if (!sector) return false;
		p = (flex_ptr_t *)sector->GetSectorBuffer();
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
		wxUint32 free_group_num = basic->GetSectorPosFromNum(free_track_num, free_sector_num);
		group_items.Add(free_group_num, 0, free_track_num, 0, free_sector_num, 0);

		sector = basic->GetSector(free_track_num, free_sector_num);
		if (!sector) break;
		p = (flex_ptr_t *)sector->GetSectorBuffer();
		if (!p) break;

		free_track_num = p->next_track;
		free_sector_num = p->next_sector;
	}

	// 空き領域をソートしてチェインを作り直す
	group_items.SortItems();
	size_t group_items_count = group_items.Count();
	for(size_t idx = 0; idx < group_items_count; idx++) {
		DiskBasicGroupItem *gitem = &group_items.Item(idx);

		sector = basic->GetSector(gitem->track, gitem->sector_start);
		if (!sector) break;
		p = (flex_ptr_t *)sector->GetSectorBuffer();
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
