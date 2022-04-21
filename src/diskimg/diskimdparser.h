/// @file diskimdparser.h
///
/// @brief IMageDisk imdディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKIMD_PARSER_H_
#define _DISKIMD_PARSER_H_

#include "../common.h"
#include "diskplainparser.h"


class wxInputStream;
class wxOutputStream;
class wxArrayString;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskParser;
class DiskParam;
class DiskParamPtrs;
class DiskResult;
class FileParam;

/// IMageDisk imdディスクパーサ
class DiskIMDParser
{
private:
	DiskD88File	*file;
	short		mod_flags;
	DiskResult	*result;

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int disk_number, int track_number, int side_number, int sector_nums, int sector_number, int sector_size, bool single_density, DiskD88Track *track);
	/// トラックデータの作成
	int ParseTrack(wxInputStream &istream, int disk_number, int offset_pos, wxUint32 offset, DiskD88Disk *disk);
	/// ディスクの解析
	int ParseDisk(wxInputStream &istream, int disk_number);

public:
	DiskIMDParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskIMDParser();

	/// チェック
	int Check(DiskParser &dp, wxInputStream &istream);
	int Parse(wxInputStream &istream);
};

#endif /* _DISKIMD_PARSER_H_ */
