/// @file diskplainparser.cpp
///
/// @brief べたディスクパーサー
///
#include "diskplainparser.h"
#include "diskd88.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"

//
//
//
DiskPlainParser::DiskPlainParser(DiskD88File *file, short mod_flags, DiskResult *result)
{
	this->file = file;
	this->mod_flags = mod_flags;
	this->result = result;
}

DiskPlainParser::~DiskPlainParser()
{
}

/// セクタデータの解析
wxUint32 DiskPlainParser::ParseSector(wxInputStream *istream, int disk_number, int track_number, int side_number, int sector_number, int sector_nums, int sector_size, bool single_density, DiskD88Track *track)
{
	DiskD88Sector *sector = new DiskD88Sector(track_number, side_number, sector_number, sector_size, sector_nums, single_density);
	track->Add(sector);

	wxUint8 *buf = sector->GetSectorBuffer();
	int siz = sector->GetSectorBufferSize();

	size_t len = istream->Read(buf, siz).LastRead();
	if (len == 0) {
		// ファイルデータが足りない
		result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
	}

	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + siz;
}

/// トラックデータの解析
wxUint32 DiskPlainParser::ParseTrack(wxInputStream *istream, int offset_pos, wxUint32 offset, int disk_number, int track_number, int side_number, int sector_nums, int sector_size, bool single_density, DiskD88Disk *disk)
{
	DiskD88Track *track = new DiskD88Track(disk, track_number, side_number, offset_pos, 1);

	wxUint32 track_size = 0;
	for(int sector_number = 1; sector_number <= sector_nums && result->GetValid() >= 0; sector_number++) {
		track_size += ParseSector(istream, disk_number, track_number, side_number, sector_number, sector_nums, sector_size, single_density, track);
	}

	if (result->GetValid() >= 0) {
		// トラックを追加
		track->SetSize(track_size);
		disk->Add(track);
		disk->SetOffset(offset_pos, offset);
	} else {
		delete track;
	}

	return track_size;
}

/// ディスクデータの解析
wxUint32 DiskPlainParser::ParseDisk(wxInputStream *istream, int disk_number, const DiskParam *disk_param)
{
	DiskD88Disk *disk = new DiskD88Disk(file);

	wxUint32 offset = (int)sizeof(d88_header_t);
	int offset_pos = 0;
	for(int track_num = 0; track_num < disk_param->GetTracksPerSide() && result->GetValid() >= 0; track_num++) {
		for(int side_num = 0; side_num < disk_param->GetSidesPerDisk() && result->GetValid() >= 0; side_num++) {
			int sector_nums = disk_param->GetSectorsPerTrack();
			int sector_size = disk_param->GetSectorSize();
			bool single_density = disk_param->FindSingleDensity(track_num, side_num, &sector_nums, &sector_size);
			offset += ParseTrack(istream, offset_pos, offset, disk_number, track_num, side_num, sector_nums, sector_size, single_density, disk); 
			offset_pos++;
			if (offset_pos >= DISKD88_MAX_TRACKS) {
				result->SetError(DiskResult::ERRV_OVERFLOW_SIZE, 0, offset);
			}
		}
	}
	disk->SetSize(offset);

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

	return offset;
}

/// ベタファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_hints ディスクパラメータのヒント
/// @param [in] disk_param ディスクパラメータ disk_hints指定時はNullable
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskPlainParser::Parse(wxInputStream *istream, const wxArrayString *disk_hints, const DiskParam *disk_param)
{
	/// パラメータ
	if (disk_hints != NULL && disk_param == NULL) {
		for(size_t i=0; i<disk_hints->Count(); i++) {
			wxString hint = disk_hints->Item(i);
			DiskParam *param = gDiskTypes.Find(hint);
			if (param) {
				int disk_size_hint = param->CalcDiskSize();
				if (istream->GetLength() == disk_size_hint) {
					// ファイルサイズが一致
					disk_param = param;
					break;
				}
			}
		}
	}
	if (!disk_param) {
		result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return result->GetValid();
	}

	ParseDisk(istream
		, 0
		, disk_param);

	return result->GetValid();
}
