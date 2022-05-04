/// @file basictype_trsdos.cpp
///
/// @brief disk basic type for Tandy TRSDOS 2.x / 1.3
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_trsdos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "basicdiritem_trsdos.h"
#include "../utils.h"
#include "../logging.h"

#pragma pack(1)
/// @brief GATエリア構造
typedef struct st_trsdos_gat {
	wxUint8  gat[0x60];
	wxUint8  tlt[0x60];
	wxUint8  reserved1[14];
	wxUint16 password;
	wxUint8  name[8];
	wxUint8  date[8];	// MM/DD/YY
	wxUint8  apt[32];
} trsdos_gat_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////
//
// TRSDOS GAT
//
TRSDOS_GAT::TRSDOS_GAT()
{
	m_buffer = NULL;
	m_size = 0;
	m_groups_per_track = 1;
}

TRSDOS_GAT::TRSDOS_GAT(wxUint8 *n_buffer, size_t n_size, int n_groups_per_track)
{
	m_buffer = n_buffer;
	m_size = n_size;
	m_groups_per_track = n_groups_per_track;
}

/// 指定位置のビットを変更する
/// @param[in] num ビット位置
/// @param[in] val true:セット / false:リセット
void TRSDOS_GAT::Modify(wxUint32 num, bool val)
{
	wxUint32 pos, bit;
	GetPos(num, pos, bit);
	if (val) {
		m_buffer[pos] |= (1 << bit);
	} else {
		m_buffer[pos] &= ~(1 << bit);
	}
}

/// 指定位置のビットがセットされているか
/// @param[in] num ビット位置
/// @return    true:セット / false:リセット
bool TRSDOS_GAT::IsSet(wxUint32 num) const
{
	wxUint32 pos, bit;
	GetPos(num, pos, bit);
	return ((m_buffer[pos] & (1 << bit)) != 0);
}

/// 指定位置のビット位置を計算
/// @param[in]  num ビット位置
/// @param[out] pos バッファ位置
/// @param[out] bit ビット
void TRSDOS_GAT::GetPos(wxUint32 num, wxUint32 &pos, wxUint32 &bit) const
{
	pos = num / m_groups_per_track;
	bit = num % m_groups_per_track;
}

//////////////////////////////////////////////////////////////////////
//
// TRSDOS HIT
//
TRSDOS_HIT::TRSDOS_HIT()
{
	m_hit_buffer = NULL;
	m_hit_size = 0;
}

void TRSDOS_HIT::AssignHIT(wxUint8 *n_buffer, size_t n_size)
{
	m_hit_buffer = n_buffer;
	m_hit_size = n_size;
}

/// ハッシュを得る
wxUint8 TRSDOS_HIT::GetHI(wxUint32 pos)
{
	if (!m_hit_buffer) return 0;
	if (m_hit_size <= pos) return 0;

	return m_hit_buffer[pos];
}

/// ハッシュをセット
void TRSDOS_HIT::SetHI(wxUint32 pos, wxUint8 val)
{
	if (!m_hit_buffer) return;
	if (m_hit_size <= pos) return;

	m_hit_buffer[pos] = val;
}

/// ハッシュを削除
void TRSDOS_HIT::DeleteHI(wxUint32 pos)
{
	SetHI(pos, 0);
}

//////////////////////////////////////////////////////////////////////
//
// TRSDOS Base の処理
//
DiskBasicTypeTRSDOS::DiskBasicTypeTRSDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir), TRSDOS_HIT()
{
	if (basic->GetGroupsPerTrack() <= 0) {
		basic->SetGroupsPerTrack(basic->GetSectorsPerTrackOnBasic() / basic->GetSectorsPerGroup());
	}
}

DiskBasicTypeTRSDOS::~DiskBasicTypeTRSDOS()
{
}

/// エリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeTRSDOS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// GATエリア
	int sec = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec);
	if (!sector) {
		return -1.0;
	}
	trsdos_gat_t *gat_sector = (trsdos_gat_t *)sector->GetSectorBuffer();

	gat_table = TRSDOS_GAT(gat_sector->gat, sizeof(gat_sector->gat), basic->GetGroupsPerTrack());
	tlt_table = TRSDOS_GAT(gat_sector->tlt, sizeof(gat_sector->tlt), basic->GetGroupsPerTrack());

	sector = basic->GetSectorFromSectorPos(sec+1);
	if (!sector) {
		return -1.0;
	}
	AssignHIT(sector->GetSectorBuffer(), sector->GetSectorBufferSize());

	basic->SetSectorsPerFat(1);

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeTRSDOS::ParseParamOnDisk(bool is_formatting)
{
	if (is_formatting) return 1.0;

	double valid_ratio = 1.0;

	// GATエリアにあるボリューム名
	int sec = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec);
	if (!sector) {
		return -1.0;
	}
	trsdos_gat_t *gat_sector = (trsdos_gat_t *)sector->GetSectorBuffer();
	wxString volname(gat_sector->name, sizeof(gat_sector->name));
	if (!volname.IsAscii()) {
		return -1.0;
	}

	basic->SetFatEndGroup(basic->GetSidesPerDiskOnBasic() * basic->GetTracksPerSideOnBasic() * basic->GetGroupsPerTrack() - 1);

	return valid_ratio;
}


