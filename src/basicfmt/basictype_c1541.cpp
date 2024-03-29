/// @file basictype_c1541.cpp
///
/// @brief disk basic type for Commodore 1541
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_c1541.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "basicdiritem_c1541.h"
#include "../utils.h"
#include "../logging.h"


//////////////////////////////////////////////////////////////////////
//
// C1541 BAM ビットマップ
//
C1541Bitmap::C1541Bitmap()
{
	m_my_group_num = 0;
	m_bam = NULL;
}

/// 指定位置のビットを変更する
/// @param[in] track_num  トラック番号(0 ..)
/// @param[in] sector_num セクタ番号(0 ..)
/// @param[in] use セットする場合true
void C1541Bitmap::Modify(int track_num, int sector_num, bool use)
{
	int pos = sector_num >> 3;
	int bit = sector_num & 7;
	if (use) {
		m_bam->map[track_num].bits[pos] &= ~(1 << bit);
		m_bam->map[track_num].remain--;
	} else {
		m_bam->map[track_num].bits[pos] |= (1 << bit);
		m_bam->map[track_num].remain++;
	}
}

/// 指定位置が空いているか
/// @param[in] track_num  トラック番号(0 ..)
/// @param[in] sector_num セクタ番号(0 ..)
/// @return 空いている場合 true
bool C1541Bitmap::IsFree(int track_num, int sector_num) const
{
	int pos = sector_num >> 3;
	int bit = sector_num & 7;
	return ((m_bam->map[track_num].bits[pos] & (1 << bit)) != 0);
}

/// 指定トラックをすべて未使用にする
/// @param[in] track_num  トラック番号(0 ..)
/// @param[in] num_of_sector セクタ数
void C1541Bitmap::FreeTrack(int track_num, int num_of_sector)
{
	int val = (1 << num_of_sector) - 1;
	for(int pos = 0; pos < 3; pos++) {
		m_bam->map[track_num].bits[pos] = (val & 0xff);
		val >>= 8;
	}
	m_bam->map[track_num].remain = num_of_sector;
}

/// ディスク名を返す
size_t C1541Bitmap::GetDiskName(wxUint8 *buf, size_t len) const
{
	if (len > sizeof(m_bam->disk_name)) len = sizeof(m_bam->disk_name);
	memcpy(buf, m_bam->disk_name, len);
	return sizeof(m_bam->disk_name);
}

/// ディスク名を設定
void C1541Bitmap::SetDiskName(const wxUint8 *buf, size_t len)
{
	if (len > sizeof(m_bam->disk_name)) len = sizeof(m_bam->disk_name);
	memcpy(m_bam->disk_name, buf, len);
}

/// ディスク名サイズを返す
size_t C1541Bitmap::GetDiskNameSize() const
{
	return sizeof(m_bam->disk_name);
}

/// ディスクIDを返す
int C1541Bitmap::GetDiskID() const
{
	return wxUINT16_SWAP_ON_LE(m_bam->disk_id);
}

/// ディスクIDを設定
void C1541Bitmap::SetDiskID(int val)
{
	m_bam->disk_id = wxUINT16_SWAP_ON_LE(val);
}

//////////////////////////////////////////////////////////////////////
//
// C1541 セクタ位置変換マップリスト
//
void C1541SectorPosTrans::CreateSectorSkewMap(DiskBasic *basic)
{
	// インポート時の空きセクタの探し方をセット
	for(size_t i=0; i<Count(); i++) {
		SectorsPerTrack *item = &Item(i);
		DiskBasicSectorSkewForSave *map = new DiskBasicSectorSkewForSave();
		map->Create(basic, item->GetNumOfSectors());
		item->SetSectorSkewMap(map);
	}
}

