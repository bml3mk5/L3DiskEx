/// @file common.cpp
///
///

#include <stdio.h>

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
