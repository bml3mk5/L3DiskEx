/// @file basictype_m68fdos.cpp
///
/// @brief disk basic type for Sord M68 FDOS (KDOS)
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_m68fdos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdiritem_m68fdos.h"
#include "basicdir.h"
#include "../logging.h"


//
//
//
DiskBasicTypeM68FDOS::DiskBasicTypeM68FDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeMZBase(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeM68FDOS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// FATエリア
	DiskImageSector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
	if (!sector) {
		return -1.0;
	}
	if (sector->Get16(0, basic->IsBigEndian()) != 0x003f) {
		valid_ratio = -1.0;
	}

	// トラック１
	sector = basic->GetSector(1, 0, 1);
	if (!sector) {
		return -1.0;
	}
	if (sector->Find("FDDOS", 5) < 0 && sector->Find("SORD", 4) < 0) {
		valid_ratio = -1.0;
	}

	// 使用状況エリア 2セクタ
	for(int i=0; i<2; i++) {
		sector = basic->GetManagedSector(basic->GetFatStartSector() + i);
		if (!sector) {
			return -1.0;
		}
		m_bitmap.AddBuffer(sector->GetSectorBuffer(), sector->GetSectorBufferSize());
	}

	// 最終グループ番号
	if (basic->GetFatEndGroup() == 0) {
		basic->SetFatEndGroup(basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic() - 1);
	}

	return valid_ratio;
}

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
/// @return true
bool DiskBasicTypeM68FDOS::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	group_items.Empty();
	size_t dir_size = 0;
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;
	int div_num = 0;
	int div_nums = 1;
	int sec_pos = start_sector - 1;
	int end_sec_pos = basic->GetFatEndGroup() * basic->GetSectorsPerGroup();
	size_t max_dir_size = end_sec_pos * basic->GetSectorSize();
	DiskImageSector *sector = basic->GetManagedSector(sec_pos, &trk_num, &sid_num, &sec_num, &div_num, &div_nums);
	if (!sector) return false;
	while(dir_size < max_dir_size) {
		group_items.Add(sec_pos, 0, trk_num, sid_num, sec_num, sec_num, div_num, div_nums);
		dir_size += (sector->GetSectorSize() / div_nums);

		// 次のセクタ番号を得る
		sec_pos = sector->Get16(-2, true);
		if (sec_pos <= 0 || sec_pos > end_sec_pos) break;
		sector = basic->GetSectorFromSectorPos(sec_pos, trk_num, sid_num, &div_num, &div_nums);
		if (!sector) break;
		sec_num = sector->GetSectorNumber();
	}
		
	group_items.SetSize(dir_size);
	return true;
}

/// ディレクトリエリアのサイズに達したらアサイン終了するか
/// @param[in,out] pos         ディレクトリの位置
/// @param[in,out] size        ディレクトリのセクタサイズ
/// @param[in,out] size_remain ディレクトリの残りサイズ
/// @retval  0 : 終了しない
/// @retval  1 : 強制的に未使用とする アサインは継続
/// @retval -1 : 現グループでアサイン終了。次のグループから継続
/// @retval -2 : 強制的にアサイン終了する
int DiskBasicTypeM68FDOS::FinishAssigningDirectory(int &pos, int &size, int &size_remain) const
{
	return pos + (int)sizeof(directory_m68fdos_t) > size ? -1 : 0;
}

/// ディレクトリアサインでセクタ毎に位置を調整する
/// @param[in] pos ディレクトリの位置
/// @return 調整後のディレクトリの位置
int DiskBasicTypeM68FDOS::AdjustPositionAssigningDirectory(int pos)
{
	return 0;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeM68FDOS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1 - data_start_group;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
/// @param [in] wrote 書込み操作を行った後か
void DiskBasicTypeM68FDOS::CalcDiskFreeSize(bool wrote)
{
//	int used = 0;
	fat_availability.Empty();

	// 使用済みかチェック
	int grps = 0;
	int fsts;
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup(); gnum++) {
		if (gnum < data_start_group) {
//			used++;
			fsts = FAT_AVAIL_SYSTEM;
		} else if (!IsUsedGroupNumber(gnum)) {
			grps++;
			fsts = FAT_AVAIL_FREE;
		} else {
//			used++;
			fsts = FAT_AVAIL_USED;
		}
		fat_availability.Add(fsts, 0, 0);
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
					fat_availability.Set(gnum, FAT_AVAIL_USED_LAST);
				}
			}
		}
	}

	int fsize = grps * basic->GetSectorsPerGroup() * basic->GetSectorSize();

	fat_availability.SetFreeSize(fsize);
	fat_availability.SetFreeGroups(grps);
}

/// 使用しているグループの位置を得る
void DiskBasicTypeM68FDOS::CalcUsedGroupPos(wxUint32 num, int &pos, int &mask)
{
	mask = 1 << (pos & 7);
	pos = (pos >> 3);
}

/// FAT位置をセット
void DiskBasicTypeM68FDOS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	m_bitmap.Modify(num, val != 0);
}

/// FAT位置を返す
wxUint32 DiskBasicTypeM68FDOS::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// 使用しているグループ番号か
bool DiskBasicTypeM68FDOS::IsUsedGroupNumber(wxUint32 num)
{
	return m_bitmap.IsSet(num);
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeM68FDOS::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return num + 1;
}

