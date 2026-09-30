/// @file basicfat.cpp
///
/// @brief disk basic fat
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicfat.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../diskimg/diskimage.h"


//////////////////////////////////////////////////////////////////////
//
// 使用状況テーブル
//
//////////////////////////////////////////////////////////////////////

DiskBasicAvailabillity::DiskBasicAvailabillity()
	: wxArrayInt()
{
	m_free_size = -1;
	m_free_grps = -1;
}

/// 初期化 空きサイズを 0 にする
void DiskBasicAvailabillity::Clear()
{
	wxArrayInt::Clear();
	m_free_size = 0;
	m_free_grps = 0;
}

/// 初期化 空きサイズを 0 にする
void DiskBasicAvailabillity::Empty()
{
	wxArrayInt::Empty();
	m_free_size = 0;
	m_free_grps = 0;
}

/// 初期化 空きサイズを -1 にする
void DiskBasicAvailabillity::EmptyInit()
{
	wxArrayInt::Empty();
	m_free_size = -1;
	m_free_grps = -1;
}

/// 追加
/// @param[in] val   値
/// @param[in] size  空きサイズ
/// @param[in] group 空きグループ数
void DiskBasicAvailabillity::Add(int val, int size, int group)
{
	wxArrayInt::Add(val);
	m_free_size += size;
	m_free_grps += group;
}

/// セット (Safety)
/// @param[in] idx 位置
/// @param[in] val 値
void DiskBasicAvailabillity::Set(size_t idx, int val)
{
	if (idx < Count()) {
		Item(idx) = val;
	}
}

/// ゲット (Safety)
/// @param[in] idx 位置
/// @return 値
int DiskBasicAvailabillity::Get(size_t idx) const
{
	int val = 0;
	if (idx < Count()) {
		val = Item(idx);
	}
	return val;
}

//////////////////////////////////////////////////////////////////////
//
// ビット ON/OFF バッファ １つ
//
//////////////////////////////////////////////////////////////////////

BitMLBuffer::BitMLBuffer()
{
	m_buffer = NULL;
	m_size = 0;
}
BitMLBuffer::BitMLBuffer(wxUint8 *buffer, size_t size)
{
	m_buffer = buffer;
	m_size = size;
}

/// 指定位置のビットを変更する
/// @param[in] num ビット位置
/// @param[in] val true:セット / false:リセット
void BitMLBuffer::Modify(wxUint32 num, bool val)
{
	wxUint32 pos = num >> 3;
	wxUint32 bit = num & 7;
	if (val) {
		m_buffer[pos] |= (0x80 >> bit);
	} else {
		m_buffer[pos] &= ~(0x80 >> bit);
	}
}

/// 指定位置のビットがセットされているか
/// @param[in] num ビット位置
/// @return    true:セット / false:リセット
bool BitMLBuffer::IsSet(wxUint32 num) const
{
	wxUint32 pos = num >> 3;
	wxUint32 bit = num & 7;
	return ((m_buffer[pos] & (0x80 >> bit)) != 0);
}

/// 指定位置のビット位置を計算
/// @param[in]  num ビット位置
/// @param[out] pos バッファ位置
/// @param[out] bit ビット
void BitMLBuffer::GetPos(wxUint32 num, wxUint32 &pos, wxUint32 &bit) const
{
	pos = num >> 3;
	bit = num & 7;
}

//

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfBitMLBuffer);

//

DiskBasicBitMLMap::DiskBasicBitMLMap()
	: ArrayOfBitMLBuffer()
{
}

/// ポインタをセット
void DiskBasicBitMLMap::AddBuffer(wxUint8 *buffer, size_t size)
{
	Add(BitMLBuffer(buffer, size));
}

/// 指定位置のビットを変更する
/// @param[in] group_num 位置
/// @param[in] val true:セット / false:リセット
void DiskBasicBitMLMap::Modify(wxUint32 group_num, bool val)
{
	for(size_t idx = 0; idx < Count(); idx++) {
		BitMLBuffer *item = &Item(idx);
		wxUint32 size = (wxUint32)item->GetBitSize();
		if (group_num < size) {
			item->Modify(group_num, val);
			break;
		}
		group_num -= size;
	}
}

