/// @file diskcqmparser.h
///
/// @brief CopyQM imgディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKCQM_PARSER_H_
#define _DISKCQM_PARSER_H_

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
class DiskTypeHints;

/// CopyQMディスクパーサ
class DiskCQMParser : public DiskPlainParser
{
private:
//	void CalcParamFromSize(int disk_size, DiskParam &disk_param);
	size_t ExpandData(wxInputStream &istream, wxOutputStream &ostream);

public:
	DiskCQMParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskCQMParser();

	/// チェック
	int Check(DiskParser &dp, wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* _DISKCQM_PARSER_H_ */
