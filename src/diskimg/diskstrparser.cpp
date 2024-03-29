/// @file diskstrparser.cpp
///
/// @brief X68000/PC9801用DSKSTR ディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskstrparser.h"
#include <wx/mstream.h>
#include "diskimage.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"
#include "../logging.h"


//
#pragma pack(1)
/// DSKSTRヘッダ
typedef struct st_str_header {
	wxUint32 datasize;	// BE
	wxUint8  reserved[28];
} str_header_t;

/// DSKSTRトラックヘッダ
typedef struct st_str_track_header {
	wxUint8  attr;
	wxUint8  secs;
	wxUint16 off1;	// BE
	wxUint16 off2;
	wxUint16 offd;
	wxUint16 dat1;
	wxUint16 dat2;
	wxUint8  reserved[4];
} str_track_header_t;

/// セクタID
typedef	struct st_str_sector_id {
		wxUint8 c;
		wxUint8 h;
		wxUint8 r;
		wxUint8 n;
} str_sector_id_t;
#pragma pack()

//
//
//
Expand2FIFOBuffer::Expand2FIFOBuffer()
	: Utils::FIFOBuffer()
{
	m_last_pos = 0;
	memset(m_istr_pos, 0, sizeof(m_istr_pos));
	memset(m_estr_pos, 0, sizeof(m_estr_pos));
}

void Expand2FIFOBuffer::Clear()
{
	Utils::FIFOBuffer::Clear();
	m_last_pos = 0;
	memset(m_istr_pos, 0, sizeof(m_istr_pos));
	memset(m_estr_pos, 0, sizeof(m_estr_pos));
}

//
//
//
DiskSTRParser::DiskSTRParser(DiskImageFile *file, short mod_flags, DiskResult *result)
	: DiskImageParser(file, mod_flags, result)
{
	m_compress_type = 0;
}

DiskSTRParser::~DiskSTRParser()
{
}

/// セクタデータの作成
/// @param [in] istream         ディスクイメージ
/// @param [in] disk_number     ディスク番号
/// @param [in] track_number    トラック番号
/// @param [in] side_number     サイド番号
/// @param [in] sector_nums     セクタ数
/// @param [in] sector_number   セクタ番号
/// @param [in] sector_size     セクタサイズ
/// @param [in] single_density  単密度か
/// @param [in,out] track       トラック
/// @return ヘッダ込みのセクタサイズ
wxUint32 DiskSTRParser::ParseSector(wxInputStream &istream, int disk_number, int track_number, int side_number, int sector_nums, int sector_number, int sector_size, bool single_density, DiskImageTrack *track)
{
	// セクタ作成
	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, sector_nums, false);
	track->Add(sector);

	wxUint8 *buffer = sector->GetSectorBuffer();

	// plain data
	istream.Read(buffer, sector_size);

	sector->SetSingleDensity(single_density);
	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sector->GetSize();
}

