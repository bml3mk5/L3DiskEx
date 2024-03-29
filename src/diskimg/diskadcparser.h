/// @file diskadcparser.h
///
/// @brief Apple Disk Copy ディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKADC_PARSER_H
#define DISKADC_PARSER_H

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

/// Apple Disk Copyディスクパーサー
class DiskADCParser : public DiskPlainParser
{
private:
//	void CalcParamFromSize(int disk_size, DiskParam &disk_param);

public:
	DiskADCParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskADCParser();

	/// チェック
	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* DISKADC_PARSER_H */
