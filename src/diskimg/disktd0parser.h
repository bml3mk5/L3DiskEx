/// @file disktd0parser.h
///
/// @brief Teledisk td0ディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKTD0_PARSER_H_
#define _DISKTD0_PARSER_H_

#include "../common.h"
#include "diskplainparser.h"


class wxInputStream;
class wxOutputStream;
class wxArrayString;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskParser;
class DiskParam;
class DiskParamPtrs;
class DiskResult;
class FileParam;

/// Teledisk td0ディスクパーサ
class DiskTD0Parser
{
private:
	DiskD88File	*file;
	short		mod_flags;
	DiskResult	*result;
	bool		is_compressed;	// TODO: Advanced compress version is not supported.

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int disk_number, int sector_nums, void *user_data, DiskD88Track *track);
	/// トラックデータの作成
	int ParseTrack(wxInputStream &istream, int disk_number, int offset_pos, wxUint32 offset, DiskD88Disk *disk);
	/// ディスクの解析
	int ParseDisk(wxInputStream &istream, int disk_number);

	int DecodeRepeatedData(wxInputStream &istream, int disk_number, int pos, int slen, int repeat, wxUint8 *buffer, int buflen);
	int DecodePlainData(wxInputStream &istream, int disk_number, int pos, int slen, wxUint8 *buffer, int buflen);
	int DecodeData(wxInputStream &istream, int disk_number, wxUint8 *buffer, int buflen);

public:
	DiskTD0Parser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskTD0Parser();

	/// チェック
	int Check(DiskParser &dp, wxInputStream &istream);
	int Parse(wxInputStream &istream);
};

#endif /* _DISKTD0_PARSER_H_ */
