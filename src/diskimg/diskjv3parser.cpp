/// @file diskjv3parser.cpp
///
/// @brief TRS-80 JV3形式 ディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskjv3parser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"

#define JV3_DENSITY     0x80  // 1=dden, 0=sden
#define JV3_DAM         0x60  // data address mark code; see below
#define JV3_SIDE        0x10  // 0=side 0, 1=side 1
#define JV3_ERROR       0x08  // 0=ok, 1=CRC error
#define JV3_NONIBM      0x04  // 0=normal, 1=short
#define JV3_SIZE        0x03  // in used sectors: 0=256,1=128,2=1024,3=512
                              // in free sectors: 0=512,1=1024,2=128,3=256

#define JV3_FREE        0xFF  // in track and sector fields of free sectors
#define JV3_FREEF       0xFC  // in flags field, or'd with size code

#define JV3_WRITABLE    0xFF
#define JV3_WPROTECT    0x00

#pragma pack(1)
/// セクタ番号情報
typedef struct st_jv3_sector_id {
	wxUint8  track_number;
	wxUint8  sector_number;
	wxUint8  flags;
} jv3_sector_id_t;

/// ヘッダ情報
typedef struct st_jv3_header {
	jv3_sector_id_t	ids[2901];
	wxUint8			write_protected;
} jv3_header_t;
#pragma pack()

wxInt16 jv3_size_map[4] = { 1, 0, 3, 2 };

//
//
//
DiskJV3Parser::DiskJV3Parser(DiskImageFile *file, short mod_flags, DiskResult *result)
{
	p_file = file;
	m_mod_flags = mod_flags;
	p_result = result;
}

DiskJV3Parser::~DiskJV3Parser()
{
}

/// セクタデータの作成
wxUint32 DiskJV3Parser::ParseSector(wxInputStream &istream, int track_number, int side_number, int sector_number, int sector_size, int sector_nums, bool single_density, DiskImageTrack *track)
{
	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, sector_nums, single_density);
	track->Add(sector);

	wxUint8 *buf = sector->GetSectorBuffer();
	int siz = sector->GetSectorBufferSize();

	size_t len = istream.Read(buf, siz).LastRead();
	if (len < (size_t)siz) {
		// ファイルデータが足りない
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
	}

	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sector->GetSize();

}

/// ファイルを解析
/// @param [in,out] istream     解析対象データ
/// @return  サイズ
wxUint32 DiskJV3Parser::ParseDisk(wxInputStream &istream)
{
	jv3_header_t header;

	DiskImageDisk *disk = p_file->NewImageDisk(0);
	wxUint32 d88_offset = disk->GetOffsetStart();	// header size
	int d88_offset_pos = 0;
	int max_track_number = -1;
	int limit_offset_pos = disk->GetCreatableTracks();
	for(int disk_part = 0; disk_part < 2; disk_part++) {
		size_t len = istream.Read(&header, sizeof(header)).LastRead();
		if (len == 0) {
			break;
		}

		for(int i=0; i<2901; i++) {
			if (header.ids[i].track_number == JV3_FREE || header.ids[i].sector_number == JV3_FREE) {
				continue;
			}
			int track_number = header.ids[i].track_number;
			int side_number = (header.ids[i].flags & JV3_SIDE) ? 1 : 0;
			int sector_number = header.ids[i].sector_number;
			int sector_size = (128 << jv3_size_map[header.ids[i].flags & JV3_SIZE]);
			bool single_density = ((header.ids[i].flags & JV3_DENSITY) == 0);

			// トラックが存在するか
			DiskImageTrack *track = disk->GetTrack(track_number, side_number);
			if (!track) {
				// 新規トラック
				track = disk->NewImageTrack(track_number, side_number, d88_offset_pos, 1);
				disk->Add(track);
				d88_offset_pos++;

				if (d88_offset_pos >= limit_offset_pos) {
					p_result->SetError(DiskResult::ERRV_OVERFLOW_SIZE, 0, d88_offset);
				}
			}
			int track_size = track->GetSize();
			// セクタ作成
			int sector_newsize = ParseSector(istream, track_number, side_number, sector_number, sector_size, 1, single_density, track);
			// トラックサイズ更新
			track->SetSize(track_size + sector_newsize);
			d88_offset += sector_newsize;

			max_track_number = max_track_number < track_number ? track_number : max_track_number;
		}
	}
	// ディスクサイズ設定
	disk->SetSize(d88_offset);
	// 最大トラック番号設定
	disk->SetMaxTrackNumber(max_track_number);

	if (p_result->GetValid() >= 0) {
		d88_offset = disk->GetOffsetStart();	// header size
		DiskImageTracks *tracks = disk->GetTracks();
		for(int pos = 0; pos < (int)tracks->Count(); pos++) {
			DiskImageTrack *track = tracks->Item(pos);

			// インターリーブの計算
			track->CalcInterleave();
			// サイズの再計算
			track->Shrink(false);
			// オフセットの設定
			disk->SetOffset(pos, d88_offset);

			d88_offset += track->GetSize();
		}

		// ディスクを追加
		const DiskParam *disk_param = disk->CalcMajorNumber();
		if (disk_param) {
			disk->SetDensity(disk_param->GetParamDensity());
		}
		disk->SetWriteProtect(header.write_protected == JV3_WPROTECT);
		disk->ClearModify();

		p_file->Add(disk, m_mod_flags);

	} else {
		delete disk;
	}

	return d88_offset;
}