/// Allocation Mapの開始位置を得る（ダイアログ用）
void DiskBasicTypeTRSDOS::GetStartNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	int sec = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	GetNumFromSectorPos(sec, track_num, side_num, sector_num);
}

/// Allocation Mapの終了位置を得る（ダイアログ用）
void DiskBasicTypeTRSDOS::GetEndNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	int sec = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() + 1;
	GetNumFromSectorPos(sec, track_num, side_num, sector_num);
}

/// タイトル名（ダイアログ用）
wxString DiskBasicTypeTRSDOS::GetTitleForFat() const
{
	return _("Allocation Map");
}

#if 0
/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
bool DiskBasicTypeTRSDOS::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	bool valid = true;

	group_items.Empty();

	// ディレクトリのチェインをたどる
	size_t dir_size = 0;
	int limit = basic->GetDirEndSector() - basic->GetDirStartSector() + 1;
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;

	// 開始セクタ
	int sector_pos = basic->GetDirStartSector();
//	voldir.Empty();

	while(valid && limit >= 0) {
		trsdos_dir_ptr_t next;
		next.next_block = 0;

		wxUint32 group_num = (wxUint32)(sector_pos / basic->GetSectorsPerGroup());
//		voldir.Add((int)group_num);

		for(int ss = 0; ss < basic->GetSectorsPerGroup(); ss++) {
			DiskD88Sector *sector = basic->GetSectorFromSectorPos(sector_pos, trk_num, sid_num);
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
			if (ss == 0) {
				// 次のブロックへのポインタを保持
				memcpy(&next, buffer, sizeof(trsdos_dir_ptr_t));
			}

			group_items.Add(group_num, 0, trk_num, sid_num, sec_num, sec_num);

			dir_size += sector->GetSectorSize();
			sector_pos++;
		}

		// 次のセクタなし
		if (next.next_block == 0) {
			break;
		}

		sector_pos = (int)(wxUINT16_SWAP_ON_BE(next.next_block) * basic->GetSectorsPerGroup());

		limit--;
	}
	group_items.SetSize(dir_size);

	if (limit < 0) {
		valid = false;
	}

	return valid;
}

/// ディレクトリエリアのサイズに達したらアサイン終了するか
/// @param[in,out] pos         ディレクトリの位置
/// @param[in,out] size        ディレクトリのセクタサイズ
/// @param[in,out] size_remain ディレクトリの残りサイズ
/// @retval  0 : 終了しない
/// @retval -1 : このブロックでのアサイン終了 次のブロックへ
int DiskBasicTypeTRSDOS::FinishAssigningDirectory(int &pos, int &size, int &size_remain) const
{
	// サイズに達したら以降は未使用とする
	int blk_size = basic->GetSectorSize() * basic->GetSectorsPerGroup();
	return ((size_remain % blk_size) < (int)sizeof(directory_trsd23_t) ? -1 : 0);
}

/// セクタをディレクトリとして初期化
/// @param [in]     group_items 確保したセクタリスト
/// @param [in,out] file_size   サイズ ディレクトリを拡張した時は既存サイズに加算
/// @param [in,out] size_remain 残りサイズ
/// @param [in,out] errinfo     エラー情報
/// @return 0:正常 <0:エラー 
int DiskBasicTypeTRSDOS::InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &size_remain, DiskBasicError &errinfo)
{
	file_size = (int)group_items.Count() * basic->GetSectorSize();
	size_remain = 0;

	return 0;
}
#endif

/// 使用可能なディスクサイズを得る
void DiskBasicTypeTRSDOS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
void DiskBasicTypeTRSDOS::CalcDiskFreeSize(bool wrote)
{
//	wxUint32 fsize = 0;
//	wxUint32 grps = 0;

	fat_availability.Empty();

	// GAT & TLT
	if (!gat_table.GetBuffer()) return;
	if (!tlt_table.GetBuffer()) return;

	int mng_trk = basic->GetManagedTrackNumber();
	wxUint32 mng_start = mng_trk * basic->GetGroupsPerTrack();
	wxUint32 mng_end = mng_trk * basic->GetGroupsPerTrack() + basic->GetGroupsPerTrack() - 1;

	int group_size = basic->GetSectorsPerGroup() * basic->GetSectorSize();
	wxUint32 max_group = basic->GetFatEndGroup();

	for(wxUint32 grp = 0; grp <= max_group; grp++) {
		if (mng_start <= grp && grp <= mng_end) {
			fat_availability.Add(FAT_AVAIL_SYSTEM, 0, 0);
		} else if (tlt_table.IsSet(grp)) {
			fat_availability.Add(FAT_AVAIL_USED, 0, 0);
		} else {
			if (gat_table.IsSet(grp)) {
				fat_availability.Add(FAT_AVAIL_USED, 0, 0);
			} else {
				fat_availability.Add(FAT_AVAIL_FREE, group_size, 1);
			}
		}
	}


//	free_disk_size = (int)fsize;
//	free_groups = (int)grps;
}

/// グループ番号を使用済みにする
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeTRSDOS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	gat_table.Modify(num, val != INVALID_GROUP_NUMBER);
}

