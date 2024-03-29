/// @file basictype.cpp
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "../common.h"
#include <wx/stream.h>
#include "basictype.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "../logging.h"


//////////////////////////////////////////////////////////////////////
//
// セクタスキューマップ
//
//////////////////////////////////////////////////////////////////////

SectorSkewBase::SectorSkewBase()
{
	m_num_secs = 0;
}

SectorSkewBase::SectorSkewBase(const SectorSkewBase &src)
{
	m_num_secs = src.m_num_secs;
}

void SectorSkewBase::Create(DiskBasic *basic, int num_secs)
{
	m_num_secs = num_secs;
}

//////////////////////////////////////////////////////////////////////

DiskBasicSectorSkew::DiskBasicSectorSkew()
	: SectorSkewBase()
{
	m_ltop_map = NULL;
	m_ptol_map = NULL;
}

DiskBasicSectorSkew::DiskBasicSectorSkew(const DiskBasicSectorSkew &src)
	: SectorSkewBase(src)
{
	if (src.m_ltop_map) {
		m_ltop_map = new int[src.m_num_secs];
		memcpy(m_ltop_map, src.m_ltop_map, src.m_num_secs);
	}
	if (src.m_ptol_map) {
		m_ptol_map = new int[src.m_num_secs];
		memcpy(m_ptol_map, src.m_ptol_map, src.m_num_secs);
	}
}

DiskBasicSectorSkew::~DiskBasicSectorSkew()
{
	Delete();
}

/// スキューマップを削除する
void DiskBasicSectorSkew::Delete()
{
	delete [] m_ltop_map; m_ltop_map = NULL;
	delete [] m_ptol_map; m_ptol_map = NULL;
}

/// スキューマップを作成する
void DiskBasicSectorSkew::Create(DiskBasic *basic, int num_secs)
{
	SectorSkewBase::Create(basic, num_secs);

	m_ltop_map = new int[m_num_secs];
	m_ptol_map = new int[m_num_secs];

	for(int i=0; i<m_num_secs; i++) {
		m_ltop_map[i] = -1;
		m_ptol_map[i] = -1;
	}

	Mapping(basic);
}

/// パラメータで指定したマップを元にマッピング
void DiskBasicSectorSkew::MappingFromParam(DiskBasic *basic)
{
	// パラメータで指定したマップを使用する
	int psec = 0;
	for(int lsec = 0; lsec < m_num_secs; lsec++) {
		psec = basic->GetSectorSkewMap(lsec);
		m_ltop_map[lsec] = psec;
		if (psec < m_num_secs) m_ptol_map[psec] = lsec;
	}
}

/// パラメータで指定した間隔から計算してマッピング
void DiskBasicSectorSkew::MappingFromCalc(DiskBasic *basic, int skew)
{
	if (skew < 1) skew = 1;

	int psec = 0;
	for(int lsec = 0; lsec < m_num_secs; lsec++) {
		m_ltop_map[lsec] = psec;
		m_ptol_map[psec] = lsec;
		psec += skew;
		if (psec >= m_num_secs) {
			psec -= m_num_secs;
			for(int limit = m_num_secs; m_ptol_map[psec] >= 0 && limit > 0; limit--) {
				psec++;
			}
		}
	}
}

/// マッピング
void DiskBasicSectorSkew::Mapping(DiskBasic *basic)
{
	if (basic->HasSectorSkewMap()) {
		MappingFromParam(basic);
	} else {
		// システムはソフトセクタスキュー(仮想的インターリーブ)を持っている
		MappingFromCalc(basic, basic->GetSectorSkew());
	}
}

/// 物理番号を返す
int DiskBasicSectorSkew::ToPhysical(int val) const
{
	if (m_ltop_map) val = m_ltop_map[val];
	return val;
}

/// 論理番号を返す
int DiskBasicSectorSkew::ToLogical(int val) const
{
	if (m_ptol_map) val = m_ptol_map[val];
	return val;
}

//////////////////////////////////////////////////////////////////////

DiskBasicSectorSkewForSave::DiskBasicSectorSkewForSave()
	: DiskBasicSectorSkew()
{
}

DiskBasicSectorSkewForSave::DiskBasicSectorSkewForSave(const DiskBasicSectorSkewForSave &src)
	: DiskBasicSectorSkew(src)
{
}

/// マッピング
void DiskBasicSectorSkewForSave::Mapping(DiskBasic *basic)
{
	MappingFromCalc(basic, basic->GetVariousIntegerParam(wxT("SectorSkewForSave")));
}

//////////////////////////////////////////////////////////////////////
//
// セクタ位置変換マップリスト
//
//////////////////////////////////////////////////////////////////////

SectorsPerTrack::SectorsPerTrack()
{
	m_num_of_tracks = 0;
	m_num_of_sectors = 0;
	m_total_sectors = 0;
	m_ssmap = NULL;
}

SectorsPerTrack::SectorsPerTrack(const SectorsPerTrack &src)
{
	m_num_of_tracks = src.m_num_of_tracks;
	m_num_of_sectors = src.m_num_of_sectors;
	m_total_sectors = src.m_total_sectors;
	if (src.m_ssmap) {
		m_ssmap = src.m_ssmap->Clone();
	} else {
		m_ssmap = NULL;
	}
}

SectorsPerTrack::SectorsPerTrack(int num_of_tracks, int num_of_sectors, int total_sectors)
{
	m_num_of_tracks = num_of_tracks;
	m_num_of_sectors = num_of_sectors;
	m_total_sectors = total_sectors;
	m_ssmap = NULL;
}

SectorsPerTrack::~SectorsPerTrack()
{
	delete m_ssmap;
}

//int SectorsPerTrack::GetSkewSectorNum(int sector_pos) const
//{
//	return m_ssmap;
//}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfSectorsPerTrack);

