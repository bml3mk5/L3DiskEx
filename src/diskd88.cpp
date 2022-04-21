/// @file diskd88.cpp
///
/// @brief D88ディスクイメージ入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskd88.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include "diskparser.h"
#include "diskwriter.h"
#include "diskd88creator.h"
#include "basicparam.h"
#include "basicfmt.h"


/// disk density 0: 2D, 1: 2DD, 2: 2HD, 3: 1DD(unofficial)
const struct st_disk_density gDiskDensity[] = {
	{ 0x00, "2D" },
	{ 0x10, "2DD" },
	{ 0x20, "2HD" },
	{ 0x30, "0x30 1DD" },
	{ 0xff, NULL }
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
	header = NULL;
	data = NULL;

//	modified = false;
	memset(&header_origin, 0, sizeof(header_origin));
	data_origin = NULL;
}

/// ファイルから読み込み用
DiskD88Sector::DiskD88Sector(int n_num, d88_sector_header_t *n_header, wxUint8 *n_data)
{
	num = n_num;
	header = n_header;
	data = n_data;

//	modified = false;
	header_origin = *n_header;
	data_origin = new wxUint8[header->size];
	memcpy(data_origin, data, header->size);
}

/// 新規作成用
DiskD88Sector::DiskD88Sector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density)
{
	num = sector_number;
	header = new d88_sector_header_t;
	memset(header, 0, sizeof(d88_sector_header_t));
	header->id.c = (wxUint8)track_number;
	header->id.h = (wxUint8)side_number;
	header->id.r = (wxUint8)sector_number;
//	if (single_density) sector_size = 128;
	this->SetSectorSize(sector_size);
	header->size = (wxUint16)sector_size;
	header->secnums = (wxUint16)number_of_sector;
	header->density = (single_density ? 0x40 : 0);

	data = new wxUint8[header->size];
	memset(data, 0, header->size);
	memset(&header_origin, 0xff, sizeof(header_origin));

	data_origin = new wxUint8[header->size];
	memset(data_origin, 0, header->size);

//	modified = true;
}

DiskD88Sector::~DiskD88Sector()
{
	delete [] data_origin;
	delete [] data;
	delete header;
}

/// セクタのデータを置き換える
/// セクタサイズは変更しない
bool DiskD88Sector::Replace(DiskD88Sector *src_sector)
{
	if (!header || !data) {
		return false;
	}
	wxUint8 *src_data = src_sector->data;
	if (!src_data) {
		// データなし
		return false;
	}
	size_t sz = src_sector->GetSectorBufferSize() > GetSectorBufferSize() ? GetSectorBufferSize() : src_sector->GetSectorBufferSize();
	if (sz > 0) {
		memset(data, 0, GetSectorBufferSize());
		memcpy(data, src_data, sz);
//		SetModify();
	}
	return true;
}

/// セクタのデータを埋める
bool DiskD88Sector::Fill(wxUint8 code, size_t len, size_t start)
{
	if (!header || !data) {
		return false;
	}
	if (start >= header->size) {
		return false;
	}

	if ((start + len) > header->size) len = (size_t)header->size - start;
	memset(&data[start], code, len);
//	SetModify();
	return true;
}

/// セクタのデータを上書き
bool DiskD88Sector::Copy(const void *buf, size_t len, size_t start)
{
	if (!header || !data) {
		return false;
	}
	if (start >= header->size) {
		return false;
	}

	if ((start + len) > header->size) len = (size_t)header->size - start;
	memcpy(&data[start], buf, len);
//	SetModify();
	return true;
}

/// セクタのデータに指定したバイト列があるか
/// @return -1:なし >=0:あり
int DiskD88Sector::Find(const void *buf, size_t len)
{
	if (!header || !data) {
		return -1;
	}
	int match = -1;
	for(int pos = 0; pos < (GetSectorBufferSize() - (int)len); pos++) { 
		if (memcmp(&data[pos], buf, len) == 0) {
			match = pos;
			break;
		}
	}
	return match;
}

/// 指定位置のセクタデータを返す
wxUint8	DiskD88Sector::Get(int pos) const
{
	if (!header || !data) {
		return 0;
	}
	return data[pos];
}

/// 指定位置のセクタデータを返す
wxUint16 DiskD88Sector::Get16(int pos, bool big_endian) const
{
	if (!header || !data) {
		return 0;
	}
	return big_endian ? ((wxUint16)data[pos] << 8 | data[pos+1]) : ((wxUint16)data[pos+1] << 8 | data[pos]);
}

/// セクタサイズを変更
/// @return 変更後のサイズ差分
int DiskD88Sector::ModifySectorSize(int size)
{
	int diff = 0;
	if (!header || !data) {
		return diff;
	}
	if (size != header->size) {
		diff = (int)header->size - size;

		wxUint8 *newdata = new wxUint8[size];
		memset(newdata, 0, size);
		memcpy(newdata, data, size < header->size ? size : header->size);
		delete [] data;
		data = newdata;

		newdata = new wxUint8[size];
		memset(newdata, 0, size);
		memcpy(newdata, data_origin, size < header->size ? size : header->size);
		delete [] data_origin;
		data_origin = newdata;

		header->size = (wxUint16)size;
		SetSectorSize(size);

//		SetModify();
	}
	return diff;
}

