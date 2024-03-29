/// @file basicfat.h
///
/// @brief disk basic fat
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICFAT_H
#define BASICFAT_H

#include "../common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"


//////////////////////////////////////////////////////////////////////

/// @brief 使用状況テーブル enum
enum en_fat_availability {
	FAT_AVAIL_FREE = 0,
	FAT_AVAIL_SYSTEM,
	FAT_AVAIL_USED,
	FAT_AVAIL_USED_FIRST,
	FAT_AVAIL_USED_LAST,
	FAT_AVAIL_MISSING,
	FAT_AVAIL_LEAK,
	FAT_AVAIL_NULLEND
};

/// @brief 使用状況テーブル
class DiskBasicAvailabillity : public wxArrayInt
{
private:
	int m_free_size;	///< 空きサイズ
	int m_free_grps;	///< 空きグループ数

public:
	DiskBasicAvailabillity();
	/// @brief 初期化 空きサイズを 0 にする
	void Clear();
	/// @brief 初期化 空きサイズを 0 にする
	void Empty();
	/// @brief 初期化 空きサイズを -1 にする
	void EmptyInit();
	/// @brief 追加
	void Add(int val, int size, int group);
	/// @brief セット (Safety)
	void Set(size_t idx, int val);
	/// @brief ゲット (Safety)
	int  Get(size_t idx) const;
	/// @brief 空きサイズを返す
	int GetFreeSize() const { return m_free_size; }
	/// @brief 空きグループ数を返す
	int GetFreeGroups() const { return m_free_grps; }
	/// @brief 空きサイズをセット
	void SetFreeSize(int val) { m_free_size = val; }
	/// @brief 空きグループ数をセット
	void SetFreeGroups(int val) { m_free_grps = val; }
};

//////////////////////////////////////////////////////////////////////

/// @brief ビット ON/OFF バッファ １つ
///
/// @sa DiskBasicBitMLMap
class BitMLBuffer
{
protected:
	wxUint8 *m_buffer;
	size_t	 m_size;
public:
	BitMLBuffer();
	BitMLBuffer(wxUint8 *buffer, size_t size);
	virtual ~BitMLBuffer() {}

	/// @brief 指定位置のビットを変更する
	virtual void Modify(wxUint32 num, bool val);
	/// @brief 指定位置のビットがセットされているか
	virtual bool IsSet(wxUint32 num) const;
	/// @brief 指定位置のビット位置を計算
	virtual void GetPos(wxUint32 num, wxUint32 &pos, wxUint32 &bit) const;
	/// @brief バッファを返す
	wxUint8 *GetBuffer() { return m_buffer; }
	/// @brief サイズ(ビット数)を返す
	size_t GetBitSize() const { return m_size << 3; }
	/// @brief バッファサイズを返す
	size_t GetSize() const { return m_size; }
};

WX_DECLARE_OBJARRAY(BitMLBuffer, ArrayOfBitMLBuffer);

/// @brief ビット ON/OFF マップ BitMLBuffer の配列
class DiskBasicBitMLMap : public ArrayOfBitMLBuffer
{
protected:
	DiskBasicBitMLMap(const DiskBasicBitMLMap &src) {}

public:
	DiskBasicBitMLMap();
	virtual ~DiskBasicBitMLMap() {}

	/// @brief ポインタをセット
	void AddBuffer(wxUint8 *buffer, size_t size);

	/// @brief 指定位置のビットを変更する
	virtual void Modify(wxUint32 group_num, bool val);
	/// @brief 指定位置が空いているか
	virtual bool IsSet(wxUint32 group_num) const;
	/// @brief 指定位置からバッファ内の位置を計算
	bool GetPosInMap(wxUint32 group_num, size_t &idx, wxUint32 &pos, wxUint32 &bit) const;
};

//////////////////////////////////////////////////////////////////////

