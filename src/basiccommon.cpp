/// @file basiccommon.cpp
///
/// @brief disk basic common functions
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basiccommon.h"


//
//
//
DiskBasicFileType::DiskBasicFileType()
{
	format = FORMAT_TYPE_NONE;
	type = 0;
	origin = 0;
}

/// @param [in] n_format   : フォーマット
/// @param [in] n_type     : enum #en_file_type_mask の値の組み合わせ
/// @param [in] n_origin   : 本来の属性
DiskBasicFileType::DiskBasicFileType(DiskBasicFormatType n_format, int n_type, int n_origin)
{
	format = n_format;
	type = n_type;
	origin = n_origin;
}

DiskBasicFileType::~DiskBasicFileType()
{
}

bool DiskBasicFileType::MatchType(int mask, int value) const
{
	return ((type & mask) == value);
}

bool DiskBasicFileType::UnmatchType(int mask, int value) const
{
	return ((type & mask) != value);
}

bool DiskBasicFileType::IsVolume() const
{
	return ((type & (FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK)) == FILE_TYPE_VOLUME_MASK);
}

bool DiskBasicFileType::IsDirectory() const
{
	return ((type & (FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK)) == FILE_TYPE_DIRECTORY_MASK);
}
