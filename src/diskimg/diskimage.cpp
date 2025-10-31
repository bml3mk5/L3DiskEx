/// @file diskimage.cpp
///
/// @brief ディスクイメージ入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskimage.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include "diskparam.h"
#include "diskparser.h"
#include "diskwriter.h"
#include "diskimagecreator.h"
#include "../basicfmt/basicparam.h"
#include "../basicfmt/basicfmt.h"


// ----------------------------------------------------------------------
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
int IntHashMapUtil::GetKeyCount(IntHashMap &hash_map)
{
	return (int)hash_map.size();
}
int IntHashMapUtil::MaxValue(int src, int value)
{
	return (src > value ? src : value);
}
int IntHashMapUtil::MinValue(int src, int value)
{
	return (src < value ? src : value);
}

// ----------------------------------------------------------------------
//
//
//
DiskImageSector::DiskImageSector(int n_num)
{
	m_num = n_num;
}

DiskImageSector::~DiskImageSector()
{
}
/// セクタ番号の比較
int DiskImageSector::Compare(DiskImageSector *item1, DiskImageSector *item2)
{
    return (item1->m_num - item2->m_num);
}
/// セクタ番号の比較
int DiskImageSector::CompareIDR(DiskImageSector **item1, DiskImageSector **item2)
{
    return ((int)(*item1)->GetIDR() - (int)(*item2)->GetIDR());
}
/// ID Nからセクタサイズを計算
int DiskImageSector::ConvIDNToSecSize(wxUint8 n)
{
	int sec = 0;
	if (n <= 3) sec = gSectorSizes[n];
	return sec;
}
/// セクタサイズからID Nを計算
wxUint8 DiskImageSector::ConvSecSizeToIDN(int size)
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

// ----------------------------------------------------------------------
//
//
//
DiskImageTrack::DiskImageTrack(DiskImageDisk *disk)
{
	parent = disk;
	m_trk_num = 0;
	m_sid_num = 0;
//	offset  = 0;
	sectors = NULL;
	m_size    = 0;
	m_interleave = 1;

	m_orig_sectors = 0;

	extra_data = NULL;
	extra_size = 0;
}

/// @param [in] disk            ディスク
/// @param [in] n_trk_num       トラック番号
/// @param [in] n_sid_num       サイド番号
/// @param [in] n_offset_pos    オフセットインデックス
/// @param [in] n_interleave    インターリーブ
DiskImageTrack::DiskImageTrack(DiskImageDisk *disk, int n_trk_num, int n_sid_num, int n_offset_pos, int n_interleave)
{
	parent = disk;
	m_trk_num = n_trk_num;
	m_sid_num = n_sid_num;
	m_offset_pos = n_offset_pos;
	sectors = NULL;
	m_size    = 0;
	m_interleave = n_interleave;

	m_orig_sectors = 0;

	extra_data = NULL;
	extra_size = 0;
}

DiskImageTrack::~DiskImageTrack()
{
	if (sectors) {
		for(size_t i=0; i<sectors->Count(); i++) {
			DiskImageSector *p = sectors->Item(i);
			delete p;
		}
		delete sectors;
	}
	delete [] extra_data;
}

/// セクタを追加する
/// @return セクタ数
size_t DiskImageTrack::Add(DiskImageSector *newsec)
{
	if (!sectors) sectors = new DiskImageSectors;
	sectors->Add(newsec);
	m_orig_sectors = sectors->Count();
	return m_orig_sectors;
}

/// トラック内のセクタデータを置き換える
/// @param [in] src_track
/// @return 0:正常 -1:エラー 1:置換できないセクタあり
int DiskImageTrack::Replace(DiskImageTrack *src_track)
{
	int rc = 0;
	if (!sectors) return -1;
	for(size_t i=0; i<sectors->Count(); i++) {
		DiskImageSector *tag_sector = sectors->Item(i);
		DiskImageSector *src_sector = src_track->GetSector(tag_sector->GetSectorNumber());
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
/// @param [in] status   新規セクタのステータス(通常0)
/// @return 0 正常
int DiskImageTrack::AddNewSector(int trknum, int sidnum, int secnum, int secsize, bool sdensity, int status)
{
	int rc = 0;

	// 新規セクタ
	DiskImageSector *new_sector = NewImageSector(
		trknum, sidnum, secnum, secsize, 1, sdensity, status
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
int DiskImageTrack::DeleteSectorByIndex(int pos)
{
	int rc = 0;
	if (!sectors || pos < 0 || pos >= (int)sectors->Count()) return -1;

	int removed_size = 0;
	DiskImageSector *sector = sectors->Item(pos);
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
int DiskImageTrack::DeleteSectors(int start_sector_num, int end_sector_num)
{
	int rc = 0;
	if (!sectors) return -1;
	bool removed = false;
	int  removed_size = 0;
	for(size_t i=0; i<sectors->Count(); i++) {
		DiskImageSector *sector = sectors->Item(i);
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
wxUint32 DiskImageTrack::Shrink(bool trim_unused_data)
{
	wxUint32 newsize = 0;

	size_t count = sectors ? sectors->Count() : 0;

	for(size_t i=0; i<count; i++) {
		DiskImageSector *sector = sectors->Item(i);
		sector->SetSectorsPerTrack((wxUint16)count);
		newsize += (wxUint32)sector->GetHeaderSize();
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
void DiskImageTrack::ShrinkAndCalcOffsets(bool trim_unused_data)
{
	Shrink(trim_unused_data);
	parent->CalcOffsets();
}

/// 余りバッファ領域のサイズを増やす
void DiskImageTrack::IncreaseExtraDataSize(size_t size)
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
void DiskImageTrack::DecreaseExtraDataSize(size_t size)
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
void DiskImageTrack::CalcInterleave()
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
		DiskImageSector *s = sectors->Item(sec_pos);
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
int DiskImageTrack::GetSectorsPerTrack() const
{
	int cnt = 0;
	if (sectors) {
		cnt = (int)sectors->Count();
	}
	return cnt;
}

/// 指定セクタ番号のセクタを返す
/// @param[in] sector_number セクタ番号
/// @param[in] density       密度で絞る 0:倍密度 1:単密度 -1:条件から除外
/// @return セクタ or NULL
DiskImageSector *DiskImageTrack::GetSector(int sector_number, int density)
{
	DiskImageSector *sector = NULL;
	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskImageSector *s = sectors->Item(pos);
			if (s->IsSameSector(sector_number, density)) {
				sector = s;
				break;
			}
		}
	}
	return sector;
}

/// 指定位置のセクタを返す
DiskImageSector *DiskImageTrack::GetSectorByIndex(int pos)
{
	DiskImageSector *sector = NULL;
	if (sectors && pos >= 0 && pos < (int)sectors->Count()) {
		sector = sectors->Item(pos);
	}
	return sector;
}

/// トラック内のもっともらしいID Cを返す
wxUint8	DiskImageTrack::GetMajorIDC() const
{
	wxUint8 id = 0;
	IntHashMap map;

	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskImageSector *s = sectors->Item(pos);
			IntHashMapUtil::IncleaseValue(map, s->GetIDC());
		}
		id = IntHashMapUtil::GetMaxKeyOnMaxValue(map);
	}
	return id;
}

/// トラック内のもっともらしいID Hを返す
wxUint8	DiskImageTrack::GetMajorIDH() const
{
	wxUint8 id = 0;
	IntHashMap map;

	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskImageSector *s = sectors->Item(pos);
			IntHashMapUtil::IncleaseValue(map, s->GetIDH());
		}
		id = IntHashMapUtil::GetMaxKeyOnMaxValue(map);
	}
	return id;
}

/// トラック内のすべてのID Cを変更
void DiskImageTrack::SetAllIDC(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskImageSector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDC(val);
//			sector->SetModify();
		}
	}
}

/// トラック内のすべてのID Hを変更
void DiskImageTrack::SetAllIDH(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskImageSector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDH(val);
//			sector->SetModify();
		}
	}
}

