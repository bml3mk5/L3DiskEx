/// @file diskd88.cpp
///
/// @brief D88ディスクイメージ入出力
///
#include "diskd88.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include "diskparser.h"
#include "diskwriter.h"
#include "diskd88creator.h"
#include "basicparam.h"

/// disk density 0: 2D, 1: 2DD, 2: 2HD
const char *gDiskDensity[] = {
	wxTRANSLATE("2D"),
	wxTRANSLATE("2DD"),
	wxTRANSLATE("2HD"),
	NULL
};

//
//
//
void IntHashMapUtil::IncleaseValue(IntHashMap &hash_map, int key)
{
	hash_map[key]++;
}
int IntHashMapUtil::GetMaxKeyOnMaxValue(IntHashMap &hash_map)
{
	IntHashMap::iterator it;
	int key_result = 0;
	int key2 = 0;
	int val = 0;
	for(it = hash_map.begin(); it != hash_map.end(); it++) {
		if (val < it->second) {
			val = it->second;
			key_result = it->first;
		} else if (val == it->second) {
			key2 = it->first;
			if (key_result < key2) {
				key_result = key2;
			}
		}
	}
	return key_result;
}
int IntHashMapUtil::MaxValue(int src, int value)
{
	return (src > value ? src : value);
}
int IntHashMapUtil::MinValue(int src, int value)
{
	return (src < value ? src : value);
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

/// ファイルから読み込み用
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
/// 新規作成用
DiskD88Sector::DiskD88Sector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density)
{
	num = sector_number;
	memset(&header, 0, sizeof(header));
	header.id.c = (wxUint8)track_number;
	header.id.h = (wxUint8)side_number;
	header.id.r = (wxUint8)sector_number;
//	if (single_density) sector_size = 128;
	this->SetSectorSize(sector_size);
	header.size = (wxUint16)sector_size;
	header.secnums = (wxUint16)number_of_sector;
	header.density = (single_density ? 0x40 : 0);

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

/// セクタのデータを埋める
bool DiskD88Sector::Fill(wxUint8 code)
{
	if (!data) return false;
	memset(data, code, header.size);
	SetModify();
	return true;
}

/// セクタのデータを上書き
bool DiskD88Sector::Copy(const wxUint8 *buf, size_t len)
{
	if (!data) return false;
	if (len > header.size) len = header.size;
	memcpy(data, buf, len);
	SetModify();
	return true;
}
/// セクタのデータを上書き
bool DiskD88Sector::Copy(const char *buf, size_t len)
{
	return Copy((const wxUint8 *)buf, len);
}

/// 指定位置のセクタデータを返す
wxUint8	DiskD88Sector::Get(int pos) const
{
	return data[pos];
}

/// セクタサイズを変更
void DiskD88Sector::ModifySectorSize(int size)
{
	if (size != header.size) {
		wxUint8 *newdata = new wxUint8[size];
		memset(newdata, 0, size);
		memcpy(newdata, data, size < header.size ? size : header.size);
		delete [] data;
		data = newdata;

		newdata = new wxUint8[size];
		memset(newdata, 0, size);
		memcpy(newdata, data_origin, size < header.size ? size : header.size);
		delete [] data_origin;
		data_origin = newdata;

		header.size = (wxUint16)size;
		SetSectorSize(size);

		SetModify();
	}
}

/// 変更済みを設定
void DiskD88Sector::SetModify()
{
	modified = (memcmp(&header_origin, &header, sizeof(header)) != 0);
	if (!modified && data && data_origin) {
		modified = (memcmp(data_origin, data, header_origin.size) != 0);
	}
}
/// 変更済みをクリア
void DiskD88Sector::ClearModify()
{
	memcpy(&header_origin, &header, sizeof(header_origin));
	if (data && data_origin) {
		memcpy(data_origin, data, header_origin.size);
	}
	modified = false;
}
/// 削除マークの設定
void DiskD88Sector::SetDeletedMark(bool val)
{
	deleted = val;
	header.deleted = (val ? 0x10 : 0);
}
/// 同じセクタか
bool DiskD88Sector::IsSameSector(int sector_number, bool deleted_mark)
{
	return (sector_number == num && deleted_mark == deleted);
}

/// セクタサイズ
int DiskD88Sector::GetSectorSize() const
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
/// セクタ番号の比較
int DiskD88Sector::CompareIDR(DiskD88Sector **item1, DiskD88Sector **item2)
{
    return ((int)(*item1)->GetIDR() - (int)(*item2)->GetIDR());
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
DiskD88Track::DiskD88Track(DiskD88Disk *disk)
{
	parent = disk;
	trk_num = 0;
	sid_num = 0;
//	offset  = 0;
	sectors = NULL;
	size    = 0;
	interleave = 1;
}

DiskD88Track::DiskD88Track(DiskD88Disk *disk, int newtrknum, int newsidnum, int newoffpos, int newinterleave)
{
	parent = disk;
	trk_num = newtrknum;
	sid_num = newsidnum;
	offset_pos = newoffpos;
	sectors = NULL;
	size    = 0;
	interleave = newinterleave;
}

#if 0
DiskD88Track::DiskD88Track(DiskD88Disk *disk, int newtrknum, int newsidnum, int newoffpos, wxUint32 newoffset, int newinterleave)
{
	parent = disk;
	trk_num = newtrknum;
	sid_num = newsidnum;
	offset_pos = newoffpos;
//	offset  = newoffset;
	sectors = NULL;
	size    = 0;
	interleave = newinterleave;
}

DiskD88Track::DiskD88Track(DiskD88Disk *disk, int newtrknum, int newsidnum, wxUint32 newoffset, DiskD88Sectors *newsecs)
{
	parent = disk;
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
	interleave = 1;
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

/// セクタを追加する
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

/// トラック内の指定セクタを削除する
/// @param [in] start_sector_num : 開始セクタ番号
/// @param [in] end_sector_num : 終了セクタ番号 -1なら全て
/// @return 0:正常 -1:エラー
int DiskD88Track::DeleteSectors(int start_sector_num, int end_sector_num)
{
	int rc = 0;
	if (!sectors) return -1;
	bool removed = false;
	for(size_t i=0; i<sectors->Count(); i++) {
		DiskD88Sector *sector = sectors->Item(i);
		int num = sector->GetSectorNumber();
		if (start_sector_num <= num && (num <= end_sector_num || end_sector_num < 0)) {
			delete sector;
			sectors->RemoveAt(i);

			removed = true;

			i--; // because sectors count is declement
		}
	}
	if (removed) {
		// トラックサイズを再計算
		Shrink();
		// トラックのオフセットを再計算する
		parent->Shrink();
	}

	return rc;
}

/// トラックサイズの再計算
wxUint32 DiskD88Track::Shrink()
{
	wxUint32 newsize = 0;

	if (!sectors) return newsize;

	size_t count = sectors->Count();

	for(size_t i=0; i<count; i++) {
		DiskD88Sector *sector = sectors->Item(i);
		sector->SetSectorsPerTrack((wxUint16)count);
		newsize += (wxUint32)sizeof(d88_sector_header_t);
		newsize += sector->GetSectorBufferSize();
	}
	SetSize(newsize);

	return newsize;
}

/// 指定セクタ番号のセクタを返す
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

/// トラック内のすべてのID Cを変更
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
	trk_num = val;
}

/// トラック内のすべてのID Hを変更
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
	sid_num = val;
}

/// トラック内のすべてのID Rを変更
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

/// トラック内のすべてのID Nを変更
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

/// トラック内のすべての密度を変更
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

/// トラック内のすべてのセクタ数を変更
void DiskD88Track::SetAllSectorsPerTrack(int val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetSectorsPerTrack(val);
			sector->SetModify();
		}
	}
}

