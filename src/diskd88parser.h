/// @file diskd88parser.h
///
/// @brief D88ディスクイメージパーサ
///
#ifndef _DISKD88_PARSER_H_
#define _DISKD88_PARSER_H_

#include "common.h"

class wxInputStream;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskResult;
class FileParam;

/// D88ディスクパーサー
class DiskD88Parser
{
private:
	DiskD88File		*file;
	short			mod_flags;
	DiskResult		*result;

	wxUint32 ParseSector(wxInputStream *istream, int disk_number, int track_number, int &sector_nums, DiskD88Track *track);
	wxUint32 ParseTrack(wxInputStream *istream, size_t start_pos, int offset_pos, wxUint32 offset, int disk_number, DiskD88Disk *disk);
	wxUint32 ParseDisk(wxInputStream *istream, size_t start_pos, int disk_number);

public:
	DiskD88Parser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskD88Parser();

	int Parse(wxInputStream *istream);
};

#endif /* _DISKD88_PARSER_H_ */