//////////////////////////////////////////////////////////////////////
//
// Commodore 1541 の処理
//
DiskBasicTypeC1541::DiskBasicTypeC1541(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

DiskBasicTypeC1541::~DiskBasicTypeC1541()
{
}

/// エリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeC1541::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeC1541::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 0;

	double valid_ratio = 1.0;

	// 可変数セクタなのでトラックごとのセクタ数を集計
	sector_map.Create(basic);

	// セクタ数の合計
	basic->SetFatEndGroup((wxUint32)sector_map.GetTotalSectors() - 1);

	// インポート時の空きセクタの探し方を設定
	sector_map.CreateSectorSkewMap(basic);

	// BAM
	DiskImageSector *sector = basic->GetManagedSector(0);
	if (!sector) {
		return -1.0;
	}
	c1541_bam_t *bam = (c1541_bam_t *)sector->GetSectorBuffer();
	if (!bam) {
		return -1.0;
	}

	// チェック
	if (bam->space0 != 0xa0 || bam->space1 != 0xa0) {
		valid_ratio = 0.0;
	} else if (bam->dos_version != '2' || bam->dos_format != 'A') {
		valid_ratio = 0.5;
	}

	c1541_bam.SetBitmap(bam);

	int track_num = basic->GetManagedTrackNumber();
	int sector_num = basic->GetSectorNumberBase();
	int bam_sector_pos = GetSectorPosFromNumS(track_num, sector_num);
	c1541_bam.SetMyGroupNumber((wxUint32)bam_sector_pos);

	// ディレクトリエリア
	if (bam->start_dir.track >= (basic->GetTracksPerSideOnBasic() + basic->GetTrackNumberBaseOnDisk())
		|| bam->start_dir.sector > basic->GetSectorsPerTrack()) {
		return -1.0;
	}
	track_num = bam->start_dir.track - C1541_START_TRACK_OFFSET;
	sector_num = bam->start_dir.sector - C1541_START_SECTOR_OFFSET;

	int dir_sector_num = GetSectorPosFromNumS(track_num, sector_num);

	// ディレクトリ開始はBAMセクタからの相対位置とする
	dir_sector_num = dir_sector_num - bam_sector_pos + basic->GetSectorNumberBase();
	basic->SetDirStartSector(dir_sector_num);

	basic->SetSectorsPerFat(1);

	return valid_ratio;
}


/// Allocation Mapの開始位置を得る（ダイアログ用）
void DiskBasicTypeC1541::GetStartNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	track_num = basic->GetManagedTrackNumber();
	side_num = 0;
	sector_num = basic->GetSectorNumberBase();
}

/// Allocation Mapの終了位置を得る（ダイアログ用）
void DiskBasicTypeC1541::GetEndNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	track_num = basic->GetManagedTrackNumber();
	side_num = 0;
	sector_num = basic->GetSectorNumberBase();
}

/// タイトル名（ダイアログ用）
wxString DiskBasicTypeC1541::GetTitleForFat() const
{
	return _("Allocation Map");
}

#if 0
/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicTypeC1541::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool sts = DiskBasicType::AssignRootDirectory(start_sector, end_sector, group_items, dir_item);

	return sts;
}
#endif

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
bool DiskBasicTypeC1541::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	bool valid = true;

	group_items.Empty();

	// ディレクトリのチェインをたどる
	size_t dir_size = 0;
	int limit = basic->GetSectorsPerTrackOnBasic();
	int mng_trk_num = basic->GetManagedTrackNumber();
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 0;

	// 開始セクタ
	int sector_pos = GetSectorPosFromNumS(mng_trk_num, basic->GetDirStartSector());

	while(valid && limit >= 0) {
		DiskImageSector *sector = basic->GetSectorFromSectorPos(sector_pos, trk_num, sid_num);
		if (!sector) {
			valid = false;
			break;
		}
		sec_num = sector->GetSectorNumber();
		wxUint8 *buffer = sector->GetSectorBuffer();
		if (!buffer) {
			valid = false;
			break;
		}
		wxUint32 group_num = (wxUint32)sector_pos;
		group_items.Add(group_num, 0, trk_num, sid_num, sec_num, sec_num);

		dir_size += sector->GetSectorSize();

		// 次のセクタ
		c1541_ptr_t *next = (c1541_ptr_t *)buffer;
		if (next->track == 0 || (int)next->track > basic->GetTracksPerSideOnBasic()) {
			break;
		}

		sector_pos = GetSectorPosFromNumS(next->track - C1541_START_TRACK_OFFSET, next->sector - C1541_START_SECTOR_OFFSET);

		limit--;
	}
	group_items.SetSize(dir_size);

	if (limit < 0) {
		valid = false;
	}

	int sta_sector_num = GetSectorPosFromNumS(mng_trk_num, 1);
	int end_sector_num = GetSectorPosFromNumS(trk_num, sec_num);
	basic->SetDirEndSector(end_sector_num - sta_sector_num + 1);

	return valid;
}