/// トラック内のすべてのセクタサイズを変更
void DiskD88Track::SetAllSectorSize(int val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->ModifySectorSize(val);
		}
	}
	// トラックサイズを再計算
	Shrink();
	// トラックのオフセットを再計算する
	parent->Shrink();
}

/// トラック内の最小セクタ番号を返す
int DiskD88Track::GetMinSectorNumber() const
{
	int sector_number = 0x7fffffff;
	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskD88Sector *s = sectors->Item(pos);
			if (sector_number > s->GetSectorNumber()) {
				sector_number = s->GetSectorNumber();
			}
		}
	}
	return sector_number;
}

/// トラック内の最大セクタ番号を返す
int DiskD88Track::GetMaxSectorNumber() const
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

/// トラック内の最大セクタサイズを返す
int DiskD88Track::GetMaxSectorSize() const
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

/// 変更されているか
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

/// 変更済みをクリア
void DiskD88Track::ClearModify()
{
	if (!sectors) return;
	for(size_t sector_num = 0; sector_num < sectors->Count(); sector_num++) {
		DiskD88Sector *sector = sectors->Item(sector_num);
		if (!sector) continue;

		sector->ClearModify();
	}
}

#if 0
void DiskD88Track::SetFATArea(bool val)
{
	if (!sectors) return;
	for(size_t sector_num = 0; sector_num < sectors->Count(); sector_num++) {
		DiskD88Sector *sector = sectors->Item(sector_num);
		if (!sector) continue;

		sector->SetFATArea(val);
	}
}
#endif