/// トラックデータの作成
/// @param [in] istream         ディスクイメージ
/// @param [in] disk_number     ディスク番号
/// @param [in] offset_pos      オフセット番号
/// @param [in] offset          オフセット位置
/// @param [in,out] disk        ディスク
/// @return -1:エラー or 終り >0:トラックサイズ
int DiskSTRParser::ParseTrack(wxInputStream &istream, int disk_number, int offset_pos, wxUint32 offset, DiskImageDisk *disk)
{
	str_track_header_t h_track;

	// 圧縮データを展開
	wxMemoryOutputStream oestream;
	size_t oelimit = sizeof(h_track);
	int rc = ExpandFirst(istream, oestream, oelimit);

	// ヘッダをチェック
	size_t len = 0;
	{
		wxMemoryInputStream iestream(oestream);
		len = iestream.Read(&h_track, sizeof(h_track)).LastRead();
	}
	if (rc < 0 || len < sizeof(h_track) || h_track.attr == 0) {
		// end of file
		return -1;
	}

	int sectors_per_track = h_track.secs;
	if (sectors_per_track <= 0) {
		p_result->SetError(DiskResult::ERRV_DISK_HEADER, disk_number);
		return -1;
	}

	// データ開始位置
//	oelimit += ((sectors_per_track + 3) / 4) * 4 + sectors_per_track * 4;
	oelimit = wxUINT16_SWAP_ON_LE(h_track.offd);
	// 圧縮データを展開つづき
	rc = ExpandNext(istream, oestream, oelimit);

	wxUint8 attr[256];
	memset(attr, 0, sizeof(attr));

	str_sector_id_t id[256];

	{
		wxMemoryInputStream iestream(oestream);

		// セクタ属性を得る
		iestream.SeekI(wxUINT16_SWAP_ON_LE(h_track.off1));

		for(int sec = 0; sec < sectors_per_track; sec += 4) {
			// 4バイト境界
			len = iestream.Read(&attr[sec], 4).LastRead();
			if (len < 4) {
				p_result->SetError(DiskResult::ERRV_DISK_HEADER, disk_number);
				return -1;
			}
		}

		// セクタIDを得る
		iestream.SeekI(wxUINT16_SWAP_ON_LE(h_track.off2));

		for(int sec = 0; sec < sectors_per_track; sec++) {
			// C H R N
			len = iestream.Read(&id[sec], sizeof(str_sector_id_t)).LastRead();
			if (len < sizeof(str_sector_id_t)) {
				p_result->SetError(DiskResult::ERRV_DISK_HEADER, disk_number);
				return -1;
			}

			int sector_size = (128 << id[sec].n);
			if (id[sec].n > 5) {
				p_result->SetError(DiskResult::ERRV_SECTOR_SIZE_SECTOR, disk_number, id[sec].c, id[sec].h, id[sec].r, id[sec].n, sector_size);
				return -1;
			}
//#ifdef _DEBUG
//			myLog.SetInfo("[C:%d H:%d R:%d N:%d]", (int)id[sec].c, (int)id[sec].h, (int)id[sec].r, (int)id[sec].n);
//#endif
			oelimit += (size_t)sector_size;
		}
	}

//#ifdef _DEBUG
//	{
//		int ii = (int)istream.TellI();
//		int ee = (int)oelimit;
//		myLog.SetInfo("TellI:%d(%x) Size:%d(%x) Total:%d(%x)", ii, ii, ee, ee, ii+ee, ii+ee);
//	}
//#endif

	// 圧縮データを展開つづき
	rc = ExpandNext(istream, oestream, oelimit);

	DiskImageTrack *track = NULL;
	wxUint32 d88_track_size = 0;
	{
		wxMemoryInputStream iestream(oestream);
		iestream.SeekI(wxUINT16_SWAP_ON_LE(h_track.offd));

		// トラックの作成
		track = disk->NewImageTrack(id[0].c, id[0].h, offset_pos, 1);
		disk->SetMaxTrackNumber(id[0].c);

		for(int pos = 0; pos < sectors_per_track && p_result->GetValid() >= 0; pos++) {
			int sector_size = (128 << id[pos].n);
			d88_track_size += ParseSector(iestream, disk_number, id[pos].c, id[pos].h, sectors_per_track, id[pos].r, sector_size, (attr[pos] & 0x40) == 0, track);
		}

	}

	// 入力データの位置を補正
	AdjustIStream(istream);

	if (p_result->GetValid() >= 0) {
		// インターリーブの計算
		track->CalcInterleave();
	}

	if (p_result->GetValid() >= 0) {
		// トラックサイズ設定
		track->SetSize(d88_track_size);
		// サイド番号は各セクタのID Hに合わせる
		track->SetSideNumber(track->GetMajorIDH());

		// ディスクに追加
		disk->Add(track);
		// オフセット設定
		disk->SetOffset(offset_pos, offset);
	} else {
		delete track;
	}

	return (int)d88_track_size;
}