/// 可変数セクタマップを作成 トラックごとのセクタ数を集計
void DiskBasicSectorPosTrans::Create(DiskBasic *basic)
{
	Clear();

	const NumSectorsParams *sp = &basic->SectorsPerTrackOnBasicList();
	if (sp->Count() > 0) {
		// BASICパラメータ側の設定を適用
		for (size_t i = 0; i < sp->Count(); i++) {
			const NumSectorsParam *p = &sp->Item(i);
			Add(SectorsPerTrack(p->GetNumberOfTracks(), p->GetSectorsPerTrack() * basic->GetSidesPerDiskOnBasic(), p->GetNumberOfTracks() * p->GetSectorsPerTrack() * basic->GetSidesPerDiskOnBasic()));
		}
	} else if (basic->IsVariableSectorsPerTrack()) {
		// ディスクパラメータを適用
		int num_of_tracks = 0;
		int prev_num_of_sectors = 0;
		int total_sectors = 0;
		int trk = basic->GetTrackNumberBaseOnDisk();
		int trks = basic->GetTracksPerSideOnBasic() + trk;
		for(; trk < trks; trk++) {
			for(int sid = 0; sid < basic->GetSidesPerDiskOnBasic(); sid++) {
				DiskImageTrack *track = basic->GetTrack(trk, sid);
				if (!track) {
					// Why?
					continue;
				}
				// セクタ数/トラック サイド通し
				int num_of_sectors = track->GetSectorsPerTrack() * basic->GetSidesPerDiskOnBasic();
				if (prev_num_of_sectors == 0) {
					total_sectors = 0;
					prev_num_of_sectors = num_of_sectors;
					num_of_tracks = 0;
				} else if (prev_num_of_sectors != num_of_sectors) {
					Add(SectorsPerTrack(num_of_tracks, prev_num_of_sectors, total_sectors));
					total_sectors = 0;
					prev_num_of_sectors = num_of_sectors;
					num_of_tracks = 0;
				}
				total_sectors += num_of_sectors;
			}
			num_of_tracks++;
		}
		Add(SectorsPerTrack(num_of_tracks, prev_num_of_sectors, total_sectors));
	} else {
		// 固定数
		Add(SectorsPerTrack(basic->GetTracksPerSideOnBasic(), basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic(), basic->GetTracksPerSideOnBasic() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic()));
	}
}

/// 指定トラックのアイテムを返す
/// @param [out] track_num  トラック番号(0 .. )
const SectorsPerTrack *DiskBasicSectorPosTrans::FindByTrackNum(int track_num) const
{
	SectorsPerTrack *match = NULL;

	for(size_t i=0; i<Count(); i++) {
		SectorsPerTrack *item = &Item(i);
		if (track_num < item->GetNumOfTracks()) {
			match = item;
			break;
		}
		track_num -= item->GetNumOfTracks();
	}
	return match;
}

/// セクタ数の合計を返す
int DiskBasicSectorPosTrans::GetTotalSectors() const
{
	int val = 0;
	for(size_t i = 0; i < Count(); i++) {
		SectorsPerTrack *item = &Item(i);
		val += item->GetTotalSectors();
	}
	return val;
}

/// 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号(0 .. )
/// @param [out] sector_num 論理セクタ番号(0 .. )
/// @param [out] num_of_sectors セクタ数
void DiskBasicSectorPosTrans::GetNumFromSectorPos(int sector_pos, int &track_num, int &sector_num, int &num_of_sectors) const
{
	track_num = 0;

	// セクタ位置がどのトラックにあるか
	for(size_t i=0; i<Count(); i++) {
		SectorsPerTrack *item = &Item(i);
		if (sector_pos < item->GetTotalSectors()) {
			num_of_sectors = item->GetNumOfSectors();
			track_num += (sector_pos / num_of_sectors);
			sector_num = sector_pos % num_of_sectors;
			break;
		}
		sector_pos -= item->GetTotalSectors();
		track_num += item->GetNumOfTracks();
	}
}

/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号(0 .. )
/// @param [in] sector_num 論理セクタ番号(0 .. )
/// @param [out] num_of_sectors セクタ数
/// @return 論理セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int DiskBasicSectorPosTrans::GetSectorPosFromNum(int track_num, int sector_num, int &num_of_sectors) const
{
	int sector_pos = 0;

	// トラックがどのセクタ位置になるか
	for(size_t i=0; i<Count(); i++) {
		SectorsPerTrack *item = &Item(i);
		if (track_num < item->GetNumOfTracks()) {
			num_of_sectors = item->GetNumOfSectors();
			sector_pos += track_num * num_of_sectors + sector_num;
			break;
		}
		track_num -= item->GetNumOfTracks();
		sector_pos += item->GetTotalSectors();
	}

	return sector_pos;
}

//////////////////////////////////////////////////////////////////////
//
// DISK BASIC 個別の処理テンプレート
//
//////////////////////////////////////////////////////////////////////

/// コンストラクタ
/// @param [in] basic DISK BASIC
/// @param [in] fat   DISK BASIC FAT情報
/// @param [in] dir   DISK BASIC ディレクトリ情報
DiskBasicType::DiskBasicType(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
{
	this->basic = basic;
	this->fat = fat;
	this->dir = dir;

	this->data_start_group = 0;
}
/// デストラクタ
DiskBasicType::~DiskBasicType()
{
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicType::SetGroupNumber(wxUint32 num, wxUint32 val)
{
}

/// FAT位置を返す
/// @param [in] num グループ番号(0...)
/// @return グループ番号
wxUint32 DiskBasicType::GetGroupNumber(wxUint32 num) const
{
	return 0;
}

/// 使用しているグループ番号か
/// @param [in] num グループ番号(0...)
/// @return true 使用している
bool DiskBasicType::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
/// @param [in] num        グループ番号(0...)
/// @param [in] sector_pos セクタ位置
/// @return 次のグループ番号 / INVALID_GROUP_NUMBER 空きなし
wxUint32 DiskBasicType::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return 0;
}

/// 空きFAT位置を返す
/// @return INVALID_GROUP_NUMBER 空きなし
wxUint32 DiskBasicType::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	// 若い番号順に検索
	for(wxUint32 num = 0; num <= basic->GetFatEndGroup(); num++) {
		wxUint32 gnum = GetGroupNumber(num);
		if (gnum == basic->GetGroupUnusedCode()) {
			new_num = num;
			break;
		}
	}
	return new_num;
}

