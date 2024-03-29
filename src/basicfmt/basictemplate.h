/// @file basictemplate.h
///
/// @brief disk basic template
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTEMPLATE_H
#define BASICTEMPLATE_H

#include "../common.h"
#include "../parambase.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/variant.h>
#include <wx/hashmap.h>
#include "basiccommon.h"
#include "basicparam.h"
#include "basiccategory.h"
#include "../diskimg/diskparam.h"
#include "../diskimg/diskimage.h"


//////////////////////////////////////////////////////////////////////

class wxXmlNode;

/// @brief DISK BASICパラメータのテンプレートを提供する
class DiskBasicTemplates : public TemplatesBase
{
private:
	DiskBasicFormats	formats;
	DiskBasicParams		types;
	DiskBasicCategories	categories;

public:
	DiskBasicTemplates();
	~DiskBasicTemplates() {}

	/// @brief XMLファイル読み込み
	bool Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs);

	/// @brief カテゴリとタイプに一致するパラメータを検索
	const DiskBasicParam *FindType(const wxString &n_category, const wxString &n_basic_type) const;
	/// @brief カテゴリが一致し、タイプリストに含まれるパラメータを検索
	const DiskBasicParam *FindType(const wxString &n_category, const DiskParamNames &n_basic_types) const;
	/// @brief カテゴリ、タイプ、サイド数とセクタ数が一致するパラメータを検索
	const DiskBasicParam *FindType(const wxString &n_category, const wxString &n_basic_type, int n_sides, int n_sectors) const;
	/// @brief DISK BASICフォーマット種類に一致するタイプを検索
	size_t FindTypes(const wxArrayInt &n_format_types, DiskBasicParams &n_types) const;
	/// @brief カテゴリ番号に一致するタイプ名リストを検索
	size_t FindTypeNames(size_t n_category_index, wxArrayString &n_type_names) const;
	/// @brief カテゴリ名に一致するタイプ名リストを検索
	size_t FindTypeNames(const wxString &n_category_name, wxArrayString &n_type_names) const;
	/// @brief フォーマット種類を検索
	const DiskBasicFormat *FindFormat(DiskBasicFormatType format_type) const;
	/// @brief タイプリストと一致するパラメータを得る
	size_t FindParams(const DiskParamNames &n_type_names, DiskBasicParamPtrs &params) const;
	/// @brief カテゴリを検索
	const DiskBasicCategory *FindCategory(const wxString &n_category) const;
	/// @brief カテゴリリストを返す
	const DiskBasicCategories &GetCategories() const { return categories; }
	/// @brief カテゴリを検索
	const wxString &GetCategoryName(int idx) const;
};

extern DiskBasicTemplates gDiskBasicTemplates;

#endif /* BASICTEMPLATE_H */
