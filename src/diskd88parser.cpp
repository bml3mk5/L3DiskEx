/// @file diskd88parser.cpp
///
/// @brief D88ディスクパーサー
///
#include "diskd88parser.h"
#include "diskd88.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"

//
static int compare_int(int *a, int *b)
{
	return (*a < *b ? -1 : (*a > *b ? 1 : 0));
}

//
//
//
DiskD88Parser::DiskD88Parser(DiskD88File *file, short mod_flags, DiskResult *result)
{
	this->file = file;
	this->mod_flags = mod_flags;
	this->result = result;
}

DiskD88Parser::~DiskD88Parser()
{
}

/// セクタデータ解析前のパラメータ取得
void DiskD88Parser::PreParseSectors(wxInputStream *istream, int disk_number, int &track_number, int &side_number, int &sector_nums, int &sector_size)
{
	IntHashMap track_number_map;
	IntHashMap side_number_map;
	IntHashMap sector_nums_map;
	IntHashMap sector_size_map;

	d88_sector_header_t sector_header;

	wxFileOffset ipos = istream->TellI();

	if (sector_nums == 0) {
		sector_nums = 16;
	}
	for(int num = 0; num < sector_nums; num++) {
		istream->Read((void *)&sector_header, sizeof(d88_sector_header_t)).LastRead();

		IntHashMapUtil::IncleaseValue(track_number_map, sector_header.id.c);
		IntHashMapUtil::IncleaseValue(side_number_map, sector_header.id.h);
		IntHashMapUtil::IncleaseValue(sector_nums_map, sector_header.secnums);

		if (2 < sector_header.secnums && sector_header.secnums < sector_nums) {
			sector_nums = sector_header.secnums;
		}

		size_t real_size = sector_header.size;
		if (real_size == 0) {
			real_size = 256;
		}
		IntHashMapUtil::IncleaseValue(sector_size_map, (int)real_size);

		istream->SeekI(real_size, wxFromCurrent);
	}

	istream->SeekI(ipos);

	track_number = IntHashMapUtil::GetMaxKeyOnMaxValue(track_number_map);
	side_number = IntHashMapUtil::GetMaxKeyOnMaxValue(side_number_map);
	sector_nums = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_nums_map);
	sector_size = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_size_map);
}

