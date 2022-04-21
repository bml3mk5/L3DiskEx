/// @file basicparam.cpp
///
/// @brief disk basic parameter
///
#include "basicparam.h"
#include "utils.h"
#include <wx/xml/xml.h>

DiskBasicTemplates gDiskBasicTemplates;

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
	DiskBasicFormatType	n_format_type,
	int					n_sectors_per_group,
	int					n_sides_on_basic,
	int					n_sectors_on_basic,
	int					n_managed_track_number,
	int					n_reserved_sectors,
	int					n_number_of_fats,
	int					n_sectors_per_fat,
	int					n_fat_start_pos,
	int					n_fat_end_group,
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
	int					n_dir_start_pos_on_sec,
	int					n_groups_per_dir_entry,
	int					n_id_sector_pos,
	const wxString &	n_id_string,
	const wxString &	n_ipl_string,
	const wxString &	n_volume_string,
	wxUint8				n_fillcode_on_format,
	wxUint8				n_fillcode_on_fat,
	wxUint8				n_fillcode_on_dir,
	wxUint8				n_delete_code,
	wxUint8				n_media_id,
	const wxString &	n_basic_description
) {
	basic_type_name = n_basic_type_name;
	basic_category_name = n_basic_category_name;
	format_type = n_format_type;
	sectors_per_group = n_sectors_per_group;
	sides_on_basic = n_sides_on_basic;
	sectors_on_basic = n_sectors_on_basic;
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
	dir_start_pos_on_sec = n_dir_start_pos_on_sec;
	groups_per_dir_entry = n_groups_per_dir_entry;
	id_sector_pos = n_id_sector_pos;
	id_string = n_id_string;
	ipl_string = n_ipl_string;
	volume_string = n_volume_string;
	fillcode_on_format = n_fillcode_on_format;
	fillcode_on_fat = n_fillcode_on_fat;
	fillcode_on_dir = n_fillcode_on_dir;
	delete_code = n_delete_code;
	media_id = n_media_id;
	basic_description = n_basic_description;
}
void DiskBasicParam::ClearBasicParam()
{
	basic_type_name.Empty();
	basic_category_name.Empty();
	format_type			 = FORMAT_TYPE_NONE;
	sectors_per_group	 = 0;
	sides_on_basic		 = 0;
	sectors_on_basic	 = -1;
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
	dir_start_pos_on_sec = 0;
	groups_per_dir_entry = 0;
	id_sector_pos		 = 0;
	id_string.Empty();
	ipl_string.Empty();
	volume_string.Empty();
	fillcode_on_format	 = 0x40;
	fillcode_on_fat		 = 0xff;
	fillcode_on_dir		 = 0xff;
	delete_code			 = 0x00;
	media_id			 = 0x00;
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
	dir_start_pos_on_sec = src.dir_start_pos_on_sec;
	groups_per_dir_entry = src.groups_per_dir_entry;
	id_sector_pos = src.id_sector_pos;
	id_string = src.id_string;
	ipl_string = src.ipl_string;
	volume_string = src.volume_string;
	fillcode_on_format = src.fillcode_on_format;
	fillcode_on_fat = src.fillcode_on_fat;
	fillcode_on_dir = src.fillcode_on_dir;
	delete_code = src.delete_code;
	media_id = src.media_id;
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
#if 0
int DiskBasicParam::GetDirStartSector(int sector_size)
{
	if (dir_start_sector < 0) {
		dir_start_sector = reserved_sectors + number_of_fats * sectors_per_fat + 1;
		dir_end_sector = dir_start_sector + dir_entry_count * 32 / sector_size - 1;
	}
	return dir_start_sector;
}
int DiskBasicParam::GetDirEndSector(int sector_size)
{
	if (dir_end_sector < 0) {
		dir_start_sector = reserved_sectors + number_of_fats * sectors_per_fat + 1;
		dir_end_sector = dir_start_sector + dir_entry_count * 32 / sector_size - 1;
	}
	return dir_end_sector;
}
int DiskBasicParam::GetDirEntryCount(int sector_size)
{
	if (dir_entry_count < 0) {
		dir_entry_count = (dir_end_sector - dir_start_sector + 1) * sector_size / 32;
	}
	return dir_entry_count;
}
#endif

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicParams);

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

