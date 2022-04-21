/// @file diskd88.cpp
///
/// @brief D88ディスクイメージ入出力
///
#include "diskd88.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/hashmap.h>

/// disk density 0: 2D, 1: 2DD, 2: 2HD
const wxString gDiskDensity[] = { wxT("2D"), wxT("2DD"), wxT("2HD"), wxEmptyString };

/// (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
const int gSectorSizes[5] = { 128,256,512,1024,0 };

DiskTypes gDiskTypes;

//
//
//
SingleDensity::	SingleDensity(int n_track_num, int n_side_num)
{
	track_num = n_track_num;
	side_num = n_side_num;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(SingleDensities);

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
DiskParam::DiskParam(const wxString &n_type_name, wxUint32 n_disk_type, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, SingleDensities &n_singles)
{
	this->SetDiskParam(n_type_name, n_disk_type, n_sides_per_disk, n_tracks_per_side, n_sectors_per_track, n_sector_size, n_density, n_singles);
}
void DiskParam::SetDiskParam(const wxString &n_type_name, wxUint32 n_disk_type, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, SingleDensities &n_singles)
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
		if (singles.Count() != n_singles.Count()) {
			match = false;
		} else {
			for(size_t ni=0; ni<n_singles.Count(); ni++) {
				bool sm = false;
				for(size_t i=0; i<singles.Count(); i++) {
					const SingleDensity *s = &singles.Item(i);
					const SingleDensity *ns = &n_singles.Item(ni);
					if (s->track_num == ns->track_num && s->side_num == ns->side_num) {
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

//
//
//
DiskD88Sector::DiskD88Sector()
{
	num = 0;
	memset(&header, 0, sizeof(header));
	deleted = false;
	data = NULL;

	modified = false;
	memset(&header_origin, 0, sizeof(header_origin));
	data_origin = NULL;
}

DiskD88Sector::DiskD88Sector(const DiskD88Sector &src)
{
	// cannot copy
}

DiskD88Sector::DiskD88Sector(int newnum, const d88_sector_header_t &newhdr, wxUint8 *newdata)
{
	num = newnum;
	header = newhdr;
	deleted = (header.deleted == 0x10);
	data = newdata;

	modified = false;
	header_origin = newhdr;
	data_origin = new wxUint8[header.size];
	memcpy(data_origin, data, header.size);
}

DiskD88Sector::DiskD88Sector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density)
{
	num = sector_number;
	memset(&header, 0, sizeof(header));
	header.id.c = (wxUint8)track_number;
	header.id.h = (wxUint8)side_number;
	header.id.r = (wxUint8)sector_number;
	if (single_density) sector_size = 128;
	this->SetSectorSize(sector_size);
	header.size = (wxUint16)sector_size;
	header.secnums = (wxUint16)number_of_sector;
	header.density = (sector_size <= 128 ? 0x40 : 0);

	deleted = 0;
	data = new wxUint8[header.size];
	memset(data, 0, header.size);
	memset(&header_origin, 0xff, sizeof(header_origin));

	data_origin = new wxUint8[header.size];
	memset(data_origin, 0, header.size);

	modified = true;
}

DiskD88Sector::~DiskD88Sector()
{
	delete [] data;
	delete [] data_origin;
}

void DiskD88Sector::SetModify()
{
	modified = (memcmp(&header_origin, &header, sizeof(header)) != 0);
	if (!modified && data && data_origin) {
		modified = (memcmp(data_origin, data, header_origin.size) != 0);
	}
}
void DiskD88Sector::ClearModify()
{
	memcpy(&header_origin, &header, sizeof(header_origin));
	if (data && data_origin) {
		memcpy(data_origin, data, header_origin.size);
	}
	modified = false;
}
void DiskD88Sector::SetDeletedMark(bool val)
{
	deleted = val;
	header.deleted = (val ? 0x10 : 0);
}

bool DiskD88Sector::IsSameSector(int sector_number, bool deleted_mark)
{
	return (sector_number == num && deleted_mark == deleted);
}

/// セクタサイズ
int DiskD88Sector::GetSectorSize()
{
	int sec = ConvIDNToSecSize(header.id.n);
	if (sec > GetSectorBufferSize()) sec = GetSectorBufferSize(); 
	return sec;
}
void DiskD88Sector::SetSectorSize(int val)
{
	header.id.n = ConvSecSizeToIDN(val);
}

/// 単密度か
bool DiskD88Sector::IsSingleDensity()
{
	return (header.density == 0x40);
}
void DiskD88Sector::SetSingleDensity(bool val)
{
	header.density = (val ? 0x40 : 0);
}

int DiskD88Sector::Compare(DiskD88Sector *item1, DiskD88Sector *item2)
{
    return (item1->num - item2->num);
}
int DiskD88Sector::ConvIDNToSecSize(wxUint8 n)
{
	int sec = 0;
	if (n <= 3) sec = gSectorSizes[n];
	return sec;
}
wxUint8 DiskD88Sector::ConvSecSizeToIDN(int size)
{
	wxUint8 n = 1;
	for(int i=0; gSectorSizes[i] != 0; i++) {
		if (gSectorSizes[i] == size) {
			n = (wxUint8)i;
			break;
		}
	}
	return n;
}

//
//
//
DiskD88Track::DiskD88Track()
{
	trk_num = 0;
	sid_num = 0;
	offset  = 0;
	sectors = NULL;
}

DiskD88Track::DiskD88Track(const DiskD88Track &src)
{
	// cannot copy
}

DiskD88Track::DiskD88Track(int newtrknum, int newsidnum, wxUint32 newoffset)
{
	trk_num = newtrknum;
	sid_num = newsidnum;
	offset  = newoffset;
	sectors = NULL;
}

#if 0
DiskD88Track::DiskD88Track(int newtrknum, int newsidnum, wxUint32 newoffset, DiskD88Sectors *newsecs)
{
	trk_num = newtrknum;
	sid_num = newsidnum;
	offset  = newoffset;
//	sectors = new DiskD88Sectors(DiskD88Sector::Compare);
	sectors = new DiskD88Sectors;
	if (newsecs) {
		for(size_t i=0; i<newsecs->Count(); i++) {
			sectors->Add(new DiskD88Sector(*newsecs->Item(i)));
		}
	}
}
#endif

DiskD88Track::~DiskD88Track()
{
	if (sectors) {
		for(size_t i=0; i<sectors->Count(); i++) {
			DiskD88Sector *p = sectors->Item(i);
			delete p;
		}
		delete sectors;
	}
}

size_t DiskD88Track::Add(DiskD88Sector *newsec)
{
//	if (!sectors) sectors = new DiskD88Sectors(DiskD88Sector::Compare);
//	return sectors->Add(new DiskD88Sector(newsec));
	if (!sectors) sectors = new DiskD88Sectors;
	sectors->Add(newsec);
	return sectors->Count();
}

DiskD88Sector *DiskD88Track::GetSector(int sector_number)
{
	DiskD88Sector *sector = NULL;
	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskD88Sector *s = sectors->Item(pos);
			if (s->IsSameSector(sector_number)) {
				sector = s;
				break;
			}
		}
	}
	return sector;
}

void DiskD88Track::SetAllIDC(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDC(val);
			sector->SetModify();
		}
	}
}

