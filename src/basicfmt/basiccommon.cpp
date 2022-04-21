/// @file basiccommon.cpp
///
/// @brief disk basic common functions
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basiccommon.h"


//////////////////////////////////////////////////////////////////////
/// リストの中に一致する文字列があるか
/// @param[in] list リスト(NULL終り)
/// @param[in] str  文字列
/// @return 一致する位置 or -1
int st_name_value::IndexOf(const struct st_name_value *list, const wxString &str)
{
	int match = -1;
	for(int i=0; list[i].name != NULL; i++) {
		if (str == list[i].name) {
			match = i;
			break;
		}
	}
	return match;
}

/// リストの中に一致する値があるか
/// @param[in] list リスト(NULL終り)
/// @param[in] val  値
/// @return 一致する位置 or -1
int st_name_value::IndexOf(const struct st_name_value *list, int val)
{
	int match = -1;
	for(int i=0; list[i].name != NULL; i++) {
		if (val == list[i].value) {
			match = i;
			break;
		}
	}
	return match;
}

//////////////////////////////////////////////////////////////////////
//
// ファイルプロパティでファイル名変更した時に渡す値
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
// 属性保存クラス
//
DiskBasicFileType::DiskBasicFileType()
{
	format = FORMAT_TYPE_UNKNOWN;
	type = 0;
	for(int i=0; i<(int)(sizeof(origin)/sizeof(origin[0])); i++) {
		origin[i] = 0;
	}
}

/// @param [in] n_format   : フォーマット
/// @param [in] n_type     : enum #en_file_type_mask の値の組み合わせ
/// @param [in] n_origin0  : 本来の属性
/// @param [in] n_origin1  : 本来の属性 つづき1
/// @param [in] n_origin2  : 本来の属性 つづき2
DiskBasicFileType::DiskBasicFileType(DiskBasicFormatType n_format, int n_type, int n_origin0, int n_origin1, int n_origin2)
{
	format = n_format;
	type = n_type;
	origin[0] = n_origin0;
	origin[1] = n_origin1;
	origin[2] = n_origin2;
}

DiskBasicFileType::~DiskBasicFileType()
{
}

/// 共通属性が一致するか
bool DiskBasicFileType::MatchType(int mask, int value) const
{
	return ((type & mask) == value);
}

/// 共通属性が一致しないか
bool DiskBasicFileType::UnmatchType(int mask, int value) const
{
	return ((type & mask) != value);
}

/// 共通属性がアスキー属性か
bool DiskBasicFileType::IsAscii() const
{
	return ((type & FILE_TYPE_ASCII_MASK) != 0);
}

/// 共通属性がボリューム属性か
bool DiskBasicFileType::IsVolume() const
{
	return ((type & (FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK)) == FILE_TYPE_VOLUME_MASK);
}

