/// @file version.h
///
///
#ifndef _VERSION_H_
#define _VERSION_H_

#define APPLICATION_VERSION	"0.2.3"
#define APP_VER_MAJOR	0
#define APP_VER_MINOR	2
#define APP_VER_REV	3
#define APP_VER_BUILD	0
#define APP_COPYRIGHT	"Copyright (C) 2015-2017 Sasaji"

#if defined(__MINGW32__)
#ifdef x86_64
#define PLATFORM "Windows(MinGW) 64bit"
#elif i386
#define PLATFORM "Windows(MinGW) 32bit"
#else
#define PLATFORM "Windows(MinGW)"
#endif
#elif defined(_WIN32)
#if defined(_WIN64) || defined(_M_X64)
#define PLATFORM "Windows 64bit"
#else
#define PLATFORM "Windows 32bit"
#endif
#elif defined(linux)
#ifdef __x86_64
#define PLATFORM "Linux 64bit"
#elif __i386
#define PLATFORM "Linux 32bit"
#else
#define PLATFORM "Linux"
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#ifdef __x86_64
#define PLATFORM "MacOSX 64bit"
#elif __i386
#define PLATFORM "MacOSX 32bit"
#else
#define PLATFORM "MacOSX"
#endif
#elif defined(__FreeBSD__)
#define PLATFORM "FreeBSD"
#else
#define PLATFORM "Unknown"
#endif

#endif