/// 変更済みを設定
void DiskD88Sector::SetModify()
{
//	modified = (memcmp(&header_origin, &header, sizeof(header)) != 0);
//	if (!modified && data && data_origin) {
//		modified = (memcmp(data_origin, data, header_origin.size) != 0);
//	}
}

/// 変更されているか
bool DiskD88Sector::IsModifiedBase() const
{
	if (!header || !data) {
		return false;
	}
	bool mod = (memcmp(&header_origin, header, sizeof(d88_sector_header_t)) != 0);
	if (!mod && data && data_origin) {
		mod = (memcmp(data_origin, data, header_origin.size) != 0);
	}
	return mod;
}

/// 変更されているか
bool DiskD88Sector::IsModified() const
{
//	if (modified) return true;
	return IsModifiedBase();
}

/// 変更済みをクリア
void DiskD88Sector::ClearModify()
{
	if (!header || !data) {
		return;
	}
	memcpy(&header_origin, header, sizeof(d88_sector_header_t));
	if (data && data_origin) {
		memcpy(data_origin, data, header_origin.size);
	}
//	modified = false;
}
/// セクタ番号を設定
void DiskD88Sector::SetSectorNumber(int val)
{
	num = val;
	if (header) {
		header->id.r = (wxUint8)val;
	}
}
/// 削除マークがついているか
bool DiskD88Sector::IsDeleted() const
{
	return (header && header->deleted != 0);
}
/// 削除マークの設定
void DiskD88Sector::SetDeletedMark(bool val)
{
	if (header) {
		header->deleted = (val ? 0x10 : 0);
	}
}
/// 同じセクタか
bool DiskD88Sector::IsSameSector(int sector_number, bool deleted_mark)
{
	return (sector_number == num && deleted_mark == IsDeleted());
}

/// セクタサイズ
int DiskD88Sector::GetSectorSize() const
{
	if (!header) {
		return 0;
	}
	int sec = ConvIDNToSecSize(header->id.n);
	if (sec > GetSectorBufferSize()) sec = GetSectorBufferSize(); 
	return sec;
}
void DiskD88Sector::SetSectorSize(int val)
{
	if (header) {
		header->id.n = ConvSecSizeToIDN(val);
	}
}

/// セクタサイズ（バッファのサイズ）を返す
int DiskD88Sector::GetSectorBufferSize() const
{
	return (header ? header->size : 0);
}

/// セクタサイズ（ヘッダ＋バッファのサイズ）を返す
int DiskD88Sector::GetSize() const
{
	return (int)sizeof(d88_sector_header_t) + GetSectorBufferSize();
}

/// セクタ数を返す
wxUint16 DiskD88Sector::GetSectorsPerTrack() const
{
	return (header ? header->secnums : 0);
}
/// セクタ数を設定
void DiskD88Sector::SetSectorsPerTrack(wxUint16 val)
{
	if (header) {
		header->secnums = val;
	}
}

/// ID Cを返す
wxUint8	DiskD88Sector::GetIDC() const
{
	return (header ? header->id.c : 0);
}
/// ID Hを返す
wxUint8	DiskD88Sector::GetIDH() const
{
	return (header ? header->id.h : 0);
}
/// ID Rを返す
wxUint8	DiskD88Sector::GetIDR() const
{
	return (header ? header->id.r : 0);
}
/// ID Nを返す
wxUint8	DiskD88Sector::GetIDN() const
{
	return (header ? header->id.n : 0);
}

/// ID Cを設定
void DiskD88Sector::SetIDC(wxUint8 val)
{
	if (header) {
		header->id.c = val;
	}
}
/// ID Hを設定
void DiskD88Sector::SetIDH(wxUint8 val)
{
	if (header) {
		header->id.h = val;
	}
}
/// ID Rを設定
void DiskD88Sector::SetIDR(wxUint8 val)
{
	if (header) {
		header->id.r = val;
	}
}
/// ID Nを設定
void DiskD88Sector::SetIDN(wxUint8 val)
{
	if (header) {
		header->id.n = val;
	}
}

/// 単密度か
bool DiskD88Sector::IsSingleDensity()
{
	return (header && header->density == 0x40);
}
void DiskD88Sector::SetSingleDensity(bool val)
{
	if (header) {
		header->density = (val ? 0x40 : 0);
	}
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

	orig_sectors = 0;

	extra_data = NULL;
	extra_size = 0;
}

/// @param [in] disk            ディスク
/// @param [in] newtrknum       トラック番号
/// @param [in] newsidnum       サイド番号
/// @param [in] newoffpos       オフセットインデックス
/// @param [in] newinterleave   インターリーブ
DiskD88Track::DiskD88Track(DiskD88Disk *disk, int newtrknum, int newsidnum, int newoffpos, int newinterleave)
{
	parent = disk;
	trk_num = newtrknum;
	sid_num = newsidnum;
	offset_pos = newoffpos;
	sectors = NULL;
	size    = 0;
	interleave = newinterleave;

	orig_sectors = 0;

	extra_data = NULL;
	extra_size = 0;
}

