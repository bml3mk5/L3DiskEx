/// @file diskd88.cpp
///
/// @brief D88ディスクイメージ入出力
///
#include "diskd88.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/hashmap.h>
#include "diskd88parser.h"
#include "diskd88creator.h"

/// disk density 0: 2D, 1: 2DD, 2: 2HD
const wxString gDiskDensity[] = { wxT("2D"), wxT("2DD"), wxT("2HD"), wxEmptyString };

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

/// セクタのデータを置き換える
/// セクタサイズは変更しない
bool DiskD88Sector::Replace(DiskD88Sector *src_sector)
{
	wxUint8 *src_data = src_sector->data;
	if (!src_data) {
		// データなし
		return false;
	}
	memset(data, 0, header.size);
	memcpy(data, src_data, header.size);
	SetModify();
	return true;
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
	size    = 0;
}

DiskD88Track::DiskD88Track(const DiskD88Track &src)
{
	// cannot copy
}

DiskD88Track::DiskD88Track(int newtrknum, int newsidnum, int newoffpos, wxUint32 newoffset)
{
	trk_num = newtrknum;
	sid_num = newsidnum;
	offset_pos = newoffpos;
	offset  = newoffset;
	sectors = NULL;
	size    = 0;
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
	size    = 0;
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

/// トラック内のセクタデータを置き換える
/// @param [in] src_track
/// @return 0:正常 -1:エラー 1:置換できないセクタあり
int DiskD88Track::Replace(DiskD88Track *src_track)
{
	int rc = 0;
	if (!sectors) return -1;
	for(size_t i=0; i<sectors->Count(); i++) {
		DiskD88Sector *sector = sectors->Item(i);
		DiskD88Sector *src_sector = src_track->GetSector(sector->GetSectorNumber());
		if (!src_sector) {
			// セクタなし
			continue;
		}
		if (!sector->Replace(src_sector)) {
			rc = 1;
		}
	}
	return rc;
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

/// ディスクにトラックを追加
/// @return トラック数
size_t DiskD88Disk::Add(DiskD88Track *newtrk)
{
//	if (!tracks) tracks = new DiskD88Tracks(DiskD88Track::Compare);
//	return tracks->Add(new DiskD88Track(newtrk));
	if (!tracks) tracks = new DiskD88Tracks;
	tracks->Add(newtrk);
	return tracks->Count();
}

/// ディスクの内容を置き換える
/// @param [in] side_number : サイド番号
/// @param [in] src_disk : 置換元のディスクイメージ
/// @param [in] src_side_number : 置換元のディスクイメージのサイド番号
int DiskD88Disk::Replace(int side_number, DiskD88Disk *src_disk, int src_side_number)
{
	int rc = 0;
	if (!tracks) return -1;
	for(size_t i=0; i<tracks->Count(); i++) {
		DiskD88Track *track = tracks->Item(i);
		int curr_side_number = track->GetSideNumber();
		if (side_number >= 0) {
			if (curr_side_number != side_number) {
				continue;
			}
			curr_side_number = src_side_number;
		}
		DiskD88Track *src_track = src_disk->GetTrack(track->GetTrackNumber(), curr_side_number);
		int rct = track->Replace(src_track);
		if (rct != 0) rc = rct;
	}
	return rc;
}

/// トラックを削除する
size_t DiskD88Disk::DeleteTracks(int start_offset_pos, int end_offset_pos)
{
	size_t deleted_size = 0;

	if (!tracks) return deleted_size;

	for(size_t i=0; i<tracks->Count(); i++) {
		DiskD88Track *track = tracks->Item(i);
		if (!track) continue;
		int pos = track->GetOffsetPos();
		if (pos < start_offset_pos) continue;
		if (end_offset_pos >= start_offset_pos && pos > end_offset_pos) continue;

		wxUint32 track_size = track->GetSize();
		tracks->Remove(track);
		SetOffset(pos, 0);
		delete track;

		deleted_size += track_size;
		SetSize(GetSize() - track_size);

		SetModify();

		i--; // because tracks count is declement
	}
	return deleted_size;
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

wxString DiskD88Disk::GetName(bool real)
{
	wxString name((const char *)header.diskname);
	if (!real && name.IsEmpty()) {
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
	header.diskname[16] = 0;
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
	if (tracks) {
		for(size_t pos=0; pos<tracks->Count(); pos++) {
			DiskD88Track *t = tracks->Item(pos);
			if (t->GetOffset() == offset) {
				track = t;
				break;
			}
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

	if (tracks) {
		if (sector_masize == 128 && side_number == 0 && offset_maxpos > track_number) {
			// 単密度で両面タイプ
			side_number++;
			for(size_t ti=0; ti<tracks->Count(); ti++) {
				DiskD88Track *t =tracks->Item(ti);
				if (t->GetOffsetPos() & 1) {
					// 奇数の場合はサイドを1にする
					t->SetSideNumber(1);
				}
			}
		}
	}

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

/// ディスクサイズ（ヘッダサイズ含む）
wxUint32 DiskD88Disk::GetSize()
{
	return header.disk_size;
}

/// @param [in] val サイズ（ヘッダサイズ含む）
void DiskD88Disk::SetSize(wxUint32 val)
{
	header.disk_size = val;
}

/// ディスクサイズ（ヘッダサイズを除く）
wxUint32 DiskD88Disk::GetSizeWithoutHeader()
{
	return header.disk_size - (wxUint32)sizeof(header);
}

/// @param [in] val サイズ（ヘッダサイズを除く）
void DiskD88Disk::SetSizeWithoutHeader(wxUint32 val)
{
	header.disk_size = (val + sizeof(header));
}

/// offset 位置（ヘッダサイズ含む）
wxUint32 DiskD88Disk::GetOffset(int num)
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return 0;
	return header.offsets[num];
}

/// @param [in] offset 位置（ヘッダサイズ含む）
void DiskD88Disk::SetOffset(int num, wxUint32 offset)
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return;
	header.offsets[num] = offset;
}

/// @param [in] offset 位置（ヘッダサイズを除く）
void DiskD88Disk::SetOffsetWithoutHeader(int num, wxUint32 offset)
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return;
	header.offsets[num] = (offset + sizeof(header));
}

/// offsetがNULLでない最大位置
int DiskD88Disk::GetOffsetMaxPos()
{
	return offset_maxpos;
}

/// offsetがNULLでない最大位置をセット
void DiskD88Disk::SetOffsetMaxPos(int pos)
{
	offset_maxpos = pos;
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

/// ディスクのトラックを作り直す
/// @param [in] selected_side >=0なら指定サイドのみ初期化
bool DiskD88Disk::Rebuild(const DiskParam &param, int selected_side)
{
	if (selected_side >= 0) {
		SetDiskParam(param.GetSidesPerDisk(), param.GetTracksPerSide(), param.GetSectorsPerTrack(), param.GetSectorSize(), param.GetDensity(), param.GetSingles()); 
	} else {
		SetDiskParam(param);
	}

	DiskD88Result result;
	wxString diskname;
	DiskD88Creator cr(diskname, param, false, NULL, result);
	bool rc = true;
	int trk = 0;
	int sid = 0;
	int sides = param.GetSidesPerDisk();
	for(int pos=0; pos<DISKD88_MAX_TRACKS; pos++) {
		if (selected_side >= 0) {
			sid = selected_side;
			// サイド指定ありの時はそのサイドのみ初期化
			if (selected_side != (pos % sides)) {
				continue;
			}
		}

		wxUint32 offset = GetOffset(pos);
		DiskD88Track *track = GetTrackByOffset(offset);
		if (tracks && track) {
			// トラック削除
			tracks->Remove(track);
			delete track;
		}
		if (!offset) {
			// オフセット計算
			offset = GetSize();
		}
		wxUint32 track_size = cr.CreateTrack(trk, sid, pos, offset, this);
		SetOffset(pos, offset);
		SetSize(offset + track_size);

		sid++;
		if (sid >= sides || selected_side >= 0) {
			trk++;
			sid = 0;
		}
		if (trk >= param.GetTracksPerSide()) {
			SetOffsetMaxPos(pos);
			break;
		}
	}
	return rc;
}

int DiskD88Disk::Compare(DiskD88Disk *item1, DiskD88Disk *item2)
{
    return (item1->num - item2->num);
}

bool DiskD88Disk::ExistTrack(int side_number)
{
	bool found = false;
	DiskD88Tracks *tracks = GetTracks();
	if (tracks) {
		for(size_t num=0; num < tracks->Count(); num++) {
			DiskD88Track *trk = tracks->Item(num);
			if (!trk) continue;
			if (side_number >= 0) {
				if (side_number != trk->GetSideNumber()) continue;
			}
			found = true;
			break;
		}
	}
	return found;
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
int DiskD88::SaveDisk(int disk_number, int side_number, const wxString &filepath)
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

	if (side_number < 0) {
		SaveDisk(disk, &fstream);
	} else {
		SaveDisk(disk, side_number, &fstream); 
	}

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
/// ディスクの1つサイドだけを保存(1S用)
int DiskD88::SaveDisk(DiskD88Disk *disk, int side_number, wxOutputStream *stream)
{
	if (!disk) {
		result.SetError(DiskD88Result::ERR_NO_DISK);
		return result.GetValid();
	}

	// 1S用のディスクを作成
	SingleDensities singles;
	singles.Add(new SingleDensity(-1, -1));
	DiskParam param("1S", 0, 1, 40, 16, 128, 0, singles);

	DiskD88File tmpfile;
	DiskD88Creator cr("", param, false, &tmpfile, result);
	int valid_disk = cr.Create();
	if (valid_disk != 0) {
		return valid_disk;
	}

	DiskD88Disk *tmpdisk = tmpfile.GetDisk(0);
	if (!tmpdisk) {
		result.SetError(DiskD88Result::ERR_NO_DATA);
		return result.GetValid();
	}

	// 新しいディスクにコピーする
	valid_disk = tmpdisk->Replace(0, disk, side_number);
	if (valid_disk != 0) {
		result.SetError(DiskD88Result::ERR_REPLACE);
	}

	return SaveDisk(tmpdisk, stream);
}
/// ディスクを削除
bool DiskD88::Delete(size_t disk_number)
{
	if (!file) return false;
	file->Delete(disk_number);
	file->SetModify();
	return true;
}
/// ファイルでディスクを置換
/// @return 0:正常 -1:エラーあり 1:警告あり
int DiskD88::ReplaceDisk(int disk_number, int side_number, const wxString &filepath)
{
	if (!file) return 0;

	result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskD88Result::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskD88File tmpfile;
	DiskD88Parser ps(&fstream, &tmpfile, result);
	int valid_disk = ps.Parse();

	// エラーあり
	if (valid_disk < 0) {
		return valid_disk;
	}

	// ディスクを選択 
	DiskD88Disk *disk = file->GetDisk(disk_number);
	if (!disk) {
		result.SetError(DiskD88Result::ERR_NO_DATA);
		return result.GetValid();
	}
	DiskD88Disk *src_disk = tmpfile.GetDisk(0);
	if (!src_disk) {
		result.SetError(DiskD88Result::ERR_NO_DATA);
		return result.GetValid();
	}
	if (side_number >= 0) {
		// AB面選択時、対象ディスクは1Sのみ
		if (src_disk->GetDiskTypeName() != "1S") {
			result.SetError(DiskD88Result::ERR_FILE_ONLY_1S);
			return result.GetValid();
		}
	} else {
		// 同じディスクタイプのみ
		if (src_disk->GetDiskTypeName() != disk->GetDiskTypeName()) {
			result.SetError(DiskD88Result::ERR_FILE_SAME);
			return result.GetValid();
		}
	}

	int src_side_number = 0;
	valid_disk = disk->Replace(side_number, src_disk, src_side_number);
	if (valid_disk != 0) {
		result.SetError(DiskD88Result::ERR_REPLACE);
	}

	file->SetModify();

	return valid_disk;
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
const wxString DiskD88::GetDiskName(size_t disk_number, bool real)
{
	DiskD88Disk *disk = GetDisk(disk_number);
	if (!disk) return wxEmptyString;
	return disk->GetName(real);
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

wxString DiskD88::GetFileNameBase() const
{
	return filename.GetName();
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
