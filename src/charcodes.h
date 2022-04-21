/// @file charcodes.h
///
/// @brief キャラクタコード
///
#ifndef _CHARCODES_H_
#define _CHARCODES_H_

#include "common.h"
#include <wx/wx.h>

///
class CharCode
{
public:
	wxString str;
	wxUint8  code;

public:
	CharCode();
	CharCode(const wxString &newstr, const wxString &newcode);
	~CharCode() {}
};

WX_DEFINE_ARRAY(CharCode *, CharCodeList);

///
class CharCodeMap
{
public:
	wxString     type;
	CharCodeList list;

public:
	CharCodeMap();
	CharCodeMap(const wxString &newtype);
	~CharCodeMap();
};

WX_DEFINE_ARRAY(CharCodeMap *, CharCodeMaps);

///
class CharCodes
{
private:
	CharCodeMaps maps;
	CharCodeList *list_cache;

public:
	CharCodes();
	~CharCodes();

	bool Load(const wxString &data_path);

	void ConvToString(const wxUint8 *src, size_t len, wxString &dst);
	bool ConvToChars(const wxString &src, wxUint8 *dst, size_t len);

	bool FindString(wxUint8 src, wxString &dst, const wxString &unknownchar = wxT("?"));
	bool FindCode(const wxString &src, wxUint8 *dst, size_t *pos);

	void SetMap(const wxString &type);
};

extern CharCodes gCharCodes;

#endif /* _CHARCODES_H_ */
