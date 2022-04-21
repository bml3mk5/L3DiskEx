/// @file diskdskparser.h
///
/// @brief CPC DSKディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKDSK_PARSER_H_
#define _DISKDSK_PARSER_H_

#include "common.h"


class wxInputStream;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskResult;
class FileParamFormat;

/// CPC DSKディスクパーサー
class DiskDskParser
{
private:
	DiskD88File	*file;
	short		mod_flags;
	DiskResult	*result;
	int			is_extended;

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int sector_nums, void *user_data, DiskD88Track *track);
	/// トラックデータの作成
	wxUint32 ParseTrack(wxInputStream &istream, int track_size, int offset_pos, wxUint32 offset, DiskD88Disk *disk);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

public:
	DiskDskParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskDskParser();

	/// CPC DSKファイルかどうかをチェック
	int Check(wxInputStream &istream);
	/// CPC DSKファイルを解析
	int Parse(wxInputStream &istream);
};

#endif /* _DISKDSK_PARSER_H_ */
