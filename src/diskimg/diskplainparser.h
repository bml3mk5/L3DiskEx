/// @file diskplainparser.h
///
/// @brief べたディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKPLAIN_PARSER_H
#define DISKPLAIN_PARSER_H

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
class DiskTypeHints;

/// べたディスクパーサー
class DiskPlainParser : public DiskImageParser
{
protected:
	void ParseInterleave(DiskImageTrack *track, int interleave, int sector_offset);
	wxUint32 ParseSector(wxInputStream &istream, int disk_number, const DiskParam *disk_param, int track_number, int side_number, int sector_number, int sector_nums, int sector_size, bool is_dummy, DiskImageTrack *track);
	wxUint32 ParseTrack(wxInputStream &istream, int offset_pos, wxUint32 offset, int disk_number, const DiskParam *disk_param, int track_number, int side_number, bool is_dummy_side, DiskImageDisk *disk);
	virtual wxUint32 ParseDisk(wxInputStream &istream, int disk_number, const DiskParam *disk_param);

	void CalcParamFromSize(int disk_size, DiskParam &disk_param);

	virtual int Check(wxInputStream &istream);

public:
	DiskPlainParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	virtual ~DiskPlainParser();

	/// チェック
	virtual int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	/// 解析
	virtual int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* DISKPLAIN_PARSER_H */