/// グループ番号を得る
wxUint32 DiskBasicTypeTRSDOS::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeTRSDOS::IsUsedGroupNumber(wxUint32 num)
{
	return gat_table.IsSet(num) || tlt_table.IsSet(num);
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeTRSDOS::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeTRSDOS::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	wxUint32 mng_grp_sta = basic->GetManagedTrackNumber() * basic->GetGroupsPerTrack();
	wxUint32 mng_grp_end = mng_grp_sta + basic->GetGroupsPerTrack() - 1;

	for(wxUint32 grp = 0; grp <= basic->GetFatEndGroup(); grp++) {
		if ((grp < mng_grp_sta || mng_grp_end < grp) && !IsUsedGroupNumber(grp)) {
			new_num = grp;
			break;
		}
	}
	return new_num;
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeTRSDOS::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	// 次の空き位置候補
	return GetEmptyGroupNumber();
}

#if 0
/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeTRSDOS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
//	// チェインセクタをクリア
//	nitem->ClearChainSector();

	return true;
}

/// チェインセクタを確保する
/// @param [in]  idx          チェイン番号
/// @param [in]  item         ディレクトリアイテム
/// @return 確保したグループ番号 / エラー時 INVALID_GROUP_NUMBER
wxUint32 DiskBasicTypeTRSDOS::AllocChainSector(int idx, DiskBasicDirItem *item)
{
	wxUint32 gnum = GetEmptyGroupNumber();
	if (gnum == INVALID_GROUP_NUMBER) {
		return INVALID_GROUP_NUMBER;
	}
	// セクタ
	int st_pos = GetStartSectorFromGroup(gnum);
	int ed_pos = GetEndSectorFromGroup(gnum, 0, st_pos, 0, 0);
	for(int sec = st_pos; sec <= ed_pos; sec++) {
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec);
		if (!sector) {
			return INVALID_GROUP_NUMBER;
		}
		sector->Fill(0);
	}

	// チェイン情報にセクタをセット
	item->SetChainSector(gnum, st_pos, NULL);

	// 開始グループを設定
	if (idx == 0) {
		item->SetStartGroup(0, gnum, 1);
	}
	// セクタを予約
	SetGroupNumber(gnum, 1);

	return gnum;
}
#endif

#if 0
/// グループをつなげる
/// @return 0 正常
int DiskBasicTypeTRSDOS::ChainDirectoryGroups(DiskBasicDirItem *item, DiskBasicGroups &group_items)
{
	DiskBasicGroups orig_group_items;
	item->GetAllGroups(orig_group_items);
	orig_group_items.Add(group_items);
	group_items = orig_group_items;

	// ディレクトリチェインを再作成
	wxUint32 group_num = INVALID_GROUP_NUMBER;
	wxUint32 prev_group_num = INVALID_GROUP_NUMBER;
	trsdos_dir_ptr_t *prev = NULL;
	for(size_t i=0; i<group_items.Count(); i++) {
		DiskBasicGroupItem *item = &group_items.Item(i);
		if (item->group != group_num) {
			group_num = item->group;
			DiskD88Sector *sector = basic->GetSectorFromGroup(group_num);
			trsdos_dir_ptr_t *curr = (trsdos_dir_ptr_t *)sector->GetSectorBuffer();

			curr->prev_block = prev_group_num != INVALID_GROUP_NUMBER ? prev_group_num : 0;
			curr->prev_block = wxUINT16_SWAP_ON_BE(curr->prev_block);

			if (prev) {
				prev->next_block = group_num;
				prev->next_block = wxUINT16_SWAP_ON_BE(prev->next_block);
			}

			prev = curr;
			prev_group_num = group_num;
		}
	}

	return 0;
}
#endif

#if 0
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
int DiskBasicTypeTRSDOS::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	int size = sector_size < remain_size ? sector_size : remain_size;

	if (ostream) {
		// 書き出し
		temp.SetData(sector_buffer, size, basic->IsDataInverted());
		ostream->Write(temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read(temp.GetData(), temp.GetSize());
		temp.InvertData(basic->IsDataInverted());

		if (memcmp(temp.GetData(), sector_buffer, size) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}
#endif

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeTRSDOS::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	return remain_size;
}

/// グループ番号からセクタ番号を得る
/// @param [in] group_num グループ番号
int DiskBasicTypeTRSDOS::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num * basic->GetSectorsPerGroup();
}

/// グループ番号から最終セクタ番号を得る
/// @param [in] group_num    グループ番号
/// @param [in] next_group   未使用
/// @param [in] sector_start 未使用
/// @param [in] sector_size  未使用
/// @param [in] remain_size  未使用
int DiskBasicTypeTRSDOS::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return (group_num + 1) * basic->GetSectorsPerGroup() - 1;
}

