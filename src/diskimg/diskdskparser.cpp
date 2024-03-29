/// @file diskdskparser.cpp
///
/// @brief CPC DSKディスクパーサー
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskdskparser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "fileparam.h"
#include "diskresult.h"


#pragma pack(1)
/// CPC DSK header
typedef struct st_cpc_dsk_header {
	wxUint8		ident[34];
	wxUint8		creator[14];
	wxUint8		num_of_tracks;
	wxUint8		num_of_sides;
	wxUint16	track_size;	// bytes	// use only in normal disk
	wxUint8		track_sizes[204];		// use only in extended disk 
} cpc_dsk_header_t;

/// CPC DSK sector
typedef struct st_cpc_dsk_sector {
	wxUint8		C;
	wxUint8		H;
	wxUint8		R;
	wxUint8		N;
	wxUint8		fdc_status_1;
	wxUint8		fdc_status_2;
	wxUint16	data_length;	// bytes // use only in extended disk 
} cpc_dsk_sector_t;

/// CPC DSK track
typedef struct st_cpc_dsk_track {
	wxUint8		ident[12];
	wxUint8		unused1[4];
	wxUint8		track_number;
	wxUint8		side_number;
	wxUint8		unused2[2];
	wxUint8		sector_size;
	wxUint8		num_of_sectors;
	wxUint8		gap3_length;
	wxUint8		filler_byte;
	cpc_dsk_sector_t sectors[29];
} cpc_dsk_track_t;
#pragma pack()

//
// CPC DSK形式をD88形式にする
//
DiskDskParser::DiskDskParser(DiskImageFile *file, short mod_flags, DiskResult *result)
{
	p_file = file;
	m_mod_flags = mod_flags;
	p_result = result;
	m_is_extended = 0;	// normal
}

DiskDskParser::~DiskDskParser()
{
}

/// セクタデータの作成
wxUint32 DiskDskParser::ParseSector(wxInputStream &istream, int sector_nums, void *user_data, DiskImageTrack *track)
{
	cpc_dsk_sector_t *id = (cpc_dsk_sector_t *)user_data;

	int track_number = id->C;
	int side_number = id->H;
	int sector_number = id->R;
	int sector_size = id->N;

	if (sector_size > 7) {
		// セクタサイズが大きすぎる
		p_result->SetError(DiskResult::ERRV_SECTOR_SIZE_SECTOR, 0, track_number, side_number, sector_number, sector_size, id->data_length);
		return 0;
	}

	sector_size = (128 << sector_size);

	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, sector_nums, false);
	track->Add(sector);

	wxUint8 *buf = sector->GetSectorBuffer();
	int siz = sector->GetSectorBufferSize();

	size_t len = istream.Read(buf, siz).LastRead();
	if (len < (size_t)siz) {
		// ファイルデータが足りない
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
	}
	if (m_is_extended) {
		// バッファが大きいのでスキップ
		if (id->data_length > siz) {
			istream.SeekI(id->data_length - siz, wxFromCurrent);
		}
	}

	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sector->GetSize();
}

/// トラックデータの作成
wxUint32 DiskDskParser::ParseTrack(wxInputStream &istream, int track_size, int offset_pos, wxUint32 offset, DiskImageDisk *disk)
{
	cpc_dsk_track_t track_header;
	size_t len = istream.Read(&track_header, sizeof(track_header)).LastRead();
	if (len != sizeof(track_header)) {
		p_result->SetError(DiskResult::ERR_NO_TRACK, 0);
		return 0;
	}
	if (memcmp(track_header.ident, "Track-Info\r\n", sizeof(track_header.ident)) != 0) {
		p_result->SetError(DiskResult::ERR_NO_TRACK, 0);
		return 0;
	}

	DiskImageTrack *track = disk->NewImageTrack(track_header.track_number, track_header.side_number, offset_pos, 1);
	disk->SetMaxTrackNumber(track_header.track_number);

	wxUint32 d88_track_size = 0;
	for(int pos = 0; pos < track_header.num_of_sectors && p_result->GetValid() >= 0; pos++) {
		d88_track_size += ParseSector(istream
			, track_header.num_of_sectors
			, (void *)&track_header.sectors[pos], track);
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

	return d88_track_size;
}

/// ディスクの解析
wxUint32 DiskDskParser::ParseDisk(wxInputStream &istream)
{
	DiskImageDisk *disk = p_file->NewImageDisk(0);

	cpc_dsk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len != sizeof(header)) {
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return 0;
	}

	disk->SetName(header.creator, sizeof(header.creator));
	int max_tracks = header.num_of_tracks * header.num_of_sides;

	wxUint32 d88_offset = disk->GetOffsetStart();	// header size
	int d88_offset_pos = 0;
	for(int pos = 0; pos < 204 && pos < max_tracks; pos++) {
		d88_offset += ParseTrack(istream
			, m_is_extended ? (int)header.track_sizes[pos] * 256 : header.track_size
			, d88_offset_pos, d88_offset, disk); 
		d88_offset_pos++;
		if (d88_offset_pos >= disk->GetCreatableTracks()) {
			p_result->SetError(DiskResult::ERRV_OVERFLOW_SIZE, 0, d88_offset);
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

	return d88_offset;
}

int DiskDskParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return -1;
}

/// CPC DSKファイルかどうかをチェック
/// @param [in] istream    解析対象データ
/// @return 0:Ok -1:NG
int DiskDskParser::Check(wxInputStream &istream)
{
	istream.SeekI(0);

	cpc_dsk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return -1;
	}

	istream.SeekI(0);

	// check identifier
	int valid = -1;
	if (memcmp(header.ident, "MV - CPCEMU Disk-File\r\nDisk-Info\r\n", sizeof(header.ident)) == 0) {
		m_is_extended = 0;	// normal
		valid = 0;
	} else if (memcmp(header.ident, "EXTENDED CPC DSK File\r\nDisk-Info\r\n", sizeof(header.ident)) == 0) {
		m_is_extended = 1;	// extended
		valid = 0;
	}
	return valid;
}

/// CPC DSKファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param パラメータ通常不要
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskDskParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	ParseDisk(istream);
	return p_result->GetValid();
}