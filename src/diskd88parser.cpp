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
DiskD88ParseOffset::DiskD88ParseOffset()
{
	num		= 0;
	offset	= 0;
	size	= 0;
}

DiskD88ParseOffset::DiskD88ParseOffset(const DiskD88ParseOffset &src)
{
	num		= src.num;
	offset	= src.offset;
	size	= src.size;
}

DiskD88ParseOffset::DiskD88ParseOffset(int n_num, wxUint32 n_offset, int n_size)
{
	num		= n_num;
	offset	= n_offset;
	size	= n_size;
}

DiskD88ParseOffset::~DiskD88ParseOffset()
{
}

DiskD88ParseOffset &DiskD88ParseOffset::operator=(const DiskD88ParseOffset &src)
{
	num		= src.num;
	offset	= src.offset;
	size	= src.size;
	return *this;
}

int	DiskD88ParseOffset::CmpByNum(DiskD88ParseOffset **item1, DiskD88ParseOffset **item2)
{
	int cmp = ((*item1)->num < (*item2)->num ? -1 : ((*item1)->num > (*item2)->num ? 1 : 0));
	return cmp;
}

int	DiskD88ParseOffset::CmpByOffset(DiskD88ParseOffset **item1, DiskD88ParseOffset **item2)
{
	int cmp = ((*item1)->offset < (*item2)->offset ? -1 : ((*item1)->offset > (*item2)->offset ? 1 : 0));
	return cmp;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskD88ParseOffsets);

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
void DiskD88Parser::PreParseSectors(wxInputStream &istream, int disk_number, int &track_number, int &side_number, int &sector_nums, int &sector_size)
{
	IntHashMap track_number_map;
	IntHashMap side_number_map;
	IntHashMap sector_nums_map;
	IntHashMap sector_size_map;

	d88_sector_header_t sector_header;

	wxFileOffset ipos = istream.TellI();

	if (sector_nums == 0) {
		sector_nums = 4;
	}
	for(int num = 0; num < sector_nums; num++) {
		istream.Read((void *)&sector_header, sizeof(d88_sector_header_t)).LastRead();

		IntHashMapUtil::IncleaseValue(track_number_map, sector_header.id.c);
		IntHashMapUtil::IncleaseValue(side_number_map, sector_header.id.h);
		IntHashMapUtil::IncleaseValue(sector_nums_map, sector_header.secnums);

		if (2 < sector_header.secnums && sector_header.secnums < sector_nums) {
			sector_nums = sector_header.secnums;
		}

		size_t real_size = sector_header.size;
//		if (real_size == 0) {
//			real_size = 256;
//		}
		IntHashMapUtil::IncleaseValue(sector_size_map, (int)real_size);

		istream.SeekI(real_size, wxFromCurrent);
	}

	istream.SeekI(ipos);

	track_number = IntHashMapUtil::GetMaxKeyOnMaxValue(track_number_map);
	side_number = IntHashMapUtil::GetMaxKeyOnMaxValue(side_number_map);
	sector_nums = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_nums_map);
	sector_size = IntHashMapUtil::GetMaxKeyOnMaxValue(sector_size_map);
}

/// セクタデータの解析
wxUint32 DiskD88Parser::ParseSector(wxInputStream &istream, int disk_number, int track_number, int sector_nums, int sector_size, DiskD88Track *track)
{
	d88_sector_header_t *sector_header = new d88_sector_header_t;

	size_t header_size = istream.Read((void *)sector_header, sizeof(d88_sector_header_t)).LastRead();
			
	// track number is same ?
	if (sector_header->id.c != track_number) {
		result->SetWarn(DiskResult::ERRV_ID_TRACK, disk_number, track_number, sector_header->id.c, sector_header->id.h, sector_header->id.r);
	}
	// side number is valid ?
	if (sector_header->id.h > 1) {
		result->SetWarn(DiskResult::ERRV_ID_SIDE, disk_number, track_number, sector_header->id.c, sector_header->id.h, sector_header->id.r);
	}
	int sector_number = sector_header->id.r;
	// sector number is valid ?
	if (sector_number <= 0) {
		result->SetWarn(DiskResult::ERRV_ID_SECTOR, disk_number, track_number, sector_header->id.c, sector_header->id.h, sector_header->id.r, sector_nums);
	}
	// invalid sector size
	if (sector_header->size > 2048) {
		result->SetError(DiskResult::ERRV_SECTOR_SIZE, disk_number, sector_header->id.c, sector_header->id.h, sector_header->id.r, sector_header->id.n, sector_header->size);
	} else if (sector_header->size == 0) {
		result->SetWarn(DiskResult::ERRV_SECTOR_SIZE, disk_number, sector_header->id.c, sector_header->id.h, sector_header->id.r, sector_header->id.n, sector_header->size);
	}

	// 追加
	size_t data_size = sector_header->size;
	if (result->GetValid() >= 0) {
		wxUint8 *sector_data = NULL;
		if (data_size > 0) {
			sector_data = new wxUint8[data_size];
			istream.Read((void *)sector_data, data_size);
		}
		DiskD88Sector *sector = new DiskD88Sector(sector_number, sector_header, sector_data);
		track->Add(sector);

	} else {
		data_size = 0;
		delete sector_header;
	}

	// このセクタデータのサイズを返す
	return (wxUint32)(header_size + data_size);
}