DiskD88Track::~DiskD88Track()
{
	if (sectors) {
		for(size_t i=0; i<sectors->Count(); i++) {
			DiskD88Sector *p = sectors->Item(i);
			delete p;
		}
		delete sectors;
	}
	delete [] extra_data;
}

/// セクタを追加する
/// @return セクタ数
size_t DiskD88Track::Add(DiskD88Sector *newsec)
{
	if (!sectors) sectors = new DiskD88Sectors;
	sectors->Add(newsec);
	orig_sectors = sectors->Count();
	return orig_sectors;
}

/// トラック内のセクタデータを置き換える
/// @param [in] src_track
/// @return 0:正常 -1:エラー 1:置換できないセクタあり
int DiskD88Track::Replace(DiskD88Track *src_track)
{
	int rc = 0;
	if (!sectors) return -1;
	for(size_t i=0; i<sectors->Count(); i++) {
		DiskD88Sector *tag_sector = sectors->Item(i);
		DiskD88Sector *src_sector = src_track->GetSector(tag_sector->GetSectorNumber());
		if (!src_sector) {
			// セクタなし
			continue;
		}
		if (!tag_sector->Replace(src_sector)) {
			rc = 1;
		}
	}
	return rc;
}

/// トラックに新規セクタを追加する
/// @param [in] trknum   新規セクタのトラック番号(ID C)
/// @param [in] sidnum   新規セクタのサイド番号(ID H)
/// @param [in] secnum   新規セクタのセクタ番号(ID R)
/// @param [in] secsize  新規セクタのセクタサイズ(128,256,512,1024,2048)
/// @param [in] sdensity 新規セクタが単密度か
/// @return 0 正常
int DiskD88Track::AddNewSector(int trknum, int sidnum, int secnum, int secsize, bool sdensity)
{
	int rc = 0;

	// 新規セクタ
	DiskD88Sector *new_sector = new DiskD88Sector(
		trknum, sidnum, secnum, secsize, 1, sdensity
	);
	// 追加
	Add(new_sector);
	// 余りバッファ領域のサイズを減らす
	DecreaseExtraDataSize(new_sector->GetSize());
	// トラックのサイズを再計算&オフセットを再計算する
	ShrinkAndCalcOffsets(false);

	return rc;
}

/// トラック内の指定位置のセクタを削除する
/// @param [in] pos : セクタ位置
int DiskD88Track::DeleteSectorByIndex(int pos)
{
	int rc = 0;
	if (!sectors || pos < 0 || pos >= (int)sectors->Count()) return -1;

	int removed_size = 0;
	DiskD88Sector *sector = sectors->Item(pos);
	removed_size += sector->GetSize();
	delete sector;
	sectors->RemoveAt(pos);

	// 余りバッファ領域のサイズを増やす
	IncreaseExtraDataSize(removed_size);
	// トラックのサイズを再計算&オフセットを再計算する
	ShrinkAndCalcOffsets(false);

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
	int  removed_size = 0;
	for(size_t i=0; i<sectors->Count(); i++) {
		DiskD88Sector *sector = sectors->Item(i);
		int num = sector->GetSectorNumber();
		if (start_sector_num <= num && (num <= end_sector_num || end_sector_num < 0)) {
			removed_size += sector->GetSize();
			delete sector;
			sectors->RemoveAt(i);

			removed = true;

			i--; // because sectors count is declement
		}
	}
	if (removed) {
		// 余りバッファ領域のサイズを増やす
		IncreaseExtraDataSize(removed_size);
		// トラックのサイズを再計算&オフセットを再計算する
		ShrinkAndCalcOffsets(false);
	}

	return rc;
}

/// トラックサイズの再計算
wxUint32 DiskD88Track::Shrink(bool trim_unused_data)
{
	wxUint32 newsize = 0;

	size_t count = sectors ? sectors->Count() : 0;

	for(size_t i=0; i<count; i++) {
		DiskD88Sector *sector = sectors->Item(i);
		sector->SetSectorsPerTrack((wxUint16)count);
		newsize += (wxUint32)sizeof(d88_sector_header_t);
		if (trim_unused_data) {
			newsize += sector->GetSectorSize();
		} else {
			newsize += sector->GetSectorBufferSize();
		}
	}
	if (!trim_unused_data) {
		newsize += (wxUint32)extra_size;
	}
	SetSize(newsize);

	return newsize;
}

/// トラックサイズの再計算&オフセット計算
void DiskD88Track::ShrinkAndCalcOffsets(bool trim_unused_data)
{
	Shrink(trim_unused_data);
	parent->CalcOffsets();
}

/// 余りバッファ領域のサイズを増やす
void DiskD88Track::IncreaseExtraDataSize(size_t size)
{
	if (size == 0) return;

	wxUint8 *new_data = new wxUint8[extra_size + size];
	memset(new_data, 0, size);
	memcpy(&new_data[size], extra_data, extra_size);
	delete [] extra_data;
	extra_data = new_data;
	extra_size += size;
}