/// 次の空きFAT位置を返す
/// @param [in] curr_group グループ番号(0...)
/// @return INVALID_GROUP_NUMBER 空きなし
wxUint32 DiskBasicType::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	// 同じトラックでグループが連続するように検索
	int secs_per_grp = basic->GetSectorsPerGroup();
	int secs_per_trk = basic->GetSectorsPerTrackOnBasic();
	int sides = basic->GetSidesPerDiskOnBasic();

	int sed = secs_per_trk * sides / secs_per_grp;
	int group_max = (basic->GetFatEndGroup() / sed) + 1;
	int group_manage = managed_start_group / sed;
	int group_start = curr_group / sed;
	int group_end;
	int dir;
	int sst = 0; // curr_group % sed; // なるべく同じトラックを優先して埋める
	bool found = false;

	// 管理エリアより大きいグループは+方向、小さいグループなら-方向に検索
	dir = (group_start >= group_manage ? 1 : -1);

	for(int i=0; i<2; i++) {
		group_end = (dir > 0 ? group_max : -1);
		for(int g = group_start; g != group_end; g += dir) {
			for(int s = sst; s < sed; s++) {
				wxUint32 num = g * sed + s;
				if (num > basic->GetFatEndGroup()) {
					break;
				}
				wxUint32 gnum = GetGroupNumber(num);
//				myLog.SetDebug("DiskBasicType::GetNextEmptyGroupNumber num:0x%03x gnum:0x%03x", num, gnum);
				if (gnum == basic->GetGroupUnusedCode()) {	// 0xff
					new_num = num;
					found = true;
					break;
				}
			}
			if (found) break;
			sst = 0;
		}
		if (found) break;
		dir = -dir;
		group_start = group_manage;
	}
	return new_num;
}

/// FATの開始位置を得る（ダイアログ用）
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetStartNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	int sec_pos = basic->GetFatStartSector() - 1;
	int sec_fat = basic->GetSectorsPerFat();
	DiskImageTrack *track = NULL;
	if (sec_pos >= 0 && sec_fat > 0) {
		if (basic->GetFatSideNumber() >= 0) sec_pos += basic->GetFatSideNumber() * basic->GetSectorsPerTrackOnBasic();
		track = basic->GetManagedTrack(sec_pos, &side_num, &sector_num);
	}
	if (track) {
// GetManagedTrack内で反転する
//		side_num = basic->GetReversedSideNumber(side_num);
		track_num = track->GetTrackNumber();
	} else {
		track_num = -1;
	}
}

/// FATの終了位置を得る（ダイアログ用）
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetEndNumOnFat(int &track_num, int &side_num, int &sector_num)
{
	int sec_sta = basic->GetFatStartSector() - 1;
	int sec_pos = sec_sta + basic->GetSectorsPerFat() * basic->GetNumberOfFats() - 1;
	int sec_fat = basic->GetSectorsPerFat();
	DiskImageTrack *track = NULL;
	if (sec_sta >= 0 && sec_fat > 0) {
		if (basic->GetFatSideNumber() >= 0) sec_pos += basic->GetFatSideNumber() * basic->GetSectorsPerTrackOnBasic();
		track = basic->GetManagedTrack(sec_pos, &side_num, &sector_num);
	}
	if (track) {
// GetManagedTrack内で反転する
//		side_num = basic->GetReversedSideNumber(side_num);
		track_num = track->GetTrackNumber();
	} else {
		track_num = -1;
	}
}

/// "FAT"などのタイトル名（ダイアログ用）
wxString DiskBasicType::GetTitleForFat() const
{
	return wxT("FAT");
}

/// 管理エリアのトラック番号からグループ番号を計算
/// @return グループ番号
wxUint32 DiskBasicType::CalcManagedStartGroup()
{
	int trk = basic->GetManagedTrackNumber();
	int sid = basic->GetFatSideNumber();
	if (sid < 0) sid = 0;
	int sides = basic->GetSidesPerDiskOnBasic();
	int secs_per_grp = basic->GetSectorsPerGroup();
	int secs_per_trk = basic->GetSectorsPerTrackOnBasic();
	managed_start_group = (trk * sides + sid) * secs_per_trk / secs_per_grp;
	return managed_start_group;
}

//

/// ルートディレクトリのセクタリストを計算
/// @param [in] start_sector  ディレクトリ開始セクタ番号
/// @param [in] end_sector    ディレクトリ終了セクタ番号
/// @param [out] group_items  セクタリスト
/// @return true
bool DiskBasicType::CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items)
{
	group_items.Empty();
	size_t dir_size = 0;
	int sector_base = basic->GetSectorNumberBase();
	for(int sec_pos = start_sector - sector_base; sec_pos <= end_sector - sector_base; sec_pos++) {
		int trk_num = 0;
		int sid_num = 0;
		int sec_num = 1;
		int div_num = 0;
		int div_nums = 1;
		DiskImageSector *sector = basic->GetManagedSector(sec_pos, &trk_num, &sid_num, &sec_num, &div_num, &div_nums);
		if (!sector) continue;
		group_items.Add(sec_pos, 0, trk_num, sid_num, sec_num, sec_num, div_num, div_nums);
		dir_size += (sector->GetSectorSize() / div_nums);
	}
	group_items.SetSize(dir_size);

	return true;
}

/// ルートディレクトリのチェック
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in]     is_formatting フォーマット中か
/// @return <0.0 エラー 1.0:正常
double DiskBasicType::CheckRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, bool is_formatting)
{
	// フォーマット中はチェックしない
	if (is_formatting) return 1.0;

	double valid_ratio = -1.0;
	if (CalcGroupsOnRootDirectory(start_sector, end_sector, group_items)) {
		valid_ratio = CheckDirectory(true, group_items);
	}
	return valid_ratio;
}

/// ルートディレクトリをアサイン
/// @param [in]     start_sector 開始セクタ番号
/// @param [in]     end_sector   終了セクタ番号
/// @param [out]    group_items  セクタリスト
/// @param [in,out] dir_item     ルートディレクトリアイテム
/// @return true / false
bool DiskBasicType::AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	CalcGroupsOnRootDirectory(start_sector, end_sector, group_items);

	return AssignDirectory(true, group_items, dir_item);
}

