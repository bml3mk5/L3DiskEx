/// @file common.h
///
/// @brief common definitions
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _COMMON_H_
#define _COMMON_H_

#include <wx/defs.h>
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

#endif /* _COMMON_H_ */
