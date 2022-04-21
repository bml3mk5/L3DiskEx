/// @file diskvfdparser.h
///
/// @brief Virtual98 FDディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKVFD_PARSER_H
#define DISKVFD_PARSER_H

#include "../common.h"
#include "diskplainparser.h"


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


/// Virtual98 FDディスクイメージパーサ
class DiskVFDParser
{
private:
	DiskD88File	*file;
	short		mod_flags;
	DiskResult	*result;

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int sector_nums, void *user_data, DiskD88Track *track);
	/// トラックデータの作成
	wxUint32 ParseTrack(wxInputStream &istream, void *user_data, int offset_pos, wxUint32 offset, DiskD88Disk *disk);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

public:
	DiskVFDParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskVFDParser();

	/// チェック
	int Check(wxInputStream &istream);
	int Parse(wxInputStream &istream);
};

#endif /* DISKVFD_PARSER_H */
