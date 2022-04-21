/// @file charcodes.h
///
/// @brief キャラクタコード
///
#ifndef _CHARCODES_H_
#define _CHARCODES_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>

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

	/// 文字コードが文字変換テーブルにあるか
	virtual size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// 文字が文字変換テーブルにあるか
	virtual bool FindCode(const wxString &src, wxUint8 *dst, size_t *pos);
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
	bool FindCode(const wxString &src, wxUint8 *dst, size_t *pos);
};

WX_DEFINE_ARRAY(CharCodeMap *, CharCodeMapList);

/// キャラクターコード変換マップリスト
class CharCodeMaps
{
private:
	CharCodeMapList maps;
public:
	CharCodeMaps();
	~CharCodeMaps();

	bool Load(const wxString &data_path);
	const CharCodeMapList &GetMaps() const { return maps; }
	CharCodeMap *GetMap(size_t index);
	CharCodeMap *FindMap(const wxString &name);
};

/// キャラクターコード変換操作
class CharCodes
{
private:
	CharCodeMap *cache;

public:
	CharCodes();
	~CharCodes();

	/// 文字コードを文字列に変換する
	void ConvToString(const wxUint8 *src, size_t len, wxString &dst);
	/// 文字列を文字コードに変換する
	bool ConvToChars(const wxString &src, wxUint8 *dst, size_t len);

	/// 文字コードが文字変換テーブルにあるか
	size_t FindString(const wxUint8 *src, size_t remain, wxString &dst, wxUint8 unknownchar = '_');
	/// 文字が文字変換テーブルにあるか
	bool FindCode(const wxString &src, wxUint8 *dst, size_t *pos);

	void SetMap(const wxString &name);
};

extern CharCodeMaps gCharCodeMaps;

#endif /* _CHARCODES_H_ */
