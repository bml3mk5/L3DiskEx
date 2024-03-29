/// @file diskdmkparser.h
///
/// @brief TRS-80 DMKディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKDMK_PARSER_H
#define DISKDMK_PARSER_H

#include "../common.h"
#include "diskparser.h"


class wxInputStream;
class DiskImageTrack;
class DiskImageDisk;
class DiskImageFile;
class DiskResult;
class FileParamFormat;

/// TRS-80 DMKディスクパーサー
class DiskDmkParser : public DiskImageParser
{
private:
	/// データマークをさがす
	bool FindDataMark(wxInputStream &istream, int sector_size, bool double_density, int &deleted);

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int sector_nums, int flags, DiskImageTrack *track);
	/// トラックデータの作成
	wxUint32 ParseTrack(wxInputStream &istream, int track_size, int offset_pos, wxUint32 offset, DiskImageDisk *disk);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);

public:
	DiskDmkParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskDmkParser();

	/// TRS-80 DMKファイルかどうかをチェック
	int Check(wxInputStream &istream);
	/// TRS-80 DMKファイルを解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param = NULL);
};

#endif /* DISKDMK_PARSER_H */
