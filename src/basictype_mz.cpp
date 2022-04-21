/// @file basictype_mz.cpp
///
/// @brief disk basic fat type for MZ DISK BASIC
///
#include "basictype_mz.h"
#include "basicfmt.h"
#include "basicdiritem.h"
#include "logging.h"

/// MZ 使用状況セクタ
struct st_fat_mz {
	wxUint8 volume;		// ボリューム番号
	wxUint8 offset;		// オフセット データ領域開始クラスタ
	wxUint16 used;		// 使用クラスタ数
	wxUint16 all;		// 全体クラスタ数
	wxUint8 bits[0xf9];	// 使用状況
	wxUint8 mag;		// １クラスタのセクタ数-1
};

//
//
//
DiskBasicTypeMZ::DiskBasicTypeMZ(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// FATエリアをチェック
bool DiskBasicTypeMZ::CheckFat()
{
	bool valid = true;

	// FATエリア
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	DiskBasicFatBuffers *fatbufs = &bufs->Item(0);
	DiskBasicFatBuffer *fatbuf = &fatbufs->Item(0);
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->buffer;
	// オフセット(1バイト目)が16未満ならエラー
	if ((b->offset ^ 0xff) < 16) {	// invert
		valid = false;
	}
	if (valid) {
		data_start_group = (b->offset ^ 0xff);
	}

	// クラスタ倍率(255バイト目)が16以上ならエラー
	if ((b->mag ^ 0xff) >= 16) {	// invert
		valid = false;
	}
	if (valid) {
		secs_per_group = (b->mag ^ 0xff) + 1;
		basic->SetSectorsPerGroup(secs_per_group);
	}

	return valid;
}

/// 残りディスクサイズを計算
void DiskBasicTypeMZ::CalcDiskFreeSize()
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	int used = 0;
	fat_availability.Empty();

	// FATエリア
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	DiskBasicFatBuffers *fatbufs = &bufs->Item(0);
	DiskBasicFatBuffer *fatbuf = &fatbufs->Item(0);
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->buffer;
	int used_groups = (b->used ^ 0xffff);
//	int all_groups = (b->all ^ 0xffff);
//	grps = all_groups - used_groups;
//	fsize = grps * secs_per_group * sector_size;

	// 使用済みかチェック
	int fsts;
	for(wxUint32 gnum = 0; gnum <= end_group; gnum++) {
		if (gnum < data_start_group) {
			used++;
			fsts = FAT_AVAIL_SYSTEM;
		} else if (!IsUsedGroupNumber(gnum)) {
			grps++;
			fsts = FAT_AVAIL_FREE;
		} else {
			used++;
			fsts = FAT_AVAIL_USED;
		}
		fat_availability.Add(fsts);
	}

	fsize = grps * secs_per_group * sector_size;
	// 使用済みクラスタ数を更新
	if (used_groups != used) {
		b->used = (used ^ 0xffff);	// invert
	}

	free_disk_size = (int)fsize;
	free_groups = (int)grps;
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeMZ::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	if (num > end_group) {
		return;
	}

	int pos = num - data_start_group;
	if (pos < 0) {
		return;
	}

	pos = (pos >> 3) + 6;
	int mask = 1 << (num & 7);

	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	DiskBasicFatBuffers *fatbufs = &bufs->Item(0);
	DiskBasicFatBuffer *fatbuf = &fatbufs->Item(0);
	// FATには未使用使用テーブルがある
	if (pos < fatbuf->size) {
		wxUint8 bit = (fatbuf->buffer[pos] ^ 0xff);	// invert
		bit = (val ? (bit | mask) : (bit & ~mask));
		fatbuf->buffer[pos] = (bit ^ 0xff);	// invert
	}

	// FATの使用済み最終クラスタ数を更新
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->buffer;
	wxUint32 used_group = (b->used ^ 0xffff);	// invert
	if (val) {
		// セットした時は最終クラスタ数を増やす
		used_group++;
	} else {
		// クリアした時は最終クラスタ数を減らす
		used_group--;
	}
	b->used = (used_group ^ 0xffff);	// invert

	myLog.SetDebug("DiskBasicTypeMZ::SetGroupNumber: g:%d v:%d pos:%d msk:%d used:%d"
		, num, val, pos, mask, used_group);
}

