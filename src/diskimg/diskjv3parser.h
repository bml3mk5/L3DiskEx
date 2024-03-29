/// @file diskjv3parser.h
///
/// @brief TRS-80 JV3形式 ディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKJV3_PARSER_H
#define DISKJV3_PARSER_H

#include "../common.h"

class wxInputStream;
class wxArrayString;
class DiskImageTrack;
class DiskImageDisk;
class DiskImageFile;
class DiskParser;
class DiskParam;
class DiskParamPtrs;
class DiskResult;
class FileParam;

/// TRS-80 JV3ディスクパーサー
class DiskJV3Parser
{
private:
	DiskImageFile	*p_file;
	short			 m_mod_flags;
	DiskResult		*p_result;

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int track_number, int side_number, int sector_number, int sector_size, int sector_nums, bool single_density, DiskImageTrack *track);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

public:
	DiskJV3Parser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskJV3Parser();

	/// チェック
	int Check(wxInputStream &istream);
	int Parse(wxInputStream &istream);
};

#endif /* DISKJV3_PARSER_H */
