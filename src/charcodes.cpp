/// @file charcodes.cpp
///
/// @brief キャラクタコード
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "charcodes.h"
#include <wx/xml/xml.h>
#include <wx/translation.h>


CharCodeMaps	gCharCodeMaps;
CharCodeChoices	gCharCodeChoices;

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

/// 文字コードが文字変換テーブルにあるか
/// @param [in]  src         : 文字コード(1～2バイト)
/// @param [in]  remain      : srcの残りバイト数
/// @param [out] dst         : 文字列
/// @param [in]  unknownchar : 変換できない場合に置き換える文字
/// @return 変換したバイト数
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

/// 文字コード１文字を(SJIS)を文字列に変換する
/// @param [in]  src         : 文字コード(1～2バイト)
/// @param [in]  remain      : srcの残りバイト数
/// @param [out] dst         : 文字列
/// @param [in]  unknownchar : 変換できない場合に置き換える文字
/// @return 変換したバイト数
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

	wchar_t wc[4];
	size_t len = cs->ToWChar(wc, 4, (const char *)c, remain > 1 ? 2 : remain);
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
void CharCodeMaps::Add(CharCodeMap *map)
{
	maps.Add(map);
}
CharCodeMap *CharCodeMaps::GetMap(size_t index)
{
	return (index < maps.Count() ? maps[index] : NULL);
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
CharCodeChoice::CharCodeChoice(const wxString &n_name, const wxArrayString &n_item_names)
{
	name = n_name;
	item_names = n_item_names;
}
CharCodeChoice::~CharCodeChoice()
{
}
void CharCodeChoice::AssignMaps()
{
	for(size_t i=0; i<item_names.Count(); i++) {
		CharCodeMap *map = gCharCodeMaps.FindMap(item_names[i]);
		if (map) {
			maps.Add(map);
		}
	}
}
size_t CharCodeChoice::Count() const
{
	return maps.Count();
}
CharCodeMap *CharCodeChoice::Item(size_t idx) const
{
	return maps.Item(idx);
}
const wxString &CharCodeChoice::GetItemName(size_t idx) const
{
	if (idx >= maps.Count()) {
		idx = 0;
	}
	return maps[idx]->GetName();
}
CharCodeMap *CharCodeChoice::Find(const wxString &name) const
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
int CharCodeChoice::IndexOf(const wxString &name) const
{
	int match = 0;
	for(size_t i=0; i<maps.Count(); i++) {
		if (name == maps[i]->GetName()) {
			match = (int)i;
			break;
		}
	}
	return match;
}

//
//
//
CharCodeChoices::CharCodeChoices()
{
}
CharCodeChoices::~CharCodeChoices()
{
	for(size_t i=0; i<choices.Count(); i++) {
		CharCodeChoice *item = choices[i];
		delete item;
	}
	choices.Empty();
}
void CharCodeChoices::AssignMaps()
{
	for(size_t i=0; i<choices.Count(); i++) {
		choices[i]->AssignMaps();
	}
}
void CharCodeChoices::Add(CharCodeChoice *choice)
{
	choices.Add(choice);
}
CharCodeChoice *CharCodeChoices::Find(const wxString &n_name) const
{
	CharCodeChoice *match = NULL;
	for(size_t i=0; i<choices.Count(); i++) {
		if (n_name == choices[i]->GetName()) {
			match = choices[i];
			break;
		}
	}
	return match;
}
const wxString &CharCodeChoices::GetItemName(const wxString &name, size_t item_idx) const
{
	const CharCodeChoice *choice = Find(name);
	if (choice) {
		return choice->GetItemName(item_idx);
	} else {
		return gCharCodeMaps.GetMap(0)->GetName();
	}
}
int CharCodeChoices::IndexOf(const wxString &name, const wxString &item_name) const
{
	int idx = 0;
	const CharCodeChoice *choice = Find(name);
	if (choice) {
		idx = choice->IndexOf(item_name);
	}
	return idx;
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
/// XMLからパラメータをロード
bool CharCodes::Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("char_codes.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "CharCodes") return false;

	bool sts = true;
	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item && sts) {
		if (item->GetName() == "Maps") {
			sts = LoadMaps(item, locale_name, errmsgs);
		}
		else if (item->GetName() == "Choices") {
			sts = LoadChoices(item, locale_name, errmsgs);
		}
		item = item->GetNext();
	}
	if (sts) {
		gCharCodeChoices.AssignMaps();
	}
	return sts;
}

bool CharCodes::LoadMaps(wxXmlNode *item, const wxString &locale_name, wxString &errmsgs)
{
	item = item->GetChildren();
	while(item) {
		if (item->GetName() == "Map") {
			wxString sname = item->GetAttribute("name");
			wxString stype = item->GetAttribute("type");
			long type = 0;
			CharCodeMap *map = NULL;
			wxString desc, desc_locale;
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
				else if (mitem->GetName() == "Description") {
					if (mitem->HasAttribute("lang")) {
						wxString lang = mitem->GetAttribute("lang");
						if (locale_name.Find(lang) != wxNOT_FOUND) {
							desc_locale = mitem->GetNodeContent();
						}
					} else {
						desc = mitem->GetNodeContent();
					}
				}
				mitem = mitem->GetNext();
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			map->Initialize();
			map->SetDescription(desc);
			if (gCharCodeMaps.FindMap(sname) == NULL) {
				gCharCodeMaps.Add(map);
			} else {
				errmsgs += wxT("\n");
				errmsgs += _("Duplicate name in CharCodes::Maps : ");
				errmsgs += sname;
				delete map;
				return false;
			}
		}
		item = item->GetNext();
	}
	return true;
}

bool CharCodes::LoadChoices(wxXmlNode *item, const wxString &locale_name, wxString &errmsgs)
{
	item = item->GetChildren();
	while(item) {
		if (item->GetName() == "Choice") {
			wxString sname = item->GetAttribute("name");
			wxArrayString item_names;
			wxXmlNode *mitem = item->GetChildren();
			while(mitem) {
				if (mitem->GetName() == "Item") {
					wxString str = mitem->GetNodeContent();
					item_names.Add(str);
				}
#if 0
				else if (mitem->GetName() == "Description") {
					if (mitem->HasAttribute("lang")) {
						wxString lang = mitem->GetAttribute("lang");
						if (locale_name.Find(lang) != wxNOT_FOUND) {
							desc_locale = mitem->GetNodeContent();
						}
					} else {
						desc = mitem->GetNodeContent();
					}
				}
#endif
				mitem = mitem->GetNext();
			}
#if 0
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
#endif
			if (gCharCodeChoices.Find(sname) == NULL) {
				gCharCodeChoices.Add(new CharCodeChoice(sname, item_names));
			} else {
				errmsgs += wxT("\n");
				errmsgs += _("Duplicate name in CharCodes::Choices : ");
				errmsgs += sname;
				return false;
			}
		}
		item = item->GetNext();
	}
	return true;
}

/// 文字コードを文字列に変換する
/// @param [in]  src         : 文字コード列
/// @param [in]  len         : srcバイト数
/// @param [out] dst         : 文字列
/// @param [in]  term_code   : 終端コード -1ならバイト数全て
void CharCodes::ConvToString(const wxUint8 *src, size_t len, wxString &dst, int term_code)
{
//	cache = maps[0];
	for(size_t i=0; i<len; ) {
		if (term_code >= 0 && *src == (wxUint8)term_code) break;
		size_t l = cache->FindString(src, i + 1 == len ? 1 : 2, dst);
		i += l;
		src += l;
	}
}

/// 文字列を文字コードに変換する
/// @param [in]  src         : 文字列
/// @param [out] dst         : 文字コード列
/// @param [in]  len         : dstバッファイサイズ
/// @return  >=0:バイト数  -1: 変換できなかった
int CharCodes::ConvToChars(const wxString &src, wxUint8 *dst, size_t len)
{
	bool rc = true;
	size_t pos = 0;
//	cache = maps[0];
	for(size_t i=0; i<src.Length() && pos < (len - 1) && rc; i++) {
		rc = cache->FindCode(src.Mid(i,1), dst, &pos);
	}
	if (dst) dst[pos]='\0';
	return rc ? (int)pos : -1;
}

/// 文字コードが文字変換テーブルにあるか
/// @param [in]  src         : 文字コード(1～2バイト)
/// @param [in]  len         : srcのバイト数
/// @param [out] dst         : 文字列
/// @param [in]  unknownchar : 変換できない場合に置き換える文字
/// @return 変換したバイト数
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

void CharCodes::SetMap(int idx)
{
	cache = gCharCodeMaps.GetMap(idx);
	if (!cache) {
		cache = gCharCodeMaps.GetMap(0);
	}
}
