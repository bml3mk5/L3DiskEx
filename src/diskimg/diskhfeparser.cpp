/// @file diskhfeparser.cpp
///
/// @brief HxC HFEディスクパーサー
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskhfeparser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "fileparam.h"
#include "diskresult.h"

#define DISK_HFE_HEADER "HXCPICFE"
#define DISK_HFE_HEADV3 "HXCHFEV3"

#define ISOIBM_MFM_ENCODING		0x00
#define AMIGA_MFM_ENCODING		0x01
#define ISOIBM_FM_ENCODING		0x02
#define EMU_FM_ENCODING			0x03
#define UNKNOWN_ENCODING		0xFF

#pragma pack(1)
/// HxC HFE header (512bytes) (LE)
typedef struct st_hfe_header {
	char     signature[8];
	wxUint8  revision;				// always 0
	wxUint8  tracks;
	wxUint8  sides;
	wxUint8  encoding;
	wxUint16 bit_rate;
	wxUint16 rpm;

	wxUint8  interface_mode;
	wxUint8  dnu;
	wxUint16 track_list_offset;		// Offset of the track list LUT in block of 512bytes
	wxUint8  write_allowed;
	wxUint8  single_step;
	wxUint8  track0s0_encode_enable;
	wxUint8  track0s0_encode;
	wxUint8  track0s1_encode_enable;
	wxUint8  track0s1_encode;

	char     reserved[486];
} hfe_header_t;

/// HxC HFE track offset (LE)
typedef struct st_hfe_track_offset {
	wxUint16 offset;	// Offset of the track data in block of 512bytes
	wxUint16 track_len;
} hfe_track_offset_t;

/// HxC HFE track offset LUT (up to 1024bytes) (LE)
typedef struct st_hfe_track_offset_list {
	hfe_track_offset_t at[256];
} hfe_track_offset_list_t;
#pragma pack()

//
// Run-length limited(RLL)パーサ
//
RunLengthLimitedParser::RunLengthLimitedParser()
{
	disk = NULL;
	track = NULL;
	track_size = 0;
	data = NULL;
	data_len = 0;
	track_number = 0;
	side_number = 0;
	sector_nums = 0;
	d88_offset_pos = 0;
	result = NULL;
}
/// @param [in,out] n_disk         ディスク
/// @param [in]     n_track_number トラック番号
/// @param [in]     n_side_number  サイド番号
/// @param [in]     n_d88_offset_pos D88オフセット番号
/// @param [in,out] n_data         解析対象データ
/// @param [in]     n_data_len     データサイズ
/// @param [in,out] n_result       解析エラー情報
RunLengthLimitedParser::RunLengthLimitedParser(DiskImageDisk *n_disk, int n_track_number, int n_side_number, int n_d88_offset_pos, wxUint8 *n_data, int n_data_len, DiskResult *n_result)
{
	disk = n_disk;
	track = NULL;
	track_size = 0;
	data = n_data;
	data_len = n_data_len;
	track_number = n_track_number;
	side_number = n_side_number;
	sector_nums = 0;
	d88_offset_pos = n_d88_offset_pos;
	result = n_result;
}
RunLengthLimitedParser::~RunLengthLimitedParser()
{
}

/// データの解析
/// @return D88形式でのトラックサイズ
wxUint32 RunLengthLimitedParser::Parse()
{
	track = disk->NewImageTrack(track_number, side_number, d88_offset_pos, 1);
	track_size = 0;

	while(data_len > 0) {
		if (!AdjustGap()) {
			break;
		}
		if (!GetData()) {
			break;
		}
	}

	return track_size;
}

/// GAPをさがす
bool RunLengthLimitedParser::AdjustGap()
{
	return false;
}

/// データを得る
bool RunLengthLimitedParser::GetData()
{
	return false;
}

