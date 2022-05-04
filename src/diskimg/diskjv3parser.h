/// @file diskjv3parser.h
///
/// @brief TRS-80 JV3形式 ディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKJV3_PARSER_H_
#define _DISKJV3_PARSER_H_

#include "../common.h"

class wxInputStream;
class wxArrayString;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskParser;
class DiskParam;
class DiskParamPtrs;
class DiskResult;
class FileParam;

/// TRS-80 JV3ディスクパーサー
class DiskJV3Parser
{
private:
	DiskD88File	*file;
	short		mod_flags;
	DiskResult	*result;

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int track_number, int side_number, int sector_number, int sector_size, int sector_nums, bool single_density, DiskD88Track *track);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

public:
	DiskJV3Parser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskJV3Parser();

	/// チェック
	int Check(wxInputStream &istream);
	int Parse(wxInputStream &istream);
};

#endif /* _DISKJV3_PARSER_H_ */