/// @brief FATエリア（セクタ）へのポインタを保持
class DiskBasicFatBuffer
{
private:
	size_t   size;		///< バッファサイズ
	wxUint8 *buffer;	///< バッファポインタ（セクタ内の開始ポインタ）

public:
	DiskBasicFatBuffer();
	DiskBasicFatBuffer(wxUint8 *newbuf, int newsize);
	~DiskBasicFatBuffer() {}
	/// @brief バッファポインタを返す
	wxUint8 *GetBuffer() const { return buffer; }
	/// @brief バッファサイズを返す
	size_t   GetSize() const { return size; }
	/// @brief バッファを指定コードで埋める
	void	 Fill(wxUint8 code);
	/// @brief バッファにコピー
	void	 Copy(const wxUint8 *buf, size_t len);
	/// @brief 指定位置のデータを返す(8ビット)
	wxUint32 Get(size_t pos) const;
	/// @brief 指定位置にデータをセット(8ビット)
	void	 Set(size_t pos, wxUint32 val);
	/// @brief 指定位置のビットをセット/リセット
	bool	 Bit(wxUint32 pos, wxUint8 mask, bool val, bool invert);
	/// @brief 指定位置のビットがONか
	bool	 BitTest(wxUint32 pos, wxUint8 mask, bool invert);
	/// @brief 指定位置のデータを返す(16ビット、リトルエンディアン)
	wxUint32 Get16LE(size_t pos) const;
	/// @brief 指定位置にデータをセット(16ビット、リトルエンディアン)
	void	 Set16LE(size_t pos, wxUint32 val);
	/// @brief 指定位置のデータを返す(16ビット、ビッグエンディアン)
	wxUint32 Get16BE(size_t pos) const;
	/// @brief 指定位置にデータをセット(16ビット、ビッグエンディアン)
	void	 Set16BE(size_t pos, wxUint32 val);
};

//////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(DiskBasicFatBuffer, ArrayDiskBasicFatBuffer);

//////////////////////////////////////////////////////////////////////

/// @brief FAT１つ分のバッファ（複数セクタあり） DiskBasicFatBuffer の配列
class DiskBasicFatBuffers : public ArrayDiskBasicFatBuffer
{
public:
	/// @brief 8ビットデータを返す
	wxUint32 GetData8(wxUint32 pos) const;
	/// @brief 8ビットデータをセット
	void     SetData8(wxUint32 pos, wxUint32 val);
	/// @brief 8ビットデータが一致するか
	bool     MatchData8(wxUint32 pos, wxUint32 val) const;
	/// @brief 8ビットデータのビットをセット/リセット
	bool     BitData8(wxUint32 pos, wxUint8 mask, bool val, bool invert);

	/// @brief 12ビットデータ(リトルエンディアン)を返す
	wxUint32 GetData12LE(wxUint32 pos) const;
	/// @brief 12ビットデータ(リトルエンディアン)をセット
	void     SetData12LE(wxUint32 pos, wxUint32 val);

	/// @brief 16ビットデータ(リトルエンディアン)を返す
	wxUint32 GetData16LE(wxUint32 pos) const;
	/// @brief 16ビットデータ(リトルエンディアン)をセット
	void     SetData16LE(wxUint32 pos, wxUint32 val);

	/// @brief 16ビットデータ(ビッグエンディアン)を返す
	wxUint32 GetData16BE(wxUint32 pos) const;
	/// @brief 16ビットデータ(ビッグエンディアン)をセット
	void     SetData16BE(wxUint32 pos, wxUint32 val);
};

//////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(DiskBasicFatBuffers, ArrayArrayDiskBasicFatBuffer);

//////////////////////////////////////////////////////////////////////

/// @brief FATバッファ（ミラーリング含む） DiskBasicFatBuffers の配列
class DiskBasicFatArea : public ArrayArrayDiskBasicFatBuffer
{
private:
	size_t	valid_count;	///< 有効なバッファ数

public:
	DiskBasicFatArea();
	/// @brief コピーコンストラクタ
	DiskBasicFatArea(const DiskBasicFatArea &src);
	/// @brief 代入
	DiskBasicFatArea &operator=(const DiskBasicFatArea &src);
	/// @brief クリア
	void	Empty();
	/// @brief 追加
	void	Add(const DiskBasicFatBuffers &lItem, size_t nInsert = 1);
	/// @brief 有効なバッファ数をセット
	void	SetValidCount(size_t val) { valid_count = val; }
	/// @brief 有効なバッファ数を返す
	size_t	GetValidCount() const { return valid_count; }