/// 指定位置が空いているか
/// @param[in] group_num 位置
/// @return    true:セット / false:リセット
bool DiskBasicBitMLMap::IsSet(wxUint32 group_num) const
{
	bool val = false;
	for(size_t idx = 0; idx < Count(); idx++) {
		BitMLBuffer *item = &Item(idx);
		wxUint32 size = (wxUint32)item->GetBitSize();
		if (group_num < size) {
			val = item->IsSet(group_num);
			break;
		}
		group_num -= size;
	}
	return val;
}

/// 指定位置からバッファ内の位置を計算
/// @param[in]  group_num 位置
/// @param[out] idx MAPの位置
/// @param[out] pos MAP内のバッファ位置(byte)
/// @param[out] bit ビット位置
/// @return true / false オーバフロー
bool DiskBasicBitMLMap::GetPosInMap(wxUint32 group_num, size_t &idx, wxUint32 &pos, wxUint32 &bit) const
{
	bool valid = false;
	for(idx = 0; idx < Count(); idx++) {
		BitMLBuffer *item = &Item(idx);
		wxUint32 size = (wxUint32)item->GetBitSize();
		if (group_num < size) {
			item->GetPos(group_num, pos, bit);
			valid = true;
			break;
		}
		group_num -= size;
	}
	return valid;
}

//////////////////////////////////////////////////////////////////////
//
// FATエリア（セクタ）へのポインタを保持
//
//////////////////////////////////////////////////////////////////////

DiskBasicFatBuffer::DiskBasicFatBuffer()
{
	p_buffer = NULL;
	m_size = 0;
	m_accume_start = 0;
}
DiskBasicFatBuffer::DiskBasicFatBuffer(wxUint8 *buf, wxUint32 size, wxUint32 accume_start)
{
	p_buffer = buf;
	m_size = size;
	m_accume_start = accume_start;
}
DiskBasicFatBuffer::~DiskBasicFatBuffer()
{
}
/// バッファを指定コードで埋める
/// @param[in] code コード
void DiskBasicFatBuffer::Fill(wxUint8 code)
{
	if (p_buffer) {
		memset(p_buffer, code, m_size);
	}
}
/// バッファにコピー
/// @param[in] buf バッファ
/// @param[in] len サイズ
void DiskBasicFatBuffer::Copy(const wxUint8 *buf, size_t len)
{
	if (p_buffer) {
		len = len < (size_t)m_size ? len : m_size;
		memcpy(p_buffer, buf, len);
	}
}
/// 累計位置が範囲内か
/// @return 0:範囲内にある -1:範囲外(accume_posのほうが小さい) 1:範囲外(accume_posの方が大きい)
int DiskBasicFatBuffer::InRange(wxUint32 accume_pos) const
{
	if (accume_pos < m_accume_start) return -1;
	else if (m_accume_start + m_size <= accume_pos) return 1;
	else return 0;
}

/// 累計位置を範囲内の指定位置に変換
wxUint32 DiskBasicFatBuffer::ConvPos(wxUint32 accume_pos) const
{
	return accume_pos - m_accume_start;
}

