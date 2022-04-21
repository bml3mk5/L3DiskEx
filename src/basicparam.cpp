/// @file basicparam.cpp
///
/// @brief disk basic parameter
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicparam.h"
#include "utils.h"
#include <wx/xml/xml.h>


DiskBasicTemplates gDiskBasicTemplates;

//////////////////////////////////////////////////////////////////////
//
//
//
SpecialAttribute::SpecialAttribute(int n_idx, int n_type, int n_value, int n_mask)
{
	idx = n_idx;
	type = n_type;
	value = n_value;
	mask = n_mask;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfSpecialAttribute);

SpecialAttributes::SpecialAttributes()
	: ArrayOfSpecialAttribute()
{
}
/// 属性タイプと値に一致するアイテムを返す
const SpecialAttribute *SpecialAttributes::Find(int type, int value) const
{
	const SpecialAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		SpecialAttribute *attr = &Item(i);
		if (attr->GetType() == type && attr->GetValue() == (value & attr->GetMask())) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性値に一致するアイテムを返す
const SpecialAttribute *SpecialAttributes::Find(int value) const
{
	const SpecialAttribute *match = NULL;
	for(size_t i=0; i<Count(); i++) {
		SpecialAttribute *attr = &Item(i);
		if (attr->GetValue() == (value & attr->GetMask())) {
			match = attr;
			break;
		}
	}
	return match;
}
/// 属性値に一致するアイテムの位置を返す
int SpecialAttributes::GetIndexByValue(int value) const
{
	int idx = -1;
	const SpecialAttribute *match = Find(value);
	if (match) {
		idx = match->GetIndex();
	}
	return idx;
}
/// 属性値に一致するアイテムの属性タイプを返す
int SpecialAttributes::GetTypeByValue(int value) const
{
	int type = 0;
	const SpecialAttribute *match = Find(value);
	if (match) {
		type = match->GetType();
	}
	return type;
}
/// 位置から属性タイプを返す
int SpecialAttributes::GetTypeByIndex(int idx) const
{
	return idx < (int)Count() ? Item(idx).GetType() : 0;
}
/// 位置から属性値を返す
int SpecialAttributes::GetValueByIndex(int idx) const
{
	return idx < (int)Count() ? Item(idx).GetValue() : 0;
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicFormat::DiskBasicFormat()
{
	type_number = FORMAT_TYPE_NONE;
	has_volume_name = false;
	has_volume_number = false;
}
DiskBasicFormat::DiskBasicFormat(
		int	 n_type_number,
		bool n_has_volume_name,
		bool n_has_volume_number
) {
	type_number = (DiskBasicFormatType)n_type_number;
	has_volume_name = n_has_volume_name;
	has_volume_number = n_has_volume_number;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicFormats);

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicParam::DiskBasicParam()
{
	this->ClearBasicParam();
}
DiskBasicParam::DiskBasicParam(const DiskBasicParam &src)
{
	this->SetBasicParam(src);
}
DiskBasicParam::DiskBasicParam(
	const wxString &	n_basic_type_name,
	const wxString &	n_basic_category_name,
	const DiskBasicFormat *n_format_type,
	int					n_sectors_per_group,
	int					n_sides_on_basic,
	int					n_sectors_on_basic,
	int					n_tracks_on_basic,
	int					n_managed_track_number,
	int					n_reserved_sectors,
	int					n_number_of_fats,
	int					n_sectors_per_fat,
	int					n_fat_start_pos,
	wxUint32			n_fat_end_group,
	int					n_fat_side_number,
	const wxArrayInt &	n_reserved_groups,
	wxUint32			n_group_final_code,
	wxUint32			n_group_system_code,
	wxUint32			n_group_unused_code,
	int					n_dir_start_sector,
	int					n_dir_end_sector,
	int					n_dir_entry_count,
	int					n_subdir_group_size,
	wxUint8				n_dir_space_code,
	int					n_dir_start_pos,
	int					n_dir_start_pos_on_root,
	int					n_dir_start_pos_on_sec,
	int					n_groups_per_dir_entry,
	int					n_id_sector_pos,
	const wxString &	n_id_string,
	const wxString &	n_ipl_string,
	const wxString &	n_volume_string,
	const SpecialAttributes & n_special_attrs,
	wxUint8				n_fillcode_on_format,
	wxUint8				n_fillcode_on_fat,
	wxUint8				n_fillcode_on_dir,
	wxUint8				n_delete_code,
	wxUint8				n_media_id,
	const wxString &	n_invalidate_chars,
	bool				n_data_inverted,
	bool				n_side_reversed,
	bool				n_mount_each_sides,
	const wxString &	n_basic_description
) {
	basic_type_name = n_basic_type_name;
	basic_category_name = n_basic_category_name;
	format_type = n_format_type;
	sectors_per_group = n_sectors_per_group;
	sides_on_basic = n_sides_on_basic;
	sectors_on_basic = n_sectors_on_basic;
	tracks_on_basic = n_tracks_on_basic;
	managed_track_number = n_managed_track_number;
	reserved_sectors = n_reserved_sectors;
	number_of_fats = n_number_of_fats;
	sectors_per_fat = n_sectors_per_fat;
	fat_start_pos = n_fat_start_pos;
	fat_end_group = n_fat_end_group;
	fat_side_number = n_fat_side_number;
	reserved_groups = n_reserved_groups;
	group_final_code = n_group_final_code;
	group_system_code = n_group_system_code;
	group_unused_code = n_group_unused_code;
	dir_start_sector = n_dir_start_sector;
	dir_end_sector = n_dir_end_sector;
	dir_entry_count = n_dir_entry_count;
	subdir_group_size = n_subdir_group_size;
	dir_space_code = n_dir_space_code;
	dir_start_pos = n_dir_start_pos;
	dir_start_pos_on_root = n_dir_start_pos_on_root;
	dir_start_pos_on_sec = n_dir_start_pos_on_sec;
	groups_per_dir_entry = n_groups_per_dir_entry;
	id_sector_pos = n_id_sector_pos;
	id_string = n_id_string;
	ipl_string = n_ipl_string;
	volume_string = n_volume_string;
	special_attrs = n_special_attrs;
	fillcode_on_format = n_fillcode_on_format;
	fillcode_on_fat = n_fillcode_on_fat;
	fillcode_on_dir = n_fillcode_on_dir;
	delete_code = n_delete_code;
	media_id = n_media_id;
	invalidate_chars = n_invalidate_chars;
	data_inverted = n_data_inverted;
	side_reversed = n_side_reversed;
	mount_each_sides = n_mount_each_sides;
	basic_description = n_basic_description;
}
void DiskBasicParam::ClearBasicParam()
{
	basic_type_name.Empty();
	basic_category_name.Empty();
	format_type			 = NULL;
	sectors_per_group	 = 0;
	sides_on_basic		 = 0;
	sectors_on_basic	 = -1;
	tracks_on_basic		 = -1;
	managed_track_number = 0;
	reserved_sectors	 = 0;
	number_of_fats		 = 0;
	sectors_per_fat		 = 0;
	fat_start_pos		 = 0;
	fat_end_group		 = 0;
	fat_side_number		 = -1;
	reserved_groups.Empty();
	group_final_code	 = 0;
	group_system_code	 = 0;
	group_unused_code	 = 0;
	dir_start_sector	 = 0;
	dir_end_sector		 = 0;
	dir_entry_count		 = 0;
	subdir_group_size	 = 0;
	dir_space_code		 = 0x20;
	dir_start_pos		 = 0;
	dir_start_pos_on_root = 0;
	dir_start_pos_on_sec = 0;
	groups_per_dir_entry = 0;
	id_sector_pos		 = 0;
	id_string.Empty();
	ipl_string.Empty();
	volume_string.Empty();
	special_attrs.Empty();
	fillcode_on_format	 = 0x40;
	fillcode_on_fat		 = 0xff;
	fillcode_on_dir		 = 0xff;
	delete_code			 = 0x00;
	media_id			 = 0x00;
	invalidate_chars.Empty();
	data_inverted		 = false;
	side_reversed		 = false;
	mount_each_sides	 = false;
	basic_description.Empty();
}
void DiskBasicParam::SetBasicParam(const DiskBasicParam &src)
{
	basic_type_name = src.basic_type_name;
	basic_category_name = src.basic_category_name;
	format_type = src.format_type;
	sectors_per_group = src.sectors_per_group;
	sides_on_basic = src.sides_on_basic;
	sectors_on_basic = src.sectors_on_basic;
	tracks_on_basic = src.tracks_on_basic;
	managed_track_number = src.managed_track_number;
	reserved_sectors = src.reserved_sectors;
	number_of_fats = src.number_of_fats;
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
	dir_space_code = src.dir_space_code;
	dir_start_pos = src.dir_start_pos;
	dir_start_pos_on_root = src.dir_start_pos_on_root;
	dir_start_pos_on_sec = src.dir_start_pos_on_sec;
	groups_per_dir_entry = src.groups_per_dir_entry;
	id_sector_pos = src.id_sector_pos;
	id_string = src.id_string;
	ipl_string = src.ipl_string;
	volume_string = src.volume_string;
	special_attrs = src.special_attrs;
	fillcode_on_format = src.fillcode_on_format;
	fillcode_on_fat = src.fillcode_on_fat;
	fillcode_on_dir = src.fillcode_on_dir;
	delete_code = src.delete_code;
	media_id = src.media_id;
	invalidate_chars = src.invalidate_chars;
	data_inverted = src.data_inverted;
	side_reversed = src.side_reversed;
	mount_each_sides = src.mount_each_sides;
	basic_description = src.basic_description;
}
const DiskBasicParam &DiskBasicParam::GetBasicParam() const
{
	return *this;
}
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
/// 属性が一致するか
const SpecialAttribute *DiskBasicParam::FindSpecialAttr(int type, int value) const
{
	return special_attrs.Find(type, value);
}
/// 属性が一致するか
const SpecialAttribute *DiskBasicParam::FindSpecialAttr(int value) const
{
	return special_attrs.Find(value);
}
/// 特別な属性の位置
int DiskBasicParam::GetIndexByValueOfSpecialAttr(int value) const
{
	return special_attrs.GetIndexByValue(value);
}
/// 特別な属性の属性値を返す
int DiskBasicParam::GetTypeByValueOfSpecialAttr(int value) const
{
	return special_attrs.GetTypeByValue(value);
}
/// 特別な属性の属性タイプを返す
int DiskBasicParam::GetTypeByIndexOfSpecialAttr(int idx) const
{
	return special_attrs.GetTypeByIndex(idx);
}
/// 特別な属性の属性値を返す
int DiskBasicParam::GetValueByIndexOfSpecialAttr(int idx) const
{
	return special_attrs.GetValueByIndex(idx);
}
/// 説明文でソート
int DiskBasicParam::SortByDescription(const DiskBasicParam **item1, const DiskBasicParam **item2)
{
	return (*item1)->GetBasicDescription().Cmp((*item2)->GetBasicDescription());
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicParams);

//////////////////////////////////////////////////////////////////////
//
//
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

WX_DEFINE_OBJARRAY(DiskBasicCategories);

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicTemplates::DiskBasicTemplates()
{
}
/// XMLファイル読み込み
bool DiskBasicTemplates::Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("basic_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "DiskBasics") return false;

	bool valid = true;
	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item && valid) {
		if (item->GetName() == "DiskBasicTypes") {
			valid = LoadTypes(item, locale_name, errmsgs);
		}
		else if (item->GetName() == "DiskBasicFormats") {
			valid = LoadFormats(item, locale_name, errmsgs);
		}
		else if (item->GetName() == "DiskBasicCategories") {
			valid = LoadCategories(item, locale_name, errmsgs);
		}
		item = item->GetNext();
	}
	return valid;
}

/// DiskBasicTypeタグのロード
bool DiskBasicTemplates::LoadTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	bool valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "DiskBasicType") {
			wxString type_name = item->GetAttribute("name");
			wxString type_category = item->GetAttribute("category");

			wxArrayInt reserved_groups;
			int format_type_number	 = -1;
			int sides_on_basic		 = 0;
			int sectors_on_basic	 = -1;
			int tracks_on_basic		 = -1;
			int sectors_per_group	 = 0;
			int managed_track_number = 0;
			int fat_start_sector	 = 0;
			int fat_end_sector		 = 0;
			int fat_side_number		 = -1;
			int fat_start_position	 = 0;
			int fat_end_group		 = 0;
			int dir_start_sector	 = -1;
			int dir_end_sector		 = -1;
			int dir_entry_count		 = -1;
			int subdir_group_size	 = 1;
			int dir_space_code		 = 0x20;
			int dir_start_pos		 = 0;
			int dir_start_pos_on_root = 0;
			int dir_start_pos_on_sec = 0;
			int groups_per_dir_entry = 0;
			int fill_code_on_format	 = 0;
			int fill_code_on_fat	 = 0;
			int fill_code_on_dir	 = 0;
			int delete_code_on_dir	 = 0;
			int reserved_sectors	 = -2;
			int number_of_fats		 = 1;
			int sectors_per_fat		 = 0;
			int media_id			 = 0;
			int group_final_code	 = 0;
			int group_system_code	 = 0;
			int group_unused_code	 = 0;
			int id_sector_pos		 = 0;
			wxString invalidate_chars;
			bool data_inverted		 = false;
			bool side_reversed		 = false;
			bool mount_each_sides	 = false;
			wxString desc, desc_locale;
			wxString id_string;
			wxString ipl_string;
			wxString volume_string;
			SpecialAttributes special_attrs;

			wxXmlNode *itemnode = item->GetChildren();
			while (itemnode) {
				if (itemnode->GetName() == "FormatType") {
					wxString str = itemnode->GetNodeContent();
					format_type_number = Utils::ToInt(str);
				} else if (itemnode->GetName() == "SectorsPerGroup") {
					wxString str = itemnode->GetNodeContent();
					sectors_per_group = Utils::ToInt(str);
				} else if (itemnode->GetName() == "SidesPerDisk") {
					wxString str = itemnode->GetNodeContent();
					sides_on_basic = Utils::ToInt(str);
				} else if (itemnode->GetName() == "SectorsPerTrack") {
					wxString str = itemnode->GetNodeContent();
					sectors_on_basic = Utils::ToInt(str);
				} else if (itemnode->GetName() == "TracksPerSide") {
					wxString str = itemnode->GetNodeContent();
					tracks_on_basic = Utils::ToInt(str);
				} else if (itemnode->GetName() == "ManagedTrackNumber") {
					wxString str = itemnode->GetNodeContent();
					managed_track_number = Utils::ToInt(str);
				} else if (itemnode->GetName() == "ReservedSectors") {
					wxString str = itemnode->GetNodeContent();
					reserved_sectors = Utils::ToInt(str);
				} else if (itemnode->GetName() == "NumberOfFATs") {
					wxString str = itemnode->GetNodeContent();
					number_of_fats = Utils::ToInt(str);
				} else if (itemnode->GetName() == "SectorsPerFAT") {
					wxString str = itemnode->GetNodeContent();
					sectors_per_fat = Utils::ToInt(str);
				} else if (itemnode->GetName() == "FATStartSector") {
					wxString str = itemnode->GetNodeContent();
					fat_start_sector = Utils::ToInt(str);
				} else if (itemnode->GetName() == "FATEndSector") {
					wxString str = itemnode->GetNodeContent();
					fat_end_sector = Utils::ToInt(str);
				} else if (itemnode->GetName() == "FATStartPosition") {
					wxString str = itemnode->GetNodeContent();
					fat_start_position = Utils::ToInt(str);
				} else if (itemnode->GetName() == "FATEndGroup") {
					wxString str = itemnode->GetNodeContent();
					fat_end_group = Utils::ToInt(str);
				} else if (itemnode->GetName() == "FATSideNumber") {
					wxString str = itemnode->GetNodeContent();
					fat_side_number = Utils::ToInt(str);
				} else if (itemnode->GetName() == "ReservedGroups") {
					LoadReservedGroupsInTypes(itemnode, locale_name, errmsgs, reserved_groups);
				} else if (itemnode->GetName() == "GroupFinalCode") {
					wxString str = itemnode->GetNodeContent();
					group_final_code = Utils::ToInt(str);
				} else if (itemnode->GetName() == "GroupSystemCode") {
					wxString str = itemnode->GetNodeContent();
					group_system_code = Utils::ToInt(str);
				} else if (itemnode->GetName() == "GroupUnusedCode") {
					wxString str = itemnode->GetNodeContent();
					group_unused_code = Utils::ToInt(str);
				} else if (itemnode->GetName() == "DirStartSector") {
					wxString str = itemnode->GetNodeContent();
					dir_start_sector = Utils::ToInt(str);
				} else if (itemnode->GetName() == "DirEndSector") {
					wxString str = itemnode->GetNodeContent();
					dir_end_sector = Utils::ToInt(str);
				} else if (itemnode->GetName() == "DirEntryCount") {
					wxString str = itemnode->GetNodeContent();
					dir_entry_count = Utils::ToInt(str);
				} else if (itemnode->GetName() == "DirSpaceCode") {
					wxString str = itemnode->GetNodeContent();
					dir_space_code = Utils::ToInt(str);
				} else if (itemnode->GetName() == "DirStartPosition") {
					wxString str = itemnode->GetNodeContent();
					dir_start_pos = Utils::ToInt(str);
				} else if (itemnode->GetName() == "DirStartPositionOnRoot") {
					wxString str = itemnode->GetNodeContent();
					dir_start_pos_on_root = Utils::ToInt(str);
				} else if (itemnode->GetName() == "DirStartPositionOnSector") {
					wxString str = itemnode->GetNodeContent();
					dir_start_pos_on_sec = Utils::ToInt(str);
				} else if (itemnode->GetName() == "GroupsPerDirEntry") {
					wxString str = itemnode->GetNodeContent();
					groups_per_dir_entry = Utils::ToInt(str);
				} else if (itemnode->GetName() == "SubDirGroupSize") {
					wxString str = itemnode->GetNodeContent();
					subdir_group_size = Utils::ToInt(str);
				} else if (itemnode->GetName() == "FillCodeOnFormat") {
					wxString str = itemnode->GetNodeContent();
					fill_code_on_format = Utils::ToInt(str);
				} else if (itemnode->GetName() == "FillCodeOnFAT") {
					wxString str = itemnode->GetNodeContent();
					fill_code_on_fat = Utils::ToInt(str);
				} else if (itemnode->GetName() == "FillCodeOnDir") {
					wxString str = itemnode->GetNodeContent();
					fill_code_on_dir = Utils::ToInt(str);
				} else if (itemnode->GetName() == "DeleteCodeOnDir") {
					wxString str = itemnode->GetNodeContent();
					delete_code_on_dir = Utils::ToInt(str);
				} else if (itemnode->GetName() == "MediaID") {
					wxString str = itemnode->GetNodeContent();
					media_id = Utils::ToInt(str);
				} else if (itemnode->GetName() == "InvalidateChars") {
					invalidate_chars = Utils::Escape(itemnode->GetNodeContent());
				} else if (itemnode->GetName() == "DataInverted") {
					wxString str = itemnode->GetNodeContent();
					data_inverted = Utils::ToBool(str);
				} else if (itemnode->GetName() == "SideReversed") {
					wxString str = itemnode->GetNodeContent();
					side_reversed = Utils::ToBool(str);
				} else if (itemnode->GetName() == "CanMountEachSides") {
					wxString str = itemnode->GetNodeContent();
					mount_each_sides = Utils::ToBool(str);
				} else if (itemnode->GetName() == "IDSectorPosition") {
					wxString str = itemnode->GetNodeContent();
					id_sector_pos = Utils::ToInt(str);
				} else if (itemnode->GetName() == "IDString") {
					id_string = Utils::Escape(itemnode->GetNodeContent());
				} else if (itemnode->GetName() == "IPLString") {
					ipl_string = Utils::Escape(itemnode->GetNodeContent());
				} else if (itemnode->GetName() == "VolumeString") {
					volume_string = Utils::Escape(itemnode->GetNodeContent());
				} else if (itemnode->GetName() == "SpecialAttributes") {
					LoadSpecialAttributesInTypes(itemnode, locale_name, errmsgs, special_attrs);
				} else if (itemnode->GetName() == "Description") {
					LoadDescription(itemnode, locale_name, desc, desc_locale);
				}
				itemnode = itemnode->GetNext();
			}
			if (fat_start_sector > 0 && reserved_sectors <= 0) {
				reserved_sectors = fat_start_sector - 1;
			}
			if (fat_end_sector > 0 && sectors_per_fat <= 0) {
				sectors_per_fat = fat_end_sector - fat_start_sector + 1;
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			const DiskBasicFormat *format_type = FindFormat((DiskBasicFormatType)format_type_number);
			DiskBasicParam p(
				type_name,
				type_category,
				format_type,
				sectors_per_group,
				sides_on_basic,
				sectors_on_basic,
				tracks_on_basic,
				managed_track_number,
				reserved_sectors,
				number_of_fats,
				sectors_per_fat,
				fat_start_position,
				fat_end_group,
				fat_side_number,
				reserved_groups,
				group_final_code,
				group_system_code,
				group_unused_code,
				dir_start_sector,
				dir_end_sector,
				dir_entry_count,
				subdir_group_size,
				dir_space_code,
				dir_start_pos,
				dir_start_pos_on_root,
				dir_start_pos_on_sec,
				groups_per_dir_entry,
				id_sector_pos,
				id_string,
				ipl_string,
				volume_string,
				special_attrs,
				fill_code_on_format,
				fill_code_on_fat,
				fill_code_on_dir,
				delete_code_on_dir,
				media_id,
				invalidate_chars,
				data_inverted,
				side_reversed,
				mount_each_sides,
				desc
			);
			if (FindType(wxEmptyString, type_name) == NULL) {
				types.Add(p);
			} else {
				// タイプ名が重複している
				errmsgs += wxT("\n");
				errmsgs += _("Duplicate type name in DiskBasicType : ");
				errmsgs += type_name;
				return false;
			}
		}
		item = item->GetNext();
	}
	return valid;
}

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

