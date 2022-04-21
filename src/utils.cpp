/// @file utils.cpp
///
/// @brief いろいろ
///

#include "utils.h"

namespace L3DiskUtils
{

wxString DumpBinary(const wxUint8 *buffer, size_t bufsize)
{
	wxString str;
	for(size_t pos = 0, col = 0; pos < bufsize; pos++, col++) {
		if (col >= 16) {
			str += wxT("\n");
			col = 0;
		}
		str += wxString::Format(wxT(" %02x"), buffer[pos]);
	}
	return str;
}

};
