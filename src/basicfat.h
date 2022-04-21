/// @file basicfat.h
///
/// @brief disk basic fat
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICFAT_H_
#define _BASICFAT_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"
#include "diskd88.h"


//////////////////////////////////////////////////////////////////////

/// 使用状況テーブル
class DiskBasicAvailabillity : public wxArrayInt
{
public:
	DiskBasicAvailabillity();
};

//////////////////////////////////////////////////////////////////////

/// FATエリア（セクタ）へのポインタを保持
class DiskBasicFatBuffer
{
private:
	size_t   size;
	wxUint8 *buffer;

public:
	DiskBasicFatBuffer();
	DiskBasicFatBuffer(wxUint8 *newbuf, int newsize);
	~DiskBasicFatBuffer() {}
	wxUint8 *GetBuffer() const { return buffer; }
	size_t   GetSize() const { return size; }
	void	 Fill(wxUint8 code);
	void	 Copy(const wxUint8 *buf, size_t len);
	wxUint32 Get(size_t pos) const;
	void	 Set(size_t pos, wxUint32 val);
	bool	 Bit(wxUint32 pos, wxUint8 mask, bool val, bool invert);
	bool	 BitTest(wxUint32 pos, wxUint8 mask, bool invert);
	wxUint32 Get16LE(size_t pos) const;
	void	 Set16LE(size_t pos, wxUint32 val);
	wxUint32 Get16BE(size_t pos) const;
	void	 Set16BE(size_t pos, wxUint32 val);
};

WX_DECLARE_OBJARRAY(DiskBasicFatBuffer, ArrayDiskBasicFatBuffer);

//////////////////////////////////////////////////////////////////////

/// FAT１つ分のバッファ（複数セクタあり）
class DiskBasicFatBuffers : public ArrayDiskBasicFatBuffer
{
public:
	wxUint32 GetData8(wxUint32 pos) const;
	void     SetData8(wxUint32 pos, wxUint32 val);
	bool     MatchData8(wxUint32 pos, wxUint32 val) const;

	bool     BitData8(wxUint32 pos, wxUint8 mask, bool val, bool invert);

	wxUint32 GetData12LE(wxUint32 pos) const;
	void     SetData12LE(wxUint32 pos, wxUint32 val);

	wxUint32 GetData16LE(wxUint32 pos) const;
	void     SetData16LE(wxUint32 pos, wxUint32 val);

	wxUint32 GetData16BE(wxUint32 pos) const;
	void     SetData16BE(wxUint32 pos, wxUint32 val);
};

/// FATの個数分（多重）
WX_DECLARE_OBJARRAY(DiskBasicFatBuffers, ArrayArrayDiskBasicFatBuffer);

//////////////////////////////////////////////////////////////////////

/// FATバッファ
class DiskBasicFatArea : public ArrayArrayDiskBasicFatBuffer
{
private:
	size_t	valid_count;

public:
	DiskBasicFatArea();
	DiskBasicFatArea(const DiskBasicFatArea &src);
	DiskBasicFatArea &operator=(const DiskBasicFatArea &src);

	void	Empty();
	void	Add(const DiskBasicFatBuffers &lItem, size_t nInsert = 1);

	void	SetValidCount(size_t val) { valid_count = val; }
	size_t	GetValidCount() const { return valid_count; }

	wxUint32 GetData8(size_t idx, wxUint32 pos) const;
	void     SetData8(wxUint32 pos, wxUint32 val);
	void     SetData8(size_t idx, wxUint32 pos, wxUint32 val);
	int      MatchData8(wxUint32 pos, wxUint32 val) const;
	bool     MatchData8(size_t idx, wxUint32 pos, wxUint32 val) const;

	void     BitData8(size_t idx, wxUint32 pos, wxUint8 mask, bool val, bool invert);

	wxUint32 GetData12LE(size_t idx, wxUint32 pos) const;
	void     SetData12LE(wxUint32 pos, wxUint32 val);
	void     SetData12LE(size_t idx, wxUint32 pos, wxUint32 val);

	wxUint32 GetData16LE(size_t idx, wxUint32 pos) const;
	void     SetData16LE(wxUint32 pos, wxUint32 val);
	void     SetData16LE(size_t idx, wxUint32 pos, wxUint32 val);

	wxUint32 GetData16BE(size_t idx, wxUint32 pos) const;
	void     SetData16BE(wxUint32 pos, wxUint32 val);
	void     SetData16BE(size_t idx, wxUint32 pos, wxUint32 val);
};

class DiskBasic;
class DiskBasicType;

//////////////////////////////////////////////////////////////////////

/// FATアクセス
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

	/// FAT領域をアサイン
	double Assign(bool is_formatting);
	/// FAT領域をクリア
	void Clear();
	/// FAT領域をクリア
	void Empty();

	/// FAT領域の最初のセクタの指定位置のデータを取得
	wxUint8 Get(int pos) const;
	/// FAT領域の最初のセクタの指定位置にデータを書く
	void Set(int pos, wxUint8 code);
	/// FAT領域の最初のセクタにデータを書く
	void Copy(const wxUint8 *buf, size_t len);
	/// FAT領域を指定データで埋める
	void Fill(wxUint8 code);

	DiskBasicFatArea	 *GetDiskBasicFatArea() { return &bufs; }
	DiskBasicFatBuffers	 *GetDiskBasicFatBuffers(size_t idx);
	DiskBasicFatBuffer	 *GetDiskBasicFatBuffer(size_t idx, size_t subidx);
};

#endif /* _BASICFAT_H_ */
