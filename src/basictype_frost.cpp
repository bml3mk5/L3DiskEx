/// @file basictype_frost.cpp
///
/// @brief disk basic type for Frost-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_frost.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_frost.h"
#include "charcodes.h"


//
//
//
DiskBasicTypeFROST::DiskBasicTypeFROST(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT8(basic, fat, dir)
{
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeFROST::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	// 16bit
	fat->GetDiskBasicFatArea()->SetData16BE(num, ConvTrackSectorFromSectorPos(val));
}
/// FAT位置を返す
/// @param [in] num グループ番号(0...)
wxUint32 DiskBasicTypeFROST::GetGroupNumber(wxUint32 num) const
{
	// 16bit
	return ConvSectorPosFromTrackSector(fat->GetDiskBasicFatArea()->GetData16BE(0, num));
}

/// 次の空きFAT位置を返す
/// @param [in] curr_group グループ番号(0...)
/// @return INVALID_GROUP_NUMBER 空きなし
wxUint32 DiskBasicTypeFROST::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	// 現在の番号と連続するように検索
	bool found = false;
	for(int i=0; i<2 && !found; i++) {
		wxUint32 sgnum = (i == 0 ? curr_group + 1 : 0);
		for(wxUint32 gnum = sgnum; gnum <= basic->GetFatEndGroup(); gnum++) {
			wxUint32 next_gnum = GetGroupNumber(gnum);
			if (next_gnum == basic->GetGroupUnusedCode()) {
				new_num = gnum;
				found = true;
				break;
			}
		}
	}
	return new_num;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] disk          ディスク
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0 正常
double DiskBasicTypeFROST::ParseParamOnDisk(DiskD88Disk *disk, bool is_formatting)
{
	// １トラック当たりのグループ数を計算する
	if (basic->GetGroupsPerTrack() == 0) {
		// 512バイトを１グループとして計算する
		int cnt = 0;
		DiskD88Track *track = basic->GetTrack(1, 0);
		if (track) {
			DiskD88Sectors *secs = track->GetSectors();
			if (secs) {
				for(size_t sec = 0; sec < secs->Count(); sec++) {
					int siz = secs->Item(sec)->GetSectorSize();
					cnt += (siz / FROST_GROUP_SIZE);
				}
			}
		}
		if (cnt == 0) {
			cnt = 11;
		}
		basic->SetGroupsPerTrack(cnt);
	}
	// １セクタ当たりのグループ数
	int grps_per_sec = (basic->GetGroupsPerTrack() + basic->GetSectorsPerTrackOnBasic() - 1) / basic->GetSectorsPerTrackOnBasic();
	basic->SetGroupsPerSector(grps_per_sec);

	// グループ数
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetGroupsPerTrack();
		basic->SetFatEndGroup(end_group - 1);
	}

	return 1.0;
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFROST::CheckFat(bool is_formatting)
{
	double valid_ratio = DiskBasicTypeFAT8::CheckFat(is_formatting);
	if (valid_ratio >= 0.0) {
		// FAT,ディレクトリエリアはシステム予約となっているか
		wxArrayInt groups = basic->GetReservedGroups();
		for(size_t idx = 0; idx < groups.Count(); idx++) {
			wxUint32 grp = GetGroupNumber((wxUint32)groups.Item(idx));
			if (grp != basic->GetGroupSystemCode()) {
				valid_ratio = -1.0;
				break;
			}
		}
	}
	return valid_ratio;
}

/// 使用可能なディスクサイズを得る
/// @param [out] disk_size  ディスクサイズ
/// @param [out] group_size グループ数
void DiskBasicTypeFROST::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = 0;
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		if (gnum != basic->GetGroupSystemCode()) group_size++;
	}
	disk_size = group_size * basic->GetSectorSize() / basic->GetGroupsPerSector();
}

