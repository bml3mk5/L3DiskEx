/// @file diskparam.cpp
///
/// @brief ディスクパラメータ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskparam.h"
#include <wx/xml/xml.h>
#include <wx/translation.h>
#include "logging.h"
#include "utils.h"


/// (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
const int gSectorSizes[5] = { 128,256,512,1024,0 };

DiskTemplates gDiskTemplates;

//
//
//
TrackParam::TrackParam()
{
	track_num = -1;
	side_num = -1;
	sector_num = -1;
	sectors_per_track = -1;
	sector_size = 128;
	memset(id, 0, sizeof(id));
}
/// 特殊なトラックやセクタを登録
/// @param [in] n_track_num         トラック番号(-1:ALL)
/// @param [in] n_side_num          サイド番号(-1:ALL)
/// @param [in] n_sector_num        セクタ番号(特殊セクタ指定時のみ)
/// @param [in] n_sectors_per_track セクタ数(特殊トラック指定時のみ)
/// @param [in] n_sector_size       セクタサイズ
TrackParam::TrackParam(int n_track_num, int n_side_num, int n_sector_num, int n_sectors_per_track, int n_sector_size)
{
	track_num = n_track_num;
	side_num = n_side_num;
	sector_num = n_sector_num;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	memset(id, 0, sizeof(id));
}
bool TrackParam::operator==(const TrackParam &dst) const
{
	return (track_num == dst.track_num
		 && side_num == dst.side_num
		 && sector_num == dst.sector_num
		 && (sectors_per_track < 0 || dst.sectors_per_track < 0 || sectors_per_track == dst.sectors_per_track)
		 && sector_size == dst.sector_size
	);
}
/// 指定したトラック、サイドが単密度か
/// @param [in] n_track_num トラック
/// @param [in] n_side_num  サイド
bool TrackParam::Match(int n_track_num, int n_side_num) const
{
	return (track_num < 0
		|| (track_num == n_track_num
		&& (side_num < 0 || side_num == n_side_num)));
}
/// IDをセット
void TrackParam::SetID(int idx, wxUint8 val)
{
	if (idx >= 0 && idx < 4) id[idx] = val;
}
/// IDを返す
wxUint8 TrackParam::GetID(int idx) const
{
	if (idx >= 0 && idx < 4) return id[idx];
	else return 0;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(TrackParams);

//
//
//
DiskParticular::DiskParticular()
	: TrackParam()
{
}
/// 特殊なトラックやセクタを登録
/// @param [in] n_track_num         トラック番号(-1:ALL)
/// @param [in] n_side_num          サイド番号(-1:ALL)
/// @param [in] n_sector_num        セクタ番号(特殊セクタ指定時のみ)
/// @param [in] n_sectors_per_track セクタ数(特殊トラック指定時のみ)
/// @param [in] n_sector_size       セクタサイズ
DiskParticular::DiskParticular(int n_track_num, int n_side_num, int n_sector_num, int n_sectors_per_track, int n_sector_size)
	: TrackParam(n_track_num, n_side_num, n_sector_num, n_sectors_per_track, n_sector_size)
{
}

/// 除外リストに追加
void DiskParticular::AddExclude(const TrackParam &n_param)
{
	excludes.Add(n_param);
}

/// 除外リストに含まれるか
bool DiskParticular::FindExclude(int n_track_num, int n_side_num) const
{
	TrackParam *match = NULL;
	for(size_t i = 0; i < excludes.Count(); i++) {
		TrackParam *item = &excludes.Item(i);
		if (n_track_num != item->GetTrackNumber()) continue;
		if (n_side_num != item->GetSideNumber()) continue;

		match = item;
		break;
	}
	return (match != NULL);
}

/// 同じトラックやサイドのものをまとめる
/// @param [in] tracks      トラック数
/// @param [in] sides       サイド数
/// @param [in] both_sides  両面タイプか
/// @param [in,out] arr     リスト
void DiskParticular::UniqueTracks(int tracks, int sides, bool both_sides, DiskParticulars &arr)
{
	size_t count = arr.Count();
	if (count <= 1) return;

	DiskParticulars newarr;
	DiskParticular *prev_sd;

	prev_sd = &arr.Item(0);
	// 同じトラック番号で全サイドが同じパラメータ(単密度)であればまとめる
	int side_count = 1;
	bool all_sides = true;
	for(size_t idx = 1; idx <= count; idx++) {
		DiskParticular *sd = idx < count ? &arr.Item(idx) : NULL;
		if (sd == NULL || prev_sd->GetTrackNumber() != sd->GetTrackNumber()) {
			if (side_count >= sides || both_sides) {
				newarr.Add(DiskParticular(prev_sd->GetTrackNumber(), -1, -1, prev_sd->GetSectorsPerTrack(), prev_sd->GetSectorSize()));
			} else {
				newarr.Add(DiskParticular(prev_sd->GetTrackNumber(), prev_sd->GetSideNumber(), -1, prev_sd->GetSectorsPerTrack(), prev_sd->GetSectorSize()));
				all_sides = false;
			}
			side_count = 0;
		}
		side_count++;
		prev_sd = sd;
	}
	arr = newarr;

	count = arr.Count();
	newarr.Empty();
	// 全トラックが同じパラメータ(単密度)であればまとめる
	if (all_sides && (int)count >= tracks) {
		DiskParticular *sd = &arr.Item(0);
		newarr.Add(DiskParticular(-1, -1, -1, sd->GetSectorsPerTrack(), sd->GetSectorSize()));
		arr = newarr;
	}
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskParticulars);
/// 全ての値が一致するか
bool operator==(const DiskParticulars &src, const DiskParticulars &dst)
{
	bool match = true;
	if (src.Count() != dst.Count()) {
		match = false;
	} else {
		for(size_t di=0; di<dst.Count(); di++) {
			bool sm = false;
			for(size_t si=0; si<src.Count(); si++) {
				const DiskParticular *ss = &src.Item(si);
				const DiskParticular *ds = &dst.Item(di);
				if (*ss == *ds) {
					sm = true;
					break;
				}
			}
			if (!sm) {
				match = false;
				break;
			}
		}

	}
	return match;
}

//
//
//
DiskParamName::DiskParamName()
{
	flags = 0;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskParamNames);

//
//
//
DiskParam::DiskParam()
{
	this->ClearDiskParam();
}
DiskParam::DiskParam(const DiskParam &src)
{
	this->SetDiskParam(src);
}
DiskParam &DiskParam::operator=(const DiskParam &src)
{
	this->SetDiskParam(src);
	return *this;
}
void DiskParam::SetDiskParam(const wxString &n_type_name
	, const DiskParamNames &n_basic_types
	, bool n_reversible
	, int n_sides_per_disk
	, int n_tracks_per_side
	, int n_sectors_per_track
	, int n_sector_size
	, int n_numbering_sector
	, int n_disk_density
	, int n_interleave
	, const DiskParticulars &n_singles
	, const DiskParticulars &n_ptracks
	, const DiskParticulars &n_psectors
	, const wxString &n_density_name
	, const wxString &n_desc
) {
	disk_type_name = n_type_name;
	basic_types = n_basic_types;
	reversible = n_reversible;
	sides_per_disk = n_sides_per_disk;
	tracks_per_side = n_tracks_per_side;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	numbering_sector = n_numbering_sector;
	disk_density = n_disk_density;
	interleave = n_interleave;
//	if (density < 0 || 2 < density) density = 0;
	singles = n_singles;
	ptracks = n_ptracks;
	psectors = n_psectors;
	density_name = n_density_name;
	description = n_desc;
}
void DiskParam::SetDiskParam(int n_sides_per_disk
	, int n_tracks_per_side
	, int n_sectors_per_track
	, int n_sector_size
	, int n_disk_density
	, int n_interleave
	, const DiskParticulars &n_singles
) {
	sides_per_disk = n_sides_per_disk;
	tracks_per_side = n_tracks_per_side;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	disk_density = n_disk_density;
//	if (density < 0 || 2 < density) density = 0;
	interleave = n_interleave;
	singles = n_singles;
}
void DiskParam::SetDiskParam(const DiskParam &src)
{
	disk_type_name = src.disk_type_name;
	basic_types = src.basic_types;
	reversible = src.reversible;
	sides_per_disk = src.sides_per_disk;
	tracks_per_side = src.tracks_per_side;
	sectors_per_track = src.sectors_per_track;
	sector_size = src.sector_size;
	numbering_sector = src.numbering_sector;
	disk_density = src.disk_density;
	interleave = src.interleave;
	singles = src.singles;
	ptracks = src.ptracks;
	psectors = src.psectors;
	density_name = src.density_name;
	description = src.description;
}
void DiskParam::ClearDiskParam()
{
	disk_type_name.Empty();
	basic_types.Empty();
	reversible			 = false;
	sides_per_disk		 = 0;
	tracks_per_side		 = 0;
	sectors_per_track	 = 0;
	sector_size			 = 0;
	numbering_sector	 = 0;
	disk_density		 = 0;
	interleave			 = 1;
	singles.Empty();
	ptracks.Empty();
	psectors.Empty();
	density_name.Empty();
	description.Empty();
}
/// 指定したパラメータで一致するものがあるか
/// @return true:一致する
bool DiskParam::Match(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks)
{
	bool match = (sides_per_disk == n_sides_per_disk)
		&& (tracks_per_side == n_tracks_per_side)
		&& (sectors_per_track == n_sectors_per_track)
		&& (sector_size == n_sector_size)
		&& (interleave == n_interleave)
		&& (numbering_sector == n_numbering_sector)
		&& (singles == n_singles)
		&& (ptracks == n_ptracks);
	
	return match;
}
/// 指定したパラメータで一致するものがあるか
/// @return true:一致する
bool DiskParam::Match(const DiskParam &param)
{
	bool match = (disk_type_name == param.disk_type_name)
		&& (reversible == param.reversible)
		&& (sides_per_disk == param.sides_per_disk)
		&& (tracks_per_side == param.tracks_per_side)
		&& (sectors_per_track == param.sectors_per_track)
		&& (sector_size == param.sector_size)
		&& (interleave == param.interleave)
		&& (numbering_sector == param.numbering_sector)
		&& (singles == param.singles);

	return match;
}
/// 指定したパラメータで一致するものがあるか
/// @return true:一致する
bool DiskParam::MatchExceptName(const DiskParam &param)
{
	bool match = (reversible == param.reversible)
		&& (sides_per_disk == param.sides_per_disk)
		&& (tracks_per_side == param.tracks_per_side)
		&& (sectors_per_track == param.sectors_per_track)
		&& (sector_size == param.sector_size)
		&& (interleave == param.interleave)
		&& (numbering_sector == param.numbering_sector)
		&& (singles == param.singles)
		&& (ptracks == param.ptracks);

	return match;
}
/// 指定したパラメータに近い値で一致するものがあるか
/// @return true:一致する
bool DiskParam::MatchNear(int num, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, bool &last)
{
	bool match = false;
	switch(num) {
	case 0:
		// 特殊なトラックを除いて比較
		match = (sides_per_disk == n_sides_per_disk)
			&& (tracks_per_side == n_tracks_per_side)
			&& (sectors_per_track == n_sectors_per_track)
			&& (sector_size == n_sector_size)
			&& (interleave == n_interleave)
			&& (numbering_sector == n_numbering_sector)
			&& (singles == n_singles);
		break;
	case 1:
		// インターリーブを除いて比較
		// compare without interleave 
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (tracks_per_side == n_tracks_per_side) // トラック数は一致
			&& (sectors_per_track == n_sectors_per_track) // セクタ数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& (numbering_sector == n_numbering_sector)	// セクタ番号の付番方法は一致
			&& (singles == n_singles);	// 単密度のトラックは一致
		break;
	case 2:
		// インターリーブを入れて
		// トラック数が指定範囲内で比較
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (sectors_per_track == n_sectors_per_track) // セクタ数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& (numbering_sector == n_numbering_sector)	// セクタ番号の付番方法は一致
			&& (singles == n_singles)	// 単密度のトラックは一致
			&& (interleave == n_interleave) // インターリーブは一致
			&& ((n_tracks_per_side - 5) <= tracks_per_side && tracks_per_side <= n_tracks_per_side); // トラック数は-5 - 0の範囲
		break;
	case 3:
		// インターリーブを除いて、
		// トラック数が指定範囲内で比較
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (sectors_per_track == n_sectors_per_track) // セクタ数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& (numbering_sector == n_numbering_sector)	// セクタ番号の付番方法は一致
			&& (singles == n_singles)	// 単密度のトラックは一致
			&& ((n_tracks_per_side - 5) <= tracks_per_side && tracks_per_side <= n_tracks_per_side); // トラック数は-5 - 0の範囲
		break;
	case 4:
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (sectors_per_track == n_sectors_per_track) // セクタ数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& (numbering_sector == n_numbering_sector)	// セクタ番号の付番方法は一致
			&& ((n_tracks_per_side - 5) <= tracks_per_side && tracks_per_side <= n_tracks_per_side); // トラック数は-5 - 0の範囲
		break;
	case 5:
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& (numbering_sector == n_numbering_sector)	// セクタ番号の付番方法は一致
			&& ((n_tracks_per_side - 5) <= tracks_per_side && tracks_per_side <= n_tracks_per_side) // トラック数は-5 - 0の範囲
			&& (sectors_per_track <= n_sectors_per_track); // セクタ数は小さければよし
		break;
	default:
		last = true;
		break;
	}
	return match;
}
/// 指定したトラック、サイドが単密度か
/// @param [in]  track_num         トラック番号
/// @param [in]  side_num          サイド番号
/// @param [out] sectors_per_track セクタ数
/// @param [out] sector_size       セクタサイズ
/// @return true/false
bool DiskParam::FindSingleDensity(int track_num, int side_num, int *sectors_per_track, int *sector_size)  const
{
	bool match = false;
	for(size_t i=0; i<singles.Count(); i++) {
		const DiskParticular *sd = &singles.Item(i);
		if (sd->Match(track_num, side_num)) {
			match = true;
			if (sectors_per_track && sd->GetSectorsPerTrack() > 0) {
				*sectors_per_track = sd->GetSectorsPerTrack();
			}
			if (sector_size && sd->GetSectorSize() > 0) {
				*sector_size = sd->GetSectorSize();
			}
			break;
		}
	}
	return match;
}
/// 単密度を持っているか
/// @param [out] sectors_per_track セクタ数
/// @param [out] sector_size       セクタサイズ
/// @retval 0 なし
/// @retval 1 全トラック
/// @retval 2 トラック0,サイド0
/// @retval 3 トラック0,両面
int DiskParam::HasSingleDensity(int *sectors_per_track, int *sector_size) const
{
	int val = 0;
	DiskParticular *sd = NULL;
	for(size_t i=0; i<singles.Count(); i++) {
		if (singles[i].GetTrackNumber() < 0 && singles[i].GetSideNumber() < 0) {
			sd = &singles[i];
			val = 1;
			break;
		} else if (singles[i].GetTrackNumber() == 0 && singles[i].GetSideNumber() < 0) {
			sd = &singles[i];
			val = 3;
			break;
		} else if (singles[i].GetTrackNumber() == 0 && singles[i].GetSideNumber() == 0) {
			sd = &singles[i];
			val = 2;
			break;
		}
	}
	int max_tracks = GetTracksPerSide() * GetSidesPerDisk();
	if (max_tracks > 0 && max_tracks == (int)singles.Count()) {
		sd = &singles[0];
		val = 1;
	}
	if (val > 0) {
		if (sectors_per_track) {
			*sectors_per_track = sd->GetSectorsPerTrack();
			if (*sectors_per_track < 0) {
				*sectors_per_track = GetSectorsPerTrack();
			}
		}
		if (sector_size) *sector_size = sd->GetSectorSize();
	}
	return val;
}

/// ディスクサイズを計算する（ベタディスク用）
int DiskParam::CalcDiskSize() const
{
	int disk_size = 0;
	for(int trk = 0; trk < GetTracksPerSide(); trk++) {
		for(int sid = 0; sid < GetSidesPerDisk(); sid++) {
			int sec_nums = GetSectorsPerTrack();
			int sec_size = GetSectorSize();
			// 単密度か
			for(size_t ss = 0; ss < singles.Count(); ss++) {
				DiskParticular *sd = &singles.Item(ss);
				if (sd->Match(trk, sid)) {
					sec_nums = sd->GetSectorsPerTrack() > 0 ? sd->GetSectorsPerTrack() : sec_nums;
					sec_size = sd->GetSectorSize() > 0 ? sd->GetSectorSize() : sec_size;
					break;
				}
			}
			disk_size += (sec_nums * sec_size);
		}
	}
	return disk_size;
}

/// 特殊なトラックか
bool DiskParam::FindParticularTrack(int track_num, int side_num, int &sectors_per_track, int &sector_size) const
{
	const DiskParticular *match = NULL;
	for(size_t i=0; i<ptracks.Count(); i++) {
		const DiskParticular *pt = &ptracks.Item(i);
		if (pt->GetSectorsPerTrack() <= 0) continue;
		if (pt->GetSectorNumber() >= 0) continue;
		if (pt->GetTrackNumber() != track_num) continue;
		if (pt->GetSideNumber() >= 0 && pt->GetSideNumber() != side_num) continue;
		if (pt->FindExclude(track_num, side_num)) continue;

		match = pt;
		sectors_per_track = pt->GetSectorsPerTrack();
		if (pt->GetSectorSize() > 0) sector_size = pt->GetSectorSize();
		break;
	}
	return (match != NULL);
}

/// 特殊なセクタか
bool DiskParam::FindParticularSector(int track_num, int side_num, int sector_num, int &sector_size, const wxUint8 **sector_id) const
{
	const DiskParticular *match = NULL;
	for(size_t i=0; i<psectors.Count(); i++) {
		const DiskParticular *ps = &psectors.Item(i);
		if (ps->GetSectorSize() <= 0) continue;
		if (ps->GetSectorsPerTrack() >= 0) continue;
		if (ps->GetSectorNumber() >= 0 && ps->GetSectorNumber() != sector_num) continue;
		if (ps->GetSideNumber() >= 0 && ps->GetSideNumber() != side_num) continue;
		if (ps->GetTrackNumber() >= 0 && ps->GetTrackNumber() != track_num) continue;
		if (ps->FindExclude(track_num, side_num)) continue;

		match = ps;
		sector_size = ps->GetSectorSize();
		if (sector_id != NULL) *sector_id = ps->GetID();
		break;
	}
	return (match != NULL);
}

/// DISK BASICをさがす
const DiskParamName *DiskParam::FindBasicType(const wxString &type_name, int flags) const
{
	const DiskParamName *match = NULL;
	for(size_t i=0; i<basic_types.Count(); i++) {
		const DiskParamName *item = &basic_types.Item(i);
		if (item->GetName() == type_name && (flags < 0 || item->GetFlags() == flags)) {
			match = item;
			break;
		}
	}
	return match;
}

/// ディスクパラメータを文字列にフォーマットして返す
wxString DiskParam::GetDiskDescription() const
{
	wxString str;

	if (!density_name.IsEmpty()) {
		str += density_name;
		str += wxT("  ");
	}

	str += wxString::Format(_("%dtracks, %dsides, %dsectors, %dbytes/sector, Interleave:%d"), tracks_per_side, sides_per_disk, sectors_per_track, sector_size, interleave);

	if (!description.IsEmpty()) {
		str += wxT(" ");
		str += description;
	}
	return str;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskParams);

//
//
//
DiskTemplates::DiskTemplates()
{
}
/// XMLファイルから読み込み
bool DiskTemplates::Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("disk_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "DiskTypes") return false;

	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item) {
		if (item->GetName() == "DiskType") {
			DiskParam p;

			wxString type_name = item->GetAttribute("name");
			p.SetDiskTypeName(type_name);

			wxXmlNode *itemnode = item->GetChildren();
			wxString den_name, den_name_locale;
			wxString desc, desc_locale;
			while (itemnode) {
				wxString str = itemnode->GetNodeContent();
				if (itemnode->GetName() == "Reversible") {
					p.Reversible(Utils::ToBool(str));
				} else if (itemnode->GetName() == "SidesPerDisk") {
					p.SetSidesPerDisk(Utils::ToInt(str));
				} else if (itemnode->GetName() == "TracksPerSide") {
					p.SetTracksPerSide(Utils::ToInt(str));
				} else if (itemnode->GetName() == "SectorsPerTrack") {
					p.SetSectorsPerTrack(Utils::ToInt(str));
				} else if (itemnode->GetName() == "SectorSize") {
					p.SetSectorSize(Utils::ToInt(str));
				} else if (itemnode->GetName() == "NumberingSector") {
					str = str.Lower();
					if (str == wxT("track")) {
						p.SetNumberingSector(1);
					}
				} else if (itemnode->GetName() == "Density") {
					p.SetParamDensity(Utils::ToInt(str));
				} else if (itemnode->GetName() == "Interleave") {
					p.SetInterleave(Utils::ToInt(str));
				} else if (itemnode->GetName() == "DiskBasicTypes") {
					DiskParamNames basic_types;
					LoadDiskBasicTypes(itemnode, basic_types, errmsgs);
					p.SetBasicTypes(basic_types);
				} else if (itemnode->GetName() == "SingleDensity") {
					DiskParticular s;
					LoadSingleDensity(itemnode, s, errmsgs);
					p.AddSingleDensity(s);
				} else if (itemnode->GetName() == "ParticularTrack") {
					DiskParticular d;
					LoadParticularTrack(itemnode, d, errmsgs);
					p.AddParticularTrack(d);
				} else if (itemnode->GetName() == "ParticularSector") {
					DiskParticular d;
					LoadParticularSector(itemnode, d, errmsgs);
					p.AddParticularSector(d);
				} else if (itemnode->GetName() == "DensityName") {
					LoadDescription(itemnode, locale_name, den_name, den_name_locale);
				} else if (itemnode->GetName() == "Description") {
					LoadDescription(itemnode, locale_name, desc, desc_locale);
				}
				itemnode = itemnode->GetNext();
			}
			if (!den_name_locale.IsEmpty()) {
				den_name = den_name_locale;
			}
			p.SetDensityName(den_name);
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			p.SetDescription(desc);

			if (Find(type_name) == NULL) {
				params.Add(p);
			} else {
				errmsgs += wxT("\n");
				errmsgs += _("Duplicate type name in DiskType : ");
				errmsgs += type_name;
				return false;
			}
		}
		item = item->GetNext();
	}
	return true;
}