/// セクタをディレクトリとして初期化
/// @param [in]     group_items 確保したセクタリスト
/// @param [in,out] file_size   サイズ ディレクトリを拡張した時は既存サイズに加算
/// @param [in,out] size_remain 残りサイズ
/// @param [in,out] errinfo     エラー情報
/// @return 0:正常 <0:エラー 
int DiskBasicTypeC1541::InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &size_remain, DiskBasicError &errinfo)
{
	for(size_t i=0; i<group_items.Count(); i++) {
		DiskImageSector *sector = basic->GetSectorFromSectorPos(group_items.Item(i).group);
		sector->Fill(0, sector->GetSectorSize() - 2, 2);
	}

	file_size += (int)group_items.Count() * (basic->GetSectorSize() - 2);

	size_remain = 0;
	return 0;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeC1541::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	disk_size = group_size * basic->GetSectorSize();
}

/// 残りディスクサイズを計算
void DiskBasicTypeC1541::CalcDiskFreeSize(bool wrote)
{
	fat_availability.Empty();

	// BITMAP table
	for(int sec_pos = 0; sec_pos <= (int)basic->GetFatEndGroup(); sec_pos++) {
		int trk_num = 0;
		int sec_num = 0;
		GetNumFromSectorPosS(sec_pos, trk_num, sec_num);
		trk_num -= basic->GetTrackNumberBaseOnDisk();
		sec_num -= basic->GetSectorNumberBase();
		if (c1541_bam.IsFree(trk_num, sec_num)) {
			fat_availability.Add(FAT_AVAIL_FREE, basic->GetSectorSize(), 1);
		} else {
			fat_availability.Add(FAT_AVAIL_USED, 0, 0);
		}
	}
	fat_availability.Set(c1541_bam.GetMyGroupNumber(), FAT_AVAIL_SYSTEM);
	DiskBasicDirItem *root = dir->GetRootItem();
	if (root) {
		const DiskBasicGroups *root_groups = &root->GetGroups();
		for(size_t i=0; i<root_groups->Count(); i++) {
			fat_availability.Set(root_groups->Item(i).group, FAT_AVAIL_SYSTEM);
		}
	}

//	free_disk_size = (int)fat_availability.GetFreeSize();
//	free_groups = (int)fat_availability.GetFreeGroups();
}

/// グループ番号を使用済みにする
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeC1541::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	int trk_num = 0;
	int sec_num = 0;
	GetNumFromSectorPosS((int)num, trk_num, sec_num);
	trk_num -= basic->GetTrackNumberBaseOnDisk();
	sec_num -= basic->GetSectorNumberBase();
	c1541_bam.Modify(trk_num, sec_num, val != 0);
}