/// 残りディスクサイズを計算
void DiskBasicTypeFROST::CalcDiskFreeSize(bool wrote)
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	fat_availability.Empty();

	// 使用済みかチェック
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		int fsts = FAT_AVAIL_USED;
		if (gnum == basic->GetGroupUnusedCode()) {
			fsize += (basic->GetSectorSize() / basic->GetGroupsPerSector());
			grps++;
			fsts = FAT_AVAIL_FREE;
		} else if (gnum == basic->GetGroupSystemCode()) {
			fsts = FAT_AVAIL_SYSTEM;
		} else if (gnum >= basic->GetGroupFinalCode()) {
			fsts = FAT_AVAIL_USED_LAST;
		}
		fat_availability.Add(fsts);
	}

	free_disk_size = (int)fsize;
	free_groups = (int)grps;
}

/// 未使用が連続している位置をさがす
wxUint32 DiskBasicTypeFROST::FindContinuousArea(wxUint32 group_size)
{
	// 未使用が連続している位置をさがす
	wxUint32 group = INVALID_GROUP_NUMBER;
	wxUint32 group_start = INVALID_GROUP_NUMBER;
	wxUint32 cnt = 0;
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup() && cnt < group_size; gnum++) {
		if (GetGroupNumber(gnum) == basic->GetGroupUnusedCode()) {
			if (cnt == 0) {
				group_start = gnum;
			}
			cnt++;
		} else {
			cnt = 0;
		}
	}
	if (cnt == group_size) {
		group = group_start;
	}
	return group;
}

/// データサイズ分のグループを確保する
/// @param [in]  fileunit_num ファイル番号
/// @param [in]  item         ディレクトリアイテム
/// @param [in]  data_size    確保するデータサイズ（バイト）
/// @param [in]  flags        新規か追加か
/// @param [out] group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeFROST::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int groups = 0; 

	// FAT
	int  rc = 0;
	bool first_group = (flags == ALLOCATE_GROUPS_NEW);
	int sizeremain = data_size;

	int bytes_per_group = basic->GetSectorSize() / basic->GetGroupsPerSector();
	wxUint32 group_size = (data_size + bytes_per_group - 1) / bytes_per_group;
	// 連続して確保できる領域
	wxUint32 group_num = FindContinuousArea(group_size);
	if (group_num == INVALID_GROUP_NUMBER) {
		group_num = GetEmptyGroupNumber();
	}
	int limit = basic->GetFatEndGroup() + 1;
	while(rc >= 0 && limit >= 0 && sizeremain > 0) {
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			rc = first_group ? -1 : -2;
			break;
		}
		// 位置を予約
		SetGroupNumber(group_num, basic->GetGroupFinalCode());

		// グループ番号の書き込み
		if (first_group) {
			item->SetStartGroup(fileunit_num, group_num);
			first_group = false;
		}

		// 次の空きグループをさがす
		wxUint32 next_group_num = GetNextEmptyGroupNumber(group_num);

		// 次の空きがない場合 or 残りサイズがこのグループで収まる場合
		if (next_group_num == INVALID_GROUP_NUMBER || sizeremain <= bytes_per_group) {
			// 最後のグループ番号
			next_group_num = CalcLastGroupNumber(next_group_num, sizeremain);
		}
		
		basic->GetNumsFromGroup(group_num, next_group_num, basic->GetSectorSize(), sizeremain, group_items);

		// グループ番号設定
		SetGroupNumber(group_num, next_group_num);

		group_num = next_group_num;

		sizeremain -= bytes_per_group;
		groups++;

		limit--;
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = first_group ? -1 : -2;
	}

	if (rc >= 0) {
		if (flags == ALLOCATE_GROUPS_APPEND) {
			// 追加のときはチェインをつなぐ
			if (group_items.Count() > 0) {
				rc = ChainGroups(item->GetStartGroup(0), group_items.Item(0).group);
			}
		}
	} else {
		// グループを削除
		DeleteGroups(group_items);
		rc = -1;
	}

	return rc;
}

/// グループ番号から開始セクタ番号を得る
/// @param [in] group_num グループ番号
/// @return 開始セクタ番号
int DiskBasicTypeFROST::GetStartSectorFromGroup(wxUint32 group_num)
{
	return (int)group_num;
}

/// トラック＋セクタ番号から論理セクタ番号を得る
wxUint32 DiskBasicTypeFROST::ConvSectorPosFromTrackSector(wxUint32 trk_sec) const
{ 
	if (trk_sec == basic->GetGroupUnusedCode() || trk_sec == basic->GetGroupFinalCode() || trk_sec == basic->GetGroupSystemCode()) {
		return trk_sec;
	}

	return (trk_sec >> 8) * basic->GetGroupsPerTrack() + (trk_sec & 0xff) - 1;
}

