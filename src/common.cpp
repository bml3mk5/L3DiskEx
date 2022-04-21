/// @file common.cpp
///
/// @brief common functions
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "common.h"
#include <stdio.h>
#include <string.h>
#include <wx/datetime.h>


/// 右側の指定文字をトリミング
/// @param [in,out] buf 文字列
/// @param [in]     len 文字列長さ
/// @param [in]     ch  指定文字
/// @return トリミング後の文字列長さ
size_t rtrim(void *buf, size_t len, char ch)
{
	char *p = (char *)buf;
	p = &p[len - 1];
	while(len > 0) {
		if (*p != 0 && *p != ch) break;
		*p = 0;
		len--;
		p--;
	}
	return len;
}

/// 指定文字までの文字列の長さ
/// @param [in]     buf 文字列
/// @param [in]     len 文字列バッファサイズ
/// @param [in]     ch  指定文字
/// @return 文字列長さ
size_t str_length(const void *buf, size_t len, char ch)
{
	const char *p = (const char *)buf;
	for(size_t i=0; i<len; i++) {
		if (p[i] == ch) {
			len = i;
			break;
		}
	}
	return len;
}

/// メモリの内容を反転する
/// @param [in,out] buf バッファ
/// @param [in]     len バッファサイズ
void mem_invert(void *buf, size_t len)
{
	char *p = (char *)buf;
	while(len > 0) {
		*p = ~(*p);
		len--;
		p++;
	}
}

/// 改行を終端文字にする
/// @param [in,out] buf 文字列
/// @param [in]     len 文字列長さ
/// @return 文字列長さ
size_t str_shrink(void *buf, size_t len)
{
	char *p = (char *)buf;
	size_t l = 0;
	while(l < len) {
		if (*p == 0 || *p == 0x0a || *p == 0x0d) {
			*p = 0;
			break;
		}
		l++;
		p++;
	}
	return l;
}

/// 文字列をバッファにコピー 余りはfillで埋める
/// @param [in]   src     元バッファ
/// @param [in]   slen    元バッファの長さ
/// @param [in]   fill    余り部分を埋める文字
/// @param [out]  dst     出力先バッファ
/// @param [in]   dlen    出力先バッファの長さ
void mem_copy(const void *src, size_t slen, char fill, void *dst, size_t dlen)
{
	size_t l = slen;
	if (l > dlen) l = dlen;
	memset(dst, fill, dlen);
	memcpy(dst, src, l);
}

/// バッファ末尾から一致する文字をさがす
int mem_rchr(const void *buf, size_t len, int ch)
{
	const char *p = (const char *)buf;
	int match = -1;
	for(int i=(int)len-1; i>=0; i--) {
		if (p[i] == (char)ch) {
			match = i;
			break;
		}
	}
	return match;
}

/// バッファ末尾から指定文字で埋める
size_t padding(void *buf, size_t len, char fill)
{
	char *p = (char *)buf;
	p = &p[len - 1];
	while(len > 0) {
		if (*p != 0) break;
		*p = fill;
		len--;
		p--;
	}
	return len;
}

/// バッファのアルファベットASCII小文字を大文字にする
void to_upper(void *src, size_t len)
{
	unsigned char *p = (unsigned char *)src;
	for(size_t i=0; i<len; i++) {
		if (p[i] >= 0x61 && p[i] <= 0x7a) {
			p[i] -= 0x20;
		}
	}
}

//
//
//

TM::TM()
{
	Clear();
}
TM::TM(const struct tm *src)
{
	tm = *src;
}
TM::~TM()
{
}
/// 時間構造体を初期化
void TM::Clear()
{
	ClearDate();
	ClearTime();
	tm.tm_isdst = 0;
	tm.tm_wday = 0;
	tm.tm_yday = 0;
}
/// 日付を初期化
void TM::ClearDate()
{
	tm.tm_year = -1;
	tm.tm_mon = -2;
	tm.tm_mday = -1;
}
/// 時間を初期化
void TM::ClearTime()
{
	tm.tm_hour = -1;
	tm.tm_min = -1;
	tm.tm_sec = -1;
}
/// 日時をゼロにする
void TM::AllZero()
{
	tm.tm_year = 0;
	tm.tm_mon = 0;
	tm.tm_mday = 0;
	tm.tm_hour = 0;
	tm.tm_min = 0;
	tm.tm_sec = 0;
}
/// 時間構造体を代入
TM &TM::operator=(const TM &src)
{
	tm = src.tm;
	return *this;
}
/// 時間構造体を得る
void TM::Get(wxDateTime &dst) const
{
	dst.Set(tm);
}
/// 時間構造体を設定
void TM::Set(const wxDateTime &src)
{
	wxDateTime::Tm wtm = src.GetTm();
	tm.tm_year = wtm.year - 1900;
	tm.tm_mon = wtm.mon;
	tm.tm_mday = wtm.mday;
	tm.tm_hour = wtm.hour;
	tm.tm_min = wtm.min;
	tm.tm_sec = wtm.sec;
}
/// 現在日時を得る
TM &TM::Now()
{
	wxDateTime::GetTmNow(&tm);
	return *this;
}
/// 現在日時を得る
TM TM::GetNow()
{
	struct tm tm;
	wxDateTime::GetTmNow(&tm);
	return TM(&tm);
}
/// 日付が有効な値か
bool TM::IsValidDate() const
{
	return (tm.tm_mon >= -1 && tm.tm_mday >= 0);
}
/// 時間が有効な値か
bool TM::IsValidTime() const
{
	return (tm.tm_hour >= 0 && tm.tm_min >= 0);
}
/// 日時設定を無視する値か
bool TM::Ignorable() const
{
	return (tm.tm_mon == -1 &&
	((tm.tm_mday == 0 && tm.tm_hour == 0 && tm.tm_min == 0)
		|| tm.tm_mday == 0 || tm.tm_mday > 31 || tm.tm_hour > 24 || tm.tm_min > 61
	));
}
