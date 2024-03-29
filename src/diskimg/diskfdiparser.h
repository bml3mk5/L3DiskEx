/// @file diskfdiparser.h
///
/// @brief FDIディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKFDI_PARSER_H
#define DISKFDI_PARSER_H

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

/// FDIディスクパーサー
class DiskFDIParser : public DiskPlainParser
{
private:
//	void CalcParamFromSize(int disk_size, DiskParam &disk_param);

public:
	DiskFDIParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskFDIParser();

	/// チェック
	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* DISKFDI_PARSER_H */
