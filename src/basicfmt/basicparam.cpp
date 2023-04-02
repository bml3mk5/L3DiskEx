/// @file basicparam.cpp
///
/// @brief disk basic parameter
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicparam.h"
#include "../utils.h"
#include <wx/xml/xml.h>


DiskBasicTemplates gDiskBasicTemplates;

//////////////////////////////////////////////////////////////////////
//
// 特別な属性などを保持する
//
L3Attribute::L3Attribute()
{
	idx = 0;
	type = 0;
	value = 0;
	mask = -1;
}
L3Attribute::L3Attribute(int n_idx, int n_type, int n_value, int n_mask, const wxString &n_name, const wxString &n_desc)
{
	idx = n_idx;
	type = n_type;
	value = n_value;
	mask = n_mask;
	name = n_name;
	desc = n_desc;

	if (name.IsEmpty()) name = wxT("???");
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfL3Attribute);

//////////////////////////////////////////////////////////////////////
//
// 特別な属性のリスト L3Attribute の配列
//
L3Attributes::L3Attributes()
	: ArrayOfL3Attribute()
{
}
/// 属性タイプと値に一致するアイテムを返す
const L3Attribute *L3Attributes::Find(int type, int value) const
{
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if (attr->GetType() == type && attr->GetValue() == (value & attr->GetMask())) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性タイプと値に一致するアイテムを返す
const L3Attribute *L3Attributes::Find(int type, int mask, int value) const
{
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if ((attr->GetType() & mask) == (type & mask) && attr->GetValue() == (value & attr->GetMask())) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性タイプと値に一致するアイテムを返す
const L3Attribute *L3Attributes::FindType(int type, int mask) const
{
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if ((attr->GetType() & mask) == (type & mask)) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性値に一致するアイテムを返す
const L3Attribute *L3Attributes::FindValue(int value) const
{
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if (attr->GetValue() == (value & attr->GetMask())) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名に一致するアイテムを返す
const L3Attribute *L3Attributes::Find(int type, const wxString &name) const
{
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if (attr->GetType() == type && attr->GetName() == name) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名に一致するアイテムを返す
const L3Attribute *L3Attributes::Find(const wxString &name) const
{
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if (attr->GetName() == name) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名に一致するアイテムを返す 大文字でマッチング
const L3Attribute *L3Attributes::FindUpperCase(const wxString &name) const
{
	wxString iname = name.Upper();
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if (attr->GetName().Upper() == iname) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名と属性タイプに一致するアイテムを返す 大文字でマッチング
const L3Attribute *L3Attributes::FindUpperCase(const wxString &name, int type, int mask) const
{
	wxString iname = name.Upper();
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if (attr->GetName().Upper() == iname && (attr->GetType() & mask) == (type & mask)) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性名、属性タイプ、属性値に一致するアイテムを返す 大文字でマッチング
const L3Attribute *L3Attributes::FindUpperCase(const wxString &name, int type, int mask, int value) const
{
	wxString iname = name.Upper();
	const L3Attribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		L3Attribute *attr = &Item(i);
		if (attr->GetName().Upper() == iname && (attr->GetType() & mask) == (type & mask) && attr->GetValue() == value) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性値に一致するアイテムの位置を返す
int L3Attributes::GetIndexByValue(int value) const
{
	int idx = -1;
	const L3Attribute *match = FindValue(value);
	if (match) {
		idx = match->GetIndex();
	}
	return idx;
}
/// 属性値に一致するアイテムの属性タイプを返す
int L3Attributes::GetTypeByValue(int value) const
{
	int type = 0;
	const L3Attribute *match = FindValue(value);
	if (match) {
		type = match->GetType();
	}
	return type;
}
/// 位置から属性タイプを返す
int L3Attributes::GetTypeByIndex(int idx) const
{
	return idx < (int)Count() ? Item(idx).GetType() : 0;
}
/// 位置から属性値を返す
int L3Attributes::GetValueByIndex(int idx) const
{
	return idx < (int)Count() ? Item(idx).GetValue() : 0;
}

//////////////////////////////////////////////////////////////////////
//
// ファイル名の命名規則を保持（バリデータ用）
//
ValidNameRule::ValidNameRule()
{
	name_require = false;
	max_length = 0;
}

/// 空にする
void ValidNameRule::Empty()
{
	valid_first_chars.Empty();
	valid_chars.Empty();
	invalid_chars.Empty();
	deduplicate_chars.Empty();
	name_require = false;
	max_length = 0;
}

//////////////////////////////////////////////////////////////////////
//
// DISK BASICのフォーマットタイプ
//
DiskBasicFormat::DiskBasicFormat()
{
	type_number			 = FORMAT_TYPE_UNKNOWN;
	has_volume_name		 = false;
	has_volume_number	 = false;
	has_volume_date		 = false;
	sectors_per_group	 = 0;
	group_final_code	 = 0;
	group_system_code	 = 0;
	group_unused_code	 = 0;
	dir_terminate_code	 = 0x20;
	dir_space_code		 = 0x20;
	dir_trimming_code	 = 0;
	dir_start_pos		 = 0;
	dir_start_pos_on_root = 0;
	dir_start_pos_on_sec = 0;
	dir_start_pos_on_group = 0;
	fillcode_on_format	 = 0;
	fillcode_on_fat		 = 0;
	fillcode_on_dir		 = 0;
	delete_code			 = 0;
	text_terminate_code	 = 0x1a;
	extension_pre_code	 = 0x2e; // '.'
	compare_case_insense = false;
	to_upper_before_dialog = false;
	to_upper_after_renamed = false;
	big_endian			 = false;
	various_params.clear();
}
/// 固有のパラメータ
void DiskBasicFormat::SetVariousParam(const wxString &key, const wxVariant &val)
{
	various_params[key] = val;
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicFormats);

//////////////////////////////////////////////////////////////////////
//
// DISK BASICのパラメータを保持するクラス
//
DiskBasicParam::DiskBasicParam()
{
	this->ClearBasicParam();
}
DiskBasicParam::DiskBasicParam(const DiskBasicParam &src)
{
	this->SetBasicParam(src);
}
/// 初期化
void DiskBasicParam::ClearBasicParam()
{
	basic_type_name.Empty();
	basic_category_name.Empty();
	format_type			 = NULL;
	format_subtype_number = 0;
	sectors_per_group	 = 0;
	sides_on_basic		 = 0;
	sectors_on_basic	 = -1;
	sectors_on_basic_list.Empty();
	sector_number_base	 = -1;
	tracks_on_basic		 = -1;
	managed_track_number = 0;
	groups_per_track	 = 0;
	groups_per_sector	 = 1;
	reserved_sectors	 = 0;
	number_of_fats		 = 1;
	valid_number_of_fats = -1;
	sectors_per_fat		 = 0;
	fat_start_pos		 = 0;
	fat_end_group		 = 0;
	fat_side_number		 = -1;
	reserved_groups.Empty();
	group_final_code	 = 0;
	group_system_code	 = 0;
	group_unused_code	 = 0;
	dir_start_sector	 = -1;
	dir_end_sector		 = -1;
	dir_entry_count		 = -1;
	subdir_group_size	 = 1;
	dir_terminate_code	 = 0x20;
	dir_space_code		 = 0x20;
	dir_trimming_code	 = 0;
	dir_start_pos		 = 0;
	dir_start_pos_on_root = 0;
	dir_start_pos_on_sec = 0;
	dir_start_pos_on_group = 0;
	group_width			 = 1;
	groups_per_dir_entry = 0;
	valid_density_type   = -1;
	sector_skew.Set(1);
	special_attrs.Empty();
	attrs_by_extension.Empty();
	fillcode_on_format	 = 0;
	fillcode_on_fat		 = 0;
	fillcode_on_dir		 = 0;
	delete_code			 = 0;
	media_id			 = 0x00;
	text_terminate_code	 = 0x1a;
	extension_pre_code	 = 0x2e; // '.'
	valid_file_name.Empty();
	valid_volume_name.Empty();
	compare_case_insense = false;
	to_upper_before_dialog = false;
	to_upper_after_renamed = false;
	data_inverted		 = false;
	side_reversed		 = false;
	big_endian			 = false;
	mount_each_sides	 = false;
	various_params.clear();
	basic_description.Empty();
}
/// 設定
/// @param[in] src 元
void DiskBasicParam::SetBasicParam(const DiskBasicParam &src)
{
	basic_type_name = src.basic_type_name;
	basic_category_name = src.basic_category_name;
	format_type = src.format_type;
	format_subtype_number = src.format_subtype_number;
	sectors_per_group = src.sectors_per_group;
	sides_on_basic = src.sides_on_basic;
	sectors_on_basic = src.sectors_on_basic;
	sectors_on_basic_list = src.sectors_on_basic_list;
	sector_number_base = src.sector_number_base;
	tracks_on_basic = src.tracks_on_basic;
	managed_track_number = src.managed_track_number;
	groups_per_track = src.groups_per_track;
	groups_per_sector = src.groups_per_sector;
	reserved_sectors = src.reserved_sectors;
	number_of_fats = src.number_of_fats;
	valid_number_of_fats = src.valid_number_of_fats;
	sectors_per_fat = src.sectors_per_fat;
	fat_start_pos = src.fat_start_pos;
	fat_end_group = src.fat_end_group;
	fat_side_number = src.fat_side_number;
	reserved_groups = src.reserved_groups;
	group_final_code = src.group_final_code;
	group_system_code = src.group_system_code;
	group_unused_code = src.group_unused_code;
	dir_start_sector = src.dir_start_sector;
	dir_end_sector = src.dir_end_sector;
	dir_entry_count = src.dir_entry_count;
	subdir_group_size = src.subdir_group_size;
	dir_terminate_code = src.dir_terminate_code;
	dir_space_code = src.dir_space_code;
	dir_trimming_code = src.dir_trimming_code;
	dir_start_pos = src.dir_start_pos;
	dir_start_pos_on_root = src.dir_start_pos_on_root;
	dir_start_pos_on_sec = src.dir_start_pos_on_sec;
	dir_start_pos_on_group = src.dir_start_pos_on_group;
	group_width = src.group_width;
	groups_per_dir_entry = src.groups_per_dir_entry;
	valid_density_type = src.valid_density_type;
	sector_skew	= src.sector_skew;
	special_attrs = src.special_attrs;
	attrs_by_extension = src.attrs_by_extension;
	fillcode_on_format = src.fillcode_on_format;
	fillcode_on_fat = src.fillcode_on_fat;
	fillcode_on_dir = src.fillcode_on_dir;
	delete_code = src.delete_code;
	media_id = src.media_id;
	text_terminate_code = src.text_terminate_code;
	extension_pre_code = src.extension_pre_code;
	valid_file_name = src.valid_file_name;
	valid_volume_name = src.valid_volume_name;
	compare_case_insense = src.compare_case_insense;
	to_upper_before_dialog = src.to_upper_before_dialog;
	to_upper_after_renamed = src.to_upper_after_renamed;
	data_inverted = src.data_inverted;
	side_reversed = src.side_reversed;
	big_endian = src.big_endian;
	mount_each_sides = src.mount_each_sides;
	various_params = src.various_params;
	basic_description = src.basic_description;
}
/// パラメータを返す
const DiskBasicParam &DiskBasicParam::GetBasicParam() const
{
	return *this;
}
/// 開始終了セクタを計算
void DiskBasicParam::CalcDirStartEndSector(int sector_size)
{
	if (dir_start_sector < 0) {
		dir_start_sector = reserved_sectors + number_of_fats * sectors_per_fat + 1;
	}
	if (dir_end_sector < 0) {
		dir_end_sector = dir_start_sector + dir_entry_count * 32 / sector_size - 1;
	}
	if (dir_entry_count < 0) {
		dir_entry_count = (dir_end_sector - dir_start_sector + 1) * sector_size / 32;
	}
}
/// サイド番号を返す 反転しているときは計算する
int DiskBasicParam::GetReversedSideNumber(int side_num) const
{
	return (side_reversed &&  0 <= side_num && side_num < sides_on_basic ? sides_on_basic - side_num - 1 : side_num);
}
/// 片面のみ使用するOSで、各面を独立してアクセスできるか
bool DiskBasicParam::CanMountEachSides() const
{
	return (mount_each_sides && sides_on_basic == 1);
}
/// 固有のパラメータ
int DiskBasicParam::GetVariousIntegerParam(const wxString &key) const
{
	VariantHash::const_iterator invalid = various_params.end();
	VariantHash::const_iterator it = various_params.find(key);
	if (it != invalid) {
		return (int)it->second.GetLong();
	} else {
		return 0;
	}
}
/// 固有のパラメータ
bool DiskBasicParam::GetVariousBoolParam(const wxString &key) const
{
	VariantHash::const_iterator invalid = various_params.end();
	VariantHash::const_iterator it = various_params.find(key);
	if (it != invalid) {
		return it->second.GetBool();
	} else {
		return false;
	}
}
/// 固有のパラメータ
wxString DiskBasicParam::GetVariousStringParam(const wxString &key) const
{
	VariantHash::const_iterator invalid = various_params.end();
	VariantHash::const_iterator it = various_params.find(key);
	if (it != invalid) {
		return it->second.GetString();
	} else {
		return wxT("");
	}
}

/// 固有のパラメータ
void DiskBasicParam::SetVariousParam(const wxString &key, const wxVariant &val)
{
	various_params[key] = val;
}
/// 説明文でソート
int DiskBasicParam::SortByDescription(const DiskBasicParam **item1, const DiskBasicParam **item2)
{
	return (*item1)->GetBasicDescription().Cmp((*item2)->GetBasicDescription());
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicParams);

//////////////////////////////////////////////////////////////////////
//
//　DISK BASICのカテゴリ(メーカ毎、OS毎にまとめる)クラス
//
DiskBasicCategory::DiskBasicCategory()
{
}
DiskBasicCategory::DiskBasicCategory(const DiskBasicCategory &src)
{
	basic_type_name = src.basic_type_name;
	description = src.description;
}
DiskBasicCategory::DiskBasicCategory(const wxString & n_basic_type_name, const wxString & n_description)
{
	basic_type_name = n_basic_type_name;
	description = n_description;
}

//////////////////////////////////////////////////////////////////////

WX_DEFINE_OBJARRAY(DiskBasicCategories);

//////////////////////////////////////////////////////////////////////
//
// DISK BASICパラメータのテンプレートを提供する
//
DiskBasicTemplates::DiskBasicTemplates()
{
}
/// XMLファイル読み込み
/// @param[in]  data_path   XMLファイルがあるフォルダ
/// @param[in]  locale_name ローケル名
/// @param[out] errmsgs     エラーメッセージ
/// @return true/false
bool DiskBasicTemplates::Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("basic_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "DiskBasics") return false;

	bool valid = true;
	wxXmlNode *item;
	item = doc.GetRoot()->GetChildren();
	while (item && valid) {
		if (item->GetName() == "DiskBasicFormats") {
			valid = LoadFormats(item, locale_name, errmsgs);
		}
		else if (item->GetName() == "DiskBasicCategories") {
			valid = LoadCategories(item, locale_name, errmsgs);
		}
		item = item->GetNext();
	}
	item = doc.GetRoot()->GetChildren();
	while (item && valid) {
		if (item->GetName() == "DiskBasicTypes") {
			valid = LoadTypes(item, locale_name, errmsgs);
		}
		item = item->GetNext();
	}
	return valid;
}

/// DiskBasicTypeエレメントのロード
/// @param[in] node        ノード
/// @param[in] locale_name ローケル名
/// @param[out] errmsgs    エラー時メッセージ
/// @return true / false
bool DiskBasicTemplates::LoadTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	bool valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "DiskBasicType") {
			DiskBasicParam p;

			wxString type_name = item->GetAttribute("name");
			p.SetBasicTypeName(type_name);

			wxString format_name = item->GetAttribute("type");
			const DiskBasicFormat *format_type = FindFormat((DiskBasicFormatType)Utils::ToInt(format_name));
			if (!format_name.IsEmpty() && format_type) {
				p.SetFormatType(format_type);
				p.SetSectorsPerGroup(format_type->GetSectorsPerGroup());
				p.SetGroupFinalCode(format_type->GetGroupFinalCode());
				p.SetGroupSystemCode(format_type->GetGroupSystemCode());
				p.SetGroupUnusedCode(format_type->GetGroupUnusedCode());
				p.SetDirTerminateCode(format_type->GetDirTerminateCode());
				p.SetDirSpaceCode(format_type->GetDirSpaceCode());
				p.SetDirTrimmingCode(format_type->GetDirTrimmingCode());
				p.SetDirStartPos(format_type->GetDirStartPos());
				p.SetDirStartPosOnRoot(format_type->GetDirStartPosOnRoot());
				p.SetDirStartPosOnSector(format_type->GetDirStartPosOnSector());
				p.SetDirStartPosOnGroup(format_type->GetDirStartPosOnGroup());
				p.SetSpecialAttributes(format_type->GetSpecialAttributes());
				p.SetAttributesByExtension(format_type->GetAttributesByExtension());
				p.SetFillCodeOnFormat(format_type->GetFillCodeOnFormat());
				p.SetFillCodeOnFAT(format_type->GetFillCodeOnFAT());
				p.SetFillCodeOnDir(format_type->GetFillCodeOnDir());
				p.SetDeleteCode(format_type->GetDeleteCode());
				p.SetTextTerminateCode(format_type->GetTextTerminateCode());
				p.SetExtensionPreCode(format_type->GetExtensionPreCode());
				p.SetValidFileName(format_type->GetValidFileName());
				p.SetValidVolumeName(format_type->GetValidVolumeName());
				p.CompareCaseInsense(format_type->IsCompareCaseInsense());
				p.ToUpperBeforeDialog(format_type->ToUpperBeforeDialog());
				p.ToUpperAfterRenamed(format_type->ToUpperAfterRenamed());
//				p.RequireFileName(format_type->IsFileNameRequired());
				p.BigEndian(format_type->IsBigEndian());
				p.SetVariousParams(format_type->GetVariousParams());
			} else {
				// フォーマットタイプがない
				errmsgs += wxT("\n");
				errmsgs += _("Unknown format type in DiskBasicType : ");
				errmsgs += type_name;
				return false;
			}

			p.SetBasicCategoryName(item->GetAttribute("category"));

			int reserved_sectors	 = -2;
			int fat_start_sector	 = 0;

			int sectors_per_fat		 = 0;
			int fat_end_sector		 = 0;

			wxString desc, desc_locale;

			wxXmlNode *itemnode = item->GetChildren();
			while (itemnode) {
				wxString name = itemnode->GetName();
				wxString str = itemnode->GetNodeContent();
//				if (itemnode->GetName() == "FormatType") {
//					const DiskBasicFormat *format_type = FindFormat((DiskBasicFormatType)Utils::ToInt(str));
//					p.SetFormatType(format_type);
				if (name == "FormatSubType") {
					p.SetFormatSubTypeNumber(Utils::ToInt(str));
				} else if (name == "SectorsPerGroup") {
					p.SetSectorsPerGroup(Utils::ToInt(str));
				} else if (name == "SidesPerDisk") {
					p.SetSidesPerDiskOnBasic(Utils::ToInt(str));
				} else if (name == "SectorsPerTrack") {
					int sec_param = p.GetSectorsPerTrackOnBasic();
					LoadNumSectorsMap(itemnode, str, sec_param, p.SectorsPerTrackOnBasicList());
					p.SetSectorsPerTrackOnBasic(sec_param);
				} else if (name == "SectorNumberBase") {
					p.SetSectorNumberBaseOnBasic(Utils::ToInt(str));
				} else if (name == "TracksPerSide") {
					p.SetTracksPerSideOnBasic(Utils::ToInt(str));
				} else if (name == "ManagedTrackNumber") {
					p.SetManagedTrackNumber(Utils::ToInt(str));
				} else if (name == "GroupsPerTrack") {
					p.SetGroupsPerTrack(Utils::ToInt(str));
				} else if (name == "GroupsPerSector") {
					p.SetGroupsPerSector(Utils::ToInt(str));
				} else if (name == "ReservedSectors") {
					reserved_sectors = Utils::ToInt(str);
				} else if (name == "NumberOfFATs") {
					p.SetNumberOfFats(Utils::ToInt(str));
				} else if (name == "ValidNumberOfFATs") {
					p.SetValidNumberOfFats(Utils::ToInt(str));
				} else if (name == "SectorsPerFAT") {
					sectors_per_fat = Utils::ToInt(str);
				} else if (name == "FATStartSector") {
					fat_start_sector = Utils::ToInt(str);
				} else if (name == "FATEndSector") {
					fat_end_sector = Utils::ToInt(str);
				} else if (name == "FATStartPosition") {
					p.SetFatStartPos(Utils::ToInt(str));
				} else if (name == "FATEndGroup") {
					p.SetFatEndGroup(Utils::ToInt(str));
				} else if (name == "FATSideNumber") {
					p.SetFatSideNumber(Utils::ToInt(str));
				} else if (name == "ReservedGroups") {
					wxArrayInt reserved_groups;
					LoadReservedGroupsInTypes(itemnode, locale_name, errmsgs, reserved_groups);
					p.SetReservedGroups(reserved_groups);
				} else if (name == "GroupFinalCode") {
					p.SetGroupFinalCode(Utils::ToInt(str));
				} else if (name == "GroupSystemCode") {
					p.SetGroupSystemCode(Utils::ToInt(str));
				} else if (name == "GroupUnusedCode") {
					p.SetGroupUnusedCode(Utils::ToInt(str));
				} else if (name == "DirStartSector") {
					p.SetDirStartSector(Utils::ToInt(str));
				} else if (name == "DirEndSector") {
					p.SetDirEndSector(Utils::ToInt(str));
				} else if (name == "DirEntryCount") {
					p.SetDirEntryCount(Utils::ToInt(str));
				} else if (name == "DirTerminateCode") {
					p.SetDirTerminateCode(Utils::ToInt(str));
				} else if (name == "DirSpaceCode") {
					p.SetDirSpaceCode(Utils::ToInt(str));
				} else if (name == "DirTrimmingCode") {
					p.SetDirTrimmingCode(Utils::ToInt(str));
				} else if (name == "DirStartPosition") {
					p.SetDirStartPos(Utils::ToInt(str));
				} else if (name == "DirStartPositionOnRoot") {
					p.SetDirStartPosOnRoot(Utils::ToInt(str));
				} else if (name == "DirStartPositionOnSector") {
					p.SetDirStartPosOnSector(Utils::ToInt(str));
				} else if (name == "DirStartPositionOnGroup") {
					p.SetDirStartPosOnGroup(Utils::ToInt(str));
				} else if (name == "GroupWidth") {
					p.SetGroupWidth(Utils::ToInt(str));
				} else if (name == "GroupsPerDirEntry") {
					p.SetGroupsPerDirEntry(Utils::ToInt(str));
				} else if (name == "ValidDensityType") {
					p.SetValidDensityType(Utils::ToInt(str));
				} else if (name == "SectorSkew") {
					p.SetSectorSkew(Utils::ToInt(str));
				} else if (name == "SectorSkewMap") {
					wxArrayInt skew_map;
					LoadSectorSkewMap(itemnode, skew_map);
					p.SetSectorSkewMap(skew_map);
				} else if (name == "SubDirGroupSize") {
					p.SetSubDirGroupSize(Utils::ToInt(str));
				} else if (name == "FillCodeOnFormat") {
					p.SetFillCodeOnFormat(Utils::ToInt(str));
				} else if (name == "FillCodeOnFAT") {
					p.SetFillCodeOnFAT(Utils::ToInt(str));
				} else if (name == "FillCodeOnDir") {
					p.SetFillCodeOnDir(Utils::ToInt(str));
				} else if (name == "DeleteCodeOnDir") {
					p.SetDeleteCode(Utils::ToInt(str));
				} else if (name == "MediaID") {
					p.SetMediaId(Utils::ToInt(str));
				} else if (name == "TextTerminateCode") {
					p.SetTextTerminateCode(Utils::ToInt(str));
				} else if (name == "ExtensionPreCode") {
					p.SetExtensionPreCode(Utils::ToInt(str));
				} else if (name == "FileNameCharacters") {
					valid = LoadValidChars(itemnode, p.ValidFileName(), errmsgs);
				} else if (name == "VolumeNameCharacters") {
					valid = LoadValidChars(itemnode, p.ValidVolumeName(), errmsgs);
				} else if (name == "FileNameCompareCase") {
					bool val;
					LoadFileNameCompareCase(itemnode, val);
					p.CompareCaseInsense(val);
				} else if (name == "ToUpperFileNameBeforeDialog") {
					p.ToUpperBeforeDialog(Utils::ToBool(str));
				} else if (name == "ToUpperFileNameAfterRenamed") {
					p.ToUpperAfterRenamed(Utils::ToBool(str));
				} else if (name == "RequireFileName") {
					p.ValidFileName().RequireName(Utils::ToBool(str));
				} else if (name == "RequireVolumeName") {
					p.ValidVolumeName().RequireName(Utils::ToBool(str));
				} else if (name == "VolumeNameMaxLength") {
					p.ValidVolumeName().SetMaxLength(Utils::ToInt(str));
				} else if (name == "DataInverted") {
					p.DataInverted(Utils::ToBool(str));
				} else if (name == "SideReversed") {
					p.SideReversed(Utils::ToBool(str));
				} else if (name == "Endian") {
					p.BigEndian(str.Upper() == "BIG");
				} else if (name == "CanMountEachSides") {
					p.MountEachSides(Utils::ToBool(str));
				} else if (name == "SpecialAttributes") {
					L3Attributes attrs;
					LoadL3AttributesInTypes(itemnode, locale_name, errmsgs, attrs);
					p.SetSpecialAttributes(attrs);
				} else if (name == "AttributesByExtension") {
					L3Attributes attrs;
					LoadL3AttributesInTypes(itemnode, locale_name, errmsgs, attrs);
					p.SetAttributesByExtension(attrs);
				} else if (name == "Description") {
					LoadDescription(itemnode, locale_name, desc, desc_locale);
				} else if (!name.IsEmpty()) {
					wxVariant nval;
					LoadVariousParam(itemnode, str, nval);
					p.SetVariousParam(name, nval);
				}
				itemnode = itemnode->GetNext();
			}
			if (fat_start_sector > 0 && reserved_sectors <= 0) {
				reserved_sectors = fat_start_sector - 1;
			}
			p.SetReservedSectors(reserved_sectors);

			if (fat_end_sector > 0 && sectors_per_fat <= 0) {
				sectors_per_fat = fat_end_sector - fat_start_sector + 1;
			}
			p.SetSectorsPerFat(sectors_per_fat);

			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			p.SetBasicDescription(desc);

			if (FindType(wxEmptyString, type_name) == NULL) {
				types.Add(p);
			} else {
				// タイプ名が重複している
				errmsgs += wxT("\n");
				errmsgs += _("Duplicate type name in DiskBasicType : ");
				errmsgs += type_name;
				valid = false;
				break;
			}
		}
		item = item->GetNext();
	}
	return valid;
}

/// Descriptionエレメントをロード
/// @param[in]  node        子ノード
/// @param[in]  locale_name ローケル名
/// @param[out] desc        説明
/// @param[out] desc_locale 説明ローケル
/// @return true
bool DiskBasicTemplates::LoadDescription(const wxXmlNode *node, const wxString &locale_name, wxString &desc, wxString &desc_locale)
{
	if (node->HasAttribute("lang")) {
		wxString lang = node->GetAttribute("lang");
		if (locale_name.Find(lang) != wxNOT_FOUND) {
			desc_locale = node->GetNodeContent();
		}
	} else {
		desc = node->GetNodeContent();
	}
	return true;
}

/// ReservedGroupsエレメントをロード
/// @param[in]  node        子ノード
/// @param[in]  locale_name ローケル名
/// @param[out] errmsgs     エラーメッセージ
/// @param[out] reserved_groups 予約済みグループ
/// @return true
bool DiskBasicTemplates::LoadReservedGroupsInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs, wxArrayInt &reserved_groups)
{
	wxXmlNode *citemnode = node->GetChildren();
	while(citemnode) {
		if (citemnode->GetName() == "Group") {
			wxString first = citemnode->GetAttribute("first");
			wxString last = citemnode->GetAttribute("last");
			if (!first.IsEmpty() && !last.IsEmpty()) {
				int fval = Utils::ToInt(first);
				int lval = Utils::ToInt(last);
				for(int i=fval; i<=lval; i++) {
					reserved_groups.Add(i);
				}
			}
			wxString str = citemnode->GetNodeContent();
			if (!str.IsEmpty()) {
				int reserved_group = Utils::ToInt(str);
				reserved_groups.Add(reserved_group);
			}
		}
		citemnode = citemnode->GetNext();
	}
	return true;
}

/// アトリビュート名と属性の関係マップ
static const struct st_special_attr_names {
	const char *name;
	int type;
} cSpecialAttrNames[] = {
	{ "MachineBinary",	FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK },
	{ "BasicBinary",	FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK },
	{ "BasicAscii",		FILE_TYPE_BASIC_MASK | FILE_TYPE_ASCII_MASK },
	{ "BasicInteger",	FILE_TYPE_BASIC_MASK | FILE_TYPE_INTEGER_MASK },
	{ "DataAscii",		FILE_TYPE_DATA_MASK | FILE_TYPE_ASCII_MASK },
	{ "DataBinary",		FILE_TYPE_DATA_MASK | FILE_TYPE_BINARY_MASK },
	{ "DataRandom",		FILE_TYPE_DATA_MASK | FILE_TYPE_RANDOM_MASK },
	{ "Binary",			FILE_TYPE_BINARY_MASK },
	{ "Ascii",			FILE_TYPE_ASCII_MASK },
	{ "Random",			FILE_TYPE_RANDOM_MASK },
	{ "Volume",			FILE_TYPE_VOLUME_MASK },
	{ "System",			FILE_TYPE_SYSTEM_MASK },
	{ NULL, 0 }
};

/// SpecialAttributes/AttributesByExtensionエレメントをロード
/// @param[in]  node        子ノード
/// @param[in]  locale_name ローケル名
/// @param[out] errmsgs     エラーメッセージ
/// @param[out] attrs       値
/// @return true
bool DiskBasicTemplates::LoadL3AttributesInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs, L3Attributes &attrs)
{
	wxXmlNode *citemnode = node->GetChildren();
	while(citemnode) {
		for(int i=0; cSpecialAttrNames[i].name != NULL; i++) {
			if (citemnode->GetName() == cSpecialAttrNames[i].name) {
				LoadL3Attribute(citemnode, locale_name, cSpecialAttrNames[i].type, attrs);
				break;
			}
		}
		citemnode = citemnode->GetNext();
	}
	return true;
}

/// SpecialAttributes/AttributesByExtensionエレメントをロード
/// @param[in]  node        子ノード
/// @param[in]  locale_name ローケル名
/// @param[in]  type        タイプ
/// @param[out] attrs       値
/// @return true
bool DiskBasicTemplates::LoadL3Attribute(const wxXmlNode *node, const wxString &locale_name, int type, L3Attributes &attrs)
{
	wxString desc, desc_locale;
	wxString name = node->GetAttribute("name");
	wxString sval = node->GetAttribute("value");
	wxString smsk = node->GetAttribute("mask");
	int val = Utils::ToInt(sval);
	int msk = (!smsk.IsEmpty() ? Utils::ToInt(smsk) : -1);
	wxXmlNode *cnode = node->GetChildren();
	while(cnode) {
		if (cnode->GetName() == "Description") {
			LoadDescription(cnode, locale_name, desc, desc_locale);
		}
		cnode = cnode->GetNext();
	}
	if (!desc_locale.IsEmpty()) {
		desc = desc_locale;
	}
	attrs.Add(L3Attribute((int)attrs.Count(), type, val, msk, name, desc));
	return true;
}

/// FileNameCharacters/VolumeNameCharactersエレメントをロード
/// @param[in]  node        子ノード
/// @param[out] valid_chars 値 
/// @param[out] errmsgs     エラー時メッセージ
/// @return true
bool DiskBasicTemplates::LoadValidChars(const wxXmlNode *node, ValidNameRule &valid_chars, wxString &errmsgs)
{
	bool valid = true;
	wxString chars[4];
	wxXmlNode *cnode = node->GetChildren();
	while(cnode) {
		int encoding = Utils::ToInt(cnode->GetAttribute(wxT("encoding")));
		wxString name = cnode->GetName();
		int num = -1;
		if (name.Left(5) == "Valid") {
			num = 0;
			name = name.Mid(5);
		} else if (name.Left(7) == "Invalid") {
			num = 1;
			name = name.Mid(7);
		} else if (name.Left(9) == "Duplicate") {
			num = 2;
			name = name.Mid(9);
		}

		if (num == 0) {
			if (name.Left(5) == "First") {
				num = 3;
				name = name.Mid(5);
			}
		}

		if (num >= 0) {
			if (name == "CharSet") {
				wxString rstr;
				Utils::DecodeEscape(cnode->GetNodeContent(), rstr);
				chars[num] += rstr;
			} else if (name == "Code") {
				int c = Utils::ToInt(cnode->GetNodeContent());
				if (encoding == 0) {
					if (c < 0 || c >= 0x80) {
						errmsgs += wxT("\n");
						errmsgs += _("Out of range in InvalidateCharacters::Code");
						errmsgs += wxString::Format(_("(line #%d)"), cnode->GetLineNumber());
						valid = false;
					} else {
						chars[num] += wxString((char)c, (size_t)1);
					}
				} else {
					// unicode
					chars[num] += wxString((wchar_t)c, (size_t)1);
				}
			} else if (name == "CodeRange") {
				int st = Utils::ToInt(cnode->GetAttribute("first"));
				int ed = Utils::ToInt(cnode->GetAttribute("last"));
				if (encoding == 0) {
					for(int c=st; c<=ed; c++) {
						if (c < 0 || c >= 0x80) {
							errmsgs += wxT("\n");
							errmsgs += _("Out of range in InvalidateCharacters::CodeRange");
							errmsgs += wxString::Format(_("(line #%d)"), cnode->GetLineNumber());
							valid = false;
							break;
						} else {
							chars[num] += wxString((char)c, (size_t)1);
						}
					}
				} else {
					// unicode
					for(int c=st; c<=ed; c++) {
						chars[num] += wxString((wchar_t)c, (size_t)1);
					}
				}
			}
		}
		cnode = cnode->GetNext();
	}
	if (valid) {
		valid_chars.SetValidChars(chars[0]);
		valid_chars.SetInvalidChars(chars[1]);
		valid_chars.SetDeduplicateChars(chars[2]);
		valid_chars.SetValidFirstChars(chars[3]);
	}
	return valid;
}

/// FileNameCompareCaseエレメントをロード
/// @param[in] node 子ノード
/// @param[out] val 値
/// @return true
bool DiskBasicTemplates::LoadFileNameCompareCase(const wxXmlNode *node, bool &val)
{
	wxString str = node->GetNodeContent();
	val = (str.Upper() == wxT("INSENSITIVE"));
	return true;
}

/// SectorSkewMapエレメントをロード
/// @param[in] node 子ノード
/// @param[out] map マップ
/// @return true
bool DiskBasicTemplates::LoadSectorSkewMap(const wxXmlNode *node, wxArrayInt &map)
{
	wxXmlNode *cnode = node->GetChildren();
	while(cnode) {
		wxString name = cnode->GetName();
		if (name == "Value") {
			map.Add(Utils::ToInt(cnode->GetNodeContent()));
		}
		cnode = cnode->GetNext();
	}
	return true;
}

/// SectorsPerTrackエレメントをロード
/// @param[in] node        子ノード
/// @param[in] val         入力値
/// @param[out] sec_param  セクタ数/トラック(全トラック同じの場合)
/// @param[out] sec_params セクタ数/トラック(トラック毎に異なる場合)
/// @return true
bool DiskBasicTemplates::LoadNumSectorsMap(const wxXmlNode *node, const wxString &val, int &sec_param, NumSectorsParams &sec_params)
{
	wxString str;
	int start_track = -1;
	int num_of_tracks = -1;
	int sec_per_trk = 1;
	if (node->GetAttribute("start", &str)) {
		start_track = Utils::ToInt(str);
	}
	if (node->GetAttribute("tracks", &str)) {
		num_of_tracks = Utils::ToInt(str);
	}
	sec_per_trk = Utils::ToInt(val);

	if (start_track < 0 && num_of_tracks < 0) {
		sec_param = sec_per_trk;
	} else {
		sec_params.Add(NumSectorsParam(start_track, num_of_tracks, sec_per_trk));
	}
	return true;
}

/// 独自エレメントのロード
/// @param[in] node  子ノード
/// @param[in] val   入力値
/// @param[out] nval 変換後の値
/// @return true
bool DiskBasicTemplates::LoadVariousParam(const wxXmlNode *node, const wxString &val, wxVariant &nval)
{
	wxString type = node->GetAttribute("type").Upper();
	if (type == "INT") {
		int ival = Utils::ToInt(val);
		nval = (long)ival;
	} else if (type == "BOOL") {
		bool bval = Utils::ToBool(val);
		nval = bval;
	} else {
		wxString sval;
		Utils::DecodeEscape(val, sval);
		nval = sval;
	}
	return true;
}

/// DiskBasicFormatエレメントのロード
/// @param[in] node        ノード
/// @param[in] locale_name ローケル名
/// @param[out] errmsgs    エラー時メッセージ
/// @return true / false
bool DiskBasicTemplates::LoadFormats(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	bool valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "DiskBasicFormat") {
			DiskBasicFormat f;
			bool set_volume_rule = false;
			wxString s_type_number = item->GetAttribute("type");
			int type_number = Utils::ToInt(s_type_number);
			f.SetTypeNumber((DiskBasicFormatType)type_number);

			wxXmlNode *itemnode = item->GetChildren();
			while (itemnode) {
				wxString name = itemnode->GetName();
				wxString str = itemnode->GetNodeContent();
				if (name == "HasVolumeName") {
					f.HasVolumeName(Utils::ToBool(str));
				} else if (name == "HasVolumeNumber") {
					f.HasVolumeNumber(Utils::ToBool(str));
				} else if (name == "HasVolumeDate") {
					f.HasVolumeDate(Utils::ToBool(str));
				} else if (name == "SectorsPerGroup") {
					f.SetSectorsPerGroup(Utils::ToInt(str));
				} else if (name == "GroupFinalCode") {
					f.SetGroupFinalCode(Utils::ToInt(str));
				} else if (name == "GroupSystemCode") {
					f.SetGroupSystemCode(Utils::ToInt(str));
				} else if (name == "GroupUnusedCode") {
					f.SetGroupUnusedCode(Utils::ToInt(str));
				} else if (name == "DirTerminateCode") {
					f.SetDirTerminateCode(Utils::ToInt(str));
				} else if (name == "DirSpaceCode") {
					f.SetDirSpaceCode(Utils::ToInt(str));
				} else if (name == "DirTrimmingCode") {
					f.SetDirTrimmingCode(Utils::ToInt(str));
				} else if (name == "DirStartPosition") {
					f.SetDirStartPos(Utils::ToInt(str));
				} else if (name == "DirStartPositionOnRoot") {
					f.SetDirStartPosOnRoot(Utils::ToInt(str));
				} else if (name == "DirStartPositionOnSector") {
					f.SetDirStartPosOnSector(Utils::ToInt(str));
				} else if (name == "DirStartPositionOnGroup") {
					f.SetDirStartPosOnGroup(Utils::ToInt(str));
				} else if (name == "SpecialAttributes") {
					L3Attributes attrs;
					LoadL3AttributesInTypes(itemnode, locale_name, errmsgs, attrs);
					f.SetSpecialAttributes(attrs);
				} else if (name == "AttributesByExtension") {
					L3Attributes attrs;
					LoadL3AttributesInTypes(itemnode, locale_name, errmsgs, attrs);
					f.SetAttributesByExtension(attrs);
				} else if (name == "FillCodeOnFormat") {
					f.SetFillCodeOnFormat(Utils::ToInt(str));
				} else if (name == "FillCodeOnFAT") {
					f.SetFillCodeOnFAT(Utils::ToInt(str));
				} else if (name == "FillCodeOnDir") {
					f.SetFillCodeOnDir(Utils::ToInt(str));
				} else if (name == "DeleteCodeOnDir") {
					f.SetDeleteCode(Utils::ToInt(str));
				} else if (name == "TextTerminateCode") {
					f.SetTextTerminateCode(Utils::ToInt(str));
				} else if (name == "ExtensionPreCode") {
					f.SetExtensionPreCode(Utils::ToInt(str));
				} else if (name == "FileNameCharacters") {
					valid = LoadValidChars(itemnode, f.ValidFileName(), errmsgs);
					if (!set_volume_rule) {
						f.SetValidVolumeName(f.GetValidFileName());
					}
				} else if (name == "VolumeNameCharacters") {
					valid = LoadValidChars(itemnode, f.ValidVolumeName(), errmsgs);
					set_volume_rule = valid;
				} else if (name == "FileNameCompareCase") {
					bool val;
					LoadFileNameCompareCase(itemnode, val);
					f.CompareCaseInsense(val);
				} else if (name == "ToUpperFileNameBeforeDialog") {
					f.ToUpperBeforeDialog(Utils::ToBool(str));
				} else if (name == "ToUpperFileNameAfterRenamed") {
					f.ToUpperAfterRenamed(Utils::ToBool(str));
				} else if (name == "RequireFileName") {
					f.ValidFileName().RequireName(Utils::ToBool(str));
				} else if (name == "RequireVolumeName") {
					f.ValidVolumeName().RequireName(Utils::ToBool(str));
				} else if (name == "VolumeNameMaxLength") {
					f.ValidVolumeName().SetMaxLength(Utils::ToInt(str));
				} else if (name == "Endian") {
					f.BigEndian(str.Upper() == "BIG");
				} else if (!name.IsEmpty()) {
					wxVariant nval;
					LoadVariousParam(itemnode, str, nval);
					f.SetVariousParam(name, nval);
				}
				itemnode = itemnode->GetNext();
			}

			if (FindFormat((DiskBasicFormatType)type_number) == NULL) {
				formats.Add(f);
			} else {
				errmsgs += wxT("\n");
				errmsgs += _("Duplicate type number in DiskBasicFormat : ");
				errmsgs += wxString::Format(wxT("%d"), type_number);
				valid = false;
				break;
			}
		}
		item = item->GetNext();
	}
	return valid;
}

/// DiskBasicCategoryエレメントのロード
/// @param[in] node        ノード
/// @param[in] locale_name ローケル名
/// @param[out] errmsgs    エラー時メッセージ
/// @return true / false
bool DiskBasicTemplates::LoadCategories(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	bool valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "DiskBasicCategory") {
			wxString type_name = item->GetAttribute("name");
			wxString desc, desc_locale;

			wxXmlNode *itemnode = item->GetChildren();
			while (itemnode) {
				if (itemnode->GetName() == "Description") {
					if (itemnode->HasAttribute("lang")) {
						wxString lang = itemnode->GetAttribute("lang");
						if (locale_name.Find(lang) != wxNOT_FOUND) {
							desc_locale = itemnode->GetNodeContent();
						}
					} else {
						desc = itemnode->GetNodeContent();
					}
				}
				itemnode = itemnode->GetNext();
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			DiskBasicCategory c(
				type_name,
				desc
			);
			if (FindCategory(type_name) == NULL) {
				categories.Add(c);
			} else {
				errmsgs += wxT("\n");
				errmsgs += _("Duplicate type name in DiskBasicCategory : ");
				errmsgs += type_name;
				valid = false;
				break;
			}
		}
		item = item->GetNext();
	}
	return valid;
}

/// カテゴリとタイプに一致するパラメータを検索
/// @param [in] n_category  : カテゴリ名 空文字列の場合は検索条件からはずす
/// @param [in] n_basic_type: タイプ名
/// @return 一致したパラメータ
const DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const wxString &n_basic_type) const
{
	DiskBasicParam *match_item = NULL;
	for(size_t i=0; i<types.Count(); i++) {
		DiskBasicParam *item = &types[i];
		if (n_category.IsEmpty() || n_category == item->GetBasicCategoryName()) {
			if (n_basic_type == item->GetBasicTypeName()) {
				match_item = item;
				break;
			}
		}
	}
	return match_item;
}
/// カテゴリが一致し、タイプリストに含まれるパラメータを検索
/// @param [in] n_category   : カテゴリ名 空文字列の場合は検索条件からはずす
/// @param [in] n_basic_types: タイプ名リスト
/// @return 一致したパラメータ
const DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const DiskParamNames &n_basic_types) const
{
	const DiskBasicParam *match_item = NULL;
	for(size_t i=0; i<n_basic_types.Count() && match_item == NULL; i++) {
		match_item = this->FindType(n_category, n_basic_types.Item(i).GetName());
	}
	return match_item;
}
/// カテゴリ、タイプ、サイド数とセクタ数が一致するパラメータを検索
/// @note まず、カテゴリ＆タイプで検索し、なければカテゴリ＆サイド数＆セクタ数で検索
/// @param [in] n_category   : カテゴリ名 必須
/// @param [in] n_basic_type : タイプ名 必須
/// @param [in] n_sides      : サイド数
/// @param [in] n_sectors    : セクタ数/トラック -1の場合は検索条件からはずす
/// @return 一致したパラメータ
const DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const wxString &n_basic_type, int n_sides, int n_sectors) const
{
	const DiskBasicParam *match_item = NULL;
	// カテゴリ、タイプで一致するか
	match_item = FindType(n_category, n_basic_type);
	// カテゴリ、サイド数、セクタ数で一致するか
	if (!match_item) {
		for(size_t i=0; i<types.Count(); i++) {
			DiskBasicParam *item = &types[i];
			if (n_category == item->GetBasicCategoryName()) {
				if (n_sides == item->GetSidesPerDiskOnBasic()) {
					if (n_sectors < 0 || item->GetSectorsPerTrackOnBasic() < 0 || n_sectors == item->GetSectorsPerTrackOnBasic()) {
						match_item = item;
						break;
					}
				}
			}
		}
	}
	return match_item;
}
/// DISK BASICフォーマット種類に一致するタイプを検索
/// @param [in]  n_format_types : DISK BASICフォーマット種類
/// @param [out] n_types        : 一致したタイプリスト
/// @return リストの数
size_t DiskBasicTemplates::FindTypes(const wxArrayInt &n_format_types, DiskBasicParams &n_types) const
{
	n_types.Clear();
	for(size_t n=0; n<types.Count(); n++) {
		const DiskBasicParam *item = &types[n];
		for(size_t i=0; i<n_format_types.Count(); i++) {
			const DiskBasicFormat *fmt = item->GetFormatType();
			if (fmt && n_format_types.Item(i) == fmt->GetTypeNumber()) {
				n_types.Add(*item);
			}
		}
	}
	return n_types.Count();
}
/// カテゴリ番号に一致するタイプ名リストを検索
/// @param [in]  n_category_index : カテゴリ番号
/// @param [out] n_type_names     : タイプ名リスト
/// @return リストの数
size_t DiskBasicTemplates::FindTypeNames(size_t n_category_index, wxArrayString &n_type_names) const
{
	return FindTypeNames(categories.Item(n_category_index).GetBasicTypeName(), n_type_names);
}
/// カテゴリ名に一致するタイプ名リストを検索
/// @param [in]  n_category_name  : カテゴリ名
/// @param [out] n_type_names     : タイプ名リスト
/// @return リストの数
size_t DiskBasicTemplates::FindTypeNames(const wxString &n_category_name, wxArrayString &n_type_names) const
{
	n_type_names.Clear();
	for(size_t n=0; n<types.Count(); n++) {
		const DiskBasicParam *item = &types[n];
		if (item->GetBasicCategoryName() == n_category_name) {
			n_type_names.Add(item->GetBasicTypeName());
		}
	}
	return n_type_names.Count();
}
/// フォーマット種類を検索
/// @param [in]  format_type  : フォーマット種類
const DiskBasicFormat *DiskBasicTemplates::FindFormat(DiskBasicFormatType format_type) const
{
	DiskBasicFormat *match = NULL;
	for(size_t n=0; n<formats.Count(); n++) {
		DiskBasicFormat *item = &formats.Item(n);
		if (item->GetTypeNumber() == format_type) {
			match = item;
			break;
		}
	}
	return match;
}

/// タイプリストと一致するパラメータを得る
/// @note パラメータリストは説明文でソートする
/// @param [in]  n_type_names     : タイプ名リスト
/// @param [out] params           : パラメータリスト
/// @return パラメータリストの数
size_t DiskBasicTemplates::FindParams(const DiskParamNames &n_type_names, DiskBasicParamPtrs &params) const
{
	for(size_t n = 0; n < n_type_names.Count(); n++) {
		const DiskBasicParam *param = FindType(wxEmptyString, n_type_names.Item(n).GetName());
		if (!param) continue;
		params.Add(param);
	}
	params.Sort(&DiskBasicParam::SortByDescription);

	return params.Count();
}

/// カテゴリを検索
/// @param [in]  n_category : カテゴリ名
const DiskBasicCategory *DiskBasicTemplates::FindCategory(const wxString &n_category) const
{
	DiskBasicCategory *match = NULL;
	for(size_t n=0; n<categories.Count(); n++) {
		DiskBasicCategory *item = &categories.Item(n);
		if (item->GetBasicTypeName() == n_category) {
			match = item;
			break;
		}
	}
	return match;
}