/// ファイルを解析
/// @param [in,out] istream    解析対象データ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskJV3Parser::Parse(wxInputStream &istream)
{
	p_result->Clear();
	istream.SeekI(0);

	ParseDisk(istream);

	return p_result->GetValid();
}

/// チェック
/// @param [in,out] istream       解析対象データ
/// @retval 0 正常 -1 対象データではない
int DiskJV3Parser::Check(wxInputStream &istream)
{
	istream.SeekI(0);

	jv3_header_t header;

	for(int disk_part = 0; disk_part < 2; disk_part++) {
		size_t len = istream.Read(&header, sizeof(header)).LastRead();
		if (disk_part > 0 && len == 0) {
			break;
		}
		if (len < sizeof(header)) {
			// too short
			p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
			return p_result->GetValid();
		}
		// 書き込み禁止エリア
		if (header.write_protected != JV3_WRITABLE && header.write_protected != JV3_WPROTECT) {
			p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
			return p_result->GetValid();
		}
		// ディスクサイズを計算
		wxUint32 data_size = 0;
		int err_zero = 0;
		int err_trks = 0;
		int err_secs = 0;
		for(int i=0; i<2901; i++) {
			if (header.ids[i].track_number == JV3_FREE || header.ids[i].sector_number == JV3_FREE) {
				continue;
			}
			// 全て０か
			if (header.ids[i].track_number == 0 && header.ids[i].sector_number == 0 && header.ids[i].flags == 0) {
				err_zero++;
			}
			// トラック番号が連続しているか
			if (i > 0) {
				if (!(header.ids[i-1].track_number == header.ids[i].track_number || header.ids[i-1].track_number + 1 == header.ids[i].track_number)) {
					err_trks++;
				}
			}
			// トラック番号は80以内か
			if (header.ids[i].track_number > 80) {
				err_trks++;
			}
			// セクタ番号は32以内か
			if (header.ids[i].sector_number > 32) {
				err_secs++;
			}
			data_size += (128 << jv3_size_map[header.ids[i].flags & JV3_SIZE]);
		}

		if (err_zero >= 40) {
			// ゼロパディングなのでJV3形式ではない
			p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
			return p_result->GetValid();
		}
		if (err_trks >= 20) {
			// トラック番号がJV3形式ではない
			p_result->SetError(DiskResult::ERRV_ID_TRACK, 0);
			return p_result->GetValid();
		}
		if (err_secs >= 20) {
			// セクタ番号がJV3形式ではない
			p_result->SetError(DiskResult::ERRV_ID_SECTOR, 0);
			return p_result->GetValid();
		}

		wxFileOffset file_offset = istream.SeekI(data_size, wxFromCurrent);
		if (file_offset == wxInvalidOffset || file_offset < (wxFileOffset)(sizeof(jv3_header_t) + data_size)) {
			// ファイルサイズ足りない
			p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
			return p_result->GetValid();
		}
	}

	return 0;
}