void DiskD88Track::SetAllIDH(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDH(val);
			sector->SetModify();
		}
	}
}

void DiskD88Track::SetAllIDR(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDR(val);
			sector->SetModify();
		}
	}
}

void DiskD88Track::SetAllIDN(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDN(val);
			sector->SetModify();
		}
	}
}

void DiskD88Track::SetAllSingleDensity(bool val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetSingleDensity(val);
			sector->SetModify();
		}
	}
}

int DiskD88Track::GetMaxSectorNumber()
{
	int sector_number = 0;
	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskD88Sector *s = sectors->Item(pos);
			if (sector_number < s->GetSectorNumber()) {
				sector_number = s->GetSectorNumber();
			}
		}
	}
	return sector_number;
}

int DiskD88Track::GetMaxSectorSize()
{
	int sector_size = 0;
	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskD88Sector *s = sectors->Item(pos);
			if (sector_size < s->GetSectorSize()) {
				sector_size = s->GetSectorSize();
			}
		}
	}
	return sector_size;
}

bool DiskD88Track::IsModified() const
{
	if (!sectors) return false;
	bool modified = false;
	for(size_t sector_num = 0; sector_num < sectors->Count() && !modified; sector_num++) {
		DiskD88Sector *sector = sectors->Item(sector_num);
		if (!sector) continue;

		modified = sector->IsModified();
		if (modified) break;
	}
	return modified;
}

void DiskD88Track::ClearModify()
{
	if (!sectors) return;
	for(size_t sector_num = 0; sector_num < sectors->Count(); sector_num++) {
		DiskD88Sector *sector = sectors->Item(sector_num);
		if (!sector) continue;

		sector->ClearModify();
	}
}

int DiskD88Track::Compare(DiskD88Track *item1, DiskD88Track *item2)
{
    return ((item1->trk_num - item2->trk_num) | (item1->sid_num - item2->sid_num));
}

//
//
//
DiskD88Disk::DiskD88Disk() : DiskParam()
{
	num = 0;
	memset(&header, 0, sizeof(header));
	tracks = NULL;

	memset(&header_origin, 0, sizeof(header_origin));

//	buffer = NULL;
//	buffer_size = 0;

	modified = false;
}

DiskD88Disk::DiskD88Disk(const wxString &newname, int newnum, const DiskParam &param, bool write_protect) : DiskParam(param)
{
	num = newnum;
	memset(&header, 0, sizeof(header));
	tracks = NULL;

	memset(&header_origin, 0xff, sizeof(header_origin));

	this->SetName(newname);
	this->SetDensity(param.GetDensity());
	this->SetWriteProtect(write_protect);

	modified = true;
}