/// 指定位置のデータを返す(8ビット)
/// @param[in] pos 位置(8ビット1単位)
/// @return 値
wxUint32 DiskBasicFatBuffer::Get(size_t pos) const
{
	return p_buffer ? p_buffer[pos] : INVALID_GROUP_NUMBER;
}
/// 指定位置にデータをセット(8ビット)
/// @param[in] pos 位置(8ビット1単位)
/// @param[in] val 値
void DiskBasicFatBuffer::Set(size_t pos, wxUint32 val)
{
	if (p_buffer) {
		p_buffer[pos] = (wxUint8)val;
	}
}
/// 指定位置のビットをセット/リセット
/// @param[in] pos    位置(8ビット1単位)
/// @param[in] mask   対象のビット
/// @param[in] val    セット/リセット
/// @param[in] invert 反転するか
/// @return 処理したか
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
/// @param[in] pos    位置(8ビット1単位)
/// @param[in] mask   対象のビット
/// @param[in] invert 反転するか
/// @return ビットがON
bool DiskBasicFatBuffer::BitTest(wxUint32 pos, wxUint8 mask, bool invert)
{
	if (pos >= (wxUint32)GetSize()) return false;

	wxUint8 bit = (wxUint8)Get(pos);
	if (invert) bit ^= 0xff;
	return (bit & mask) != 0;
}
/// 指定位置のデータを返す(16ビット、リトルエンディアン)
/// @param[in] pos    位置(8ビット1単位)
/// @return 値
wxUint32 DiskBasicFatBuffer::Get16LE(size_t pos) const
{
	return p_buffer ? ((wxUint32)p_buffer[pos] | p_buffer[pos+1] << 8) : INVALID_GROUP_NUMBER;
}
/// 指定位置にデータをセット(16ビット、リトルエンディアン)
/// @param[in] pos    位置(8ビット1単位)
/// @param[in] val    値
void DiskBasicFatBuffer::Set16LE(size_t pos, wxUint32 val)
{
	if (p_buffer) {
		p_buffer[pos]   = (val & 0xff);
		p_buffer[pos+1] = ((val >> 8) & 0xff);
	}
}
/// 指定位置のデータを返す(16ビット、ビッグエンディアン)
/// @param[in] pos    位置(8ビット1単位)
/// @return 値 
wxUint32 DiskBasicFatBuffer::Get16BE(size_t pos) const
{
	return p_buffer ? ((wxUint32)p_buffer[pos] << 8 | p_buffer[pos+1]) : INVALID_GROUP_NUMBER;
}
/// 指定位置にデータをセット(16ビット、ビッグエンディアン)
/// @param[in] pos    位置(8ビット1単位)
/// @param[in] val    値
void DiskBasicFatBuffer::Set16BE(size_t pos, wxUint32 val)
{
	if (p_buffer) {
		p_buffer[pos]   = ((val >> 8) & 0xff);
		p_buffer[pos+1] = (val & 0xff);
	}
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayDiskBasicFatBuffer);

//////////////////////////////////////////////////////////////////////
//
// FAT１つ分のバッファ（複数セクタあり）DiskBasicFatBuffer の配列
//
//////////////////////////////////////////////////////////////////////

DiskBasicFatBuffers::DiskBasicFatBuffers()
	: ArrayDiskBasicFatBuffer()
{
	m_size = 1;
}

/// コピーコンストラクタ
DiskBasicFatBuffers::DiskBasicFatBuffers(const DiskBasicFatBuffers &src)
	: ArrayDiskBasicFatBuffer(src)
{
	m_size = src.m_size;
}

/// 代入
DiskBasicFatBuffers &DiskBasicFatBuffers::operator=(const DiskBasicFatBuffers &src)
{
	ArrayDiskBasicFatBuffer::operator=(src);
	m_size = src.m_size;
	return *this;
}

/// 位置からセクタ位置を返す
/// @return -1:範囲外
bool DiskBasicFatBuffers::GetSectorIndex(wxUint32 &pos, size_t &idx) const
{
	idx = pos / m_size;
	int cmp = 0;
	while(idx < Count()) {
		cmp = Item(idx).InRange(pos);
		if (cmp == 0) {
			pos = Item(idx).ConvPos(pos);
			break;
		}
		idx += cmp;
	}
	return (idx < Count());
}