/// トラック番号とサイド番号の比較
int DiskD88Track::Compare(DiskD88Track *item1, DiskD88Track *item2)
{
    return ((item1->trk_num - item2->trk_num) | (item1->sid_num - item2->sid_num));
}

//
//
//
DiskD88Disk::DiskD88Disk(DiskD88File *file) : DiskParam()
{
	this->parent = file;
	this->num = 0;
	memset(&this->header, 0, sizeof(this->header));
	this->tracks = NULL;
	this->offset_start = sizeof(header);

	memset(&this->header_origin, 0, sizeof(this->header_origin));

//	this->max_track_num = 0;
//	this->buffer = NULL;
//	this->buffer_size = 0;

	this->basic_param = NULL;

	this->modified = false;
}

DiskD88Disk::DiskD88Disk(DiskD88File *file, const wxString &newname, int newnum, const DiskParam &param, bool write_protect) : DiskParam(param)
{
	this->parent = file;
	this->num = newnum;
	memset(&this->header, 0, sizeof(this->header));
	this->tracks = NULL;
//	this->max_track_num = 0;
	this->offset_start = sizeof(header);

	memset(&this->header_origin, 0xff, sizeof(this->header_origin));

	this->SetName(newname);
	this->SetDensity(param.GetDensity());
	this->SetWriteProtect(write_protect);

	this->basic_param = NULL;

	this->modified = true;
}

DiskD88Disk::DiskD88Disk(DiskD88File *file, int newnum, const d88_header_t &newhdr) : DiskParam()
{
	this->parent = file;
	this->num = newnum;
	this->header = newhdr;
	this->tracks = NULL;
//	this->max_track_num = 0;
	this->offset_start = sizeof(header);

	this->header_origin = newhdr;

//	this->buffer = NULL;
//	this->buffer_size = 0;
	this->density = (newhdr.disk_density >> 4);

	this->basic_param = NULL;

	this->modified = false;
}

#if 0
DiskD88Disk::DiskD88Disk(wxUint8 *newbuf, size_t newbuflen, int newnum)
{
	num = newnum;
	write_protect = newwriteprotect;
	tracks = NULL;
//	max_track_num = 0;
	offset_start = sizeof(header);

	buffer = newbuf;
	buffer_size = newbuflen;

	header = (d88_header_t *)newbuf;

	basic_param = NULL;
}
#endif

#if 0
DiskD88Disk::DiskD88Disk(int newnum, d88_header_t *newhdr, DiskD88Tracks *newtrks) : DiskParam()
{
	num = newnum;
	write_protect = newwriteprotect;
	header = newhdr;
	offset_start = sizeof(header);
//	tracks = new DiskD88Tracks(DiskD88Track::Compare);
	tracks = new DiskD88Tracks;
//	max_track_num = 0;
	if (newtrks) {
		for(size_t i=0; i<newtrks->Count(); i++) {
			newtrks->Add(new DiskD88Track(*newtrks->Item(i)));
		}
	}

	this->CalcMajorNumber();

	basic_param = NULL;
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
//	SetMaxTrackNumber(newtrk->GetTrackNumber());
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
		if (!src_track) {
			continue;
		}
		int rct = track->Replace(src_track);
		if (rct != 0) rc = rct;
	}
	return rc;
}