#if 0
/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num 物理セクタ番号
/// @param [out] div_num    分割番号
/// @param [out] div_nums   分割数
void DiskBasicTypeTRSDOS::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sectors_per_track = sides_per_disk;

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
void DiskBasicTypeTRSDOS::GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num)
{
	int sectors_per_track = 1;

//	sector_map.GetNumFromSectorPos(sector_pos, track_num, sector_num, sectors_per_track);

	// マッピング
//	sector_num = sector_skew.ToPhysical(sector_num);

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBase();
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  物理セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeTRSDOS::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
//	int selected_side = basic->GetSelectedSide();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = 1;
//	int sector_pos;

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	// マッピング
//	sector_num = sector_skew.ToLogical(sector_num);

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

//	int sector_pos = sector_map.GetSectorPosFromNum(track_num, sector_num, sectors_per_track);

	// 連番でない場合
	if (numbering_sector != 1) {
//		sector_pos += side_num * sectors_per_track / sides_per_disk;
	}

//	if (selected_side >= 0) {
//		// 1S
//		sector_pos = track_num * sectors_per_track + sector_num;
//	} else {
//		// 2D, 2HD
//		sector_pos = track_num * sectors_per_track * sides_per_disk;
//		sector_pos += (side_num % sides_per_disk) * sectors_per_track;
//		sector_pos += sector_num;
//	}
	return 0;
}

/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] sector_num 論理セクタ番号
/// @return 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeTRSDOS::GetSectorPosFromNumS(int track_num, int sector_num)
{
//	int selected_side = basic->GetSelectedSide();
//	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
//	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	int sectors_per_track = 1;

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBase();

	// マッピング
//	sector_num = sector_skew.ToLogical(sector_num);

//	sector_pos = sector_map.GetSectorPosFromNum(track_num, sector_num, sectors_per_track);

	return sector_pos;
}
#endif

#if 0
/// ルートディレクトリか
bool DiskBasicTypeTRSDOS::IsRootDirectory(wxUint32 group_num)
{
	return false;
}

/// サブディレクトリを作成する前にディレクトリ名を編集する
bool DiskBasicTypeTRSDOS::RenameOnMakingDirectory(wxString &dir_name)
{
	// 名前が空は作成不可
	if (dir_name.IsEmpty()) {
		return false;
	}
	return true;
}

/// サブディレクトリを作成した後の個別処理
/// @param[in] item        アイテム
/// @param[in] group_items グループ番号
/// @param[in] parent_item 親ディレクトリ
void DiskBasicTypeTRSDOS::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	if (group_items.Count() <= 0) return;

	int block_size = basic->GetSectorSize() * basic->GetSectorsPerGroup();

	DiskBasicDirItemTRSDOS *ditem = (DiskBasicDirItemTRSDOS *)item;
	DiskBasicDirItemTRSDOS *parent_ditem = (DiskBasicDirItemTRSDOS *)parent_item;

	// ディレクトリの最初のブロックをセット
	item->SetParentGroup(parent_item->GetStartGroup(0));
	// バージョンはヘッダと合わせる
//	ditem->SetVersion(parent_ditem->GetVersion());

	// サブボリュームヘッダのエントリを作成する

	DiskBasicGroupItem *gitem = &group_items.Item(0);

	DiskD88Sector *sector = basic->GetSector(gitem->track, gitem->side, gitem->sector_start);

	wxUint8 *buf = sector->GetSectorBuffer(4);
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, 0, buf);
	DiskBasicDirItemTRSDOS *newditem = (DiskBasicDirItemTRSDOS *)newitem;

	newitem->CopyData(item->GetData());
//	newitem->SetFileAttr(FORMAT_TYPE_PRODOS, 0, FILETYPE_MASK_PRODOS_SUBVOL << 16 | 0x75 << 8 | (FILETYPE_MASK_PRODOS_ACCESS_ALL & ~FILETYPE_MASK_PRODOS_CHANGE));
	newitem->SetStartGroup(0, 0);
	newitem->SetFileSize(0);
	// バージョン