/// セクタデータをセット
/// @param [in,out] indata       解析対象データ
/// @param [in]     single       single sided?
/// @param [in]     deleted      Deleted mark?
/// @return セクタサイズ
int RunLengthLimitedParser::SetSectorData(wxUint8 *indata, bool single, bool deleted)
{
	int track_number = curr_ids.C;
	int side_number = curr_ids.H;
	int sector_number = curr_ids.R;
	int sector_size = curr_ids.N;

	if (sector_size > 7) {
		// セクタサイズが大きすぎる
		result->SetError(DiskResult::ERRV_SECTOR_SIZE_SECTOR, 0, track_number, side_number, sector_number, sector_size, sector_size);
		return 0;
	}

	sector_size = (128 << sector_size);

	sector_nums++;
	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, 1, false);
	track->Add(sector);

	wxUint8 *buf = sector->GetSectorBuffer();
	int siz = sector->GetSectorBufferSize();
	int unit = GetDecodeUnit();
	for(int i=0; i<siz; i++) {
		buf[i] = DecodeData(&indata[i * unit]);
	}

	sector->SetSingleDensity(single);
	sector->SetDeletedMark(deleted);
	sector->ClearModify();

	// このセクタデータのサイズを返す
	return sector->GetSize();
}
/// データをデコード
wxUint8 RunLengthLimitedParser::DecodeData(const wxUint8 *indata)
{
	return 0;
}

/// バッファをシフト
/// @param [in,out] data         データ
/// @param [in]     len          データ長さ
/// @param [in]     sftcnt       シフト数
/// @return シフトした後のデータ数
int RunLengthLimitedParser::ShiftBytes(wxUint8 *data, int len, int sftcnt)
{
	if (sftcnt <= 0) return len;

	int endpos = len - sftcnt;

	for(int i=0; i<endpos; i++) {
		data[i] = data[i + sftcnt];
	}
	data[endpos] = 0x00;

	len -= sftcnt;

	return len;
}

/// バッファをビットシフト
/// @param [in,out] data         データ
/// @param [in]     len          データ長さ
/// @param [in]     sftcnt       シフト数
/// @return シフトした後のデータ数
int RunLengthLimitedParser::ShiftBits(wxUint8 *data, int len, int sftcnt)
{
	if (sftcnt <= 0) return len;

	int divn = (sftcnt >> 3);
	int modn = (sftcnt & 7);

	// lshift bytes
	if (divn > 0) {
		len = ShiftBytes(data, len, divn);
	}

	if (modn == 0) return len;

	// bit shift
	wxUint8 carry = 0x00;
	for(int i=len-1; i>=0; i--) {
		wxUint8 c = (data[i] << (8-modn));
		data[i] >>= modn;
		data[i] |= carry;
		carry = c;
	}

	return len;
}

