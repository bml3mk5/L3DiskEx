/// @file diskdskparser.h
///
/// @brief CPC DSKディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKDSK_PARSER_H
#define DISKDSK_PARSER_H

#include "../common.h"
#include "diskparser.h"


class wxInputStream;
class DiskImageTrack;
class DiskImageDisk;
class DiskImageFile;
class DiskResult;
class FileParamFormat;

/// CPC DSKディスクパーサー
class DiskDskParser : public DiskImageParser
{
private:
	int				 m_is_extended;

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int sector_nums, void *user_data, DiskImageTrack *track);
	/// トラックデータの作成
	wxUint32 ParseTrack(wxInputStream &istream, int track_size, int offset_pos, wxUint32 offset, DiskImageDisk *disk);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);

public:
	DiskDskParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskDskParser();

	/// CPC DSKファイルかどうかをチェック
	int Check(wxInputStream &istream);
	/// CPC DSKファイルを解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param = NULL);
};

#endif /* DISKDSK_PARSER_H */