/// グループ番号を得る
wxUint32 DiskBasicTypeC1541::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeC1541::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeC1541::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeC1541::GetEmptyGroupNumber()
{
	return GetEmptyGroupNumberM(0);
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeC1541::GetEmptyGroupNumberM(int method)
{
	const int findtrkmap[2][3] = {
		{ 0, 1, 2 },
		{ 2, 0, 1 }
	};

	wxUint32 new_num = INVALID_GROUP_NUMBER;

	int sta_trk = 0;
	int end_trk = 0;
	int ndir = 1;

	for(int n=0; n<2; n++) {
		int i = findtrkmap[method][n];
		switch(i) {
		case 0:
			// 内側から検索
			sta_trk = basic->GetManagedTrackNumber() - 1;
			end_trk = basic->GetTrackNumberBaseOnDisk() - 1;
			ndir = -1;
			break;
		case 1:
			// 外側へ検索
			sta_trk = basic->GetManagedTrackNumber() + 1;
			end_trk = basic->GetTracksPerSideOnBasic() + basic->GetTrackNumberBaseOnDisk();
			ndir = 1;
			break;
		case 2:
			// 管理トラック
			sta_trk = basic->GetManagedTrackNumber();
			end_trk = sta_trk + 1;
			ndir = 1;
			break;
		}

		for(int trk_num = sta_trk; trk_num != end_trk && new_num == INVALID_GROUP_NUMBER; trk_num += ndir) {
			int trk_anum = trk_num - basic->GetTrackNumberBaseOnDisk();
			const SectorsPerTrack *item = sector_map.FindByTrackNum(trk_anum);
			int num_of_secs = item->GetNumOfSectors();
			for(int sec_pos = 0; sec_pos < num_of_secs; sec_pos++) {
				const SectorSkewBase *ss = item->GetSectorSkewMap();
				int sec_num = ss->ToPhysical(sec_pos);
				int sec_anum = sec_num - basic->GetSectorNumberBase();
				if (c1541_bam.IsFree(trk_anum, sec_anum)) {
					new_num = (wxUint32)GetSectorPosFromNumS(trk_num, sec_num);
					break;
				}
			}
		}
	}

	return new_num;
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeC1541::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	// 次の空き位置候補
	wxUint32 next_group_num = GetEmptyGroupNumberM(0);
	if (next_group_num == INVALID_GROUP_NUMBER) {
		return INVALID_GROUP_NUMBER;
	}
	// 現在のセクタに次のセクタへのポインタをセット
	if (ChainGroups(curr_group, next_group_num) < 0) {
		return INVALID_GROUP_NUMBER;
	}

	return next_group_num;
}

/// 次の空きFAT位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeC1541::GetDirNextEmptyGroupNumber(wxUint32 curr_group)
{
	// 次の空き位置候補
	wxUint32 next_group_num = GetEmptyGroupNumberM(1);
	if (next_group_num == INVALID_GROUP_NUMBER) {
		return INVALID_GROUP_NUMBER;
	}
	// 現在のセクタに次のセクタへのポインタをセット
	if (ChainGroups(curr_group, next_group_num) < 0) {
		return INVALID_GROUP_NUMBER;
	}

	return next_group_num;
}

#if 0
/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeC1541::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	return true;
}
#endif

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeC1541::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
//	myLog.SetDebug("DiskBasicTypeC1541::AllocateGroups {");

//	int file_size = 0;
	int groups = 0;

	int rc = 0;
//	int sector_size = basic->GetSectorSize();
	// 1セクタ当たり2バイトはチェイン用のリンクポインタになるので減算
	int bytes_per_group = basic->GetSectorSize() - 2;
	int remain = data_size;
	int limit = basic->GetFatEndGroup() + 1;
	int chain_idx = 0;
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	if (flags == ALLOCATE_GROUPS_APPEND) {
		// ディレクトリ拡張時
		remain = bytes_per_group;
		group_num = item->GetGroups().Last().group;
	}
	while(remain > 0 && limit >= 0) {
		// 空きをさがす
		if (flags != ALLOCATE_GROUPS_APPEND) {
			group_num = (chain_idx == 0) ? GetEmptyGroupNumber() : GetNextEmptyGroupNumber(group_num);
		} else {
			group_num = GetDirNextEmptyGroupNumber(group_num);
		}
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			rc = groups > 0 ? -2 : -1;
			return rc;
		}

		// 使用済みにする
		basic->GetNumsFromGroup(group_num, 0, basic->GetSectorSize(), remain, group_items);
		SetGroupNumber(group_num, 1);

		if (flags != ALLOCATE_GROUPS_APPEND && chain_idx == 0) {
			item->SetStartGroup(0, group_num, 1);
		}
		chain_idx++;