/// 空きFAT位置を返す
wxUint32 DiskBasicTypeM68FDOS::GetEmptyGroupNumber()
{
	wxUint32 found = INVALID_GROUP_NUMBER;
	for(wxUint32 grp_num = 0; grp_num <= basic->GetFatEndGroup(); grp_num++) {
		if (!IsUsedGroupNumber(grp_num)) {
			found = grp_num;
			break;
		}
	}
	return found;
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeM68FDOS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	return true;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    確保するデータサイズ（バイト）
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeM68FDOS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int file_size = 0;
	int groups = 0;

	int rc = 0;
	int remain = data_size;
	// セクタ末尾に次のセクタ番号を入れるか
	bool is_chain = item->NeedChainInData();
	int sec_size = basic->GetSectorSize();
	if (is_chain) {
		sec_size -= 2;
	}

	// 必要なグループ数
	wxUint32 group_size = ((data_size - 1) / sec_size / basic->GetSectorsPerGroup()) + 1;
	if (is_chain) {
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

	// データの開始グループ決定
	item->SetStartGroup(fileunit_num, group_start);

	// 領域を確保する
	rc = AllocateGroupsSub(item, group_start, remain, sec_size, group_items, file_size, groups);

	// 確保したグループ数をセット
	item->SetGroupSize(groups);
	// 最終グループをセット
	item->SetExtraGroup(group_items.Last().group);

	return rc;
}

/// グループを確保して使用中にする
int DiskBasicTypeM68FDOS::AllocateGroupsSub(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, DiskBasicGroups &group_items, int &file_size, int &groups)
{
	int rc = 0;
	wxUint32 group_num = group_start;
	wxUint32 prev_group = 0;

//	DiskBasicDirItemM68FDOS *ditem = (DiskBasicDirItemM68FDOS *)item;

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
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @param [in] sector_num    セクタ番号
/// @param [in] sector_end    最終セクタ番号
/// @return >=0 : 処理したサイズ  -1:比較不一致  -2:セクタがおかしい
int DiskBasicTypeM68FDOS::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
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

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeM68FDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskImageSector *sector;

	//
	// FATエリア
	//
	int mng_sec_pos = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	for(int sec = basic->GetFatStartSector() - 1, pos = 0; sec < basic->GetDirStartSector() - 1; sec++, pos++) {
		sector = basic->GetSectorFromSectorPos(mng_sec_pos + sec);
		if (!sector) {
			return false;
		}
		sector->Fill(basic->GetFillCodeOnFAT());
		switch(pos) {
		case 0:
			// first sector
			sector->Copy("\x00\x3f", 2);
			break;
		case 1:
			// set bits
			{
				int n = mng_sec_pos + basic->GetDirEndSector();
				int len = (n >> 3);
				sector->Fill(0xff, len);
				int mod = (n & 7);
				sector->Fill((0xff00 >> mod) & 0xff, 1, len);
			}
			break;
		case 2:
			// set bits
			{
				int n = basic->GetSidesPerDiskOnBasic() * basic->GetTracksPerSideOnBasic() * basic->GetSectorsPerTrackOnBasic();
				n -= (sector->GetSectorSize() * (pos - 1) * 8);
				if (n >= 0 && n < (sector->GetSectorSize() * 8)) {
					int len = (n >> 3);
					sector->Fill(0xff, -1, len);
					int mod = (n & 7);
					sector->Fill(0x00ff >> mod, 1, len);
				}
			}
			break;
		default:
			break;
		}
	}

	//
	// DIRエリア
	// 
	for(int sec = basic->GetDirStartSector() - 1, pos = 0; sec < basic->GetDirEndSector() - 1; sec++, pos++) {
		sector = basic->GetSectorFromSectorPos(mng_sec_pos + sec);
		if (!sector) {
			return false;
		}
		sector->Fill(basic->GetFillCodeOnFAT());
		if (sec < basic->GetDirEndSector() - 2) {
			wxUint16 next = basic->OrderUint16((mng_sec_pos + sec + 1) & 0xffff);
			sector->Copy(&next, 2, basic->GetSectorSize() - 2);
		}
		if (pos == 0) {
			// エントリ１つ
			sector->Copy("\x62\x56\xc1\xc0\xa7\x30\xf9\x80\x00\x00\x00\x5c\x00\x00\x00\x00\x00\x0d\xa4", 19);
		}
	}

	// 
	sector = basic->GetSectorFromSectorPos(mng_sec_pos);
	if (!sector) {
		return false;
	}
	sector->Copy("\xf3\x76NOT FDDOS MEDIA\x00", 18);

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
int DiskBasicTypeM68FDOS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
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

	// 次のセクタ番号を書く
	if (need_chain) {
		wxUint16 next_sector = (wxUint16)next_group * basic->GetSectorsPerGroup();
		if (next_sector >= 0) {
			// bigendien
			buffer[size] = ((next_sector >> 8) & 0xff); 
			buffer[size + 1] = (next_sector & 0xff); 
		}
	}

	return len;
}

/// データの書き込み終了後の処理
void DiskBasicTypeM68FDOS::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
//	DiskBasicDirItemM68FDOS *ditem = (DiskBasicDirItemM68FDOS *)item;
//	ditem->SetUnknownData();
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeM68FDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeM68FDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
}
