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

/// @brief キャラクターコード１文字の変換情報
///
/// @sa CharCodeMap
class CharCode
{
public:
	wxString str;
	wxUint8  code[4];
	size_t   code_len;

public:
	CharCode();
	/// @brief 文字登録
	CharCode(const wxString &newstr, const wxString &newcode);
	~CharCode() {}
};

/// @class CharCodeList
///
/// @brief CharCode のポインタ配列
///
/// @sa CharCode , CharCodeMap
WX_DEFINE_ARRAY(CharCode *, CharCodeList);

/// @brief キャラクターコード変換マップ
///
/// @sa CharCodeList, CharCode
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
	/// @brief マップ情報登録
	CharCodeMap(const wxString &n_name, int n_type);
	virtual ~CharCodeMap();

	virtual void Initialize() {}

	/// @brief マップ名を返す
	const wxString &GetName() const { return name; }
	/// @brief マップリストを返す
	CharCodeList &GetList() { return list; }
	/// @brief エンコード番号を返す
	int GetFontEncoding() const { return font_encoding; }
	/// @brief エンコード番号を設定
	void SetFontEncoding(int val) { font_encoding = val; }
	/// @brief マップ説明を返す
	const wxString &GetDescription() const { return description; }
	/// @brief マップ説明を設定
	void SetDescription(const wxString &val) { description = val; }

	/// @brief コントロールコードを(必要なら)変換する
	virtual void ConvCtrlCodes(wxUint8 *str, size_t len) {}
	/// @brief 文字コードが文字変換テーブルにあるか
	virtual size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// @brief 文字が文字変換テーブルにあるか
	virtual bool FindCode(const wxString &src, wxUint8 *dst, size_t &pos);
};

/// @brief キャラクターコード変換マップ Shift-JIS変換用 wxMBConvを使って変換する
class CharCodeMapMB : public CharCodeMap
{
protected:
	wxCSConv *cs;

	CharCodeMapMB(const CharCodeMapMB &src) : CharCodeMap(src) {}

public:
	CharCodeMapMB();
	/// @brief マップ情報登録
	CharCodeMapMB(const wxString &n_name, int n_type);
	virtual ~CharCodeMapMB();

	virtual void Initialize();

	/// @brief 文字コード１文字を(SJIS)を文字列に変換する
	virtual size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// @brief 文字が文字変換テーブルにあるか
	virtual bool FindCode(const wxString &src, wxUint8 *dst, size_t &pos);
};

/// @brief キャラクターコード変換マップ iso-8859-1用
class CharCodeMapSB : public CharCodeMapMB
{
protected:
	CharCodeMapSB(const CharCodeMapSB &src) : CharCodeMapMB(src) {}

public:
	CharCodeMapSB();
	CharCodeMapSB(const wxString &n_name, int n_type);

	/// @brief 文字コード１文字を文字列に変換する
	virtual size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
};

/// @brief キャラクターコード変換マップ Ascii 7bit用
class CharCodeMap7 : public CharCodeMap
{
public:
	CharCodeMap7();
	CharCodeMap7(const wxString &n_name, int n_type);

	/// @brief コントロールコードを(必要なら)変換する
	void ConvCtrlCodes(wxUint8 *str, size_t len);
	/// @brief 文字コード１文字を文字列に変換する
	size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// @brief 文字が文字変換テーブルにあるか
	bool FindCode(const wxString &src, wxUint8 *dst, size_t &pos);
};

WX_DEFINE_ARRAY(CharCodeMap *, ArrayOfCharCodeMap);

/// @brief キャラクターコード変換マップ一覧リスト CharCodeMap のポインタリスト
///
/// キャラクターコード変換マップの集合体
///
/// @sa CharCodeMap
class CharCodeMaps : public ArrayOfCharCodeMap
{
public:
	CharCodeMaps();
	~CharCodeMaps();

//	void Add(CharCodeMap *map);
//	const ArrayOfCharCodeMap &GetMaps() const { return maps; }
	/// @brief 指定位置のマップを返す
	CharCodeMap *GetMap(size_t index);
	/// @brief マップを探す
	CharCodeMap *FindMap(const wxString &name);
//	wxArrayString GetDescriptions() const;
};

/// @brief キャラクターコード選択リスト CharCodeMap のポインタリスト
///
/// @sa CharCodeMap
class CharCodeChoice : public ArrayOfCharCodeMap
{
private:
	wxString			name;
	wxArrayString		item_names;

public:
	CharCodeChoice(const wxString &n_name, const wxArrayString &n_item_names);
	~CharCodeChoice();
	/// @brief 使用するマップを設定
	void AssignMaps();
	/// @brief 選択リスト名を返す
	const wxString &GetName() const { return name; }
//	size_t Count() const;
//	CharCodeMap *Item(size_t idx) const;
	/// @brief マップ名を返す
	const wxString &GetItemName(size_t idx) const;
	/// @brief マップ名に一致するマップを探す
	CharCodeMap *Find(const wxString &name) const;
	/// @brief マップ名に一致するマップを探す
	int IndexOf(const wxString &name) const;
};

WX_DEFINE_ARRAY(CharCodeChoice *, ArrayOfCharCodeChoice);

/// @brief キャラクターコード選択リストの集合 CharCodeChoice のポインタリスト
///
/// ファイルリスト画面やダンプ画面で選択できるキャラクターコードリストを保持
///
/// @sa CharCodeChoice
class CharCodeChoices : public ArrayOfCharCodeChoice
{
public:
	CharCodeChoices();
	~CharCodeChoices();
	/// @brief 使用するマップを設定
	void AssignMaps();
//	void Add(CharCodeChoice *choice);
	/// @brief 選択リスト名に一致する選択リストを探す
	CharCodeChoice *Find(const wxString &n_name) const;
	/// @brief 選択リスト名に一致するマップを探す
	const wxString &GetItemName(const wxString &name, size_t item_idx) const;
	/// @brief 選択リスト名に一致するマップを探す
	int IndexOf(const wxString &name, const wxString &item_name) const;
};

class wxXmlNode;

/// @brief キャラクターコード変換操作
class CharCodes
{
private:
	CharCodeMap *cache;

	/// @brief Mapsエレメントをロード
	static bool LoadMaps(wxXmlNode *item, const wxString &locale_name, wxString &errmsgs);
	/// @brief Choicesエレメントをロード
	static bool LoadChoices(wxXmlNode *item, const wxString &locale_name, wxString &errmsgs);

public:
	CharCodes();
	~CharCodes();

	/// @brief XMLからパラメータをロード
	static bool Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs);

	/// @brief 文字コードを文字列に変換する
	void ConvToString(const wxUint8 *src, size_t len, wxString &dst, int term_code = -1);
	/// @brief 文字列を文字コードに変換する
	int  ConvToChars(const wxString &src, wxUint8 *dst, size_t len);
	/// @brief コントロールコードを(必要なら)変換する
	void ConvCtrlCodes(wxUint8 *str, size_t len);

	/// @brief 文字コードが文字変換テーブルにあるか
	size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// @brief 文字が文字変換テーブルにあるか
	bool FindCode(const wxString &src, wxUint8 *dst, size_t &pos);

	/// @brief マップを設定
	void SetMap(const wxString &name);
	/// @brief マップを設定
	void SetMap(int idx);
};

extern CharCodeMaps		gCharCodeMaps;
extern CharCodeChoices	gCharCodeChoices;

#endif /* _CHARCODES_H_ */
