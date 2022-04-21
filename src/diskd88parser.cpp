/// @file diskd88parser.cpp
///
/// @brief D88ディスクパーサー
///
#include "diskd88parser.h"
#include "diskd88.h"

//
//
//
void DiskD88Result::SetMessage(int error_number, va_list ap)
{
	wxString msg;
	switch(error_number) {
	case ERR_CANNOT_OPEN:
// use system message.
//		msg = (_("Cannot open file."));
		break;
	case ERR_CANNOT_SAVE:
// use system message.
//		msg = (_("Cannot save file."));
		break;
	case ERR_INVALID_DISK:
		msg = wxString::FormatV(_("[Disk%d] This is invalid or non supported disk."), ap);
		break;
	case ERR_OVERFLOW:
		msg = wxString::FormatV(_("[Disk%d] Overflow offset. offset:%d disk size:%d"), ap);
		break;
	case ERR_ID_TRACK:
		msg = wxString::FormatV(_("[Disk%d] Unmatch id C and track number %d. id[C:%d H:%d R:%d]"), ap);
		break;
	case ERR_ID_SIDE:
		msg = wxString::FormatV(_("[Disk%d] Invalid id H in track %d. id[C:%d H:%d R:%d]"), ap);
		break;
	case ERR_ID_SECTOR:
		msg = wxString::FormatV(_("[Disk%d] Invalid id R in track %d. id[C:%d H:%d R:%d] num of sector:%d"), ap);
		break;
	case ERR_SECTOR_SIZE:
		msg = wxString::FormatV(_("[Disk%d] Invalid sector size in sector. id[C:%d H:%d R:%d N:%d] sector size:%d"), ap);
		break;
	case ERR_DUPLICATE_TRACK:
		msg = wxString::FormatV(_("[Disk%d] Duplicate track %d and side %d. Side number change to %d."), ap);
		break;
	case ERR_IGNORE_DATA:
		msg = wxString::FormatV(_("[Disk%d] Deleted data found. This sector is ignored. id[C:%d H:%d R:%d]"), ap);
		break;
	case ERR_NO_DATA:
		msg = (_("No data exists."));
		break;
	case ERR_NO_DISK:
		msg = (_("No disk exists."));
		break;
	case ERR_NO_TRACK:
		msg = (_("No track exists."));
		break;
	case ERR_NO_FOUND_TRACK:
		msg = (_("No track found. Run \"Initialize\" to create tracks."));
		break;
	case ERR_REPLACE:
		msg = (_("Couldn't replace a part of sector."));
		break;
	case ERR_FILE_ONLY_1S:
		msg = (_("Supported file is only single side and single density (1S)."));
		break;
	case ERR_FILE_SAME:
		msg = (_("Must be the same disk image type."));
		break;
	case ERR_INTERLEAVE:
		msg = (_("Couldn't create the disk specified interleave."));
		break;
	default:
		msg = wxString::Format(_("Unknown error. code:%d"), error_number);
		break;
	}
	if (!msg.IsEmpty()) msgs.Add(msg);
}

//
//
//
DiskD88Parser::DiskD88Parser(wxInputStream *stream, DiskD88File *file, DiskD88Result &result)
{
	this->stream = stream;
	this->file = file;
	this->result = &result;
}

DiskD88Parser::~DiskD88Parser()
{
}