/// トラック内のすべてのID Rを変更
void DiskImageTrack::SetAllIDR(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskImageSector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDR(val);
//			sector->SetModify();
		}
	}
}

/// トラック内のすべてのID Nを変更
void DiskImageTrack::SetAllIDN(wxUint8 val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskImageSector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetIDN(val);
//			sector->SetModify();
		}
	}
}

/// トラック内のすべての密度を変更
void DiskImageTrack::SetAllSingleDensity(bool val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskImageSector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetSingleDensity(val);
//			sector->SetModify();
		}
	}
}

/// トラック内のすべてのセクタ数を変更
void DiskImageTrack::SetAllSectorsPerTrack(int val)
{
	if (!sectors) return;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskImageSector *sector = sectors->Item(pos);
		if (sector) {
			sector->SetSectorsPerTrack(val);
//			sector->SetModify();
		}
	}
}

/// トラック内のすべてのセクタサイズを変更
void DiskImageTrack::SetAllSectorSize(int val)
{
	if (!sectors) return;

	int sum = 0;
	for(size_t pos=0; pos<sectors->Count(); pos++) {
		DiskImageSector *sector = sectors->Item(pos);
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
void DiskImageTrack::SetExtraData(wxUint8 *buf, size_t size)
{
	delete extra_data;
	extra_data = buf;
	extra_size = size;
}

/// トラック内の最小セクタ番号を返す
int DiskImageTrack::GetMinSectorNumber() const
{
	int sector_number = 0x7fffffff;
	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskImageSector *s = sectors->Item(pos);
			if (sector_number > s->GetSectorNumber()) {
				sector_number = s->GetSectorNumber();
			}
		}
	}
	return sector_number;
}

/// トラック内の最大セクタ番号を返す
int DiskImageTrack::GetMaxSectorNumber() const
{
	int sector_number = 0;
	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskImageSector *s = sectors->Item(pos);
			if (sector_number < s->GetSectorNumber()) {
				sector_number = s->GetSectorNumber();
			}
		}
	}
	return sector_number;
}

/// トラック内の最大セクタサイズを返す
int DiskImageTrack::GetMaxSectorSize() const
{
	int sector_size = 0;
	if (sectors) {
		for(size_t pos=0; pos<sectors->Count(); pos++) {
			DiskImageSector *s = sectors->Item(pos);
			if (sector_size < s->GetSectorSize()) {
				sector_size = s->GetSectorSize();
			}
		}
	}
	return sector_size;
}

/// 変更されているか
bool DiskImageTrack::IsModified() const
{
	if (!sectors) return false;
	if (m_orig_sectors != sectors->Count()) return true;

	bool modified = false;
	for(size_t sector_num = 0; sector_num < sectors->Count() && !modified; sector_num++) {
		DiskImageSector *sector = sectors->Item(sector_num);
		if (!sector) continue;

		modified = sector->IsModified();
		if (modified) {
			break;
		}
	}
	return modified;
}

/// 変更済みをクリア
void DiskImageTrack::ClearModify()
{
	if (!sectors) return;
	for(size_t sector_num = 0; sector_num < sectors->Count(); sector_num++) {
		DiskImageSector *sector = sectors->Item(sector_num);
		if (!sector) continue;

		sector->ClearModify();
	}
	m_orig_sectors = sectors->Count();
}

/// トラック番号とサイド番号の比較
int DiskImageTrack::Compare(DiskImageTrack *item1, DiskImageTrack *item2)
{
    return ((item1->m_trk_num - item2->m_trk_num) | (item1->m_sid_num - item2->m_sid_num));
}

