/// @file diskresult.h
///
/// @brief ディスク解析結果
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISK_RESULT_H_
#define _DISK_RESULT_H_

#include "../common.h"
#include "../result.h"


extern const char *gDiskResultMsgs[];

/// パース結果
class DiskResult : public ResultInfo
{
public:
	DiskResult() : ResultInfo() {}

	enum {
		ERR_NONE = 0,
		// 引数なしのメッセージ
		ERR_CANNOT_OPEN,
		ERR_CANNOT_SAVE,
		ERR_NO_DATA,
		ERR_NO_DISK,
		ERR_NO_TRACK,
		ERR_REPLACE,
		ERR_FILE_ONLY_1S,
		ERR_FILE_SAME,
		ERR_INTERLEAVE,
		ERR_TOO_LARGE,
		ERR_WRITE_PROTECTED,
		ERR_UNSUPPORTED,

		ERRV_START,
		// 引数あり（フォーマットあり）のメッセージ
		ERRV_INVALID_DISK,
		ERRV_DISK_SIZE_ZERO,
		ERRV_DISK_TOO_SMALL,
		ERRV_DISK_TOO_LARGE,
		ERRV_DISK_HEADER,
		ERRV_OVERFLOW_OFFSET,
		ERRV_OVERFLOW_SIZE,
		ERRV_ID_TRACK,
		ERRV_ID_SIDE,
		ERRV_ID_SECTOR,
		ERRV_TRACKS_HEADER,
		ERRV_SIDES_HEADER,
		ERRV_SECTORS_HEADER,
		ERRV_ID_NUM_OF_SECTOR,
		ERRV_TOO_MANY_SECTORS,
		ERRV_SHORT_SECTORS,
		ERRV_SECTOR_SIZE_HEADER,
		ERRV_SECTOR_SIZE_SECTOR,
		ERRV_DUPLICATE_TRACK,
		ERRV_DUPLICATE_SECTOR,
		ERRV_NO_SECTOR,
		ERRV_IGNORE_DATA,
		ERRV_TOO_MANY_TRACKS,
		ERRV_UNSUPPORTED_TYPE,
		ERRV_UNSUPPORTED_TRACK_TYPE,
		ERRV_END
	};

	void SetMessageV(int error_number, va_list ap);
};

#endif /* _DISK_RESULT_H_ */
