/// @file utils.cpp
///
/// @brief いろいろ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "utils.h"
#include <wx/translation.h>
#include <wx/string.h>
#include <wx/regex.h>


namespace Utils
{

//
//
//
TempData::TempData()
{
	alloc_size = TEMP_DATA_SIZE;
	data = new wxUint8[alloc_size];
	memset(data, 0, alloc_size);
	size = 0;
}

TempData::~TempData()
{
	delete data;
}

/// バッファを(再)確保
/// @param[in] val バッファサイズ
void TempData::SetSize(size_t val)
{
	if (val > alloc_size) {
		delete data;
		alloc_size = val;
		data = new wxUint8[alloc_size];
		memset(data, 0, alloc_size);
	}
	size = val;
}

/// バッファにデータをセット
/// @param[in] data データ
/// @param[in] len  データサイズ
/// @param[in] invert データを反転するか
void TempData::SetData(const wxUint8 *data, size_t len, bool invert)
{
	SetSize(len);
	memcpy(this->data, data, this->size);
	if (invert) {
		mem_invert(this->data, this->size);
	}
}

/// バッファにデータをセット
/// @param[in] pos  データ位置
/// @param[in] val  データ
void TempData::Set(size_t pos, wxUint8 val)
{
	if (pos < size) {
		data[pos] = val;
	}
}

/// 一致するバイトデータを置換
/// @param[in] src  置換対象データ
/// @param[in] dst  置換するデータ
void TempData::Replace(wxUint8 src, wxUint8 dst)
{
	for(size_t pos=0; pos<size; pos++) {
		if (data[pos] == src) data[pos] = dst;
	}
}

/// データを反転
/// @param[in] invert 反転するか
void TempData::InvertData(bool invert)
{
	if (invert) {
		mem_invert(data, size);
	}
}

//
//
//

/// バイナリダンプ
/// @param[in]  buffer  元データ
/// @param[in]  bufsize 元データの長さ
/// @param[out] str     ダンプした文字列
/// @param[in]  invert  データを反転するか
/// @return ダンプ行数
int Dump::Binary(const wxUint8 *buffer, size_t bufsize, wxString &str, bool invert)
{
	int rows = 0;
	wxUint8 inv = invert ? 0xff : 0;
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
		str += wxString::Format(wxT(" %02x"), (buffer[pos] ^ inv));
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

/// アスキーダンプ
/// @param[in]  buffer  元データ
/// @param[in]  bufsize 元データの長さ
/// @param[in]  char_code キャラクターコード マップID
/// @param[out] str     ダンプした文字列
/// @param[in]  invert  データを反転するか
/// @return ダンプ行数
int Dump::Ascii(const wxUint8 *buffer, size_t bufsize, const wxString &char_code, wxString &str, bool invert)
{
	wxUint16 inv = invert ? 0xff : 0;

	codes.SetMap(char_code);

	for(size_t col = 0; col < 16; col++) {
		str += wxString::Format(wxT("%x"), (int)col);
	}
	str += wxT("\n");
	for(size_t col = 0; col < 16; col++) {
		str += wxT("-");
	}
	str += wxT("\n");

	for(size_t pos = 0, col = 0; pos < bufsize; ) {
		if (col >= 16) {
			str += wxT("\n");
			col -= 16;
			if (col > 0) {
				str += wxString(" ", (size_t)col);
			}
		}

		wxString cstr;
		wxUint8 c[4];
		c[0] = buffer[pos] ^ inv;
		c[1] = pos + 1 == bufsize ? 0 : buffer[pos + 1] ^ inv;
		c[2] = 0;

		size_t len = codes.FindString(c, 2, cstr, '.');
		str += cstr;
		pos += len;
		col += len;
	}
	str += wxT("\n");
	return 0;
}

/// テキストダンプ
/// @param[in]  buffer  元データ
/// @param[in]  bufsize 元データの長さ
/// @param[in]  char_code キャラクターコード マップID
/// @param[out] str     ダンプした文字列
/// @param[in]  invert  データを反転するか
/// @return ダンプ行数
int Dump::Text(const wxUint8 *buffer, size_t bufsize, const wxString &char_code, wxString &str, bool invert)
{
	wxUint16 inv = invert ? 0xff : 0;

	codes.SetMap(char_code);

	int col = 0;
	int row = 1;
	for(size_t pos = 0; pos < bufsize; ) {
		wxString cstr;
		wxUint8 c[4];
		c[0] = buffer[pos] ^ inv;
		c[1] = pos + 1 == bufsize ? 0 : buffer[pos + 1] ^ inv;
		c[2] = 0;

		if (col >= 80) {
			row++;
			col=0;
		}

		if (c[0] == '\r' && c[1] == '\n') {
			// 改行
			str += wxT("\n");
			row++;
			col=0;
			pos+=2;
			continue;
		} else if (c[0] == '\r' || c[0] == '\n') {
			// 改行
			str += wxT("\n");
			row++;
			col=0;
			pos++;
			continue;
		} else if (c[0] == '\t') {
			// タブ
			if (c[1] >= 1 && c[1] < 0x20) {
				// for FLEX
				for(int i=0; i<c[1]; i++) {
					str += wxT(" ");
				}
				col+=(int)c[1];
				pos+=2;
			} else {
				str += wxT("\t");
				col+=8;
				pos++;
			}
			continue;
		} else if (c[0] < 0x20) {
			// コントロールコードは"."に変換
			str += wxT(".");
			col++;
			pos++;
			continue;
		}

		size_t len = codes.FindString(c, 2, cstr, '.');
		str += cstr;
		col += (int)len;
		pos += len;
	}
	return row;
}

/// 時間構造体を日時データに変換(MS-DOS)
/// @param[in]  tm   時間構造体
/// @param[out] date 日付データ
/// @param[out] time 時間データ
void ConvTmToDateTime(const struct tm *tm, wxUint8 *date, wxUint8 *time)
{
	date[0] = (tm->tm_year & 0xff);
	date[1] = ((tm->tm_mon & 0x0f) << 4) | ((tm->tm_year & 0xf00) >> 8);
	date[2] = (tm->tm_mday & 0xff);

	time[0] = (tm->tm_hour & 0xff);
	time[1] = (tm->tm_min & 0xff);
	time[2] = (tm->tm_sec & 0xff);
}
/// 日時データを構造体に変換(MS-DOS)
/// @param[in]  date 日付データ
/// @param[in]  time 時間データ
/// @param[out] tm   時間構造体
void ConvDateTimeToTm(const wxUint8 *date, const wxUint8 *time, struct tm *tm)
{
	tm->tm_year = (int)date[0] | ((int)date[1] & 0xf) << 8;
	tm->tm_mon  = (date[1] & 0xf0) >> 4;
	if (tm->tm_mon == 0xf) tm->tm_mon = -1;
	tm->tm_mday = date[2];

	tm->tm_hour = time[0];
	tm->tm_min = time[1];
	tm->tm_sec = time[2];
}
/// 日付文字列を構造体に変換
/// @param[in]  date 日付文字列
/// @param[out] tm   時間構造体
/// @return 変換できた:true
bool ConvDateStrToTm(const wxString &date, struct tm *tm)
{
	wxRegEx re("^([0-9]+)[/:.-]([0-9]+)[/:.-]([0-9]+)$");
	wxString sval;
	long lval;
	bool valid = true;
	if (re.Matches(date)) {
		// year
		sval = re.GetMatch(date, 1);
		sval.ToLong(&lval);
		if (lval >= 1900) lval -= 1900;
		tm->tm_year = (int)lval;

		// month
		sval = re.GetMatch(date, 2);
		sval.ToLong(&lval);
		tm->tm_mon = (int)lval - 1;

		// day
		sval = re.GetMatch(date, 3);
		sval.ToLong(&lval);
		tm->tm_mday = (int)lval;
	} else {
		// invalid
		tm->tm_year = -1;
		tm->tm_mon = -2;
		tm->tm_mday = -1;

		valid = false;
	}
	return valid;
}
/// 時間文字列を構造体に変換
/// @param[in]  time 時間文字列
/// @param[out] tm   時間構造体
/// @return 変換できた:true
bool ConvTimeStrToTm(const wxString &time, struct tm *tm)
{
	wxRegEx re1("^([0-9]+)[/:.-]([0-9]+)[/:.-]([0-9]+)$");
	wxRegEx re2("^([0-9]+)[/:.-]([0-9]+)$");
	wxString sval;
	long lval;
	bool valid = true;
	if (re1.Matches(time)) {
		// hour
		sval = re1.GetMatch(time, 1);
		sval.ToLong(&lval);
		tm->tm_hour = (int)lval;

		// minute
		sval = re1.GetMatch(time, 2);
		sval.ToLong(&lval);
		tm->tm_min = (int)lval;

		// day
		sval = re1.GetMatch(time, 3);
		sval.ToLong(&lval);
		tm->tm_sec = (int)lval;

	} else if (re2.Matches(time)) {
		// hour
		sval = re2.GetMatch(time, 1);
		sval.ToLong(&lval);
		tm->tm_hour = (int)lval;

		// minute
		sval = re2.GetMatch(time, 2);
		sval.ToLong(&lval);
		tm->tm_min = (int)lval;
	} else {
		// invalid
		tm->tm_hour = -1;
		tm->tm_min = -1;
		tm->tm_sec = -1;

		valid = false;
	}
	return valid;
}
/// BCD形式の日付を変換
/// @param[in]  yy 年
/// @param[in]  mm 月
/// @param[in]  dd 日
/// @param[out] tm 時間構造体
void ConvYYMMDDToTm(wxUint8 yy, wxUint8 mm, wxUint8 dd, struct tm *tm)
{
	tm->tm_year = (yy >> 4) * 10 + (yy & 0xf);
	if (0 <= tm->tm_year && tm->tm_year < 80) tm->tm_year += 100;
	tm->tm_mon = (mm >> 4) * 10 + (mm & 0xf);
	tm->tm_mon--;
	tm->tm_mday = (dd >> 4) * 10 + (dd & 0xf);
}
/// BCD形式の日付に変換
/// @param[in]  tm 時間構造体
/// @param[out] yy 年
/// @param[out] mm 月
/// @param[out] dd 日
void ConvTmToYYMMDD(const struct tm *tm, wxUint8 &yy, wxUint8 &mm, wxUint8 &dd)
{
	yy = (wxUint8)(((tm->tm_year / 10) % 10) << 4) + (tm->tm_year % 10);
	mm = (wxUint8)(((tm->tm_mon + 1) / 10) << 4) + ((tm->tm_mon + 1) % 10);
	dd = (wxUint8)((tm->tm_mday / 10) << 4) + (tm->tm_mday % 10);
}
/// 日付を文字列で返す
/// @param[in] tm 時間構造体
/// @return "YYYY/MM/DD" or "----/--/--"
wxString FormatYMDStr(const struct tm *tm)
{
	wxString str;
	str = (tm->tm_year >= 0 ? wxString::Format(wxT("%04d"), tm->tm_year + 1900) : wxT("----"));
	str += wxT("/");
	str += (tm->tm_mon >= -1 ? wxString::Format(wxT("%02d"), tm->tm_mon + 1) : wxT("--"));
	str += wxT("/");
	str += (tm->tm_mday >= 0 ? wxString::Format(wxT("%02d"), tm->tm_mday) : wxT("--"));
	return str;
}
/// 時分秒を文字列で返す
/// @param[in] tm 時間構造体
/// @return "HH:MI:SS" or "--:--:--"
wxString FormatHMSStr(const struct tm *tm)
{
	wxString str;
	str = (tm->tm_hour >= 0 ? wxString::Format(wxT("%02d"), tm->tm_hour) : wxT("--"));
	str += wxT(":");
	str += (tm->tm_min >= 0 ? wxString::Format(wxT("%02d"), tm->tm_min) : wxT("--"));
	str += wxT(":");
	str += (tm->tm_sec >= 0 ? wxString::Format(wxT("%02d"), tm->tm_sec) : wxT("--"));
	return str;
}
/// 時分を文字列で返す
/// @param[in] tm 時間構造体
/// @return "HH:MI" or "--:--"
wxString FormatHMStr(const struct tm *tm)
{
	wxString str;
	str = (tm->tm_hour >= 0 ? wxString::Format(wxT("%02d"), tm->tm_hour) : wxT("--"));
	str += wxT(":");
	str += (tm->tm_min >= 0 ? wxString::Format(wxT("%02d"), tm->tm_min) : wxT("--"));
	return str;
}

/// 文字列をint値に変換
/// @param[in] val 文字列
int ToInt(const wxString &val)
{
	long lval = 0;
	wxString h = val.Left(2).Lower();
	if (h == wxT("0x")) {
		val.Mid(2).ToLong(&lval, 16);
	} else if (h == wxT("0b")) {
		val.Mid(2).ToLong(&lval, 2);
	} else {
		val.ToLong(&lval);
	}
	return (int)lval;
}

/// 文字列をbool値に変換
/// "1","TRUE","true" => true
/// @param[in] val 文字列
bool ToBool(const wxString &val)
{
	bool bval = false;
	if (val == wxT("1") || val.Upper() == wxT("TRUE")) {
		bval = true;
	}
	return bval;
}

/// エスケープ文字を展開
/// @note \\\\                \\ そのもの
/// @note \\x[0-9a-f][0-9a-f] 16進数で指定
/// @param[in]  src 文字列
/// @param[out] dst 展開後文字列
void DecodeEscape(const wxString &src, wxString &dst)
{
	wxString str = src;

	while (str.Length() > 0) {
		if (str.Left(1) == wxT("\\")) {
			str = str.Mid(1);
			if (str.Left(1) == wxT("x")) {
				long v;
				str.Mid(1,2).ToLong(&v, 16);
				dst += wxString(wxUniChar(v), 1);
				str = str.Mid(3);
			} else {
				dst += str.Left(1);
				str = str.Mid(1);
			}
		} else {
			dst += str.Left(1);
			str = str.Mid(1);
		}
	}
}

/// エスケープ文字を展開
/// @note \\\\                \\ そのもの
/// @note \\x[0-9a-f][0-9a-f] 16進数で指定
/// @param[in]  src 文字列
/// @param[out] dst 展開後文字列
/// @param[in]  len dstのバッファ長さ
void DecodeEscape(const wxString &src, wxUint8 *dst, size_t len)
{
	wxString str = src;

	size_t pos = 0;
	while (str.Length() > 0 && len > pos) {
		if (str.Left(1) == wxT("\\")) {
			str = str.Mid(1);
			if (str.Left(1) == wxT("x")) {
				long v;
				str.Mid(1,2).ToLong(&v, 16);
				dst[pos] = (v & 0xff);
				str = str.Mid(3);
				pos++;
			} else {
				dst[pos] = str.at(0);
				str = str.Mid(1);
				pos++;
			}
		} else {
			dst[pos] = str.at(0);
			str = str.Mid(1);
			pos++;
		}
	}
}

/// 文字をエスケープ
/// 英数字以外を"\x00"のように変換
/// @param[in] src 文字列
/// @param[in] len 文字列長さ
wxString EncodeEscape(const wxUint8 *src, size_t len)
{
	wxString rstr;

	for(size_t i=0; i<len; i++) {
		if (src[i] == 0) {
			break;
		} else if ((src[i] >= 0x00 && src[i] <= 0x1f)
		 || (src[i] == 0x60)
		 || (src[i] >= 0x7f && src[i] <= 0xff)) {
			rstr += wxString::Format(wxT("\\x%02x"), (int)src[i]);
		} else {
			rstr += wxString(1, (char)src[i]);
		}
	}
	return rstr;
}

const wxString gSystemInvalidChars = wxT("%\\/:*?\"<>|");

/// ファイル名を展開
/// "%xx"を実際の文字に変換
/// @param[in] src ファイル名
wxString DecodeFileName(const wxString &src)
{
	wxString dst;
	size_t pos = 0;

	while (pos < src.Length()) {
		bool dec = false;
		if (src.Mid(pos, 1) == wxT("%")) {
			wxString sval = src.Mid(pos + 1, 2);
			long lval = 0;
			if (sval.ToLong(&lval, 16)) {
				dst += wxUniChar(lval);
				dec = true;
				pos += 3;
			}
		}
		if (!dec) {
			dst += src.Mid(pos, 1);
			pos++;
		}
	}
	return dst;
}

/// ファイル名をエスケープ
/// %\/:*?"<>| を "%%xx"に変換する
/// @param[in] src ファイル名
wxString EncodeFileName(const wxString &src)
{
	wxString str;
	for(size_t i=0; i<src.Length(); i++) {
		wxUniChar c = src.at(i);
		if (gSystemInvalidChars.Find(c) >= 0) {
			str += wxString::Format(wxT("%%%02x"), (int)c);
		} else {
			str += c;
		}
	}
	return str;
}

/// サイド番号を文字列にする
/// @param[in] side_number サイド番号(0,1)
/// @param[in] each_sides  数字で返す場合true
/// @return "0","1" or "A","B"
wxString GetSideNumStr(int side_number, bool each_sides)
{
	wxString str;
	if (side_number >= 0) {
		if (each_sides) {
			str = wxString::Format(wxT("%d"), side_number);
		} else {
			str = wxString::Format(wxT("%c"), side_number + 0x41);
		}
	}
	return str;
}

/// サイド番号を文字列にする
/// @param[in] side_number サイド番号(0,1)
/// @param[in] each_sides  数字で返す場合true
/// @return "side 0" or "side A"
wxString GetSideStr(int side_number, bool each_sides)
{
	wxString str;
	if (side_number >= 0) {
		if (each_sides) {
			str = wxString::Format(_("side %d"), side_number);
		} else {
			str = wxString::Format(_("side %c"), side_number + 0x41);
		}
	}
	return str;
}

/// リストの中に一致する文字列があるか
/// @param[in] list リスト(NULL終り)
/// @param[in] substr 文字列
/// @return 一致する位置 or -1
int IndexOf(const char *list[], const wxString &substr)
{
	int match = -1;
	for(int i=0; list[i] != NULL; i++) {
		if (substr == list[i]) {
			match = i;
			break;
		}
	}
	return match;
}

/// 大文字の方が多いか
/// @param[in] str 文字列
bool IsUpperString(const wxString &str)
{
	size_t len = str.Length();
	int u = 0;
	int l = 0;
	for(size_t i=0; i<len; i++) {
		wxUniChar c = str[i];
		if (c >= 0x41 && c < 0x5b) {
			u++;
		} else if (c >= 0x51 && c < 0x7b) {
			l++;
		}
	}
	return (u > l); 
}

/// 2^nかどうか
/// @param[in] val 値
/// @param[in] digit チェックする桁数
bool IsPowerOfTwo(wxUint32 val, int digit)
{
	bool valid = true;

	while(digit > 0) {
		if (val & 1) {
			val >>= 1;
			valid = (val == 0);
			break;
		}
		val >>= 1;
		digit--;
	}
	return valid;
}

}; /* namespace L3DiskUtils */