DiskD88Disk::DiskD88Disk(const DiskD88Disk &src) : DiskParam(src)
{
	// cannot copy
}

DiskD88Disk::DiskD88Disk(int newnum, const d88_header_t &newhdr) : DiskParam()
{
	num = newnum;
	header = newhdr;
	tracks = NULL;

	header_origin = newhdr;

//	buffer = NULL;
//	buffer_size = 0;

	modified = false;
}

#if 0
DiskD88Disk::DiskD88Disk(wxUint8 *newbuf, size_t newbuflen, int newnum)
{
	num = newnum;
	write_protect = newwriteprotect;
	tracks = NULL;

	buffer = newbuf;
	buffer_size = newbuflen;

	header = (d88_header_t *)newbuf;
}
#endif

#if 0
DiskD88Disk::DiskD88Disk(int newnum, d88_header_t *newhdr, DiskD88Tracks *newtrks) : DiskParam()
{
	num = newnum;
	write_protect = newwriteprotect;
	header = newhdr;
//	tracks = new DiskD88Tracks(DiskD88Track::Compare);
	tracks = new DiskD88Tracks;
	if (newtrks) {
		for(size_t i=0; i<newtrks->Count(); i++) {
			newtrks->Add(new DiskD88Track(*newtrks->Item(i)));
		}
	}

	this->CalcMajorNumber();
}
#endif

DiskD88Disk::~DiskD88Disk()
{
	if (tracks) {
		for(size_t i=0; i<tracks->Count(); i++) {
			DiskD88Track *p = tracks->Item(i);
			delete p;
		}
		delete tracks;
	}
//	delete [] buffer;
}

size_t DiskD88Disk::Add(DiskD88Track *newtrk)
{
//	if (!tracks) tracks = new DiskD88Tracks(DiskD88Track::Compare);
//	return tracks->Add(new DiskD88Track(newtrk));
	if (!tracks) tracks = new DiskD88Tracks;
	tracks->Add(newtrk);
	return tracks->Count();
}

void DiskD88Disk::SetModify()
{
	modified = (memcmp(&header_origin, &header, sizeof(header)) != 0);
}

void DiskD88Disk::ClearModify()
{
	memcpy(&header_origin, &header, sizeof(header_origin));
	modified = false;
}

bool DiskD88Disk::IsModified()
{
	if (!modified && tracks) {
		for(size_t track_num = 0; track_num < tracks->Count() && !modified; track_num++) {
			DiskD88Track *track = tracks->Item(track_num);
			if (!track) continue;

			modified = track->IsModified();
			if (modified) break;
		}
	}
	return modified;
}

wxString DiskD88Disk::GetName()
{
	wxString name(header.diskname, 16);
	if (name.IsEmpty()) {
		name = _("(no name)");
	}
	return name;
}

void DiskD88Disk::SetName(const wxString &val)
{
	wxString name = val;
	if (name == _("(no name)")) {
		name.Empty();
	}
	strncpy((char *)header.diskname, name.ToAscii(), 16);
}

DiskD88Track *DiskD88Disk::GetTrack(int track_number, int side_number)
{
	DiskD88Track *track = NULL;
	for(size_t pos=0; pos<tracks->Count(); pos++) {
		DiskD88Track *t = tracks->Item(pos);
		if (t->GetTrackNumber() == track_number && t->GetSideNumber() == side_number) {
			track = t;
			break;
		}
	}
	return track;
}
DiskD88Track *DiskD88Disk::GetTrackByOffset(wxUint32 offset)
{
	DiskD88Track *track = NULL;
	for(size_t pos=0; pos<tracks->Count(); pos++) {
		DiskD88Track *t = tracks->Item(pos);
		if (t->GetOffset() == offset) {
			track = t;
			break;
		}
	}
	return track;
}

WX_DECLARE_STRING_HASH_MAP( int, IntHashMap );
#define INCLEASE_HASH_MAP(hash_map, value) { \
	val = value; \
	key = wxString::Format(wxT("%d"), val); \
	hash_map[key]++; \
}
#define GET_MAXVALUE_FROM_HASH_MAP(hash_map, long_value) { \
	for(val = 0, it = hash_map.begin(); it != hash_map.end(); it++) { \
		if (val < it->second) { \
			val = it->second; \
			it->first.ToLong(&long_value); \
		} \
	} \
}
#define SET_MAXVALUE(long_value, value) { \
	val = value; \
	if (long_value < val) { \
		long_value = val; \
	} \
}