wxUint32 DiskBasicTypeMZ::GetGroupNumber(wxUint32 num)
{
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	DiskBasicFatBuffers *fatbufs = &bufs->Item(0);
	DiskBasicFatBuffer *fatbuf = &fatbufs->Item(0);
	// FATのオフセットを得る
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->buffer;
	wxUint32 offset = (b->offset ^ 0xff);	// invert
	if (offset > num) num = offset;
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeMZ::IsUsedGroupNumber(wxUint32 num)
{
	bool exist = false;

	if (num > end_group) {
		return false;
	}

	int pos = num - data_start_group;
	if (pos < 0) {
		// システムエリアは使用済み
		return true;
	}

	pos = (pos >> 3) + 6;
	int mask = 1 << (num & 7);

	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	DiskBasicFatBuffers *fatbufs = &bufs->Item(0);
	DiskBasicFatBuffer *fatbuf = &fatbufs->Item(0);
	// FATには未使用使用テーブルがある
	if (pos < fatbuf->size) {
		if ((fatbuf->buffer[pos] ^ 0xff) & mask) {	// invert
			exist = true;
		}
	}
	return exist;
}

/// 次のグループ番号を得る
/// 属性がBSDの時のみ
wxUint32 DiskBasicTypeMZ::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	int trk_num, sid_num, sec_num;
	basic->CalcNumFromSectorPosForGroup(sector_pos, trk_num, sid_num, sec_num);
	DiskD88Sector *sector = basic->GetDisk()->GetSector(trk_num, sid_num, sec_num);
	if (!sector) return 0;
	wxUint16 next_sec = *(wxUint16 *)(&sector->GetSectorBuffer()[sector->GetSectorSize()-2]);
	next_sec ^= 0xffff;	// invert
	return next_sec / secs_per_group;	
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeMZ::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	DiskBasicFatBuffers *fatbufs = &bufs->Item(0);
	DiskBasicFatBuffer *fatbuf = &fatbufs->Item(0);
	// FATの使用済み最終クラスタを得る
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->buffer;
	wxUint32 offset = (b->offset ^ 0xff);	// invert
	wxUint32 used_group = (b->used ^ 0xffff);	// invert
	wxUint32 all_group = (b->all ^ 0xffff);	// invert

	if (used_group < all_group) {
		new_num = used_group;
	}
	if (new_num < offset) {
		new_num = offset;
	}
	return new_num;
}

/// 次の空き位置を返す 未使用
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeMZ::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	return INVALID_GROUP_NUMBER;
}

/// 未使用が連続している位置をさがす
wxUint32 DiskBasicTypeMZ::FindContinuousArea(wxUint32 group_size, wxUint32 &group_start)
{
	// 未使用が連続している位置をさがす
	wxUint32 cnt = 0;
	for(wxUint32 gnum = GetGroupNumber(0); gnum <= end_group && cnt < group_size; gnum++) {
		if (!IsUsedGroupNumber(gnum)) {
			if (cnt == 0) {
				group_start = gnum;
			}
			cnt++;
		} else {
			cnt = 0;
		}
	}
	return cnt;
}

