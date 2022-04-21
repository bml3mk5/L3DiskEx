/// @file diskvfdparser.cpp
///
/// @brief Virtual98 FDディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskvfdparser.h"
#include "diskd88.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"


#pragma pack(1)
typedef struct st_vfd_sector_header {
	wxUint8  c;
	wxUint8  h;
	wxUint8  r;
	wxUint8  n;
	wxUint8  data;	// if all data is same in sector, set this. 
	wxUint8  unknown0;
	wxUint8  dden;
	wxUint8  unknown2;
	wxUint32 start;	// -1(0xffffffff) is no sector data
} vfd_sector_header_t;

typedef struct st_vfd_track_header {
	vfd_sector_header_t sectors[26];
} vfd_track_header_t;

typedef struct st_vfd_header {
	wxUint8  identifier[8];	// VFD1.00
	wxUint8  label[0xd4];
	vfd_track_header_t tracks[160];
} vfd_header_t;
#pragma pack()

//
//
//
DiskVFDParser::DiskVFDParser(DiskD88File *file, short mod_flags, DiskResult *result)
{
	this->file = file;
	this->mod_flags = mod_flags;
	this->result = result;
}

DiskVFDParser::~DiskVFDParser()
{
}

/// セクタデータの作成
wxUint32 DiskVFDParser::ParseSector(wxInputStream &istream, int sector_nums, void *user_data, DiskD88Track *track)
{
	vfd_sector_header_t *sector_header = (vfd_sector_header_t *)user_data;

	if (sector_header->c == 0xff || sector_header->h == 0xff || sector_header->r == 0xff) {
		// セクタなし
		return 0;
	}

	int track_number = sector_header->c;
	int side_number = sector_header->h;
	int sector_number = sector_header->r;
	int sector_size = sector_header->n;

	if (sector_size > 7) {
		// セクタサイズが大きすぎる
		result->SetError(DiskResult::ERRV_SECTOR_SIZE_SECTOR, 0, track_number, side_number, sector_number, sector_size, 128 << sector_size);
		return 0;
	}

	sector_size = (128 << sector_size);

	// セクタ作成
	DiskD88Sector *sector = new DiskD88Sector(track_number, side_number, sector_number, sector_size, sector_nums, sector_header->dden == 0);
	track->Add(sector);

	wxUint8 *buf = sector->GetSectorBuffer();
	int siz = sector->GetSectorBufferSize();

	if (sector_header->start != (wxUint32)-1) {
		// 実際のデータを取得
		istream.SeekI(sector_header->start);

		size_t len = istream.Read(buf, siz).LastRead();
		if (len < (size_t)siz) {
			// ファイルデータが足りない
			result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		}
	} else {
		// データはないので特定データでサプレスする
		int data = sector_header->data;
		memset(buf, data, siz);
	}
	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + siz;
}

/// トラックデータの作成
wxUint32 DiskVFDParser::ParseTrack(wxInputStream &istream, void *user_data, int offset_pos, wxUint32 offset, DiskD88Disk *disk)
{
	vfd_track_header_t *track_header = (vfd_track_header_t *)user_data;

	// メジャーな番号を調べる
	int num_of_sectors = 0;
	IntHashMap track_number_map;
	IntHashMap side_number_map;
	for(int sec = 0; sec < 26; sec++) {
		if (track_header->sectors[sec].c == 0xff || track_header->sectors[sec].h == 0xff || track_header->sectors[sec].r == 0xff) {
			continue;
		}
		num_of_sectors++;
		IntHashMapUtil::IncleaseValue(track_number_map, track_header->sectors[sec].c);
		IntHashMapUtil::IncleaseValue(side_number_map, track_header->sectors[sec].h);
	}
	if (num_of_sectors == 0) {
		// セクタなし
		return 0;
	}

	int track_number = IntHashMapUtil::GetMaxKeyOnMaxValue(track_number_map);
	int side_number = IntHashMapUtil::GetMaxKeyOnMaxValue(side_number_map);

	// トラック作成
	DiskD88Track *track = new DiskD88Track(disk, track_number, side_number, offset_pos, 1);
	disk->SetMaxTrackNumber(track_number);

	wxUint32 d88_track_size = 0;
	for(int sec = 0; sec < 26 && result->GetValid() >= 0; sec++) {
		d88_track_size += ParseSector(istream
			, num_of_sectors
			, &track_header->sectors[sec], track);
	}

	if (result->GetValid() >= 0) {
		// インターリーブの計算
		track->CalcInterleave();
	}

	if (result->GetValid() >= 0) {
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
wxUint32 DiskVFDParser::ParseDisk(wxInputStream &istream)
{
	DiskD88Disk *disk = new DiskD88Disk(file, 0);

	vfd_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len != sizeof(header)) {
		result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return 0;
	}

	disk->SetName(header.label, sizeof(header.label));

	// d88トラックの作成
	wxUint32 d88_offset = (int)sizeof(d88_header_t);
	int d88_offset_pos = 0;
	for(int pos = 0; pos < 160; pos++) {
		d88_offset += ParseTrack(istream, &header.tracks[pos]
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
			disk->SetDensity(disk_param->GetParamDensity());
		}
		file->Add(disk, mod_flags);
	} else {
		delete disk;
	}

	return d88_offset;
}

/// チェック
/// @param [in] istream       解析対象データ
/// @retval  0 正常
/// @retval -1 エラー
int DiskVFDParser::Check(wxInputStream &istream)
{
	istream.SeekI(0);

	vfd_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return -1;
	}

	istream.SeekI(0);

	// check identifier
	if (memcmp(header.identifier, "VFD1", 4) != 0) {
		return -1;
	}

	return 0;
}

/// VFDファイルを解析
/// @param [in] istream    解析対象データ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskVFDParser::Parse(wxInputStream &istream)
{
	ParseDisk(istream);
	return result->GetValid();
}
