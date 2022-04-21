/// @file charcodes.h
///
/// @brief キャラクタコード
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _CHARCODES_H_
#define _CHARCODES_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/arrstr.h>

class wxCSConv;

/// キャラクターコード１文字の変換情報
class CharCode
{
public:
	wxString str;
	wxUint8  code[4];
	size_t   code_len;

public:
	CharCode();
	CharCode(const wxString &newstr, const wxString &newcode);
	~CharCode() {}
};

WX_DEFINE_ARRAY(CharCode *, CharCodeList);

/// キャラクターコード変換マップ
class CharCodeMap
{
protected:
	wxString     name;
	CharCodeList list;
	int          type;
	int          font_encoding;
	wxString	 description;

	CharCodeMap(const CharCodeMap &) {}

public:
	CharCodeMap();
	CharCodeMap(const wxString &n_name, int n_type);
	virtual ~CharCodeMap();

	virtual void Initialize() {}

	const wxString &GetName() const { return name; }
	CharCodeList &GetList() { return list; }
	int GetFontEncoding() const { return font_encoding; }
	void SetFontEncoding(int val) { font_encoding = val; }
	const wxString &GetDescription() const { return description; }
	void SetDescription(const wxString &val) { description = val; }

	/// 文字コードが文字変換テーブルにあるか
	virtual size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// 文字が文字変換テーブルにあるか
	virtual bool FindCode(const wxString &src, wxUint8 *dst, size_t &pos);
};

/// キャラクターコード変換マップ Shift-JIS変換用
class CharCodeMapMB : public CharCodeMap
{
private:
	wxCSConv *cs;

	CharCodeMapMB(const CharCodeMapMB &src) : CharCodeMap(src) {}

public:
	CharCodeMapMB();
	CharCodeMapMB(const wxString &n_name, int n_type);
	~CharCodeMapMB();

	void Initialize();

	/// 文字コード１文字を(SJIS)を文字列に変換する
	size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// 文字が文字変換テーブルにあるか
	bool FindCode(const wxString &src, wxUint8 *dst, size_t &pos);
};

WX_DEFINE_ARRAY(CharCodeMap *, ArrayOfCharCodeMap);

/// キャラクターコード変換マップリスト
class CharCodeMaps
{
private:
	ArrayOfCharCodeMap maps;
public:
	CharCodeMaps();
	~CharCodeMaps();

	void Add(CharCodeMap *map);
	const ArrayOfCharCodeMap &GetMaps() const { return maps; }
	CharCodeMap *GetMap(size_t index);
	CharCodeMap *FindMap(const wxString &name);
//	wxArrayString GetDescriptions() const;
};

/// キャラクターコード選択リスト
class CharCodeChoice
{
private:
	wxString			name;
	wxArrayString		item_names;
	ArrayOfCharCodeMap	maps;

public:
	CharCodeChoice(const wxString &n_name, const wxArrayString &n_item_names);
	~CharCodeChoice();

	void AssignMaps();
	const wxString &GetName() const { return name; }
	size_t Count() const;
	CharCodeMap *Item(size_t idx) const;
	const wxString &GetItemName(size_t idx) const;
	CharCodeMap *Find(const wxString &name) const;
	int IndexOf(const wxString &name) const;
};

WX_DEFINE_ARRAY(CharCodeChoice *, ArrayOfCharCodeChoice);

/// キャラクターコード選択リスト
class CharCodeChoices
{
private:
	ArrayOfCharCodeChoice choices;

public:
	CharCodeChoices();
	~CharCodeChoices();

	void AssignMaps();
	void Add(CharCodeChoice *choice);
	CharCodeChoice *Find(const wxString &n_name) const;
	const wxString &GetItemName(const wxString &name, size_t item_idx) const;
	int IndexOf(const wxString &name, const wxString &item_name) const;
};

class wxXmlNode;

/// キャラクターコード変換操作
class CharCodes
{
private:
	CharCodeMap *cache;

	static bool LoadMaps(wxXmlNode *item, const wxString &locale_name, wxString &errmsgs);
	static bool LoadChoices(wxXmlNode *item, const wxString &locale_name, wxString &errmsgs);

public:
	CharCodes();
	~CharCodes();

	static bool Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs);

	/// 文字コードを文字列に変換する
	void ConvToString(const wxUint8 *src, size_t len, wxString &dst, int term_code = -1);
	/// 文字列を文字コードに変換する
	int  ConvToChars(const wxString &src, wxUint8 *dst, size_t len);

	/// 文字コードが文字変換テーブルにあるか
	size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// 文字が文字変換テーブルにあるか
	bool FindCode(const wxString &src, wxUint8 *dst, size_t &pos);

	void SetMap(const wxString &name);
	void SetMap(int idx);
};

extern CharCodeMaps		gCharCodeMaps;
extern CharCodeChoices	gCharCodeChoices;

#endif /* _CHARCODES_H_ */
