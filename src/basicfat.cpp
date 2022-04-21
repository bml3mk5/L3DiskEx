/// @file basicfat.cpp
///
/// @brief disk basic fat
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicfat.h"
#include "basicfmt.h"
#include "basictype.h"


//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicAvailabillity::DiskBasicAvailabillity()
	: wxArrayInt()
{
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicFatBuffer::DiskBasicFatBuffer()
{
	size = 0;
	buffer = NULL;
}
DiskBasicFatBuffer::DiskBasicFatBuffer(wxUint8 *newbuf, int newsize)
{
	size = newsize;
	buffer = newbuf;
}
/// バッファを指定コードで埋める
void DiskBasicFatBuffer::Fill(wxUint8 code)
{
	if (buffer) {
		memset(buffer, code, size);
	}
}
/// バッファにコピー
void DiskBasicFatBuffer::Copy(const wxUint8 *buf, size_t len)
{
	if (buffer) {
		memcpy(buffer, buf, len < size ? len : size);
	}
}
/// 指定位置のデータを返す
wxUint32 DiskBasicFatBuffer::Get(size_t pos) const
{
	return buffer ? buffer[pos] : INVALID_GROUP_NUMBER;
}
/// 指定位置にデータをセット
void DiskBasicFatBuffer::Set(size_t pos, wxUint32 val)
{
	if (buffer) {
		buffer[pos] = (wxUint8)val;
	}
}
/// 指定位置のビットをセット/リセット
bool DiskBasicFatBuffer::Bit(wxUint32 pos, wxUint8 mask, bool val, bool invert)
{
	if (pos >= (wxUint32)GetSize()) return false;

	wxUint8 bit = (wxUint8)Get(pos);
	if (invert) bit ^= 0xff;
	bit = (val ? (bit | mask) : (bit & ~mask));
	if (invert) bit ^= 0xff;
	Set(pos, bit);
	return true;
}
/// 指定位置のビットがONか
bool DiskBasicFatBuffer::BitTest(wxUint32 pos, wxUint8 mask, bool invert)
{
	if (pos >= (wxUint32)GetSize()) return false;

	wxUint8 bit = (wxUint8)Get(pos);
	if (invert) bit ^= 0xff;
	return (bit & mask) != 0;
}
/// 指定位置のデータを返す
wxUint32 DiskBasicFatBuffer::Get16LE(size_t pos) const
{
	return buffer ? ((wxUint32)buffer[pos] | buffer[pos+1] << 8) : INVALID_GROUP_NUMBER;
}
/// 指定位置にデータをセット
void DiskBasicFatBuffer::Set16LE(size_t pos, wxUint32 val)
{
	if (buffer) {
		buffer[pos]   = (val & 0xff);
		buffer[pos+1] = ((val >> 8) & 0xff);
	}
}
/// 指定位置のデータを返す
wxUint32 DiskBasicFatBuffer::Get16BE(size_t pos) const
{
	return buffer ? ((wxUint32)buffer[pos] << 8 | buffer[pos+1]) : INVALID_GROUP_NUMBER;
}
/// 指定位置にデータをセット
void DiskBasicFatBuffer::Set16BE(size_t pos, wxUint32 val)
{
	if (buffer) {
		buffer[pos]   = ((val >> 8) & 0xff);
		buffer[pos+1] = (val & 0xff);
	}
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayDiskBasicFatBuffer);

//////////////////////////////////////////////////////////////////////
//
//
//
/// 8ビットデータを返す
wxUint32 DiskBasicFatBuffers::GetData8(wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	for(size_t i = 0; i < Count(); i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		if (pos < (wxUint32)buf->GetSize()) {
			val = buf->Get(pos);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
	return val;
}
/// 8ビットデータをセット
void DiskBasicFatBuffers::SetData8(wxUint32 pos, wxUint32 val)
{
	for(size_t i = 0; i < Count(); i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		if (pos < (wxUint32)buf->GetSize()) {
			buf->Set(pos, val);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
}
/// 8ビットデータが一致するか
bool DiskBasicFatBuffers::MatchData8(wxUint32 pos, wxUint32 val) const
{
	bool match = false;
	for(size_t i = 0; i < Count(); i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		if (pos < (wxUint32)buf->GetSize()) {
			match = (buf->Get(pos) == val);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
	return match;
}
/// 8ビットデータのビットをセット/リセット
/// @return 処理したか
bool DiskBasicFatBuffers::BitData8(wxUint32 pos, wxUint8 mask, bool val, bool invert)
{
	bool processed = false;
	for(size_t i = 0; i < Count(); i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		processed = buf->Bit(pos, mask, val, invert);
		if (processed) break;
		pos -= (wxUint32)buf->GetSize();
	}
	return processed;
}
/// 12ビットデータ(リトルエンディアン)を返す
wxUint32 DiskBasicFatBuffers::GetData12LE(wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	bool odd = ((pos & 1) != 0);
	pos = pos * 3 / 2;
	int cnt = 0;
	for(size_t i = 0; i < Count() && cnt < 2; i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		while (pos < (wxUint32)buf->GetSize() && cnt < 2) {
			wxUint32 tmp = buf->Get(pos);
			if (cnt == 0) {
				val = odd ? tmp >> 4 : tmp;
			} else {
				val |= odd ? tmp << 4 : (tmp & 0x0f) << 8;
			}
			pos++;
			cnt++;
		}
		pos -= (wxUint32)buf->GetSize();
	}
	if (cnt != 2) val = INVALID_GROUP_NUMBER;
	return val;
}
/// 12ビットデータ(リトルエンディアン)をセット
void DiskBasicFatBuffers::SetData12LE(wxUint32 pos, wxUint32 val)
{
	bool odd = ((pos & 1) != 0);
	pos = pos * 3 / 2;
	int cnt = 0;
	for(size_t i = 0; i < Count() && cnt < 2; i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		while (pos < (wxUint32)buf->GetSize() && cnt < 2) {
			wxUint32 tmp = buf->Get(pos);
			if (cnt == 0) {
				tmp = odd ? ((val & 0x0f) << 4) | (tmp & 0x0f) : (val & 0xff);
			} else {
				tmp = odd ? (val >> 4) & 0xff : ((val >> 8) & 0x0f) | (tmp & 0xf0);
			}
			buf->Set(pos, tmp);
			pos++;
			cnt++;
		}
		pos -= (wxUint32)buf->GetSize();
	}
}

/// 16ビットデータ(リトルエンディアン)を返す
wxUint32 DiskBasicFatBuffers::GetData16LE(wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	pos *= 2;
	for(size_t i = 0; i < Count(); i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		if (pos < (wxUint32)buf->GetSize()) {
			val = buf->Get16LE(pos);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
	return val;
}
/// 16ビットデータ(リトルエンディアン)をセット
void DiskBasicFatBuffers::SetData16LE(wxUint32 pos, wxUint32 val)
{
	pos *= 2;
	for(size_t i = 0; i < Count(); i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		if (pos < (wxUint32)buf->GetSize()) {
			buf->Set16LE(pos, val);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
}

/// 16ビットデータ(ビッグエンディアン)を返す
wxUint32 DiskBasicFatBuffers::GetData16BE(wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	pos *= 2;
	for(size_t i = 0; i < Count(); i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		if (pos < (wxUint32)buf->GetSize()) {
			val = buf->Get16BE(pos);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
	return val;
}
/// 16ビットデータ(ビッグエンディアン)をセット
void DiskBasicFatBuffers::SetData16BE(wxUint32 pos, wxUint32 val)
{
	pos *= 2;
	for(size_t i = 0; i < Count(); i++) {
		DiskBasicFatBuffer *buf = &Item(i);
		if (pos < (wxUint32)buf->GetSize()) {
			buf->Set16BE(pos, val);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
}

WX_DEFINE_OBJARRAY(ArrayArrayDiskBasicFatBuffer);

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicFatArea::DiskBasicFatArea()
	: ArrayArrayDiskBasicFatBuffer()
{
	valid_count = Count();
}
DiskBasicFatArea::DiskBasicFatArea(const DiskBasicFatArea &src)
	: ArrayArrayDiskBasicFatBuffer(src)
{
	valid_count = src.valid_count;
}
DiskBasicFatArea &DiskBasicFatArea::operator=(const DiskBasicFatArea &src)
{
	ArrayArrayDiskBasicFatBuffer::operator=(src);
	valid_count = src.valid_count;
	return *this;
}
void DiskBasicFatArea::Empty()
{
	ArrayArrayDiskBasicFatBuffer::Empty();
	valid_count = 0;
}
void DiskBasicFatArea::Add(const DiskBasicFatBuffers &lItem, size_t nInsert)
{
	ArrayArrayDiskBasicFatBuffer::Add(lItem, nInsert);
	valid_count = Count();
}
/// 8ビットデータを返す
wxUint32 DiskBasicFatArea::GetData8(size_t idx, wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	if (idx >= Count()) return val;

	DiskBasicFatBuffers *bufs = &Item(idx);
	val = bufs->GetData8(pos);
	return val;
}
/// 8ビットデータをセット
void DiskBasicFatArea::SetData8(wxUint32 pos, wxUint32 val)
{
	for(size_t n = 0; n < GetValidCount(); n++) {
		SetData8(n, pos, val);
	}
}
/// 8ビットデータをセット
void DiskBasicFatArea::SetData8(size_t idx, wxUint32 pos, wxUint32 val)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->SetData8(pos, val);
}
/// 8ビットデータが一致するか
/// @return 一致した数（多重分）
int DiskBasicFatArea::MatchData8(wxUint32 pos, wxUint32 val) const
{
	int match_count = 0;
	for(size_t n = 0; n < GetValidCount(); n++) {
		if (MatchData8(n, pos, val)) match_count++;
	}
	return match_count;
}
/// 8ビットデータが一致するか
bool DiskBasicFatArea::MatchData8(size_t idx, wxUint32 pos, wxUint32 val) const
{
	if (idx >= Count()) return false;

	DiskBasicFatBuffers *bufs = &Item(idx);
	return bufs->MatchData8(pos, val);
}
/// 8ビットデータのビットをセット/リセット
void DiskBasicFatArea::BitData8(size_t idx, wxUint32 pos, wxUint8 mask, bool val, bool invert)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->BitData8(pos, mask, val, invert);
}
/// 12ビットデータ(リトルエンディアン)を返す
wxUint32 DiskBasicFatArea::GetData12LE(size_t idx, wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	if (idx >= Count()) return val;

	DiskBasicFatBuffers *bufs = &Item(idx);
	val = bufs->GetData12LE(pos);
	return val;
}
/// 12ビットデータ(リトルエンディアン)をセット
void DiskBasicFatArea::SetData12LE(wxUint32 pos, wxUint32 val)
{
	for(size_t n = 0; n < GetValidCount(); n++) {
		SetData12LE(n, pos, val);
	}
}
/// 12ビットデータ(リトルエンディアン)をセット
void DiskBasicFatArea::SetData12LE(size_t idx, wxUint32 pos, wxUint32 val)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->SetData12LE(pos, val);
}
/// 16ビットデータ(リトルエンディアン)を返す
wxUint32 DiskBasicFatArea::GetData16LE(size_t idx, wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	if (idx >= Count()) return val;

	DiskBasicFatBuffers *bufs = &Item(idx);
	val = bufs->GetData16LE(pos);
	return val;
}
/// 16ビットデータ(リトルエンディアン)をセット
void DiskBasicFatArea::SetData16LE(wxUint32 pos, wxUint32 val)
{
	for(size_t n = 0; n < GetValidCount(); n++) {
		SetData16LE(n, pos, val);
	}
}
/// 16ビットデータ(リトルエンディアン)をセット
void DiskBasicFatArea::SetData16LE(size_t idx, wxUint32 pos, wxUint32 val)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->SetData16LE(pos, val);
}
/// 16ビットデータ(ビッグエンディアン)を返す
wxUint32 DiskBasicFatArea::GetData16BE(size_t idx, wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	if (idx >= Count()) return val;

	DiskBasicFatBuffers *bufs = &Item(idx);
	val = bufs->GetData16BE(pos);
	return val;
}
/// 16ビットデータ(ビッグエンディアン)をセット
void DiskBasicFatArea::SetData16BE(wxUint32 pos, wxUint32 val)
{
	for(size_t n = 0; n < GetValidCount(); n++) {
		SetData16BE(n, pos, val);
	}
}
/// 16ビットデータ(ビッグエンディアン)をセット
void DiskBasicFatArea::SetData16BE(size_t idx, wxUint32 pos, wxUint32 val)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->SetData16BE(pos, val);
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicFat::DiskBasicFat(DiskBasic *basic)
{
	this->basic = basic;
	type = NULL;
	Clear();
}
DiskBasicFat::~DiskBasicFat()
{
}
/// FATエリアをアサイン
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0  正常
/// @retval <1.0 警告あり
/// @retval <0.0 エラーあり
double DiskBasicFat::Assign(bool is_formatting)
{
	double valid_ratio = 1.0;

	int sector_num = basic->GetFatStartSector();
	int side_num = basic->GetReversedSideNumber(basic->GetFatSideNumber());

	bufs.Empty();

	type = basic->GetType();

	if (sector_num >= 0) {
		DiskD88Track *managed_track;
		if (side_num >= 0) {
			// トラック、サイド番号から計算
			managed_track = basic->GetTrack(basic->GetManagedTrackNumber(), side_num);
		} else {
			// セクタ番号の通し番号で計算
			managed_track = basic->GetManagedTrack(basic->GetReservedSectors(), &side_num, &sector_num);
		}
		if (!managed_track) {
			return -1.0;
		}
		// セクタ位置を得る
		start = type->GetSectorPosFromNum(basic->GetManagedTrackNumber(), side_num, sector_num, 0, 1);

		count =			basic->GetNumberOfFats();
		size =			basic->GetSectorsPerFat();
		start_pos =		basic->GetFatStartPos();
		vcount =		basic->GetValidNumberOfFats();
		if (vcount < 0) {
			vcount = count;
		}

		type->CalcManagedStartGroup();

		// set buffer pointer for useful accessing
		int start_sector = start;
		int end_sector = start + size - 1;
		for(int fat_num = 0; fat_num < count && valid_ratio >= 0.0; fat_num++) {
			DiskBasicFatBuffers fatbufs;
			for(int sec_num = start_sector; sec_num <= end_sector; sec_num++) {
				int div_num = 0;
				int div_nums = 1;
				DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec_num, &div_num, &div_nums);
				if (!sector) {
					valid_ratio = -1.0;
					break;
				}

				int ssize = sector->GetSectorSize();
				ssize /= div_nums;
				wxUint8 *buf = sector->GetSectorBuffer();
				buf += (ssize * div_num);

//				sector_size = ssize;
				if (sec_num == start_sector) {
					// 最初のセクタだけ開始位置がずれる
					buf += start_pos;
					ssize -= start_pos;
				}
				DiskBasicFatBuffer fatbuf(buf, ssize);
				fatbufs.Add(fatbuf);
			}
			bufs.Add(fatbufs);

			start_sector += size;
			end_sector += size;
		}

		bufs.SetValidCount(vcount);
	}

	if (valid_ratio >= 0.0) {
		valid_ratio = type->CheckFat(is_formatting);
	}

	return valid_ratio;
}
/// FATエリアのアサインを解除
void DiskBasicFat::Clear()
{
	count = 0;
	vcount = 0;
	size = 0;
	start = 0;
	start_pos = 0;

	bufs.Clear();
}
/// FATエリアのアサインを解除
void DiskBasicFat::Empty()
{
	Clear();
}

/// FAT領域の最初のセクタの指定位置のデータを取得
wxUint8 DiskBasicFat::Get(int pos) const
{
	wxUint8 code = 0;
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(start);
	if (sector) {
		wxUint8 *buf = sector->GetSectorBuffer();
		int size = sector->GetSectorBufferSize();
		if (buf && pos < size) {
			code = buf[pos];
		}
	}
	return code;
}

/// FAT領域の最初のセクタにデータを書く
void DiskBasicFat::Set(int pos, wxUint8 code)
{
	int start_sector = start;
	for(int fat_num = 0; fat_num < vcount; fat_num++) {
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(start_sector);
		if (sector) {
			wxUint8 *buf = sector->GetSectorBuffer();
			int size = sector->GetSectorBufferSize();
			if (buf && pos < size) {
				buf[pos] = code;
			}
		}
		start_sector += size;
	}
}

/// FAT領域の最初のセクタにデータを書く
void DiskBasicFat::Copy(const wxUint8 *buf, size_t len)
{
	int start_sector = start;
	for(int fat_num = 0; fat_num < vcount; fat_num++) {
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(start_sector);
		if (sector) {
			sector->Copy(buf, len);
		}
		start_sector += size;
	}
}

/// FAT領域を指定データで埋める
void DiskBasicFat::Fill(wxUint8 code)
{
	int start_sector = start;
	int end_sector = start + size - 1;
	for(int fat_num = 0; fat_num < count; fat_num++) {
		for(int sec_num = start_sector; sec_num <= end_sector; sec_num++) {
			DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec_num);
			if (sector) {
				sector->Fill(code);
			}
		}
		start_sector += size;
		end_sector += size;
	}
}

DiskBasicFatBuffers *DiskBasicFat::GetDiskBasicFatBuffers(size_t idx)
{
	if (idx >= bufs.Count()) {
		return NULL;
	}
	return &bufs.Item(idx);
}

DiskBasicFatBuffer *DiskBasicFat::GetDiskBasicFatBuffer(size_t idx, size_t subidx)
{
	DiskBasicFatBuffers *fatbufs = GetDiskBasicFatBuffers(idx);
	if (!fatbufs || subidx >= fatbufs->Count()) {
		return NULL;
	}
	return &fatbufs->Item(subidx);
}