bool DiskTemplates::LoadDiskBasicTypes(const wxXmlNode *node, DiskParamNames &basic_types, wxString &errmsgs)
{
	wxXmlNode *citemnode = node->GetChildren();
	while(citemnode) {
		wxString str = citemnode->GetNodeContent();
		if (citemnode->GetName() == "Type") {
			DiskParamName p;
			str = str.Trim(false).Trim(true);
			if (!str.IsEmpty()) {
				p.SetName(str);
			}
			str = citemnode->GetAttribute("p");
			if (str.Lower() == "major") {
				p.SetFlags(1);
			}
			basic_types.Add(p);
		}
		citemnode = citemnode->GetNext();
	}
	return true;
}

bool DiskTemplates::LoadSingleDensity(const wxXmlNode *node, DiskParticular &s, wxString &errmsgs)
{
	wxString str;
	str = node->GetAttribute("track");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		s.SetTrackNumber(Utils::ToInt(str));
	}
	str = node->GetAttribute("side");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		s.SetSideNumber(Utils::ToInt(str));
	}
	str = node->GetAttribute("sectors");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		s.SetSectorsPerTrack(Utils::ToInt(str));
	}
	str = node->GetAttribute("size");
	if (!str.IsEmpty()) {
		s.SetSectorSize(Utils::ToInt(str));
	}
	return true;
}