bool DiskBasicTemplates::LoadReservedGroupsInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs, wxArrayInt &reserved_groups)
{
	wxXmlNode *citemnode = node->GetChildren();
	while(citemnode) {
		if (citemnode->GetName() == "Group") {
			wxString str = citemnode->GetNodeContent();
			int reserved_group = Utils::ToInt(str);
			reserved_groups.Add(reserved_group);
		}
		citemnode = citemnode->GetNext();
	}
	return true;
}

static const struct st_special_attr_names {
	const char *name;
	int type;
} cSpecialAttrNames[] = {
	{ "MachineBinary",	FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK },
	{ "BasicBinary",	FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK },
	{ "BasicAscii",		FILE_TYPE_BASIC_MASK | FILE_TYPE_ASCII_MASK },
	{ "Binary",			FILE_TYPE_BINARY_MASK },
	{ "Ascii",			FILE_TYPE_ASCII_MASK },
	{ "Volume",			FILE_TYPE_VOLUME_MASK },
	{ NULL, 0 }
};

bool DiskBasicTemplates::LoadSpecialAttributesInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs, SpecialAttributes &special_attrs)
{
	wxXmlNode *citemnode = node->GetChildren();
	while(citemnode) {
		for(int i=0; cSpecialAttrNames[i].name != NULL; i++) {
			if (citemnode->GetName() == cSpecialAttrNames[i].name) {
				LoadSpecialAttribute(citemnode, locale_name, cSpecialAttrNames[i].type, special_attrs);
				break;
			}
		}
		citemnode = citemnode->GetNext();
	}
	return true;
}