/// ディスクの中でもっともらしいパラメータを設定
void DiskD88Disk::CalcMajorNumber()
{
	IntHashMap sector_size_map;
	IntHashMap::iterator it;

	long track_number = 0;
	long side_number = 0;
	long sector_number = 0;
	long sector_masize = 0;
	SingleDensities singles;
	int val;
	wxString key;

	if (tracks) {
		for(size_t ti=0; ti<tracks->Count(); ti++) {
			DiskD88Track *t =tracks->Item(ti);

			SET_MAXVALUE(track_number, t->GetTrackNumber());
			SET_MAXVALUE(side_number, t->GetSideNumber());
			SET_MAXVALUE(sector_number, t->GetMaxSectorNumber());
			INCLEASE_HASH_MAP(sector_size_map, t->GetMaxSectorSize());	// セクタサイズはディスク内で最も使用されているサイズ

			DiskD88Sector *s =t->GetSector(1);
			if (s && s->IsSingleDensity()) {
				SingleDensity sd(t->GetTrackNumber(), t->GetSideNumber());
				singles.Add(sd);
			}
		}
	}
	GET_MAXVALUE_FROM_HASH_MAP(sector_size_map, sector_masize);

	sides_per_disk = (int)side_number + 1;
	tracks_per_side = (int)track_number + 1;
	sectors_per_track = (int)sector_number;
	sector_size = (int)sector_masize;

	// メディアのタイプ
	DiskParam *disk_param = gDiskTypes.Find(sides_per_disk, tracks_per_side, sectors_per_track, sector_size, singles);
	if (disk_param != NULL) {
		disk_type_name = disk_param->GetDiskTypeName();
		disk_type = disk_param->GetDiskType();
//		SetDiskParam(*disk_type);
	}
}

wxString DiskD88Disk::GetAttrText()
{
	wxString str;

	str = disk_type_name;
	str += wxT("  ");
	str += wxString::Format(_("%dSide(s) %dTracks/Side %dSectors/Track %dbytes/Sector"), sides_per_disk, tracks_per_side, sectors_per_track, sector_size);

	return str;
}

bool DiskD88Disk::GetWriteProtect()
{
	return (header.write_protect != 0);
}

void DiskD88Disk::SetWriteProtect(bool val)
{
	header.write_protect = (val ? 0x10 : 0);
}

wxString DiskD88Disk::GetDensityText()
{
	wxUint8 num = ((header.disk_density) >> 4);
	if (num > 2) num = 3;
	return gDiskDensity[num];
}

void DiskD88Disk::SetDensity(int val)
{
	if (0 <= val && val <= 2) {
		header.disk_density = (val << 4);
	}
}

wxUint32 DiskD88Disk::GetSize()
{
	return header.disk_size;
}

wxUint32 DiskD88Disk::GetDiskSize()
{
	return header.disk_size - (wxUint32)sizeof(header);
}

/// @param [in] val サイズ（ヘッダサイズを除く）
void DiskD88Disk::SetSize(wxUint32 val)
{
	header.disk_size = (val + sizeof(header));
}

wxUint32 DiskD88Disk::GetOffset(int num)
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return 0;
	return header.offsets[num];
}

/// @param [in] offset 位置（ヘッダサイズを除く）
void DiskD88Disk::SetOffset(int num, wxUint32 offset)
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return;
	header.offsets[num] = (offset + sizeof(header));
}

/// ディスクの内容を初期化する(0パディング)
/// @param [in] selected_side >=0なら指定サイドのみ初期化
bool DiskD88Disk::Initialize(int selected_side)
{
	if (!tracks) {
		return false;
	}

	bool rc = true;
	for(size_t track_pos=0; track_pos<tracks->Count(); track_pos++) {
		DiskD88Track *track = tracks->Item(track_pos);
		if (selected_side >= 0) {
			// サイド指定ありの時はそのサイドのみ初期化
			if (selected_side != track->GetSideNumber()) {
				continue;
			}
		}

		DiskD88Sectors *secs = track->GetSectors();
		if (!secs) {
			rc = false;
			continue;
		}

		for(size_t sec_pos=0; sec_pos<secs->Count(); sec_pos++) {
			DiskD88Sector *sec = secs->Item(sec_pos);
			wxUint8 *buffer = sec->GetSectorBuffer();
			size_t buffer_size = sec->GetSectorBufferSize();

			memset(buffer, 0, buffer_size);

		}
	}
	return rc;
}

int DiskD88Disk::Compare(DiskD88Disk *item1, DiskD88Disk *item2)
{
    return (item1->num - item2->num);
}


//
//
//
DiskD88File::DiskD88File()
{
	disks = NULL;
	modified = false;
}

DiskD88File::DiskD88File(const DiskD88File &src)
{
#if 0 // cannot copy
//	disks = new DiskD88Disks(DiskD88Disk::Compare);
	disks = new DiskD88Disks;
	if (src.disks) {
		for(size_t i=0; i<src.disks->Count(); i++) {
			disks->Add(new DiskD88Disk(*src.disks->Item(i)));
		}
	}
#endif
}

DiskD88File::~DiskD88File()
{
	Clear();
}

size_t DiskD88File::Add(DiskD88Disk *newdsk)
{
//	if (!disks) disks = new DiskD88Disks(DiskD88Disk::Compare);
//	return disks->Add(new DiskD88Disk(newdsk));
	if (!disks) disks = new DiskD88Disks;
	disks->Add(newdsk);
	return disks->Count();
}