//		file_size += bytes_per_group;
		groups++;
		remain -= bytes_per_group;
		limit--;
	}

	if (groups > 0) {
		// 最終セクタは残りサイズを設定
		remain += bytes_per_group;
		ChainLastGroup(group_num, remain);
	}

	if (limit < 0) {
		// 無限ループ？
		rc = groups > 0 ? -2 : -1;
	}

//	myLog.SetDebug("rc: %d }", rc);
	return rc;
}

/// グループをつなげる
int DiskBasicTypeC1541::ChainGroups(wxUint32 group_num, wxUint32 append_group_num)
{
	// 現在のセクタに次のセクタへのポインタをセット
	DiskImageSector *sector = basic->GetSectorFromSectorPos(group_num);
	if (!sector) {
		// why?
		return -1;
	}
	c1541_ptr_t *p = (c1541_ptr_t *)sector->GetSectorBuffer();
	if (!p) {
		// why?
		return -1;
	}
	int next_track_num = 0;
	int next_sector_num = 0;
	GetNumFromSectorPosS(append_group_num, next_track_num, next_sector_num);
	p->track = (wxUint8)(next_track_num + C1541_START_TRACK_OFFSET);
	p->sector = (wxUint8)(next_sector_num + C1541_START_SECTOR_OFFSET);

	return 0;
}

/// 最終グループをつなげる
int DiskBasicTypeC1541::ChainLastGroup(wxUint32 group_num, int remain)
{
	// 現在のセクタに残りサイズをセット
	DiskImageSector *sector = basic->GetSectorFromSectorPos(group_num);
	if (!sector) {
		// why?
		return -1;
	}
	c1541_ptr_t *p = (c1541_ptr_t *)sector->GetSectorBuffer();
	if (!p) {
		// why?
		return -1;
	}
	p->track = 0;
	p->sector = (wxUint8)(remain + 1);

	return 0;
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
/// @return >=0 : 処理したサイズ  -1:比較不一致
int DiskBasicTypeC1541::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	const wxUint8 *buf = &sector_buffer[2];
	int size = (sector_size - 2) < remain_size ? (sector_size - 2) : remain_size;

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

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeC1541::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	return remain_size;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeC1541::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeC1541::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return group_num;
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num 物理セクタ番号
/// @param [out] div_num    分割番号
/// @param [out] div_nums   分割数
void DiskBasicTypeC1541::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();

	// セクタ位置がどのトラックにあるか
	sector_map.GetNumFromSectorPos(sector_pos, track_num, sector_num, sectors_per_track);

	// サイド番号
	side_num = sector_num * sides_per_disk / sectors_per_track;

	// 連番でない場合
	if (numbering_sector != 1) {
		sector_num = sector_num % (sectors_per_track / sides_per_disk);
	}

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBase();

	if (div_num)  *div_num = 0;
	if (div_nums) *div_nums = 1;
}

/// 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] sector_num 論理セクタ番号
void DiskBasicTypeC1541::GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num)
{
	int sectors_per_track = 1;

	sector_map.GetNumFromSectorPos(sector_pos, track_num, sector_num, sectors_per_track);

	sector_num += basic->GetSectorNumberBase();
	track_num += basic->GetTrackNumberBaseOnDisk();
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  物理セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeC1541::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	int sector_pos = sector_map.GetSectorPosFromNum(track_num, sector_num, sectors_per_track);

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	// 連番でない場合
	if (numbering_sector != 1) {
		sector_pos += side_num * sectors_per_track / sides_per_disk;
	}

	return sector_pos;
}

/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] sector_num 論理セクタ番号
/// @return 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeC1541::GetSectorPosFromNumS(int track_num, int sector_num)
{
	int sectors_per_track = 1;

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	return sector_map.GetSectorPosFromNum(track_num, sector_num, sectors_per_track);
}