/// ディレクトリのチェック
/// @param [in]     is_root     ルートか
/// @param [in]    group_items  セクタリスト
/// @return <0.0 エラーあり 1.0:正常 使用しているディレクトリアイテムの有効度
double DiskBasicType::CheckDirectory(bool is_root, const DiskBasicGroups &group_items)
{
	bool valid = true;
	bool last = false;
	int  n_used_items = 0;
	double n_normals = 0.0;

	int index_number = 0;
	int pos = 0;
	int size_remain = (int)group_items.GetSize();
	int finish = 0;
	wxUint32 prev_grp_num = (wxUint32)-1;
	DiskBasicDirItem *nitem = dir->NewItem(NULL, 0, NULL);
	for(size_t idx = 0; idx < group_items.Count() && finish >= -1; idx++) {
		const DiskBasicGroupItem *gitem = group_items.ItemPtr(idx);
		wxUint32 grp_num = gitem->group;
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		int div_num = gitem->div_num;	// 分割番号
		int div_nums = gitem->div_nums;	// 分割数
		DiskImageTrack *track = basic->GetTrack(trk_num, sid_num);
		if (!track) {
			valid = false;
			break;
		}
		const DiskBasicGroupItem *next_gitem = idx + 1 < group_items.Count() ? group_items.ItemPtr(idx + 1) : NULL;

		for(int sec_num = gitem->sector_start; sec_num <= gitem->sector_end && finish >= -1; sec_num++) {
			DiskImageSector *sector = track->GetSector(sec_num);
//			nitem->SetSector(sector);
			if (!sector) {
				valid = false;
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				valid = false;
				break;
			}

			int size = sector->GetSectorSize() / div_nums;

			SectorParam next_sec(trk_num, sid_num
				, sec_num < gitem->sector_end ? sec_num + 1 : (next_gitem ? next_gitem->sector_start : -1)
				, -1);

			// オフセットを足す
			buffer += (size * div_num);
			buffer += pos;

			if (grp_num != prev_grp_num) {	
				// グループ番号が変わるときにスキップする位置
				buffer += basic->GetDirStartPosOnGroup();
				pos    += basic->GetDirStartPosOnGroup();
				size_remain -= basic->GetDirStartPosOnGroup();
				prev_grp_num = grp_num;
			}

			if (idx == 0 && sec_num == gitem->sector_start) {
				// ディレクトリエリア先頭をスキップする位置
				if (is_root) {
					buffer += basic->GetDirStartPosOnRoot();
					pos    += basic->GetDirStartPosOnRoot();
					size_remain -= basic->GetDirStartPosOnRoot();
				} else {
					buffer += basic->GetDirStartPos();
					pos    += basic->GetDirStartPos();
					size_remain -= basic->GetDirStartPos();
				}
			}

			// ディレクトリエリア各セクタの先頭をスキップする位置
			buffer += basic->GetDirStartPosOnSector();
			pos    += basic->GetDirStartPosOnSector();
			size_remain -= basic->GetDirStartPosOnSector();

			// ディレクトリのチェック
			while(valid && !last && pos < size) {
				finish = FinishAssigningDirectory(pos, size, size_remain);
				if (finish < 0) {
					// 終了 ポジションは次グループの先頭に
					size_remain = size - pos;
					pos = size;
					break;
				}
				nitem->SetDataPtr(index_number, gitem, sector, pos, buffer, &next_sec);
				valid = nitem->Check(last);
				if (valid) {
					if (nitem->CheckUsed(false)) {
						n_normals += nitem->NormalCodesInFileName();
						n_used_items++;
					}
				}
				pos    += nitem->GetDataSize();
				buffer += nitem->GetDataSize();
				size_remain -= nitem->GetDataSize();
				index_number++;
			}

			pos -= size;
			pos = AdjustPositionAssigningDirectory(pos);
		}
	}

	double valid_ratio = 0.0;
	if (!valid) {
		valid_ratio = -1.0;
	} else if (n_used_items > 0) {
		valid_ratio = n_normals / (double)n_used_items;
	}

	delete nitem;

	return valid_ratio;
}

/// ディレクトリが空か
/// @param [in]     is_root     ルートか
/// @param [in]    group_items  セクタリスト
/// @return true ファイルなし / false 空ではない or エラーあり
bool DiskBasicType::IsEmptyDirectory(bool is_root, const DiskBasicGroups &group_items)
{
	bool valid = true;
	bool last = false;

	int index_number = 0;
	int pos = 0;
	int size_remain = (int)group_items.GetSize();
	int finish = 0;
	wxUint32 prev_grp_num = (wxUint32)-1;
	DiskBasicDirItem *nitem = dir->NewItem(NULL, 0, NULL);
	for(size_t idx = 0; idx < group_items.Count() && finish >= -1; idx++) {
		const DiskBasicGroupItem *gitem = group_items.ItemPtr(idx);
		wxUint32 grp_num = gitem->group;
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		int div_num = gitem->div_num;	// 分割番号
		int div_nums = gitem->div_nums;	// 分割数
		DiskImageTrack *track = basic->GetTrack(trk_num, sid_num);
		if (!track) {
			valid = false;
			break;
		}
		const DiskBasicGroupItem *next_gitem = idx + 1 < group_items.Count() ? group_items.ItemPtr(idx + 1) : NULL;

		for(int sec_num = gitem->sector_start; sec_num <= gitem->sector_end && valid && !last && finish >= -1; sec_num++) {
			DiskImageSector *sector = track->GetSector(sec_num);
//			nitem->SetSector(sector);
			if (!sector) {
				valid = false;
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				valid = false;
				break;
			}

			int size = sector->GetSectorSize() / div_nums;

			SectorParam next_sec(trk_num, sid_num
				, sec_num < gitem->sector_end ? sec_num + 1 : (next_gitem ? next_gitem->sector_start : -1)
				, -1);

			// オフセットを足す
			buffer += (size * div_num);
			buffer += pos;

			if (grp_num != prev_grp_num) {	
				// グループ番号が変わるときにスキップする位置
				buffer += basic->GetDirStartPosOnGroup();
				pos    += basic->GetDirStartPosOnGroup();
				size_remain -= basic->GetDirStartPosOnGroup();
				prev_grp_num = grp_num;
			}

			if (idx == 0 && sec_num == gitem->sector_start) {
				// ディレクトリエリア先頭をスキップする位置
				if (is_root) {
					buffer += basic->GetDirStartPosOnRoot();
					pos    += basic->GetDirStartPosOnRoot();
					size_remain -= basic->GetDirStartPosOnRoot();
				} else {
					buffer += basic->GetDirStartPos();
					pos    += basic->GetDirStartPos();
					size_remain -= basic->GetDirStartPos();
				}
			}

			// ディレクトリエリア各セクタの先頭をスキップする位置
			buffer += basic->GetDirStartPosOnSector();
			pos    += basic->GetDirStartPosOnSector();
			size_remain -= basic->GetDirStartPosOnSector();

			// ディレクトリにファイルがないかのチェック
			while(valid && !last && pos < size) {
				finish = FinishAssigningDirectory(pos, size, size_remain);
				if (finish < 0) {
					// 終了 ポジションは次グループの先頭に
					size_remain = size - pos;
					pos = size;
					break;
				}
				nitem->SetDataPtr(index_number, gitem, sector, pos, buffer, &next_sec);
				if (nitem->IsNormalFile()) {
					valid = !nitem->CheckUsed(last);
				}
				pos    += nitem->GetDataSize();
				buffer += nitem->GetDataSize();
				size_remain -= nitem->GetDataSize();
				index_number++;
			}

			pos -= size;
			pos = AdjustPositionAssigningDirectory(pos);
		}
	}
	delete nitem;
	return valid;
}

