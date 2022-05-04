/// @file disk2mgparser.h
///
/// @brief 2MG ディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISK2MG_PARSER_H_
#define _DISK2MG_PARSER_H_

#include "../common.h"
#include "diskplainparser.h"


class wxInputStream;
class wxArrayString;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
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
	Disk2MGParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~Disk2MGParser();

	/// チェック
	int Check(DiskParser &dp, wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* _DISK2MG_PARSER_H_ */