bool DiskTemplates::LoadParticularTrack(const wxXmlNode *node, DiskParticular &d, wxString &errmsgs)
{
	wxString str;
	str = node->GetAttribute("track");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		d.SetTrackNumber(Utils::ToInt(str));
	}
	str = node->GetAttribute("side");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		d.SetSideNumber(Utils::ToInt(str));
	}
	str = node->GetAttribute("sectors");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		d.SetSectorsPerTrack(Utils::ToInt(str));
	}
	str = node->GetAttribute("size");
	if (!str.IsEmpty()) {
		d.SetSectorSize(Utils::ToInt(str));
	}
	wxXmlNode *cnode = node->GetChildren();
	while(cnode) {
		if (cnode->GetName() == "Exclude") {
			TrackParam e;
			str = cnode->GetAttribute("track");
			if (!str.IsEmpty()) {
				e.SetTrackNumber(Utils::ToInt(str));
			}
			str = cnode->GetAttribute("side");
			if (!str.IsEmpty()) {
				e.SetSideNumber(Utils::ToInt(str));
			}
			d.AddExclude(e);
		}
		cnode = cnode->GetNext();
	}
	return true;
}

bool DiskTemplates::LoadParticularSector(const wxXmlNode *node, DiskParticular &d, wxString &errmsgs)
{
	wxString str;
	str = node->GetAttribute("track");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		d.SetTrackNumber(Utils::ToInt(str));
	}
	str = node->GetAttribute("side");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		d.SetSideNumber(Utils::ToInt(str));
	}
	str = node->GetAttribute("sector");
	if (!str.IsEmpty() && str.Upper() != wxT("ALL")) {
		d.SetSectorNumber(Utils::ToInt(str));
	}
	str = node->GetAttribute("size");
	if (!str.IsEmpty()) {
		d.SetSectorSize(Utils::ToInt(str));
	}
	str = node->GetAttribute("id_r");
	if (!str.IsEmpty()) {
		d.SetID(2, (wxUint8)Utils::ToInt(str));
	}
	wxXmlNode *cnode = node->GetChildren();
	while(cnode) {
		if (cnode->GetName() == "Exclude") {
			TrackParam e;
			str = cnode->GetAttribute("track");
			if (!str.IsEmpty()) {
				e.SetTrackNumber(Utils::ToInt(str));
			}
			str = cnode->GetAttribute("side");
			if (!str.IsEmpty()) {
				e.SetSideNumber(Utils::ToInt(str));
			}
			d.AddExclude(e);
		}
		cnode = cnode->GetNext();
	}
	return true;
}

