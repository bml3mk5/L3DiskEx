/// @file diskdskparser.cpp
///
/// @brief CPC DSKディスクパーサー
///
#include "diskdskparser.h"
#include "diskd88parser.h"
#include "diskd88.h"
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
DiskDskParser::DiskDskParser(DiskD88File *file, short mod_flags, DiskResult *result)
{
	this->file = file;
	this->mod_flags = mod_flags;
	this->result = result;
	this->is_extended = 0;	// normal
}

DiskDskParser::~DiskDskParser()
{
}

/// セクタデータの作成
wxUint32 DiskDskParser::ParseSector(wxInputStream *istream, int track_number, int side_number, int sector_size, int sector_nums, void *user_data, DiskD88Track *track)
{
	cpc_dsk_sector_t *id = (cpc_dsk_sector_t *)user_data;

	int sector_number = id->R;

	DiskD88Sector *sector = new DiskD88Sector(track_number, side_number, sector_number, sector_size, sector_nums, false);
	track->Add(sector);

	wxUint8 *buf = sector->GetSectorBuffer();
	int siz = sector->GetSectorBufferSize();

	size_t len = istream->Read(buf, siz).LastRead();
	if (len < (size_t)siz) {
		// ファイルデータが足りない
		result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
	}
	if (is_extended) {
		// バッファが大きいのでスキップ
		if (id->data_length > siz) {
			istream->SeekI(id->data_length - siz, wxFromCurrent);
		}
	}

	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + siz;
}

/// トラックデータの作成
wxUint32 DiskDskParser::ParseTrack(wxInputStream *istream, int track_size, int offset_pos, wxUint32 offset, DiskD88Disk *disk)
{
	cpc_dsk_track_t track_header;
	size_t len = istream->Read(&track_header, sizeof(track_header)).LastRead();
	if (len != sizeof(track_header)) {
		result->SetError(DiskResult::ERR_NO_FOUND_TRACK, 0);
		return 0;
	}
	if (memcmp(track_header.ident, "Track-Info\r\n", sizeof(track_header.ident)) != 0) {
		result->SetError(DiskResult::ERR_NO_FOUND_TRACK, 0);
		return 0;
	}

	DiskD88Track *track = new DiskD88Track(disk, track_header.track_number, track_header.side_number, offset_pos, 1);

	wxUint32 d88_track_size = 0;
	for(int pos = 0; pos < track_header.num_of_sectors && result->GetValid() >= 0; pos++) {
		d88_track_size += ParseSector(istream
			, track_header.track_number, track_header.side_number, 128 << track_header.sector_size, track_header.num_of_sectors
			, (void *)&track_header.sectors[pos], track);
	}

	if (result->GetValid() >= 0) {
		// トラックを追加
		track->SetSize(d88_track_size);
		disk->Add(track);
		disk->SetOffset(offset_pos, offset);
	} else {
		delete track;
	}

	return d88_track_size;
}

/// ディスクの解析
wxUint32 DiskDskParser::ParseDisk(wxInputStream *istream)
{
	DiskD88Disk *disk = new DiskD88Disk(file);

	cpc_dsk_header_t header;
	size_t len = istream->Read(&header, sizeof(header)).LastRead();
	if (len != sizeof(header)) {
		result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return 0;
	}

	disk->SetName(header.creator, sizeof(header.creator));
	int max_tracks = header.num_of_tracks * header.num_of_sides;

	wxUint32 d88_offset = (int)sizeof(d88_header_t);
	int d88_offset_pos = 0;
	for(int pos = 0; pos < 204 && pos < max_tracks; pos++) {
		d88_offset += ParseTrack(istream
			, is_extended ? (int)header.track_sizes[pos] * 256 : header.track_size
			, d88_offset_pos, d88_offset, disk); 
		d88_offset_pos++;
		if (d88_offset_pos >= DISKD88_MAX_TRACKS) {
			result->SetError(DiskResult::ERRV_OVERFLOW_SIZE, 0, d88_offset);
		}
	}
	disk->SetSize(d88_offset);

	if (result->GetValid() >= 0) {
		// ディスクを追加
		const DiskParam *disk_param = disk->CalcMajorNumber();
		if (disk_param) {
			disk->SetDensity(disk_param->GetDensity());
		}
		file->Add(disk, mod_flags);
	} else {
		delete disk;
	}

	return d88_offset;
}

/// CPC DSKファイルかどうかをチェック
/// @param [in] istream    解析対象データ
/// @return true / false
bool DiskDskParser::Check(wxInputStream *istream)
{
	istream->SeekI(0);

	cpc_dsk_header_t header;
	size_t len = istream->Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return false;
	}

	istream->SeekI(0);

	// check identifier
	bool valid = false;
	if (memcmp(header.ident, "MV - CPCEMU Disk-File\r\nDisk-Info\r\n", sizeof(header.ident)) == 0) {
		is_extended = 0;	// normal
		valid = true;
	} else if (memcmp(header.ident, "EXTENDED CPC DSK File\r\nDisk-Info\r\n", sizeof(header.ident)) == 0) {
		is_extended = 1;	// extended
		valid = true;
	}
	return valid;
}

/// CPC DSKファイルを解析
/// @param [in] istream    解析対象データ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskDskParser::Parse(wxInputStream *istream)
{
	ParseDisk(istream);
	return result->GetValid();
}