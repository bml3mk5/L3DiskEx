/// @file disk2mgparser.h
///
/// @brief 2MG ディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISK2MG_PARSER_H
#define DISK2MG_PARSER_H

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

/// 2MGディスクパーサー
class Disk2MGParser : public DiskPlainParser
{
private:
//	void CalcParamFromSize(int disk_size, DiskParam &disk_param);

public:
	Disk2MGParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~Disk2MGParser();

	/// チェック
	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* DISK2MG_PARSER_H */