/// ディレクトリをアサイン
/// @param [in]     is_root      ルートか
/// @param [in]     group_items  セクタリスト
/// @param [in,out] dir_item     ディレクトリアイテム
/// @return true
bool DiskBasicType::AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item)
{
	int index_number = 0;
	int pos = 0;
	bool unuse = false;
	int size_remain = (int)group_items.GetSize();
	int finish = 0;
	wxUint32 prev_grp_num = (wxUint32)-1;
	for(size_t idx = 0; idx < group_items.Count() && finish >= -1; idx++) {
		const DiskBasicGroupItem *gitem = group_items.ItemPtr(idx);
		wxUint32 grp_num = gitem->group;
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		int div_num = gitem->div_num;	// 分割番号
		int div_nums = gitem->div_nums;	// 分割数
		DiskImageTrack *track = basic->GetTrack(trk_num, sid_num);
		if (!track) {
			continue;
		}
		const DiskBasicGroupItem *next_gitem = idx + 1 < group_items.Count() ? group_items.ItemPtr(idx + 1) : NULL;

		for(int sec_num = gitem->sector_start; sec_num <= gitem->sector_end && finish >= -1; sec_num++) {
			DiskImageSector *sector = track->GetSector(sec_num);
			if (!sector) continue;

			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) continue;

			int size = sector->GetSectorSize() / div_nums;

			SectorParam next_sec(trk_num, sid_num
				, sec_num < gitem->sector_end ? sec_num + 1 : (next_gitem ? next_gitem->sector_start : -1)
				, -1);

			// オフセットを足す
			buffer += (size * div_num);
			buffer += pos;

			if (grp_num != prev_grp_num) {	
				// グループ番号が変わるときにスキップする位置
				buffer += basic->GetDirStartPosOnGroup();
				pos    += basic->GetDirStartPosOnGroup();
				size_remain -= basic->GetDirStartPosOnGroup();
				prev_grp_num = grp_num;
			}

			if (idx == 0 && sec_num == gitem->sector_start) {
				// ディレクトリエリア先頭をスキップする位置
				if (is_root) {
					buffer += basic->GetDirStartPosOnRoot();
					pos    += basic->GetDirStartPosOnRoot();
					size_remain -= basic->GetDirStartPosOnRoot();
				} else {
					buffer += basic->GetDirStartPos();
					pos    += basic->GetDirStartPos();
					size_remain -= basic->GetDirStartPos();
				}
			}

			// ディレクトリエリア各セクタの先頭をスキップする位置
			buffer += basic->GetDirStartPosOnSector();
			pos    += basic->GetDirStartPosOnSector();
			size_remain -= basic->GetDirStartPosOnSector();

			while(pos < size) {
				finish = FinishAssigningDirectory(pos, size, size_remain);
				if (finish < 0) {
					// 終了 ポジションは次グループの先頭に
					size_remain -= (size - pos);
					pos = size;
					break;
				}
//				DiskBasicDirItem *nitem = dir->AssignItem(index_number, track->GetTrackNumber(), track->GetSideNumber(), sector, pos, buffer, unuse);
				DiskBasicDirItem *nitem = dir->NewItem(index_number, gitem, sector, pos, buffer, &next_sec, unuse);
				// サイズに達したら以降のエントリは未使用とする
				if (finish > 0) {
					nitem->Used(false);
				}
				// 親ディレクトリを設定
				nitem->SetParent(dir_item);
				// 子ディレクトリに追加
				dir_item->AddChild(nitem);

				pos    += nitem->GetDataSize();
				buffer += nitem->GetDataSize();
				size_remain -= nitem->GetDataSize();
				index_number++;
			}

			pos -= size;
			pos = AdjustPositionAssigningDirectory(pos);
		}
	}

	return true;
}