	/// @brief 8ビットデータを返す
	wxUint32 GetData8(size_t idx, wxUint32 pos) const;
	/// @brief 8ビットデータをセット
	void     SetData8(wxUint32 pos, wxUint32 val);
	/// @brief 8ビットデータをセット
	void     SetData8(size_t idx, wxUint32 pos, wxUint32 val);
	/// @brief 8ビットデータが一致するか
	int      MatchData8(wxUint32 pos, wxUint32 val) const;
	/// @brief 8ビットデータが一致するか
	bool     MatchData8(size_t idx, wxUint32 pos, wxUint32 val) const;

	/// @brief 8ビットデータのビットをセット/リセット
	void     BitData8(size_t idx, wxUint32 pos, wxUint8 mask, bool val, bool invert);

	/// @brief 12ビットデータ(リトルエンディアン)を返す
	wxUint32 GetData12LE(size_t idx, wxUint32 pos) const;
	/// @brief 12ビットデータ(リトルエンディアン)をセット
	void     SetData12LE(wxUint32 pos, wxUint32 val);
	/// @brief 12ビットデータ(リトルエンディアン)をセット
	void     SetData12LE(size_t idx, wxUint32 pos, wxUint32 val);

	/// @brief 16ビットデータ(リトルエンディアン)を返す
	wxUint32 GetData16LE(size_t idx, wxUint32 pos) const;
	/// @brief 16ビットデータ(リトルエンディアン)をセット
	void     SetData16LE(wxUint32 pos, wxUint32 val);
	/// @brief 16ビットデータ(リトルエンディアン)をセット
	void     SetData16LE(size_t idx, wxUint32 pos, wxUint32 val);

	/// @brief 16ビットデータ(ビッグエンディアン)を返す
	wxUint32 GetData16BE(size_t idx, wxUint32 pos) const;
	/// @brief 16ビットデータ(ビッグエンディアン)をセット
	void     SetData16BE(wxUint32 pos, wxUint32 val);
	/// @brief 16ビットデータ(ビッグエンディアン)をセット
	void     SetData16BE(size_t idx, wxUint32 pos, wxUint32 val);
};

class DiskBasic;
class DiskBasicType;

//////////////////////////////////////////////////////////////////////

/// @brief FATアクセス
class DiskBasicFat
{
private:
	DiskBasic			*basic;
	DiskBasicType		*type;
	int count;			///< FATの数
	int vcount;			///< 使用しているFATの数
	int size;			///< FATサイズ(セクタ数)
	int start;			///< 開始セクタ番号
	int start_pos;		///< 開始位置

	DiskBasicFatArea bufs;

	DiskBasicFat();

public:
	DiskBasicFat(DiskBasic *basic);
	~DiskBasicFat();

	/// @brief FAT領域をアサイン
	double Assign(bool is_formatting);
	/// @brief FAT領域をクリア
	void Clear();
	/// @brief FAT領域をクリア
	void Empty();

	/// @brief FAT領域の最初のセクタの指定位置のデータを取得
	wxUint8 Get(int pos) const;
	/// @brief FAT領域の最初のセクタの指定位置にデータを書く
	void Set(int pos, wxUint8 code);
	/// @brief FAT領域の最初のセクタにデータを書く
	void Copy(const wxUint8 *buf, size_t len);
	/// @brief FAT領域を指定コードで埋める
	void Fill(wxUint8 code);

	/// @brief FAT領域を返す
	DiskBasicFatArea	 *GetDiskBasicFatArea() { return &bufs; }
	/// @brief FATバッファを返す
	DiskBasicFatBuffers	 *GetDiskBasicFatBuffers(size_t idx);
	/// @brief FATバッファ（セクタ）を返す
	DiskBasicFatBuffer	 *GetDiskBasicFatBuffer(size_t idx, size_t subidx);
};

#endif /* BASICFAT_H */