bool DiskBasicTemplates::LoadSpecialAttribute(const wxXmlNode *node, const wxString &locale_name, int type, SpecialAttributes &special_attrs)
{
	wxString desc, desc_locale;
	wxString sval = node->GetAttribute("value");
	wxString smsk = node->GetAttribute("mask");
	int val = Utils::ToInt(sval);
	int msk = Utils::ToInt(smsk);
	special_attrs.Add(SpecialAttribute((int)special_attrs.Count(), type, val, msk));
	return true;
}

/// DiskBasicFormatタグのロード
bool DiskBasicTemplates::LoadFormats(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	bool valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "DiskBasicFormat") {
			wxString s_type_number = item->GetAttribute("type");
			int type_number = Utils::ToInt(s_type_number);
//			wxString name = item->GetAttribute("name");
			wxString str;
			bool has_volume_name = false;
			bool has_volume_number = false;

			wxXmlNode *itemnode = item->GetChildren();
			while (itemnode) {
				if (itemnode->GetName() == "HasVolumeName") {
					str = itemnode->GetNodeContent();
					has_volume_name = Utils::ToBool(str);
				} else if (itemnode->GetName() == "HasVolumeNumber") {
					str = itemnode->GetNodeContent();
					has_volume_number = Utils::ToBool(str);
				}
				itemnode = itemnode->GetNext();
			}
			DiskBasicFormat f(
				type_number,
				has_volume_name,
				has_volume_number
			);
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

/// DiskBasicCategoryタグのロード
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
const DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const wxArrayString &n_basic_types) const
{
	const DiskBasicParam *match_item = NULL;
	for(size_t i=0; i<n_basic_types.Count() && match_item == NULL; i++) {
		match_item = this->FindType(n_category, n_basic_types.Item(i));
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
size_t DiskBasicTemplates::FindParams(const wxArrayString &n_type_names, DiskBasicParamPtrs &params) const
{
	for(size_t n = 0; n < n_type_names.Count(); n++) {
		const DiskBasicParam *param = FindType(wxEmptyString, n_type_names.Item(n));
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