/// セクタをディレクトリとして初期化
/// @param [in]     group_items 確保したセクタリスト
/// @param [in,out] file_size   サイズ ディレクトリを拡張した時は既存サイズに加算
/// @param [in,out] size_remain 残りサイズ
/// @param [in,out] errinfo     エラー情報
/// @return 0:正常 <0:エラー 
int DiskBasicType::InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &size_remain, DiskBasicError &errinfo)
{
	int rc = 0;

	DiskBasicDirItem *newitem = basic->CreateDirItem(NULL, 0, NULL);
	int dir_size = (int)newitem->GetDataSize();	// 1アイテムのサイズ
	int index_number = 0;
	int pos = 0;
	int finish = 0;
	wxUint32 prev_grp_num = (wxUint32)-1;
	for(size_t idx = 0; idx < group_items.Count() && finish >= -1 && rc >= 0; idx++) {
		const DiskBasicGroupItem *gitem = &group_items.Item(idx);
		wxUint32 grp_num = gitem->group;
		int trk_num = gitem->track;
		int sid_num = gitem->side;
		int div_num = gitem->div_num;	// 分割番号
		int div_nums = gitem->div_nums;	// 分割数
		DiskImageTrack *track = basic->GetTrack(trk_num, sid_num);
		if (!track) {
			// トラックがない！
			errinfo.SetError(DiskBasicError::ERRV_NO_TRACK, grp_num, trk_num, sid_num);
			rc = -2;
			break;
		}

		const DiskBasicGroupItem *next_gitem = idx + 1 < group_items.Count() ? group_items.ItemPtr(idx + 1) : NULL;

		for(int sec_num = gitem->sector_start; sec_num <= gitem->sector_end && finish >= -1 && rc >= 0; sec_num++) {
			DiskImageSector *sector = basic->GetSector(trk_num, sid_num, sec_num);
			if (!sector) {
				// セクタがない！
				errinfo.SetError(DiskBasicError::ERRV_NO_SECTOR, grp_num, trk_num, sid_num, sec_num);
				rc = -2;
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				// セクタがない！
				errinfo.SetError(DiskBasicError::ERRV_NO_SECTOR, grp_num, trk_num, sid_num, sec_num);
				rc = -2;
				break;
			}

			int size = sector->GetSectorSize() / div_nums;

			SectorParam next_sec(trk_num, sid_num
				, sec_num < gitem->sector_end ? sec_num + 1 : (next_gitem ? next_gitem->sector_start : -1)
				, -1);

			// オフセットを足す
			buffer += (size * div_num);
			buffer += pos;

			if (grp_num != prev_grp_num) {	
				// グループ番号が変わるときにスキップする位置
				buffer += basic->GetDirStartPosOnGroup();
				pos    += basic->GetDirStartPosOnGroup();
				size_remain -= basic->GetDirStartPosOnGroup();
				prev_grp_num = grp_num;
			}

			if (idx == 0 && sec_num == gitem->sector_start) {
				// ディレクトリエリア先頭をスキップする位置
//				if (is_root) {
//					buffer += basic->GetDirStartPosOnRoot();
//					pos    += basic->GetDirStartPosOnRoot();
//					size_remain -= basic->GetDirStartPosOnRoot();
//				} else {
					buffer += basic->GetDirStartPos();
					pos    += basic->GetDirStartPos();
					size_remain -= basic->GetDirStartPos();
//				}
			}

			// ディレクトリエリア各セクタの先頭をスキップする位置
			buffer += basic->GetDirStartPosOnSector();
			pos    += basic->GetDirStartPosOnSector();
			size_remain -= basic->GetDirStartPosOnSector();

			while(pos < size) {
				finish = FinishAssigningDirectory(pos, size, size_remain);
				if (finish < 0) {
					// 終了 ポジションは次グループの先頭に
					file_size   += (size - pos);
					size_remain -= (size - pos);
					pos = size;
					break;
				}
				// ディスク内に書き込む
				newitem->SetDataPtr(index_number, gitem, sector, pos, buffer, &next_sec);
				// 初期値を入れる
				newitem->InitialData();

				pos    += dir_size;
				buffer += dir_size;
				file_size  += dir_size;
				size_remain -= dir_size;
				index_number++;
			}

			pos -= size;
			pos = AdjustPositionAssigningDirectory(pos);
		}
	}

	delete newitem;

	return rc;
}

/// ルートディレクトリの開始位置を得る
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetStartNumOnRootDirectory(int &track_num, int &side_num, int &sector_num)
{
	DiskImageTrack *track = basic->GetManagedTrack(basic->GetDirStartSector() - basic->GetSectorNumberBase(), &side_num, &sector_num);
	if (track) {
		track_num = track->GetTrackNumber();
	}
}

/// ルートディレクトリの終了位置を得る
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetEndNumOnRootDirectory(int &track_num, int &side_num, int &sector_num)
{
	DiskImageTrack *track = basic->GetManagedTrack(basic->GetDirEndSector() - basic->GetSectorNumberBase(), &side_num, &sector_num);
	if (track) {
		track_num = track->GetTrackNumber();
	}
}

//

/// 使用可能なディスクサイズを得る
/// @param [out] disk_size  ディスクサイズ
/// @param [out] group_size グループ数
void DiskBasicType::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = 0;
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		if (gnum != basic->GetGroupSystemCode()) group_size++;
	}
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
void DiskBasicType::CalcDiskFreeSize(bool wrote)
{
	fat_availability.Empty();

	// 使用済みかチェック
	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		int fsize = 0;
		int grps = 0;
		wxUint32 gnum = GetGroupNumber(pos);
		int fsts = FAT_AVAIL_USED;
		if (gnum == basic->GetGroupUnusedCode()) {
			fsize = (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			grps = 1;
			fsts = FAT_AVAIL_FREE;
		} else if (gnum == basic->GetGroupSystemCode()) {
			fsts = FAT_AVAIL_SYSTEM;
		} else if (gnum >= basic->GetGroupFinalCode()) {
			fsts = FAT_AVAIL_USED_LAST;
		}
		fat_availability.Add(fsts, fsize, grps);
	}

//	free_disk_size = (int)fat_availability.GetFreeSize();
//	free_groups = (int)fat_availability.GetFreeGroups();
}

/// 残りディスクサイズをクリア
void DiskBasicType::ClearDiskFreeSize()
{
//	free_disk_size = -1;
//	free_groups = -1;
	fat_availability.EmptyInit();
}

/// 残りディスクサイズを得る(CalcDiskFreeSize()で計算した結果)
void DiskBasicType::GetFreeDiskSize(int &disk_size, int &group_size) const
{
	disk_size = fat_availability.GetFreeSize();
	group_size = fat_availability.GetFreeGroups();
}