////	newditem->SetVersion(parent_ditem->GetVersion());

	directory_trsd23_t *vol = (directory_trsd23_t *)buf;

	// エントリのサイズ
	vol->sv.entry_len = (wxUint8)sizeof(directory_trsd23_t);
	// ブロック内のファイルエントリ数
	vol->sv.entries_per_block = (wxUint8)((block_size - 4) / (int)sizeof(directory_trsd23_t));
	// ファイルエントリ数
	vol->sv.file_count = 0;

	int parent_start_block = (int)parent_item->GetStartGroup(0);
	int item_number = item->GetNumber();
	int parent_pointer = (item_number / (int)vol->sv.entries_per_block) + parent_start_block;
	int parent_entry = item_number % (int)vol->sv.entries_per_block;

	// 親のブロック番号
	vol->sv.parent_pointer = wxUINT16_SWAP_ON_BE(parent_pointer);
	// 親エントリ
	vol->sv.parent_entry = (wxUint8)parent_entry;

	// 親エントリのサイズ
	vol->sv.parent_entry_len = (wxUint8)sizeof(directory_trsd23_t);

	delete newitem;
}
#endif
#if 0
/// フォーマット時セクタデータを指定コードで埋める
/// @param[in] track  トラック
/// @param[in] sector セクタ
void DiskBasicTypeTRSDOS::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}
#endif

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeTRSDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskD88Sector *sector;

	// GAT
	int st_pos = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() + basic->GetDirStartSector() - 2;
	sector = basic->GetSectorFromSectorPos(st_pos);
	if (!sector) {
		// Why?
		return false;
	}
	sector->Fill(basic->GetFillCodeOnFAT());

	trsdos_gat_t *gat_sector = (trsdos_gat_t *)sector->GetSectorBuffer();

	// GAT set free area
	wxUint32 mnt_st_grp = basic->GetManagedTrackNumber() * basic->GetGroupsPerTrack() * basic->GetSidesPerDiskOnBasic();
	wxUint32 mnt_ed_grp = mnt_st_grp + basic->GetGroupsPerTrack() - 1;
	for(wxUint32 grp = 0; grp <= basic->GetFatEndGroup(); grp++) {
		if (grp < mnt_st_grp || mnt_ed_grp < grp) {
			gat_table.Modify(grp, false);
		}
	}

	// 日付
	DiskBasicIdentifiedData ndata = data;
	TM tm;
	if (!Utils::ConvDateStrToTm(data.GetVolumeDate(), tm)) {
		// 現在日付をセット
		tm.Now();
		ndata.SetVolumeDate(Utils::FormatYMDStr(tm));
	}

	// ボリューム名を設定
	SetIdentifiedData(ndata);

	// APT
	memset(gat_sector->apt, 0x20, sizeof(gat_sector->apt));
	gat_sector->apt[0] = 0x0d;

	// HIT, FDE ディレクトリ
	st_pos++;
	int ed_pos = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() + basic->GetDirEndSector();
	for(int sec = st_pos; sec <= ed_pos; sec++) {
		sector = basic->GetSectorFromSectorPos(sec);
		if (!sector) {
			// Why?
			return false;
		}
		sector->Fill(basic->GetFillCodeOnDir());
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
/// @param [in]  seq_num		通し番号(0...)
/// @return 書き込んだバイト数
int DiskBasicTypeTRSDOS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
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

	return len;
}

#if 0
/// データの書き込み終了後の処理
/// @param [in]	 item			ディレクトリアイテム
void DiskBasicTypeTRSDOS::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	DiskBasicDirItemTRSDOS *ditem = (DiskBasicDirItemTRSDOS *)item;

	// ディレクトリのヘッダにあるファイル数を＋１する
	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) {
		// Why?
		return;
	}
	DiskBasicDirItems *children = parent->GetChildren();
	if (!children) {
		// Why?
		return;
	}
	DiskBasicDirItemTRSDOS *vol = (DiskBasicDirItemTRSDOS *)children->Item(0);
	if (!vol) {
		// Why?
		return;
	}
//	vol->IncreaseFileCount();
	// ディレクトリの最初のブロックをセット
	item->SetParentGroup(parent->GetStartGroup(0));
	// バージョンはヘッダと合わせる
//	ditem->SetVersion(vol->GetVersion());
}
#endif

/// FAT領域を削除する
void DiskBasicTypeTRSDOS::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, INVALID_GROUP_NUMBER);
}

#if 0
/// ファイル削除後の処理
bool DiskBasicTypeTRSDOS::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// チェインセクタを未使用にする
	item->ClearChainSector();

	// ディレクトリのヘッダにあるファイル数を－１する
	DiskBasicDirItem *parent = item->GetParent();
	if (!parent) {
		// Why?
		return true;
	}
	DiskBasicDirItems *children = parent->GetChildren();
	if (!children) {
		// Why?
		return true;
	}
	DiskBasicDirItemTRSDOS *vol = (DiskBasicDirItemTRSDOS *)children->Item(0);
	if (!vol) {
		// Why?
		return true;
	}
//	vol->DecreaseFileCount();

	return true;
}
#endif

/// IPLや管理エリアの属性を得る
void DiskBasicTypeTRSDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// GATエリア
	int sec = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec);
	if (!sector) {
		return;
	}
	trsdos_gat_t *gat_sector = (trsdos_gat_t *)sector->GetSectorBuffer();

	// volume name
	wxString wname;
	basic->GetCharCodes().ConvToString(gat_sector->name, sizeof(gat_sector->name), wname);
	wname.Trim();
	data.SetVolumeName(wname);
	data.SetVolumeNameMaxLength(sizeof(gat_sector->name));
	// volume date
	TM tm;
	int yy = (gat_sector->date[6] & 0xf)*10+(gat_sector->date[7] & 0xf);
	if (yy < 70) yy += 100;
	tm.SetYear(yy);
	tm.SetMonth((gat_sector->date[0] & 0xf)*10+(gat_sector->date[1] & 0xf)-1);
	tm.SetDay((gat_sector->date[3] & 0xf)*10+(gat_sector->date[4] & 0xf));
	data.SetVolumeDate(Utils::FormatYMDStr(tm));
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeTRSDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	// GATエリア
	int sec = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec);
	if (!sector) {
		return;
	}
	trsdos_gat_t *gat_sector = (trsdos_gat_t *)sector->GetSectorBuffer();

	// volume name
	wxCharBuffer volname = data.GetVolumeName().Upper().To8BitData();
	size_t len = sizeof(gat_sector->name);
	if (len > volname.length()) len = volname.length();
	memset(gat_sector->name, basic->GetDirSpaceCode(), sizeof(gat_sector->name));
	memcpy(gat_sector->name, volname.data(), len);
	// volume date
	TM tm;
	Utils::ConvDateStrToTm(data.GetVolumeDate(), tm);
	gat_sector->date[0] = ((tm.GetMonth() + 1) / 10) + 0x30;
	gat_sector->date[1] = ((tm.GetMonth() + 1) % 10) + 0x30;
	gat_sector->date[2] = '/';
	gat_sector->date[3] = (tm.GetDay() / 10) + 0x30;
	gat_sector->date[4] = (tm.GetDay() % 10) + 0x30;
	gat_sector->date[5] = '/';
	gat_sector->date[6] = ((tm.GetYear() / 10) % 10) + 0x30;
	gat_sector->date[7] = (tm.GetYear() % 10) + 0x30;
}

