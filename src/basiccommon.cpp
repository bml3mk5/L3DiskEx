/// @file basiccommon.cpp
///
/// @brief disk basic common functions
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basiccommon.h"


//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicFileName::DiskBasicFileName()
{
	optional = 0;
}
DiskBasicFileName::DiskBasicFileName(const wxString &n_name, int n_optional)
{
	name = n_name;
	optional = n_optional;
}
DiskBasicFileName::~DiskBasicFileName()
{
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicFileType::DiskBasicFileType()
{
	format = FORMAT_TYPE_UNKNOWN;
	type = 0;
	origin = 0;
}

/// @param [in] n_format   : フォーマット
/// @param [in] n_type     : enum #en_file_type_mask の値の組み合わせ
/// @param [in] n_origin   : 本来の属性
DiskBasicFileType::DiskBasicFileType(DiskBasicFormatType n_format, int n_type, int n_origin)
{
	format = n_format;
	type = n_type;
	origin = n_origin;
}

DiskBasicFileType::~DiskBasicFileType()
{
}

bool DiskBasicFileType::MatchType(int mask, int value) const
{
	return ((type & mask) == value);
}

bool DiskBasicFileType::UnmatchType(int mask, int value) const
{
	return ((type & mask) != value);
}

bool DiskBasicFileType::IsAscii() const
{
	return ((type & FILE_TYPE_ASCII_MASK) != 0);
}

bool DiskBasicFileType::IsVolume() const
{
	return ((type & (FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK)) == FILE_TYPE_VOLUME_MASK);
}

bool DiskBasicFileType::IsDirectory() const
{
	return ((type & (FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK)) == FILE_TYPE_DIRECTORY_MASK);
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicGroupItem::DiskBasicGroupItem()
{
	group = 0;
	next = 0;
	track = 0;
	side = 0;
	sector_start = 0;
	sector_end = 0;
	div_num = 0;
	div_nums = 1;
}
DiskBasicGroupItem::DiskBasicGroupItem(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div, int n_divs)
{
	this->Set(n_group, n_next, n_track, n_side, n_start, n_end, n_div, n_divs);
}
void DiskBasicGroupItem::Set(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div, int n_divs)
{
	group = n_group;
	next = n_next;
	track = n_track;
	side = n_side;
	sector_start = n_start;
	sector_end = n_end;
	div_num = n_div;
	div_nums = n_divs;
}
/// グループ番号でソートする際の比較
int DiskBasicGroupItem::Compare(DiskBasicGroupItem **item1, DiskBasicGroupItem **item2)
{
	return ((*item1)->group == (*item2)->group ? 0 : ((*item1)->group > (*item2)->group ? 1 : -1));
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicGroupItems);

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicGroups::DiskBasicGroups()
{
	nums = 0;
	size = 0;
	size_per_group = 0;
}
/// 追加
void DiskBasicGroups::Add(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div, int n_divs)
{
	items.Add(DiskBasicGroupItem(n_group, n_next, n_track, n_side, n_start, n_end, n_div, n_divs));
}
/// 追加
void DiskBasicGroups::Add(const DiskBasicGroupItem &item)
{
	items.Add(item);
}
/// 追加
void DiskBasicGroups::Add(const DiskBasicGroups &n_items)
{
	for(size_t i=0; i<n_items.Count(); i++) {
		items.Add(n_items.Item(i));
	}
	nums += n_items.nums;
	size += n_items.size;
}
/// クリア
void DiskBasicGroups::Empty()
{
	items.Empty();
	nums = 0;
	size = 0;
	size_per_group = 0;
}
/// アイテム数
size_t DiskBasicGroups::Count() const
{
	return items.Count();
}
/// 最終アイテム
DiskBasicGroupItem &DiskBasicGroups::Last() const
{
	return items.Last();
}
/// アイテム
DiskBasicGroupItem &DiskBasicGroups::Item(size_t idx) const
{
	return items.Item(idx);
}
/// アイテム
DiskBasicGroupItem *DiskBasicGroups::ItemPtr(size_t idx) const
{
	return &items.Item(idx);
}
/// グループ数を足す
int DiskBasicGroups::AddNums(int val)
{
	nums += val;
	return nums;
}

/// 占有サイズを足す
int DiskBasicGroups::AddSize(int val)
{
	size += val;
	return (int)size;
}

/// グループ番号でソート
void DiskBasicGroups::SortItems()
{
	items.Sort(&DiskBasicGroupItem::Compare);
}