/// 8ビットデータを返す
/// @param[in] pos 位置(8ビット1単位)
/// @return 値
wxUint32 DiskBasicFatBuffers::GetData8(wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	size_t idx = 0;
#if 0
	if (GetSectorIndex(pos, idx)) {
		val = Item(idx).Get(pos);
	}
#else
	for(; idx < Count(); idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		if (pos < (wxUint32)buf->GetSize()) {
			val = buf->Get(pos);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
#endif
	return val;
}
/// 8ビットデータをセット
/// @param[in] pos 位置(8ビット1単位)
/// @param[in] val 値
void DiskBasicFatBuffers::SetData8(wxUint32 pos, wxUint32 val)
{
	size_t idx = 0;
#if 0
	if (GetSectorIndex(pos, idx)) {
		Item(idx).Set(pos, val);
	}
#else
	for(; idx < Count(); idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		if (pos < (wxUint32)buf->GetSize()) {
			buf->Set(pos, val);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
#endif
}
/// 8ビットデータが一致するか
/// @param[in] pos 位置(8ビット1単位)
/// @param[in] val 値
/// @return 一致する
bool DiskBasicFatBuffers::MatchData8(wxUint32 pos, wxUint32 val) const
{
	bool match = false;
	size_t idx = 0;
#if 0
	if (GetSectorIndex(pos, idx)) {
		match = (Item(idx).Get(pos) == val);
	}
#else
	for(; idx < Count(); idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		if (pos < (wxUint32)buf->GetSize()) {
			match = (buf->Get(pos) == val);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
#endif
	return match;
}
/// 8ビットデータのビットをセット/リセット
/// @param[in] pos    位置(8ビット1単位)
/// @param[in] mask   対象のビット
/// @param[in] val    セット/リセット
/// @param[in] invert 反転するか
/// @return 処理したか
bool DiskBasicFatBuffers::BitData8(wxUint32 pos, wxUint8 mask, bool val, bool invert)
{
	bool processed = false;
	size_t idx = 0;
#if 0
	if (GetSectorIndex(pos, idx)) {
		processed = Item(idx).Bit(pos, mask, val, invert);
	}
#else
	for(; idx < Count(); idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		processed = buf->Bit(pos, mask, val, invert);
		if (processed) break;
		pos -= (wxUint32)buf->GetSize();
	}
#endif
	return processed;
}
/// 12ビットデータ(リトルエンディアン)を返す
/// @param[in] pos 位置(12ビット1単位)
/// @return 値
wxUint32 DiskBasicFatBuffers::GetData12LE(wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	bool odd = ((pos & 1) != 0);
	pos = pos * 3 / 2;
	int cnt = 0;
	size_t idx = 0;
#if 0
	if (!GetSectorIndex(pos, idx)) {
		return val;
	}
#endif
	for (; idx < Count() && cnt < 2; idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		while (pos < (wxUint32)buf->GetSize() && cnt < 2) {
			wxUint32 tmp = buf->Get(pos);
			if (cnt == 0) {
				val = odd ? tmp >> 4 : tmp;
			}
			else {
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
/// @param[in] pos 位置(12ビット1単位)
/// @param[in] val 値
void DiskBasicFatBuffers::SetData12LE(wxUint32 pos, wxUint32 val)
{
	bool odd = ((pos & 1) != 0);
	pos = pos * 3 / 2;
	int cnt = 0;
	size_t idx = 0;
#if 0
	if (!GetSectorIndex(pos, idx)) {
		return;
	}
#endif
	for (; idx < Count() && cnt < 2; idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		while (pos < (wxUint32)buf->GetSize() && cnt < 2) {
			wxUint32 tmp = buf->Get(pos);
			if (cnt == 0) {
				tmp = odd ? ((val & 0x0f) << 4) | (tmp & 0x0f) : (val & 0xff);
			}
			else {
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
/// @param[in] pos 位置(16ビット1単位)
/// @return 値
wxUint32 DiskBasicFatBuffers::GetData16LE(wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	pos *= 2;
	size_t idx = 0;
#if 0
	if (GetSectorIndex(pos, idx)) {
		val = Item(idx).Get16LE(pos);
	}
#else
	for(; idx < Count(); idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		if (pos < (wxUint32)buf->GetSize()) {
			val = buf->Get16LE(pos);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
#endif
	return val;
}
/// 16ビットデータ(リトルエンディアン)をセット
/// @param[in] pos 位置(16ビット1単位)
/// @param[in] val 値
void DiskBasicFatBuffers::SetData16LE(wxUint32 pos, wxUint32 val)
{
	pos *= 2;
	size_t idx = 0;
#if 0
	if (GetSectorIndex(pos, idx)) {
		Item(idx).Set16LE(pos, val);
	}
#else
	for(; idx < Count(); idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		if (pos < (wxUint32)buf->GetSize()) {
			buf->Set16LE(pos, val);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
#endif
}

/// 16ビットデータ(ビッグエンディアン)を返す
/// @param[in] pos 位置(16ビット1単位)
/// @return 値
wxUint32 DiskBasicFatBuffers::GetData16BE(wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	pos *= 2;
	size_t idx = 0;
#if 0
	if (GetSectorIndex(pos, idx)) {
		val = Item(idx).Get16BE(pos);
	}
#else
	for(; idx < Count(); idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		if (pos < (wxUint32)buf->GetSize()) {
			val = buf->Get16BE(pos);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
#endif
	return val;
}
/// 16ビットデータ(ビッグエンディアン)をセット
/// @param[in] pos 位置(16ビット1単位)
/// @param[in] val 値
void DiskBasicFatBuffers::SetData16BE(wxUint32 pos, wxUint32 val)
{
	pos *= 2;
	size_t idx = 0;
#if 0
	if (GetSectorIndex(pos, idx)) {
		Item(idx).Set16BE(pos, val);
	}
#else
	for(; idx < Count(); idx++) {
		DiskBasicFatBuffer *buf = &Item(idx);
		if (pos < (wxUint32)buf->GetSize()) {
			buf->Set16BE(pos, val);
			break;
		}
		pos -= (wxUint32)buf->GetSize();
	}
#endif
}

WX_DEFINE_OBJARRAY(ArrayArrayDiskBasicFatBuffer);

//////////////////////////////////////////////////////////////////////
//
// FATバッファ（ミラーリング含む） DiskBasicFatBuffers の配列
//
//////////////////////////////////////////////////////////////////////

DiskBasicFatArea::DiskBasicFatArea()
	: ArrayArrayDiskBasicFatBuffer()
{
	m_valid_count = Count();
}
/// コピーコンストラクタ
DiskBasicFatArea::DiskBasicFatArea(const DiskBasicFatArea &src)
	: ArrayArrayDiskBasicFatBuffer(src)
{
	m_valid_count = src.m_valid_count;
}
/// 代入
DiskBasicFatArea &DiskBasicFatArea::operator=(const DiskBasicFatArea &src)
{
	ArrayArrayDiskBasicFatBuffer::operator=(src);
	m_valid_count = src.m_valid_count;
	return *this;
}
/// クリア
void DiskBasicFatArea::Empty()
{
	ArrayArrayDiskBasicFatBuffer::Empty();
	m_valid_count = 0;
}
/// 追加
/// @param[in] lItem   追加するアイテム
/// @param[in] nInsert 追加する数
void DiskBasicFatArea::Add(const DiskBasicFatBuffers &lItem, size_t nInsert)
{
	ArrayArrayDiskBasicFatBuffer::Add(lItem, nInsert);
	m_valid_count = Count();
}
/// 8ビットデータを返す
/// @param[in] idx    ミラーリング位置
/// @param[in] pos    位置(8ビット1単位)
/// @return 値
wxUint32 DiskBasicFatArea::GetData8(size_t idx, wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	if (idx >= Count()) return val;

	DiskBasicFatBuffers *bufs = &Item(idx);
	val = bufs->GetData8(pos);
	return val;
}
/// 8ビットデータをセット
/// @param[in] pos 位置(8ビット1単位)
/// @param[in] val 値
void DiskBasicFatArea::SetData8(wxUint32 pos, wxUint32 val)
{
	for(size_t n = 0; n < GetValidCount(); n++) {
		SetData8(n, pos, val);
	}
}
/// 8ビットデータをセット
/// @param[in] idx ミラーリング位置
/// @param[in] pos 位置(8ビット1単位)
/// @param[in] val 値
void DiskBasicFatArea::SetData8(size_t idx, wxUint32 pos, wxUint32 val)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->SetData8(pos, val);
}
/// 8ビットデータが一致するか
/// @param[in] pos 位置(8ビット1単位)
/// @param[in] val 値
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
/// @param[in] idx ミラーリング位置
/// @param[in] pos 位置(8ビット1単位)
/// @param[in] val 値
/// @return 一致した
bool DiskBasicFatArea::MatchData8(size_t idx, wxUint32 pos, wxUint32 val) const
{
	if (idx >= Count()) return false;

	DiskBasicFatBuffers *bufs = &Item(idx);
	return bufs->MatchData8(pos, val);
}
/// 8ビットデータのビットをセット/リセット
/// @param[in] idx    ミラーリング位置
/// @param[in] pos    位置(8ビット1単位)
/// @param[in] mask   対象のビット
/// @param[in] val    セット/リセット
/// @param[in] invert 反転するか
void DiskBasicFatArea::BitData8(size_t idx, wxUint32 pos, wxUint8 mask, bool val, bool invert)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->BitData8(pos, mask, val, invert);
}
/// 12ビットデータ(リトルエンディアン)を返す
/// @param[in] idx ミラーリング位置
/// @param[in] pos 位置(12ビット1単位)
/// @return 値
wxUint32 DiskBasicFatArea::GetData12LE(size_t idx, wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	if (idx >= Count()) return val;

	DiskBasicFatBuffers *bufs = &Item(idx);
	val = bufs->GetData12LE(pos);
	return val;
}
/// 12ビットデータ(リトルエンディアン)をセット
/// @param[in] pos 位置(12ビット1単位)
/// @param[in] val 値
void DiskBasicFatArea::SetData12LE(wxUint32 pos, wxUint32 val)
{
	for(size_t n = 0; n < GetValidCount(); n++) {
		SetData12LE(n, pos, val);
	}
}
/// 12ビットデータ(リトルエンディアン)をセット
/// @param[in] idx ミラーリング位置
/// @param[in] pos 位置(12ビット1単位)
/// @param[in] val 値
void DiskBasicFatArea::SetData12LE(size_t idx, wxUint32 pos, wxUint32 val)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->SetData12LE(pos, val);
}
/// 16ビットデータ(リトルエンディアン)を返す
/// @param[in] idx ミラーリング位置
/// @param[in] pos 位置(16ビット1単位)
/// @return 値
wxUint32 DiskBasicFatArea::GetData16LE(size_t idx, wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	if (idx >= Count()) return val;

	DiskBasicFatBuffers *bufs = &Item(idx);
	val = bufs->GetData16LE(pos);
	return val;
}
/// 16ビットデータ(リトルエンディアン)をセット
/// @param[in] pos 位置(16ビット1単位)
/// @param[in] val 値
void DiskBasicFatArea::SetData16LE(wxUint32 pos, wxUint32 val)
{
	for(size_t n = 0; n < GetValidCount(); n++) {
		SetData16LE(n, pos, val);
	}
}
/// 16ビットデータ(リトルエンディアン)をセット
/// @param[in] idx ミラーリング位置
/// @param[in] pos 位置(16ビット1単位)
/// @param[in] val 値
void DiskBasicFatArea::SetData16LE(size_t idx, wxUint32 pos, wxUint32 val)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->SetData16LE(pos, val);
}
/// 16ビットデータ(ビッグエンディアン)を返す
/// @param[in] idx ミラーリング位置
/// @param[in] pos 位置(16ビット1単位)
/// @return 値
wxUint32 DiskBasicFatArea::GetData16BE(size_t idx, wxUint32 pos) const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	if (idx >= Count()) return val;

	DiskBasicFatBuffers *bufs = &Item(idx);
	val = bufs->GetData16BE(pos);
	return val;
}
/// 16ビットデータ(ビッグエンディアン)をセット
/// @param[in] pos 位置(16ビット1単位)
/// @param[in] val 値
void DiskBasicFatArea::SetData16BE(wxUint32 pos, wxUint32 val)
{
	for(size_t n = 0; n < GetValidCount(); n++) {
		SetData16BE(n, pos, val);
	}
}
/// 16ビットデータ(ビッグエンディアン)をセット
/// @param[in] idx ミラーリング位置
/// @param[in] pos 位置(16ビット1単位)
/// @param[in] val 値
void DiskBasicFatArea::SetData16BE(size_t idx, wxUint32 pos, wxUint32 val)
{
	if (idx >= Count()) return;

	DiskBasicFatBuffers *bufs = &Item(idx);
	bufs->SetData16BE(pos, val);
}

//////////////////////////////////////////////////////////////////////
//
// FATアクセス
//
//////////////////////////////////////////////////////////////////////

DiskBasicFat::DiskBasicFat(DiskBasic *basic)
{
	p_basic = basic;
	p_type = NULL;
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

	int sector_num = p_basic->GetFatStartSector();
	int side_num = p_basic->GetReversedSideNumber(p_basic->GetFatSideNumber());

	m_bufs.Empty();

	p_type = p_basic->GetType();

	if (sector_num >= 0) {
		DiskImageTrack *managed_track;
		if (side_num >= 0) {
			// トラック、サイド番号から計算
			managed_track = p_basic->GetTrack(p_basic->GetManagedTrackNumber(), side_num);
		} else {
			// セクタ番号の通し番号で計算
			managed_track = p_basic->GetManagedTrack(p_basic->GetReservedSectors(), &side_num, &sector_num);
		}
		if (!managed_track) {
			return -1.0;
		}
		// セクタ位置を得る
		m_start = p_type->GetSectorPosFromNum(p_basic->GetManagedTrackNumber(), side_num, sector_num, 0, 1);

		m_count =		p_basic->GetNumberOfFats();
		m_size =		p_basic->GetSectorsPerFat();
		m_start_pos =	p_basic->GetFatStartPos();
		m_vcount =		p_basic->GetValidNumberOfFats();
		if (m_vcount < 0) {
			m_vcount = m_count;
		}

		p_type->CalcManagedStartGroup();

		// set buffer pointer for useful accessing
		int start_sector = m_start;
		int end_sector = m_start + m_size - 1;
		for(int fat_num = 0; fat_num < m_count && valid_ratio >= 0.0; fat_num++) {
			DiskBasicFatBuffers fatbufs;
			int totalpos = 0;
			for(int sec_num = start_sector; sec_num <= end_sector; sec_num++) {
				int div_num = 0;
				int div_nums = 1;
				DiskImageSector *sector = p_basic->GetSectorFromSectorPos(sec_num, &div_num, &div_nums);
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
					buf += m_start_pos;
					ssize -= m_start_pos;
				}
				DiskBasicFatBuffer fatbuf(buf, ssize, totalpos);
				fatbufs.Add(fatbuf);
				totalpos += ssize;
				if (ssize > fatbufs.GetSize()) {
					fatbufs.SetSize(ssize);
				}
			}
			m_bufs.Add(fatbufs);

			start_sector += m_size;
			end_sector += m_size;
		}

		m_bufs.SetValidCount(m_vcount);
	}

	if (valid_ratio >= 0.0) {
		valid_ratio = p_type->CheckFat(is_formatting);
	}

	return valid_ratio;
}
/// FATエリアのアサインを解除
void DiskBasicFat::Clear()
{
	m_count = 0;
	m_vcount = 0;
	m_size = 0;
	m_start = 0;
	m_start_pos = 0;

	m_bufs.Clear();
}
/// FATエリアのアサインを解除
void DiskBasicFat::Empty()
{
	Clear();
}

/// FAT領域の最初のセクタの指定位置のデータを取得
/// @param[in] pos  位置
wxUint8 DiskBasicFat::Get(int pos) const
{
	wxUint8 code = 0;
	DiskImageSector *sector = p_basic->GetSectorFromSectorPos(m_start);
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
/// @param[in] pos  位置
/// @param[in] code コード
void DiskBasicFat::Set(int pos, wxUint8 code)
{
	int start_sector = m_start;
	for(int fat_num = 0; fat_num < m_vcount; fat_num++) {
		DiskImageSector *sector = p_basic->GetSectorFromSectorPos(start_sector);
		if (sector) {
			wxUint8 *buf = sector->GetSectorBuffer();
			int size = sector->GetSectorBufferSize();
			if (buf && pos < size) {
				buf[pos] = code;
			}
		}
		start_sector += m_size;
	}
}

/// FAT領域の最初のセクタにデータを書く
/// @param[in] buf バッファ
/// @param[in] len サイズ
void DiskBasicFat::Copy(const wxUint8 *buf, size_t len)
{
	int start_sector = m_start;
	for(int fat_num = 0; fat_num < m_vcount; fat_num++) {
		DiskImageSector *sector = p_basic->GetSectorFromSectorPos(start_sector);
		if (sector) {
			sector->Copy(buf, (int)len);
		}
		start_sector += m_size;
	}
}

/// FAT領域を指定コードで埋める
/// @param[in] code コード
void DiskBasicFat::Fill(wxUint8 code)
{
	int start_sector = m_start;
	int end_sector = m_start + m_size - 1;
	for(int fat_num = 0; fat_num < m_count; fat_num++) {
		for(int sec_num = start_sector; sec_num <= end_sector; sec_num++) {
			DiskImageSector *sector = p_basic->GetSectorFromSectorPos(sec_num);
			if (sector) {
				sector->Fill(code);
			}
		}
		start_sector += m_size;
		end_sector += m_size;
	}
}

/// FATバッファを返す
/// @param[in] idx ミラーリングしているときのインデックス
DiskBasicFatBuffers *DiskBasicFat::GetDiskBasicFatBuffers(size_t idx)
{
	if (idx >= m_bufs.Count()) {
		return NULL;
	}
	return &m_bufs.Item(idx);
}

/// FATバッファ（セクタ）を返す
/// @param[in] idx    ミラーリングしているときのインデックス
/// @param[in] subidx バッファ位置
DiskBasicFatBuffer *DiskBasicFat::GetDiskBasicFatBuffer(size_t idx, size_t subidx)
{
	DiskBasicFatBuffers *fatbufs = GetDiskBasicFatBuffers(idx);
	if (!fatbufs || subidx >= fatbufs->Count()) {
		return NULL;
	}
	return &fatbufs->Item(subidx);
}
