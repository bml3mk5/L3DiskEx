/// @file diskresult.h
///
/// @brief ディスク解析結果
///
#ifndef _DISK_RESULT_H_
#define _DISK_RESULT_H_

#include "common.h"
#include "result.h"

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
		ERR_NO_FOUND_TRACK,
		ERR_REPLACE,
		ERR_FILE_ONLY_1S,
		ERR_FILE_SAME,
		ERR_INTERLEAVE,
		ERR_TOO_LARGE,
		ERR_UNSUPPORTED,

		ERRV_START,
		// 引数あり（用フォーマット）のメッセージ
		ERRV_INVALID_DISK,
		ERRV_DISK_HEADER,
		ERRV_OVERFLOW_OFFSET,
		ERRV_OVERFLOW_SIZE,
		ERRV_ID_TRACK,
		ERRV_ID_SIDE,
		ERRV_ID_SECTOR,
		ERRV_SECTOR_SIZE,
		ERRV_DUPLICATE_TRACK,
		ERRV_IGNORE_DATA,
		ERRV_END
	};

	void SetMessage(int error_number, va_list ap);
};

#endif /* _DISK_RESULT_H_ */