/// 余りバッファ領域のサイズを減らす
void DiskD88Track::DecreaseExtraDataSize(size_t size)
{
	if (size == 0) return;

	size_t remain_size = (extra_size > size ? extra_size - size : 0);

	wxUint8 *new_data = NULL;
	if (remain_size > 0) {
		new_data = new wxUint8[remain_size];
		memcpy(new_data, &extra_data[size], remain_size);
	}
	delete [] extra_data;
	extra_data = new_data;
	extra_size = remain_size;
}

/// インターリーブを計算して設定
void DiskD88Track::CalcInterleave()
{
	if (!sectors) return;

	size_t count = sectors->Count();
	if (count == 1) {
		SetInterleave(1);
		return;
	}

	int start = sectors->Item(0)->GetSectorNumber();
	int next = start + 1;
	int state = 0;
	int intl = 0;
	for(size_t sec_pos = 0; sec_pos < count; sec_pos++) {
		DiskD88Sector *s = sectors->Item(sec_pos);
		switch(state) {
		case 1:
			intl++;
			if (s->GetSectorNumber() == next) {
				state = 2;
				sec_pos = count;
			}
			break;
		default:
			if (s->GetSectorNumber() == start) {
				state = 1;
				intl = 0;
			}
			break;
		}
	}
	if (intl <= 0) {
		intl = 1;
	}
	SetInterleave(intl);
}

/// セクタ数を返す
int DiskD88Track::GetSectorsPerTrack() const
{
	int cnt = 0;
	if (sectors) {
		cnt = (int)sectors->Count();
	}
	return cnt;
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

/// 指定位置のセクタを返す
DiskD88Sector *DiskD88Track::GetSectorByIndex(int pos)
{
	DiskD88Sector *sector = NULL;
	if (sectors && pos >= 0 && pos < (int)sectors->Count()) {
		sector = sectors->Item(pos);
	}
	return sector;
}

/// トラック内のもっともらしいID Hを返す
wxUint8	DiskD88Track::GetMajorIDH() const
{
	wxUint8 id = 0;
	IntHashMap map;

	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskD88Sector *s = sectors->Item(pos);
			IntHashMapUtil::IncleaseValue(map, s->GetIDH());
		}
		id = IntHashMapUtil::GetMaxKeyOnMaxValue(map);
	}
	return id;
}

/// トラック内のすべてのID Cを変更
void DiskD88Track::SetAllIDC(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDC(val);
//			sector->SetModify();
		}
	}
}

/// トラック内のすべてのID Hを変更
void DiskD88Track::SetAllIDH(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDH(val);
//			sector->SetModify();
		}
	}
}

/// トラック内のすべてのID Rを変更
void DiskD88Track::SetAllIDR(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDR(val);
//			sector->SetModify();
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
//			sector->SetModify();
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
//			sector->SetModify();
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
//			sector->SetModify();
		}
	}
}

/// トラック内のすべてのセクタサイズを変更
void DiskD88Track::SetAllSectorSize(int val)
{
	if (!sectors) return;

	int sum = 0;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		if (sector) {
			sum += sector->ModifySectorSize(val);
		}
	}
	// 余りバッファの増減
	if (sum > 0) {
		IncreaseExtraDataSize(sum);
	} else if (sum < 0) {
		DecreaseExtraDataSize(sum);
	}
	// トラックのサイズを再計算&オフセットを再計算する
	ShrinkAndCalcOffsets(false);
}