/// セクタデータの解析
wxUint32 DiskD88Parser::ParseSector(int disk_number, int track_number, int &sector_nums, DiskD88Track *track)
{
	d88_sector_header_t sector_header;

	size_t read_size = stream->Read((void *)&sector_header, sizeof(d88_sector_header_t)).LastRead();
			
	// track number is same ?
	if (sector_header.id.c != track_number) {
		result->SetWarn(DiskD88Result::ERR_ID_TRACK, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
	}
	// side number is valid ?
	if (sector_header.id.h > 1) {
		result->SetWarn(DiskD88Result::ERR_ID_SIDE, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
	}
	// sector number is valid ?
	int sector_number = sector_header.id.r;
	if (sector_number <= 0 || sector_nums < sector_number) {
		result->SetWarn(DiskD88Result::ERR_ID_SECTOR, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_nums);
	}
	// nums of sector is valid ?
	if (sector_nums > sector_header.secnums && sector_header.secnums > 2) {
		sector_nums = sector_header.secnums;
	}
	// deleted data ?
//	if (sector_ptr->deleted != 0) {
//		result->SetWarn(DiskD88Result::ERR_IGNORE_DATA, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
//	}
	// invalid sector size
	size_t real_size = sector_header.size;
	if (sector_header.size > 1024 * 2) {
		result->SetError(DiskD88Result::ERR_SECTOR_SIZE, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_header.id.n, sector_header.size);
		return (wxUint32)read_size;
	} else if (sector_header.size > 1024) {
		result->SetWarn(DiskD88Result::ERR_SECTOR_SIZE, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_header.id.n, sector_header.size);
		sector_header.size = 1024;
	}

	// 追加
	if (result->GetValid() >= 0) {
		wxUint8 *sector_data = new wxUint8[sector_header.size];
		stream->Read((void *)sector_data, sector_header.size);

		DiskD88Sector *sector = new DiskD88Sector(sector_number, sector_header, sector_data);
		track->Add(sector);

		if (real_size > sector_header.size) {
			stream->SeekI(real_size - sector_header.size, wxFromCurrent);
		}
	}

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + sector_header.size;
}

/// トラックデータの解析
wxUint32 DiskD88Parser::ParseTrack(size_t start_pos, int offset_pos, wxUint32 offset, int disk_number, DiskD88Disk *disk)
{
	d88_sector_header_t track_header;

	stream->SeekI(start_pos + offset, wxFromStart);
	size_t read_size = stream->Read((void *)&track_header, sizeof(d88_sector_header_t)).LastRead();
	if (read_size > 0) {
		stream->SeekI((int)read_size * -1, wxFromCurrent);
	}

	int track_number = track_header.id.c;
	int side_number = track_header.id.h;
	int sector_nums = track_header.secnums;
	int sector_size_id = track_header.id.n;

	if (sector_nums > 64) {
		sector_nums = 64;
	}

	DiskD88Track *track = new DiskD88Track(track_number, side_number, offset_pos, offset, 1);

	// sectors
	wxUint32 track_size = 0;
	for(int sec_pos = 0; sec_pos < sector_nums && result->GetValid() >= 0; sec_pos++) {
		track_size += ParseSector(disk_number, track_number, sector_nums, track);
	}

	if (result->GetValid() >= 0) {
		// interleave of sector check
		DiskD88Sectors *ss = track->GetSectors();
		int state = 0;
		int intl = 0;
		for(int sec_pos = 0; sec_pos < (int)ss->Count(); sec_pos++) {
			DiskD88Sector *s = ss->Item(sec_pos);
			switch(state) {
			case 1:
				intl++;
				if (s->GetSectorNumber() == 2) {
					state = 2;
					sec_pos = (int)ss->Count();
				}
				break;
			default:
				if (s->GetSectorNumber() == 1) {
					state = 1;
					intl = 0;
				}
				break;
			}
		}
		if (intl <= 0) {
			intl = 1;
		}
		track->SetInterleave(intl);
	}

	if (result->GetValid() >= 0) {
		// 重複チェック
		DiskD88Tracks *tracks = disk->GetTracks();
		if (tracks) {
			bool dup = false;
			do {
				dup = false;
				for(size_t i=0; i<tracks->Count(); i++) {
					DiskD88Track *t = tracks->Item(i);
					if (t->GetTrackNumber() == track_number && t->GetSideNumber() == side_number) {
						// すでに同じトラック番号とサイド番号がある
						if (sector_size_id > 0) {
							// セクターサイズが256バイト以上なら警告を出す。
							result->SetWarn(DiskD88Result::ERR_DUPLICATE_TRACK, disk_number, track_number, side_number, side_number + 1);
						}
						// サイド番号を変更する
						side_number++;
						track->SetSideNumber(side_number);
						dup = true;
						break;	// チェックやり直し
					}
				}
			} while(dup);
		}
		// トラックを追加
		track->SetSize(track_size);
		if (disk->GetInterleave() < track->GetInterleave()) {
			disk->SetInterleave(track->GetInterleave());
		}
		disk->Add(track);
	} else {
		delete track;
	}

	return track_size;
}

/// ディスクデータの解析
wxUint32 DiskD88Parser::ParseDisk(size_t start_pos, int disk_number)
{
	d88_header_t disk_header;

	wxUint32 read_size = (wxUint32)stream->Read((void *)&disk_header, sizeof(d88_header_t)).LastRead();

	// EOF(0x1a)ならスキップ
	wxByte *p = (wxByte *)&disk_header;
	bool all_eot = true;
	for(wxUint32 pos = 0; pos < read_size; pos++) {
		if (p[pos] != 0x1a) {
			all_eot = false;
			break;
		}
	}
	if (all_eot) {
		return read_size;
	}

	// ディスクサイズが小さすぎる
	if (read_size < sizeof(d88_header_t)) {
			result->SetError(DiskD88Result::ERR_INVALID_DISK, disk_number);
			return read_size;
	}

	wxUint32 disk_size = disk_header.disk_size;
	// ディスクサイズが小さすぎる
	if (disk_size < sizeof(d88_header_t)) {
			result->SetError(DiskD88Result::ERR_INVALID_DISK, disk_number);
			return read_size;
	}

	wxUint32 stream_size = (wxUint32)stream->GetLength();
	// ディスクサイズがファイルサイズより大きい、または4MBを超えている
	if (stream_size < disk_size || (1024*1024*4) < disk_size) {
			result->SetError(DiskD88Result::ERR_INVALID_DISK, disk_number);
			return stream_size;
	}
	// 名前の17文字目は'\0'
	if (disk_header.diskname[16] != '\0') {
			result->SetError(DiskD88Result::ERR_INVALID_DISK, disk_number);
			return disk_size;
	}

	DiskD88Disk *disk = new DiskD88Disk(disk_number, disk_header);

	// parse tracks
	for(int pos = 0; pos < DISKD88_MAX_TRACKS && result->GetValid() >= 0; pos++) {
		wxUint32 offset = disk_header.offsets[pos];
		if (offset == 0) continue;

		// オフセットがディスクサイズを超えている？
		if (offset >= disk_size) {
			if (pos < 160) {
				result->SetError(DiskD88Result::ERR_OVERFLOW, disk_number, offset, disk_size);
				break;
			} else {
				// 一部ヘッダのオフセットが少ないものがある
				result->SetWarn(DiskD88Result::ERR_OVERFLOW, disk_number, offset, disk_size);
				break;
			}
		}

		ParseTrack(start_pos, pos, offset, disk_number, disk);
		disk->SetOffsetMaxPos(pos);
	}

	if (result->GetValid() >= 0) {
		// ディスクを追加
		disk->CalcMajorNumber();
		file->Add(disk);
	} else {
		delete disk;
	}

	return disk_size;
}

int DiskD88Parser::Parse()
{
	size_t read_size = 0;
	size_t stream_size = stream->GetLength();
	int disk_number = (int)file->Count();
	for(; read_size < stream_size && result->GetValid() >= 0; disk_number++) {
		read_size += ParseDisk(read_size, disk_number);
	}
	return result->GetValid();
}

