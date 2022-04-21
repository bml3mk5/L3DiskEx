/// @file common.cpp
///
/// @brief common functions
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include <stdio.h>
#include <string.h>


/// 右側の指定文字をトリミング
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

/// メモリの内容を反転する
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
