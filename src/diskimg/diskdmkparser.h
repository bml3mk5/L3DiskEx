/// @file diskdmkparser.h
///
/// @brief TRS-80 DMKディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKDMK_PARSER_H_
#define _DISKDMK_PARSER_H_

#include "../common.h"


class wxInputStream;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskResult;
class FileParamFormat;

/// TRS-80 DMKディスクパーサー
class DiskDmkParser
{
private:
	DiskD88File	*file;
	short		mod_flags;
	DiskResult	*result;
//	int			is_extended;

	/// データマークをさがす
	bool FindDataMark(wxInputStream &istream, int sector_size, bool double_density, int &deleted);

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int sector_nums, int flags, DiskD88Track *track);
	/// トラックデータの作成
	wxUint32 ParseTrack(wxInputStream &istream, int track_size, int offset_pos, wxUint32 offset, DiskD88Disk *disk);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

public:
	DiskDmkParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskDmkParser();

	/// TRS-80 DMKファイルかどうかをチェック
	int Check(wxInputStream &istream);
	/// TRS-80 DMKファイルを解析
	int Parse(wxInputStream &istream);
};

#endif /* _DISKDMK_PARSER_H_ */