/// データサイズ分のグループを確保する
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeMZ::AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items)
{
	int file_size = 0;
	int groups = 0;

	int rc = 0;
//	wxUint32 group_num = 0;
	int remain = data_size;
	bool is_chain = item->NeedChainInData();
	bool is_brd = ((item->GetFileType() & FILE_TYPE_RANDOM_MASK) != 0);
	int sec_size = sector_size;
	if (is_chain) {
		sec_size -= 2;
	}

	// 必要なグループ数
	wxUint32 group_size = ((data_size - 1) / sec_size / secs_per_group) + 1;
	if (is_chain || is_brd) {
		group_size = 1;
	}

	// 未使用が連続している位置をさがす
	wxUint32 group_start;
	wxUint32 cnt = FindContinuousArea(group_size, group_start);
	if (cnt < group_size) {
		// 十分な空きがない
		rc = -1;
		return rc;
	}

	// 開始グループ決定
	item->SetStartGroup(group_start);

	if (is_brd) {
		// BRD ランダムアクセス
		// マップ領域を確保
		DiskD88Sector *sector = basic->GetSectorFromGroup(group_start);
		if (!sector) {
			// セクタ無い？！
			rc = -1;
			return rc;
		}
		sector->Fill(0xff);	// invert

		wxUint16 *brd_maps = NULL;
		brd_maps = (wxUint16 *)sector->GetSectorBuffer();

		SetGroupNumber(group_start, 1);

		int brd_pos = 0;

		do {
			// 連続した16セクタを確保できるかをさがす
			group_size = 16 / secs_per_group;
			wxUint32 bcnt = FindContinuousArea(group_size, group_start);
			if (bcnt < group_size) {
				// 十分な空きがない
				rc = -2;
				return rc;
			}

			// 開始セクタ
			wxUint32 sec_pos = group_start * secs_per_group;
			sec_pos ^= 0xffff;	// invert
			brd_maps[brd_pos] = wxUINT16_SWAP_ON_BE(sec_pos);

			// 領域を確保する
			int block_remain = (16 * sector_size);
			int block_size = 0;
			rc = AllocateGroupsSub(item, group_start, block_remain, sector_size, group_items, block_size, groups);
			if (block_remain >= remain) {
				file_size += (((remain + 31) / 32) * 32);
				break;
			}
			remain -= block_remain;
			file_size += block_remain;
			brd_pos++;
		} while((brd_pos * 2) < sector_size);

	} else {
		// BRD 以外

		// 領域を確保する
		rc = AllocateGroupsSub(item, group_start, remain, sec_size, group_items, file_size, groups);
	}
	return rc;
}

/// グループを確保して使用中にする
int DiskBasicTypeMZ::AllocateGroupsSub(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, DiskBasicGroups &group_items, int &file_size, int &groups)
{
	int rc = 0;
	wxUint32 group_num = group_start;
	wxUint32 prev_group = 0;

	int limit = end_group + 1;
	while(remain > 0 && limit >= 0) {
		// 使用しているか
		bool used_group = IsUsedGroupNumber(group_num);
		if (!used_group) {
			if (prev_group > 0 && prev_group <= end_group) {
				// 使用済みにする
				basic->GetNumsFromGroup(prev_group, group_num, sec_size, remain, group_items);
				SetGroupNumber(prev_group, 1);
				file_size += (sector_size * secs_per_group);
				groups++;
			}
			remain -= (sec_size * secs_per_group);
			prev_group = group_num;
		}
		// 次のグループ
		group_num++;
		limit--;
	}
	if (prev_group > 0 && prev_group <= end_group) {
		// 使用済みにする
		basic->GetNumsFromGroup(prev_group, 0, sec_size, remain, group_items);
		SetGroupNumber(prev_group, 1);
		file_size += (sector_size * secs_per_group);
		groups++;
	}
	if (prev_group > end_group) {
		// ファイルがオーバフローしている
		rc = -2;
	} else if (limit < 0) {
		// 無限ループ？
		rc = -2;
	}
	return rc;
}

///// ファイルの最終セクタのデータサイズを求める
//int DiskBasicTypeMZ::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
//{
//	return remain_size;
//}