//
// IBM MFMパーサ
//
FormatMFMParser::FormatMFMParser()
	: RunLengthLimitedParser()
{
}
/// @param [in,out] n_disk         ディスク
/// @param [in]     n_track_number トラック番号
/// @param [in]     n_side_number  サイド番号
/// @param [in]     n_d88_offset_pos D88オフセット番号
/// @param [in,out] n_data         解析対象データ
/// @param [in]     n_data_len     データサイズ
/// @param [in,out] n_result       解析エラー情報
///
/// @note Bit stream order is:
/// first <- b0 <- b1 <- b2 <- ... <- b7 <- next byte b0 <- b1 ...
FormatMFMParser::FormatMFMParser(DiskImageDisk *n_disk, int n_track_number, int n_side_number, int n_d88_offset_pos, wxUint8 *n_data, int n_data_len, DiskResult *n_result)
	: RunLengthLimitedParser(n_disk, n_track_number, n_side_number, n_d88_offset_pos, n_data, n_data_len, n_result)
{
}
/** GAPをさがす(MFM)

@par GAP code
 4E ->  0 1 0 0  1 1 1 0 \n
 clk   1 0 0 1  0 0 0 0  \n
       10010010 01010100 -> 9245 \n
 rev   01001001 00101010 -> 492A \n

 @par SYNC code
 00 ->  0 0 0 0  0 0 0 0 \n
 clk   1 1 1 1  1 1 1 1  \n
       10101010 10101010 \n
 rev   01010101 01010101 -> 5555 \n
*/
/// @return GAP and SYNCフィールドあり
bool FormatMFMParser::AdjustGap()
{
	bool found = false;
	int maxlen = data_len;
	wxUint8 buf[6];
	int pos = 0;
	// search GAP field
	for(; pos<maxlen; pos++) {
		memcpy(buf, &data[pos], 3);
		int cnt = 0;
		for(; cnt<8; cnt++) {
			if (memcmp(buf, "\x49\x2a", 2) == 0) {
				found = true;
				break;
			}

			// bit shift left
			ShiftBits(buf, 3, 1);
		}
		if (found) {
			data_len = ShiftBits(data, data_len, pos * 8 + cnt);
			break;
		}
	}
	if (!found) {
		data_len = 0;
		return found;
	}
	// search the terminate of SYNC field
	found =false;
	pos = 0;
	for(; pos<maxlen; pos++) {
		memcpy(buf, &data[pos], 4);
		int cnt = 0;
		for(; cnt<8; cnt++) {
			if (memcmp(buf, "\x55\x55\x25", 3) == 0
			 || memcmp(buf, "\x55\x55\xa5", 3) == 0) {
				found = true;
				break;
			}

			// bit shift left
			ShiftBits(buf, 4, 1);
		}
		if (found) {
			if (cnt >= 4) {
				cnt -= 4;
			} else {
				pos--;
				cnt += 4;
			}
			if (pos >= 0) {
				data_len = ShiftBits(data, data_len, pos * 8 + cnt);
			}
			break;
		}
	}
	if (!found) {
		data_len = 0;
	}
	return found;
}
/** データの解析(MFM)

@par PRE AM
 A1 ->  1 0 1 0  0 0 0 1 \n
 clk   0 0 0 0  1 x 1 0  \n
       01000100 10001001 \n
 rev   00100010 10010001 -> 2291 \n

@par PRE IDX
 C2 ->  1 1 0 0  0 0 1 0 \n
 clk   0 0 0 1  x 1 0 0  \n
       01010010 00100100 \n
 rev   01001010 00100100 -> 4a24 \n

@par INDEX mark
 FC ->  1 1 1 1  1 1 0 0 \n
 clk   0 0 0 0  0 0 0 1  \n
       01010101 01010010 \n
 rev   10101010 01001010 -> aa4a \n

@par ID mark
 FE ->  1 1 1 1  1 1 1 0 \n
 clk   0 0 0 0  0 0 0 0  \n
       01010101 01010100 \n
 rev   10101010 00101010 -> aa2a \n

@par DATA mark
 FB ->  1 1 1 1  1 0 1 1 \n
 clk   0 0 0 0  0 0 0 0  \n
       01010101 01000101 \n
 rev   10101010 10100010 -> aaa2 \n

@par Deleted DATA mark
 F8 ->  1 1 1 1  1 0 0 0 \n
 clk   0 0 0 0  0 0 1 1  \n
       01010101 01001010 \n
 rev   10101010 01010010 -> aa52 \n
*/
/// @return AMフィールドあり
bool FormatMFMParser::GetData()
{
	bool found = false;
	int maxlen = data_len;
	int pos = 0;
	for(; pos<maxlen && !found; pos++) {
		if (memcmp(&data[pos], "\x55\x55\x4a\x24\x4a\x24\x4a\x24\xaa\x4a", 10) == 0) {
			// INDEX MARK
			found = true;
			data_len = ShiftBytes(data, data_len, pos + 10);
			break;
		} else if (memcmp(&data[pos], "\x55\x55\x22\x91\x22\x91\x22\x91\xaa\x2a", 10) == 0) {
			// ID MARK
			found = true;
			// Get C,H,R,N,CRC
			curr_ids.C = DecodeData(&data[pos+10]);
			curr_ids.H = DecodeData(&data[pos+12]);
			curr_ids.R = DecodeData(&data[pos+14]);
			curr_ids.N = DecodeData(&data[pos+16]);
			curr_ids.CRC = (wxUint16)DecodeData(&data[pos+18]) * 256 + DecodeData(&data[pos+20]);

			data_len = ShiftBytes(data, data_len, pos + 22);
			break;
		} else if (memcmp(&data[pos], "\x55\x55\x22\x91\x22\x91\x22\x91\xaa\xa2", 10) == 0) {
			// DATA MARK
			found = true;
			// Get Data
			int siz = SetSectorData(&data[pos+10], false, false);
			track_size += (wxUint32)siz;

			int unit = GetDecodeUnit();
			data_len = ShiftBytes(data, data_len, pos + ((siz + 2) * unit) + 10);
			break;
		} else if (memcmp(&data[pos], "\x55\x55\x22\x91\x22\x91\x22\x91\xaa\x52", 10) == 0) {
			// DELETED DATA MARK
			found = true;
			// Get Data
			int siz = SetSectorData(&data[pos+10], false, true);
			track_size += (wxUint32)siz;

			int unit = GetDecodeUnit();
			data_len = ShiftBytes(data, data_len, pos + ((siz + 2) * unit) + 10);
			break;
		}
	}
	if (!found) {
		data_len = 0;
	}
	return found;
}
/// データをデコード(MFM)
/// @param [in] data 解析対象データ(2bytes)
/// @return デコード後のデータ
wxUint8 FormatMFMParser::DecodeData(const wxUint8 *indata)
{
	wxUint8 outdata;
	outdata = ((indata[0] & 0x80) >> 3) | ((indata[0] & 0x20)) | ((indata[0] & 0x08) << 3) | ((indata[0] & 0x02) << 6);
	outdata |= ((indata[1] & 0x80) >> 7) | ((indata[1] & 0x20) >> 4) | ((indata[1] & 0x08) >> 1) | ((indata[1] & 0x02) << 2);

	return outdata;
}