/// 余分なデータを設定する
void DiskD88Track::SetExtraData(wxUint8 *buf, size_t size)
{
	delete extra_data;
	extra_data = buf;
	extra_size = size;
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
	if (orig_sectors != sectors->Count()) return true;

	bool modified = false;
	for(size_t sector_num = 0; sector_num < sectors->Count() && !modified; sector_num++) {
		DiskD88Sector *sector = sectors->Item(sector_num);
		if (!sector) continue;

		modified = sector->IsModified();
		if (modified) {
			break;
		}
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
	orig_sectors = sectors->Count();
}

/// トラック番号とサイド番号の比較
int DiskD88Track::Compare(DiskD88Track *item1, DiskD88Track *item2)
{
    return ((item1->trk_num - item2->trk_num) | (item1->sid_num - item2->sid_num));
}

/// インターリーブを考慮したセクタ番号リストを返す
bool DiskD88Track::CalcSectorNumbersForInterleave(int interleave, size_t sectors_count, wxArrayInt &sector_nums, int sector_offset)
{
	sector_nums.SetCount(sectors_count, 0);
	int sector_pos = 0;
	bool err = false;
	for(int sector_number = 0; sector_number < (int)sectors_count && err == false; sector_number++) {
		if (sector_pos >= (int)sectors_count) {
			sector_pos -= sectors_count;
			while (sector_nums[sector_pos] > 0) {
				sector_pos++;
				if (sector_pos >= (int)sectors_count) {
					// ?? error
					err = true;
					break;
				}
			}
		}
		sector_nums[sector_pos] = (sector_number + sector_offset + 1);
		sector_pos += interleave;
	}

	return !err;
}

//
//
//
DiskD88Disk::DiskD88Disk(DiskD88File *file, int n_num) : DiskParam()
{
	this->parent = file;
	this->num = n_num;
	this->header = new d88_header_t;
	memset(this->header, 0, sizeof(d88_header_t));
	this->tracks = NULL;
	this->offset_start = sizeof(d88_header_t);

	memset(&this->header_origin, 0, sizeof(this->header_origin));

//	this->max_track_num = 0;
//	this->buffer = NULL;
//	this->buffer_size = 0;
	this->param_changed = false;

//	this->basic_param = NULL;
	this->basics = new DiskBasics;

	this->modified = false;
}

DiskD88Disk::DiskD88Disk(DiskD88File *file, const wxString &newname, int newnum, const DiskParam &param, bool write_protect) : DiskParam(param)
{
	this->parent = file;
	this->num = newnum;
	this->header = new d88_header_t;
	memset(this->header, 0, sizeof(d88_header_t));
	this->tracks = NULL;
	this->offset_start = sizeof(d88_header_t);

	memset(&this->header_origin, 0xff, sizeof(this->header_origin));

	this->SetName(newname);
	this->SetDensity(param.GetParamDensity());
	this->SetWriteProtect(write_protect);

	this->param_changed = false;

//	this->basic_param = NULL;
	this->basics = new DiskBasics;

	this->modified = true;
}

/// @note n_header はnewで確保しておくこと
DiskD88Disk::DiskD88Disk(DiskD88File *file, int n_num, d88_header_t *n_header) : DiskParam()
{
	this->parent = file;
	this->num = n_num;
	this->header = n_header;
	this->tracks = NULL;
	this->offset_start = sizeof(d88_header_t);

	this->header_origin = *n_header;

	this->disk_density = n_header->disk_density;

	this->param_changed = false;

//	this->basic_param = NULL;
	this->basics = new DiskBasics;

	this->modified = false;
}

DiskD88Disk::~DiskD88Disk()
{
	if (tracks) {
		for(size_t i=0; i<tracks->Count(); i++) {
			DiskD88Track *p = tracks->Item(i);
			delete p;
		}
		delete tracks;
	}
	delete basics;
	delete header;
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
		DiskD88Track *tag_track = tracks->Item(i);
		int tag_side_number = tag_track->GetSideNumber();
		if (side_number >= 0) {
			// 指定したサイドだけ置き換える
			if (tag_side_number != side_number) {
				continue;
			}
			// 片面ディスクの場合
			if (src_side_number >= 0) {
				tag_side_number = src_side_number;
			}
			if (src_disk->GetSidesPerDisk() <= tag_side_number) {
				tag_side_number = src_disk->GetSidesPerDisk() - 1;
			}
		}
		DiskD88Track *src_track = src_disk->GetTrack(tag_track->GetTrackNumber(), tag_side_number);
		if (!src_track) {
			continue;
		}
		int rct = tag_track->Replace(src_track);
		if (rct != 0) rc = rct;
	}
	return rc;
}

/// ディスクにトラックを追加
/// @param [in] side_number サイド番号 両面なら-1
int DiskD88Disk::AddNewTrack(int side_number)
{
	int rc = 0;

	// 最大トラック＆サイド番号
	DiskD88Track *src_track = NULL;
	int trk_num = -1;
	int sid_num = -1;
	int max_sid_num = -1;
	for(size_t pos = 0; pos < tracks->Count(); pos++) {
		DiskD88Track *track = tracks->Item(pos);
		if (track->GetTrackNumber() > trk_num && (side_number < 0 || side_number == track->GetSideNumber())) {
			trk_num = track->GetTrackNumber();
			sid_num = track->GetSideNumber();
			if (sid_num > max_sid_num) {
				max_sid_num = sid_num;
			}
			src_track = track;
		} else if (side_number < 0 && track->GetSideNumber() > sid_num) {
			sid_num = track->GetSideNumber();
			if (sid_num > max_sid_num) {
				max_sid_num = sid_num;
			}
			src_track = track;
		}
	}
	if (!src_track) {
		return -1;
	}
	DiskD88Sectors *sectors = src_track->GetSectors();
	if (!sectors) {
		return -1;
	}

	// 新規トラック番号
	if (side_number < 0 && sid_num < max_sid_num) {
		sid_num++;
	} else {
		trk_num++;
		sid_num = (side_number < 0 ? 0 : side_number);
	}

	// 空きオフセットがあるか
	int offset_pos = 0;
	for(int pos = (side_number < 0 ? 0 : side_number); pos < DISKD88_MAX_TRACKS; pos += (side_number < 0 ? 1 : max_sid_num + 1)) {
		if (GetOffset(pos) == 0) {
			offset_pos = pos;
			break;
		}
	}
	if (offset_pos == 0) {
		// 空きなし
		return -1;
	}

	DiskD88Track *new_track = new DiskD88Track(
		this,
		trk_num,
		sid_num,
		offset_pos,
		src_track->GetInterleave()
	);
	// セクタを追加
	wxUint32 trk_size = 0;
	for(size_t pos = 0; pos < sectors->Count(); pos++) {
		DiskD88Sector *sector = sectors->Item(pos);
		// 新規セクタ
		int sec_num = sector->GetIDR();
		int sec_size = sector->GetSectorSize();
		bool sdensity = sector->IsSingleDensity();

		DiskD88Sector *new_sector = new DiskD88Sector(
			trk_num, sid_num, sec_num, sec_size, (int)sectors->Count(), sdensity
		);
		// 追加
		new_track->Add(new_sector);
		trk_size += (wxUint32)new_sector->GetSize();
	}
	new_track->IncreaseExtraDataSize(src_track->GetExtraDataSize());
	trk_size += (wxUint32)new_track->GetExtraDataSize();
	new_track->SetSize(trk_size);
	Add(new_track);

	// オフセット再計算
	CalcOffsets();

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
		CalcOffsets();
	}

