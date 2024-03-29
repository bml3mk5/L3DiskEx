/// @file utils.h
///
/// @brief いろいろ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKUTILS_H
#define DISKUTILS_H

#include "common.h"
#include <wx/string.h>
#include <wx/stopwatch.h>
#include "charcodes.h"


/// 各種ユーティリティー
namespace Utils
{
#define TEMP_DATA_SIZE 2048

/// データの書き出しや読み込みで使用するテンポラリバッファ
class TempData
{
private:
	wxUint8 *data;
	size_t  alloc_size;
	size_t  size;
public:
	TempData();
	TempData(size_t newsize);
	~TempData();
	/// @brief バッファを(再)確保
	void SetSize(size_t newsize);
	/// @brief バッファにデータをセット
	void SetData(const wxUint8 *data, size_t len, bool invert = false);
	/// @brief バッファにデータをセット
	void Set(size_t pos, wxUint8 val);
	/// @brief 一致するバイトデータを置換
	void Replace(wxUint8 src, wxUint8 dst);
	/// @brief バッファのポインタを返す
	wxUint8 *GetData() { return data; }
	/// @brief バッファの指定位置のポインタを返す
	wxUint8 *GetData(size_t pos) { return &data[pos]; }
	/// @brief データサイズを返す
	size_t GetSize() const { return size; }
	/// @brief バッファサイズを返す
	size_t GetBufferSize() const { return alloc_size; }
	/// @brief データを反転
	void InvertData(bool invert);
};

/// FIFOバッファ
class FIFOBuffer
{
private:
	wxUint8 *m_data;
	size_t	 m_size;
	size_t   m_rpos;
	size_t	 m_wpos;
public:
	FIFOBuffer(size_t val = 4096);
	virtual ~FIFOBuffer();
	/// @brief バッファサイズを設定
	void SetBufSize(size_t val);
	/// @brief バッファをクリア
	virtual void Clear();
	/// @brief データを追加
	void AppendByte(wxUint8 val);
	/// @brief データを追加
	void AppendData(const wxUint8 *buf, size_t size);
	/// @brief データを返す
	int PeekByte() const;
	/// @brief データを返す
	int GetByte();
	/// @brief データを得る
	size_t GetData(wxUint8 *buf, size_t size);
	/// @brief データバッファを返す
	wxUint8 *GetData() { return m_data; }
	/// @brief リード位置を返す
	size_t GetReadPos() const { return m_rpos; }
	/// @brief ライト位置を返す
	size_t GetWritePos() const { return m_wpos; }
	/// @brief 読み残した残りを返す
	size_t Remain() const { return m_wpos - m_rpos; }
	/// @brief リード位置をセット
	void SetReadPos(size_t val) { m_rpos = val; }
	/// @brief ライト位置をセット
	void SetWritePos(size_t val) { m_wpos = val; }
	/// @brief すべて読み込んだことにする
	void Fix() { m_rpos = m_wpos; }
};

/// ダンプ用補助クラス
class Dump
{
private:
	::CharCodes codes;

public:
	Dump() {}
	~Dump() {}

	/// @brief バイナリダンプ
	int Binary(const wxUint8 *buffer, size_t bufsize, wxString &str, bool invert);
	/// @brief アスキーダンプ
	int Ascii(const wxUint8 *buffer, size_t bufsize, const wxString &char_code, wxString &str, bool invert);
	/// @brief テキストダンプ
	int Text(const wxUint8 *buffer, size_t bufsize, const wxString &char_code, wxString &str, bool invert);
};

//////////////////////////////////////////////////////////////////////

/// ストップウォッチ
class StopWatch : public wxStopWatch
{
private:
	int  m_id;
	bool m_now_wait_cursor;

public:
	StopWatch();
	void Busy();
	void Restart();
	void Finish();
	int GetID() const { return m_id; }
	void SetID(int id) { m_id = id; }
};

//////////////////////////////////////////////////////////////////////

/// @brief 時間構造体を日時データに変換(MS-DOS)
void	ConvTmToDateTime(const TM &tm, wxUint8 *date, wxUint8 *time);
/// @brief 日時データを構造体に変換(MS-DOS)
void	ConvDateTimeToTm(const wxUint8 *date, const wxUint8 *time, TM &tm);
/// @brief 日付文字列を構造体に変換
bool	ConvDateStrToTm(const wxString &date, TM &tm);
/// @brief 時間文字列を構造体に変換
bool	ConvTimeStrToTm(const wxString &time, TM &tm);
/// @brief BCD形式の日付を変換
void	ConvYYMMDDToTm(wxUint8 yy, wxUint8 mm, wxUint8 dd, TM &tm);
/// @brief BCD形式の日付に変換
void	ConvTmToYYMMDD(const TM &tm, wxUint8 &yy, wxUint8 &mm, wxUint8 &dd);
/// @brief 日付を文字列で返す
wxString FormatYMDStr(const TM &tm);
/// @brief 時分秒を文字列で返す
wxString FormatHMSStr(const TM &tm);
/// @brief 時分を文字列で返す
wxString FormatHMStr(const TM &tm);

/// @brief 文字列をint値に変換
int		ToInt(const wxString &val);
/// @brief 文字列をbool値に変換
bool	ToBool(const wxString &val);

/// @brief 16進文字列を数値に変換
int		ConvFromHexa(const wxString &sval);

/// @brief エスケープ文字を展開
void	 DecodeEscape(const wxString &src, wxString &dst);
/// @brief エスケープ文字を展開
void	 DecodeEscape(const wxString &src, wxUint8 *dst, size_t len);
/// @brief 文字をエスケープ
wxString EncodeEscape(const wxUint8 *src, size_t len);

/// @brief ファイル名を展開
wxString DecodeFileName(const wxString &src);
/// @brief ファイル名をエスケープ
wxString EncodeFileName(const wxString &src);

/// @brief サイド番号を文字列にする
wxString GetSideNumStr(int side_number, bool each_sides);
/// @brief サイド番号を文字列にする
wxString GetSideStr(int side_number, bool each_sides);

/// @brief リストの中に一致する文字列があるか
int		IndexOf(const char *list[], const wxString &substr);

/// @brief 大文字の方が多いか
bool	IsUpperString(const wxString &str);

/// @brief 2のn乗かどうか
bool	IsPowerOfTwo(wxUint32 val, int digit);

/// @brief CRC32を計算する
wxUint32 CRC32(wxUint8 *data, int size);

}; /* namespace Utils */

#endif /* DISKUTILS_H */