/// トラックを削除する
/// @param [in] start_offset_pos : 削除開始トラック位置(0 ... 163)
/// @param [in] end_offset_pos : 削除終了トラック位置(0 ... 163)
/// @param [in] side_number : 特定のサイドのみ削除する場合 >= 0 , 全サイドの場合 = -1
void DiskD88Disk::DeleteTracks(int start_offset_pos, int end_offset_pos, int side_number)
{
//	size_t deleted_size = 0;

	if (!tracks) return;

	bool removed = false;
	for(size_t i=0; i<tracks->Count(); i++) {
		DiskD88Track *track = tracks->Item(i);
		if (!track) continue;
		int pos = track->GetOffsetPos();
		if (pos < start_offset_pos) continue;
		if (end_offset_pos >= start_offset_pos && pos > end_offset_pos) continue;
		if (side_number >= 0 && track->GetSideNumber() != side_number) continue;

//		wxUint32 track_size = track->GetSize();
		tracks->Remove(track);
		SetOffset(pos, 0);
		delete track;

		removed = true;

//		deleted_size += track_size;
//		SetSize(GetSize() - track_size);

		i--; // because tracks count is declement
	}

	if (removed) {
		// オフセットの再計算＆ディスクサイズ変更
		Shrink();
	}

//	return deleted_size;
}

/// オフセットの再計算＆ディスクサイズ変更
size_t DiskD88Disk::Shrink()
{
	size_t new_size = 0;

	if (!tracks) return new_size;

	// オフセットをクリア
	for(int pos = 0; pos < DISKD88_MAX_TRACKS; pos++) {
		SetOffset(pos, 0);
	}

	// 再計算
	wxUint32 max_offset = offset_start;
	for(size_t i=0; i<tracks->Count(); i++) {
		DiskD88Track *track = tracks->Item(i);
		if (!track) continue;
		int pos = track->GetOffsetPos();
		if (pos < 0 || pos >= DISKD88_MAX_TRACKS) continue;
		size_t size = track->GetSize();
		if (size == 0) continue;

//		wxUint32 offset = GetOffset(pos);
//		if (offset != max_offset) {
			SetOffset(pos, max_offset);
			SetModify();
//		}
		max_offset += size;
		new_size += size;
	}

	SetSize((wxUint32)new_size);

	return new_size;
}

/// ディスクサイズ計算（ディスクヘッダ分を除く）
size_t DiskD88Disk::CalcSizeWithoutHeader()
{
	size_t new_size = 0;

	if (!tracks) return new_size;

	for(size_t i=0; i<tracks->Count(); i++) {
		DiskD88Track *track = tracks->Item(i);
		if (!track) continue;
		new_size += track->GetSize();
	}
	return new_size;
}

/// 変更済みに設定
void DiskD88Disk::SetModify()
{
	modified = (memcmp(&header_origin, &header, sizeof(header)) != 0);
}

/// 変更済みをクリア
void DiskD88Disk::ClearModify()
{
	memcpy(&header_origin, &header, sizeof(header_origin));
	if (tracks) {
		for(size_t track_num = 0; track_num < tracks->Count(); track_num++) {
			DiskD88Track *track = tracks->Item(track_num);
			if (!track) continue;

			track->ClearModify();
		}
	}
	modified = false;
}

/// 変更されているか
bool DiskD88Disk::IsModified()
{
	bool modified_sector = false;
	if (!modified && tracks) {
		for(size_t track_num = 0; track_num < tracks->Count() && !modified_sector; track_num++) {
			DiskD88Track *track = tracks->Item(track_num);
			if (!track) continue;

			modified_sector = track->IsModified();
			if (modified_sector) break;
		}
	}
	return modified || modified_sector;
}

/// ディスク名を返す
wxString DiskD88Disk::GetName(bool real) const
{
	wxString name((const char *)header.diskname);
	if (!real && name.IsEmpty()) {
		name = _("(no name)");
	}
	return name;
}

/// ディスク名を設定
void DiskD88Disk::SetName(const wxString &val)
{
	wxString name = val;
	if (name == _("(no name)")) {
		name.Empty();
	}
//	wxCSConv cs(wxFONTENCODING_CP932);
//	wxCSConv cs2 = wxGet_wxConvLibc();
	strncpy((char *)header.diskname, name.mb_str(), 16);
	header.diskname[16] = 0;
}

/// ディスク名を設定
void DiskD88Disk::SetName(const wxUint8 *buf, size_t len)
{
	if (len > 16) len = 16;
	memcpy(header.diskname, buf, len);
	if (len < 16) len++;
	header.diskname[len] = 0;
}

/// 指定トラックを返す
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

