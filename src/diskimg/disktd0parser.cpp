/// @file disktd0parser.cpp
///
/// @brief Teledisk td0ディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "disktd0parser.h"
#include <wx/mstream.h>
#include "diskimage.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"


#pragma pack(1)
/// Teledisk td0 ディスクイメージヘッダ
typedef struct st_td0_image_header {
	wxUint8  ident[2];
	wxUint8  sequence;
	wxUint8  check_sequence;
	wxUint8  teledisk_version;
	wxUint8  data_rate;
	wxUint8  dive_type;
	wxUint8  stepping;
	wxUint8  dos_alloc_flag;
	wxUint8  sides_per_disk;
	wxUint16 crc;
} td0_image_header_t;

/// Teledisk td0 コメントヘッダ
typedef struct st_td0_comment_header {
	wxUint16 crc;
	wxUint16 data_length;
	wxUint8  year;	// since 1900
	wxUint8  month;
	wxUint8  day;
	wxUint8  hour;
	wxUint8  minute;
	wxUint8  second;
//	wxUint8  comment[1];	// data_length
} td0_comment_header_t;

/// Teledisk td0 トラックヘッダ
typedef struct st_td0_track_header {
	wxUint8  num_of_sectors;
	wxUint8  track_num;
	wxUint8  side_num;
	wxUint8  crc;
} td0_track_header_t;

/// Teledisk td0 セクタヘッダ
typedef struct st_td0_sector_header {
	wxUint8  track_num;
	wxUint8  side_num;
	wxUint8  sector_num;
	wxUint8  sector_size;
	wxUint8  flags;
	wxUint8  crc;
} td0_sector_header_t;

/// Teledisk td0 データヘッダ
typedef struct st_td0_data_header {
	wxUint16 size;
//	wxUint8  method;
//	wxUint8  data[1];
} td0_data_header_t;

#pragma pack()

//
//
//
DiskTD0Parser::DiskTD0Parser(DiskImageFile *file, short mod_flags, DiskResult *result)
{
	p_file = file;
	m_mod_flags = mod_flags;
	p_result = result;
	m_is_compressed = false;
}

DiskTD0Parser::~DiskTD0Parser()
{
}

/// 繰り返しデータを展開
int DiskTD0Parser::DecodeRepeatedData(wxInputStream &istream, int disk_number, int pos, int slen, int repeat, wxUint8 *buffer, int buflen)
{
	wxCharBuffer ptn(slen);

	size_t len = istream.Read(ptn.data(), ptn.length()).LastRead();
	if (len == 0) {
		return pos;
	}
	for(int j=0; j < repeat && pos < buflen; j++) {
		for(int i=0; i < slen && pos < buflen; i++) {
			buffer[pos] = ptn[(size_t)i];
			pos++;
		}
	}
	return pos;
}

/// ベタデータを展開
int DiskTD0Parser::DecodePlainData(wxInputStream &istream, int disk_number, int pos, int slen, wxUint8 *buffer, int buflen)
{
	for(int i=0; i < slen && pos < buflen && !istream.Eof(); i++) {
		buffer[pos] = (wxUint8)istream.GetC();
		pos++;
	}
	return pos;
}

/// データを展開してバッファに書き込む
int DiskTD0Parser::DecodeData(wxInputStream &istream, int disk_number, wxUint8 *buffer, int buflen)
{
	td0_data_header_t h_data;
	size_t len = istream.Read(&h_data, sizeof(h_data)).LastRead();
	if (len != sizeof(h_data)) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
		return 0;
	}

	// データブロックを先読みする
	wxMemoryBuffer data(h_data.size);
	len = istream.Read(data.GetData(), h_data.size).LastRead();
	if (len < h_data.size) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
		return 0;
	}
	data.SetDataLen(len);
	data.AppendByte(0);

	wxMemoryInputStream itemp(data.GetData(), data.GetDataLen());

	int pos = 0;
	int method = itemp.GetC();
	if (method == 0) {
		// ベタ
		pos = DecodePlainData(itemp, disk_number, pos, h_data.size, buffer, buflen);
	} else if (method == 1) {
		// 繰り返しデータ
		int repeat = itemp.GetC();
		repeat |= (itemp.GetC() << 8);
		pos = DecodeRepeatedData(itemp, disk_number, pos, 2, repeat, buffer, buflen);
	} else if (method == 2) {
		do {
			int sub = itemp.GetC();
			if (sub == 0) {
				// ベタデータ
				int slen = itemp.GetC();
				pos = DecodePlainData(itemp, disk_number, pos, slen, buffer, buflen);
			} else {
				// 繰り返しデータ
				int slen = sub * 2;
				int repeat = itemp.GetC();
				pos = DecodeRepeatedData(itemp, disk_number, pos, slen, repeat, buffer, buflen);
			}
		} while(pos < buflen);
	}
	return pos;
}