//
// IBM FMパーサ
//
FormatFMParser::FormatFMParser()
	: RunLengthLimitedParser()
{
}
/// @param [in,out] n_disk         ディスク
/// @param [in]     n_track_number トラック番号
/// @param [in]     n_side_number  サイド番号
/// @param [in]     n_d88_offset_pos D88オフセット番号
/// @param [in,out] n_data         解析対象データ
/// @param [in]     n_data_len     データサイズ
/// @param [in,out] n_result       解析エラー情報
///
/// @note Bit stream order is:
/// first <- b0 <- b1 <- b2 <- ... <- b7 <- next byte b0 <- b1 ...
FormatFMParser::FormatFMParser(DiskImageDisk *n_disk, int n_track_number, int n_side_number, int n_d88_offset_pos, wxUint8 *n_data, int n_data_len, DiskResult *n_result)
	: RunLengthLimitedParser(n_disk, n_track_number, n_side_number, n_d88_offset_pos, n_data, n_data_len, n_result)
{
}
/** GAPをさがす(FM)

@par GAP code
 FF ->   01  01   01  01   01  01   01  01 \n
 clk   01  01   01  01   01  01   01  01   \n
       01010101 01010101 01010101 01010101 -> 55555555 \n
 rev   10101010 10101010 10101010 10101010 -> AAAAAAAA \n

 @par SYNC code
 00 ->   00  00   00  00   00  00   00  00 \n
 clk   01  01   01  01   01  01   01  01   \n
       01000100 01000100 01000100 01000100 \n
 rev   00100010 00100010 00100010 00100010 -> 22222222 \n
*/
/// @return GAP and SYNCフィールドあり
bool FormatFMParser::AdjustGap()
{
	bool found = false;
	int maxlen = data_len;
	wxUint8 buf[8];
	int pos = 0;
	// search GAP field
	for(; pos<maxlen; pos++) {
		memcpy(buf, &data[pos], 5);
		int cnt = 0;
		for(; cnt<8; cnt++) {
			if (memcmp(buf, "\xaa\xaa\xaa\xaa", 4) == 0) {
				found = true;
				break;
			}

			// bit shift left
			ShiftBits(buf, 5, 1);
		}
		if (found) {
			data_len = ShiftBits(data, data_len, pos * 8 + cnt);
			break;
		}
	}
	if (!found) {
		data_len = 0;
		return found;
	}
	// search the terminate of SYNC field
	found =false;
	pos = 0;
	for(; pos<maxlen; pos++) {
//		if (pos == 0xb3) {
//			int nn=0;
//		}
		memcpy(buf, &data[pos], 6);
		int cnt = 0;
		for(; cnt<8; cnt++) {
			if (memcmp(buf, "\x22\x22\x22\x22\xa2", 5) == 0) {
				found = true;
				break;
			}

			// bit shift left
			ShiftBits(buf, 6, 1);
		}
		if (found) {
			if (cnt >= 4) {
				cnt -= 4;
			} else {
				pos--;
				cnt += 4;
			}
			if (pos >= 0) {
				data_len = ShiftBits(data, data_len, pos * 8 + cnt);
			}
			break;
		}
	}
	if (!found) {
		data_len = 0;
	}
	return found;
}
/** データの解析(FM)

@par INDEX mark
 FC ->   01  01   01  01   01  01   00  00 \n
 clk   01  01   0x  01   0x  01   01  01   \n
       01010101 00010101 00010101 01000100 \n
 rev   10101010 10101000 10101000 00100010 -> aaa8a822 \n

@par ID mark
 FE ->   01  01   01  01   01  01   01  00 \n
 clk   01  01   0x  0x   0x  01   01  01   \n
       01010101 00010001 00010101 01010100 \n
 rev   10101010 10001000 10101000 00101010 -> aa88a82a \n

@par DATA mark
 FB ->   01  01   01  01   01  00   01  01 \n
 clk   01  01   0x  0x   0x  01   01  01   \n
       01010101 00010001 00010100 01010101 \n
 rev   10101010 10001000 00101000 10101010 -> aa8828aa \n

@par Deleted DATA mark
 F8 ->   01  01   01  01   01  00   00  00 \n
 clk   01  01   0x  0x   0x  01   01  01   \n
       01010101 00010001 00010100 01000100 \n
 rev   10101010 10001000 00101000 00100010 -> aa882822 \n
*/
/// @return AMフィールドあり
bool FormatFMParser::GetData()
{
	bool found = false;
	int maxlen = data_len;
	int pos = 0;
	for(; pos<maxlen && !found; pos++) {
		if (memcmp(&data[pos], "\x22\x22\x22\x22\xaa\xa8\xa8\x22", 8) == 0) {
			// INDEX MARK
			found = true;
			data_len = ShiftBytes(data, data_len, pos + 8);
			break;
		} else if (memcmp(&data[pos], "\x22\x22\x22\x22\xaa\x88\xa8\x2a", 8) == 0) {
			// ID MARK
			found = true;
			// Get C,H,R,N,CRC
			curr_ids.C = DecodeData(&data[pos+8]);
			curr_ids.H = DecodeData(&data[pos+12]);
			curr_ids.R = DecodeData(&data[pos+16]);
			curr_ids.N = DecodeData(&data[pos+20]);
			curr_ids.CRC = (wxUint16)DecodeData(&data[pos+24]) * 256 + DecodeData(&data[pos+28]);

			data_len = ShiftBytes(data, data_len, pos + 32);
			break;
		} else if (memcmp(&data[pos], "\x22\x22\x22\x22\xaa\x88\x28\xaa", 8) == 0) {
			// DATA MARK
			found = true;
			// Get Data
			int siz = SetSectorData(&data[pos+8], true, false);
			track_size += (wxUint32)siz;

			int unit = GetDecodeUnit();
			data_len = ShiftBytes(data, data_len, pos + ((siz + 2) * unit) + 8);
			break;
		} else if (memcmp(&data[pos], "\x22\x22\x22\x22\xaa\x88\x28\x22", 8) == 0) {
			// DELETED DATA MARK
			found = true;
			// Get Data
			int siz = SetSectorData(&data[pos+8], true, true);
			track_size += (wxUint32)siz;

			int unit = GetDecodeUnit();
			data_len = ShiftBytes(data, data_len, pos + ((siz + 2) * unit) + 8);
			break;
		}
	}
	if (!found) {
		data_len = 0;
	}
	return found;
}
/// データをデコード(FM)
/// @param [in] data 解析対象データ(4bytes)
/// @return デコード後のデータ
wxUint8 FormatFMParser::DecodeData(const wxUint8 *indata)
{
	wxUint8 outdata;
	outdata = ((indata[0] & 0x80) >> 1) | ((indata[0] & 0x08) << 4);
	outdata |= ((indata[1] & 0x80) >> 3) | ((indata[1] & 0x08) << 2);
	outdata |= ((indata[2] & 0x80) >> 5) | ((indata[2] & 0x08));
	outdata |= ((indata[3] & 0x80) >> 7) | ((indata[3] & 0x08) >> 2);

	return outdata;
}

