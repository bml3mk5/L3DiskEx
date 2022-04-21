/// @file charcodes.cpp
///
/// @brief キャラクタコード
///
#include "charcodes.h"
#include <wx/xml/xml.h>


CharCodeMaps gCharCodeMaps;

//
//
//
CharCode::CharCode()
{
	memset(code, 0, sizeof(code));
	code_len = 0;
}
CharCode::CharCode(const wxString &newstr, const wxString &newcode)
{
	long lcode;

	memset(code, 0, sizeof(code));

	str = newstr;
	code_len = (newcode.Length() >> 1);
	if (code_len >= sizeof(code)) code_len = sizeof(code) - 1;
	for(size_t i=0; i<code_len; i++) {
		newcode.Mid(i*2, 2).ToLong(&lcode, 16);
		code[i] = (lcode & 0xff);
	}
}

//
//
//
CharCodeMap::CharCodeMap()
{
	type = 0;
	font_encoding = 0;
}
CharCodeMap::CharCodeMap(const wxString &n_name, int n_type)
{
	name = n_name;
	type = n_type;
	font_encoding = 0;
}
CharCodeMap::~CharCodeMap()
{
	for(size_t i=0; i<list.Count(); i++) {
		CharCode *item = list[i];
		delete item;
	}
	list.Empty();
}
#if 0 
bool CharCodeMap::FindString(wxUint8 src, wxString &dst, wxUint8 unknownchar)
{
	bool match = false;
	for(size_t i=0; i<list.Count(); i++) {
		CharCode *itm = list.Item(i);
		if (itm->code[0] == src) {
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
#endif
size_t CharCodeMap::FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar)
{
	bool match = false;
	size_t len = 0;

	if (remain == 0) return len;

	for(size_t i=0; i<list.Count(); i++) {
		CharCode *itm = list.Item(i);
		if (memcmp(itm->code, src, itm->code_len) == 0) {
			dst += itm->str;
			len = itm->code_len;
			match = true;
			break;
		}
	}
	if (!match) {
		if (0x20 <= src[0] && src[0] <= 0x7e) {
			dst += src[0];
			match = true;
		} else {
			dst += unknownchar;
		}
		len = 1;
	}
	return len;
}

/// 文字が文字変換テーブルにあるか
/// @param [in]     src : 文字列(１文字)
/// @param [in,out] dst : バイト列 Nullable
/// @param [out]    pos : ある場合、その位置
/// @retval true  一致した
/// @retval false 一致しなかった
bool CharCodeMap::FindCode(const wxString &src, wxUint8 *dst, size_t *pos)
{
	bool match = false;
	for(size_t i=0; i<list.Count(); i++) {
		CharCode *itm = list.Item(i);
		if (itm->str == src) {
			if (dst) {
				memcpy(&dst[*pos], itm->code, itm->code_len);
				(*pos) += itm->code_len;
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

/// wxMBConvを使って変換する Shift-JIS変換用
CharCodeMapMB::CharCodeMapMB()
	: CharCodeMap()
{
	cs = NULL;
}
CharCodeMapMB::CharCodeMapMB(const wxString &n_name, int n_type)
	: CharCodeMap(n_name, n_type)
{
	cs = NULL;
}
CharCodeMapMB::~CharCodeMapMB()
{
	delete cs;	
}
void CharCodeMapMB::Initialize()
{
	cs = new wxCSConv((wxFontEncoding)font_encoding);
}
#if 0
bool CharCodeMapMB::FindString(wxUint8 src, wxString &dst, wxUint8 unknownchar)
{
	return false;
}
#endif
size_t CharCodeMapMB::FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar)
{
	wxUint8 c[4],cc[4];
	size_t pos = 0;

	c[0] = src[0];
	c[1] = src[1];
	c[2] = 0;
	if (c[0] < 0x20) {
		// ascii control char
		c[0] = unknownchar;
		c[1] = 0;
		dst += wxString((const char *)c, *cs);
		pos++;
		return pos;
	} else if (c[0] < 0x80) {
		// ascii visible char
		c[1] = 0;
		dst += wxString((const char *)c, *cs);
		pos++;
		return pos;
	}

	if (remain <= 1) {
		c[1] = 0;
	} else {
		if (c[1] < 0x20) {
			// ascii control char
			c[1] = unknownchar;
		}
	}

#if 0
	wxString wc = wxString((const char *)c, *cs);
	int wcl = (int)wc.length();
	if (wcl == 0) {
		// 多分 変換できていない
		dst += unknownchar;
		pos++;
	} else if (wcl == 1) {
		// 変換できている
		dst += wc;
		wxCharBuffer cc = wc.mb_str(*cs);
		// 1バイト文字 or 2バイト文字 かどうかをチェック
		wxUint8 cc1 = (wxUint8)cc[(size_t)1];
		if (remain > 1 && c[1] == cc1) {
			// 2バイト文字を変換している
			pos++;
		}
		pos++;
	} else {
		// 変換できている
		// 2バイト文字
		dst += wc;
		pos += 2;
	}
#endif

	wchar_t wc[4];
	size_t len = cs->ToWChar(wc, 4, (const char *)c, 2);
	if (len == wxCONV_FAILED) {
		// 1文字で変換できるか
		len = cs->ToWChar(wc, 4, (const char *)c, 1);
		if (len == wxCONV_FAILED) {
			// 変換できない！
			dst += unknownchar;
			pos++;
		} else {
			// 変換できた
			dst += wxString(wc, len);
			pos += len;
		}
	} else if (len == 2) {
		// 変換できた
		dst += wxString(wc, len);
		pos += 2;
	} else if (len == 1) {
		// 変換できた
		dst += wxString(wc, len);
		// 1バイト文字 or 2バイト文字 かどうかをチェック
		if (remain > 1) {
			len = cs->FromWChar((char *)cc, 4, wc, len);
			if (c[1] == cc[1]) {
				// 2バイト文字を変換している
				pos++;
			}
		}
		pos++;
	}

	return pos;
}

/// 文字が文字変換テーブルにあるか
/// @param [in]     src : 文字列(１文字)
/// @param [in,out] dst : バイト列 Nullable
/// @param [out]    pos : ある場合、その位置
/// @retval true  一致した
/// @retval false 一致しなかった
bool CharCodeMapMB::FindCode(const wxString &src, wxUint8 *dst, size_t *pos)
{
	bool match = false;

	wxCharBuffer cb = src.mb_str(*cs);
	size_t len = cb.length();
	if (len > 0) {
		match = true;
		for(size_t i=0; i<len; i++) {
			if (dst) {
				dst[*pos] = cb[i];
				(*pos)++;
			}
		}
	}
	return match;
}

//
//
//
CharCodeMaps::CharCodeMaps()
{
}

CharCodeMaps::~CharCodeMaps()
{
	for(size_t i=0; i<maps.Count(); i++) {
		CharCodeMap *item = maps[i];
		delete item;
	}
	maps.Empty();
}

bool CharCodeMaps::Load(const wxString &data_path)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("char_codes.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "CharCodeMap") return false;

	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item) {
		if (item->GetName() == "Map") {
			wxString sname = item->GetAttribute("name");
			wxString stype = item->GetAttribute("type");
			long type = 0;
			CharCodeMap *map = NULL;
			stype.ToLong(&type);
			switch(type) {
			case 1:
				map = new CharCodeMapMB(sname, (int)type);
				break;
			default:
				map = new CharCodeMap(sname, (int)type);
				break;
			}

			wxXmlNode *mitem = item->GetChildren();
			while(mitem) {
				if (mitem->GetName() == "Char") {
					wxString codestr = mitem->GetAttribute("code");
					wxString str = mitem->GetNodeContent();
					CharCode *p = new CharCode(str, codestr);
					map->GetList().Add(p);
				}
				 else if (mitem->GetName() == "FontEncoding") {
					wxString str = mitem->GetNodeContent();
					long val = 0;
					str.ToLong(&val);
					map->SetFontEncoding((int)val);
				}
				mitem = mitem->GetNext();
			}
			map->Initialize();
			maps.Add(map);
		}
		item = item->GetNext();
	}
	return true;
}

CharCodeMap *CharCodeMaps::GetMap(size_t index)
{
	return maps[index];
}

CharCodeMap *CharCodeMaps::FindMap(const wxString &name)
{
	CharCodeMap *match = NULL;
	for(size_t i=0; i<maps.Count(); i++) {
		if (name == maps[i]->GetName()) {
			match = maps[i];
			break;
		}
	}
	return match;
}

//
//
//
CharCodes::CharCodes()
{
	cache = gCharCodeMaps.GetMap(0);
}
CharCodes::~CharCodes()
{
}

void CharCodes::ConvToString(const wxUint8 *src, size_t len, wxString &dst)
{
//	cache = maps[0];
	for(size_t i=0; i<len; ) {
		if (*src == 0) break;
		size_t l = cache->FindString(src, i + 1 == len ? 1 : 2, dst);
		i += l;
		src += l;
	}
}

bool CharCodes::ConvToChars(const wxString &src, wxUint8 *dst, size_t len)
{
	bool rc = true;
	size_t pos = 0;
//	cache = maps[0];
	for(size_t i=0; i<src.Length() && pos < (len - 1) && rc; i++) {
		rc = cache->FindCode(src.Mid(i,1), dst, &pos);
	}
	if (dst) dst[pos]='\0';
	return rc;
}

#if 0
bool CharCodes::FindString(wxUint8 src, wxString &dst, wxUint8 unknownchar)
{
	return cache->FindString(src, dst, unknownchar);
}
#endif

size_t CharCodes::FindString(const wxUint8 *src, size_t len, wxString &dst, wxUint8 unknownchar)
{
	return cache->FindString(src, len, dst, unknownchar);
}

/// 文字が文字変換テーブルにあるか
/// @param [in]     src : 文字列(１文字)
/// @param [in,out] dst : バイト列 Nullable
/// @param [out]    pos : ある場合、その位置
/// @retval true  一致した
/// @retval false 一致しなかった
bool CharCodes::FindCode(const wxString &src, wxUint8 *dst, size_t *pos)
{
	return cache->FindCode(src, dst, pos);
}

void CharCodes::SetMap(const wxString &name)
{
	cache = gCharCodeMaps.FindMap(name);
	if (!cache) {
		cache = gCharCodeMaps.GetMap(0);
	}
}
