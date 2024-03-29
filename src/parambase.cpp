/// @file parambase.cpp
///
/// @brief parameter templates
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "parambase.h"
#include "basicfmt/basiccommon.h"
#include "utils.h"
#include <wx/translation.h>
#include <wx/xml/xml.h>


//////////////////////////////////////////////////////////////////////
//
// ファイル名の命名規則を保持（バリデータ用）
//
ValidNameRule::ValidNameRule()
{
	name_require = false;
	max_length = 0;
}

/// 空にする
void ValidNameRule::Empty()
{
	valid_first_chars.Empty();
	valid_chars.Empty();
	invalid_chars.Empty();
	deduplicate_chars.Empty();
	name_require = false;
	max_length = 0;
}

//////////////////////////////////////////////////////////////////////
//
// 特別な属性などを保持する
//
MyAttribute::MyAttribute()
{
	idx = 0;
	type = 0;
	value = 0;
	mask = -1;
}
MyAttribute::MyAttribute(int n_idx, int n_type, int n_value, int n_mask, const wxString &n_name, const wxString &n_desc)
{
	idx = n_idx;
	type = n_type;
	value = n_value;
	mask = n_mask;
	name = n_name;
	desc = n_desc;

	if (name.IsEmpty()) name = wxT("???");
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfMyAttribute);

