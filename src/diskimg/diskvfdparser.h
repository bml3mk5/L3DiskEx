/// @file diskvfdparser.h
///
/// @brief Virtual98 FDディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKVFD_PARSER_H
#define DISKVFD_PARSER_H

#include "../common.h"
#include "diskparser.h"


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


/// Virtual98 FDディスクイメージパーサ
class DiskVFDParser : public DiskImageParser
{
private:
	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int sector_nums, void *user_data, DiskImageTrack *track);
	/// トラックデータの作成
	wxUint32 ParseTrack(wxInputStream &istream, void *user_data, int offset_pos, wxUint32 offset, DiskImageDisk *disk);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);

public:
	DiskVFDParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskVFDParser();

	/// チェック
	int Check(wxInputStream &istream);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param = NULL);
};

#endif /* DISKVFD_PARSER_H */
