﻿/// @file utils.h
///
/// @brief いろいろ
///
#ifndef _L3DISKUTILS_H_
#define _L3DISKUTILS_H_

#include "common.h"
#include <wx/wx.h>

namespace L3DiskUtils
{
	int DumpBinary(const wxUint8 *buffer, size_t bufsize, wxString &str);
	wxString DumpAscii(const wxUint8 *buffer, size_t bufsize);
};

#endif /* _L3DISKUTILS_H_ */