/// ファイルを解析
/// @param [in] istream     解析対象データ
/// @param [in] disk_number ディスク番号
/// @retval -1: finish parsing
/// @retval  0: parse next disk
int DiskSTRParser::ParseDisk(wxInputStream &istream, int disk_number)
{
	// skip header
	if (ParseHeader(istream, disk_number) < 0) {
		return -1;
	}

	// ディスク作成
	DiskImageDisk *disk = p_file->NewImageDisk(disk_number);

	// トラック解析
	wxUint32 d88_offset = disk->GetOffsetStart();	// header size
	int d88_offset_pos = 0;
	for(int pos = 0; pos < 204; pos++) {
		int offset = ParseTrack(istream, disk_number, d88_offset_pos, d88_offset, disk);
		if (offset == -1) {
			break;
		}
		d88_offset += offset;

		d88_offset_pos++;
		if (d88_offset_pos >= disk->GetCreatableTracks()) {
			p_result->SetError(DiskResult::ERRV_OVERFLOW_SIZE, disk_number, d88_offset);
		}
	}
	disk->SetSize(d88_offset);

	if (p_result->GetValid() >= 0) {
		// ディスクを追加
		const DiskParam *disk_param = disk->CalcMajorNumber();
		if (disk_param) {
			disk->SetDensity(disk_param->GetParamDensity());
		}
		p_file->Add(disk, m_mod_flags);
	} else {
		delete disk;
	}

	return 0;
}

/// ヘッダ解析
/// @param [in] istream     解析対象データ
/// @param [in] disk_number ディスク番号
/// @retval -1: エラー
/// @retval  0:
int DiskSTRParser::ParseHeader(wxInputStream &istream, int disk_number)
{
	wxUint8 buf[16];
	size_t len = 1;
	do {
		len = istream.Read(buf, sizeof(buf)).LastRead();
		if (len < sizeof(buf)) {
			// too short
			return -1;
		}
		for(size_t i=0; i<len; i++) {
			if (buf[i] == 0x1a) {
				len = 0;
				break;
			}
		}
	} while(len > 0);

//	size_t start = istream.TellI();

	len = istream.Read(buf, sizeof(buf)).LastRead();
	if (len < sizeof(buf)) {
		// too short
		return -1;
	}
	if (memcmp(buf, "DSKSTR ver", 10) != 0) {
		// not a image
		return -1;
	}

	str_header_t header;
	len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return -1;
	}

//	size_t list_offset = wxUINT32_SWAP_ON_LE(header.datasize) + start;

	return 0;
}

/// 入力ストリームの位置を補正する
/// @param[in,out] istream 元データ
void DiskSTRParser::AdjustIStream(wxInputStream &istream)
{
	if ((m_compress_type & 2) != 0) {
		// 2次圧縮の場合、入力データを読み過ぎている場合があるので位置を補正する
		int match = -1;
		for(int i=0; i<8; i++) {
			if (m_estream.GetEStreamPos(i) < m_estream.GetReadPos() && m_estream.GetReadPos() <= m_estream.GetEStreamPos(i+1)) {
				match = i;
				break;
			}
		}
		if (match >= 0) {
//#ifdef _DEBUG
//			int ii = (int)estream.GetIStreamPos(match + 1);
//			myLog.SetInfo("Adjust:%d(%x)", ii, ii);
//#endif
			istream.SeekI(m_estream.GetIStreamPos(match + 1));
		}
	}
}

/// 圧縮データを判定して展開
/// @param[in]  istream 元データ
/// @param[out] ostream 展開後データ
/// @param[in]  olimit  出力バッファサイズ
/// @return -1:no data
int DiskSTRParser::ExpandFirst(wxInputStream &istream, wxOutputStream &ostream, size_t olimit)
{
	// データなし？
	if (istream.Eof()) {
		return -1;
	}

	// 最初のデータ
	m_compress_type = 0;
//	size_t pos = istream.TellI();
	char ch = istream.Peek();
	if (ch == 0x08 || ch == 0x0c) {
		m_compress_type = 1;
	} else if (ch == -1) {
		m_compress_type = 2;
	}

	m_estream.Clear();
	if (m_compress_type == 2) {
		// 2次圧縮データを展開
		Expand2(istream, ostream, olimit, true);
	} else if (m_compress_type == 1) {
		// 1次圧縮データを展開
		Expand1(istream, ostream, olimit);
	} else {
		// 非圧縮データ
		Expand0(istream, ostream, olimit);
	}
	return 0;
}

/// 圧縮データを展開つづき
/// @param[in]  istream 元データ
/// @param[out] ostream 展開後データ
/// @param[in]  olimit  出力バッファサイズ
int DiskSTRParser::ExpandNext(wxInputStream &istream, wxOutputStream &ostream, size_t olimit)
{
	if (m_compress_type & 2) {
		// 2次圧縮データを展開
		Expand2(istream, ostream, olimit);
	} else if (m_compress_type & 1) {
		// 1次圧縮データとみなす
		Expand1(istream, ostream, olimit);
	} else {
		// 非圧縮データ
		Expand0(istream, ostream, olimit);
	}
	return 0;
}