/// ルートディレクトリか
bool DiskBasicTypeC1541::IsRootDirectory(wxUint32 group_num)
{
	return false;
}

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeC1541::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskImageSector *sector;

	// 可変数セクタなのでトラックごとのセクタ数を集計
	sector_map.Create(basic);

	// セクタ数の合計
	basic->SetFatEndGroup((wxUint32)sector_map.GetTotalSectors() - 1);

	// インポート時の空きセクタの探し方を設定
	sector_map.CreateSectorSkewMap(basic);

	// BAMの作成
	int trk_num = basic->GetManagedTrackNumber();
	int sec_num = - C1541_START_SECTOR_OFFSET;
	sector = basic->GetSector(trk_num, sec_num);
	if (!sector) {
		// Why?
		return false;
	}
	c1541_bam_t *bam = (c1541_bam_t *)sector->GetSectorBuffer();
	if (!bam) {
		// Why?
		return false;
	}
	sector->Fill(0);

	c1541_bam.SetBitmap(bam);
	int sector_pos = GetSectorPosFromNumS(trk_num, sec_num);
	c1541_bam.SetMyGroupNumber(sector_pos);

	// directory
	bam->start_dir.track = (trk_num + C1541_START_TRACK_OFFSET);
	bam->start_dir.sector = (sec_num + 1 + C1541_START_SECTOR_OFFSET);

	bam->format_type = 'A';	// 4040format
	bam->disk_id = 0x3030;	// "00"
	bam->space0 = 0xa0;
	bam->dos_version = '2';
	bam->dos_format = 'A';
	bam->space1 = 0xa0;

	// bitmap クリア
	for(int trk = 0; trk < basic->GetTracksPerSide(); trk++) {
		const SectorsPerTrack *item = sector_map.FindByTrackNum(trk);
		c1541_bam.FreeTrack(trk, item->GetNumOfSectors());
	}
	int trk_npos = trk_num - basic->GetTrackNumberBaseOnDisk();
	int sec_npos = sec_num - basic->GetSectorNumberBase();
	c1541_bam.Modify(trk_npos, sec_npos, true);
	c1541_bam.Modify(trk_npos, sec_npos + 1, true);

	//
	// ディレクトリ
	//
	sec_num++;
	sector = basic->GetSector(trk_num, sec_num);
	if (!sector) {
		// Why?
		return false;
	}
	c1541_ptr_t *next = (c1541_ptr_t *)sector->GetSectorBuffer();
	if (!next) {
		// Why?
		return false;
	}
	sector->Fill(0);
	next->sector = 0xff;

	basic->SetDirStartSector(sec_num);

	// volume name
	SetIdentifiedData(data);

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
int DiskBasicTypeC1541::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	int len = 0;

	// セクタの2バイト目から
	buffer += 2;
	size -= 2;

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

	return len;
}

