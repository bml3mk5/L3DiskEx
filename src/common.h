/// @file common.h
///
///
#ifndef _COMMON_H_
#define _COMMON_H_

#include <wx/platform.h>
#include "version.h"

#ifndef _MAX_PATH
#define _MAX_PATH	260
#endif

#define DEFAULT_TEXTWIDTH 140

#define USE_DND_ON_TOP_PANEL 1

#if defined(__WXMSW__)

// ignore warning
#ifndef __GNUC__
//#pragma warning(disable:4482)
#pragma warning(disable:4996)
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