/// セクタデータの解析
wxUint32 DiskD88Parser::ParseSector(wxInputStream *istream, int disk_number, int track_number, int sector_nums, int sector_size, DiskD88Track *track)
{
	d88_sector_header_t sector_header;

	size_t read_size = istream->Read((void *)&sector_header, sizeof(d88_sector_header_t)).LastRead();
			
	// track number is same ?
	if (sector_header.id.c != track_number) {
		result->SetWarn(DiskResult::ERRV_ID_TRACK, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
	}
	// side number is valid ?
	if (sector_header.id.h > 1) {
		result->SetWarn(DiskResult::ERRV_ID_SIDE, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
	}
	int sector_number = sector_header.id.r;
	// sector number is valid ?
	if (sector_number <= 0) {
		result->SetWarn(DiskResult::ERRV_ID_SECTOR, disk_number, track_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_nums);
	}
//	// nums of sector is valid ?
//	if (sector_nums > sector_header.secnums && sector_header.secnums > 2) {
//		sector_nums = sector_header.secnums;
//	}
//	// deleted data ?
//	if (sector_ptr->deleted != 0) {
//		result->SetWarn(DiskResult::ERR_IGNORE_DATA, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r);
//	}
	// invalid sector size
	size_t real_size = sector_header.size;
	if (sector_header.size > 1024 * 2) {
		result->SetError(DiskResult::ERRV_SECTOR_SIZE, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_header.id.n, sector_header.size);
		return (wxUint32)read_size;
	} else if (sector_header.size > 1024) {
		result->SetWarn(DiskResult::ERRV_SECTOR_SIZE, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_header.id.n, sector_header.size);
		sector_header.size = 1024;
	} else if (sector_header.size == 0) {
		result->SetWarn(DiskResult::ERRV_SECTOR_SIZE, disk_number, sector_header.id.c, sector_header.id.h, sector_header.id.r, sector_header.id.n, sector_header.size);
		sector_header.size = sector_size;
	}

	// 追加
	if (result->GetValid() >= 0) {
		wxUint8 *sector_data = new wxUint8[sector_header.size];
		istream->Read((void *)sector_data, sector_header.size);

		DiskD88Sector *sector = new DiskD88Sector(sector_number, sector_header, sector_data);
		track->Add(sector);

		if (real_size > sector_header.size) {
			istream->SeekI(real_size - sector_header.size, wxFromCurrent);
		}
	}

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + sector_header.size;
}

/// トラックデータの解析
wxUint32 DiskD88Parser::ParseTrack(wxInputStream *istream, size_t start_pos, int offset_pos, wxUint32 offset, int disk_number, DiskD88Disk *disk)
{
	int track_number = 0;
	int side_number = 0;
	int sector_nums = 0;
	int sector_size = 0;

	istream->SeekI(start_pos + offset, wxFromStart);

	PreParseSectors(istream, disk_number, track_number, side_number, sector_nums, sector_size);

	if (sector_nums == 0) {
		// セクタ数０はおかしい！
		return 0;
	}

	if (sector_nums > 64) {
		sector_nums = 64;
	}

	DiskD88Track *track = new DiskD88Track(disk, track_number, side_number, offset_pos, 1);

	// sectors
	wxUint32 track_size = 0;
	for(int sec_pos = 0; sec_pos < sector_nums && result->GetValid() >= 0; sec_pos++) {
		track_size += ParseSector(istream, disk_number, track_number, sector_nums, sector_size, track);
	}

	// sector number is valid ?
	DiskD88Sectors *sectors = track->GetSectors(); 
	if (!sectors && sector_nums != (int)sectors->Count()) {
		result->SetWarn(DiskResult::ERRV_ID_NUM_OF_SECTOR, disk_number, track_number, side_number);
	}

	if (result->GetValid() >= 0) {
		// インターリーブの計算
		track->CalcInterleave();
	}

	if (result->GetValid() >= 0) {
		// セクタの重複や存在をチェック
		wxArrayInt arr;
		DiskD88Sectors *ss = track->GetSectors();
		for(size_t sec_pos = 0; sec_pos < ss->Count(); sec_pos++) {
			DiskD88Sector *s = ss->Item(sec_pos);
			arr.Add(s->GetSectorNumber());
		}
		arr.Sort(&compare_int);
		int prev = -1;
		for(size_t sec_pos = 0; sec_pos < arr.Count(); sec_pos++) {
			int curr = arr.Item(sec_pos);
			if (prev >= 0) {
				if (curr == prev) {
					// duplicate
					result->SetWarn(DiskResult::ERRV_DUPLICATE_SECTOR, disk_number, curr, track_number, side_number);
				} else if (prev + 1 != curr) {
					// non sequential
					result->SetWarn(DiskResult::ERRV_NO_SECTOR, disk_number, prev + 1, track_number, side_number);
				}
			}
			prev = curr;
		}
	}

	if (result->GetValid() >= 0) {
		// トラックの重複チェック
		DiskD88Tracks *tracks = disk->GetTracks();
		if (tracks) {
			bool dup = false;
			do {
				dup = false;
				for(size_t i=0; i<tracks->Count(); i++) {
					DiskD88Track *t = tracks->Item(i);
					if (t->GetTrackNumber() == track_number && t->GetSideNumber() == side_number) {
						// すでに同じトラック番号とサイド番号がある
						if (sector_size > 0) {
							// セクターサイズが256バイト以上なら警告を出す。
							result->SetWarn(DiskResult::ERRV_DUPLICATE_TRACK, disk_number, track_number, side_number, side_number + 1);
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
/// @return ディスクサイズ
wxUint32 DiskD88Parser::ParseDisk(wxInputStream *istream, size_t start_pos, int disk_number)
{
	d88_header_t disk_header;

	// seek 
	istream->SeekI(start_pos);

	wxUint32 read_size = (wxUint32)istream->Read((void *)&disk_header, sizeof(d88_header_t)).LastRead();

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
			result->SetError(DiskResult::ERRV_INVALID_DISK, disk_number);
			return read_size;
	}

	wxUint32 disk_size = disk_header.disk_size;
	// ディスクサイズが小さすぎる
	if (disk_size < sizeof(d88_header_t)) {
			result->SetError(DiskResult::ERRV_INVALID_DISK, disk_number);
			return read_size;
	}

	wxUint32 stream_size = (wxUint32)istream->GetLength();
	// ディスクサイズがファイルサイズより大きい、または4MBを超えている
	if (stream_size < disk_size || (1024*1024*4) < disk_size) {
			result->SetError(DiskResult::ERRV_INVALID_DISK, disk_number);
			return stream_size;
	}
	// 名前の17文字目は'\0'
	if (disk_header.diskname[16] != '\0') {
			result->SetWarn(DiskResult::ERRV_DISK_HEADER, disk_number);
	}

	// オフセット部分の最小値を求める
	// -> 古いd88はオフセット部分が少ない
	wxUint32 offset_start = (wxUint32)-1;
	for(int pos = 0; pos < (DISKD88_MAX_TRACKS - 16); pos++) {
		wxUint32 offset = disk_header.offsets[pos];
		if (offset_start > offset && offset > 0) {
			offset_start = offset;
		}
	}
	// オフセットなし（トラックなしの場合）初期値をセット
	if (offset_start == (wxUint32)-1) {
		offset_start = (wxUint32)sizeof(d88_header_t);
	}

	int max_tracks = DISKD88_MAX_TRACKS - ((int)sizeof(d88_header_t) - (int)offset_start) / 4; 
	for(int pos = max_tracks; pos < DISKD88_MAX_TRACKS; pos++) {
		disk_header.offsets[pos] = 0;
	}

	DiskD88Disk *disk = new DiskD88Disk(file, disk_number, disk_header);

	disk->SetOffsetStart(offset_start);

	// parse tracks
	for(int pos = 0; pos < max_tracks && result->GetValid() >= 0; pos++) {
		wxUint32 offset = disk_header.offsets[pos];
		if (offset == 0) continue;

		// オフセットがディスクサイズを超えている？
		if (offset >= disk_size) {
			result->SetWarn(DiskResult::ERRV_OVERFLOW_OFFSET, disk_number, pos, offset, disk_size);
			disk->SetOffset(pos, 0);
			continue;
		}

		ParseTrack(istream, start_pos, pos, offset, disk_number, disk);
		disk->SetMaxTrackNumber(pos);
	}

	if (result->GetValid() >= 0) {
		// ディスクを追加
		disk->CalcMajorNumber();
		file->Add(disk, mod_flags);
	} else {
		delete disk;
	}

	return disk_size;
}

/// D88ファイルを解析
/// @param [in] istream    解析対象データ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88Parser::Parse(wxInputStream *istream)
{
	size_t read_size = 0;
	size_t stream_size = istream->GetLength();
	int disk_number = (int)file->Count();
	// ディスクサイズが0
	if (stream_size == 0) {
		result->SetError(DiskResult::ERRV_INVALID_DISK, disk_number);
		return result->GetValid();
	}
	for(; read_size < stream_size && result->GetValid() >= 0; disk_number++) {
		wxUint32 size = ParseDisk(istream, read_size, disk_number);
		if (size == 0) break;
		read_size += size;
	}
	return result->GetValid();
}
