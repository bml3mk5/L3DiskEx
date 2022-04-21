/// @file diskg64parser.h
///
/// @brief Commodore G64 ディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKG64_PARSER_H_
#define _DISKG64_PARSER_H_

#include "../common.h"
#include "../utils.h"


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


#pragma pack(1)
/// G64 ヘッダ
typedef struct st_g64_header {
	wxUint8  sig[8];
	wxUint8  version;
	wxUint8  num_of_tracks;
	wxUint16 max_track_size;
} g64_header_t;
#pragma pack()


/// Commodore G64 ディスクパーサ
class DiskG64Parser
{
private:
	DiskD88File	*file;
	short		mod_flags;
	DiskResult	*result;
	g64_header_t header;

	/// セクタデータの作成
	wxUint32 ParseSector(wxUint8 *indata, int disk_number, int track_number, int side_number, int sector_nums, int sector_number, int sector_size, bool single_density, DiskD88Track *track);
	/// トラックデータの作成
	int ParseTrack(wxInputStream &istream, int disk_number, int side_number, int offset_pos, wxUint32 offset, DiskD88Disk *disk);
	/// ディスクの解析
	int ParseDisk(wxInputStream &istream, int disk_number);
	/// ヘッダ解析
	int ParseHeader(wxInputStream &istream, int disk_number);
	/// GCRデータをデコード
	static size_t DecodeGCR(const wxUint8 *indata, int inbitlen, wxUint8 *outdata, size_t outlen);

public:
	DiskG64Parser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskG64Parser();

	/// チェック
	int Check(DiskParser &dp, wxInputStream &istream);
	int Parse(wxInputStream &istream);
};

#endif /* _DISKG64_PARSER_H_ */
