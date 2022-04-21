/// @file utils.h
///
/// @brief いろいろ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _L3DISKUTILS_H_
#define _L3DISKUTILS_H_

#include "common.h"
#include <wx/string.h>
#include "charcodes.h"


/// 各種ユーティリティー
namespace Utils
{
#define TEMP_DATA_SIZE 2048

/// データの書き出しや読み込みで使用するテンポラリバッファ
class TempData {
private:
	wxUint8 *data;
	size_t  alloc_size;
	size_t  size;
public:
	TempData();
	~TempData();
	void SetSize(size_t val);
	void SetData(const wxUint8 *data, size_t len, bool invert = false);
	void Set(size_t pos, wxUint8 val);
	void Replace(wxUint8 src, wxUint8 dst);
	wxUint8 *GetData() { return data; }
	size_t GetSize() const { return size; }
	void InvertData(bool invert);
};

/// ダンプ用補助クラス
class Dump
{
private:
	::CharCodes codes;

public:
	Dump() {}
	~Dump() {}

	int Binary(const wxUint8 *buffer, size_t bufsize, wxString &str, bool invert);
	int Ascii(const wxUint8 *buffer, size_t bufsize, int char_code, wxString &str, bool invert);
	int Text(const wxUint8 *buffer, size_t bufsize, int char_code, wxString &str, bool invert);
};

void	ConvTmToDateTime(const struct tm *tm, wxUint8 *date, wxUint8 *time);
void	ConvDateTimeToTm(const wxUint8 *date, const wxUint8 *time, struct tm *tm);
bool	ConvDateStrToTm(const wxString &date, struct tm *tm);
bool	ConvTimeStrToTm(const wxString &time, struct tm *tm);
void	ConvYYMMDDToTm(wxUint8 yy, wxUint8 mm, wxUint8 dd, struct tm *tm);
void	ConvTmToYYMMDD(const struct tm *tm, wxUint8 &yy, wxUint8 &mm, wxUint8 &dd);
wxString FormatYMDStr(const struct tm *tm);
wxString FormatHMSStr(const struct tm *tm);
wxString FormatHMStr(const struct tm *tm);


int		ToInt(const wxString &val);
bool	ToBool(const wxString &val);

wxString Escape(const wxString &src);

wxString GetSideNumStr(int side_number, bool each_sides);
wxString GetSideStr(int side_number, bool each_sides);

}; /* namespace L3DiskUtils */

#endif /* _L3DISKUTILS_H_ */