/// 指定オフセット値からトラックを返す
DiskD88Track *DiskD88Disk::GetTrackByOffset(wxUint32 offset)
{
	DiskD88Track *track = NULL;
	if (tracks) {
		for(size_t i=0; i<tracks->Count(); i++) {
			DiskD88Track *t = tracks->Item(i);
			if (!t) continue;
			int pos = t->GetOffsetPos();
			if (GetOffset(pos) == offset) {
				track = t;
				break;
			}
		}
	}
	return track;
}

/// 指定セクタを返す
DiskD88Sector *DiskD88Disk::GetSector(int track_number, int side_number, int sector_number)
{
	DiskD88Track *trk = GetTrack(track_number, side_number);
	if (!trk) return NULL;
	return trk->GetSector(sector_number);
}

/// ディスクの中でもっともらしいパラメータを設定
const DiskParam *DiskD88Disk::CalcMajorNumber()
{
	IntHashMap sector_number_map[2];
	IntHashMap sector_size_map;
	IntHashMap interleave_map;
//	IntHashMap::iterator it;

	int track_number_max = 0;
	int side_number_max = 0;

	int sector_number_max_side0 = 0;
	int sector_number_min_side1 = 0x7fffffff;

	long sector_masize = 0;
	long interleave_max = 0;
	SingleDensities singles;

	if (tracks) {
		for(size_t ti=0; ti<tracks->Count(); ti++) {
			DiskD88Track *t = tracks->Item(ti);

			int trk_num = t->GetTrackNumber();
			int sid_num = t->GetSideNumber();

			track_number_max = IntHashMapUtil::MaxValue(track_number_max, trk_num);
			side_number_max = IntHashMapUtil::MaxValue(side_number_max, sid_num);
			IntHashMapUtil::IncleaseValue(sector_size_map, t->GetMaxSectorSize());	// セクタサイズはディスク内で最も使用されているサイズ
			IntHashMapUtil::IncleaseValue(interleave_map, t->GetInterleave());

			if (trk_num > 0 && sid_num < 2) {
				// トラック0は除く
				int sec_num_max = t->GetMaxSectorNumber();
				int sec_num_min = t->GetMinSectorNumber();
				IntHashMapUtil::IncleaseValue(sector_number_map[sid_num], sec_num_max);
				if (sid_num == 0) {
					sector_number_max_side0 = IntHashMapUtil::MaxValue(sector_number_max_side0, sec_num_max);
				} else {
					sector_number_min_side1 = IntHashMapUtil::MinValue(sector_number_min_side1, sec_num_min);
				}
			}

			DiskD88Sector *s = t->GetSector(1);
			if (s && s->IsSingleDensity()) {
				SingleDensity sd(t->GetTrackNumber(), t->GetSideNumber(), s->GetSectorsPerTrack(), s->GetSectorSize());
				singles.Add(sd);
			}
		}
	}
	sector_masize = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_size_map);
	interleave_max = IntHashMapUtil::GetMaxKeyOnMaxValue(interleave_map);

	// サイド番号のチェック
	if (side_number_max > 1) {
		side_number_max = 1;
	}

	// トラック番号のチェック
	if (tracks) {
		int track_count = ((int)tracks->Count() + side_number_max) / (side_number_max + 1);
		// 実際に存在するトラック数よりトラック番号がかなり大きい場合
		// 最大トラック番号をトラック数にする
		if (track_number_max > (track_count + 4)) {
			track_number_max = track_count - 1;
		}
	}

	bool disk_single_type = false;
	if (tracks) {
		if (sector_masize == 128 && side_number_max == 0 && max_track_number > track_number_max) {
			// 単密度で両面タイプ
			disk_single_type = true;
			side_number_max++;
			for(size_t ti=0; ti<tracks->Count(); ti++) {
				DiskD88Track *t =tracks->Item(ti);
				if (t->GetOffsetPos() & 1) {
					// 奇数の場合はサイドを1にする
					t->SetSideNumber(1);
				}
			}
		}
	}

	// 単密度かどうかをまとめる
	SingleDensity::Unique(track_number_max + 1, side_number_max + 1, disk_single_type, singles);

	sides_per_disk = (int)side_number_max + 1;
	tracks_per_side = (int)track_number_max + 1;
	sector_size = (int)sector_masize;
	interleave = (int)interleave_max;
	if (sides_per_disk > 1 && sector_number_min_side1 != 0x7fffffff && sector_number_max_side0 < sector_number_min_side1) {
		// セクタ番号がサイドを通して連番になっている
		numbering_sector = 1;
		long sec_num_maj = 0;
		sec_num_maj = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_number_map[0]);
		sectors_per_track = (int)sec_num_maj;
	} else {
		// セクタ番号はサイド毎
		numbering_sector = 0;
		long sec_num_maj[2];
		for(int i=0; i<2; i++) {
			sec_num_maj[i] = 0;
			sec_num_maj[i] = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_number_map[i]);
		}
		sectors_per_track = (int)(sec_num_maj[0] > sec_num_maj[1] ? sec_num_maj[0] : sec_num_maj[1]);
	}

	// メディアのタイプ
	DiskParam *disk_param = gDiskTypes.Find(sides_per_disk, tracks_per_side, sectors_per_track, sector_size, interleave, numbering_sector, singles);
	if (disk_param != NULL) {
		disk_type_name = disk_param->GetDiskTypeName();
		disk_type = disk_param->GetDiskType();
		basic_types = disk_param->GetBasicTypes();
		this->singles = singles;
	}
	return disk_param;
}