//
// HxC HFE形式をD88形式にする
//
DiskHfeParser::DiskHfeParser(DiskImageFile *file, short mod_flags, DiskResult *result)
{
	p_file = file;
	m_mod_flags = mod_flags;
	p_result = result;
}

DiskHfeParser::~DiskHfeParser()
{
}

/// トラックデータの作成
/// @param [in,out] istream      解析対象データ
/// @param [in]     track_number トラック番号
/// @param [in]     sides        サイド数
/// @param [in]     file_offset  ファイルオフセット
/// @param [in]     track_size   トラックサイズ
/// @param [in]     encoding     エンコード形式
/// @param [in,out] d88_offset_pos D88オフセット番号
/// @param [in]     d88_offset   D88オフセット
/// @param [in,out] disk         ディスク
/// @return D88オフセット
wxUint32 DiskHfeParser::ParseTracks(wxInputStream &istream, int track_number, int sides, int file_offset, int track_size, wxUint8 encoding[2], int &d88_offset_pos, wxUint32 d88_offset, DiskImageDisk *disk)
{
	int track_blocks = (track_size / 512);

	wxUint8 *buffers[2];
	buffers[0] = new wxUint8[track_blocks * 256];
	buffers[1] = new wxUint8[track_blocks * 256];

	do {
		istream.SeekI(file_offset, wxFromStart);
		bool working = true;
		for(int block = 0; block < track_blocks && working; block++) {
			for(int side = 0; side < 2 && working; side++) {
				size_t len = istream.Read(&buffers[side][block * 256], 256).LastRead();
				if (len != 256) {
					p_result->SetError(DiskResult::ERR_NO_TRACK, 0);
					working = false;
					break;
				}
			}
		}

		for(int side = 0; side < sides; side++) {
			wxUint32 d88_track_size = 0;
			DiskImageTrack *track = NULL;
			int sector_nums = 0;

			switch(encoding[side]) {
			case ISOIBM_FM_ENCODING:
				{
					// parse FM
					FormatFMParser ps(disk, track_number, side, d88_offset_pos, buffers[side], track_blocks * 256, p_result); 
					d88_track_size = ps.Parse();
					track = ps.GetTrack();
					sector_nums = ps.GetSectorNums();
				}
				break;
			default:
				{
					// parse MFM
					FormatMFMParser ps(disk, track_number, side, d88_offset_pos, buffers[side], track_blocks * 256, p_result); 
					d88_track_size = ps.Parse();
					track = ps.GetTrack();
					sector_nums = ps.GetSectorNums();
				}
				break;
			}

			if (p_result->GetValid() >= 0) {
				// インターリーブの計算
				track->CalcInterleave();
				// トラックサイズ
				track->SetSize(d88_track_size);
				// セクタ数設定
				track->SetAllSectorsPerTrack(sector_nums);

				// ディスクに追加
				disk->Add(track);
				// オフセット設定
				disk->SetOffset(d88_offset_pos, d88_offset);

				d88_offset += d88_track_size;
			} else {
				delete track;
			}

			d88_offset_pos++;
		}
	} while(0);

	delete [] buffers[1];
	delete [] buffers[0];

	return d88_offset;
}

