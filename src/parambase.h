/// @file parambase.h
///
/// @brief parameter template
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef PARAMBASE_H
#define PARAMBASE_H

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/variant.h>
#include <wx/hashmap.h>

//////////////////////////////////////////////////////////////////////

/// @class VariantHash
///
/// @brief 任意の値を保持するハッシュ
WX_DECLARE_STRING_HASH_MAP(wxVariant, VariantHash);

//////////////////////////////////////////////////////////////////////

/// @brief ファイル名の命名規則を保持（バリデータ用）
class ValidNameRule
{
private:
	wxString valid_first_chars;			///< ファイル名の先頭に設定できる文字
	wxString valid_chars;				///< ファイル名に設定できる文字
	wxString invalid_chars;				///< ファイル名に設定できない文字
	wxString deduplicate_chars;			///< ファイル名に重複指定できない文字
	bool	 name_require;				///< ファイル名が必須か
	size_t	 max_length;				///< ファイル名のサイズ
public:
	ValidNameRule();
	~ValidNameRule() {}

	/// @brief 空にする
	void Empty();

	/// @brief ファイル名の先頭に設定できる文字
	const wxString&		GetValidFirstChars() const	{ return valid_first_chars; }
	/// @brief ファイル名に設定できる文字
	const wxString&		GetValidChars() const	{ return valid_chars; }
	/// @brief ファイル名に設定できない文字
	const wxString&		GetInvalidChars() const	{ return invalid_chars; }
	/// @brief ファイル名に重複指定できない文字
	const wxString&		GetDeduplicateChars() const	{ return deduplicate_chars; }
	/// @brief ファイル名が必須か
	bool				IsNameRequired() const	{ return name_require; }
	/// @brief ファイル名のサイズ
	size_t				GetMaxLength() const	{ return max_length; }

	/// @brief ファイル名の先頭に設定できる文字
	void			SetValidFirstChars(const wxString &str)	{ valid_first_chars = str; }
	/// @brief ファイル名に設定できる文字
	void			SetValidChars(const wxString &str)	{ valid_chars = str; }
	/// @brief ファイル名に設定できない文字
	void			SetInvalidChars(const wxString &str)	{ invalid_chars = str; }
	/// @brief ファイル名に重複指定できない文字
	void			SetDeduplicateChars(const wxString &str)	{ deduplicate_chars = str; }
	/// @brief ファイル名が必須か
	void			RequireName(bool val)		{ name_require = val; }
	/// @brief ファイル名のサイズ
	void			SetMaxLength(size_t val)	{ max_length = val; }
};

//////////////////////////////////////////////////////////////////////

/// @brief 特別な属性などを保持する
///
/// @sa MyAttributes
class MyAttribute
{
private:
	int idx;		///< インデックス
	int type;		///< 属性タイプ
	int value;		///< 属性値
	int mask;		///< マスク
	wxString name;	///< 名前
	wxString desc;	///< 説明

public:
	MyAttribute();
	MyAttribute(int n_idx, int n_type, int n_value, int n_mask, const wxString &n_name, const wxString &n_desc);
	/// @brief インデックス
	int GetIndex() const { return idx; }
	/// @brief 属性タイプ
	int GetType() const { return type; }
	/// @brief 属性値
	int GetValue() const { return value; }
	/// @brief マスク
	int GetMask() const { return mask; }
	/// @brief 名前
	const wxString &GetName() const { return name; }
	/// @brief 説明
	const wxString &GetDescription() const { return desc; }
};

//////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(MyAttribute, ArrayOfMyAttribute);

//////////////////////////////////////////////////////////////////////

/// @brief 特別な属性のリスト MyAttribute の配列
class MyAttributes : public ArrayOfMyAttribute
{
public:
	MyAttributes();
	/// @brief 属性タイプと値に一致するアイテムを返す
	const MyAttribute *Find(int type, int value) const;
	/// @brief 属性タイプと値に一致するアイテムを返す
	const MyAttribute *Find(int type, int mask, int value) const;
	/// @brief 属性タイプに一致するアイテムを返す
	const MyAttribute *FindType(int type, int mask) const;
	/// @brief 属性値に一致するアイテムを返す
	const MyAttribute *FindValue(int value) const;
	/// @brief 属性名に一致するアイテムを返す
	const MyAttribute *Find(int type, const wxString &name) const;
	/// @brief 属性名に一致するアイテムを返す
	const MyAttribute *Find(const wxString &name) const;
	/// @brief 属性名に一致するアイテムを返す 大文字でマッチング
	const MyAttribute *FindUpperCase(const wxString &name) const;
	/// @brief 属性名と属性タイプに一致するアイテムを返す 大文字でマッチング
	const MyAttribute *FindUpperCase(const wxString &name, int type, int mask) const;
	/// @brief 属性名、属性タイプ、属性値に一致するアイテムを返す 大文字でマッチング
	const MyAttribute *FindUpperCase(const wxString &name, int type, int mask, int value) const;
	/// @brief 属性値に一致するアイテムの位置を返す
	int					GetIndexByValue(int value) const;
	/// @brief 属性値に一致するアイテムの属性値を返す
	int					GetTypeByValue(int value) const;
	/// @brief 位置から属性タイプを返す
	int					GetTypeByIndex(int idx) const;
	/// @brief 位置から属性値を返す
	int					GetValueByIndex(int idx) const;
};

//////////////////////////////////////////////////////////////////////

class wxXmlNode;

/// @brief パラメータのテンプレートを提供する
class TemplatesBase
{
protected:
	/// @brief Descriptionエレメントをロード
	static bool LoadDescription(const wxXmlNode *node, const wxString &locale_name, wxString &desc, wxString &desc_locale);
	/// @brief 独自エレメントのロード
	static bool LoadVariousParam(const wxXmlNode *node, const wxString &val, wxVariant &nval);
	/// @brief SpecialAttributes/AttributesByExtensionエレメントをロード
	static bool LoadMyAttribute(const wxXmlNode *node, const wxString &locale_name, int type, MyAttributes &attrs);
	/// @brief SpecialAttributes/AttributesByExtensionエレメントをロード
	static bool LoadMyAttributesInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs, MyAttributes &attrs);
	/// @brief FileNameCharacters/VolumeNameCharactersエレメントをロード
	static bool LoadValidChars(const wxXmlNode *node, ValidNameRule &valid_chars, wxString &errmsgs);
	/// @brief FileNameCompareCaseエレメントをロード
	static bool LoadFileNameCompareCase(const wxXmlNode *node, bool &val);

public:
	TemplatesBase();
	virtual ~TemplatesBase() {}

//	/// @brief XMLファイル読み込み
//	virtual bool Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs);
};

#endif /* PARAMBASE_H */