/// ディスクパラメータを文字列にフォーマットして返す
wxString DiskD88Disk::GetAttrText() const
{
	wxString str;

	str = disk_type_name;
	str += wxT("  ");
	str += wxString::Format(_("%dSide(s) %dTracks/Side %dSectors/Track %dbytes/Sector Interleave:%d"), sides_per_disk, tracks_per_side, sectors_per_track, sector_size, interleave);

	return str;
}

/// 書き込み禁止かどうかを返す
bool DiskD88Disk::IsWriteProtected() const
{
	return (header.write_protect != 0);
}

/// 書き込み禁止かどうかを設定
void DiskD88Disk::SetWriteProtect(bool val)
{
	header.write_protect = (val ? 0x10 : 0);
}

/// 密度を返す
wxString DiskD88Disk::GetDensityText() const
{
	wxUint8 num = ((header.disk_density) >> 4);
	if (num > 2) num = 3;
	return gDiskDensity[num];
}

/// 密度を設定
void DiskD88Disk::SetDensity(int val)
{
	if (0 <= val && val <= 2) {
		header.disk_density = (val << 4);
	}
}

/// ディスクサイズ（ヘッダサイズ含む）
wxUint32 DiskD88Disk::GetSize() const
{
	return header.disk_size;
}

/// ディスクサイズ（ヘッダサイズ含む）を設定
/// @param [in] val サイズ（ヘッダサイズ含む）
void DiskD88Disk::SetSize(wxUint32 val)
{
	header.disk_size = val;
}

/// ディスクサイズ（ヘッダサイズを除く）
wxUint32 DiskD88Disk::GetSizeWithoutHeader() const
{
	return header.disk_size - offset_start;
}

/// @param [in] val サイズ（ヘッダサイズを除く）を設定
void DiskD88Disk::SetSizeWithoutHeader(wxUint32 val)
{
	header.disk_size = (val + offset_start);
}

/// オフセット値を返す
/// @param [in] num    トラック番号
wxUint32 DiskD88Disk::GetOffset(int num) const
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return 0;
	return header.offsets[num];
}

/// オフセット値を設定
/// @param [in] num    トラック番号
/// @param [in] offset 位置（ヘッダサイズ含む）
void DiskD88Disk::SetOffset(int num, wxUint32 offset)
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return;
	header.offsets[num] = offset;
}

/// ヘッダサイズを除いたオフセット値を設定
/// @param [in] num    トラック番号
/// @param [in] offset 位置（ヘッダサイズを除く）
void DiskD88Disk::SetOffsetWithoutHeader(int num, wxUint32 offset)
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return;
	header.offsets[num] = (offset + offset_start);
}

/// offset最小値 -> トラックデータの開始位置を返す
wxUint32 DiskD88Disk::GetOffsetStart() const
{
	return offset_start;
}

/// offset最小値 -> トラックデータの開始位置を設定
void DiskD88Disk::SetOffsetStart(wxUint32 val)
{
	offset_start = val;
}

/// 最大トラック番号 offsetがNULLでない最大位置を返す
int DiskD88Disk::GetMaxTrackNumber() const
{
	return max_track_number;
}