/// ディスクの解析
/// @param [in,out] istream    解析対象データ
/// @result サイズ
wxUint32 DiskHfeParser::ParseDisk(wxInputStream &istream)
{
	DiskImageDisk *disk = p_file->NewImageDisk(0);

	hfe_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len != sizeof(header)) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return 0;
	}

	int tracks = header.tracks;
	int sides = header.sides;

	wxUint32 d88_offset = disk->GetOffsetStart();	// header size
	int d88_offset_pos = 0;

	// track list
	int track_list_offset = wxUINT16_SWAP_ON_BE(header.track_list_offset) * 512;
	istream.SeekI(track_list_offset, wxFromStart);

	hfe_track_offset_list_t track_offset_list;
	len = istream.Read(&track_offset_list, sizeof(track_offset_list)).LastRead();
	if (len < sizeof(hfe_track_offset_t) * tracks) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return 0;
	}

	for(int track = 0; track < tracks; track++) {
		wxUint8 encoding[2];
		encoding[0] = header.encoding;
		encoding[1] = header.encoding;
		if (track == 0 && header.track0s0_encode_enable == 0) encoding[0] = header.track0s0_encode;
		if (track == 0 && header.track0s1_encode_enable == 0) encoding[1] = header.track0s1_encode;

		d88_offset = ParseTracks(istream
			, track, sides
			, wxUINT16_SWAP_ON_BE(track_offset_list.at[track].offset) * 512
			, wxUINT16_SWAP_ON_BE(track_offset_list.at[track].track_len)
			, encoding
			, d88_offset_pos, d88_offset, disk);

		if (d88_offset_pos >= disk->GetCreatableTracks()) {
			p_result->SetError(DiskResult::ERRV_OVERFLOW_SIZE, 0, d88_offset);
		}
	}
	// 最大トラック番号設定
	disk->SetMaxTrackNumber(tracks);

	disk->SetSize(d88_offset);

	if (p_result->GetValid() >= 0) {
		// ディスクを追加
		const DiskParam *disk_param = disk->CalcMajorNumber();
		if (disk_param) {
			disk->SetDensity(disk_param->GetParamDensity());
		}
		disk->SetWriteProtect(header.write_allowed == 0);
		disk->ClearModify();

		p_file->Add(disk, m_mod_flags);
	} else {
		delete disk;
	}

	return d88_offset;
}

int DiskHfeParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return -1;
}

static const char *hfe_type_msgs[] = {
	wxTRANSLATE("IBM MFM"), wxTRANSLATE("Amiga MFM"), wxTRANSLATE("IBM FM"), wxTRANSLATE("EMU FM"), wxTRANSLATE("unknown"), NULL

};

/// HxC HFEファイルかどうかをチェック
/// @param [in,out] istream    解析対象データ
/// @return 0:Ok -1:NG
int DiskHfeParser::Check(wxInputStream &istream)
{
	istream.SeekI(0);

	hfe_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return p_result->GetValid();
	}

	// check signature
	if ((memcmp(header.signature, DISK_HFE_HEADER, sizeof(header.signature)) != 0
	  && memcmp(header.signature, DISK_HFE_HEADV3, sizeof(header.signature)) != 0) || header.revision != 0) {
		p_result->SetError(DiskResult::ERRV_DISK_HEADER, 0);
		return p_result->GetValid();
	}

	// format
	if (header.encoding != ISOIBM_MFM_ENCODING && header.encoding != ISOIBM_FM_ENCODING) {
		wxString msg;
		if (header.encoding <= 3) {
			msg = wxString(hfe_type_msgs[header.encoding]);
		} else {
			msg = wxString(hfe_type_msgs[4]);
		}
		p_result->SetError(DiskResult::ERRV_UNSUPPORTED_TYPE, 0, msg.t_str());
		return p_result->GetValid();
	}

	// track list
	int track_list_offset = wxUINT16_SWAP_ON_BE(header.track_list_offset) * 512;

	hfe_track_offset_t track_offset;

	istream.SeekI(track_list_offset, wxFromStart);
	for(int track = 0; track < (int)header.tracks; track++) {
		len = istream.Read(&track_offset, sizeof(track_offset)).LastRead();
		if (len < sizeof(track_offset)) {
			// too short
			p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
			return p_result->GetValid();
		}
		if (wxUINT16_SWAP_ON_BE(track_offset.offset) == 0xffff) {
			// invalid
			p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
			return p_result->GetValid();
		}
	}

	return p_result->GetValid();
}

/// HxC HFEファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param パラメータ通常不要
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskHfeParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	ParseDisk(istream);
	return p_result->GetValid();
}