void DiskD88File::Clear()
{
	if (disks) {
		for(size_t i=0; i<disks->Count(); i++) {
			DiskD88Disk *p = disks->Item(i);
			delete p;
		}
		delete disks;
		disks = NULL;
	}
}

size_t DiskD88File::Count() const
{
	if (!disks) return 0;
	return disks->Count();
}

bool DiskD88File::Delete(size_t idx)
{
	DiskD88Disk *disk = GetDisk(idx);
	if (!disk) return false;
	delete disk;
	disks->RemoveAt(idx);
	return true;
}

DiskD88Disk *DiskD88File::GetDisk(size_t idx)
{
	if (!disks) return NULL;
	if (idx >= disks->Count()) return NULL;
	return disks->Item(idx);
}

void DiskD88File::SetModify()
{
	modified = true;
}

bool DiskD88File::IsModified()
{
	if (!modified && disks) {
		for(size_t disk_num = 0; disk_num < disks->Count() && !modified; disk_num++) {
			DiskD88Disk *disk = disks->Item(disk_num);
			if (!disk) continue;

			modified = disk->IsModified();
			if (modified) break;
		}
	}
	return modified;
}

void DiskD88File::ClearModify()
{
	modified = false;
}

//
//
//
void DiskD88Result::SetMessage(int error_number, va_list ap)
{
	wxString msg;
	switch(error_number) {
	case ERR_CANNOT_OPEN:
// use system message.
//		msg = (_("Cannot open file."));
		break;
	case ERR_CANNOT_SAVE:
// use system message.
//		msg = (_("Cannot save file."));
		break;
	case ERR_INVALID_DISK:
		msg = wxString::FormatV(_("[Disk%d] This is not supported disk."), ap);
		break;
	case ERR_OVERFLOW:
		msg = wxString::FormatV(_("[Disk%d] Overflow offset. offset:%d disk size:%d"), ap);
		break;
	case ERR_ID_TRACK:
		msg = wxString::FormatV(_("[Disk%d] Unmatch id C and track number %d. id[C:%d H:%d R:%d]"), ap);
		break;
	case ERR_ID_SIDE:
		msg = wxString::FormatV(_("[Disk%d] Invalid id H in track %d. id[C:%d H:%d R:%d]"), ap);
		break;
	case ERR_ID_SECTOR:
		msg = wxString::FormatV(_("[Disk%d] Invalid id R in track %d. id[C:%d H:%d R:%d] num of sector:%d"), ap);
		break;
	case ERR_SECTOR_SIZE:
		msg = wxString::FormatV(_("[Disk%d] Invalid sector size in sector. id[C:%d H:%d R:%d N:%d] sector size:%d"), ap);
		break;
	case ERR_DUPLICATE_TRACK:
		msg = wxString::FormatV(_("[Disk%d] Duplicate track %d and side %d. Side number change to %d."), ap);
		break;
	case ERR_IGNORE_DATA:
		msg = wxString::FormatV(_("[Disk%d] Deleted data found. This sector is ignored. id[C:%d H:%d R:%d]"), ap);
		break;
	case ERR_NO_DATA:
		msg = (_("No data exists."));
		break;
	case ERR_NO_DISK:
		msg = (_("No disk exists."));
		break;
	default:
		msg = wxString::Format(_("Unknown error. code:%d"), error_number);
		break;
	}
	if (!msg.IsEmpty()) msgs.Add(msg);
}

//
//
//
DiskD88Parser::DiskD88Parser(wxInputStream *stream, DiskD88File *file, DiskD88Result &result)
{
	this->stream = stream;
	this->file = file;
	this->result = &result;
}

DiskD88Parser::~DiskD88Parser()
{
}