/// トラックデータの解析
wxUint32 DiskD88Parser::ParseTrack(wxInputStream &istream, size_t start_pos, int offset_pos, wxUint32 offset, int disk_number, int track_size, DiskD88Disk *disk)
{
	int track_number = 0;
	int side_number = 0;
	int sector_nums = 0;
	int sector_size = 0;

	istream.SeekI(start_pos + offset, wxFromStart);

	PreParseSectors(istream, disk_number, track_number, side_number, sector_nums, sector_size);

	// セクタ数が多すぎる
	if (sector_nums > 64) {
		result->SetWarn(DiskResult::ERRV_TOO_MANY_SECTORS, disk_number, 64, track_number, side_number, sector_nums);
		sector_nums = 64;
	}
	// セクタがない場合
	if (sector_nums == 0) {
		track_number = -1;
		side_number = -1;
	}

	DiskD88Track *track = new DiskD88Track(disk, track_number, side_number, offset_pos, 1);

	// sectors
	wxUint32 sector_total_size = 0;
	for(int sec_pos = 0; sec_pos < sector_nums && result->GetValid() >= 0; sec_pos++) {
		sector_total_size += ParseSector(istream, disk_number, track_number, sector_nums, sector_size, track);
	}

	// sector number is valid ?
	DiskD88Sectors *sectors = track->GetSectors(); 
	if (sectors && sector_nums != (int)sectors->Count()) {
		result->SetWarn(DiskResult::ERRV_ID_NUM_OF_SECTOR, disk_number, track_number, side_number);
	}

	if (result->GetValid() >= 0) {
		// インターリーブの計算
		track->CalcInterleave();
	}

	if (result->GetValid() >= 0) {
		// セクタの重複や存在をチェック
		if (sectors) {
			wxArrayInt arr;
			for(size_t sec_pos = 0; sec_pos < sectors->Count(); sec_pos++) {
				DiskD88Sector *s = sectors->Item(sec_pos);
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
	}

	if (result->GetValid() >= 0 && track_number >= 0) {
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
						if (sector_size >= 256) {
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
	}

	if (result->GetValid() >= 0) {
		// 残りデータ
		if ((int)sector_total_size < track_size) {
			size_t size = (size_t)(track_size - (int)sector_total_size);
			wxUint8 *buf = new wxUint8[size];
			istream.Read(buf, size);
			track->SetExtraData(buf, size);
		}

		// トラックサイズ設定
		track->SetSize(track_size);
		// インターリーブ設定
		if (disk->GetInterleave() < track->GetInterleave()) {
			disk->SetInterleave(track->GetInterleave());
		}
		// ディスクに追加
		disk->Add(track);
	} else {
		delete track;
	}

	return track_size;
}

/// ディスクデータの解析
/// @return ディスクサイズ
wxUint32 DiskD88Parser::ParseDisk(wxInputStream &istream, size_t start_pos, int disk_number)
{
	d88_header_t *disk_header = new d88_header_t;

	bool valid_header = false;
	wxUint32 size = 0;

	do {
		// seek 
		istream.SeekI(start_pos);

		wxUint32 header_size = (wxUint32)istream.Read((void *)disk_header, sizeof(d88_header_t)).LastRead();
		size = header_size;

		// EOF(0x1a)ならスキップ
		wxByte *p = (wxByte *)disk_header;
		bool all_eot = true;
		for(wxUint32 pos = 0; pos < header_size; pos++) {
			if (p[pos] != 0x1a) {
				all_eot = false;
				break;
			}
		}
		if (all_eot) {
			break;
		}

		// ディスクサイズが小さすぎる
		if (header_size < sizeof(d88_header_t)) {
			result->SetWarn(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
			break;
		}

		wxUint32 disk_size = disk_header->disk_size;
		disk_size = wxUINT32_SWAP_ON_BE(disk_size);
		// ディスクサイズが小さすぎる
		if (disk_size < sizeof(d88_header_t)) {
			result->SetWarn(DiskResult::ERRV_DISK_TOO_SMALL, disk_number);
			break;
		}

		wxUint32 stream_size = (wxUint32)istream.GetLength();
		// ディスクサイズがファイルサイズより大きい、または4MBを超えている
		if (stream_size < disk_size || (1024*1024*4) < disk_size) {
			result->SetWarn(DiskResult::ERRV_DISK_TOO_LARGE, disk_number);
			disk_size = stream_size;
		}

		size = disk_size;

		// 名前の17文字目は'\0'
		if (disk_header->diskname[16] != '\0') {
			result->SetWarn(DiskResult::ERRV_DISK_HEADER, disk_number);
		}

		// オフセット部分の最小値を求める
		// -> 古いd88はオフセット部分が少ない
		wxUint32 offset_start = (wxUint32)-1;
		for(int pos = 0; pos < (DISKD88_MAX_TRACKS - 16); pos++) {
			wxUint32 offset = disk_header->offsets[pos];
			offset = wxUINT32_SWAP_ON_BE(offset);
			if (offset_start > offset && offset > 0) {
				offset_start = offset;
			}
		}
		// オフセットなし（トラックなしの場合）初期値をセット
		if (offset_start == (wxUint32)-1) {
			offset_start = (wxUint32)sizeof(d88_header_t);
		}

		// オーバーしている部分は0にする
		int max_tracks = DISKD88_MAX_TRACKS - ((int)sizeof(d88_header_t) - (int)offset_start) / 4;
		if (max_tracks < 0 || max_tracks > DISKD88_MAX_TRACKS) {
			// トラック数がおかしい
			result->SetWarn(DiskResult::ERRV_INVALID_DISK, disk_number);
			break;
		}

		for(int pos = max_tracks; pos < DISKD88_MAX_TRACKS; pos++) {
			disk_header->offsets[pos] = 0;
		}

		// オフセットから各トラックのサイズを計算する
		DiskD88ParseOffsets offsets;
		for(int pos = 0; pos < max_tracks; pos++) {
			wxUint32 offset = disk_header->offsets[pos];
			offset = wxUINT32_SWAP_ON_BE(offset);
			if (offset >= offset_start) {
				offsets.Add(DiskD88ParseOffset(pos, offset, 0));
			}
		}
		offsets.Sort(&DiskD88ParseOffset::CmpByOffset);
		int offsets_count = (int)offsets.Count();
		for(int pos = 0; pos < offsets_count-1; pos++) {
			DiskD88ParseOffset *curr = &offsets.Item(pos);
			DiskD88ParseOffset *next = &offsets.Item(pos + 1);
			curr->SetSize(next->GetOffset() - curr->GetOffset());
		}
		if (offsets_count >= 1) {
			DiskD88ParseOffset *curr = &offsets.Item(offsets_count - 1);
			curr->SetSize(disk_size - curr->GetOffset());
		}
		offsets.Sort(&DiskD88ParseOffset::CmpByNum);

		//
		// ディスクの作成
		//

		valid_header = true;
		DiskD88Disk *disk = new DiskD88Disk(file, disk_number, disk_header);

		disk->SetOffsetStart(offset_start);

		// parse tracks
		for(int pos = 0; pos < offsets_count && result->GetValid() >= 0; pos++) {
			DiskD88ParseOffset *curr = &offsets.Item(pos);

			// オフセットがディスクサイズを超えている？
			if (curr->GetOffset() >= disk_size) {
				result->SetWarn(DiskResult::ERRV_OVERFLOW_OFFSET, disk_number, curr->GetNum(), curr->GetOffset(), disk_size);
				disk->SetOffset(curr->GetNum(), 0);
				continue;
			}

			ParseTrack(istream, start_pos, curr->GetNum(), curr->GetOffset(), disk_number, curr->GetSize(), disk);
			disk->SetMaxTrackNumber(curr->GetNum());
		}

		if (result->GetValid() >= 0) {
			// ディスクを追加
			disk->CalcMajorNumber();
			file->Add(disk, mod_flags);
			// セクタ数をチェック
			DiskD88Tracks *tracks = disk->GetTracks();
			if (tracks) {
				for(size_t pos = 0; pos < tracks->Count(); pos++) {
					DiskD88Track *track = tracks->Item(pos);
					DiskD88Sectors *sectors = track->GetSectors();
					if (sectors) {
						if ((int)sectors->Count() < disk->GetSectorsPerTrack()) {
							result->SetWarn(DiskResult::ERRV_SHORT_SECTORS, disk_number, disk->GetSectorsPerTrack(), track->GetTrackNumber(), track->GetSideNumber(), (int)sectors->Count());
						}
					}
				}
			}
		} else {
			delete disk;
		}
	} while(0);

	if (!valid_header) {
		delete disk_header;
	}

	return size;
}

/// D88ファイルを解析
/// @param [in] istream    解析対象データ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskD88Parser::Parse(wxInputStream &istream)
{
	size_t read_size = 0;
	size_t stream_size = istream.GetLength();
	int disk_number = (int)file->Count();
	// ディスクサイズが0
	if (stream_size == 0) {
		result->SetError(DiskResult::ERRV_DISK_SIZE_ZERO, disk_number);
		return result->GetValid();
	}
	for(; read_size < stream_size && result->GetValid() >= 0; disk_number++) {
		wxUint32 size = ParseDisk(istream, read_size, disk_number);
		if (size == 0) break;
		read_size += size;
	}
	return result->GetValid();
}

/// チェック しない
/// @return 0 
int DiskD88Parser::Check(wxInputStream &istream)
{
	return 0;
}
