/// @file utils.cpp
///
/// @brief いろいろ
///

#include "utils.h"
#include "charcodes.h"

namespace L3DiskUtils
{

int DumpBinary(const wxUint8 *buffer, size_t bufsize, wxString &str)
{
	int rows = 0;
	str += wxT("    :");
	for(size_t col = 0; col < 16; col++) {
		str += wxString::Format(wxT(" +%x"), (int)col);
	}
	str += wxT("\n");
	str += wxT("-----");
	for(size_t col = 0; col < 16; col++) {
		str += wxT("---");
	}
	str += wxT("\n");
	for(size_t pos = 0, col = 0; pos < bufsize; pos++) {
		if (col == 0) {
			str += wxString::Format(wxT("+%02x0:"), rows);
		}
		str += wxString::Format(wxT(" %02x"), buffer[pos]);
		if (col >= 15) {
			str += wxT("\n");
			col = 0;
			rows++;
		} else {
			col++;
		}
	}
	rows += 3;
	return rows;
}

wxString DumpAscii(const wxUint8 *buffer, size_t bufsize)
{
	wxString str;
	gCharCodes.SetMap(wxT("hankaku"));
	for(size_t col = 0; col < 16; col++) {
		str += wxString::Format(wxT("%x"), (int)col);
	}
	str += wxT("\n");
	for(size_t col = 0; col < 16; col++) {
		str += wxT("-");
	}
	str += wxT("\n");
	for(size_t pos = 0, col = 0; pos < bufsize; pos++, col++) {
		if (col >= 16) {
			str += wxT("\n");
			col = 0;
		}
		wxUint8 c = buffer[pos];
		wxString cstr;
		gCharCodes.FindString(c, cstr, wxT("."));
		str += cstr;
	}
	str += wxT("\n");
	return str;
}

};
