/// @file diskdimparser.h
///
/// @brief DIFC.X DIMディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKDIM_PARSER_H
#define DISKDIM_PARSER_H

#include "../common.h"
#include "diskplainparser.h"


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

/// DIFC.X DIMディスクパーサー
class DiskDIMParser : public DiskPlainParser
{
private:
//	void CalcParamFromSize(int disk_size, DiskParam &disk_param);

public:
	DiskDIMParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskDIMParser();

	/// チェック
	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* DISKDIM_PARSER_H */