/// 残りディスクサイズを得る(CalcDiskFreeSize()で計算した結果)
int DiskBasicType::GetFreeDiskSize() const
{
	return fat_availability.GetFreeSize();
}

/// 残りグループ数を得る(CalcDiskFreeSize()で計算した結果)
int DiskBasicType::GetFreeGroupSize() const
{
	return fat_availability.GetFreeGroups();
}

/// FATの空き状況を配列で返す
/// @param [out] offset オフセット
/// @param [out] arr    空き状況を入れた配列
void DiskBasicType::GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const
{
	*offset = 0;
	*arr = &fat_availability;
}

//

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    確保するデータサイズ（バイト）
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicType::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
//	myLog.SetDebug("DiskBasicType::AllocateGroups {");

//	int file_size = data_size;
	int groups = 0; 

	// FAT
	int  rc = 0;
	bool first_group = (flags == ALLOCATE_GROUPS_NEW);
	int sizeremain = data_size;

	int bytes_per_group = basic->GetSectorsPerGroup() * basic->GetSectorSize();
	wxUint32 group_num = GetEmptyGroupNumber();
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

//		myLog.SetDebug("  group_num:0x%03x next:0x%03x", group_num, next_group_num);

		// 次の空きがない場合 or 残りサイズがこのグループで収まる場合
		if (next_group_num == INVALID_GROUP_NUMBER || sizeremain <= bytes_per_group) {
			// 最後のグループ番号
			next_group_num = CalcLastGroupNumber(next_group_num, sizeremain);
		}
		
		basic->GetNumsFromGroup(group_num, next_group_num, basic->GetSectorSize(), sizeremain, group_items);

		// グループ番号設定
		SetGroupNumber(group_num, next_group_num);

//		prev_gnum = gnum;
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
//	myLog.SetDebug("rc: %d }", rc);

	return rc;
}

/// データサイズ分のグループを確保する
/// @param [in]  item         ディレクトリアイテム
/// @param [in]  data_size    確保するデータサイズ（バイト）
/// @param [in]  flags        新規か追加か
/// @param [out] group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicType::AllocateGroups(DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	return AllocateUnitGroups(0, item, data_size, flags, group_items);
}

/// グループをつなげる
///
/// 指定したグループ番号からFATをたどってその最終グループにつなぐ
///
/// @param [in] group_num グループ番号
/// @param [in] append_group_num つなげるグループ番号
/// @return 0:正常 -1:エラー
int DiskBasicType::ChainGroups(wxUint32 group_num, wxUint32 append_group_num)
{
	int limit = basic->GetFatEndGroup() + 1;
	while(limit >= 0) {
		wxUint32 next_group_num = GetGroupNumber(group_num);
		if (next_group_num >= basic->GetGroupFinalCode()) {
			SetGroupNumber(group_num, append_group_num);
			break;
		}
		group_num = next_group_num;
		limit--;
	}
	return (limit >= 0 ? 0 : -1);
}

/// グループ番号から開始セクタ番号を得る
/// @param [in] group_num グループ番号
/// @return 開始セクタ番号
int DiskBasicType::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num * basic->GetSectorsPerGroup();
}

/// グループ番号から最終セクタ番号を得る
/// @param [in] group_num    グループ番号
/// @param [in] next_group   次のグループ番号
/// @param [in] sector_start 開始セクタ番号
/// @param [in] sector_size  セクタサイズ
/// @param [in] remain_size  残りデータサイズ
/// @return 最終セクタ番号
int DiskBasicType::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int sector_end = sector_start + basic->GetSectorsPerGroup() - 1;
	if (next_group >= basic->GetGroupFinalCode()) {
		// 最終グループの場合指定したセクタまで
		sector_end = sector_start + (next_group - basic->GetGroupFinalCode());
	}
	return sector_end;
}

/// データ領域の開始セクタを計算
int DiskBasicType::CalcDataStartSectorPos()
{
	return 0;
}

/// スキップするトラック番号
int DiskBasicType::CalcSkippedTrack()
{
	return 0x7fff;
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos    セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num    トラック番号
/// @param [out] side_num     サイド番号
/// @param [out] sector_num   セクタ番号
/// @param [out] div_num      分割番号
/// @param [out] div_nums     分割数
void DiskBasicType::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();

	if (selected_side >= 0) {
		// 1S
		track_num = sector_pos / sectors_per_track;
		side_num = selected_side;
	} else {
		// 2D, 2HD
		track_num = sector_pos / sectors_per_track / sides_per_disk;
		side_num = (sector_pos / sectors_per_track) % sides_per_disk;
	}
	sector_num = (sector_pos % sectors_per_track);

	if (numbering_sector == 1) {
		// トラックごとに連番の場合
		sector_num += (side_num * sectors_per_track);
	}

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBaseOnDisk();

	if (div_num)  *div_num = 0;
	if (div_nums) *div_nums = 1;
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num)
{
	int selected_side = basic->GetSelectedSide();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();

	if (selected_side >= 0) {
		// 1S
		track_num = sector_pos / sectors_per_track;
		sector_num = (sector_pos % sectors_per_track);
	} else {
		// 2D, 2HD
		track_num = sector_pos / (sectors_per_track * sides_per_disk);
		sector_num = (sector_pos % (sectors_per_track * sides_per_disk));
	}

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBaseOnDisk();
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
/// サイド番号はトラック番号に変換、トラック番号はサイド数の倍数となる
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] sector_num セクタ番号
void DiskBasicType::GetNumFromSectorPosT(int sector_pos, int &track_num, int &sector_num)
{
//	int selected_side = basic->GetSelectedSide();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
//	int sides_per_disk = basic->GetSidesPerDiskOnBasic();

	track_num = sector_pos / sectors_per_track;

	sector_num = (sector_pos % sectors_per_track);

	track_num += basic->GetTrackNumberBaseOnDisk();
	sector_num += basic->GetSectorNumberBaseOnDisk();
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicType::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	// サイド番号を逆転するか
	side_num = basic->GetReversedSideNumber(side_num);

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBaseOnDisk();

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * sectors_per_track + sector_num;
	} else {
		// 2D, 2HD
		sector_pos = track_num * sectors_per_track * sides_per_disk;
		sector_pos += (side_num % sides_per_disk) * sectors_per_track;
		if (numbering_sector == 1) {
			sector_pos += (sector_num % sectors_per_track);
		} else {
			sector_pos += sector_num;
		}
	}
	return sector_pos;
}

