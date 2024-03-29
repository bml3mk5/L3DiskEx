/// @file diskimdparser.cpp
///
/// @brief IMageDisk imdディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskimdparser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"


#pragma pack(1)
/// IMageDisk IMD形式トラックヘッダ
typedef struct st_imd_track_header {
	wxUint8  mode;
	wxUint8  track_num;
	wxUint8  head_num_n_flg;
	wxUint8  num_of_sectors;
	wxUint8  sector_size_n;
} imd_track_header_t;
#pragma pack()

//
//
//
DiskIMDParser::DiskIMDParser(DiskImageFile *file, short mod_flags, DiskResult *result)
{
	p_file = file;
	m_mod_flags = mod_flags;
	p_result = result;
}

DiskIMDParser::~DiskIMDParser()
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
wxUint32 DiskIMDParser::ParseSector(wxInputStream &istream, int disk_number, int track_number, int side_number, int sector_nums, int sector_number, int sector_size, bool single_density, DiskImageTrack *track)
{
	wxUint8 h_sector;
	size_t len = istream.Read(&h_sector, sizeof(h_sector)).LastRead();
	if (len != sizeof(h_sector)) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
		return 0;
	}

	// セクタ作成
	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, sector_nums, false);
	track->Add(sector);

	wxUint8 *buffer = sector->GetSectorBuffer();
//	int buflen = sector->GetSectorBufferSize();

	if (h_sector == 0) {
		// missing data
		sector->Fill(0);
	} else {
		h_sector--;
		if (h_sector & 1) {
			// compressed data
			int ch = istream.GetC();
			if (ch == wxEOF) {
				ch = 0;
			}
			sector->Fill(ch & 0xff);
		} else {
			// plain data
			istream.Read(buffer, sector_size);
//			for(int i=0; i < sector_size && !istream.Eof(); i++) {
//				buffer[i] = (wxUint8)istream.GetC();
//			}
		}
		if (h_sector & 0x02) {
			// deleted data
			sector->SetDeletedMark(true);
		}
		if (h_sector & 0x04) {
			// TODO: bad sector
		}
	}

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
int DiskIMDParser::ParseTrack(wxInputStream &istream, int disk_number, int offset_pos, wxUint32 offset, DiskImageDisk *disk)
{
	imd_track_header_t h_track;
	int sector_size = 0;

	size_t len = istream.Read(&h_track, sizeof(h_track)).LastRead();
	if (len < sizeof(h_track)) {
		// end of file
		return -1;
	}
	if (h_track.mode > 5) {
		p_result->SetError(DiskResult::ERRV_DISK_HEADER, disk_number);
		return -1;
	}
	if ((h_track.head_num_n_flg & 0xf) > 1) {
		p_result->SetError(DiskResult::ERRV_ID_SIDE, disk_number
			,h_track.track_num, h_track.track_num, (h_track.head_num_n_flg & 0xf), 1);
		return -1;
	}
	if (h_track.sector_size_n > 6) {
		p_result->SetError(DiskResult::ERRV_SECTOR_SIZE_HEADER, disk_number, h_track.sector_size_n);
		return -1;
	}
	sector_size = 128 << (h_track.sector_size_n);

	// セクターマップ
	wxUint8 sector_map[256];
	wxUint8 track_map[256];
	wxUint8 head_map[256];
	if (h_track.num_of_sectors) {
		// read sector map
		len = istream.Read(sector_map, h_track.num_of_sectors).LastRead();
		if (len != h_track.num_of_sectors) {
			p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
			return -1;
		}
		if (h_track.head_num_n_flg & 0x80) {
			// read cylinder map
			len = istream.Read(track_map, h_track.num_of_sectors).LastRead();
			if (len != h_track.num_of_sectors) {
				p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
				return -1;
			}
		} else {
			memset(track_map, h_track.track_num, sizeof(track_map));
		}
		if (h_track.head_num_n_flg & 0x40) {
			// read head map
			len = istream.Read(head_map, h_track.num_of_sectors).LastRead();
			if (len != h_track.num_of_sectors) {
				p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
				return -1;
			}
		} else {
			memset(head_map, (h_track.head_num_n_flg & 0xf), sizeof(head_map));
		}
	}
	if (sector_size * h_track.num_of_sectors > 32768) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_LARGE, disk_number);
		return -1;
	}

	// トラックの作成
	DiskImageTrack *track = disk->NewImageTrack(h_track.track_num, (h_track.head_num_n_flg & 0xf), offset_pos, 1);
	disk->SetMaxTrackNumber(h_track.track_num);

	wxUint32 d88_track_size = 0;
	for(int pos = 0; pos < h_track.num_of_sectors && p_result->GetValid() >= 0; pos++) {
		d88_track_size += ParseSector(istream, disk_number, track_map[pos], head_map[pos], h_track.num_of_sectors, sector_map[pos], sector_size, h_track.mode <= 2, track);
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


/// IMDファイルを解析
/// @param [in] istream     解析対象データ
/// @param [in] disk_number ディスク番号
/// @retval -1: finish parsing
/// @retval  0: parse next disk
int DiskIMDParser::ParseDisk(wxInputStream &istream, int disk_number)
{
	// skip comment line on head of stream
	int ch = 0;
	while(ch != 0x1a && ch != wxEOF) {
		ch = istream.GetC();
	}
	if (ch == wxEOF) {
		return -1;
	}

	// ディスク作成
	DiskImageDisk *disk = p_file->NewImageDisk(disk_number);

	// トラック解析
	wxUint32 d88_offset = disk->GetOffsetStart();	// header size
	int d88_offset_pos = 0;
	int limit_offset_pos = disk->GetCreatableTracks();
	for(int pos = 0; pos < 204; pos++) {
		int offset = ParseTrack(istream, disk_number, d88_offset_pos, d88_offset, disk);
		if (offset == -1) {
			break;
		}
		d88_offset += offset;

		d88_offset_pos++;
		if (d88_offset_pos >= limit_offset_pos) {
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

int DiskIMDParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return -1;
}

/// チェック
/// @param [in] istream       解析対象データ
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskIMDParser::Check(wxInputStream &istream)
{
	istream.SeekI(0);

	wxUint8 header[31];
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return -1;
	}
	if (memcmp(&header[0], "IMD ", 4) != 0) {
		// not IMD image
		return -1;
	}
	if (header[4] != '1' || header[5] != '.') {
		// support only version 1.xx
		return -1;
	}

	// check comment line on head of stream
	int ch = 0;
	while(ch != 0x1a && ch != wxEOF) {
		ch = istream.GetC();
	}
	if (ch == wxEOF) {
		return -1;
	}

	return 0;
}

/// IMDファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param パラメータ通常不要
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskIMDParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	istream.SeekI(0);

	for(int disk_number = 0; disk_number < 1; disk_number++) {
		if (ParseDisk(istream, disk_number) < 0) {
			break;
		}
	}
	return p_result->GetValid();
}