/// 論理セクタ番号からトラック＋セクタ番号を得る
wxUint32 DiskBasicTypeFROST::ConvTrackSectorFromSectorPos(wxUint32 pos) const
{
	if (pos == basic->GetGroupUnusedCode() || pos == basic->GetGroupFinalCode() || pos == basic->GetGroupSystemCode()) {
		return pos;
	}

	return ((pos / basic->GetGroupsPerTrack()) << 8) + (pos % basic->GetGroupsPerTrack()) + 1;
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
/// @param [out] div_num    分割番号
/// @param [out] div_nums   分割数
void DiskBasicTypeFROST::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
	int groups_per_track = basic->GetGroupsPerTrack();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();

	int grps_per_sec = basic->GetGroupsPerSector();

	// 2D, 2HD
	int trksid_num = sector_pos / groups_per_track;
	track_num = trksid_num / sides_per_disk;
	side_num = trksid_num % sides_per_disk;
	sector_num = ((sector_pos % groups_per_track) / grps_per_sec) + 1;
	if (div_num) *div_num = ((sector_pos % groups_per_track) % grps_per_sec);

	if (sector_num * grps_per_sec > groups_per_track) {
		grps_per_sec = grps_per_sec - (sector_num * grps_per_sec - groups_per_track);
	}

	if (div_nums) *div_nums = grps_per_sec;
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeFROST::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
	int groups_per_track = basic->GetGroupsPerTrack();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	// 2D, 2HD
	sector_pos = (track_num * sides_per_disk + side_num) * groups_per_track;
	sector_pos += ((sector_num - 1) * div_nums + div_num);

	return sector_pos;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeFROST::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeFROST::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// FAT トラック０はシステム
	wxUint32 egnum = basic->GetGroupsPerTrack() * basic->GetSidesPerDiskOnBasic() - 1;
	for(wxUint32 gnum = 0; gnum < egnum; gnum++) {
		SetGroupNumber(gnum, basic->GetGroupSystemCode());
	}
	// FAT FAT, DIRエリアはシステム
	wxArrayInt arr = basic->GetReservedGroups();
	for(size_t idx = 0; idx < arr.Count(); idx++) {
		wxUint32 gnum = (wxUint32)arr.Item(idx);
		SetGroupNumber(gnum, basic->GetGroupSystemCode());
	}
	return true;
}

//
// for write
//

/// グループ確保時に最後のグループ番号を計算する
/// @param [in]     group_num	現在のグループ番号
/// @param [in,out] size_remain	残りのデータサイズ
/// @return 最後のグループ番号
wxUint32 DiskBasicTypeFROST::CalcLastGroupNumber(wxUint32 group_num, int &size_remain)
{
	return basic->GetGroupFinalCode();
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
int DiskBasicTypeFROST::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
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
	// 反転
	basic->InvertMem(buffer, size);

	return len;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeFROST::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// タイトル名 FATエリア 
	int div_num, div_nums;
	DiskD88Sector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1 + 3, NULL, NULL, NULL, &div_num, &div_nums);
	if (sector) {
		wxUint8 *buf = sector->GetSectorBuffer();
		buf += sector->GetSectorSize() * div_num / div_nums + 0x140;
		if (buf[0] >= 0x20 && buf[0] < 0xff) {
			wxString dst;
			basic->GetCharCodes().ConvToString(buf, 64, dst, 0);
			data.SetVolumeName(dst);
		}
	}
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeFROST::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
#if 0
	// タイトル名 FATエリア
	if (basic->GetFormatType()->HasVolumeName()) {
		int div_num, div_nums;
		DiskD88Sector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1 + 3, NULL, NULL, NULL, &div_num, &div_nums);
		if (sector) {
			wxCharBuffer n = data.GetVolumeName().To8BitData();
			size_t l = n.length();
			if (l > 0) {
				if (l > 64) {
					l = 64;
				}
				sector->Copy(n.data(), l, sector->GetSectorSize() * div_num / div_nums + 0x140);
			}
		}
	}
#endif
}
