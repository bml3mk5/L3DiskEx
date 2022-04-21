/// @file charcodes.cpp
///
/// @brief キャラクタコード
///
#include "charcodes.h"
#include <wx/xml/xml.h>

CharCodes gCharCodes;

//
//
//
CharCode::CharCode()
{
	code = 0;
}
CharCode::CharCode(const wxString &newstr, const wxString &newcode)
{
	long lcode;

	str = newstr;
	newcode.ToLong(&lcode, 16);
	code = (lcode & 0xff);
}

//
//
//
CharCodeMap::CharCodeMap()
{
}
CharCodeMap::CharCodeMap(const wxString &newtype)
{
	type = newtype;
}
CharCodeMap::~CharCodeMap()
{
	for(size_t i=0; i<list.Count(); i++) {
		CharCode *item = list[i];
		delete item;
	}
	list.Empty();
}

//
//
//
CharCodes::CharCodes()
{
}
CharCodes::~CharCodes()
{
	for(size_t i=0; i<maps.Count(); i++) {
		CharCodeMap *item = maps[i];
		delete item;
	}
	maps.Empty();
}

bool CharCodes::Load(const wxString &data_path)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("char_codes.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "CharCodeMap") return false;

	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item) {
		if (item->GetName() == "Map") {
			wxString type = item->GetAttribute("type");
			CharCodeMap *map = new CharCodeMap(type);

			wxXmlNode *mitem = item->GetChildren();
			while(mitem) {
				if (mitem->GetName() == "Char") {
					wxString codestr = mitem->GetAttribute("code");
					wxString str = mitem->GetNodeContent();
					CharCode *p = new CharCode(str, codestr);
					map->list.Add(p);
				}
				mitem = mitem->GetNext();
			}
			maps.Add(map);
		}
		item = item->GetNext();
	}
	list_cache = &maps[0]->list;
	return true;
}

void CharCodes::ConvToString(const wxUint8 *src, size_t len, wxString &dst)
{
	list_cache = &maps[0]->list;
	for(size_t i=0; i<len; i++) {
		FindString(src[i], dst);
	}
}

bool CharCodes::ConvToChars(const wxString &src, wxUint8 *dst, size_t len)
{
	bool rc = true;
	size_t pos = 0;
	list_cache = &maps[0]->list;
	for(size_t i=0; i<src.Length() && pos < (len - 1) && rc; i++) {
		rc = FindCode(src.Mid(i,1), dst, &pos);
	}
	if (dst) dst[pos]='\0';
	return rc;
}

bool CharCodes::FindString(wxUint8 src, wxString &dst, const wxString &unknownchar)
{
	bool match = false;
	for(size_t i=0; i<list_cache->Count(); i++) {
		CharCode *itm = list_cache->Item(i);
		if (itm->code == src) {
			dst += itm->str;
			match = true;
			break;
		}
	}
	if (!match) {
		if (0x20 <= src && src <= 0x7e) {
			dst += wxString((char)src, 1);
			match = true;
		} else {
			dst += unknownchar;
		}
	}
	return match;
}

bool CharCodes::FindCode(const wxString &src, wxUint8 *dst, size_t *pos)
{
	bool match = false;
	for(size_t i=0; i<list_cache->Count(); i++) {
		CharCode *itm = list_cache->Item(i);
		if (itm->str == src) {
			if (dst) {
				dst[*pos] = itm->code;
				(*pos)++;
			}
			match = true;
			break;
		}
	}
	if (!match) {
		wxScopedCharBuffer p = src.To8BitData();
		wxUint8 c = *p;
		if (c < 0x80) {
			if (dst) {
				dst[*pos] = c;
				(*pos)++;
			}
			match = true;
		}
	}
	return match;
}

void CharCodes::SetMap(const wxString &type)
{
	bool match = false;
	for(size_t i=0; i<maps.Count(); i++) {
		if (type == maps[i]->type) {
			list_cache = &maps[i]->list;
			match = true;
			break;
		}
	}
	if (!match) {
		list_cache = &maps[0]->list;
	}
}