/// 共通属性がディレクトリ属性か
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
	user_data = NULL;
}
DiskBasicGroupItem::DiskBasicGroupItem(const DiskBasicGroupItem &src)
{
	group = src.group;
	next = src.next;
	track = src.track;
	side = src.side;
	sector_start = src.sector_start;
	sector_end = src.sector_end;
	div_num = src.div_num;
	div_nums = src.div_nums;
	user_data = NULL;
	if (src.user_data) {
		user_data = src.user_data->Clone();
	}
}
/// @brief 代入
DiskBasicGroupItem &DiskBasicGroupItem::operator=(const DiskBasicGroupItem &src)
{
	group = src.group;
	next = src.next;
	track = src.track;
	side = src.side;
	sector_start = src.sector_start;
	sector_end = src.sector_end;
	div_num = src.div_num;
	div_nums = src.div_nums;
	user_data = NULL;
	if (src.user_data) {
		user_data = src.user_data->Clone();
	}
	return *this;
}
/// @param[in] n_group グループ番号
/// @param[in] n_next  次のグループ番号（任意）
/// @param[in] n_track トラック番号
/// @param[in] n_side  サイド番号
/// @param[in] n_start グループ内の開始セクタ番号
/// @param[in] n_end   グループ内の終了セクタ番号
/// @param[in] n_div   １グループがセクタ内に複数ある時の分割位置
/// @param[in] n_divs  １グループがセクタ内に複数ある時の分割数
/// @param[in] n_user  機種依存データ
DiskBasicGroupItem::DiskBasicGroupItem(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div, int n_divs, DiskBasicGroupUserData *n_user)
{
	this->Set(n_group, n_next, n_track, n_side, n_start, n_end, n_div, n_divs, n_user);
}
/// @param[in] n_group グループ番号
/// @param[in] n_next  次のグループ番号（任意）
/// @param[in] n_track トラック番号
/// @param[in] n_side  サイド番号
/// @param[in] n_start グループ内の開始セクタ番号
/// @param[in] n_user  機種依存データ
DiskBasicGroupItem::DiskBasicGroupItem(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, DiskBasicGroupUserData *n_user)
{
	this->Set(n_group, n_next, n_track, n_side, n_start, n_user);
}
/// デストラクタ
DiskBasicGroupItem::~DiskBasicGroupItem()
{
	delete user_data;
}
/// データセット
/// @param[in] n_group グループ番号
/// @param[in] n_next  次のグループ番号（任意）
/// @param[in] n_track トラック番号
/// @param[in] n_side  サイド番号
/// @param[in] n_start グループ内の開始セクタ番号
/// @param[in] n_end   グループ内の終了セクタ番号
/// @param[in] n_div   １グループがセクタ内に複数ある時の分割位置
/// @param[in] n_divs  １グループがセクタ内に複数ある時の分割数
/// @param[in] n_user  機種依存データ
void DiskBasicGroupItem::Set(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div, int n_divs, DiskBasicGroupUserData *n_user)
{
	group = n_group;
	next = n_next;
	track = n_track;
	side = n_side;
	sector_start = n_start;
	sector_end = n_end;
	div_num = n_div;
	div_nums = n_divs;
	user_data = n_user;
}
/// データセット
/// @param[in] n_group グループ番号
/// @param[in] n_next  次のグループ番号（任意）
/// @param[in] n_track トラック番号
/// @param[in] n_side  サイド番号
/// @param[in] n_start グループ内の開始セクタ番号
/// @param[in] n_user  機種依存データ
void DiskBasicGroupItem::Set(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, DiskBasicGroupUserData *n_user)
{
	group = n_group;
	next = n_next;
	track = n_track;
	side = n_side;
	sector_start = n_start;
	sector_end = n_start;
	div_num = 0;
	div_nums = 1;
	user_data = n_user;
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
/// @param[in] n_group グループ番号
/// @param[in] n_next  次のグループ番号（任意）
/// @param[in] n_track トラック番号
/// @param[in] n_side  サイド番号
/// @param[in] n_start グループ内の開始セクタ番号
/// @param[in] n_end   グループ内の終了セクタ番号
/// @param[in] n_div   １グループがセクタ内に複数ある時の分割位置
/// @param[in] n_divs  １グループがセクタ内に複数ある時の分割数
/// @param[in] n_user  機種依存データ
void DiskBasicGroups::Add(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div, int n_divs, DiskBasicGroupUserData *n_user)
{
	items.Add(DiskBasicGroupItem(n_group, n_next, n_track, n_side, n_start, n_end, n_div, n_divs, n_user));
}
/// 追加
/// @param[in] n_group グループ番号
/// @param[in] n_next  次のグループ番号（任意）
/// @param[in] n_track トラック番号
/// @param[in] n_side  サイド番号
/// @param[in] n_start グループ内の開始セクタ番号
/// @param[in] n_user  機種依存データ
void DiskBasicGroups::Add(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, DiskBasicGroupUserData *n_user)
{
	items.Add(DiskBasicGroupItem(n_group, n_next, n_track, n_side, n_start, n_user));
}
/// 追加
/// @param[in] n_item アイテム
void DiskBasicGroups::Add(const DiskBasicGroupItem &n_item)
{
	items.Add(n_item);
}
/// 追加
/// @param[in] n_items アイテムリスト
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

//////////////////////////////////////////////////////////////////////
//
//
//
KeyValItem::KeyValItem()
{
	m_value = NULL;
	m_size = 0;
}
KeyValItem::KeyValItem(const wxString &key, int val)
{
	Set(key, val);
}
KeyValItem::KeyValItem(const wxString &key, wxUint8 val, bool invert)
{
	Set(key, val, invert);
}
KeyValItem::KeyValItem(const wxString &key, wxUint16 val, bool big_endien, bool invert)
{
	Set(key, val, big_endien, invert);
}
KeyValItem::KeyValItem(const wxString &key, wxUint32 val, bool big_endien, bool invert)
{
	Set(key, val, big_endien, invert);
}
KeyValItem::KeyValItem(const wxString &key, const void *val, size_t size, bool invert)
{
	Set(key, val, size, invert);
}
KeyValItem::KeyValItem(const wxString &key, bool val)
{
	Set(key, val);
}

KeyValItem::~KeyValItem()
{
	Clear();
}
void KeyValItem::Clear()
{
	delete [] m_value;
	m_size = 0;
}

/// 設定 integer
/// @param[in] key キー名
/// @param[in] val 値
void KeyValItem::Set(const wxString &key, int val)
{
	m_key = key;
	m_value = new wxUint8[sizeof(int)];
	*(int *)m_value = val;
	m_size = sizeof(int);
	m_type = TYPE_INTEGER;
}
/// 設定 8bit
/// @param[in] key    キー名
/// @param[in] val    値
/// @param[in] invert 値を反転するか
void KeyValItem::Set(const wxString &key, wxUint8 val, bool invert)
{
	m_key = key;
	m_value = new wxUint8[sizeof(wxUint8)];
	m_value[0] = val;
	m_size = sizeof(wxUint8);
	m_type = TYPE_UINT8;
	if (invert) mem_invert(m_value, m_size);
}
/// 設定 16bit
/// @param[in] key        キー名
/// @param[in] val        値
/// @param[in] big_endien 値がビッグエンディアンか
/// @param[in] invert     値を反転するか
void KeyValItem::Set(const wxString &key, wxUint16 val, bool big_endien, bool invert)
{
	m_key = key;
	m_value = new wxUint8[sizeof(wxUint16)];
	if (big_endien) {
		m_value[1] = val & 0xff;
		val >>= 8;
		m_value[0] = val & 0xff;
	} else {
		m_value[0] = val & 0xff;
		val >>= 8;
		m_value[1] = val & 0xff;
	}
	m_size = sizeof(wxUint16);
	m_type = TYPE_UINT16;
	if (invert) mem_invert(m_value, m_size);
}
/// 設定 32bit
/// @param[in] key        キー名
/// @param[in] val        値
/// @param[in] big_endien 値がビッグエンディアンか
/// @param[in] invert     値を反転するか
void KeyValItem::Set(const wxString &key, wxUint32 val, bool big_endien, bool invert)
{
	m_key = key;
	m_value = new wxUint8[sizeof(wxUint32)];
	if (big_endien) {
		m_value[3] = val & 0xff;
		val >>= 8;
		m_value[2] = val & 0xff;
		val >>= 8;
		m_value[1] = val & 0xff;
		val >>= 8;
		m_value[0] = val & 0xff;
	} else {
		m_value[0] = val & 0xff;
		val >>= 8;
		m_value[1] = val & 0xff;
		val >>= 8;
		m_value[2] = val & 0xff;
		val >>= 8;
		m_value[3] = val & 0xff;
	}
	m_size = sizeof(wxUint32);
	m_type = TYPE_UINT32;
	if (invert) mem_invert(m_value, m_size);
}
/// 設定 byte array
/// @param[in] key    キー名
/// @param[in] val    バイト配列
/// @param[in] size   配列サイズ
/// @param[in] invert 値を反転するか
void KeyValItem::Set(const wxString &key, const void *val, size_t size, bool invert)
{
	m_key = key;
	m_value = new wxUint8[size + 1];
	memcpy(m_value, val, size);
	m_value[size] = 0;
	m_size = size;
	m_type = TYPE_STRING;
	if (invert) mem_invert(m_value, m_size);
}
/// 設定 bool
/// @param[in] key キー名
/// @param[in] val 値
void KeyValItem::Set(const wxString &key, bool val)
{
	m_key = key;
	m_value = new wxUint8[1];
	m_value[0] = val ? 1 : 0;
	m_size = 1;
	m_type = TYPE_BOOL;
}
/// 値を文字列にして返す
wxString KeyValItem::GetValueString() const
{
	wxString val;

	if (!m_value) return val;

	switch(m_type) {
	case TYPE_INTEGER:
		val = wxString::Format(wxT("%d"), *(int *)m_value);
		break;
	case TYPE_UINT8:
		val = wxString::Format(wxT("0x%02x"), *(wxUint8 *)m_value);
		break;
	case TYPE_UINT16:
		val = wxString::Format(wxT("0x%04x"), *(wxUint16 *)m_value);
		break;
	case TYPE_UINT32:
		val = wxString::Format(wxT("0x%08x"), *(wxUint32 *)m_value);
		break;
	case TYPE_STRING:
		for(size_t i=0; i<m_size; i++) {
			if (i > 0) val += wxT(" ");
			val += wxString::Format(wxT("%02x"), m_value[i]);
		}
		break;
	case TYPE_BOOL:
		val = m_value[0] ? wxT("true") : wxT("false");
		break;
	default:
		break;
	}
	return val;
}
/// キー名の比較
int KeyValItem::Compare(KeyValItem **item1, KeyValItem **item2)
{
	return (*item1)->m_key.Cmp((*item2)->m_key);
}

//

KeyValArray::KeyValArray()
	: ArrayOfKeyValItem()
{
}
KeyValArray::~KeyValArray()
{
	this->DeleteAll();
}
/// リスト内アイテムをdelete
void KeyValArray::DeleteAll()
{
	for(size_t i=0; i<Count(); i++) {
		delete Item(i);
	}
}
/// リストをクリア
void KeyValArray::Clear()
{
	this->DeleteAll();
	ArrayOfKeyValItem::Clear();
}
/// リストをクリア
void KeyValArray::Empty()
{
	this->DeleteAll();
	ArrayOfKeyValItem::Empty();
}
/// 追加 integer
/// @param[in] key キー名
/// @param[in] val 値
void KeyValArray::Add(const wxString &key, int val)
{
	ArrayOfKeyValItem::Add(new KeyValItem(key, val));
}
/// 追加 8bit
/// @param[in] key    キー名
/// @param[in] val    値
/// @param[in] invert 値を反転するか
void KeyValArray::Add(const wxString &key, wxUint8 val, bool invert)
{
	ArrayOfKeyValItem::Add(new KeyValItem(key, val, invert));
}
/// 追加 16bit
/// @param[in] key        キー名
/// @param[in] val        値
/// @param[in] big_endien 値がビッグエンディアンか
/// @param[in] invert     値を反転するか
void KeyValArray::Add(const wxString &key, wxUint16 val, bool big_endien, bool invert)
{
	ArrayOfKeyValItem::Add(new KeyValItem(key, val, big_endien, invert));
}
/// 追加 32bit
/// @param[in] key        キー名
/// @param[in] val        値
/// @param[in] big_endien 値がビッグエンディアンか
/// @param[in] invert     値を反転するか
void KeyValArray::Add(const wxString &key, wxUint32 val, bool big_endien, bool invert)
{
	ArrayOfKeyValItem::Add(new KeyValItem(key, val, big_endien, invert));
}
/// 追加 byte array
/// @param[in] key    キー名
/// @param[in] val    バイト配列
/// @param[in] size   配列サイズ
/// @param[in] invert 値を反転するか
void KeyValArray::Add(const wxString &key, const void *val, size_t size, bool invert)
{
	ArrayOfKeyValItem::Add(new KeyValItem(key, val, size, invert));
}
/// 追加 bool
/// @param[in] key キー名
/// @param[in] val 値
void KeyValArray::Add(const wxString &key, bool val)
{
	ArrayOfKeyValItem::Add(new KeyValItem(key, val));
}
