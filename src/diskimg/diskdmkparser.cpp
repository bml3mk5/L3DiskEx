/// @file diskdmkparser.cpp
///
/// @brief TRS-80 DMKディスクパーサー
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskdmkparser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "fileparam.h"
#include "diskresult.h"

#define DMK_WP_NO               0x00                                                // Flag value for disks write-unprotected
#define DMK_WP_YES              0xFF                                                // Flag value for disks write-protected

#define DMK_FLAG_SINGLE_SIDED   0b00010000                                          // 1:Single Sided
#define DMK_FLAG_SINGLE_DENSITY 0b01000000                                          // 1:Single Density, 0:Double Density
#define DMK_FLAG_IGNORE_DENSITY 0b10000000                                          // 1:Ignore Density (disk is DD but should always read 1 byte)

#define DMK_DISK_REAL           0x12345678                                          // Header signature for real disks
#define DMK_DISK_VIRTUAL        0x00000000                                          // Header signature for virtual disks

#pragma pack(1)
/// TRS-80 DMK header
typedef struct st_trs_dmk_header {
	wxUint8		write_protected;	// 0x00:no 0xff:yes 
	wxUint8		num_of_tracks;
	wxUint16	track_length;		// track length
	wxUint8		flags;
	wxUint8		reserved[7];
	wxUint32	signature;			// 0x12345678:Real Disk, 0x00000000:Virtual Disk
} trs_dmk_header_t;

#define DMK_IDAM_DENSITY        0x8000	// 0:Single, 1:Double
#define DMK_IDAM_UNDEFINED      0x4000	// Undefined flag
#define DMK_IDAM_OFFSET         0x3fff	// Pointer offset to the 0xFE byte of the IDAM

/// TRS-80 DMK track
typedef struct st_trs_dmk_track {
	wxUint16	ptr[64];			// pointer to sector IDAMs in each track
} trs_dmk_track_t;

/// TRS-80 DMK sector id
typedef struct st_trs_dmk_sector_id {
	wxUint8		IDAM;
	wxUint8		C;
	wxUint8		H;
	wxUint8		R;
	wxUint8		N;
	wxUint16	CRC;
} trs_dmk_sector_id_t;
#pragma pack()

//
// TRS-80 DMK形式をD88形式にする
//
DiskDmkParser::DiskDmkParser(DiskImageFile *file, short mod_flags, DiskResult *result)
{
	p_file = file;
	m_mod_flags = mod_flags;
	p_result = result;
}

DiskDmkParser::~DiskDmkParser()
{
}

static const struct st_dmk_amarks {
	const char *id;
	int deleted;
} amarks[2][4] = {
	{
		{ "\x00\x00\x00\xfb", 0 },
		{ "\x00\x00\x00\xf8", 0 },	// TRSDOS 1.3 SYSTEM
		{ "\x00\x00\x00\xfa", 0 },	// TRSDOS DIR
		{ NULL, 0 }
	},
	{
		{ "\xa1\xa1\xa1\xfb", 0 },
		{ "\xa1\xa1\xa1\xf8", 0 },	// TRSDOS 1.3 SYSTEM
		{ "\xa1\xa1\xa1\xfa", 0 },	// TRSDOS DIR
		{ NULL, 0 }
	}
};

/// データマークをさがす
/// @param[in,out] istream ファイル
/// @param[in]     sector_size セクタサイズ
/// @param[in]     double_density 倍密度か
/// @param[out]    deleted 削除マークありなし
/// @return セクタ見つかったか
bool DiskDmkParser::FindDataMark(wxInputStream &istream, int sector_size, bool double_density, int &deleted)
{
	char buf[64];
	
	size_t len = istream.Read(buf, sizeof(buf)).LastRead();
	if (len < sizeof(buf)) {
		return false;
	}

	bool found = false;
	int pos;
	int den = double_density ? 1 : 0;

	for(pos = 0; pos < (int)(sizeof(buf) - 4); pos++) {
		for(int i = 0; amarks[den][i].id != NULL ; i++) {
			if (memcmp(&buf[pos], amarks[den][i].id, 4) == 0) {
				found = true;
				deleted = amarks[den][i].deleted;
				break;
			}
		}
		if (found) break;
	}

	if (!found) {
		return false;
	}

	// adjust position
	wxFileOffset offset = pos + 4 - (wxFileOffset)sizeof(buf);
	istream.SeekI(offset, wxFromCurrent);

	return true;
}

/// セクタデータの作成
wxUint32 DiskDmkParser::ParseSector(wxInputStream &istream, int sector_nums, int flags, DiskImageTrack *track)
{
	trs_dmk_sector_id_t id;

	size_t len = istream.Read(&id, sizeof(id)).LastRead();
	if (len != sizeof(id)) {
		p_result->SetError(DiskResult::ERRV_SECTORS_HEADER, 0);
		return 0;
	}

	int track_number = id.C;
	int side_number = id.H;
	int sector_number = id.R;
	int sector_size = id.N;

	if (sector_size > 7) {
		// セクタサイズが大きすぎる
		p_result->SetError(DiskResult::ERRV_SECTOR_SIZE_SECTOR, 0, track_number, side_number, sector_number, sector_size, sector_size);
		return 0;
	}

	sector_size = (128 << sector_size);

	// データの開始位置をさがす
	int deleted = 0;
	if (!FindDataMark(istream, sector_size, (flags & DMK_IDAM_DENSITY) != 0, deleted)) {
		p_result->SetError(DiskResult::ERRV_NO_SECTOR, 0, sector_number, track_number, side_number);
		return 0;
	}

	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, sector_nums, false);
	track->Add(sector);

	wxUint8 *buf = sector->GetSectorBuffer();
	int siz = sector->GetSectorBufferSize();

	len = istream.Read(buf, siz).LastRead();
	if (len < (size_t)siz) {
		// ファイルデータが足りない
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
	}
	wxUint8 crc[2];
	len = istream.Read(crc, 2).LastRead();
	if (len < 2) {
		// ファイルデータが足りない
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
	}
	sector->SetRecordedCRC(((wxUint16)crc[0] << 8) | crc[1]); 

	sector->SetDeletedMark(deleted != 0);
	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sector->GetSize();
}