//////////////////////////////////////////////////////////////////////
//
// TRSDOS 2.x の処理
//
DiskBasicTypeTRSD23::DiskBasicTypeTRSD23(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeTRSDOS(basic, fat, dir)
{
}

/// ハッシュの格納位置を得る
wxUint32 DiskBasicTypeTRSD23::GetHIPosition(int sector_num, int pos_in_sector)
{
	// 下位4バイトがセクタ
	int pos = (sector_num - 2);
	// 上位4バイトが位置
	pos |= ((pos_in_sector * 2) << 4);

	return (pos & 0xff);
}

/// ハッシュの格納位置からセクタ番号と位置を得る
void DiskBasicTypeTRSD23::GetFromHIPosition(wxUint32 pos, int &sector_num, int &pos_in_sector)
{
	sector_num = (pos & 0xf) + 2;
	pos_in_sector = ((pos & 0xf0) >> 4) / 2;
}

/// ハッシュを計算
/// @param[in]  name ファイル名＋拡張子 11バイト
/// @return ハッシュ値
wxUint8 DiskBasicTypeTRSD23::ComputeHI(const wxUint8 *name)
{
	wxUint16 a;
	wxUint8 c = 0;		// HR.
	for(int b = 0; b < 11; b++) {
		a = name[b];	// get one char
		a = (a ^ c);	// a = (HR. XOR new char)
		a <<= 1;		// a = a * 2
		c = (a & 0xff) | ((a & 0x100) >> 8);	// new HR.
	}
	if (c == 0) c = 1;	// HR don't allow zero.

	return c;
}

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicTypeTRSD23::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool sts = DiskBasicType::AssignRootDirectory(start_sector, end_sector, group_items, dir_item);

	// overflowエントリとprimaryエントリを関連付ける
	DiskBasicDirItems citems = *dir_item->GetChildren();
	for(size_t i = 0; i < citems.Count(); i++) {
		DiskBasicDirItemTRSDOS *citem = (DiskBasicDirItemTRSDOS *)citems.Item(i);
		int ov = citem->GetOverflow();
		if (ov > 0 && ov < 254) {
			// 非表示
			citem->Visible(false);
			// 参照元アイテムと関連付ける
			int ov_sec_num = 0;
			int ov_sec_pos = 0;
			GetFromHIPosition(ov, ov_sec_num, ov_sec_pos);
			// 通し番号を計算
			int num = (ov_sec_num - 2) * basic->GetSectorSize() / citem->GetDataSize() + ov_sec_pos;
			if (num >= (int)citems.Count()) {
				// invalid chain
				sts = false;
				return sts;
			}
			DiskBasicDirItemTRSDOS *pitem = (DiskBasicDirItemTRSDOS *)citems.Item(num);
			pitem->SetNextItem(citem);
		}
	}

	// ファイルサイズを再計算
	for(size_t i = 0; i < citems.Count(); i++) {
		DiskBasicDirItemTRSDOS *citem = (DiskBasicDirItemTRSDOS *)citems.Item(i);
		citem->CalcFileSize();
	}
	return sts;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeTRSD23::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int groups = 0; 

	int  rc = 0;
//	bool first_group = (flags == ALLOCATE_GROUPS_NEW);
	int sizeremain = data_size;

	int bytes_per_group = basic->GetSectorsPerGroup() * basic->GetSectorSize();
	int limit = basic->GetFatEndGroup() + 1;
	int pos_max = 4;
	while(rc >= 0 && limit >= 0 && sizeremain > 0) {
		wxUint32 group_num = GetEmptyGroupNumber();
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			rc = -1;
			break;
		}
		// 位置を予約
		SetGroupNumber(group_num, 1);

		basic->GetNumsFromGroup(group_num, 0, basic->GetSectorSize(), sizeremain, group_items);

		sizeremain -= bytes_per_group;
		groups++;

		limit--;
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = -1;
	}

	if (rc >= 0) {
		DiskBasicDirItemTRSDOS *titem = (DiskBasicDirItemTRSDOS *)item;

		// 使用中にする
		titem->SetAsNewFile();

		// ディレクトリエントリに追加
		int pos = 0;
		wxUint32 pre_grp = 0;
		wxUint32 sta_grp = 0;
		int cnt = 0;
		int max_idx = (int)group_items.Count();
		for(int idx = 0; idx < max_idx; idx++) {
			wxUint32 grp = group_items.Item(idx).group;
			if (idx == 0) {
				sta_grp = grp;
			} else if (cnt > 32 || pre_grp + 1 != grp) {
				if (pos >= pos_max && CreateOverflowEntry(&titem, pos) < 0) {
					rc = -1;
					break;
				}
				titem->SetGranulesOnGap(pos, sta_grp, cnt);
				cnt = 0;
				sta_grp = grp;
				pos++;
			}
			cnt++;
			pre_grp = grp;
		}
		if (rc >= 0 && cnt > 0) {
			if (pos >= pos_max && CreateOverflowEntry(&titem, pos) < 0) {
				rc = -1;
			}
			if (rc >= 0) {
				titem->SetGranulesOnGap(pos, sta_grp, cnt);
				pos++;
			}
		}
	}

	if (rc < 0) {
		// グループを削除
		DeleteGroups(group_items);
	}

	return rc;
}

