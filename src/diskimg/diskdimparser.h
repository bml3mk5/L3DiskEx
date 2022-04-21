/// @file diskdimparser.h
///
/// @brief DIFC.X DIMディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKDIM_PARSER_H_
#define _DISKDIM_PARSER_H_

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

/// DIMディスクパーサー
class DiskDIMParser : public DiskPlainParser
{
private:
//	void CalcParamFromSize(int disk_size, DiskParam &disk_param);

public:
	DiskDIMParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskDIMParser();

	/// チェック
	int Check(DiskParser &dp, wxInputStream &istream, const wxArrayString *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* _DISKDIM_PARSER_H_ */