//////////////////////////////////////////////////////////////////////
//
// 特別な属性のリスト MyAttribute の配列
//
MyAttributes::MyAttributes()
	: ArrayOfMyAttribute()
{
}
/// 属性タイプと値に一致するアイテムを返す
const MyAttribute *MyAttributes::Find(int type, int value) const
{
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if (attr->GetType() == type && attr->GetValue() == (value & attr->GetMask())) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性タイプと値に一致するアイテムを返す
const MyAttribute *MyAttributes::Find(int type, int mask, int value) const
{
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if ((attr->GetType() & mask) == (type & mask) && attr->GetValue() == (value & attr->GetMask())) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性タイプと値に一致するアイテムを返す
const MyAttribute *MyAttributes::FindType(int type, int mask) const
{
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if ((attr->GetType() & mask) == (type & mask)) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性値に一致するアイテムを返す
const MyAttribute *MyAttributes::FindValue(int value) const
{
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if (attr->GetValue() == (value & attr->GetMask())) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名に一致するアイテムを返す
const MyAttribute *MyAttributes::Find(int type, const wxString &name) const
{
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if (attr->GetType() == type && attr->GetName() == name) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名に一致するアイテムを返す
const MyAttribute *MyAttributes::Find(const wxString &name) const
{
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if (attr->GetName() == name) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名に一致するアイテムを返す 大文字でマッチング
const MyAttribute *MyAttributes::FindUpperCase(const wxString &name) const
{
	wxString iname = name.Upper();
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if (attr->GetName().Upper() == iname) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名と属性タイプに一致するアイテムを返す 大文字でマッチング
const MyAttribute *MyAttributes::FindUpperCase(const wxString &name, int type, int mask) const
{
	wxString iname = name.Upper();
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if (attr->GetName().Upper() == iname && (attr->GetType() & mask) == (type & mask)) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名、属性タイプ、属性値に一致するアイテムを返す 大文字でマッチング
const MyAttribute *MyAttributes::FindUpperCase(const wxString &name, int type, int mask, int value) const
{
	wxString iname = name.Upper();
	const MyAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		MyAttribute *attr = &Item(i);
		if (attr->GetName().Upper() == iname && (attr->GetType() & mask) == (type & mask) && attr->GetValue() == value) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性値に一致するアイテムの位置を返す
int MyAttributes::GetIndexByValue(int value) const
{
	int idx = -1;
	const MyAttribute *match = FindValue(value);
	if (match) {
		idx = match->GetIndex();
	}
	return idx;
}
/// 属性値に一致するアイテムの属性タイプを返す
int MyAttributes::GetTypeByValue(int value) const
{
	int type = 0;
	const MyAttribute *match = FindValue(value);
	if (match) {
		type = match->GetType();
	}
	return type;
}
/// 位置から属性タイプを返す
int MyAttributes::GetTypeByIndex(int idx) const
{
	return idx < (int)Count() ? Item(idx).GetType() : 0;
}
/// 位置から属性値を返す
int MyAttributes::GetValueByIndex(int idx) const
{
	return idx < (int)Count() ? Item(idx).GetValue() : 0;
}

//////////////////////////////////////////////////////////////////////
//
// パラメータのテンプレートを提供する
//
TemplatesBase::TemplatesBase()
{
}

#if 0
/// XMLファイル読み込み
/// @param[in]  data_path   XMLファイルがあるフォルダ
/// @param[in]  locale_name ローケル名
/// @param[out] errmsgs     エラーメッセージ
/// @return true/false
bool TemplatesBase::Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs)
{
	return false;
}
#endif

/// Descriptionエレメントをロード
/// @param[in]  node        子ノード
/// @param[in]  locale_name ローケル名
/// @param[out] desc        説明
/// @param[out] desc_locale 説明ローケル
/// @return true
bool TemplatesBase::LoadDescription(const wxXmlNode *node, const wxString &locale_name, wxString &desc, wxString &desc_locale)
{
	if (node->HasAttribute("lang")) {
		wxString lang = node->GetAttribute("lang");
		if (locale_name.Find(lang) != wxNOT_FOUND) {
			desc_locale = node->GetNodeContent();
		}
	} else {
		desc = node->GetNodeContent();
	}
	return true;
}

/// 独自エレメントのロード
/// @param[in] node  子ノード
/// @param[in] val   入力値
/// @param[out] nval 変換後の値
/// @return true
bool TemplatesBase::LoadVariousParam(const wxXmlNode *node, const wxString &val, wxVariant &nval)
{
	wxString type = node->GetAttribute("type").Upper();
	if (type == "INT") {
		int ival = Utils::ToInt(val);
		nval = (long)ival;
	} else if (type == "BOOL") {
		bool bval = Utils::ToBool(val);
		nval = bval;
	} else if (type == "ARRAY") {
		wxString sval;
		Utils::DecodeEscape(val, sval);
		nval.Append(sval);
	} else {
		wxString sval;
		Utils::DecodeEscape(val, sval);
		nval = sval;
	}
	return true;
}

/// SpecialAttributes/AttributesByExtensionエレメントをロード
/// @param[in]  node        子ノード
/// @param[in]  locale_name ローケル名
/// @param[in]  type        タイプ
/// @param[out] attrs       値
/// @return true
bool TemplatesBase::LoadMyAttribute(const wxXmlNode *node, const wxString &locale_name, int type, MyAttributes &attrs)
{
	wxString desc, desc_locale;
	wxString name = node->GetAttribute("name");
	wxString sval = node->GetAttribute("value");
	wxString smsk = node->GetAttribute("mask");
	int val = Utils::ToInt(sval);
	int msk = (!smsk.IsEmpty() ? Utils::ToInt(smsk) : -1);
	wxXmlNode *cnode = node->GetChildren();
	while(cnode) {
		if (cnode->GetName() == "Description") {
			LoadDescription(cnode, locale_name, desc, desc_locale);
		}
		cnode = cnode->GetNext();
	}
	if (!desc_locale.IsEmpty()) {
		desc = desc_locale;
	}
	attrs.Add(MyAttribute((int)attrs.Count(), type, val, msk, name, desc));
	return true;
}

/// アトリビュート名と属性の関係マップ
static const struct st_special_attr_names {
	const char *name;
	int type;
} cSpecialAttrNames[] = {
	{ "MachineBinary",	FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK },
	{ "BasicBinary",	FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK },
	{ "BasicAscii",		FILE_TYPE_BASIC_MASK | FILE_TYPE_ASCII_MASK },
	{ "BasicInteger",	FILE_TYPE_BASIC_MASK | FILE_TYPE_INTEGER_MASK },
	{ "DataAscii",		FILE_TYPE_DATA_MASK | FILE_TYPE_ASCII_MASK },
	{ "DataBinary",		FILE_TYPE_DATA_MASK | FILE_TYPE_BINARY_MASK },
	{ "DataRandom",		FILE_TYPE_DATA_MASK | FILE_TYPE_RANDOM_MASK },
	{ "Binary",			FILE_TYPE_BINARY_MASK },
	{ "Ascii",			FILE_TYPE_ASCII_MASK },
	{ "Random",			FILE_TYPE_RANDOM_MASK },
	{ "Volume",			FILE_TYPE_VOLUME_MASK },
	{ "System",			FILE_TYPE_SYSTEM_MASK },
	{ NULL, 0 }
};

/// SpecialAttributes/AttributesByExtensionエレメントをロード
/// @param[in]  node        子ノード
/// @param[in]  locale_name ローケル名
/// @param[out] errmsgs     エラーメッセージ
/// @param[out] attrs       値
/// @return true
bool TemplatesBase::LoadMyAttributesInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs, MyAttributes &attrs)
{
	wxXmlNode *citemnode = node->GetChildren();
	while(citemnode) {
		for(int i=0; cSpecialAttrNames[i].name != NULL; i++) {
			if (citemnode->GetName() == cSpecialAttrNames[i].name) {
				LoadMyAttribute(citemnode, locale_name, cSpecialAttrNames[i].type, attrs);
				break;
			}
		}
		citemnode = citemnode->GetNext();
	}
	return true;
}

/// FileNameCharacters/VolumeNameCharactersエレメントをロード
/// @param[in]  node        子ノード
/// @param[out] valid_chars 値 
/// @param[out] errmsgs     エラー時メッセージ
/// @return true
bool TemplatesBase::LoadValidChars(const wxXmlNode *node, ValidNameRule &valid_chars, wxString &errmsgs)
{
	bool valid = true;
	wxString chars[4];
	wxXmlNode *cnode = node->GetChildren();
	while(cnode) {
		int encoding = Utils::ToInt(cnode->GetAttribute(wxT("encoding")));
		wxString name = cnode->GetName();
		int num = -1;
		if (name.Left(5) == "Valid") {
			num = 0;
			name = name.Mid(5);
		} else if (name.Left(7) == "Invalid") {
			num = 1;
			name = name.Mid(7);
		} else if (name.Left(9) == "Duplicate") {
			num = 2;
			name = name.Mid(9);
		}

		if (num == 0) {
			if (name.Left(5) == "First") {
				num = 3;
				name = name.Mid(5);
			}
		}

		if (num >= 0) {
			if (name == "CharSet") {
				wxString rstr;
				Utils::DecodeEscape(cnode->GetNodeContent(), rstr);
				chars[num] += rstr;
			} else if (name == "Code") {
				int c = Utils::ToInt(cnode->GetNodeContent());
				if (encoding == 0) {
					if (c < 0 || c >= 0x80) {
						errmsgs += wxT("\n");
						errmsgs += _("Out of range in InvalidateCharacters::Code");
						errmsgs += wxString::Format(_("(line #%d)"), cnode->GetLineNumber());
						valid = false;
					} else {
						chars[num] += wxString((char)c, (size_t)1);
					}
				} else {
					// unicode
					chars[num] += wxString((wchar_t)c, (size_t)1);
				}
			} else if (name == "CodeRange") {
				int st = Utils::ToInt(cnode->GetAttribute("first"));
				int ed = Utils::ToInt(cnode->GetAttribute("last"));
				if (encoding == 0) {
					for(int c=st; c<=ed; c++) {
						if (c < 0 || c >= 0x80) {
							errmsgs += wxT("\n");
							errmsgs += _("Out of range in InvalidateCharacters::CodeRange");
							errmsgs += wxString::Format(_("(line #%d)"), cnode->GetLineNumber());
							valid = false;
							break;
						} else {
							chars[num] += wxString((char)c, (size_t)1);
						}
					}
				} else {
					// unicode
					for(int c=st; c<=ed; c++) {
						chars[num] += wxString((wchar_t)c, (size_t)1);
					}
				}
			}
		}
		cnode = cnode->GetNext();
	}
	if (valid) {
		valid_chars.SetValidChars(chars[0]);
		valid_chars.SetInvalidChars(chars[1]);
		valid_chars.SetDeduplicateChars(chars[2]);
		valid_chars.SetValidFirstChars(chars[3]);
	}
	return valid;
}

/// FileNameCompareCaseエレメントをロード
/// @param[in] node 子ノード
/// @param[out] val 値
/// @return true
bool TemplatesBase::LoadFileNameCompareCase(const wxXmlNode *node, bool &val)
{
	wxString str = node->GetNodeContent();
	val = (str.Upper() == wxT("INSENSITIVE"));
	return true;
}