/// トラックデータの作成
wxUint32 DiskDmkParser::ParseTrack(wxInputStream &istream, int track_size, int offset_pos, wxUint32 offset, DiskImageDisk *disk)
{
	trs_dmk_track_t track_header;

	wxFileOffset file_offset = istream.TellI();

	size_t len = istream.Read(&track_header, sizeof(track_header)).LastRead();
	if (len != sizeof(track_header)) {
		p_result->SetError(DiskResult::ERR_NO_TRACK, 0);
		return 0;
	}

	// セクタ数を計算
	int num_of_sectors = 0;
	for(int pos = 0; pos < 64; pos++) {
		if (track_header.ptr[pos] == 0) {
			break;
		}
		num_of_sectors++;
	}

	DiskImageTrack *track = disk->NewImageTrack(0, 0, offset_pos, 1);

	wxUint32 d88_track_size = 0;
	for(int pos = 0; pos < num_of_sectors && p_result->GetValid() >= 0; pos++) {
		int ptr = wxUINT16_SWAP_ON_BE(track_header.ptr[pos]);
		wxFileOffset next_offset = (ptr & DMK_IDAM_OFFSET);
		// move position in file
		istream.SeekI(file_offset + next_offset, wxFromStart);

		d88_track_size += ParseSector(istream
			, num_of_sectors
			, (ptr & ~DMK_IDAM_OFFSET), track);
	}

	if (p_result->GetValid() >= 0) {
		// インターリーブの計算
		track->CalcInterleave();

		// トラックサイズ設定
		track->SetSize(d88_track_size);
		// トラック番号は各セクタのID Cに合わせる
		int track_number = track->GetMajorIDC();
		track->SetTrackNumber(track_number);
		// サイド番号は各セクタのID Hに合わせる
		track->SetSideNumber(track->GetMajorIDH());

		// ディスクに追加
		disk->Add(track);
		// オフセット設定
		disk->SetOffset(offset_pos, offset);
		// 最大トラック番号設定
		disk->SetMaxTrackNumber(track_number);

		disk->ClearModify();
	} else {
		delete track;
	}

	// 次のトラックデータの先頭へ
	istream.SeekI(file_offset + track_size, wxFromStart);

	return d88_track_size;
}

/// ディスクの解析
wxUint32 DiskDmkParser::ParseDisk(wxInputStream &istream)
{
	DiskImageDisk *disk = p_file->NewImageDisk(0);

	trs_dmk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len != sizeof(header)) {
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return 0;
	}

//	disk->SetName(header.creator, sizeof(header.creator));
	int max_tracks = header.num_of_tracks;

	wxUint32 d88_offset = disk->GetOffsetStart();	// header size
	int d88_offset_pos = 0;
	int limit_offset_pos = disk->GetCreatableTracks();
	for(int pos = 0; pos < 204 && pos < max_tracks; pos++) {
		d88_offset += ParseTrack(istream
			, wxUINT16_SWAP_ON_BE(header.track_length)
			, d88_offset_pos, d88_offset, disk); 
		d88_offset_pos++;
		if (d88_offset_pos >= limit_offset_pos) {
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
		disk->SetWriteProtect(header.write_protected == 0xff);

		p_file->Add(disk, m_mod_flags);
	} else {
		delete disk;
	}

	return d88_offset;
}

int DiskDmkParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return -1;
}

/// TRS-80 DMKファイルかどうかをチェック
/// @param [in] istream    解析対象データ
/// @return 0:Ok -1:NG
int DiskDmkParser::Check(wxInputStream &istream)
{
	istream.SeekI(0);

	trs_dmk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return -1;
	}

	istream.SeekI(0);

	// check header
	// first data is 0x00 or 0xff (write protected flag)
	if (header.write_protected != 0x00 && header.write_protected != 0xff) {
		return -1;
	}
	// signature
	if (header.signature != wxUINT32_SWAP_ON_BE(DMK_DISK_REAL) && header.signature != wxUINT32_SWAP_ON_BE(DMK_DISK_VIRTUAL)) {
		return -1;
	}
	// file size
	if (((wxUint32)header.num_of_tracks * wxUINT16_SWAP_ON_BE(header.track_length) + sizeof(header)) < (wxUint32)istream.GetLength()) {
		// too short
		return -1;
	}

	return 0;
}

/// TRS-80 DMKファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param パラメータ通常不要
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskDmkParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	ParseDisk(istream);
	return p_result->GetValid();
}