/// 最大トラック番号 offsetがNULLでない最大位置を設定
void DiskD88Disk::SetMaxTrackNumber(int pos)
{
	max_track_number = pos;
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
/// @param [in] param         パラメータ
/// @param [in] selected_side >=0なら指定サイドのみ初期化
bool DiskD88Disk::Rebuild(const DiskParam &param, int selected_side)
{
	if (selected_side >= 0) {
		SetDiskParam(param.GetSidesPerDisk(), param.GetTracksPerSide(), param.GetSectorsPerTrack(), param.GetSectorSize(), param.GetDensity(), param.GetInterleave(), param.GetSingles()); 
	} else {
		SetDiskParam(param);
	}

	DiskResult result;
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
		if (offset == 0) {
			// オフセット計算
			offset = GetSize();
			if (offset < GetOffsetStart()) {
				// サイズがおかしいので初期サイズをセット
				offset = GetOffsetStart();
			}
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
			SetMaxTrackNumber(pos);
			break;
		}
	}
	return rc;
}

/// ディスク番号を比較
int DiskD88Disk::Compare(DiskD88Disk *item1, DiskD88Disk *item2)
{
    return (item1->num - item2->num);
}

/// トラックが存在するか
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

#if 0
void DiskD88Disk::SetFATArea(bool val)
{
	DiskD88Tracks *tracks = GetTracks();
	if (tracks) {
		for(size_t num=0; num < tracks->Count(); num++) {
			DiskD88Track *trk = tracks->Item(num);
			if (!trk) continue;
			trk->SetFATArea(val);
		}
	}
}
#endif

//void DiskD88Disk::SetMaxTrackNumber(int track_number)
//{
//	if (max_track_num < track_number) {
//		max_track_num = track_number;
//	}
//}

//int DiskD88Disk::GetMaxTrackNumber()
//{
//	return max_track_num;
//}

