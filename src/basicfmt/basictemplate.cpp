/// @file basictemplate.cpp
///
/// @brief disk basic template
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictemplate.h"
#include "../utils.h"
#include <wx/xml/xml.h>


DiskBasicTemplates gDiskBasicTemplates;

//////////////////////////////////////////////////////////////////////
//
// DISK BASICパラメータのテンプレートを提供する
//
DiskBasicTemplates::DiskBasicTemplates()
{
}
/// XMLファイル読み込み
/// @param[in]  data_path   XMLファイルがあるフォルダ
/// @param[in]  locale_name ローケル名
/// @param[out] errmsgs     エラーメッセージ
/// @return true/false
bool DiskBasicTemplates::Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("basic_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "DiskBasics") return false;

	bool valid = true;
	valid = formats.Load(doc.GetRoot()->GetChildren(), locale_name, errmsgs);
	if (!valid) return false;

	valid = types.Load(doc.GetRoot()->GetChildren(), locale_name, formats, errmsgs);
	if (!valid) return false;


	if (!doc.Load(data_path + wxT("category_types.xml"))) return false;

	valid = categories.Load(doc.GetRoot(), locale_name, errmsgs);

	return valid;
}

/// カテゴリとタイプに一致するパラメータを検索
/// @param [in] n_category  : カテゴリ名 空文字列の場合は検索条件からはずす
/// @param [in] n_basic_type: タイプ名
/// @return 一致したパラメータ
const DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const wxString &n_basic_type) const
{
	return types.Find(n_category, n_basic_type);
}
/// カテゴリが一致し、タイプリストに含まれるパラメータを検索
/// @param [in] n_category   : カテゴリ名 空文字列の場合は検索条件からはずす
/// @param [in] n_basic_types: タイプ名リスト
/// @return 一致したパラメータ
const DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const DiskParamNames &n_basic_types) const
{
	return types.Find(n_category, n_basic_types);
}
/// カテゴリ、タイプ、サイド数とセクタ数が一致するパラメータを検索
/// @note まず、カテゴリ＆タイプで検索し、なければカテゴリ＆サイド数＆セクタ数で検索
/// @param [in] n_category   : カテゴリ名 必須
/// @param [in] n_basic_type : タイプ名 必須
/// @param [in] n_sides      : サイド数
/// @param [in] n_sectors    : セクタ数/トラック -1の場合は検索条件からはずす
/// @return 一致したパラメータ
const DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const wxString &n_basic_type, int n_sides, int n_sectors) const
{
	return types.Find(n_category, n_basic_type, n_sides, n_sectors);
}
/// DISK BASICフォーマット種類に一致するタイプを検索
/// @param [in]  n_format_types : DISK BASICフォーマット種類
/// @param [out] n_types        : 一致したタイプリスト
/// @return リストの数
size_t DiskBasicTemplates::FindTypes(const wxArrayInt &n_format_types, DiskBasicParams &n_types) const
{
	return types.FindTypes(n_format_types, n_types);
}
/// カテゴリ番号に一致するタイプ名リストを検索
/// @param [in]  n_category_index : カテゴリ番号
/// @param [out] n_type_names     : タイプ名リスト
/// @return リストの数
size_t DiskBasicTemplates::FindTypeNames(size_t n_category_index, wxArrayString &n_type_names) const
{
	return types.FindNames(categories.GetName((int)n_category_index), n_type_names);
}
/// カテゴリ名に一致するタイプ名リストを検索
/// @param [in]  n_category_name  : カテゴリ名
/// @param [out] n_type_names     : タイプ名リスト
/// @return リストの数
size_t DiskBasicTemplates::FindTypeNames(const wxString &n_category_name, wxArrayString &n_type_names) const
{
	return types.FindNames(n_category_name, n_type_names);
}
/// フォーマット種類を検索
/// @param [in]  format_type  : フォーマット種類
const DiskBasicFormat *DiskBasicTemplates::FindFormat(DiskBasicFormatType format_type) const
{
	return formats.Find(format_type);
}

/// タイプリストと一致するパラメータを得る
/// @note パラメータリストは説明文でソートする
/// @param [in]  n_type_names     : タイプ名リスト
/// @param [out] params           : パラメータリスト
/// @return パラメータリストの数
size_t DiskBasicTemplates::FindParams(const DiskParamNames &n_type_names, DiskBasicParamPtrs &params) const
{
	for(size_t n = 0; n < n_type_names.Count(); n++) {
		const DiskBasicParam *param = FindType(wxEmptyString, n_type_names.Item(n).GetName());
		if (!param) continue;
		params.Add(param);
	}
	params.Sort(&DiskBasicParam::SortByDescription);

	return params.Count();
}

/// カテゴリを検索
/// @param [in]  n_category : カテゴリ名
/// @return : カテゴリ
const DiskBasicCategory *DiskBasicTemplates::FindCategory(const wxString &n_category) const
{
	return categories.Find(n_category);
}

/// @brief カテゴリ名を返す
/// @param [in]  idx : インデックス
/// @return : カテゴリ名
const wxString &DiskBasicTemplates::GetCategoryName(int idx) const
{
	return categories.GetName(idx);
}
