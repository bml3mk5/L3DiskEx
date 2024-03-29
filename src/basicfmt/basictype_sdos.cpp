/// @file basictype_sdos.cpp
///
/// @brief disk basic type for S-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_sdos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_sdos.h"


//
//
//
DiskBasicTypeSDOS::DiskBasicTypeSDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
	m_empty_group_num = 0;
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeSDOS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
}
/// FAT位置を返す
/// @param [in] num グループ番号(0...)
wxUint32 DiskBasicTypeSDOS::GetGroupNumber(wxUint32 num) const
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return 0xff:空きなし
wxUint32 DiskBasicTypeSDOS::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	if (m_empty_group_num <= basic->GetFatEndGroup()) {
		new_num = m_empty_group_num;
	}

	return new_num;
}

/// 次の空きFAT位置を返す
wxUint32 DiskBasicTypeSDOS::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	curr_group++;
	if (curr_group <= basic->GetFatEndGroup()) {
		new_num = curr_group;
	}

	return new_num;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeSDOS::ParseParamOnDisk(bool is_formatting)
{
	// グループ数
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		basic->SetFatEndGroup(end_group - 1);
	}
	return 1.0;
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeSDOS::CheckFat(bool is_formatting)
{
	double valid_ratio = -1.0;

	// 最初のセクタにある文字列で判断
	DiskImageSector *sector = basic->GetSectorFromSectorPos(0);
	if (sector) {
		wxCharBuffer id = basic->GetVariousStringParam(wxT("IPLCompareString")).To8BitData();
		if (id.length() > 0) {
			if (sector->Find(id.data(), id.length()) >= 0) {
				valid_ratio = 1.0;
			}
		}
	}
	return valid_ratio;
}

/// ディレクトリエリアのサイズに達したらアサイン終了するか
/// @param[in,out] pos         ディレクトリの位置
/// @param[in,out] size        ディレクトリのセクタサイズ
/// @param[in,out] size_remain ディレクトリの残りサイズ
/// @retval  0 : 終了しない
/// @retval -2 : 強制的に終了
int DiskBasicTypeSDOS::FinishAssigningDirectory(int &pos, int &size, int &size_remain) const
{
	// サイズに達したら終了
	return (size_remain < (int)sizeof(directory_sdos_t) ? -2 : 0);
}

/// 使用可能なディスクサイズを得る
/// @param [out] disk_size  ディスクサイズ
/// @param [out] group_size グループ数
void DiskBasicTypeSDOS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = 0;
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		if (gnum != basic->GetGroupSystemCode()) group_size++;
	}
	disk_size = group_size * basic->GetSectorSize() / basic->GetGroupsPerSector();
}

/// 残りディスクサイズを計算
void DiskBasicTypeSDOS::CalcDiskFreeSize(bool wrote)
{
	fat_availability.Empty();
	fat_availability.SetCount(basic->GetFatEndGroup() + 1, FAT_AVAIL_FREE);

	wxUint32 gnum;
	wxUint32 max_gnum = 0;

	// ディレクトリエントリのグループ
	const DiskBasicDirItems *items = dir->GetCurrentItems();
	for(size_t idx = 0; idx < items->Count(); idx++) {
		DiskBasicDirItem *item = items->Item(idx);
		if (!item || !item->IsUsed()) continue;
		// 開始グループ
		wxUint32 sgnum = item->GetStartGroup(0);

		// グループ番号のマップを調べる
		size_t gcnt = item->GetGroupCount();
		for(size_t gidx = 0; gidx < gcnt; gidx++) {
			const DiskBasicGroupItem *gitem = item->GetGroup(gidx);
			gnum = gitem->group;
			fat_availability.Set(gnum, (gidx == gcnt - 1 ? FAT_AVAIL_USED_LAST : FAT_AVAIL_USED));
		}

		if (sgnum + (wxUint32)gcnt > max_gnum) {
			max_gnum = sgnum + (wxUint32)gcnt;
		}
	}

	// 空きをチェック
	int grps = 0;
	wxUint32 gend = basic->GetReservedSectors();
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		if (pos < gend) {
			// ディレクトリエリアは使用済み
			fat_availability.Set(pos, FAT_AVAIL_SYSTEM);
		} else if (fat_availability.Get(pos) == FAT_AVAIL_FREE) {
			if (pos < max_gnum) {
				// 削除されたエリア
				fat_availability.Set(pos, FAT_AVAIL_LEAK);
			} else {
				// 空き
				grps++;
			}
		}
	}

	m_empty_group_num = max_gnum;

	int fsize = grps * basic->GetSectorSize() * basic->GetSectorsPerGroup();

	fat_availability.SetFreeSize(fsize);
	fat_availability.SetFreeGroups(grps);
}

#if 0
/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeSDOS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	return true;
}
#endif

/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    確保するデータサイズ（バイト）
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeSDOS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int file_size = 0;
	int groups = 0;

	int rc = 0;
	int sec_size = basic->GetSectorSize();
	int remain = data_size;
	int limit = basic->GetFatEndGroup() + 1;
	wxUint32 group_num = GetEmptyGroupNumber();
	while(remain > 0 && limit >= 0 && group_num != INVALID_GROUP_NUMBER) {
		basic->GetNumsFromGroup(group_num, 0, sec_size, remain, group_items);

		file_size += (sec_size * basic->GetSectorsPerGroup());
		groups++;
		remain -= (sec_size * basic->GetSectorsPerGroup());

		group_num = GetNextEmptyGroupNumber(group_num);

		limit--;
	}
	if (group_num == INVALID_GROUP_NUMBER || limit < 0) {
		// 空きなし or 無限ループ？
		rc = -1;
	}

	if (rc == 0) {
		// 最初のグループをセット
		if (group_items.Count() > 0) {
			item->SetStartGroup(fileunit_num, group_items.Item(0).group);
		}
	}
	return rc;
}

/// グループ番号から開始セクタ番号を得る
/// @param [in] group_num グループ番号
/// @return 開始セクタ番号
int DiskBasicTypeSDOS::GetStartSectorFromGroup(wxUint32 group_num)
{
	return (int)group_num;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeSDOS::FillSector(DiskImageTrack *track, DiskImageSector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeSDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	return true;
}

//
// for write
//

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
int DiskBasicTypeSDOS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	bool need_eof_code = item->NeedCheckEofCode();

	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (need_eof_code) {
			// 最終は終端コード
			if (remain > 1) istream.Read((void *)buffer, remain - 1);
			if (remain > 0) buffer[remain - 1]=basic->GetTextTerminateCode();
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
		if (need_eof_code && remain == size + 1) {
			// のこりが終端コードだけなら終端コードを出さずここで終了
			len++;
		}
	}
	// 反転
	basic->InvertMem(buffer, size);

	return len;
}

/// ファイル削除後の処理
bool DiskBasicTypeSDOS::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// 削除したディレクトリエントリ以降をシフトする
	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) return true;

	DiskBasicDirItems *children = parent->GetChildren();
	if (!children) return true;

	bool shift = false;
	DiskBasicDirItem *prev = NULL;
	for(size_t i=0; i<children->Count(); i++) {
		DiskBasicDirItem *curr = children->Item(i);
		if (shift && prev && curr) {
			prev->CopyItem(*curr);
		}
		if (curr == item) {
			shift = true;
		}
		prev = curr;
	}

	return true;
}
