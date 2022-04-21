/// @file diskplainparser.h
///
/// @brief べたディスクイメージパーサ
///
#ifndef _DISKPLAIN_PARSER_H_
#define _DISKPLAIN_PARSER_H_

#include "common.h"

class wxInputStream;
class wxArrayString;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskParam;
class DiskResult;
class FileParam;

/// べたディスクパーサー
class DiskPlainParser
{
private:
	DiskD88File		*file;
	short			mod_flags;
	DiskResult		*result;

	wxUint32 ParseSector(wxInputStream *istream, int disk_number, int track_number, int side_number, int sector_number, int sector_nums, int sector_size, DiskD88Track *track);
	wxUint32 ParseTrack(wxInputStream *istream, int offset_pos, wxUint32 offset, int disk_number, int track_number, int side_number, int sector_nums, int sector_size, DiskD88Disk *disk);
	wxUint32 ParseDisk(wxInputStream *istream, int disk_number, int track_nums, int side_nums, int sector_nums, int sector_size);

public:
	DiskPlainParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskPlainParser();

	int Parse(wxInputStream *istream, const wxArrayString *disk_hints, const DiskParam *disk_param);
};

#endif /* _DISKPLAIN_PARSER_H_ */