/// セクタデータの解析
wxUint32 DiskD88Parser::ParseSector(int disk_number, int track_number, int sector_nums, DiskD88Track *track)
{
	d88_sector_header_t sector_header;

	size_t read_size = stream->Read((void *)&sector_header, sizeof(d88_sector_header_t)).LastRead();
			
	// track number is same ?
	if (sector_header.id.c != track_number) {
		result->SetWarn(DiskD88Result::ERR_ID_TRACK, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
	}
	// side number is valid ?
	if (sector_header.id.h > 1) {
		result->SetWarn(DiskD88Result::ERR_ID_SIDE, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
	}
	// sector number is valid ?
	int sector_number = sector_header.id.r;
	if (sector_number <= 0 || sector_nums < sector_number) {
		result->SetWarn(DiskD88Result::ERR_ID_SECTOR, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_nums);
	}
	// deleted data ?
//	if (sector_ptr->deleted != 0) {
//		result->SetWarn(DiskD88Result::ERR_IGNORE_DATA, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
//	}
	// invalid sector size
	size_t real_size = sector_header.size;
	if (sector_header.size > 1024 * 2) {
		result->SetError(DiskD88Result::ERR_SECTOR_SIZE, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_header.id.n, sector_header.size);
		return (wxUint32)read_size;
	} else if (sector_header.size > 1024) {
		result->SetWarn(DiskD88Result::ERR_SECTOR_SIZE, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_header.id.n, sector_header.size);
		sector_header.size = 1024;
	}

	// 追加
	if (result->GetValid() >= 0) {
		wxUint8 *sector_data = new wxUint8[sector_header.size];
		stream->Read((void *)sector_data, sector_header.size);

		DiskD88Sector *sector = new DiskD88Sector(sector_number, sector_header, sector_data);
		track->Add(sector);

		if (real_size > sector_header.size) {
			stream->SeekI(real_size - sector_header.size, wxFromCurrent);
		}
	}

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + sector_header.size;
}

/// トラックデータの解析
wxUint32 DiskD88Parser::ParseTrack(size_t start_pos, wxUint32 offset, int disk_number, DiskD88Disk *disk)
{
	d88_sector_header_t track_header;

	stream->SeekI(start_pos + offset, wxFromStart);
	size_t read_size = stream->Read((void *)&track_header, sizeof(d88_sector_header_t)).LastRead();
	if (read_size > 0) {
		stream->SeekI((int)read_size * -1, wxFromCurrent);
	}

	int track_number = track_header.id.c;
	int side_number = track_header.id.h;
	int sector_nums = track_header.secnums;
//	int sector_size = track_header.size;

	DiskD88Track *track = new DiskD88Track(track_number, side_number, offset);

	// sectors
	wxUint32 track_size = 0;
	for(int sec_pos = 0; sec_pos < sector_nums && result->GetValid() >= 0; sec_pos++) {
		track_size += ParseSector(disk_number, track_number, sector_nums, track);
	}

	if (result->GetValid() >= 0) {
		// 重複チェック
		DiskD88Tracks *tracks = disk->GetTracks();
		if (tracks) {
			bool dup = false;
			do {
				dup = false;
				for(size_t i=0; i<tracks->Count(); i++) {
					DiskD88Track *t = tracks->Item(i);
					if (t->GetTrackNumber() == track_number && t->GetSideNumber() == side_number) {
						// すでに同じトラック番号とサイド番号がある
						result->SetWarn(DiskD88Result::ERR_DUPLICATE_TRACK, disk_number, track_number, side_number, side_number + 1);
						// サイド番号を変更する
						side_number++;
						track->SetSideNumber(side_number);
						dup = true;
						break;	// チェックやり直し
					}
				}
			} while(dup);
		}
		// トラックを追加
		disk->Add(track);
	} else {
		delete track;
	}

	return track_size;
}

/// ディスクデータの解析
wxUint32 DiskD88Parser::ParseDisk(size_t start_pos, int disk_number)
{
	d88_header_t disk_header;

	wxUint32 read_size = (wxUint32)stream->Read((void *)&disk_header, sizeof(d88_header_t)).LastRead();
//	if (read_size > 0) {
//		stream->SeekI((int)read_size * -1, wxFromCurrent);
//	}

	// ディスクサイズが小さすぎる
	if (read_size < sizeof(d88_header_t)) {
			result->SetError(DiskD88Result::ERR_INVALID_DISK, disk_number);
			return read_size;
	}

	wxUint32 disk_size = disk_header.disk_size;
	wxUint32 stream_size = (wxUint32)stream->GetLength();
	// ディスクサイズがファイルサイズより大きい、または4MBを超えている
	if (stream_size < disk_size || (1024*1024*4) < disk_size) {
			result->SetError(DiskD88Result::ERR_INVALID_DISK, disk_number);
			return stream_size;
	}
	// 名前の17文字目は'\0'
	if (disk_header.diskname[16] != '\0') {
			result->SetError(DiskD88Result::ERR_INVALID_DISK, disk_number);
			return disk_size;
	}

	DiskD88Disk *disk = new DiskD88Disk(disk_number, disk_header);

	// parse tracks
	for(int pos = 0; pos < DISKD88_MAX_TRACKS && result->GetValid() >= 0; pos++) {
		wxUint32 offset = disk_header.offsets[pos];
		if (offset == 0) continue;

		// オフセットがディスクサイズを超えている？
		if (offset >= disk_size) {
			result->SetError(DiskD88Result::ERR_OVERFLOW, disk_number, offset, disk_size);
			break;
		}

		ParseTrack(start_pos, offset, disk_number, disk);
	}

	if (result->GetValid() >= 0) {
		// ディスクを追加
		disk->CalcMajorNumber();
		file->Add(disk);
	} else {
		delete disk;
	}

	return disk_size;
}

int DiskD88Parser::Parse()
{
	size_t read_size = 0;
	size_t stream_size = stream->GetLength();
	int disk_number = (int)file->Count();
	for(; read_size < stream_size && result->GetValid() >= 0; disk_number++) {
		read_size += ParseDisk(read_size, disk_number);
	}
	return result->GetValid();
}

//
//
//
DiskD88Creator::DiskD88Creator(const wxString &diskname, const DiskParam &param, bool write_protect, DiskD88File *file, DiskD88Result &result)
{
	this->diskname = diskname;
	this->param = &param;
	this->write_protect = write_protect;
	this->file = file;
	this->result = &result;
}

DiskD88Creator::~DiskD88Creator()
{
}

/// セクタデータの作成
wxUint32 DiskD88Creator::CreateSector(int track_number, int side_number, int sector_number, DiskD88Track *track)
{
	bool single_density = param->FindSingleDensity(track_number, side_number);
	DiskD88Sector *sector = new DiskD88Sector(track_number, side_number, sector_number, param->GetSectorSize(), param->GetSectorsPerTrack(), single_density);
	track->Add(sector);

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + sector->GetSectorBufferSize();
}

/// トラックデータの作成
wxUint32 DiskD88Creator::CreateTrack(int track_number, int side_number, wxUint32 offset, DiskD88Disk *disk)
{
	DiskD88Track *track = new DiskD88Track(track_number, side_number, offset);

	// create sectors
	wxUint32 track_size = 0;
	for(int sector_number = 1; sector_number <= param->GetSectorsPerTrack() && result->GetValid() >= 0; sector_number++) {
		track_size += CreateSector(track_number, side_number, sector_number, track);
	}

	if (result->GetValid() >= 0) {
		// トラックを追加
		disk->Add(track);
	} else {
		delete track;
	}

	return track_size;
}

/// ディスクデータの作成
wxUint32 DiskD88Creator::CreateDisk(int disk_number)
{
	DiskD88Disk *disk = new DiskD88Disk(diskname, disk_number, *param, write_protect);

	// create tracks
	size_t create_size = 0;
	int track_num = 0;
	int side_num = 0;
	for(int pos = 0; pos < DISKD88_MAX_TRACKS && result->GetValid() >= 0; pos++) {
		disk->SetOffset(pos, (wxUint32)create_size);

		create_size += CreateTrack(track_num, side_num, disk->GetOffset(pos), disk);

		side_num++;
		if (side_num >= param->GetSidesPerDisk()) {
			track_num++;
			side_num = 0;
		}
		if (track_num >= param->GetTracksPerSide()) {
			break;
		}
	}

	if (result->GetValid() >= 0) {
		// ディスクを追加
		disk->CalcMajorNumber();
		disk->SetSize((wxUint32)create_size);
		file->Add(disk);
	} else {
		delete disk;
	}

	return (wxUint32)create_size;
}

int DiskD88Creator::Create()
{
	CreateDisk(0);
	return result->GetValid();
}

int DiskD88Creator::Add()
{
	int disk_number = 0;
	DiskD88Disks *disks = file->GetDisks();
	if (disks) {
		disk_number = (int)disks->Count();
	}

	CreateDisk(disk_number);
	return result->GetValid();
}

//
//
//
DiskD88::DiskD88()
{
#ifdef DISKD88_USE_MEMORY_INPUT_STREAM
	stream = NULL;
#endif
	file = NULL;
}

DiskD88::~DiskD88()
{
	ClearFile();
#ifdef DISKD88_USE_MEMORY_INPUT_STREAM
	CloseStream();
#endif
}

/// 新規作成
int DiskD88::Create(const wxString &diskname, const DiskParam &param, bool write_protect)
{
	result.Clear();

	NewFile(wxEmptyString);
	DiskD88Creator cr(diskname, param, write_protect, file, result);
	int valid_disk = cr.Create();

	// エラーあり
	if (valid_disk < 0) {
		ClearFile();
	}

	return valid_disk;
}

/// 追加作成
int DiskD88::Add(const wxString &diskname, const DiskParam &param, bool write_protect)
{
	if (!file) return 0;

	result.Clear();

	DiskD88Creator cr(diskname, param, write_protect, file, result);
	int valid_disk = cr.Add();

	// エラーなし
	if (valid_disk >= 0) {
		file->SetModify();
	}

	return valid_disk;
}

/// ファイルを追加
/// @return 0:正常 -1:エラーあり 1:警告あり
int DiskD88::Add(const wxString &filepath)
{
	if (!file) return 0;

	result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskD88Result::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskD88Parser ps(&fstream, file, result);
	int valid_disk = ps.Parse();

	// エラーなし
	if (valid_disk >= 0) {
		file->SetModify();
	}

	return valid_disk;
}

/// ファイルを開く
/// @return 0:正常 -1:エラーあり 1:警告あり
int DiskD88::Open(const wxString &filepath)
{
	result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskD88Result::ERR_CANNOT_OPEN);
		return -1;
	}

	NewFile(filepath);
	DiskD88Parser ps(&fstream, file, result);
	int valid_disk = ps.Parse();

	// エラーあり
	if (valid_disk < 0) {
		ClearFile();
	}

	return valid_disk;
}