//
//
//
DiskBasicTemplates::DiskBasicTemplates()
{
}
/// XMLファイル読み込み
bool DiskBasicTemplates::Load(const wxString &data_path, const wxString &locale_name)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("basic_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "DiskBasicTypes") return false;

	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item) {
		if (item->GetName() == "DiskBasicType") {
			wxString type_name = item->GetAttribute("name");
			wxString type_category = item->GetAttribute("category");
			wxXmlNode *itemnode = item->GetChildren();
			wxArrayInt reserved_groups;
			int format_type			 = 0;
			int sides_on_basic		 = 0;
			int sectors_on_basic	 = -1;
			int sectors_per_group	 = 0;
			int managed_track_number = 0;
			int fat_start_sector	 = 0;
			int fat_end_sector		 = 0;
			int fat_side_number		 = -1;
			int fat_start_position	 = 0;
			int fat_end_group		 = 0;
			int reserved_group		 = 0;
			int dir_start_sector	 = -1;
			int dir_end_sector		 = -1;
			int dir_entry_count		 = -1;
			int subdir_group_size	 = 1;
			int dir_space_code		 = 0x20;
			int dir_start_pos_on_sec = 0;
			int groups_per_dir_entry = 0;
			int fill_code_on_format	 = 0;
			int fill_code_on_fat	 = 0;
			int fill_code_on_dir	 = 0;
			int delete_code_on_dir	 = 0;
			int reserved_sectors	 = 0;
			int number_of_fats		 = 1;
			int sectors_per_fat		 = 0;
			int media_id			 = 0;
			int group_final_code	 = 0;
			int group_system_code	 = 0;
			int group_unused_code	 = 0;
			int id_sector_pos		 = 0;
			wxString desc, desc_locale;
			wxString id_string, ipl_string, volume_string;

			while (itemnode) {
				if (itemnode->GetName() == "FormatType") {
					wxString str = itemnode->GetNodeContent();
					format_type = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "SectorsPerGroup") {
					wxString str = itemnode->GetNodeContent();
					sectors_per_group = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "SidesPerDisk") {
					wxString str = itemnode->GetNodeContent();
					sides_on_basic = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "SectorsPerTrack") {
					wxString str = itemnode->GetNodeContent();
					sectors_on_basic = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "ManagedTrackNumber") {
					wxString str = itemnode->GetNodeContent();
					managed_track_number = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "ReservedSectors") {
					wxString str = itemnode->GetNodeContent();
					reserved_sectors = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "NumberOfFATs") {
					wxString str = itemnode->GetNodeContent();
					number_of_fats = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "SectorsPerFAT") {
					wxString str = itemnode->GetNodeContent();
					sectors_per_fat = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "FATStartSector") {
					wxString str = itemnode->GetNodeContent();
					fat_start_sector = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "FATEndSector") {
					wxString str = itemnode->GetNodeContent();
					fat_end_sector = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "FATStartPosition") {
					wxString str = itemnode->GetNodeContent();
					fat_start_position = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "FATEndGroup") {
					wxString str = itemnode->GetNodeContent();
					fat_end_group = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "FATSideNumber") {
					wxString str = itemnode->GetNodeContent();
					fat_side_number = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "ReservedGroups") {
					wxXmlNode *citemnode = itemnode->GetChildren();
					while(citemnode) {
						if (citemnode->GetName() == "Group") {
							wxString str = citemnode->GetNodeContent();
							reserved_group = L3DiskUtils::ToInt(str);
							reserved_groups.Add((int)reserved_group);
						}
						citemnode = citemnode->GetNext();
					}
				} else if (itemnode->GetName() == "GroupFinalCode") {
					wxString str = itemnode->GetNodeContent();
					group_final_code = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "GroupSystemCode") {
					wxString str = itemnode->GetNodeContent();
					group_system_code = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "GroupUnusedCode") {
					wxString str = itemnode->GetNodeContent();
					group_unused_code = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "DirStartSector") {
					wxString str = itemnode->GetNodeContent();
					dir_start_sector = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "DirEndSector") {
					wxString str = itemnode->GetNodeContent();
					dir_end_sector = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "DirEntryCount") {
					wxString str = itemnode->GetNodeContent();
					dir_entry_count = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "DirSpaceCode") {
					wxString str = itemnode->GetNodeContent();
					dir_space_code = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "DirStartPositionOnSector") {
					wxString str = itemnode->GetNodeContent();
					dir_start_pos_on_sec = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "GroupsPerDirEntry") {
					wxString str = itemnode->GetNodeContent();
					groups_per_dir_entry = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "SubDirGroupSize") {
					wxString str = itemnode->GetNodeContent();
					subdir_group_size = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "FillCodeOnFormat") {
					wxString str = itemnode->GetNodeContent();
					fill_code_on_format = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "FillCodeOnFAT") {
					wxString str = itemnode->GetNodeContent();
					fill_code_on_fat = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "FillCodeOnDir") {
					wxString str = itemnode->GetNodeContent();
					fill_code_on_dir = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "DeleteCodeOnDir") {
					wxString str = itemnode->GetNodeContent();
					delete_code_on_dir = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "MediaID") {
					wxString str = itemnode->GetNodeContent();
					media_id = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "IDSectorPosition") {
					wxString str = itemnode->GetNodeContent();
					id_sector_pos = L3DiskUtils::ToInt(str);
				} else if (itemnode->GetName() == "IDString") {
					id_string = L3DiskUtils::Escape(itemnode->GetNodeContent());
				} else if (itemnode->GetName() == "IPLString") {
					ipl_string = L3DiskUtils::Escape(itemnode->GetNodeContent());
				} else if (itemnode->GetName() == "VolumeString") {
					volume_string = L3DiskUtils::Escape(itemnode->GetNodeContent());
				} else if (itemnode->GetName() == "Description") {
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
			if (fat_start_sector > 0 && reserved_sectors <= 0) {
				reserved_sectors = fat_start_sector - 1;
			}
			if (fat_end_sector > 0 && sectors_per_fat <= 0) {
				sectors_per_fat = fat_end_sector - fat_start_sector + 1;
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			DiskBasicParam p(
				type_name,
				type_category,
				(DiskBasicFormatType)format_type,
				sectors_per_group,
				sides_on_basic,
				sectors_on_basic,
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
				dir_start_pos_on_sec,
				groups_per_dir_entry,
				id_sector_pos,
				id_string,
				ipl_string,
				volume_string,
				fill_code_on_format,
				fill_code_on_fat,
				fill_code_on_dir,
				delete_code_on_dir,
				media_id,
				desc
			);
			types.Add(p);
		}
		else if (item->GetName() == "DiskBasicCategory") {
			wxString type_name = item->GetAttribute("name");
			wxXmlNode *itemnode = item->GetChildren();
			wxString desc, desc_locale;
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
			DiskBasicCategory c(type_name, desc);
			categories.Add(c);
		}
		item = item->GetNext();
	}
	return true;
}

/// カテゴリとタイプに一致するパラメータを検索
/// @param [in] n_category  : カテゴリ名 空文字列の場合は検索条件からはずす
/// @param [in] n_basic_type: タイプ名
/// @return 一致したパラメータ
DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const wxString &n_basic_type)
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
DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, const wxArrayString &n_basic_types)
{
	DiskBasicParam *match_item = NULL;
	for(size_t i=0; i<n_basic_types.Count() && match_item == NULL; i++) {
		match_item = this->FindType(n_category, n_basic_types.Item(i));
	}
	return match_item;
}
/// カテゴリ、サイド数とセクタ数が一致するパラメータを検索
/// @param [in] n_category   : カテゴリ名 必須
/// @param [in] n_sides      : サイド数
/// @param [in] n_sectors    : セクタ数/トラック -1の場合は検索条件からはずす
/// @return 一致したパラメータ
DiskBasicParam *DiskBasicTemplates::FindType(const wxString &n_category, int n_sides, int n_sectors)
{
	DiskBasicParam *match_item = NULL;
	for(size_t i=0; i<types.Count(); i++) {
		DiskBasicParam *item = &types[i];
		if (n_category == item->GetBasicCategoryName()) {
			if (n_sides == item->GetSidesOnBasic()) {
				if (n_sectors < 0 || item->GetSectorsOnBasic() < 0 || n_sectors == item->GetSectorsOnBasic()) {
					match_item = item;
					break;
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
size_t DiskBasicTemplates::FindTypes(const wxArrayInt &n_format_types, DiskBasicParams &n_types)
{
	n_types.Clear();
	for(size_t n=0; n<types.Count(); n++) {
		const DiskBasicParam *item = &types[n];
		for(size_t i=0; i<n_format_types.Count(); i++) {
			if (n_format_types.Item(i) == item->GetFormatType()) {
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
size_t DiskBasicTemplates::FindTypeNames(size_t n_category_index, wxArrayString &n_type_names)
{
	return FindTypeNames(categories.Item(n_category_index).GetBasicTypeName(), n_type_names);
}
/// カテゴリ名に一致するタイプ名リストを検索
/// @param [in]  n_category_name  : カテゴリ名
/// @param [out] n_type_names     : タイプ名リスト
/// @return リストの数
size_t DiskBasicTemplates::FindTypeNames(const wxString &n_category_name, wxArrayString &n_type_names)
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
