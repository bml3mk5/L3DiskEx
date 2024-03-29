/// @file basiccategory.h
///
/// @brief basic category
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICCATEGORY_H
#define BASICCATEGORY_H

#include "../common.h"
#include "../parambase.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/variant.h>
#include <wx/hashmap.h>
#include "basiccommon.h"
#include "../diskimg/diskparam.h"
#include "../diskimg/diskimage.h"

class wxXmlNode;

//////////////////////////////////////////////////////////////////////

/// @brief DISK BASICのカテゴリ(メーカ毎、OS毎にまとめる)クラス
class DiskBasicCategory
{
private:
	wxString	m_name;
	wxString	m_description;

public:
	DiskBasicCategory();
	DiskBasicCategory(const DiskBasicCategory &src);
	DiskBasicCategory(const wxString & n_name, const wxString & n_description);
	virtual ~DiskBasicCategory() {}

	/// @brief カテゴリ名
	const wxString&	GetName() const	{ return m_name; }
	/// @brief 説明
	const wxString& GetDescription() const { return m_description; }
	/// @brief 説明
	void			SetDescription(const wxString &str) { m_description = str; }
};

//////////////////////////////////////////////////////////////////////

/// @class ArrayOfDiskBasicCategory
///
/// @brief DiskBasicCategory の配列
WX_DECLARE_OBJARRAY(DiskBasicCategory, ArrayOfDiskBasicCategory);

//////////////////////////////////////////////////////////////////////

/// @brief DiskBasicCategory のリスト
class DiskBasicCategories : public ArrayOfDiskBasicCategory
{
public:
	/// @brief DiskBasicCategoryエレメントのロード
	bool Load(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs);
	/// @brief カテゴリを検索
	const DiskBasicCategory *Find(const wxString &n_category) const;
	/// @brief カテゴリ名を返す
	const wxString &GetName(int idx) const;
};

#endif /* BASICCATEGORY_H */