/// 閉じる
void DiskD88::Close()
{
#ifdef DISKD88_USE_MEMORY_INPUT_STREAM
	CloseStream();
#endif
	ClearFile();
	filename.Clear();
}

/// ストリームの内容をファイルに保存
int DiskD88::Save(const wxString &filepath)
{
	result.Clear();
	if (!file) {
		result.SetError(DiskD88Result::ERR_NO_DATA);
		return result.GetValid();
	}

	wxFileOutputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskD88Result::ERR_CANNOT_SAVE);
		return result.GetValid();
	}

	DiskD88Disks *disks = file->GetDisks();
	if (!disks || disks->Count() <= 0) {
		result.SetError(DiskD88Result::ERR_NO_DISK);
		return result.GetValid();
	}
	for(size_t disk_num = 0; disk_num < disks->Count(); disk_num++) {
		DiskD88Disk *disk = disks->Item(disk_num);
		SaveDisk(disk, &fstream); 
	}

	if (result.GetValid() >= 0) {
		file->ClearModify();
		filename = filepath;
	}

	return result.GetValid();
}
/// ストリームの内容をファイルに保存
int DiskD88::SaveDisk(int disk_number, const wxString &filepath)
{
	result.Clear();
	if (!file) {
		result.SetError(DiskD88Result::ERR_NO_DATA);
		return result.GetValid();
	}

	wxFileOutputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskD88Result::ERR_CANNOT_SAVE);
		return result.GetValid();
	}

	DiskD88Disk *disk = file->GetDisk(disk_number);
	SaveDisk(disk, &fstream); 

	return result.GetValid();
}
/// ディスク1つを保存
int DiskD88::SaveDisk(DiskD88Disk *disk, wxOutputStream *stream)
{
	if (!disk) {
		result.SetError(DiskD88Result::ERR_NO_DISK);
		return result.GetValid();
	}

	// write disk header
	stream->Write((void *)disk->GetHeader(), sizeof(d88_header_t));	

	DiskD88Tracks *tracks = disk->GetTracks();
	if (!tracks) {
		result.SetError(DiskD88Result::ERR_NO_DATA);
		return result.GetValid();
	}

	for(size_t track_num = 0; track_num < tracks->Count(); track_num++) {
		DiskD88Track *track = tracks->Item(track_num);
		if (!track) continue;
		DiskD88Sectors *sectors = track->GetSectors();
		if (!sectors) continue;
		for(size_t sector_num = 0; sector_num < sectors->Count(); sector_num++) {
			DiskD88Sector *sector = sectors->Item(sector_num);
			if (!sector) continue;

			// write sector header
			stream->Write((void *)sector->GetHeader(), sizeof(d88_sector_header_t));	
			// write sector body
			wxUint8 *buffer = sector->GetSectorBuffer();
			size_t buffer_size = sector->GetSectorBufferSize();
			if (buffer && buffer_size) {
				stream->Write((void *)buffer, buffer_size);	
			}
			sector->ClearModify();
		}
	}
	disk->ClearModify();

	return result.GetValid();
}
/// ディスクを削除
bool DiskD88::Delete(size_t disk_number)
{
	if (!file) return false;
	file->Delete(disk_number);
	file->SetModify();
	return true;
}

