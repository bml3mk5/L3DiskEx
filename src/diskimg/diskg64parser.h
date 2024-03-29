/// @file diskg64parser.h
///
/// @brief Commodore G64 ディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKG64_PARSER_H
#define DISKG64_PARSER_H

#include "../common.h"
#include "diskparser.h"
#include "../utils.h"


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
class DiskG64Parser : public DiskImageParser
{
private:
	g64_header_t	 m_header;

	/// セクタデータの作成
	wxUint32 ParseSector(wxUint8 *indata, int disk_number, int track_number, int side_number, int sector_nums, int sector_number, int sector_size, bool single_density, DiskImageTrack *track);
	/// トラックデータの作成
	int ParseTrack(wxInputStream &istream, int disk_number, int side_number, int offset_pos, wxUint32 offset, DiskImageDisk *disk);
	/// ディスクの解析
	int ParseDisk(wxInputStream &istream, int disk_number);
	/// ヘッダ解析
	int ParseHeader(wxInputStream &istream, int disk_number);
	/// GCRデータをデコード
	static size_t DecodeGCR(const wxUint8 *indata, int inbitlen, wxUint8 *outdata, size_t outlen);

	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);

public:
	DiskG64Parser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskG64Parser();

	/// チェック
	int Check(wxInputStream &istream);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param = NULL);
};

#endif /* DISKG64_PARSER_H */