/// Overflowエントリを作成する
int DiskBasicTypeTRSD23::CreateOverflowEntry(DiskBasicDirItemTRSDOS **ptitem, int &pos)
{
	int rc = 0;
	// 新しいディレクトリエントリを作る
	DiskBasicDirItemTRSDOS *new_titem = (DiskBasicDirItemTRSDOS *)dir->GetEmptyItemOnCurrent(*ptitem, NULL);
	if (new_titem == NULL) {
		// 空きなし
		rc = -1;
		return rc;
	} else {
		// 元エントリの最終を0xfeにする
		wxUint32 ccnt;
		wxUint32 cgrp = (*ptitem)->GetGranulesOnGap(4, &ccnt);
		(*ptitem)->ClearGranulesOnGap(4, 0xfe, new_titem->GetPositionInHIT());

		wxUint8 pos_in_hit = (*ptitem)->GetPositionInHIT(); 
		new_titem->SetAsOverflowFile(pos_in_hit, GetHI(pos_in_hit));

		pos = 0;
		new_titem->SetGranulesOnGap(pos, cgrp, ccnt);
		pos++;

		(*ptitem)->SetNextItem(new_titem);

		*ptitem = new_titem;
	}
	return rc;
}

/// 未使用のディレクトリアイテムを返す
/// @param [in,out] parent    ディレクトリ
/// @param [in,out] items     ディレクトリアイテム一覧
/// @param [in,out] pitem     ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out]    next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicTypeTRSD23::GetEmptyDirectoryItem(DiskBasicDirItem *WXUNUSED(parent), DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item)
{
	// システム属性がついているか
	bool is_sys = ((pitem->GetFileAttr().GetOrigin() & FILETYPE_MASK_TRSDOS_SYSTEM) != 0);

	DiskBasicDirItem *match_item = NULL;
	if (items) {
		for(size_t i=0; i < items->Count(); i++) {
			DiskBasicDirItem *item = items->Item(i);
			if (!item->IsUsed()) {
				// システムファイルは各セクタの上位２つを候補にする
				if ((is_sys && item->GetPosition() < 0x40)
				 || (!is_sys && item->GetPosition() >= 0x40)) { 
					match_item = item;
					if (next_item) {
						i++;
						if (i < items->Count() && !items->Item(i)->IsUsed()) {
							*next_item = items->Item(i);
						} else {
							*next_item = NULL;
						}
					}
					break;
				}
			}
		}
	}
	return match_item;
}

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeTRSD23::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	if (!DiskBasicTypeTRSDOS::AdditionalProcessOnFormatted(data)) {
		return false;
	}

	DiskD88Sector *sector;

	// GAT
	int st_pos = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() + basic->GetDirStartSector() - 2;
	sector = basic->GetSectorFromSectorPos(st_pos);
	if (!sector) {
		// Why?
		return false;
	}

	// TLT no lock out
	for(wxUint32 grp = 0; grp <= basic->GetFatEndGroup(); grp++) {
		tlt_table.Modify(grp, false);
	}

	trsdos_gat_t *gat_sector = (trsdos_gat_t *)sector->GetSectorBuffer();

	// ボリュームパスワード
	gat_sector->password = wxUINT16_SWAP_ON_BE(0x4296);

	DiskBasicDirItemTRSDOS *titem;

	// "BOOT/SYS"エントリを作る
	st_pos += 2;
	sector = basic->GetSectorFromSectorPos(st_pos);
	titem = (DiskBasicDirItemTRSDOS *)dir->NewItem(sector, 0, sector->GetSectorBuffer(0));
	titem->SetAsBootSysEntry();
	delete titem;

	// "DIR/SYS"エントリを作る
	st_pos++;
	sector = basic->GetSectorFromSectorPos(st_pos);
	titem = (DiskBasicDirItemTRSDOS *)dir->NewItem(sector, 0, sector->GetSectorBuffer(0));
	titem->SetAsDirSysEntry();
	delete titem;

	// セクタ０
	sector = basic->GetSectorFromSectorPos(0);
	if (!sector) {
		// Why?
		return false;
	}
	sector->Copy("\x00\xfe\x11\xf3", 4);
	gat_table.Modify(0, true);

	return true;
}