bool DiskTemplates::LoadDescription(const wxXmlNode *node, const wxString &locale_name, wxString &desc, wxString &desc_locale)
{
	if (node->HasAttribute("lang")) {
		wxString lang = node->GetAttribute("lang");
		if (locale_name.Find(lang) != wxNOT_FOUND) {
			desc_locale = node->GetNodeContent();
		}
	} else {
		desc = node->GetNodeContent();
	}
	return true;
}

/// タイプ名に一致するテンプレートの番号を返す
int DiskTemplates::IndexOf(const wxString &n_type_name) const
{
	int match = -1;
	for(size_t i=0; i<params.Count(); i++) {
		DiskParam *item = &params[i];
		if (n_type_name == item->GetDiskTypeName()) {
			match = (int)i;
			break;
		}
	}
	return match;
}

/// パラメータと一致するテンプレートを返す
const DiskParam *DiskTemplates::Find(const DiskParam &n_param) const
{
	DiskParam *match = NULL;
	for(size_t i=0; i<params.Count(); i++) {
		DiskParam *item = &params[i];
		if (item->MatchExceptName(n_param)) {
			match = item;
			break;
		}
	}
	return match;
}

/// タイプ名に一致するテンプレートを返す
const DiskParam *DiskTemplates::Find(const wxString &n_type_name) const
{
	DiskParam *match = NULL;
	for(size_t i=0; i<params.Count(); i++) {
		DiskParam *item = &params[i];
		if (n_type_name == item->GetDiskTypeName()) {
			match = item;
			break;
		}
	}
	return match;
}

/// パラメータに一致するテンプレートを返す
const DiskParam *DiskTemplates::FindStrict(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks) const
{
	DiskParam *match_item = NULL;
	bool m = false;
	for(size_t i=0; i<params.Count(); i++) {
		DiskParam *item = &params[i];
		m = item->Match(n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_interleave, n_numbering_sector, n_singles, n_ptracks);
		if (m) {
			match_item = item;
			break;
		}
	}
	return match_item;
}

/// パラメータに一致するあるいは近い物のテンプレートを返す
const DiskParam *DiskTemplates::Find(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks) const
{
	const DiskParam *match_item = FindStrict(n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_interleave, n_numbering_sector, n_singles, n_ptracks);
	if (!match_item) {
		bool last = false;
		bool m = false;
		for(int num = 0; !last && !m; num++) {
			// パラメータが一致しないときは、引数に近いパラメータ
			for(size_t i=0; i<params.Count(); i++) {
				DiskParam *item = &params[i];
				m = item->MatchNear(num, n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_interleave, n_numbering_sector, n_singles, last);
				if (last) {
					break;
				}
				if (m) {
					match_item = item;
					break;
				}
			}
		}
	}
	return match_item;
}