/// 2次圧縮データを展開
/// @param[in]  istream 元データ
/// @param[out] ostream 展開後データ
/// @param[in]  olimit  出力バッファサイズ
/// @param[in]  first   最初か
void DiskSTRParser::Expand2(wxInputStream &istream, wxOutputStream &ostream, size_t olimit, bool first)
{
	bool cont;
	do {
		Expand2Element(istream);
		if (first) {
			// 1次圧縮しているか
			int ch = m_estream.PeekByte();
			if (ch == 0x08 || ch == 0x0c) {
				m_compress_type |= 1;
			}
			first = false;
		}
		if (m_compress_type & 1) {
			// 1次圧縮データを展開
			cont = Expand1Element(ostream, olimit);
		} else {
			// 非圧縮データ
			cont = Expand0Element(ostream, olimit);
		}
	} while (cont);
}

/// 2次圧縮データを展開
/// @note estreamを入力ストリームとする
/// @param[in]  istream 元データ
void DiskSTRParser::Expand2Element(wxInputStream &istream)
{
	wxUint8 ibuf[16];
	int ibuf_len;
	wxUint8 buf[256];

	int ch = istream.GetC();
//#ifdef _DEBUG
//	int pos = (int)istream.TellI();
//	myLog.SetInfo("E2: %08x:%02x", pos - 1, ch);
//#endif
	if (ch == wxEOF) return;

	size_t ipos = istream.TellI();
	m_estream.SetIStreamPos(0, ipos);
	m_estream.SetEStreamPos(0, m_estream.GetWritePos());

	int cmd = (ch & 0xff);

	ibuf_len = 0;
	for(int i=0; i<8; i++) {
		if (cmd & 1) {
			ibuf_len++;
			m_estream.SetLastPos(ipos + ibuf_len);
		} else {
			ibuf_len += 2;
		}
		m_estream.SetIStreamPos(i+1, ipos + ibuf_len);
		cmd >>= 1;
	}

	memset(ibuf, 0, sizeof(ibuf));
	istream.Read(ibuf, ibuf_len).LastRead();

	cmd = (ch & 0xff);
	ibuf_len = 0;
	for(int i=0; i<8; i++) {
		if (cmd & 1) {
			// そのまま出力
			m_estream.AppendByte(ibuf[ibuf_len++]);
			m_estream.SetEStreamPos(i+1, m_estream.GetWritePos());
		} else {
			int d1 = ibuf[ibuf_len++];
			int d2 = ibuf[ibuf_len++];

			int len = (d2 & 0xf) + 3;
			int idx = ((d2 & 0xf0) << 4) | d1;
			idx += 18;

			// コピー元となるデータ位置を計算
			int elen = (int)m_estream.GetWritePos();
			if (elen >= 0x2000) {
				idx += (elen & ~0xfff) - 0x1000;
			} else if (elen < idx) {
				idx -= 0x1000;
			}

			// idx: 展開後データの絶対位置となる
			if (idx >= 0) {
				// ポジション補正
				if (!(elen <= idx + 0x1000 && idx < elen)) {
					idx += 0x1000;
				}

				// 元データを取得
				wxUint8 *bp = (wxUint8 *)m_estream.GetData() + idx;
				int blen = (idx + len > elen ? elen - idx : len);
				memcpy(buf, bp, blen);
				int bpos = blen;
				while (bpos < len) {
					// データを埋め合わせる
					if (bpos + blen > len) blen = len - bpos;
					memcpy(&buf[bpos], buf, blen); 
					bpos += blen;
				}
			} else {
				// 負になる場合は仮想的な位置で計算
				int nlen = -idx;
				if (nlen > len) nlen = len;
				if (nlen > 0) {
					memset(buf, 0, nlen);
				}
				int plen = (len + idx);
				if (plen > 0) {
					wxUint8 *bp = (wxUint8 *)m_estream.GetData();
					memcpy(&buf[nlen], bp, plen);
				}
			}

			// 展開データに追記
			m_estream.AppendData(buf, len);
			m_estream.SetEStreamPos(i+1, m_estream.GetWritePos());
		}
		cmd >>= 1;
	}
}