//	return deleted_size;
}

/// トラックサイズ＆オフセットの再計算＆ディスクサイズ変更
size_t DiskD88Disk::ShrinkTracks(bool trim_unused_data)
{
	if (tracks) {
		for(size_t i=0; i<tracks->Count(); i++) {
			DiskD88Track *track = tracks->Item(i);
			if (!track) continue;
			track->Shrink(trim_unused_data);
		}
	}
	return CalcOffsets();
}

/// オフセットの再計算＆ディスクサイズ変更
size_t DiskD88Disk::CalcOffsets()
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

		if (size > 0) {
			SetOffset(pos, max_offset);
		} else {
			SetOffset(pos, 0);
		}
		SetModify();
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
	modified = (memcmp(&header_origin, header, sizeof(d88_header_t)) != 0);
}

/// 変更済みをクリア
void DiskD88Disk::ClearModify()
{
	memcpy(&header_origin, header, sizeof(header_origin));
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
			if (modified_sector) {
				break;
			}
		}
	}
	return modified || modified_sector;
}

/// ディスク名を返す
wxString DiskD88Disk::GetName(bool real) const
{
	wxString name(header ? (const char *)header->diskname : "");
	if (!real && name.IsEmpty()) {
		name = _("(no name)");
	}
	return name;
}

/// ディスク名を設定
void DiskD88Disk::SetName(const wxString &val)
{
	if (!header) return;

	wxString name = val;
	if (name == _("(no name)")) {
		name.Empty();
	}
//	wxCSConv cs(wxFONTENCODING_CP932);
//	wxCSConv cs2 = wxGet_wxConvLibc();
	strncpy((char *)header->diskname, name.mb_str(), 16);
	header->diskname[16] = 0;
}

/// ディスク名を設定
void DiskD88Disk::SetName(const wxUint8 *buf, size_t len)
{
	if (!header) return;

	if (len > 16) len = 16;
	memcpy(header->diskname, buf, len);
	if (len < 16) len++;
	header->diskname[len] = 0;
}