/// インターリーブを考慮したセクタ番号リストを返す
/// @param[in]  interleave    インターリーブ(1...)
/// @param[in]  sectors_count セクタ数
/// @param[out] sector_nums   配列
/// @param[in]  sector_offset オフセット
///
/// interleave = 2 の時
/// sector_nums[0] = sector_offset, sector_nums[2] = sector_offset + 1, sector_nums[4] = sector_offset + 2, ... となる
bool DiskImageTrack::CalcSectorNumbersForInterleave(int interleave, size_t sectors_count, wxArrayInt &sector_nums, int sector_offset)
{
	sector_nums.SetCount(sectors_count, -1);
	int sector_pos = 0;
	bool err = false;
	for(int sector_number = 0; sector_number < (int)sectors_count && err == false; sector_number++) {
		if (sector_pos >= (int)sectors_count) {
			sector_pos -= sectors_count;
			while (sector_nums[sector_pos] >= 0) {
				sector_pos++;
				if (sector_pos >= (int)sectors_count) {
					// ?? error
					err = true;
					break;
				}
			}
		}
		sector_nums[sector_pos] = (sector_number + sector_offset);
		sector_pos += interleave;
	}

	return !err;
}

// ----------------------------------------------------------------------
//
//
//

/// @param[in] file ファイルイメージ
/// @param[in] n_num ディスク番号
DiskImageDisk::DiskImageDisk(DiskImageFile *file, int n_num)
	: DiskParam()
{
	parent = file;
	m_num = n_num;
	m_write_protect = false;

	tracks = NULL;
	m_offset_start = 0;

	p_temp_param = NULL;
	m_param_changed = false;

	basics = new DiskBasics;
}

/// @param[in] file ファイルイメージ
/// @param[in] n_num ディスク番号
/// @param[in] n_param ディスクパラメータ
/// @param[in] n_diskname ディスク名
/// @param[in] n_write_protect 書き込み禁止か
DiskImageDisk::DiskImageDisk(DiskImageFile *file, int n_num, const DiskParam &n_param, const wxString &n_diskname, bool n_write_protect)
	: DiskParam(n_param)
{
	parent = file;
	m_num = n_num;
	m_name = n_diskname;
	m_write_protect = n_write_protect;

	tracks = NULL;
	m_offset_start = 0;

	p_temp_param = NULL;
	m_param_changed = false;

	basics = new DiskBasics;
}

/// @param[in] file ファイルイメージ
/// @param[in] n_num ディスク番号
/// @param[in] n_header ディスクヘッダ
DiskImageDisk::DiskImageDisk(DiskImageFile *file, int n_num, const DiskImageDiskHeader &n_header)
	: DiskParam()
{
	parent = file;
	m_num = n_num;
	m_name = n_header.GetName(false);
	m_write_protect = n_header.IsWriteProtected();

	tracks = NULL;
	m_offset_start = 0;

	p_temp_param = NULL;
	m_param_changed = false;

	basics = new DiskBasics;
}

DiskImageDisk::~DiskImageDisk()
{
	if (tracks) {
		for(size_t i=0; i<tracks->Count(); i++) {
			DiskImageTrack *p = tracks->Item(i);
			delete p;
		}
		delete tracks;
	}
	delete basics;
}

/// ディスクにトラックを追加
/// @return トラック数
size_t DiskImageDisk::Add(DiskImageTrack *newtrk)
{
//	if (!tracks) tracks = new DiskImageTracks(DiskImageTrack::Compare);
//	return tracks->Add(new DiskImageTrack(newtrk));
	if (!tracks) tracks = new DiskImageTracks;
	tracks->Add(newtrk);
//	SetMaxTrackNumber(newtrk->GetTrackNumber());
	return tracks->Count();
}