/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// サイド番号はセクタ番号の通し番号に変換
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] sector_num セクタ番号
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicType::GetSectorPosFromNumS(int track_num, int sector_num)
{
	int selected_side = basic->GetSelectedSide();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBaseOnDisk();

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * sectors_per_track + sector_num;
	} else {
		// 2D, 2HD
		sector_pos = track_num * sectors_per_track * sides_per_disk + sector_num;
	}
	return sector_pos;
}


/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// サイド番号はトラック番号に変換、トラック番号はサイド数の倍数となる
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] sector_num セクタ番号
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicType::GetSectorPosFromNumT(int track_num, int sector_num)
{
//	int selected_side = basic->GetSelectedSide();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sector_pos;

	track_num -= basic->GetTrackNumberBaseOnDisk();
	sector_num -= basic->GetSectorNumberBaseOnDisk();

	sector_pos = track_num * sectors_per_track + sector_num;

	return sector_pos;
}

//
// for directory
//

/// 指定したグループ番号からルートディレクトリかどうかを判定する
bool DiskBasicType::IsRootDirectory(wxUint32 group_num)
{
	return true;
}

/// 未使用のディレクトリアイテムを返す
/// @param [in,out] parent    ディレクトリ
/// @param [in,out] items     ディレクトリアイテム一覧
/// @param [in,out] pitem     ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out]    next_item 未使用アイテムの次位置にあるアイテム
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicType::GetEmptyDirectoryItem(DiskBasicDirItem *WXUNUSED(parent), DiskBasicDirItems *items, DiskBasicDirItem *WXUNUSED(pitem), DiskBasicDirItem **next_item)
{
	DiskBasicDirItem *match_item = NULL;
	if (items) {
		for(size_t i=0; i < items->Count(); i++) {
			DiskBasicDirItem *item = items->Item(i);
			if (!item->IsUsed()) {
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
	return match_item;
}

//
// for format
//

/// セクタデータを指定コードで埋める 全トラック＆セクタで呼ばれる
/// @param [in] track  トラック
/// @param [in] sector セクタ
void DiskBasicType::FillSector(DiskImageTrack *track, DiskImageSector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// @param [in] data ダイアログで入力したデータ（ラベル名など）
bool DiskBasicType::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	return true;
}

//
// for data access (read/verify)
//

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicType::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	return remain_size;
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
int DiskBasicType::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	int modified_size = sector_size;
	if (remain_size <= sector_size) {
		// ファイルの最終セクタ
		modified_size = CalcDataSizeOnLastSector(item, istream, ostream, sector_buffer, sector_size, remain_size);
	}
	if (modified_size < 0) {
		// セクタなし
		return -2;
	}

	if (modified_size > 0) {
		if (ostream) {
			// 書き出し
			temp.SetData(sector_buffer, modified_size, basic->IsDataInverted());
			ostream->Write(temp.GetData(), temp.GetSize());
		}
		if (istream) {
			// 読み込んで比較
			temp.SetSize(modified_size);
			istream->Read(temp.GetData(), temp.GetSize());
			temp.InvertData(basic->IsDataInverted());
	
			if (memcmp(temp.GetData(), sector_buffer, temp.GetSize()) != 0) {
				// データが異なる
				return -1;
			}
		}
	}
	return sector_size;
}

/// 内部ファイルをエクスポートする際に内容を変換
/// @param [in] item          ディレクトリアイテム
/// @param [in] istream       入力ストリーム
/// @param [out] ostream      出力先ストリーム（ファイル）
bool DiskBasicType::ConvertDataForLoad(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream)
{
	ostream.Write(istream);
	return true;
}

/// エクスポートしたファイルをベリファイする際に内容を変換
/// @param [in] item          ディレクトリアイテム
/// @param [in] istream       入力ストリーム
/// @param [out] ostream      出力先ストリーム（ファイル）
bool DiskBasicType::ConvertDataForVerify(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream)
{
	ostream.Write(istream);
	return true;
}

//
// for write
//

/// ファイルをセーブする前にデータを変換
/// @param [in] item          ディレクトリアイテム
/// @param [in] istream       入力ストリーム（ファイル）
/// @param [out] ostream      出力先ストリーム
bool DiskBasicType::ConvertDataForSave(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream)
{
	ostream.Write(istream);
	return true;
}

/// グループ確保時に最後のグループ番号を計算する
/// @param [in]     group_num	現在のグループ番号
/// @param [in,out] size_remain	残りのデータサイズ
/// @return 最後のグループ番号
wxUint32 DiskBasicType::CalcLastGroupNumber(wxUint32 group_num, int &size_remain)
{
	return group_num;
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
int DiskBasicType::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	bool need_eof_code = item->NeedCheckEofCode();

	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (need_eof_code) {
			// 最終は終端コード
			if (remain > 1) istream.Read((void *)buffer, remain - 1);
			if (remain > 0) buffer[remain - 1]=item->GetEofCode();
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

//
// for delete
//

/// 指定したグループ番号のFAT領域を削除する
/// @param [in] group_items グループリスト
void DiskBasicType::DeleteGroups(const DiskBasicGroups &group_items)
{
	for(size_t gidx=0; gidx<group_items.Count(); gidx++) {
		// FATエントリを削除
		DeleteGroupNumber(group_items.Item(gidx).group);
	}
}

/// 指定したグループ番号のFAT領域を削除する
/// @param [in] group_num グループ番号
void DiskBasicType::DeleteGroupNumber(wxUint32 group_num)
{
	// FATに未使用コードを設定
	SetGroupNumber(group_num, basic->GetGroupUnusedCode());
}

//
// property
//
