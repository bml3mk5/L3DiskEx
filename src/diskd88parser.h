/// @file diskd88parser.h
///
/// @brief D88ディスクイメージパーサ
///
#ifndef _DISKD88_PARSER_H_
#define _DISKD88_PARSER_H_

#include "common.h"
#include <wx/wx.h>
#include "result.h"

class DiskD88Track;
class DiskD88Disk;
class DiskD88File;

/// パース結果
class DiskD88Result : public ResultInfo
{
public:
	DiskD88Result() : ResultInfo() {}

	enum {
		ERR_NONE = 0,
		ERR_CANNOT_OPEN,
		ERR_CANNOT_SAVE,
		ERR_INVALID_DISK,
		ERR_OVERFLOW,
		ERR_ID_TRACK,
		ERR_ID_SIDE,
		ERR_ID_SECTOR,
		ERR_SECTOR_SIZE,
		ERR_DUPLICATE_TRACK,
		ERR_IGNORE_DATA,
		ERR_NO_DATA,
		ERR_NO_DISK,
		ERR_NO_TRACK,
		ERR_NO_FOUND_TRACK,
		ERR_REPLACE,
		ERR_FILE_ONLY_1S,
		ERR_FILE_SAME,
	};

	void SetMessage(int error_number, va_list ap);
};

/// D88ディスクパーサー
class DiskD88Parser
{
private:
	wxInputStream *stream;
	DiskD88File *file;
	DiskD88Result *result;

	wxUint32 ParseSector(int disk_number, int track_number, int sector_nums, DiskD88Track *track);
	wxUint32 ParseTrack(size_t start_pos, int offset_pos, wxUint32 offset, int disk_number, DiskD88Disk *disk);
	wxUint32 ParseDisk(size_t start_pos, int disk_number);

public:
	DiskD88Parser(wxInputStream *stream, DiskD88File *file, DiskD88Result &result);
	~DiskD88Parser();

	int Parse();
};

#endif /* _DISKD88_PARSER_H_ */