/// ディスクの内容を置き換える
/// @param [in] side_number : サイド番号
/// @param [in] src_disk : 置換元のディスクイメージ
/// @param [in] src_side_number : 置換元のディスクイメージのサイド番号
int DiskImageDisk::Replace(int side_number, DiskImageDisk *src_disk, int src_side_number)
{
	int rc = 0;
	if (!tracks) return -1;
	for(size_t i=0; i<tracks->Count(); i++) {
		DiskImageTrack *tag_track = tracks->Item(i);
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
		} else {
			if (GetSidesPerDisk() <= 1) {
				// AB面のどちらかを片面ディスクにコピーする場合
				tag_side_number = src_side_number;
			}
		}
		DiskImageTrack *src_track = src_disk->GetTrack(tag_track->GetTrackNumber(), tag_side_number);
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
int DiskImageDisk::AddNewTrack(int side_number)
{
	int rc = 0;

	// 最大トラック＆サイド番号
	DiskImageTrack *src_track = NULL;
	int trk_num = -1;
	int sid_num = -1;
	int max_sid_num = -1;
	for(size_t pos = 0; pos < tracks->Count(); pos++) {
		DiskImageTrack *track = tracks->Item(pos);
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
	DiskImageSectors *sectors = src_track->GetSectors();
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
	int limit_pos = GetCreatableTracks();
	int offset_pos = 0;
	for(int pos = (side_number < 0 ? 0 : side_number); pos < limit_pos; pos += (side_number < 0 ? 1 : max_sid_num + 1)) {
		if (GetOffset(pos) == 0) {
			offset_pos = pos;
			break;
		}
	}
	if (offset_pos == 0) {
		// 空きなし
		return -1;
	}

	DiskImageTrack *new_track = NewImageTrack(
		trk_num,
		sid_num,
		offset_pos,
		src_track->GetInterleave()
	);
	// セクタを追加
	wxUint32 trk_size = 0;
	for(size_t pos = 0; pos < sectors->Count(); pos++) {
		DiskImageSector *sector = sectors->Item(pos);
		// 新規セクタ
		int sec_num = sector->GetIDR();
		int sec_size = sector->GetSectorSize();
		bool sdensity = sector->IsSingleDensity();

		DiskImageSector *new_sector = new_track->NewImageSector(
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
void DiskImageDisk::DeleteTracks(int start_offset_pos, int end_offset_pos, int side_number)
{
//	size_t deleted_size = 0;

	if (!tracks) return;

	bool removed = false;
	for(size_t i=0; i<tracks->Count(); i++) {
		DiskImageTrack *track = tracks->Item(i);
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
size_t DiskImageDisk::ShrinkTracks(bool trim_unused_data)
{
	if (tracks) {
		for(size_t i=0; i<tracks->Count(); i++) {
			DiskImageTrack *track = tracks->Item(i);
			if (!track) continue;
			track->Shrink(trim_unused_data);
		}
	}
	return CalcOffsets();
}

/// オフセットの再計算＆ディスクサイズ変更
size_t DiskImageDisk::CalcOffsets()
{
	size_t new_size = 0;

	if (!tracks) return new_size;

	// オフセットをクリア
	int limit_pos = GetCreatableTracks();
	for(int pos = 0; pos < limit_pos; pos++) {
		SetOffset(pos, 0);
	}

	// 再計算
	wxUint32 max_offset = m_offset_start;
	for(size_t i=0; i<tracks->Count(); i++) {
		DiskImageTrack *track = tracks->Item(i);
		if (!track) continue;
		int pos = track->GetOffsetPos();
		if (pos < 0 || pos >= limit_pos) continue;
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
size_t DiskImageDisk::CalcSizeWithoutHeader()
{
	size_t new_size = 0;

	if (!tracks) return new_size;

	for(size_t i=0; i<tracks->Count(); i++) {
		DiskImageTrack *track = tracks->Item(i);
		if (!track) continue;
		new_size += track->GetSize();
	}
	return new_size;
}

/// 変更済みに設定
void DiskImageDisk::SetModify()
{
}

/// 変更されているか
bool DiskImageDisk::IsModified()
{
	bool modified = false;
	if (tracks) {
		for(size_t track_num = 0; track_num < tracks->Count() && !modified; track_num++) {
			DiskImageTrack *track = tracks->Item(track_num);
			if (!track) continue;

			modified = track->IsModified();
			if (modified) {
				break;
			}
		}
	}
	return modified;
}

/// 変更済みをクリア
void DiskImageDisk::ClearModify()
{
	if (tracks) {
		for(size_t track_num = 0; track_num < tracks->Count(); track_num++) {
			DiskImageTrack *track = tracks->Item(track_num);
			if (!track) continue;

			track->ClearModify();
		}
	}
}

/// 指定トラックを返す
/// @param[in] track_number トラック番号（シリンダ）
/// @param[in] side_number  サイド番号（ヘッド）
/// @return トラック
DiskImageTrack *DiskImageDisk::GetTrack(int track_number, int side_number)
{
	DiskImageTrack *track = NULL;
	if (tracks) {
		for(size_t pos=0; pos<tracks->Count(); pos++) {
			DiskImageTrack *t = tracks->Item(pos);
			if (t->GetTrackNumber() == track_number && t->GetSideNumber() == side_number) {
				track = t;
				break;
			}
		}
	}
	return track;
}

/// 指定トラックを返す
/// @param[in] index 位置
/// @return トラック
DiskImageTrack *DiskImageDisk::GetTrack(int index)
{
	DiskImageTrack *track = NULL;
	if (tracks && index < (int)tracks->Count()) {
		track = tracks->Item(index);
	}
	return track;
}

/// 指定オフセット値からトラックを返す
/// @param[in] offset オフセット位置
/// @return トラック
DiskImageTrack *DiskImageDisk::GetTrackByOffset(wxUint32 offset)
{
	DiskImageTrack *track = NULL;
	if (tracks) {
		for(size_t i=0; i<tracks->Count(); i++) {
			DiskImageTrack *t = tracks->Item(i);
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
/// @param[in] track_number  トラック番号（シリンダ）
/// @param[in] side_number   サイド番号（ヘッド）
/// @param[in] sector_number セクタ番号（レコード）
/// @param[in] density       密度で絞る 0:倍密度 1:単密度 -1:条件から除外
/// @return セクタ
DiskImageSector *DiskImageDisk::GetSector(int track_number, int side_number, int sector_number, int density)
{
	DiskImageTrack *trk = GetTrack(track_number, side_number);
	if (!trk) return NULL;
	return trk->GetSector(sector_number, density);
}

/// ディスクの中でもっともらしいパラメータを設定
/// @return パラメータ
const DiskParam *DiskImageDisk::CalcMajorNumber()
{
	IntHashMap sector_numbers_map[2];
	IntHashMap sector_size_map;
	IntHashMap interleave_map;
//	IntHashMap::iterator it;

	int track_number_min = 0x7fffffff;
	int track_number_max = 0;
	int side_number_min = 0x7fffffff;
	int side_number_max = 0;

	int sector_number_max_side0 = 0;
	int sector_number_min_side0 = 0x7fffffff;
	int sector_number_min_side1 = 0x7fffffff;

	long sector_masize = 0;
	long interleave_max = 0;
	DiskParticulars singles;

	if (tracks) {
		// トラックごとの各値を集計
		for(size_t ti=0; ti<tracks->Count(); ti++) {
			DiskImageTrack *t = tracks->Item(ti);

			int trk_num = t->GetTrackNumber();
			int sid_num = t->GetSideNumber();

			// トラック番号の最小値
			track_number_min = IntHashMapUtil::MinValue(track_number_min, trk_num);
			// トラック番号の最大値
			track_number_max = IntHashMapUtil::MaxValue(track_number_max, trk_num);

			if (trk_num > 0) {
				// サイド番号の最小値
				side_number_min = IntHashMapUtil::MinValue(side_number_min, sid_num);
				// サイド番号の最大値
				side_number_max = IntHashMapUtil::MaxValue(side_number_max, sid_num);
			}

			// セクタサイズはディスク内で最も使用されているサイズ
			IntHashMapUtil::IncleaseValue(sector_size_map, t->GetMaxSectorSize());
			// インターリーブはディスク内で最も使用されているもの
			IntHashMapUtil::IncleaseValue(interleave_map, t->GetInterleave());

			sid_num &= 0x7f;

			// セクタ数
			if (sid_num >= 0 && sid_num < 2) {
				IntHashMapUtil::IncleaseValue(sector_numbers_map[sid_num], t->GetSectorsPerTrack());
			}

			if (trk_num > 0 && trk_num <= 10 && sid_num < 2) {
				// トラック1-10で判定

				// セクタ番号の最大と最小
				int sec_num_max = t->GetMaxSectorNumber();
				int sec_num_min = t->GetMinSectorNumber();
				if (sid_num == 0) {
					sector_number_max_side0 = IntHashMapUtil::MaxValue(sector_number_max_side0, sec_num_max);
					sector_number_min_side0 = IntHashMapUtil::MinValue(sector_number_min_side0, sec_num_min);
				} else {
					sector_number_min_side1 = IntHashMapUtil::MinValue(sector_number_min_side1, sec_num_min);
				}
			}
			// 単密度か？
			DiskImageSectors *sectors = t->GetSectors();
			if (sectors) {
				DiskParticulars sis;
				for(size_t si = 0; si < sectors->Count(); si++) {
					DiskImageSector *s = sectors->Item(si);
					if (s && s->IsSingleDensity()) {
						DiskParticular sd(t->GetTrackNumber(), t->GetSideNumber(), s->GetSectorNumber(), 1, s->GetSectorsPerTrack(), s->GetSectorSize());
						sis.Add(sd);
					}
				}
				DiskParticular::UniqueSectors(t->GetSectorsPerTrack(), sis);
				for(size_t si = 0; si < sis.Count(); si++) {
					singles.Add(sis.Item(si));
				}
			}
		}
	}
	sector_masize = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_size_map);
	interleave_max = IntHashMapUtil::GetMaxKeyOnMaxValue(interleave_map);

	// サイド番号のチェック
	sides_per_disk = side_number_max + 1 - side_number_min;

	// トラック番号のチェック
	if (tracks) {
		int track_count = ((int)tracks->Count() + sides_per_disk - 1) / sides_per_disk;
		// 実際に存在するトラック数よりトラック番号がかなり大きい場合
		// 最大トラック番号をトラック数にする
		if (track_number_max > (track_count + 4)) {
			track_number_max = track_count - 1;
		}
	}

	bool disk_single_type = false;
	if (tracks) {
		if (sector_masize == 128 && side_number_max == 0 && m_max_track_number > track_number_max) {
			// 単密度で両面タイプ
			disk_single_type = true;
			side_number_max++;
			sides_per_disk++;
			for(size_t ti=0; ti<tracks->Count(); ti++) {
				DiskImageTrack *t =tracks->Item(ti);
				if (t->GetOffsetPos() & 1) {
					// 奇数の場合はサイドを1にする
					t->SetSideNumber(1);
				}
			}
		}
	}

	// 単密度の同じパラメータをまとめる
	DiskParticular::UniqueTracks(track_number_max - track_number_min + 1, sides_per_disk, disk_single_type, singles);

	// ディスク内で最も多く使われているセクタ数を調べる
//	sides_per_disk = side_number_max + 1;
	tracks_per_side = tracks ? (track_number_max - track_number_min + 1) : 0;
	sector_size = (int)sector_masize;
	interleave = (int)interleave_max;
	if (sides_per_disk > 1 && sector_number_min_side1 != 0x7fffffff && sector_number_max_side0 < sector_number_min_side1) {
		// セクタ番号がサイドを通して連番になっている
		numbering_sector = 1;
		long sec_num_maj = 0;
		sec_num_maj = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_numbers_map[0]);
		sectors_per_track = (int)sec_num_maj;
	} else {
		// セクタ番号はサイド毎
		numbering_sector = 0;
		int sec_num_maj[2];
		for(int i=0; i<2; i++) {
//			sec_num_maj[i] = 0;
			sec_num_maj[i] = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_numbers_map[i]);
		}
		sectors_per_track = (int)(sec_num_maj[0] > sec_num_maj[1] ? sec_num_maj[0] : sec_num_maj[1]);
	}

	// セクタ数が異なるトラックを調べる
	DiskParticulars ptracks;
	if (tracks) {
		for(size_t ti=0; ti<tracks->Count(); ti++) {
			DiskImageTrack *t =tracks->Item(ti);
			if (!t) continue;
			DiskImageSectors *ss = t->GetSectors();
			if (!ss) continue;
			// 単密度リストに含まれている場合は除外
			if (singles.FindTrackSide(t->GetTrackNumber(), t->GetSideNumber()) >= 0) {
				continue;
			}
			// セクタ数が異なるか
			if ((int)ss->Count() != sectors_per_track) {
				ptracks.Add(DiskParticular(t->GetTrackNumber(), t->GetSideNumber(), -1, 1, (int)ss->Count(), t->GetMaxSectorSize()));
			}
		}
		// 同じパラメータをまとめる
		DiskParticular::UniqueTracks(tracks_per_side, sides_per_disk, false, ptracks);
	}

	// メディアのタイプ
	const DiskParam *disk_param = gDiskTemplates.Find(sides_per_disk, tracks_per_side, sectors_per_track, sector_size
		, interleave, track_number_min, side_number_min, sector_number_min_side0, numbering_sector
		, singles, ptracks);
	SetTemplateParam(disk_param);
	if (disk_param != NULL) {
		SetDiskTypeName(disk_param->GetDiskTypeName());
		Reversible(disk_param->IsReversible());
		SetBasicTypes(disk_param->GetBasicTypes());
		SetSingles(singles);
		SetTrackNumberBaseOnDisk(disk_param->GetTrackNumberBaseOnDisk());
		SetSideNumberBaseOnDisk(disk_param->GetSideNumberBaseOnDisk());
		SetSectorNumberBaseOnDisk(disk_param->GetSectorNumberBaseOnDisk());
		VariableSectorsPerTrack(disk_param->IsVariableSectorsPerTrack());
		SetParamDensity(disk_param->GetParamDensity());
		SetParticularTracks(disk_param->GetParticularTracks());
		SetDensityName(disk_param->GetDensityName());
		SetDescription(disk_param->GetDescription());
	}

	// DISK BASIC用の領域を確保
	AllocDiskBasics();

	// パラメータを保持
	SetOriginalParam(*this);

	return disk_param;
}

/// ディスクの内容を初期化する(0パディング)
/// @param [in] selected_side >=0なら指定サイドのみ初期化
bool DiskImageDisk::Initialize(int selected_side)
{
	if (!tracks) {
		return false;
	}

	bool rc = true;
	for(size_t track_pos=0; track_pos<tracks->Count(); track_pos++) {
		DiskImageTrack *track = tracks->Item(track_pos);
		if (selected_side >= 0) {
			// サイド指定ありの時はそのサイドのみ初期化
			if (selected_side != track->GetSideNumber()) {
				continue;
			}
		}

		DiskImageSectors *secs = track->GetSectors();
		if (!secs) {
//			rc = false;
			continue;
		}

		for(size_t sec_pos=0; sec_pos<secs->Count(); sec_pos++) {
			DiskImageSector *sec = secs->Item(sec_pos);
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
bool DiskImageDisk::Rebuild(const DiskParam &param, int selected_side)
{
	if (selected_side >= 0) {
		SetDiskParam(
			param.GetSidesPerDisk(),
			param.GetTracksPerSide(),
			param.GetSectorsPerTrack(),
			param.GetSectorSize(),
			param.GetParamDensity(),
			param.GetInterleave(),
			param.GetSingles(),
			param.GetParticularTracks()
		); 
	} else {
		SetDiskParam(param);
	}

	DiskResult result;
	wxString diskname;
	DiskImageCreator cr(diskname, param, false, NULL, result);
	bool rc = true;
	int trk = param.GetTrackNumberBaseOnDisk();
	int trks = param.GetTracksPerSide() + trk;
	int sid = 0;
	int sides = param.GetSidesPerDisk();
	for(int pos=0; pos<GetCreatableTracks(); pos++) {
		if (selected_side >= 0) {
			sid = selected_side;
			// サイド指定ありの時はそのサイドのみ初期化
			if (selected_side != (pos % sides)) {
				continue;
			}
		}

		wxUint32 offset = GetOffset(pos);
		DiskImageTrack *track = GetTrackByOffset(offset);
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
		if (trk >= trks) {
			SetMaxTrackNumber(pos);
			break;
		}
	}
	return rc;
}

/// ディスク番号を比較
int DiskImageDisk::Compare(DiskImageDisk *item1, DiskImageDisk *item2)
{
    return (item1->m_num - item2->m_num);
}

/// トラックが存在するか
bool DiskImageDisk::ExistTrack(int side_number)
{
	bool found = false;
	DiskImageTracks *tracks = GetTracks();
	if (tracks) {
		for(size_t num=0; num < tracks->Count(); num++) {
			DiskImageTrack *trk = tracks->Item(num);
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
void DiskImageDisk::AllocDiskBasics()
{
	basics->Add();
	if (reversible) basics->Add();
}

/// DISK BASICを返す
DiskBasic *DiskImageDisk::GetDiskBasic(int idx)
{
	if (idx < 0) idx = 0;
	return basics->Item(idx);
}

/// DISK BASICをクリア
void DiskImageDisk::ClearDiskBasics()
{
	if (!basics) return;
	for(size_t idx=0; idx<basics->Count(); idx++) {
		DiskBasic *basic = GetDiskBasic((int)idx);
		if (!basic) continue;

		basic->ClearParseAndAssign();
	}
}

/// キャラクターコードマップ番号設定
void DiskImageDisk::SetCharCode(const wxString &name)
{
	if (!basics) return;
	for(size_t idx=0; idx<basics->Count(); idx++) {
		DiskBasic *basic = GetDiskBasic((int)idx);
		if (!basic) continue;

		basic->SetCharCode(name);
	}
}

// ----------------------------------------------------------------------
//
//
//
DiskImageFile::DiskImageFile()
{
	p_image = NULL;
	disks = NULL;
	mods  = NULL;
}

DiskImageFile::DiskImageFile(const DiskImageFile &src)
{
	// cannot copy
}

DiskImageFile::DiskImageFile(DiskImage &image)
{
	p_image = &image;
	disks = NULL;
	mods  = NULL;
}

DiskImageFile::~DiskImageFile()
{
	Clear();
}

/// ディスクを追加
size_t DiskImageFile::Add(DiskImageDisk *newdsk, short mod_flags)
{
	if (!disks) disks = new DiskImageDisks;
	if (!mods)  mods  = new wxArrayShort;
	disks->Add(newdsk);
	mods->Add(mod_flags);
	return disks->Count();
}

/// 全ディスクを削除
void DiskImageFile::Clear()
{
	if (disks) {
		for(size_t i=0; i<disks->Count(); i++) {
			DiskImageDisk *p = disks->Item(i);
			delete p;
		}
		delete disks;
		disks = NULL;
	}
	if (mods) {
		delete mods;
	}
}

/// ディスク数を返す
size_t DiskImageFile::Count() const
{
	if (!disks) return 0;
	return disks->Count();
}

/// ディスクを削除
bool DiskImageFile::Delete(size_t idx)
{
	DiskImageDisk *disk = GetDisk(idx);
	if (!disk) return false;
	delete disk;
	disks->RemoveAt(idx);
	mods->RemoveAt(idx);
	return true;
}

/// ディスクを返す
DiskImageDisk *DiskImageFile::GetDisk(size_t idx)
{
	if (!disks) return NULL;
	if (idx >= disks->Count()) return NULL;
	return disks->Item(idx);
}

bool DiskImageFile::IsModified()
{
	bool modified = false;
	if (disks) {
		for(size_t disk_num = 0; disk_num < disks->Count() && !modified; disk_num++) {
			modified = (mods->Item(disk_num) != 0);
			if (modified) break;

			DiskImageDisk *disk = disks->Item(disk_num);
			if (!disk) continue;

			modified = disk->IsModified();
			if (modified) break;
		}
	}
	return modified;
}

void DiskImageFile::ClearModify()
{
	if (disks) {
		for(size_t disk_num = 0; disk_num < disks->Count(); disk_num++) {
			mods->Item(disk_num) = MODIFY_NONE;

			DiskImageDisk *disk = disks->Item(disk_num);
			if (!disk) continue;

			disk->ClearModify();
		}
	}
}

// ======================================================================
//
//
//
DiskImage::DiskImage()
{
	p_file = NULL;
}

DiskImage::~DiskImage()
{
	ClearFile();
}

/// 新規作成
/// @param [in] diskname      ディスク名
/// @param [in] param         ディスクパラメータ
/// @param [in] write_protect 書き込み禁止
/// @param [in] basic_hint    DISK BASIC種類のヒント
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskImage::Create(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint)
{
	m_result.Clear();

	NewFile(wxEmptyString);
	p_file->SetBasicTypeHint(basic_hint);
	DiskImageCreator cr(diskname, param, write_protect, p_file, m_result);
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
int DiskImage::Add(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint)
{
	if (!p_file) return 0;

	m_result.Clear();

	p_file->SetBasicTypeHint(basic_hint);
	DiskImageCreator cr(diskname, param, write_protect, p_file, m_result);
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
int DiskImage::Add(const wxString &filepath, const wxString &file_format, const DiskParam &param_hint)
{
	// ファイル開いていない
	if (!p_file) return 0;

	m_result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		m_result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskParser ps(filepath, &fstream, p_file, m_result);
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
int DiskImage::Open(const wxString &filepath, const wxString &file_format, const DiskParam &param_hint)
{
	m_result.Clear();

	// ファイルを開く
	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		m_result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	NewFile(filepath);
	DiskParser ps(filepath, &fstream, p_file, m_result);
	int valid_disk = ps.Parse(file_format, param_hint);

	if (valid_disk < 0) {
		// エラーあり
		ClearFile();
	} else {
		SetFormatType(file_format);
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
int DiskImage::Check(const wxString &filepath, wxString &file_format, DiskParamPtrs &params, DiskParam &manual_param)
{
	m_result.Clear();

	// ファイルを開く
	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		m_result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskParser ps(filepath, &fstream, p_file, m_result);
	return ps.Check(file_format, params, manual_param);
}

/// 閉じる
void DiskImage::Close()
{
	ClearFile();
	m_filename.Clear();
}

/// ストリームの内容をファイルに保存できるか
/// @param[in] file_format 保存ファイルのフォーマット
int DiskImage::CanSave(const wxString &file_format)
{
	DiskWriter dw(this, &m_result);
	return dw.CanSave(file_format);
}
/// ストリームの内容をファイルに保存
/// @param[in] filepath    保存先ファイルパス
/// @param[in] file_format 保存ファイルのフォーマット
/// @param[in] options     保存時のオプション
/// @retval  0:正常
/// @retval -1:エラー
int DiskImage::Save(const wxString &filepath, const wxString &file_format, const DiskWriteOptions &options)
{
	DiskWriter dw(this, filepath, options, &m_result);
	return dw.Save(file_format);
}
/// ストリームの内容をファイルに保存
/// @param[in] disk_number ディスク番号
/// @param[in] side_number サイド番号
/// @param[in] filepath    保存先ファイルパス
/// @param[in] file_format 保存ファイルのフォーマット
/// @param[in] options     保存時のオプション
/// @retval  0:正常
/// @retval -1:エラー
int DiskImage::SaveDisk(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const DiskWriteOptions &options)
{
	DiskWriter dw(this, filepath, options, &m_result);
	return dw.SaveDisk(disk_number, side_number, file_format);
}

/// ディスクを削除
/// @param[in] disk_number ディスク番号
/// @return true
bool DiskImage::Delete(int disk_number)
{
	if (!p_file) return false;
	p_file->Delete((size_t)disk_number);
//	p_file->SetModify();
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
int DiskImage::ParseForReplace(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const DiskParam &param_hint, DiskImageFile &src_file, DiskImageDisk* &tag_disk)
{
	if (!p_file) return 0;

	m_result.Clear();

	wxFileInputStream fstream(filepath);
	if (!fstream.IsOk()) {
		m_result.SetError(DiskResult::ERR_CANNOT_OPEN);
		return -1;
	}

	DiskParser ps(filepath, &fstream, &src_file, m_result);
	int valid_disk = ps.Parse(file_format, param_hint);

	// エラーあり
	if (valid_disk < 0) {
		return valid_disk;
	}

	// ディスクを選択 
	tag_disk = p_file->GetDisk(disk_number);
	if (!tag_disk) {
		m_result.SetError(DiskResult::ERR_NO_DATA);
		return m_result.GetValid();
	}

	return 0;
}
/// ファイルでディスクを置換
/// @param [in] disk_number     ディスク番号
/// @param [in] side_number     サイド番号
/// @param [in] src_disk        ソースディスク
/// @param [in] src_side_number ソース側のサイド番号
/// @param [in] tag_disk        ターゲットディスク
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskImage::ReplaceDisk(int disk_number, int side_number, DiskImageDisk *src_disk, int src_side_number, DiskImageDisk *tag_disk)
{
	if (!p_file) return 0;

	m_result.Clear();

	int valid_disk = tag_disk->Replace(side_number, src_disk, src_side_number);
	if (valid_disk != 0) {
		m_result.SetError(DiskResult::ERR_REPLACE);
	}

	return valid_disk;
}

/// ディスク名を設定
bool DiskImage::SetDiskName(size_t disk_number, const wxString &newname)
{
	DiskImageDisk *disk = GetDisk(disk_number);
	if (!disk) return false;

	if (disk->GetName() != newname) {
		disk->SetName(newname);
		disk->SetModify();
		return true;
	}
	return false;
}
/// ディスク名を返す
wxString DiskImage::GetDiskName(size_t disk_number, bool real) const
{
	const DiskImageDisk *disk = GetDisk(disk_number);
	if (!disk) return wxEmptyString;
	return disk->GetName(real);
}

/// ファイル構造体を作成
void DiskImage::NewFile(const wxString &filepath)
{
	if (p_file) {
		delete p_file;
	}
	p_file = NewImageFile();
	m_filename = filepath;
}

/// ファイル構造体をクリア
void DiskImage::ClearFile()
{
	delete p_file;
	p_file = NULL;
}
/// ディスクを変更したか
bool DiskImage::IsModified()
{
	bool modified = false;
	if (p_file) {
		modified = p_file->IsModified();
	}
	return modified;
}
/// ディスク枚数
size_t DiskImage::CountDisks() const
{
	if (!p_file) return 0;
	return p_file->Count();
}
/// ディスク一覧を返す
DiskImageDisks *DiskImage::GetDisks()
{
	if (!p_file) return NULL;
	return p_file->GetDisks();
}
/// 指定した位置のディスクを返す
DiskImageDisk *DiskImage::GetDisk(size_t index)
{
	if (!p_file) return NULL;
	return p_file->GetDisk(index);
}
/// 指定した位置のディスクを返す
const DiskImageDisk *DiskImage::GetDisk(size_t index) const
{
	if (!p_file) return NULL;
	return p_file->GetDisk(index);
}
/// 指定した位置のディスクのタイプ
int DiskImage::GetDiskTypeNumber(size_t index) const
{
	if (!p_file) return -1;
	DiskImageDisk *disk = p_file->GetDisk(index);
	if (!disk) return -1;
	return gDiskTemplates.IndexOf(disk->GetDiskTypeName());
}

/// ファイル名を返す
wxString DiskImage::GetFileName() const
{
	return m_filename.GetFullName();
}

/// ファイル拡張子を返す
wxString DiskImage::GetFileExt() const
{
	return m_filename.GetExt();
}

/// ファイル名ベースを返す
wxString DiskImage::GetFileNameBase() const
{
	return m_filename.GetName();
}

/// ファイルパスを返す
wxString DiskImage::GetFilePath() const
{
	return m_filename.GetFullPath();
}

/// パスを返す
wxString DiskImage::GetPath() const
{
	return m_filename.GetPath();
}

/// ファイル名を設定
void DiskImage::SetFileName(const wxString &path)
{
	m_filename = wxFileName(path);
}

/// ファイル拡張子を設定
void DiskImage::SetFileExt(const wxString &ext)
{
	m_filename.SetExt(ext);
}

/// ファイルのフォーマット種類を返す
const wxString &DiskImage::GetFormatType() const
{
	return m_format_type;
}

/// ファイルのフォーマット種類を設定
void DiskImage::SetFormatType(const wxString &format_type)
{
	m_format_type = format_type;
}

/// DISK BASICが一致するか
bool DiskImage::MatchDiskBasic(const DiskBasic *target)
{
	bool match = false;
	DiskImageDisks *disks = GetDisks();
	if (!disks) return false;
	for(size_t i = 0; i < disks->Count(); i++) {
		DiskImageDisk *disk = disks->Item(i);
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
void DiskImage::ClearDiskBasicParseAndAssign(int disk_number, int side_number)
{
	DiskImageDisk *disk = GetDisk(disk_number);
	if (!disk) return;

	if (p_file) {
		p_file->SetBasicTypeHint(wxT(""));
	}

	DiskBasics *basics = disk->GetDiskBasics();
	if (!basics) return;
	basics->ClearParseAndAssign(side_number);
}

/// キャラクターコードマップ番号設定
void DiskImage::SetCharCode(const wxString &name)
{
	DiskImageDisks *disks = GetDisks();
	if (!disks) return;
	for(size_t i = 0; i < disks->Count(); i++) {
		DiskImageDisk *disk = disks->Item(i);
		disk->SetCharCode(name);
	}
}

/// エラーメッセージ
const wxArrayString &DiskImage::GetErrorMessage(int maxrow)
{
	return m_result.GetMessages(maxrow);
}

/// エラーメッセージを表示
void  DiskImage::ShowErrorMessage()
{
	ResultInfo::ShowMessage(m_result.GetValid(), m_result.GetMessages());
}

/// エラー警告メッセージを表示
int DiskImage::ShowErrWarnMessage()
{
	return ResultInfo::ShowErrWarnMessage(m_result.GetValid(), m_result.GetMessages(-1));
}
