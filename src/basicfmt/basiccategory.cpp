/// @file basiccategory.cpp
///
/// @brief basic category
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basiccategory.h"
#include "../utils.h"
#include <wx/xml/xml.h>


//////////////////////////////////////////////////////////////////////
//
//　DISK BASICのカテゴリ(メーカ毎、OS毎にまとめる)クラス
//
DiskBasicCategory::DiskBasicCategory()
{
}
DiskBasicCategory::DiskBasicCategory(const DiskBasicCategory &src)
{
	m_name = src.m_name;
	m_description = src.m_description;
}
DiskBasicCategory::DiskBasicCategory(const wxString & n_name, const wxString & n_description)
{
	m_name = n_name;
	m_description = n_description;
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfDiskBasicCategory);

//////////////////////////////////////////////////////////////////////

/// DiskBasicCategoryエレメントのロード
/// @param[in] node        ノード
/// @param[in] locale_name ローケル名
/// @param[out] errmsgs    エラー時メッセージ
/// @return true / false
bool DiskBasicCategories::Load(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	bool valid = false;
	while(node && !valid) {
		if (node->GetName() == "DiskBasicCategories") {
			valid = true;
			break;
		}
		node = node->GetNext();
	}
	if (!valid) return false;

	valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "DiskBasicCategory") {
			wxString type_name = item->GetAttribute("name");
			wxString desc, desc_locale;

			wxXmlNode *itemnode = item->GetChildren();
			while (itemnode) {
				if (itemnode->GetName() == "Description") {
					if (itemnode->HasAttribute("lang")) {
						wxString lang = itemnode->GetAttribute("lang");
						if (locale_name.Find(lang) != wxNOT_FOUND) {
							desc_locale = itemnode->GetNodeContent();
						}
					} else {
						desc = itemnode->GetNodeContent();
					}
				}
				itemnode = itemnode->GetNext();
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			DiskBasicCategory c(
				type_name,
				desc
			);
			if (Find(type_name) == NULL) {
				Add(c);
			} else {
				errmsgs += wxT("\n");
				errmsgs += _("Duplicate type name in DiskBasicCategory : ");
				errmsgs += type_name;
				valid = false;
				break;
			}
		}
		item = item->GetNext();
	}
	return valid;
}

/// カテゴリを検索
/// @param [in]  n_category : カテゴリ名
/// @return : カテゴリ
const DiskBasicCategory *DiskBasicCategories::Find(const wxString &n_category) const
{
	DiskBasicCategory *match = NULL;
	for(size_t n=0; n<Count(); n++) {
		DiskBasicCategory *item = &Item(n);
		if (item->GetName() == n_category) {
			match = item;
			break;
		}
	}
	return match;
}

/// カテゴリ名を返す
/// @param [in]  idx : インデックス
/// @return : カテゴリ名
const wxString &DiskBasicCategories::GetName(int idx) const
{
	return Item(idx).GetName();
}