//////////////////////////////////////////////////////////////////////
//
// TRSDOS 1.3 の処理
//
DiskBasicTypeTRSD13::DiskBasicTypeTRSD13(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeTRSDOS(basic, fat, dir)
{
}

/// ハッシュの格納位置を得る
wxUint32 DiskBasicTypeTRSD13::GetHIPosition(int pos)
{
	return (pos & 0xff);
}

/// ハッシュの格納位置からセクタ番号と位置を得る
void DiskBasicTypeTRSD13::GetFromHIPosition(wxUint32 pos, int &sector_num, int &pos_in_sector)
{
	int n = basic->GetSectorSize() / (int)sizeof(directory_trsd13_t);
	sector_num = pos / n; 
	pos_in_sector = (pos % n);
}

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicTypeTRSD13::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	bool sts = DiskBasicType::AssignRootDirectory(start_sector, end_sector, group_items, dir_item);

	// ファイルサイズを再計算
	DiskBasicDirItems citems = *dir_item->GetChildren();
	for(size_t i = 0; i < citems.Count(); i++) {
		DiskBasicDirItemTRSDOS *citem = (DiskBasicDirItemTRSDOS *)citems.Item(i);
		citem->CalcFileSize();
	}
	return sts;
}

/// ディレクトリエリアのサイズに達したらアサイン終了するか
/// @param[in,out] pos         ディレクトリの位置
/// @param[in,out] size        ディレクトリのセクタサイズ
/// @param[in,out] size_remain ディレクトリの残りサイズ
/// @retval  0 : 終了しない
/// @retval  1 : 強制的に未使用とする アサインは継続
/// @retval -1 : 現グループでアサイン終了。次のグループから継続
/// @retval -2 : 強制的にアサイン終了する
int DiskBasicTypeTRSD13::FinishAssigningDirectory(int &pos, int &size, int &size_remain) const
{
	DiskBasicDirItemTRSD13 tmp(basic, NULL, 0, NULL);
	if (pos + (int)tmp.GetDataSize() > size) {
		return -1;
	}
	return 0;
}

/// ディレクトリアサインでセクタ毎に位置を調整する
/// @param[in] pos ディレクトリの位置
/// @return 調整後のディレクトリの位置
int DiskBasicTypeTRSD13::AdjustPositionAssigningDirectory(int pos)
{
	return 0;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    データサイズ RecalcFileSizeOnSave()で計算した値
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  グループ数
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeTRSD13::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int groups = 0; 

	int  rc = 0;
//	bool first_group = (flags == ALLOCATE_GROUPS_NEW);
	int sizeremain = data_size;

	int bytes_per_group = basic->GetSectorsPerGroup() * basic->GetSectorSize();
	int limit = basic->GetFatEndGroup() + 1;
	int pos_max = 13;
	while(rc >= 0 && limit >= 0 && sizeremain > 0) {
		wxUint32 group_num = GetEmptyGroupNumber();
		if (group_num == INVALID_GROUP_NUMBER) {
			// 空きなし
			rc = -1;
			break;
		}
		// 位置を予約
		SetGroupNumber(group_num, 1);

		basic->GetNumsFromGroup(group_num, 0, basic->GetSectorSize(), sizeremain, group_items);

		sizeremain -= bytes_per_group;
		groups++;

		limit--;
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = -1;
	}

	if (rc >= 0) {
		DiskBasicDirItemTRSDOS *titem = (DiskBasicDirItemTRSDOS *)item;

		// 使用中にする
		titem->SetAsNewFile();

		// ディレクトリエントリに追加
		int pos = 0;
		wxUint32 pre_grp = 0;
		wxUint32 sta_grp = 0;
		int cnt = 0;
		int max_idx = (int)group_items.Count();
		for(int idx = 0; idx < max_idx; idx++) {
			wxUint32 grp = group_items.Item(idx).group;
			if (idx == 0) {
				sta_grp = grp;
			} else if (cnt > 32 || pre_grp + 1 != grp) {
				if (pos >= pos_max) {
					rc = -1;
					break;
				}
				titem->SetGranulesOnGap(pos, sta_grp, cnt);
				cnt = 0;
				sta_grp = grp;
				pos++;
			}
			cnt++;
			pre_grp = grp;
		}
		if (rc >= 0 && cnt > 0) {
			if (pos >= pos_max) {
				rc = -1;
			}
			if (rc >= 0) {
				titem->SetGranulesOnGap(pos, sta_grp, cnt);
				pos++;
			}
		}
	}

	if (rc < 0) {
		// グループを削除
		DeleteGroups(group_items);
	}

	return rc;
}

/// フォーマット時セクタデータを埋めた後の個別処理
bool DiskBasicTypeTRSD13::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	if (!DiskBasicTypeTRSDOS::AdditionalProcessOnFormatted(data)) {
		return false;
	}

	DiskD88Sector *sector;

	// GAT
	int st_pos = basic->GetManagedTrackNumber() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic() + basic->GetDirStartSector() - 2;
	sector = basic->GetSectorFromSectorPos(st_pos);
	if (!sector) {
		// Why?
		return false;
	}

	trsdos_gat_t *gat_sector = (trsdos_gat_t *)sector->GetSectorBuffer();

	// ボリュームパスワード
	gat_sector->password = wxUINT16_SWAP_ON_BE(0x5cef);

	// セクタ０
	sector = basic->GetSectorFromSectorPos(0);
	if (!sector) {
		// Why?
		return false;
	}
	sector->Copy("\xfe\x11\x3e\xd0", 4);
	gat_table.Modify(0, true);

	return true;
}
