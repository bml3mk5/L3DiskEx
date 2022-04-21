/// @file basictype_mz.cpp
///
/// @brief disk basic type for MZ DISK BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_mz.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "logging.h"


#pragma pack(1)
/// MZ 使用状況セクタ
struct st_fat_mz {
	wxUint8 volume;		// ボリューム番号
	wxUint8 offset;		// オフセット データ領域開始クラスタ
	wxUint16 used;		// 使用クラスタ数
	wxUint16 all;		// 全体クラスタ数
	wxUint8 bits[0xf9];	// 使用状況
	wxUint8 mag;		// １クラスタのセクタ数-1
};
#pragma pack()

//
//
//
DiskBasicTypeMZ::DiskBasicTypeMZ(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeMZBase(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMZ::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return -1.0;
	}
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->GetBuffer();
	wxUint8 offset = basic->InvertUint8(b->offset);
	wxUint8 mag = basic->InvertUint8(b->mag);
	// オフセット(1バイト目)が16未満ならエラー
	if (offset < 16) {	// invert
		valid_ratio = -1.0;
	}
	if (valid_ratio >= 0.0) {
		data_start_group = offset;
	}

	// クラスタ倍率(255バイト目)が16以上ならエラー
	if (mag >= 16) {	// invert
		valid_ratio = -1.0;
	}
	if (valid_ratio >= 0.0) {
		// セクタ数/クラスタ
		basic->SetSectorsPerGroup(mag + 1);
		// クラスタ数
		basic->SetFatEndGroup(basic->InvertAndOrderUint16(b->all) - 1);
	} else {
		// クラスタ数はトラック数から計算する
		int trks = basic->GetTracksPerSideOnBasic();
		int secs = 1;
		if (trks > 44) {
			// 2DDのとき
			secs = 2;
			trks /= 2;
		}
		basic->SetSectorsPerGroup(secs);
		basic->SetFatEndGroup(trks * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() - 1);
	}

	return valid_ratio;
}

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicTypeMZ::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool sts = DiskBasicType::AssignRootDirectory(start_sector, end_sector, group_items, dir_item);

	// 開始グループ
	int sec_pos = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() + start_sector - 1;
	sec_pos /= basic->GetSectorsPerGroup();
	dir_item->SetStartGroup(0, sec_pos);

	return sts;
}

/// 残りディスクサイズを計算
/// @param [in] wrote 書込み操作を行った後か
void DiskBasicTypeMZ::CalcDiskFreeSize(bool wrote)
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	int used = 0;
	fat_availability.Empty();

	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return;
	}
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->GetBuffer();
	int used_groups = basic->InvertAndOrderUint16(b->used);	// invert
//	int all_groups = (b->all ^ 0xffff);
//	grps = all_groups - used_groups;
//	fsize = grps * basic->GetSectorsPerGroup() * basic->GetSectorSize();

	// 使用済みかチェック
	int fsts;
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup(); gnum++) {
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

	// ディレクトリエントリのグループ
	const DiskBasicDirItems *items = dir->GetCurrentItems();
	if (items) {
		for(size_t idx = 0; idx < items->Count(); idx++) {
			DiskBasicDirItem *item = items->Item(idx);
			if (!item || !item->IsUsed()) continue;

			// グループ番号のマップを調べる
			size_t gcnt = item->GetGroupCount();
			if (gcnt > 0) {
				const DiskBasicGroupItem *gitem = item->GetGroup(gcnt - 1);
				wxUint32 gnum = gitem->group;
				if (gnum <= basic->GetFatEndGroup()) {
					fat_availability.Item(gnum) = FAT_AVAIL_USED_LAST;
				}
			}
		}
	}

	fsize = grps * basic->GetSectorsPerGroup() * basic->GetSectorSize();

	// 使用済みクラスタ数を更新
	if (wrote && used_groups != used) {
		b->used = basic->InvertAndOrderUint16(used);	// invert
	}

	free_disk_size = (int)fsize;
	free_groups = (int)grps;
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeMZ::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	if (num > basic->GetFatEndGroup()) {
		return;
	}

	int pos = (int)num - (int)data_start_group;
	if (pos < 0) {
		return;
	}

	int mask = 0;
	CalcUsedGroupPos(num, pos, mask);

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return;
	}
	// FATには未使用使用テーブルがある
	fatbuf->Bit((wxUint32)pos, (wxUint8)mask, val != 0, basic->IsDataInverted());

	// FATの使用済み最終クラスタ数を更新
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->GetBuffer();
	wxUint32 used_group = basic->InvertAndOrderUint16(b->used);	// invert
	if (val) {
		// セットした時は最終クラスタ数を増やす
		used_group++;
	} else {
		// クリアした時は最終クラスタ数を減らす
		used_group--;
	}
	b->used = basic->InvertAndOrderUint16(used_group);	// invert

