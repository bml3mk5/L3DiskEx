/// @file diskparam.cpp
///
/// @brief ディスクパラメータ
///
#include "diskparam.h"
#include <wx/xml/xml.h>
#include "utils.h"

/// (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
const int gSectorSizes[5] = { 128,256,512,1024,0 };

DiskTypes gDiskTypes;

//
//
//
SingleDensity::SingleDensity(int n_track_num, int n_side_num, int n_sectors_per_track, int n_sector_size)
{
	track_num = n_track_num;
	side_num = n_side_num;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
}
bool SingleDensity::operator==(const SingleDensity &dst) const
{
	return (track_num == dst.track_num && side_num == dst.side_num);
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(SingleDensities);
/// 全ての値が一致するか
bool operator==(const SingleDensities &src, const SingleDensities &dst)
{
	bool match = true;
	if (src.Count() != dst.Count()) {
		match = false;
	} else {
		for(size_t di=0; di<dst.Count(); di++) {
			bool sm = false;
			for(size_t si=0; si<src.Count(); si++) {
				const SingleDensity *ss = &src.Item(si);
				const SingleDensity *ds = &dst.Item(di);
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
DiskParam::DiskParam()
{
	this->ClearDiskParam();
}
DiskParam::DiskParam(const DiskParam &src)
{
	this->SetDiskParam(src);
}
DiskParam::DiskParam(const wxString &n_type_name, wxUint32 n_disk_type, const wxArrayString &n_basic_types, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, int n_interleave, const SingleDensities &n_singles, const wxString &n_desc)
{
	this->SetDiskParam(n_type_name, n_disk_type, n_basic_types, n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_density, n_interleave, n_singles, n_desc);
}
DiskParam &DiskParam::operator=(const DiskParam &src)
{
	this->SetDiskParam(src);
	return *this;
}
void DiskParam::SetDiskParam(const wxString &n_type_name, wxUint32 n_disk_type, const wxArrayString &n_basic_types, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, int n_interleave, const SingleDensities &n_singles, const wxString &n_desc)
{
	disk_type_name = n_type_name;
	disk_type = n_disk_type;
	basic_types = n_basic_types;
	sides_per_disk = n_sides_per_disk;
	tracks_per_side = n_tracks_per_side;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	density = n_density;
	interleave = n_interleave;
	if (density < 0 || 2 < density) density = 0;
	singles = n_singles;
	description = n_desc;
}
#if 0
void DiskParam::SetDiskParam(const wxString &n_type_name, wxUint32 n_disk_type, const wxArrayString &n_basic_types, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, int n_interleave)
{
	disk_type_name = n_type_name;
	disk_type = n_disk_type;
	basic_types = n_basic_types;
	sides_per_disk = n_sides_per_disk;
	tracks_per_side = n_tracks_per_side;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	density = n_density;
	if (density < 0 || 2 < density) density = 0;
	interleave = n_interleave;
	singles.Empty();
}
#endif
void DiskParam::SetDiskParam(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, int n_interleave, const SingleDensities &n_singles)
{
	sides_per_disk = n_sides_per_disk;
	tracks_per_side = n_tracks_per_side;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	density = n_density;
	if (density < 0 || 2 < density) density = 0;
	interleave = n_interleave;
	singles = n_singles;
}
void DiskParam::SetDiskParam(const DiskParam &src)
{
	disk_type_name = src.disk_type_name;
	disk_type = src.disk_type;
	basic_types = src.basic_types;
	sides_per_disk = src.sides_per_disk;
	tracks_per_side = src.tracks_per_side;
	sectors_per_track = src.sectors_per_track;
	sector_size = src.sector_size;
	density = src.density;
	interleave = src.interleave;
	singles = src.singles;
	description = src.description;
}
void DiskParam::ClearDiskParam()
{
	disk_type_name.Empty();
	disk_type = 0;
	basic_types.Empty();
	sides_per_disk = 0;
	tracks_per_side = 0;
	sectors_per_track = 0;
	sector_size = 0;
	density = 0;
	interleave = 1;
	singles.Empty();
	description.Empty();
}
/// 指定したパラメータで一致するものがあるか
/// @return true:一致する
bool DiskParam::Match(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, const SingleDensities &n_singles)
{
	bool match = (sides_per_disk == n_sides_per_disk)
		&& (tracks_per_side == n_tracks_per_side)
		&& (sectors_per_track == n_sectors_per_track)
		&& (sector_size == n_sector_size)
		&& (interleave == n_interleave)
		&& (singles == n_singles);
	
	return match;
}
/// 指定したパラメータで一致するものがあるか
/// @return true:一致する
bool DiskParam::Match(const DiskParam &param)
{
	bool match = (disk_type_name == param.disk_type_name)
		&& (sides_per_disk == param.sides_per_disk)
		&& (tracks_per_side == param.tracks_per_side)
		&& (sectors_per_track == param.sectors_per_track)
		&& (sector_size == param.sector_size)
		&& (interleave == param.interleave)
		&& (singles == param.singles);

	return match;
}
/// 指定したパラメータに近い値で一致するものがあるか
/// @return true:一致する
bool DiskParam::MatchNear(int num, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, const SingleDensities &n_singles, bool &last)
{
	bool match = false;
	switch(num) {
	case 0:
		// インターリーブを除いて比較
		// compare without interleave 
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (tracks_per_side == n_tracks_per_side) // トラック数は一致
			&& (sectors_per_track == n_sectors_per_track) // セクタ数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& (singles == n_singles);	// 単密度のトラックは一致
		break;
	case 1:
		// インターリーブを入れて
		// トラック数が指定範囲内で比較
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (sectors_per_track == n_sectors_per_track) // セクタ数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& (singles == n_singles)	// 単密度のトラックは一致
			&& (interleave == n_interleave) // インターリーブは一致
			&& ((n_tracks_per_side - 5) <= tracks_per_side && tracks_per_side <= n_tracks_per_side); // トラック数は-5 - 0の範囲
		break;
	case 2:
		// インターリーブを除いて、
		// トラック数が指定範囲内で比較
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (sectors_per_track == n_sectors_per_track) // セクタ数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& (singles == n_singles)	// 単密度のトラックは一致
			&& ((n_tracks_per_side - 5) <= tracks_per_side && tracks_per_side <= n_tracks_per_side); // トラック数は-5 - 0の範囲
		break;
	case 3:
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (sectors_per_track == n_sectors_per_track) // セクタ数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
			&& ((n_tracks_per_side - 5) <= tracks_per_side && tracks_per_side <= n_tracks_per_side); // トラック数は-5 - 0の範囲
		break;
	case 4:
		match = (sides_per_disk == n_sides_per_disk) // サイド数は一致
			&& (sector_size == n_sector_size) // セクタサイズは一致
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
		if (singles[i].track_num < 0
		|| (singles[i].track_num == track_num && (singles[i].side_num < 0 || singles[i].side_num == side_num))) {
			match = true;
			if (sectors_per_track) {
				*sectors_per_track = singles[i].GetSectorsPerTrack();
				if (*sectors_per_track < 0) {
					*sectors_per_track = GetSectorsPerTrack();
				}
			}
			if (sector_size) *sector_size = singles[i].GetSectorSize();
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
int DiskParam::HasSingleDensity(int *sectors_per_track, int *sector_size) const
{
	int val = 0;
	SingleDensity *sd = NULL;
	for(size_t i=0; i<singles.Count(); i++) {
		if (singles[i].track_num < 0 && singles[i].side_num < 0) {
			sd = &singles[i];
			val = 1;
			break;
		} else if (singles[i].track_num == 0 && singles[i].side_num == 0) {
			sd = &singles[i];
			val = 2;
			break;
		}
	}
	int max_tracks = GetTracksPerSide() * GetSidesPerDisk();
	if (max_tracks == singles.Count()) {
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

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskParams);

//
//
//
DiskTypes::DiskTypes()
{
}
/// XMLファイルから読み込み
bool DiskTypes::Load(const wxString &data_path, const wxString &locale_name)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("disk_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "DiskTypes") return false;

#if 0
	wxXmlNode *prolog = doc.GetDocumentNode()->GetChildren();
	while (prolog) {
	    if (prolog->GetType() == wxXML_PI_NODE && prolog->GetName() == "DiskType") {
	        wxString pi = prolog->GetContent();
		}
	}
#endif

	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item) {
		if (item->GetName() == "DiskType") {
			wxString type_name = item->GetAttribute("name");
			wxXmlNode *itemnode = item->GetChildren();
			wxArrayString basic_types;
			long disk_type, sides_per_disk, track_per_side, sectors_per_track, sector_size, density, interleave, strk, ssid, sspt, ssiz;
			disk_type = sides_per_disk = track_per_side = sectors_per_track = sector_size = density = strk = ssid = sspt = ssiz = 0;
			interleave = 1;
			wxString type_str = item->GetAttribute("type");
			if (!type_str.IsEmpty()) {
				disk_type = L3DiskUtils::ToInt(type_str);
			}
			SingleDensities singles;
			wxString desc, desc_locale;
			wxString str;
			while (itemnode) {
				if (itemnode->GetName() == "SidesPerDisk") {
					str = itemnode->GetNodeContent();
					sides_per_disk = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "TracksPerSide") {
					str = itemnode->GetNodeContent();
					track_per_side = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "SectorsPerTrack") {
					str = itemnode->GetNodeContent();
					sectors_per_track = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "SectorSize") {
					str = itemnode->GetNodeContent();
					sector_size = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "Density") {
					str = itemnode->GetNodeContent();
					density = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "Interleave") {
					str = itemnode->GetNodeContent();
					interleave = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "DiskBasicTypes") {
					wxXmlNode *citemnode = itemnode->GetChildren();
					while(citemnode) {
						if (citemnode->GetName() == "Type") {
							str = citemnode->GetNodeContent();
							str = str.Trim(false).Trim(true);
							if (!str.IsEmpty()) {
								basic_types.Add(str);
							}
						}
						citemnode = citemnode->GetNext();
					}
				} else if (itemnode->GetName() == "SingleDensity") {
					str = itemnode->GetAttribute("track");
					if (str.IsEmpty() || str.Upper() == wxT("ALL")) {
						strk = -1;
					} else {
						strk = L3DiskUtils::ToInt(str);
					}
					str = itemnode->GetAttribute("side");
					if (str.IsEmpty()) {
						ssid = -1;
					} else {
						ssid = L3DiskUtils::ToInt(str);
					}
					str = itemnode->GetAttribute("sectors");
					if (str.IsEmpty()) {
						sspt = -1;
					} else {
						sspt = L3DiskUtils::ToInt(str);
					}
					str = itemnode->GetAttribute("size");
					if (str.IsEmpty()) {
						ssiz = 128;
					} else {
						ssiz = L3DiskUtils::ToInt(str);
					}
					SingleDensity s((int)strk, (int)ssid, (int)sspt, (int)ssiz);
					singles.Add(s);
				} else if (itemnode->GetName() == "Description") {
					if (itemnode->HasAttribute("lang")) {
						wxString lang = itemnode->GetAttribute("lang");
						if (locale_name.Find(lang) != wxNOT_FOUND) {
							desc_locale = itemnode->GetNodeContent();
						}
					} else {
						desc = itemnode->GetNodeContent();
					}
				}
				itemnode = itemnode->GetNext();
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			DiskParam p(type_name, (int)disk_type, basic_types
				, (int)sides_per_disk, (int)track_per_side, (int)sectors_per_track
				, (int)sector_size, (int)density, (int)interleave, singles, desc);
			types.Add(p);
		}
		item = item->GetNext();
	}
	return true;
}

/// タイプ名に一致するテンプレートの番号を返す
int DiskTypes::IndexOf(const wxString &n_type_name)
{
	int match = -1;
	for(size_t i=0; i<types.Count(); i++) {
		DiskParam *item = &types[i];
		if (n_type_name == item->GetDiskTypeName()) {
			match = (int)i;
			break;
		}
	}
	return match;
}

/// タイプ名に一致するテンプレートを返す
DiskParam *DiskTypes::Find(const wxString &n_type_name)
{
	DiskParam *match = NULL;
	for(size_t i=0; i<types.Count(); i++) {
		DiskParam *item = &types[i];
		if (n_type_name == item->GetDiskTypeName()) {
			match = item;
			break;
		}
	}
	return match;
}

/// パラメータに一致するあるいは近い物のテンプレートを返す
DiskParam *DiskTypes::Find(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, const SingleDensities &n_singles)
{
	DiskParam *match_item = NULL;
	bool m = false;
	for(size_t i=0; i<types.Count(); i++) {
		DiskParam *item = &types[i];
		m = item->Match(n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_interleave, n_singles);
		if (m) {
			match_item = item;
			break;
		}
	}
	if (!m) {
		bool last = false;
		for(int num = 0; !last && !m; num++) {
			// パラメータが一致しないときは、引数に近いパラメータ
			for(size_t i=0; i<types.Count(); i++) {
				DiskParam *item = &types[i];
				m = item->MatchNear(num, n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_interleave, n_singles, last);
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
