/// @file utils.h
///
/// @brief いろいろ
///
#ifndef _L3DISKUTILS_H_
#define _L3DISKUTILS_H_

#include "common.h"
#include <wx/string.h>
#include "charcodes.h"

/// 各種ユーティリティー
namespace L3DiskUtils
{

/// ダンプ用補助クラス
class Dump
{
private:
	::CharCodes codes;

public:
	Dump() {}
	~Dump() {}

	int DumpBinary(const wxUint8 *buffer, size_t bufsize, wxString &str, bool invert);
	wxString DumpAscii(const wxUint8 *buffer, size_t bufsize, int char_code, bool invert);
};

void ConvTmToDateTime(const struct tm *tm, wxUint8 *date, wxUint8 *time);
void ConvDateTimeToTm(const wxUint8 *date, const wxUint8 *time, struct tm *tm);
void ConvDateStrToTm(const wxString &date, struct tm *tm);
void ConvTimeStrToTm(const wxString &time, struct tm *tm);
wxString FormatYMDStr(const struct tm *tm);
wxString FormatHMSStr(const struct tm *tm);
wxString FormatHMStr(const struct tm *tm);

int ToInt(const wxString &val);

wxString Escape(const wxString &src);

}; /* namespace L3DiskUtils */

#endif /* _L3DISKUTILS_H_ */