/// 1次圧縮データを展開
/// @param[in]  istream 元データ
/// @param[out] ostream 展開後データ
/// @param[in]  olimit  出力バッファサイズ
void DiskSTRParser::Expand1(wxInputStream &istream, wxOutputStream &ostream, size_t olimit)
{
	wxUint8 buf[16];

	bool cont = (m_estream.Remain() == 0);
	do {
		if (cont) {
			size_t len = istream.Read(buf, sizeof(buf)).LastRead();
			if (len > 0) {
				m_estream.AppendData(buf, len);
			}
		}
		cont = Expand1Element(ostream, olimit);
	} while (cont);

	if (m_estream.Remain() > 0) {
		istream.SeekI(-(wxFileOffset)m_estream.Remain(), wxFromCurrent);
		m_estream.SetWritePos(m_estream.GetReadPos());
	}
}

/// 1次圧縮データを展開
/// @note estreamを入力ストリームとする
/// @param[out] ostream 展開後データ
/// @param[in]  olimit  出力バッファサイズ
/// @return 出力データサイズがolimitに達したらfalse
bool DiskSTRParser::Expand1Element(wxOutputStream &ostream, size_t olimit)
{
	size_t siz;
	size_t osize = ostream.GetLength();
	wxUint8 buf[128];

	do {
		// 先頭文字チェック
		int ch = m_estream.PeekByte();
		if (ch == -1) {
			break;
		}
		siz = (ch & 0xff);
		if (ch < 0x80) {
			if (siz == 0) siz = 0x80;
		} else {
			siz = 1;
		}
		if (m_estream.Remain() < (siz + 1)) {
			break;
		}

		// 展開
		ch = m_estream.GetByte();
		siz = (ch & 0xff);
		if (ch < 0x80) {
			if (siz == 0) siz = 0x80;
			siz = m_estream.GetData(buf, siz);
			osize += ostream.Write(buf, siz).LastWrite();
		} else {
			siz = (ch & 0x7f);
			ch = m_estream.GetByte();
			if (siz == 0) siz = 0x80;
			memset(buf, ch, siz);
			osize += ostream.Write(buf, siz).LastWrite();
		}
	} while (osize < olimit);

	return (osize < olimit);
}

/// 非圧縮データをそのまま展開
/// @param[in]  istream 元データ
/// @param[out] ostream 展開後データ
/// @param[in]  olimit  出力バッファサイズ
void DiskSTRParser::Expand0(wxInputStream &istream, wxOutputStream &ostream, size_t olimit)
{
	size_t siz;
	size_t osize = ostream.GetLength();
	wxUint8 buf[128];

	while(osize < olimit) {
		siz = (osize + sizeof(buf) < olimit ? sizeof(buf) : olimit - osize);

		siz = istream.Read(buf, siz).LastRead();
		if (siz == 0) {
			break;
		}
		osize += ostream.Write(buf, siz).LastWrite();
	}
}

/// 非圧縮データをそのまま展開
/// @note estreamを入力ストリームとする
/// @param[out] ostream 展開後データ
/// @param[in]  olimit  出力バッファサイズ
/// @return 出力データサイズがolimitに達したらfalse
bool DiskSTRParser::Expand0Element(wxOutputStream &ostream, size_t olimit)
{
	size_t siz;
	size_t osize = ostream.GetLength();
	wxUint8 buf[128];

	while(osize < olimit) {
		siz = (osize + sizeof(buf) < olimit ? sizeof(buf) : olimit - osize);

		siz = m_estream.GetData(buf, siz);
		if (siz == 0) {
			break;
		}
		osize += ostream.Write(buf, siz).LastWrite();
	}
	return (osize < olimit);
}

int DiskSTRParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return -1;
}

/// チェック
/// @param [in] istream       解析対象データ
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskSTRParser::Check(wxInputStream &istream)
{
	istream.SeekI(0);

	if (ParseHeader(istream, 0) < 0) {
		return -1;
	}

	return 0;
}

/// ファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param パラメータ通常不要
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskSTRParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	istream.SeekI(0);

	for(int disk_number = 0; disk_number < 1; disk_number++) {
		if (ParseDisk(istream, disk_number) < 0) {
			break;
		}
	}
	return p_result->GetValid();
}
