/// @file common.h
///
/// @brief common definitions
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _COMMON_H_
#define _COMMON_H_

#include <wx/defs.h>
#include <time.h>
#include "version.h"


#ifndef _MAX_PATH
#define _MAX_PATH	260
#endif

#define DEFAULT_TEXTWIDTH 160

size_t rtrim(void *, size_t, char);
size_t str_length(const void *, size_t, char);
void mem_invert(void *, size_t);
size_t str_shrink(void *, size_t);
void mem_copy(const void *src, size_t slen, char fill, void *dst, size_t dlen);
int mem_rchr(const void *, size_t, int);
size_t padding(void *, size_t, char);
void to_upper(void *, size_t);

#if defined(__WXMSW__)

// ignore warning
#ifndef __GNUC__
//#pragma warning(disable:4482)
//#pragma warning(disable:4996)
#endif

#else

//#include "tchar.h"
//#include "typedef.h"

#if defined(__WXOSX__)

wchar_t *_wgetenv(const wchar_t *);
int _wsystem(const wchar_t *);

#endif
#endif

class wxDateTime;

/// 時間構造体を保持するクラス
class TM
{
private:
	struct tm tm;
public:
	TM();
	TM(const struct tm *src);
	~TM();
	/// 時間構造体を初期化
	void Clear();
	/// 日付を初期化
	void ClearDate();
	/// 時間を初期化
	void ClearTime();
	/// 日時をゼロにする
	void AllZero();
	/// 時間構造体を得る
	struct tm *Get() { return &tm; }
	/// 時間構造体を得る
	void Get(wxDateTime &dst) const;
	/// 時間構造体を設定
	void Set(const struct tm *val) { tm = *val; }
	/// 時間構造体を設定
	void Set(const TM &src) { tm = src.tm; }
	/// 時間構造体を設定
	void Set(const wxDateTime &src);
	/// 時間構造体を代入
	TM &operator=(const TM &src);
	/// 時間構造体を返す
	operator struct tm *() { return &tm; }
	/// 現在日時を得る
	TM &Now();
	/// 現在日時を得る
	static TM GetNow();
	/// 日付が有効な値か
	bool IsValidDate() const;
	/// 時間が有効な値か
	bool IsValidTime() const;
	/// 日時設定を無視する値か
	bool Ignorable() const;

	int AddYear(int val) { tm.tm_year += val; return tm.tm_year; }
	int AddMonth(int val) { tm.tm_mon += val; return tm.tm_mon; }

	int GetYear() const { return tm.tm_year; }
	int GetMonth() const { return tm.tm_mon; }
	int GetDay() const { return tm.tm_mday; }
	int GetHour() const { return tm.tm_hour; }
	int GetMinute() const { return tm.tm_min; }
	int GetSecond() const { return tm.tm_sec; }

	void SetYear(int val) { tm.tm_year = val; }
	void SetMonth(int val) { tm.tm_mon = val; }
	void SetDay(int val) { tm.tm_mday = val; }
	void SetHour(int val) { tm.tm_hour = val; }
	void SetMinute(int val) { tm.tm_min = val; }
	void SetSecond(int val) { tm.tm_sec = val; }
};

#endif /* _COMMON_H_ */