//
//
//
DiskD88File::DiskD88File()
{
	disks = NULL;
	mods  = NULL;
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

size_t DiskD88File::Add(DiskD88Disk *newdsk, short mod_flags)
{
//	if (!disks) disks = new DiskD88Disks(DiskD88Disk::Compare);
//	return disks->Add(new DiskD88Disk(newdsk));
	if (!disks) disks = new DiskD88Disks;
	if (!mods)  mods  = new wxArrayShort;
	disks->Add(newdsk);
	mods->Add(mod_flags);
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
	if (mods) {
		delete mods;
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
	mods->RemoveAt(idx);
	return true;
}

DiskD88Disk *DiskD88File::GetDisk(size_t idx)
{
	if (!disks) return NULL;
	if (idx >= disks->Count()) return NULL;
	return disks->Item(idx);
}

#if 0
void DiskD88File::SetModify()
{
	modified = true;
}
#endif

bool DiskD88File::IsModified()
{
	bool modified = false;
	if (disks) {
		for(size_t disk_num = 0; disk_num < disks->Count() && !modified; disk_num++) {
			modified = (mods->Item(disk_num) != 0);
			if (modified) break;

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
	if (disks) {
		for(size_t disk_num = 0; disk_num < disks->Count(); disk_num++) {
			mods->Item(disk_num) = MODIFY_NONE;

			DiskD88Disk *disk = disks->Item(disk_num);
			if (!disk) continue;

			disk->ClearModify();
		}
	}
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
/// @param [in] diskname      ディスク名
/// @param [in] param         ディスクパラメータ
/// @param [in] write_protect 書き込み禁止
/// @param [in] basic_hint    DISK BASIC種類のヒント
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::Create(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint)
{
	result.Clear();

	NewFile(wxEmptyString);
	file->SetBasicTypeHint(basic_hint);
	DiskD88Creator cr(diskname, param, write_protect, file, result);
	int valid_disk = cr.Create();

	// エラーあり
	if (valid_disk < 0) {
		ClearFile();
	}

	return valid_disk;
}

/// 追加で新規作成
/// @param [in] diskname      ディスク名
/// @param [in] param         ディスクパラメータ
/// @param [in] write_protect 書き込み禁止
/// @param [in] basic_hint    DISK BASIC種類のヒント
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::Add(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint)
{
	if (!file) return 0;

	result.Clear();

	file->SetBasicTypeHint(basic_hint);
	DiskD88Creator cr(diskname, param, write_protect, file, result);
	int valid_disk = cr.Add();

	return valid_disk;
}

/// ファイルを追加
/// @param [in] filepath    ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] disk_type   ディスクパラメータ種類
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::Add(const wxString &filepath, const wxString &file_format, const wxString &disk_type)
{
	if (!file) return 0;

	result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskParser ps(filepath, &fstream, file, result);
	int valid_disk = ps.ParseAdd(file_format, disk_type);

	return valid_disk;
}

/// ファイルを開く
/// @param [in] filepath    ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] disk_type   ディスクパラメータ種類
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::Open(const wxString &filepath, const wxString &file_format, const wxString &disk_type)
{
	result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	NewFile(filepath);
	DiskParser ps(filepath, &fstream, file, result);
	int valid_disk = ps.Parse(file_format, disk_type);

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
	DiskWriter dw(this, filepath, &result);
	return dw.Save();
}
/// ストリームの内容をファイルに保存
int DiskD88::SaveDisk(int disk_number, int side_number, const wxString &filepath)
{
	DiskWriter dw(this, filepath, &result);
	return dw.SaveDisk(disk_number, side_number);
}

/// ディスクを削除
bool DiskD88::Delete(size_t disk_number)
{
	if (!file) return false;
	file->Delete(disk_number);
//	file->SetModify();
	return true;
}
/// ファイルでディスクを置換
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] filepath    ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] disk_type   ディスクパラメータ種類
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::ReplaceDisk(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const wxString &disk_type)
{
	if (!file) return 0;

	result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskD88File tmpfile;
	DiskParser ps(filepath, &fstream, &tmpfile, result);
	int valid_disk = ps.Parse(file_format, disk_type);

	// エラーあり
	if (valid_disk < 0) {
		return valid_disk;
	}

	// ディスクを選択 
	DiskD88Disk *disk = file->GetDisk(disk_number);
	if (!disk) {
		result.SetError(DiskResult::ERR_NO_DATA);
		return result.GetValid();
	}
	DiskD88Disk *src_disk = tmpfile.GetDisk(0);
	if (!src_disk) {
		result.SetError(DiskResult::ERR_NO_DATA);
		return result.GetValid();
	}
	if (side_number >= 0) {
		// AB面選択時、対象ディスクは1Sのみ
		if (src_disk->GetDiskTypeName() != "1S") {
			result.SetError(DiskResult::ERR_FILE_ONLY_1S);
			return result.GetValid();
		}
	} else {
		// 同じディスクタイプのみ
		if (src_disk->GetDiskTypeName() != disk->GetDiskTypeName()) {
			result.SetError(DiskResult::ERR_FILE_SAME);
			return result.GetValid();
		}
	}

	int src_side_number = 0;
	valid_disk = disk->Replace(side_number, src_disk, src_side_number);
	if (valid_disk != 0) {
		result.SetError(DiskResult::ERR_REPLACE);
	}

//	file->SetModify();

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
/// ディスク名を返す
wxString DiskD88::GetDiskName(size_t disk_number, bool real) const
{
	const DiskD88Disk *disk = GetDisk(disk_number);
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
/// 指定した位置のディスクを返す
const DiskD88Disk *DiskD88::GetDisk(size_t index) const
{
	if (!file) return NULL;
	return file->GetDisk(index);
}
/// 指定した位置のディスクのタイプ
int DiskD88::GetDiskTypeNumber(size_t index) const
{
	if (!file) return -1;
	DiskD88Disk *disk = file->GetDisk(index);
	if (!disk) return -1;
	return gDiskTypes.IndexOf(disk->GetDiskTypeName());
}

/// ファイル名を返す
wxString DiskD88::GetFileName() const
{
	return filename.GetFullName();
}

/// ファイル名ベースを返す
wxString DiskD88::GetFileNameBase() const
{
	return filename.GetName();
}

/// ファイルパスを返す
wxString DiskD88::GetFilePath() const
{
	return filename.GetFullPath();
}

/// パスを返す
wxString DiskD88::GetPath() const
{
	return filename.GetPath();
}

/// ファイル名を設定
void DiskD88::SetFileName(const wxString &path)
{
	filename = wxFileName(path);
}

/// エラーメッセージ
const wxArrayString &DiskD88::GetErrorMessage(int maxrow)
{
	return result.GetMessages(maxrow);
}

/// エラーメッセージを表示
void  DiskD88::ShowErrorMessage()
{
	ResultInfo::ShowMessage(result.GetValid(), result.GetMessages());
}