//	myLog.SetDebug("DiskBasicTypeMZ::SetGroupNumber: g:%d v:%d pos:%d msk:%d used:%d"
//		, num, val, pos, mask, used_group);
}

wxUint32 DiskBasicTypeMZ::GetGroupNumber(wxUint32 num) const
{
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return INVALID_GROUP_NUMBER;
	}
	// FATのオフセットを得る
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->GetBuffer();
	wxUint32 offset = basic->InvertUint8(b->offset);	// invert
	if (offset > num) num = offset;
	return num;
}

/// 使用しているグループの位置を得る
void DiskBasicTypeMZ::CalcUsedGroupPos(wxUint32 num, int &pos, int &mask)
{
	mask = 1 << (pos & 7);
	pos = (pos >> 3) + 6;
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
	next_sec = basic->InvertAndOrderUint16(next_sec);	// invert
	return next_sec / basic->GetSectorsPerGroup();	
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeMZ::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return new_num;
	}
	// FATの使用済み最終クラスタを得る
	struct st_fat_mz *b = (struct st_fat_mz *)fatbuf->GetBuffer();
	wxUint32 offset = basic->InvertUint8(b->offset);	// invert
	wxUint32 used_group = basic->InvertAndOrderUint16(b->used);	// invert
	wxUint32 all_group = basic->InvertAndOrderUint16(b->all);	// invert

	if (used_group < all_group) {
		new_num = used_group;
	}
	if (new_num < offset) {
		new_num = offset;
	}
	return new_num;
}

/// データサイズ分のグループを確保する
/// @param [in]  fileunit_num ファイル番号
/// @param [in]  item         ディレクトリアイテム
/// @param [in]  data_size    確保するデータサイズ（バイト）
/// @param [in]  flags        新規か追加か
/// @param [out] group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeMZ::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int file_size = 0;
	int groups = 0;

	int rc = 0;
//	wxUint32 group_num = 0;
	int remain = data_size;
	bool is_chain = item->NeedChainInData();
	bool is_brd = item->GetFileAttr().MatchType(FILE_TYPE_RANDOM_MASK, FILE_TYPE_RANDOM_MASK);
	int sec_size = basic->GetSectorSize();
	if (is_chain) {
		sec_size -= 2;
	}

	// 必要なグループ数
	wxUint32 group_size = ((data_size - 1) / sec_size / basic->GetSectorsPerGroup()) + 1;
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
	item->SetStartGroup(fileunit_num, group_start);

	if (is_brd) {
		// BRD ランダムアクセス
		// マップ領域を確保
		DiskD88Sector *sector = basic->GetSectorFromGroup(group_start);
		if (!sector) {
			// セクタ無い？！
			rc = -1;
			return rc;
		}
		sector->Fill(basic->InvertUint8(0));	// invert

		wxUint16 *brd_maps = NULL;
		brd_maps = (wxUint16 *)sector->GetSectorBuffer();

		SetGroupNumber(group_start, 1);

		int brd_pos = 0;

		do {
			// 連続した16セクタを確保できるかをさがす
			group_size = 16 / basic->GetSectorsPerGroup();
			wxUint32 bcnt = FindContinuousArea(group_size, group_start);
			if (bcnt < group_size) {
				// 十分な空きがない
				rc = -2;
				return rc;
			}

			// 開始セクタ
			wxUint32 sec_pos = group_start * basic->GetSectorsPerGroup();
			sec_pos = basic->InvertAndOrderUint16(sec_pos);	// invert
			brd_maps[brd_pos] = wxUINT16_SWAP_ON_BE(sec_pos);

			// 領域を確保する
			int block_remain = (16 * basic->GetSectorSize());
			int block_size = 0;
			rc = AllocateGroupsSub(item, group_start, block_remain, basic->GetSectorSize(), group_items, block_size, groups);
			if (block_remain >= remain) {
				file_size += (((remain + 31) / 32) * 32);
				break;
			}
			remain -= block_remain;
			file_size += block_remain;
			brd_pos++;
		} while((brd_pos * 2) < basic->GetSectorSize());

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

	int limit = basic->GetFatEndGroup() + 1;
	while(remain > 0 && limit >= 0) {
		// 使用しているか
		bool used_group = IsUsedGroupNumber(group_num);
		if (!used_group) {
			if (prev_group > 0 && prev_group <= basic->GetFatEndGroup()) {
				// 使用済みにする
				basic->GetNumsFromGroup(prev_group, group_num, sec_size, remain, group_items);
				SetGroupNumber(prev_group, 1);
				file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
				groups++;
			}
			remain -= (sec_size * basic->GetSectorsPerGroup());
			prev_group = group_num;
		}
		// 次のグループ
		group_num++;
		limit--;
	}
	if (prev_group > 0 && prev_group <= basic->GetFatEndGroup()) {
		// 使用済みにする
		basic->GetNumsFromGroup(prev_group, 0, sec_size, remain, group_items);
		SetGroupNumber(prev_group, 1);
		file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
		groups++;
	}
	if (prev_group > basic->GetFatEndGroup()) {
		// ファイルがオーバフローしている
		rc = -2;
	} else if (limit < 0) {
		// 無限ループ？
		rc = -2;
	}
	return rc;
}