/// 指定トラックを返す
DiskD88Track *DiskD88Disk::GetTrack(int track_number, int side_number)
{
	DiskD88Track *track = NULL;
	if (tracks) {
		for(size_t pos=0; pos<tracks->Count(); pos++) {
			DiskD88Track *t = tracks->Item(pos);
			if (t->GetTrackNumber() == track_number && t->GetSideNumber() == side_number) {
				track = t;
				break;
			}
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
	DiskParticulars singles;

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
				DiskParticular sd(t->GetTrackNumber(), t->GetSideNumber(), -1, s->GetSectorsPerTrack(), s->GetSectorSize());
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

	// 単密度の同じパラメータをまとめる
	DiskParticular::UniqueTracks(track_number_max + 1, side_number_max + 1, disk_single_type, singles);

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

	// セクタ数が異なるトラックを調べる
	DiskParticulars ptracks;
	if (tracks) {
		for(size_t ti=0; ti<tracks->Count(); ti++) {
			DiskD88Track *t =tracks->Item(ti);
			if (!t) continue;
			DiskD88Sectors *ss = t->GetSectors();
			if (!ss) continue;
			if ((int)ss->Count() != sectors_per_track) {
				ptracks.Add(DiskParticular(t->GetTrackNumber(), t->GetSideNumber(), -1, (int)ss->Count(), t->GetMaxSectorSize()));
			}
		}
		// 同じパラメータをまとめる
		DiskParticular::UniqueTracks(tracks_per_side, sides_per_disk, false, ptracks);
	}

	// メディアのタイプ
	const DiskParam *disk_param = gDiskTemplates.Find(sides_per_disk, tracks_per_side, sectors_per_track, sector_size, interleave, numbering_sector, singles, ptracks);
	if (disk_param != NULL) {
		SetDiskTypeName(disk_param->GetDiskTypeName());
		Reversible(disk_param->IsReversible());
		SetBasicTypes(disk_param->GetBasicTypes());
		SetSingles(singles);
		SetParamDensity(disk_param->GetParamDensity());
		SetDensityName(disk_param->GetDensityName());
		SetDescription(disk_param->GetDescription());
	}

	// DISK BASIC用の領域を確保
	AllocDiskBasics();

	// パラメータを保持
	SetOriginalParam(*this);

	return disk_param;
}

/// 書き込み禁止かどうかを返す
bool DiskD88Disk::IsWriteProtected() const
{
	return (!header || header->write_protect != 0);
}

/// 書き込み禁止かどうかを設定
void DiskD88Disk::SetWriteProtect(bool val)
{
	if (header) header->write_protect = (val ? 0x10 : 0);
}

/// 密度を返す
wxString DiskD88Disk::GetDensityText() const
{
	wxUint8 num = (header ? header->disk_density : 0);
	int match = FindDensity(num);
	return match >= 0 ? wxGetTranslation(gDiskDensity[match].name) : wxT("");
}

/// 密度を返す
int DiskD88Disk::GetDensity() const
{
	return (header ? header->disk_density : 0);
}

/// 密度を設定
void DiskD88Disk::SetDensity(int val)
{
	header->disk_density = val;
}

/// 密度を検索
/// @param [in] val 密度の値
int DiskD88Disk::FindDensity(int val)
{
	int match = -1;
	for(int i=0; gDiskDensity[i].name != NULL; i++) {
		if (gDiskDensity[i].val == val) {
			match = i;
			break;
		}
	}
	return match;
}

/// ディスクサイズ（ヘッダサイズ含む）
wxUint32 DiskD88Disk::GetSize() const
{
	return header ? header->disk_size : 0;
}

/// ディスクサイズ（ヘッダサイズ含む）を設定
/// @param [in] val サイズ（ヘッダサイズ含む）
void DiskD88Disk::SetSize(wxUint32 val)
{
	if (header) header->disk_size = val;
}

/// ディスクサイズ（ヘッダサイズを除く）
wxUint32 DiskD88Disk::GetSizeWithoutHeader() const
{
	return (header ? header->disk_size - offset_start : 0);
}

/// @param [in] val サイズ（ヘッダサイズを除く）を設定
void DiskD88Disk::SetSizeWithoutHeader(wxUint32 val)
{
	if (header) header->disk_size = (val + offset_start);
}

/// オフセット値を返す
/// @param [in] num    トラック番号
wxUint32 DiskD88Disk::GetOffset(int num) const
{
	if (!header || num < 0 || DISKD88_MAX_TRACKS <= num) return 0;
	return header->offsets[num];
}

/// オフセット値を設定
/// @param [in] num    トラック番号
/// @param [in] offset 位置（ヘッダサイズ含む）
void DiskD88Disk::SetOffset(int num, wxUint32 offset)
{
	if (!header || num < 0 || DISKD88_MAX_TRACKS <= num) return;
	header->offsets[num] = offset;
}

/// ヘッダサイズを除いたオフセット値を設定
/// @param [in] num    トラック番号
/// @param [in] offset 位置（ヘッダサイズを除く）
void DiskD88Disk::SetOffsetWithoutHeader(int num, wxUint32 offset)
{
	if (!header || num < 0 || DISKD88_MAX_TRACKS <= num) return;
	header->offsets[num] = (offset + offset_start);
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
//			rc = false;
			continue;
		}

		for(size_t sec_pos=0; sec_pos<secs->Count(); sec_pos++) {
			DiskD88Sector *sec = secs->Item(sec_pos);
			if (sec) {
				sec->Fill(0);
			}
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
		SetDiskParam(param.GetSidesPerDisk(), param.GetTracksPerSide(), param.GetSectorsPerTrack(), param.GetSectorSize(), param.GetParamDensity(), param.GetInterleave(), param.GetSingles()); 
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

/// DISK BASIC領域を確保
void DiskD88Disk::AllocDiskBasics()
{
	basics->Add();
	if (reversible) basics->Add();
}

/// DISK BASICを返す
DiskBasic *DiskD88Disk::GetDiskBasic(int idx)
{
	if (idx < 0) idx = 0;
	return basics->Item(idx);
}

/// DISK BASICをクリア
void DiskD88Disk::ClearDiskBasics()
{
	if (!basics) return;
	for(size_t idx=0; idx<basics->Count(); idx++) {
		DiskBasic *basic = GetDiskBasic((int)idx);
		if (!basic) continue;

		basic->ClearParseAndAssign();
	}
}

/// キャラクターコードマップ番号設定
void DiskD88Disk::SetCharCode(const wxString &name)
{
	if (!basics) return;
	for(size_t idx=0; idx<basics->Count(); idx++) {
		DiskBasic *basic = GetDiskBasic((int)idx);
		if (!basic) continue;

		basic->SetCharCode(name);
	}
}

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
	// cannot copy
}

DiskD88File::~DiskD88File()
{
	Clear();
}

size_t DiskD88File::Add(DiskD88Disk *newdsk, short mod_flags)
{
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
DiskWriteOptions::DiskWriteOptions()
{
	trim_unused_data = false;
}
DiskWriteOptions::DiskWriteOptions(
	bool n_trim_unused_data
) {
	trim_unused_data = n_trim_unused_data;
}
DiskWriteOptions::~DiskWriteOptions()
{
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
/// @param [in] param_hint  ディスクパラメータヒント("plain"時のみ)
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::Add(const wxString &filepath, const wxString &file_format, const DiskParam &param_hint)
{
	// ファイル開いていない
	if (!file) return 0;

	result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskParser ps(filepath, &fstream, file, result);
	int valid_disk = ps.ParseAdd(file_format, param_hint);

	return valid_disk;
}

/// ファイルを開く
/// @param [in] filepath    ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] param_hint  ディスクパラメータヒント("plain"時のみ)
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::Open(const wxString &filepath, const wxString &file_format, const DiskParam &param_hint)
{
	result.Clear();

	// ファイルを開く
	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	NewFile(filepath);
	DiskParser ps(filepath, &fstream, file, result);
	int valid_disk = ps.Parse(file_format, param_hint);

	// エラーあり
	if (valid_disk < 0) {
		ClearFile();
	}

	return valid_disk;
}

/// ファイルを開く前のチェック
/// @param [in] filepath        ファイルパス
/// @param [in,out] file_format ファイルの形式名("d88","plain"など)
/// @param [out] params         ディスクパラメータの候補
/// @param [out] manual_param   候補がないときのパラメータヒント
/// @retval  0 問題なし
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::Check(const wxString &filepath, wxString &file_format, DiskParamPtrs &params, DiskParam &manual_param)
{
	result.Clear();

	// ファイルを開く
	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskParser ps(filepath, &fstream, file, result);
	return ps.Check(file_format, params, manual_param);
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
int DiskD88::Save(const wxString &filepath, const wxString &file_format, const DiskWriteOptions &options)
{
	DiskWriter dw(this, filepath, options, &result);
	return dw.Save(file_format);
}
/// ストリームの内容をファイルに保存
int DiskD88::SaveDisk(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const DiskWriteOptions &options)
{
	DiskWriter dw(this, filepath, options, &result);
	return dw.SaveDisk(disk_number, side_number, file_format);
}

/// ディスクを削除
bool DiskD88::Delete(size_t disk_number)
{
	if (!file) return false;
	file->Delete(disk_number);
//	file->SetModify();
	return true;
}
/// 置換元のディスクを解析
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] filepath    ファイルパス
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] param_hint  ディスクパラメータヒント("plain"時のみ)
/// @param [out] src_file   ソースディスク
/// @param [out] tag_disk   ターゲットディスク
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::ParseForReplace(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const DiskParam &param_hint, DiskD88File &src_file, DiskD88Disk* &tag_disk)
{
	if (!file) return 0;

	result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskParser ps(filepath, &fstream, &src_file, result);
	int valid_disk = ps.Parse(file_format, param_hint);

	// エラーあり
	if (valid_disk < 0) {
		return valid_disk;
	}

	// ディスクを選択 
	tag_disk = file->GetDisk(disk_number);
	if (!tag_disk) {
		result.SetError(DiskResult::ERR_NO_DATA);
		return result.GetValid();
	}

	return 0;
}
/// ファイルでディスクを置換
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] src_disk    ソースディスク
/// @param [in] tag_disk    ターゲットディスク
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88::ReplaceDisk(int disk_number, int side_number, DiskD88Disk *src_disk, DiskD88Disk *tag_disk)
{
	if (!file) return 0;

	result.Clear();

#if 0
	if (side_number >= 0) {
		// AB面選択時、対象ディスクは1Sのみ
		if (src_disk->GetDiskTypeName() != "1S") {
			result.SetError(DiskResult::ERR_FILE_ONLY_1S);
			return result.GetValid();
		}
	} else {
		// 同じディスクタイプのみ
		if (src_disk->GetDiskTypeName() != tag_disk->GetDiskTypeName()) {
			result.SetError(DiskResult::ERR_FILE_SAME);
			return result.GetValid();
		}
	}
#endif
	int src_side_number = 0;
	int valid_disk = tag_disk->Replace(side_number, src_disk, src_side_number);
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
	return gDiskTemplates.IndexOf(disk->GetDiskTypeName());
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

/// DISK BASICが一致するか
bool DiskD88::MatchDiskBasic(const DiskBasic *target)
{
	bool match = false;
	DiskD88Disks *disks = GetDisks();
	if (!disks) return false;
	for(size_t i = 0; i < disks->Count(); i++) {
		DiskD88Disk *disk = disks->Item(i);
		DiskBasics *basics = disk->GetDiskBasics();
		if (!basics) return false;
		for(size_t j = 0; j < basics->Count(); j++) {
			if (target == basics->Item(j)) {
				match = true;
				break;
			}
		}
	}
	return match;
}

/// DISK BASICの解析状態をクリア
void DiskD88::ClearDiskBasicParseAndAssign(int disk_number, int side_number)
{
	DiskD88Disk *disk = GetDisk(disk_number);
	if (!disk) return;

	if (file) {
		file->SetBasicTypeHint(wxT(""));
	}

	DiskBasics *basics = disk->GetDiskBasics();
	if (!basics) return;
	basics->ClearParseAndAssign(side_number);
}

/// キャラクターコードマップ番号設定
void DiskD88::SetCharCode(const wxString &name)
{
	DiskD88Disks *disks = GetDisks();
	if (!disks) return;
	for(size_t i = 0; i < disks->Count(); i++) {
		DiskD88Disk *disk = disks->Item(i);
		disk->SetCharCode(name);
	}
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
