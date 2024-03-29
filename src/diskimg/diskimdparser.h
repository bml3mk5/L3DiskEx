/// @file diskimdparser.h
///
/// @brief IMageDisk imdディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKIMD_PARSER_H
#define DISKIMD_PARSER_H

#include "../common.h"
#include "diskparser.h"


class wxInputStream;
class wxOutputStream;
class wxArrayString;
class DiskImageTrack;
class DiskImageDisk;
class DiskImageFile;
class DiskParser;
class DiskParam;
class DiskParamPtrs;
class DiskResult;
class FileParam;

/// IMageDisk imdディスクパーサ
class DiskIMDParser : public DiskImageParser
{
private:
	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int disk_number, int track_number, int side_number, int sector_nums, int sector_number, int sector_size, bool single_density, DiskImageTrack *track);
	/// トラックデータの作成
	int ParseTrack(wxInputStream &istream, int disk_number, int offset_pos, wxUint32 offset, DiskImageDisk *disk);
	/// ディスクの解析
	int ParseDisk(wxInputStream &istream, int disk_number);

	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);

public:
	DiskIMDParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskIMDParser();

	/// チェック
	int Check(wxInputStream &istream);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param = NULL);
};

#endif /* DISKIMD_PARSER_H */
