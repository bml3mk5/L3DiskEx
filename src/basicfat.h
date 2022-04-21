/// @file basicfat.h
///
/// @brief disk basic fat
///
#ifndef _BASICFAT_H_
#define _BASICFAT_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"
#include "diskd88.h"

class DiskBasicGroupItem;

/// グループ番号に対応するパラメータを保持
class DiskBasicGroupItem
{
public:
	wxUint32 group;
	wxUint32 next;
	int track;
	int side;
	int sector_start;
	int sector_end;
public:
	DiskBasicGroupItem();
	DiskBasicGroupItem(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end);
	~DiskBasicGroupItem() {}
	void Set(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end);
	static int Compare(DiskBasicGroupItem **item1, DiskBasicGroupItem **item2);
};

WX_DECLARE_OBJARRAY(DiskBasicGroupItem, DiskBasicGroupItems);

/// グループ番号のリストを保持
class DiskBasicGroups
{
private:
	DiskBasicGroupItems	items;	///< グループ番号のリスト
	size_t				size;	///< グループ内の占有サイズ

public:
	DiskBasicGroups();
	~DiskBasicGroups() {}

	void Add(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end);
	void Add(const DiskBasicGroupItem &item);
	void Empty();
	size_t Count() const;
	DiskBasicGroupItem &Last() const;
	DiskBasicGroupItem &Item(size_t idx) const;
	DiskBasicGroupItem *ItemPtr(size_t idx) const;

	const DiskBasicGroupItems &GetItems() const { return items; }
	void SetSize(size_t val) { size = val; }
	size_t GetSize() const { return size; }
	/// グループ番号でソート
	void SortItems();
};

/// FATエリアへのポインタを保持
class DiskBasicFatBuffer
{
public:
	int      size;
	wxUint8 *buffer;

public:
	DiskBasicFatBuffer();
	DiskBasicFatBuffer(wxUint8 *newbuf, int newsize);
	~DiskBasicFatBuffer() {}
	void Fill(wxUint8 code);
	void Copy(const wxUint8 *buf, size_t len);
};

/// FAT１つ分のバッファ
WX_DECLARE_OBJARRAY(DiskBasicFatBuffer, DiskBasicFatBuffers);
/// FATの個数分
WX_DECLARE_OBJARRAY(DiskBasicFatBuffers, DiskBasicFatArea);

class DiskBasic;
class DiskBasicType;

/// FATアクセス
class DiskBasicFat
{
private:
	DiskBasic			*basic;
	DiskBasicType		*type;
	int count;			///< FATの数
	int size;			///< FATサイズ(セクタ数)
	int start;			///< 開始セクタ番号
	int start_pos;		///< 開始位置

	DiskBasicFatArea bufs;

	DiskBasicFat();

public:
	DiskBasicFat(DiskBasic *basic);
	~DiskBasicFat();

	bool Assign();
	void Clear();
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
