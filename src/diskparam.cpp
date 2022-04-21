/// @file diskparam.cpp
///
/// @brief ディスクパラメータ
///
#include "diskparam.h"
#include <wx/xml/xml.h>

/// (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
const int gSectorSizes[5] = { 128,256,512,1024,0 };

DiskTypes gDiskTypes;

//
//
//
SingleDensity::SingleDensity(int n_track_num, int n_side_num)
{
	track_num = n_track_num;
	side_num = n_side_num;
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
	bool match = false;
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
DiskParam::DiskParam(const wxString &n_type_name, wxUint32 n_disk_type, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, const SingleDensities &n_singles)
{
	this->SetDiskParam(n_type_name, n_disk_type, n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_density, n_singles);
}
void DiskParam::SetDiskParam(const wxString &n_type_name, wxUint32 n_disk_type, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, const SingleDensities &n_singles)
{
	disk_type_name = n_type_name;
	disk_type = n_disk_type;
	sides_per_disk = n_sides_per_disk;
	tracks_per_side = n_tracks_per_side;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	density = n_density;
	if (density < 0 || 2 < density) density = 0;
	singles = n_singles;
}
void DiskParam::SetDiskParam(const wxString &n_type_name, wxUint32 n_disk_type, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density)
{
	disk_type_name = n_type_name;
	disk_type = n_disk_type;
	sides_per_disk = n_sides_per_disk;
	tracks_per_side = n_tracks_per_side;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	density = n_density;
	if (density < 0 || 2 < density) density = 0;
	singles.Empty();
}
void DiskParam::SetDiskParam(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, const SingleDensities &n_singles)
{
	sides_per_disk = n_sides_per_disk;
	tracks_per_side = n_tracks_per_side;
	sectors_per_track = n_sectors_per_track;
	sector_size = n_sector_size;
	density = n_density;
	if (density < 0 || 2 < density) density = 0;
	singles = n_singles;
}
void DiskParam::SetDiskParam(const DiskParam &src)
{
	disk_type_name = src.disk_type_name;
	disk_type = src.disk_type;
	sides_per_disk = src.sides_per_disk;
	tracks_per_side = src.tracks_per_side;
	sectors_per_track = src.sectors_per_track;
	sector_size = src.sector_size;
	density = src.density;
	singles = src.singles;
}
void DiskParam::ClearDiskParam()
{
	disk_type_name.Empty();
	disk_type = 0;
	sides_per_disk = 0;
	tracks_per_side = 0;
	sectors_per_track = 0;
	sector_size = 0;
	density = 0;
	singles.Empty();
}
/// 一致するか
/// @return true:一致する
bool DiskParam::Match(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, const SingleDensities &n_singles)
{
	bool match = (sides_per_disk == n_sides_per_disk)
		&& (tracks_per_side == n_tracks_per_side)
		&& (sectors_per_track == n_sectors_per_track)
		&& (sector_size == n_sector_size);

	if (match) {
		match = (singles == n_singles);
	}
	return match;
}
/// 一致するか
/// @return true:一致する
bool DiskParam::Match(const DiskParam &param)
{
	bool match = (disk_type_name == param.disk_type_name)
		&& (sides_per_disk == param.sides_per_disk)
		&& (tracks_per_side == param.tracks_per_side)
		&& (sectors_per_track == param.sectors_per_track)
		&& (sector_size == param.sector_size);

	if (match) {
		match = (singles == param.singles);
	}
	return match;
}
/// 近い値で一致するか
/// @return true:一致する
bool DiskParam::MatchNear(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size)
{
	return ((sector_size == n_sector_size) // セクタサイズは一致
		&& (sides_per_disk == n_sides_per_disk) // サイド数は一致
		&& (sectors_per_track <= n_sectors_per_track) // セクタ数は小さければよし
		&& (tracks_per_side <= n_tracks_per_side) // トラック数は小さければよし
	);
}
/// 指定したトラック、サイドが単密度か
bool DiskParam::FindSingleDensity(int track_num, int side_num)  const
{
	bool match = false;
	for(size_t i=0; i<singles.Count(); i++) {
		if (singles[i].track_num < 0
		|| (singles[i].track_num == track_num && (singles[i].side_num < 0 || singles[i].side_num == side_num))) {
			match = true;
			break;
		}
	}
	return match;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskParams);

//
//
//
DiskTypes::DiskTypes()
{
}
bool DiskTypes::Load(const wxString &data_path)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("disk_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "L3DiskTypes") return false;

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
			long typ, spd, tps, spt, ss, den, strk, ssid;
			typ = spd = tps = spt = ss = den = strk = ssid = 0;
			wxString type_str = item->GetAttribute("type");
			if (!type_str.IsEmpty()) {
				type_str.ToLong(&typ);
			}
			SingleDensities sigs;
			wxString str;
			while (itemnode) {
				if (itemnode->GetName() == "SidesPerDisk") {
					str = itemnode->GetNodeContent();
					str.ToLong(&spd);
				} else if (itemnode->GetName() == "TracksPerSide") {
					str = itemnode->GetNodeContent();
					str.ToLong(&tps);
				} else if (itemnode->GetName() == "SectorsPerTrack") {
					str = itemnode->GetNodeContent();
					str.ToLong(&spt);
				} else if (itemnode->GetName() == "SectorSize") {
					str = itemnode->GetNodeContent();
					str.ToLong(&ss);
				} else if (itemnode->GetName() == "Density") {
					str = itemnode->GetNodeContent();
					str.ToLong(&den);
				} else if (itemnode->GetName() == "SingleDensity") {
					str = itemnode->GetAttribute("track");
					if (str.IsEmpty() || str.Upper() == wxT("ALL")) {
						strk = -1;
					} else {
						str.ToLong(&strk);
					}
					str = itemnode->GetAttribute("side");
					if (str.IsEmpty()) {
						ssid = -1;
					} else {
						str.ToLong(&ssid);
					}
					SingleDensity s(strk, ssid);
					sigs.Add(s);
				}
				itemnode = itemnode->GetNext();
			}
			DiskParam p(type_name, (int)typ, (int)spd, (int)tps, (int)spt, (int)ss, (int)den, sigs);
			types.Add(p);
		}
		item = item->GetNext();
	}
	return true;
}

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

DiskParam *DiskTypes::Find(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, const SingleDensities &n_singles)
{
	DiskParam *match_item = NULL;
	for(size_t i=0; i<types.Count(); i++) {
		DiskParam *item = &types[i];
		bool m = item->Match(n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_singles);
		if (m) {
			match_item = item;
			break;
		}
	}
	if (match_item == NULL) {
		// パラメータが一致しないときは、引数に近いパラメータ
		for(size_t i=0; i<types.Count(); i++) {
			DiskParam *item = &types[i];
			bool m = item->MatchNear(n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size);
			if (m) {
				match_item = item;
			}
		}
	}
	return match_item;
}