/// データの書き込み終了後の処理
/// @param [in]	 item			ディレクトリアイテム
void DiskBasicTypeC1541::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	// RELファイルか
	int type1 = item->GetFileAttr().GetOrigin();
	int rec_len = (type1 >> 8);
	type1 &= 0xff;
	if (type1 != FILETYPE_MASK_C1541_REL || rec_len == 0) {
		return;
	}

	// RELファイルの時は、サイドセクタを作成する
	int bytes_per_group = basic->GetSectorSize() - 2;
	const DiskBasicGroups *data_groups = &item->GetGroups();

	int blocks = (int)data_groups->Count();
	int ss_max = blocks / 120;
	if (ss_max >= 6) {
		return;
	}

	DiskBasicGroups side_groups;
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	int ss_size = 0;
	int data_pos = 0;
	c1541_side_sector_t *side_sectors[6];
	for(int ss_idx = 0; ss_idx <= ss_max; ss_idx++) {
		// 空きをさがす
		group_num = (ss_idx == 0) ? GetEmptyGroupNumber() : GetNextEmptyGroupNumber(group_num);
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			return;
		}
		int track_num = 0;
		int side_num = 0;
		DiskImageSector *sector = basic->GetSectorFromSectorPos((int)group_num, track_num, side_num);
		if (!sector) {
			return;
		}

		side_sectors[ss_idx] = (c1541_side_sector_t *)sector->GetSectorBuffer();
		c1541_side_sector_t *side_sector = side_sectors[ss_idx];
		if (!side_sector) {
			return;
		}

		sector->Fill(0);
		SetGroupNumber(group_num, 1);
		side_groups.Add(group_num, 0, track_num, side_num, sector->GetSectorNumber(), sector->GetSectorNumber());
		ss_size += bytes_per_group;
		if (ss_idx == 0) {
			// サイドセクタ開始ポインタを設定
			item->SetExtraGroup(group_num);
		} else {
			// サイドセクタへのポインタをコピー
			memcpy(side_sectors[ss_idx]->side_pos, side_sectors[ss_idx - 1]->side_pos, sizeof(side_sectors[ss_idx]->side_pos));
		}

		side_sector->side_num = (ss_idx & 0xff);
		side_sector->record_length = (rec_len & 0xff);

		int sector_num = 0;
		GetNumFromSectorPosS((int)group_num, track_num, sector_num);
		track_num += C1541_START_TRACK_OFFSET;
		sector_num += C1541_START_SECTOR_OFFSET;

		// サイドセクタへのポインタを設定
		for(int i = 0; i <= ss_idx; i++) {
			c1541_side_sector_t *ss = side_sectors[i];
			ss->side_pos[ss_idx].track = (track_num & 0xff);
			ss->side_pos[ss_idx].sector = (sector_num & 0xff);
		}

		// データへのポインタを設定
		for(int i = 0; i < 120 && data_pos < blocks; i++) {
			GetNumFromSectorPosS(data_groups->Item(data_pos).group, track_num, sector_num);
			track_num += C1541_START_TRACK_OFFSET;
			sector_num += C1541_START_SECTOR_OFFSET;

			side_sector->data_pos[i].track = (track_num & 0xff);
			side_sector->data_pos[i].sector = (sector_num & 0xff);

			data_pos++;
		}
	}

	// 最終セクタは残りサイズを設定
	ChainLastGroup(group_num, (blocks % 120) * 2 + 14);

	// ブロックサイズを設定
	ss_size = ss_size - bytes_per_group + (blocks % 120) * 2 + 14;
	side_groups.SetSize(ss_size);
	side_groups.SetNums(ss_max + 1);
	side_groups.SetSizePerGroup(basic->GetSectorSize());
	item->SetExtraGroups(side_groups);
}

/// FAT領域を削除する
void DiskBasicTypeC1541::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}

/// ファイル削除後の処理
bool DiskBasicTypeC1541::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// サイドセクタを未使用にする
	DiskBasicGroups grps;
	item->GetExtraGroups(grps);

	DeleteGroups(grps);

	return true;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeC1541::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// volume name
	wxUint8 name[sizeof(((c1541_bam_t *)NULL)->disk_name) + 1];
	memset(name, 0, sizeof(name));
	size_t len = c1541_bam.GetDiskName(name, sizeof(name));

	rtrim(name, sizeof(name), basic->GetDirSpaceCode());

	wxString wname;
	basic->GetCharCodes().ConvToString(name, len, wname, 0);
	data.SetVolumeName(wname);
	data.SetVolumeNameMaxLength(len);

	// volume id
	data.SetVolumeNumber(c1541_bam.GetDiskID());
	data.VolumeNumberIsHexa(true);
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeC1541::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	const DiskBasicFormat *fmt = basic->GetFormatType();

	// volume name
	if (fmt->HasVolumeName()) {
		wxUint8 name[sizeof(((c1541_bam_t *)NULL)->disk_name) + 1];
		memset(name, 0, sizeof(name));
		basic->GetCharCodes().ConvToChars(data.GetVolumeName(), name, sizeof(name));
		padding(name, sizeof(name), basic->GetDirSpaceCode());
		c1541_bam.SetDiskName(name, sizeof(name));
	}
	// volume id
	if (fmt->HasVolumeNumber()) {
		c1541_bam.SetDiskID(data.GetVolumeNumber());
	}
}