/// セクタデータの作成
wxUint32 DiskTD0Parser::ParseSector(wxInputStream &istream, int disk_number, int sector_nums, void *user_data, DiskImageTrack *track)
{
	td0_sector_header_t h_sector;
	size_t len = istream.Read(&h_sector, sizeof(h_sector)).LastRead();
	if (len != sizeof(h_sector)) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
		return 0;
	}

	int track_number = h_sector.track_num;
	int side_number = h_sector.side_num;
	int sector_number = h_sector.sector_num;
	int sector_size = h_sector.sector_size;

	if (sector_size > 7) {
		// セクタサイズが大きすぎる
		p_result->SetError(DiskResult::ERRV_SECTOR_SIZE_SECTOR, disk_number, track_number, side_number, sector_number, sector_size, 0);
		return 0;
	}

	sector_size = (128 << sector_size);

	// セクタ作成
	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, sector_nums, false);
	track->Add(sector);

	if (h_sector.flags & 0x04) {
		// deleted mark
		sector->SetDeletedMark(true);
	}

	wxUint8 *buffer = sector->GetSectorBuffer();
	int buflen = sector->GetSectorBufferSize();

	DecodeData(istream, disk_number, buffer, buflen);

	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sector->GetSize();
}

/// トラックデータの作成
int DiskTD0Parser::ParseTrack(wxInputStream &istream, int disk_number, int offset_pos, wxUint32 offset, DiskImageDisk *disk)
{
	td0_track_header_t h_track;
	size_t len = istream.Read(&h_track, sizeof(h_track)).LastRead();
	if (len != sizeof(h_track)) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
		return -1;
	}
	if (h_track.num_of_sectors == 0xff) {
		return -1;
	}

	// トラックの作成
	DiskImageTrack *track = disk->NewImageTrack(h_track.track_num, h_track.side_num, offset_pos, 1);
	disk->SetMaxTrackNumber(h_track.track_num);

	wxUint32 d88_track_size = 0;
	for(int pos = 0; pos < h_track.num_of_sectors && p_result->GetValid() >= 0; pos++) {
		d88_track_size += ParseSector(istream, disk_number, h_track.num_of_sectors, NULL, track);
	}

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


/// TD0ファイルを解析
/// @param [in] istream     解析対象データ
/// @param [in] disk_number ディスク番号
/// @retval -1: finish parsing
/// @retval  0: parse next disk
int DiskTD0Parser::ParseDisk(wxInputStream &istream, int disk_number)
{
	td0_image_header_t h_image;
	size_t len = istream.Read(&h_image, sizeof(h_image)).LastRead();
	if (len == 0) {
		// no disk 解析終り
		return -1;
	}
	if (len < sizeof(h_image)) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
		return p_result->GetValid();
	}

	// コメントヘッダ
	bool has_comment = ((h_image.stepping & 0x80) != 0);
	if (has_comment) {
		td0_comment_header_t h_comment;
		len = istream.Read(&h_comment, sizeof(h_comment)).LastRead();
		if (len != sizeof(h_comment)) {
			p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
			return p_result->GetValid();
		}

		int comment_length = wxUINT16_SWAP_ON_BE(h_comment.data_length);
		istream.SeekI(comment_length, wxFromCurrent);
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

int DiskTD0Parser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return -1;
}

/// チェック
/// @param [in] istream       解析対象データ
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskTD0Parser::Check(wxInputStream &istream)
{
	istream.SeekI(0);

	td0_image_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return -1;
	}
	if (memcmp(header.ident, "TD", 2) != 0) {
		// not TD0 image
		// note that "td" (lower) which advanced compress version is not supported.
		return -1;
	}
	if (header.teledisk_version != 0x15) {
		// support only 1.5 version
		return -1;
	}

	int sides_per_disk = (int)(header.sides_per_disk);
	if (sides_per_disk > 2) {
		return -1;
	}

	return 0;
}

/// TD0ファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param パラメータ通常不要
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskTD0Parser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	istream.SeekI(0);
	for(int disk_number = 0; ; disk_number++) {
		if (ParseDisk(istream, disk_number) < 0) {
			break;
		}
	}
	return p_result->GetValid();
}
