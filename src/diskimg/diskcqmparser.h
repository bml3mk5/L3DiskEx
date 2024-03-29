/// @file diskcqmparser.h
///
/// @brief CopyQM imgディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKCQM_PARSER_H
#define DISKCQM_PARSER_H

#include "../common.h"
#include "diskplainparser.h"


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
class DiskTypeHints;

/// CopyQMディスクパーサ
class DiskCQMParser : public DiskPlainParser
{
private:
//	void CalcParamFromSize(int disk_size, DiskParam &disk_param);
	size_t ExpandData(wxInputStream &istream, wxOutputStream &ostream);

public:
	DiskCQMParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskCQMParser();

	/// チェック
	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* DISKCQM_PARSER_H */
