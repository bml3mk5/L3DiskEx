/// @file disktd0parser.h
///
/// @brief Teledisk td0ディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKTD0_PARSER_H
#define DISKTD0_PARSER_H

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

/// Teledisk td0ディスクパーサ
class DiskTD0Parser : public DiskImageParser
{
private:
	bool			 m_is_compressed;	// TODO: Advanced compress version is not supported.

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int disk_number, int sector_nums, void *user_data, DiskImageTrack *track);
	/// トラックデータの作成
	int ParseTrack(wxInputStream &istream, int disk_number, int offset_pos, wxUint32 offset, DiskImageDisk *disk);
	/// ディスクの解析
	int ParseDisk(wxInputStream &istream, int disk_number);

	int DecodeRepeatedData(wxInputStream &istream, int disk_number, int pos, int slen, int repeat, wxUint8 *buffer, int buflen);
	int DecodePlainData(wxInputStream &istream, int disk_number, int pos, int slen, wxUint8 *buffer, int buflen);
	int DecodeData(wxInputStream &istream, int disk_number, wxUint8 *buffer, int buflen);

	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);

public:
	DiskTD0Parser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskTD0Parser();

	/// チェック
	int Check(wxInputStream &istream);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param = NULL);
};

#endif /* DISKTD0_PARSER_H */