/// データの読み込み/比較処理
/// @param [in] fileunit_num  ファイル番号
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファイサイズ
/// @param [in] remain_size   残りサイズ
/// @param [in] sector_num    セクタ番号
/// @param [in] sector_end    最終セクタ番号
/// @return >=0 : 処理したサイズ  -1:比較不一致  -2:セクタがおかしい  
int DiskBasicTypeMZ::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	bool need_chain = item->NeedChainInData();

	if (need_chain) {
		// セクタの最終バイトはチェイン用セクタ番号がある
		sector_size -= 2;
	}

	int size = (remain_size < sector_size ? remain_size : sector_size);

	if (ostream) {
		// 書き出し
		temp.SetData(sector_buffer, size, basic->IsDataInverted());

		ostream->Write((const void *)temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read((void *)temp.GetData(), temp.GetSize());
		temp.InvertData(basic->IsDataInverted());

		if (memcmp(temp.GetData(), sector_buffer, temp.GetSize()) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}

/// ルートディレクトリか
bool DiskBasicTypeMZ::IsRootDirectory(wxUint32 group_num)
{
	// オフセット未満だったらルート
	return ((wxUint32)(basic->InvertUint8(fat->Get(1))) > group_num);	// invert
}

/// サブディレクトリを作成する前にディレクトリ名を編集する
bool DiskBasicTypeMZ::RenameOnMakingDirectory(wxString &dir_name)
{
	// 空や"."で始まるディレクトリは作成不可
	if (dir_name.IsEmpty() || dir_name.Left(1) == wxT(".")) {
		return false;
	}
	return true;
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeMZ::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	if (group_items.Count() <= 0) return;

	// ボリューム番号、カレントと親ディレクトリのエントリを作成する
	DiskBasicGroupItem *gitem = &group_items.Item(0);

	DiskD88Sector *sector = basic->GetDisk()->GetSector(gitem->track, gitem->side, gitem->sector_start);

	wxUint8 *buf = sector->GetSectorBuffer();
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, 0, buf);

	// ボリューム番号
	newitem->CopyData(item->GetData());
	newitem->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_VOLUME_MASK | FILE_TYPE_READONLY_MASK);

	buf += newitem->GetDataSize();
	newitem->SetDataPtr(0, 0, 0, sector, 0, buf);

	// カレント
	newitem->CopyData(item->GetData());
	newitem->SetFileNamePlain(wxT("."));
	newitem->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_READONLY_MASK);

	buf += newitem->GetDataSize();
	newitem->SetDataPtr(0, 0, 0, sector, 0, buf);

	// 親
	if (parent_item) {
		// 親がサブディレクトリ
		newitem->CopyData(parent_item->GetData());
	} else {
		// 親がルート
		newitem->CopyData(item->GetData());
		newitem->SetStartGroup(0, 0);
	}
	newitem->SetFileNamePlain(wxT(".."));
	newitem->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_READONLY_MASK);

	delete newitem;

}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeMZ::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// IPL
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(0);
	if (sector) {
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));	// invert
		wxUint8 *buf = sector->GetSectorBuffer();
		if (buf) {
			wxCharBuffer ipl = basic->GetVariousStringParam(wxT("IPLString")).To8BitData();
			size_t len = ipl.length();
			if (len > 0) {
				if (len > 32) len = 32;
				basic->InvertMem((const wxUint8 *)ipl.data(), len, buf);
			}
		}
	}

	wxUint8 volume_number = 1;
	wxString volume_string = basic->GetVariousStringParam(wxT("VolumeString"));
	int volume_length = (int)volume_string.Length();
	if (volume_length > 0) {
		volume_number = volume_string[0];
	}

	// FATエリア
	DiskBasicFatArea *fats = fat->GetDiskBasicFatArea();
	for(size_t n = 0; n < fats->Count(); n++) {
		DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(n, 0);
		wxUint8 *buf = fatbuf->GetBuffer();
		int size = (int)fatbuf->GetSize();

		memset(buf, basic->GetFillCodeOnFAT(), size);

		struct st_fat_mz *fdat = (struct st_fat_mz *)buf;

		fdat->volume = volume_number;
		// トラック1 サイド1 から
		fdat->offset = GetSectorPosFromNum(1, 1, 1) / basic->GetSectorsPerGroup();
		// 使用クラスタ数
		fdat->used = fdat->offset;
		// 最大クラスタ数
//		fdat->all = basic->GetFatEndGroup() + 1;
		fdat->all = basic->GetTracksPerSide() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic() / basic->GetSectorsPerGroup();
		// セクタ数/グループ
		fdat->mag = basic->GetSectorsPerGroup() - 1;

		// invert
		basic->InvertMem(buf, size);
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
		GetNumFromSectorPos(sec_pos - 1, trk_num, sid_num, sec_num);
		sector = basic->GetSector(trk_num, sid_num, sec_num);
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
			basic->InvertMem(buf, sector->GetSectorBufferSize());
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
		wxUint16 next_sector = group_num * basic->GetSectorsPerGroup();
		// 次のデータがあるセクタ番号を入れる
		if (sector_num < sector_end) {
			next_sector++;
		} else {
			next_sector = (remain > size ? next_group * basic->GetSectorsPerGroup() : 0);
		}
		*((wxUint16 *)&buffer[size]) = basic->InvertAndOrderUint16(next_sector);
	}

	// 反転
	basic->InvertMem(buffer, size);

	return len;
}

/// データの書き込み終了後の処理
void DiskBasicTypeMZ::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	if (!item || !item->GetFileAttr().IsDirectory()) return;

	// ディレクトリの場合は、下位にあるボリューム名も変更する
	DiskD88Sector *sector = basic->GetSectorFromGroup(item->GetStartGroup(0));

	wxUint8 *buf = sector->GetSectorBuffer();
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, 0, buf);

	// ボリューム名をコピー
	if (!newitem->IsSameFileName(item)) {
		newitem->CopyFileName(*item);
	}

	delete newitem;
}

/// ファイル名変更後の処理
void DiskBasicTypeMZ::AdditionalProcessOnRenamedFile(DiskBasicDirItem *item)
{
	AdditionalProcessOnSavedFile(item);
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeMZ::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// ルートディレクトリのボリューム番号
	DiskBasicDirItem *ditem = dir->FindFileByAttrOnRoot(FILE_TYPE_VOLUME_MASK, FILE_TYPE_VOLUME_MASK | FILE_TYPE_DIRECTORY_MASK);
	if (ditem) {
		wxUint8 buf[8];
		buf[0] = 0;
		ditem->GetFileName(buf, sizeof(buf));
		data.SetVolumeNumber(buf[0]);
	}
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeMZ::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
}