/// データの読み込み/比較処理
/// @return >=0 : 処理したサイズ  -1:比較不一致
int DiskBasicTypeMZ::AccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	bool need_chain = item->NeedChainInData();

	if (need_chain) {
		// セクタの最終バイトはチェイン用セクタ番号がある
		sector_size -= 2;
	}

	int size = (remain_size < sector_size ? remain_size : sector_size);

	if (ostream) {
		// 書き出し
		temp.SetData(sector_buffer, size);

		mem_invert(temp.GetData(), temp.GetSize());

		ostream->Write((const void *)temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read((void *)temp.GetData(), temp.GetSize());

		mem_invert(temp.GetData(), temp.GetSize());

		if (memcmp(temp.GetData(), sector_buffer, temp.GetSize()) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeMZ::GetStartSectorFromGroup(wxUint32 group_num)
{
	// MZ
	return group_num * secs_per_group;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeMZ::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	// MZ
	int end_sector = sector_start + secs_per_group - 1;
	return end_sector;
}

/// サイド番号を逆転するか
bool DiskBasicTypeMZ::IsSideReversed(int sides_per_disk)
{
	return (sides_per_disk > 1);
}

/// ディスク内のデータが反転しているか
bool DiskBasicTypeMZ::IsDataInverted()
{
	return true;
}

/// ルートディレクトリか
bool DiskBasicTypeMZ::IsRootDirectory(wxUint32 group_num)
{
	// オフセット未満だったらルート
	return ((wxUint32)(fat->Get(1) ^ 0xff) > group_num);	// invert
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeMZ::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item, wxUint32 parent_group_num)
{
	if (group_items.Count() <= 0) return;

	// ボリューム番号、カレントと親ディレクトリのエントリを作成する
	DiskBasicGroupItem *gitem = &group_items.Item(0);

	DiskD88Sector *sector = basic->GetDisk()->GetSector(gitem->track, gitem->side, gitem->sector_start);

	wxUint8 *buf = sector->GetSectorBuffer();
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, buf);

	// ボリューム番号
	newitem->CopyData(item->GetData());
	newitem->SetFileAttr(FILE_TYPE_VOLUME_MASK | FILE_TYPE_READONLY_MASK);

	buf += newitem->GetDataSize();
	newitem->SetDataPtr((directory_t *)buf);

	// カレント
	newitem->CopyData(item->GetData());
	newitem->SetFileNamePlain(wxT("."));
	newitem->SetFileAttr(FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_READONLY_MASK);

	buf += newitem->GetDataSize();
	newitem->SetDataPtr((directory_t *)buf);

	// 親
	if (parent_item) {
		// 親がサブディレクトリ
		newitem->CopyData(parent_item->GetData());
	} else {
		// 親がルート
		newitem->CopyData(item->GetData());
		newitem->SetStartGroup(parent_group_num);
	}
	newitem->SetFileNamePlain(wxT(".."));
	newitem->SetFileAttr(FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_READONLY_MASK);

	delete newitem;

}

/// セクタデータを指定コードで埋める
void DiskBasicTypeMZ::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat() ^ 0xff);
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
void DiskBasicTypeMZ::AdditionalProcessOnFormatted()
{
	// IPL
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(0);
	if (sector) {
		sector->Fill(basic->GetFillCodeOnFAT() ^ 0xff);	// invert
		int len = (int)basic->GetIPLString().Length();
		if (len > 0) {
			wxUint8 buf[16];
			if (len > 16) len = 16;
			memcpy(buf, basic->GetIPLString().To8BitData(), len);
			mem_invert(buf, len);
			sector->Copy(buf, len);
		}
	}

	wxUint8 volume_number = 1;
	wxString volume_string = basic->GetVolumeString();
	int volume_length = (int)volume_string.Length();
	if (volume_length > 0) {
		volume_number = volume_string[0];
	}

	// FATエリア
	DiskBasicFatArea *fats = fat->GetDiskBasicFatArea();
	for(size_t n = 0; n < fats->Count(); n++) {
		DiskBasicFatBuffers *fatbufs = &fats->Item(n);
		DiskBasicFatBuffer *fatbuf = &fatbufs->Item(0);
		wxUint8 *buf = fatbuf->buffer;
		int size = fatbuf->size;

		memset(buf, basic->GetFillCodeOnFAT(), size);

		struct st_fat_mz *fdat = (struct st_fat_mz *)buf;

		fdat->volume = volume_number;
		// トラック1 サイド1 から
		fdat->offset = basic->GetSectorPosFromNum(1, 1, 1) / basic->GetSectorsPerGroup();
		// 使用クラスタ数
		fdat->used = fdat->offset;
		// 最大クラスタ数
		fdat->all = basic->GetFatEndGroup() + 1;
		// セクタ数/グループ
		fdat->mag = basic->GetSectorsPerGroup() - 1;

		// invert
		mem_invert(buf, size);
	}

	// DIRエリア
	directory_mz_t ditv, ditm;
	memset(&ditv, 0, sizeof(ditv));
	ditv.type = 0x80;	// VOL
	memset(ditv.name, 0x0d, sizeof(ditv.name));
	if (volume_length > 0) {
		memcpy(ditv.name, volume_string.To8BitData(), volume_length);
	}
	memset(&ditm, 0, sizeof(ditm));
	memset(ditm.name, 0x0d, sizeof(ditm.name));

	int trk_num, sid_num, sec_num;
	int index = 0;
	for (int sec_pos = basic->GetDirStartSector(); sec_pos <= basic->GetDirEndSector(); sec_pos++) {
		basic->GetNumFromSectorPos(sec_pos - 1, trk_num, sid_num, sec_num);
		sector = basic->GetDisk()->GetSector(trk_num, sid_num, sec_num);
		if (sector) {
			wxUint8 *buf = sector->GetSectorBuffer();
			int pos = 0;
			while(pos < sector->GetSectorBufferSize()) {
				if (index == 0) {
					// 先頭にはボリューム番号を設定
					memcpy(&buf[pos], &ditv, sizeof(ditv));
				} else {
					memcpy(&buf[pos], &ditm, sizeof(ditm));
				}
				pos += (int)sizeof(ditm);
				index++;
			}
			// invert
			mem_invert(buf, sector->GetSectorBufferSize());
		}
	}
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
int DiskBasicTypeMZ::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	bool need_chain = item->NeedChainInData();

	int len = 0;
	if (need_chain) {
		size -= 2;
	}

	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (remain > 0) {
			istream.Read((void *)buffer, remain);
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

	// チェーン用のセクタ番号を書く
	if (need_chain) {
		wxUint16 next_sector = group_num * secs_per_group;
		// 次のデータがあるセクタ番号を入れる
		if (sector_num < sector_end) {
			next_sector++;
		} else {
			next_sector = (remain > size ? next_group * secs_per_group : 0);
		}
		*((wxUint16 *)&buffer[size]) = (next_sector ^ 0xffff);
	}

	// 反転
	mem_invert(buffer, size);

	return len;
}

/// データの書き込み終了後の処理
void DiskBasicTypeMZ::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	if (!item || (item->GetFileType() & FILE_TYPE_DIRECTORY_MASK) == 0) return;

	// ディレクトリの場合は、下位にあるボリューム名も変更する
	DiskD88Sector *sector = basic->GetSectorFromGroup(item->GetStartGroup());

	wxUint8 *buf = sector->GetSectorBuffer();
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, buf);

	// ボリューム名をコピー
	if (!newitem->IsSameFileName(*item)) {
		newitem->CopyFileName(*item);
	}

	delete newitem;
}

/// ファイル名変更後の処理
void DiskBasicTypeMZ::AdditionalProcessOnRenamedFile(DiskBasicDirItem *item)
{
	AdditionalProcessOnSavedFile(item);
}

/// FAT領域を削除する
void DiskBasicTypeMZ::DeleteGroupNumber(wxUint32 group_num)
{
	// FATを未使用にする
	SetGroupNumber(group_num, 0);
}