/// ディスク名を設定
bool DiskD88::SetDiskName(size_t disk_number, const wxString &newname)
{
	DiskD88Disk *disk = GetDisk(disk_number);
	if (!disk) return false;

	if (disk->GetName() != newname) {
		disk->SetName(newname);
		disk->SetModify();
		return true;
	}
	return false;
}
const wxString DiskD88::GetDiskName(size_t disk_number)
{
	DiskD88Disk *disk = GetDisk(disk_number);
	if (!disk) return wxEmptyString;
	return disk->GetName();
}

/// ファイル構造体を作成
void DiskD88::NewFile(const wxString &filepath)
{
	if (file) {
		delete file;
	}
	file = new DiskD88File;
	filename = filepath;
}

/// ファイル構造体をクリア
void DiskD88::ClearFile()
{
	delete file;
	file = NULL;
}
/// ディスクを変更したか
bool DiskD88::IsModified()
{
	bool modified = false;
	if (file) {
		modified = file->IsModified();
	}
	return modified;
}
/// ディスク枚数
size_t DiskD88::CountDisks() const
{
	if (!file) return 0;
	return file->Count();
}
/// ディスク一覧を返す
DiskD88Disks *DiskD88::GetDisks()
{
	if (!file) return NULL;
	return file->GetDisks();
}
/// 指定した位置のディスクを返す
DiskD88Disk *DiskD88::GetDisk(size_t index)
{
	if (!file) return NULL;
	return file->GetDisk(index);
}
/// 指定した位置のディスクのタイプ
int DiskD88::GetDiskTypeNumber(size_t index)
{
	if (!file) return -1;
	DiskD88Disk *disk = file->GetDisk(index);
	if (!disk) return -1;
	return gDiskTypes.IndexOf(disk->GetDiskTypeName());
}

wxString DiskD88::GetFileName() const
{
	return filename.GetFullName();
}

wxString DiskD88::GetFilePath() const
{
	return filename.GetFullPath();
}

wxString DiskD88::GetPath() const
{
	return filename.GetPath();
}

/// エラーメッセージ
const wxArrayString &DiskD88::GetErrorMessage(int maxrow)
{
	return result.GetMessages(maxrow);
}
