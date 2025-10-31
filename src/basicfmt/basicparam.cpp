/// @file basicparam.cpp
///
/// @brief disk basic parameter
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicparam.h"
#include "../utils.h"
#include <wx/xml/xml.h>


//////////////////////////////////////////////////////////////////////
//
// DISK BASICの基本パラメータ
//
DiskBasicParamBase::DiskBasicParamBase()
{
	ClearBasicParamBase();
}

/// @brief 初期化
void DiskBasicParamBase::ClearBasicParamBase()
{
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
	special_attrs.Empty();
	attrs_by_extension.Empty();
	fillcode_on_format	 = 0;
	fillcode_on_fat		 = 0;
	fillcode_on_dir		 = 0;
	delete_code			 = 0;
	text_terminate_code	 = 0x1a;
	extension_pre_code	 = 0x2e; // '.'
	valid_file_name.Empty();
	valid_volume_name.Empty();
	compare_case_insense = false;
	to_upper_before_dialog = false;
	to_upper_after_renamed = false;
	big_endian			 = false;
	various_params.clear();
}

/// @brief 設定
void DiskBasicParamBase::SetBasicParamBase(const DiskBasicParamBase &src)
{
	*this = src;
}

/// 固有のパラメータ
void DiskBasicParamBase::GetVariousParam(const wxString &key, wxVariant &val) const
{
	VariantHash::const_iterator invalid = various_params.end();
	VariantHash::const_iterator it = various_params.find(key);
	if (it != invalid) {
		val = it->second;
	}
}

/// 固有のパラメータ
int DiskBasicParamBase::GetVariousIntegerParam(const wxString &key) const
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
bool DiskBasicParamBase::GetVariousBoolParam(const wxString &key) const
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
wxString DiskBasicParamBase::GetVariousStringParam(const wxString &key) const
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
void DiskBasicParamBase::SetVariousParam(const wxString &key, const wxVariant &val)
{
	various_params[key] = val;
}

//////////////////////////////////////////////////////////////////////

DiskBasicParamBases::DiskBasicParamBases()
	: TemplatesBase()
{
	m_set_volume_rule = false;
}

/// 共通パラメータ関連のロード
bool DiskBasicParamBases::Load(const wxXmlNode *node, const wxString &name, const wxString &value, const wxString &locale_name, DiskBasicParamBase &param, wxString &errmsgs)
{
	bool valid = true;
	if (name == "SectorsPerGroup") {
		param.SetSectorsPerGroup(Utils::ToInt(value));
	} else if (name == "GroupFinalCode") {
		param.SetGroupFinalCode(Utils::ToInt(value));
	} else if (name == "GroupSystemCode") {
		param.SetGroupSystemCode(Utils::ToInt(value));
	} else if (name == "GroupUnusedCode") {
		param.SetGroupUnusedCode(Utils::ToInt(value));
	} else if (name == "DirTerminateCode") {
		param.SetDirTerminateCode(Utils::ToInt(value));
	} else if (name == "DirSpaceCode") {
		param.SetDirSpaceCode(Utils::ToInt(value));
	} else if (name == "DirTrimmingCode") {
		param.SetDirTrimmingCode(Utils::ToInt(value));
	} else if (name == "DirStartPosition") {
		param.SetDirStartPos(Utils::ToInt(value));
	} else if (name == "DirStartPositionOnRoot") {
		param.SetDirStartPosOnRoot(Utils::ToInt(value));
	} else if (name == "DirStartPositionOnSector") {
		param.SetDirStartPosOnSector(Utils::ToInt(value));
	} else if (name == "DirStartPositionOnGroup") {
		param.SetDirStartPosOnGroup(Utils::ToInt(value));
	} else if (name == "SpecialAttributes") {
		MyAttributes attrs;
		LoadMyAttributesInTypes(node, locale_name, errmsgs, attrs);
		param.SetSpecialAttributes(attrs);
	} else if (name == "AttributesByExtension") {
		MyAttributes attrs;
		LoadMyAttributesInTypes(node, locale_name, errmsgs, attrs);
		param.SetAttributesByExtension(attrs);
	} else if (name == "FillCodeOnFormat") {
		param.SetFillCodeOnFormat(Utils::ToInt(value));
	} else if (name == "FillCodeOnFAT") {
		param.SetFillCodeOnFAT(Utils::ToInt(value));
	} else if (name == "FillCodeOnDir") {
		param.SetFillCodeOnDir(Utils::ToInt(value));
	} else if (name == "DeleteCodeOnDir") {
		param.SetDeleteCode(Utils::ToInt(value));
	} else if (name == "TextTerminateCode") {
		param.SetTextTerminateCode(Utils::ToInt(value));
	} else if (name == "ExtensionPreCode") {
		param.SetExtensionPreCode(Utils::ToInt(value));
	} else if (name == "FileNameCharacters") {
		valid = LoadValidChars(node, param.GetValidFileName(), errmsgs);
		if (!m_set_volume_rule) {
			param.SetValidVolumeName(param.GetValidFileName());
		}
	} else if (name == "VolumeNameCharacters") {
		valid = LoadValidChars(node, param.GetValidVolumeName(), errmsgs);
		m_set_volume_rule = valid;
	} else if (name == "FileNameCompareCase") {
		bool val;
		LoadFileNameCompareCase(node, val);
		param.CompareCaseInsense(val);
	} else if (name == "ToUpperFileNameBeforeDialog") {
		param.ToUpperBeforeDialog(Utils::ToBool(value));
	} else if (name == "ToUpperFileNameAfterRenamed") {
		param.ToUpperAfterRenamed(Utils::ToBool(value));
	} else if (name == "RequireFileName") {
		param.GetValidFileName().RequireName(Utils::ToBool(value));
	} else if (name == "RequireVolumeName") {
		param.GetValidVolumeName().RequireName(Utils::ToBool(value));
	} else if (name == "VolumeNameMaxLength") {
		param.GetValidVolumeName().SetMaxLength(Utils::ToInt(value));
	} else if (name == "Endian") {
		param.BigEndian(value.Upper() == "BIG");
	} else if (!name.IsEmpty()) {
		wxVariant nval;
		param.GetVariousParam(name, nval);
		LoadVariousParam(node, value, nval);
		param.SetVariousParam(name, nval);
	}
	return valid;
}

//////////////////////////////////////////////////////////////////////
//
// DISK BASICのフォーマットタイプ
//
DiskBasicFormat::DiskBasicFormat()
	: DiskBasicParamBase()
{
	ClearBasicFormatPrivate();
}

/// 初期化
void DiskBasicFormat::ClearBasicFormatPrivate()
{
	type_number			 = FORMAT_TYPE_UNKNOWN;
	has_volume_name		 = false;
	has_volume_number	 = false;
	has_volume_date		 = false;
}

/// 初期化
void DiskBasicFormat::ClearBasicFormat()
{
	DiskBasicParamBase();
	ClearBasicFormatPrivate();
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfDiskBasicFormat);

//////////////////////////////////////////////////////////////////////

/// DiskBasicFormatエレメントのロード
/// @param[in] node        ノード
/// @param[in] locale_name ローケル名
/// @param[out] errmsgs    エラー時メッセージ
/// @return true / false
bool DiskBasicFormats::Load(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	bool valid = false;
	while (node && !valid) {
		if (node->GetName() == "DiskBasicFormats") {
			valid = true;
			break;
		}
		node = node->GetNext();
	}
	if (!valid) return false;

	valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "DiskBasicFormat") {
			DiskBasicFormat f;
			DiskBasicParamBases param_bases;
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
				} else {
					bool rc = param_bases.Load(itemnode, name, str, locale_name, f, errmsgs);
					valid = (valid && rc);
				}
				itemnode = itemnode->GetNext();
			}

			if (Find((DiskBasicFormatType)type_number) == NULL) {
				Add(f);
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

/// フォーマット種類を検索
/// @param [in]  format_type  : フォーマット種類
const DiskBasicFormat *DiskBasicFormats::Find(DiskBasicFormatType format_type) const
{
	DiskBasicFormat *match = NULL;
	for(size_t n=0; n<Count(); n++) {
		DiskBasicFormat *item = &Item(n);
		if (item->GetTypeNumber() == format_type) {
			match = item;
			break;
		}
	}
	return match;
}

//////////////////////////////////////////////////////////////////////
//
// DISK BASICのパラメータを保持するクラス
//
DiskBasicParam::DiskBasicParam()
	: DiskBasicParamBase()
{
	ClearBasicParamPrivate();
}

/// 初期化
void DiskBasicParam::ClearBasicParamPrivate()
{
	basic_type_name.Empty();
	basic_category_names.Empty();
	format_type			 = NULL;
	format_subtype_number = 0;
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
	dir_start_sector	 = -1;
	dir_end_sector		 = -1;
	dir_entry_count		 = -1;
	subdir_group_size	 = 1;
	group_width			 = 1;
	groups_per_dir_entry = 0;
	valid_density_type   = -1;
	sector_skew.Set(1);
	media_id			 = 0x00;
	data_inverted		 = false;
	side_reversed		 = false;
	mount_each_sides	 = false;
	basic_description.Empty();
}

/// 初期化
void DiskBasicParam::ClearBasicParam()
{
	ClearBasicParamBase();
	ClearBasicParamPrivate();
}

/// 設定
/// @param[in] src 元
void DiskBasicParam::SetBasicParam(const DiskBasicParam &src)
{
	*this = src;
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

/// BASICカテゴリ名を追加
void DiskBasicParam::AddBasicCategoryName(const wxString &str)
{
	if (basic_category_names.Index(str) == wxNOT_FOUND) {
		basic_category_names.Add(str);
	}
}

/// BASICカテゴリ名を返す
wxString DiskBasicParam::GetBasicCategoryName() const
{
	if (basic_category_names.IsEmpty()) {
		return wxEmptyString;
	} else {
		return basic_category_names.Item(0);
	}
}

/// BASICカテゴリ名が存在するか
bool DiskBasicParam::FindBasicCategoryName(const wxString &str) const
{
	return (basic_category_names.Index(str) != wxNOT_FOUND);
}

/// ReservedGroupsエレメントをロード
/// @param[in]  node        子ノード
/// @param[in]  locale_name ローケル名
/// @param[out] errmsgs     エラーメッセージ
/// @return true
bool DiskBasicParam::LoadReservedGroupsInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	// reserved_groups 予約済みグループ
	wxArrayInt reserved_groups;

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

/// SectorSkewMapエレメントをロード
/// @param[in] node 子ノード
/// @return true
bool DiskBasicParam::LoadSectorSkewMap(const wxXmlNode *node)
{
	wxArrayInt map;
	wxXmlNode *cnode = node->GetChildren();
	while(cnode) {
		wxString name = cnode->GetName();
		if (name == "Value") {
			map.Add(Utils::ToInt(cnode->GetNodeContent()));
		}
		cnode = cnode->GetNext();
	}

	sector_skew.Set(map);
	return true;
}

/// SectorsPerTrackエレメントをロード
/// @param[in] node        子ノード
/// @param[in] val         入力値
/// @return true
bool DiskBasicParam::LoadNumSectorsMap(const wxXmlNode *node, const wxString &val)
{
	// sec_param  セクタ数/トラック(全トラック同じの場合)
	int sec_param = GetSectorsPerTrackOnBasic();
	// sec_params セクタ数/トラック(トラック毎に異なる場合)
	NumSectorsParams *sec_params = &SectorsPerTrackOnBasicList();

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
		sec_params->Add(NumSectorsParam(start_track, num_of_tracks, sec_per_trk));
	}

	SetSectorsPerTrackOnBasic(sec_param);
	return true;
}

/// Categoriesエレメントのロード
/// @param[in] node        ノード
/// @param[in] locale_name ローケル名
/// @param[out] errmsgs    エラー時メッセージ
/// @return true / false
bool DiskBasicParam::LoadCategories(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs)
{
	bool valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "Category") {
			wxXmlNode *itemnode = item->GetChildren();
			while(itemnode) {
				wxString name = item->GetNodeContent();
				if (!name.IsEmpty() && basic_category_names.Index(name) == wxNOT_FOUND) {
					basic_category_names.Add(name);
				}
				itemnode = itemnode->GetNext();
			}
		}
		item = item->GetNext();
	}
	return valid;
}

/// 説明文でソート
int DiskBasicParam::SortByDescription(const DiskBasicParam **item1, const DiskBasicParam **item2)
{
	return (*item1)->GetBasicDescription().Cmp((*item2)->GetBasicDescription());
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfDiskBasicParam);

//////////////////////////////////////////////////////////////////////

/// DiskBasicTypeエレメントのロード
/// @param[in] node        ノード
/// @param[in] locale_name ローケル名
/// @param[in] formats     フォーマットパラメータ
/// @param[out] errmsgs    エラー時メッセージ
/// @return true / false
bool DiskBasicParams::Load(const wxXmlNode *node, const wxString &locale_name, const DiskBasicFormats &formats, wxString &errmsgs)
{
	bool valid = false;
	while (node && !valid) {
		if (node->GetName() == "DiskBasicTypes") {
			valid = true;
			break;
		}
		node = node->GetNext();
	}
	if (!valid) return false;

	valid = true;
	wxXmlNode *item = node->GetChildren();
	while(item && valid) {
		if (item->GetName() == "DiskBasicType") {
			DiskBasicParam p;
			DiskBasicParamBases param_bases;

			wxString type_name = item->GetAttribute("name");
			p.SetBasicTypeName(type_name);

			wxString format_name = item->GetAttribute("type");
			const DiskBasicFormat *format_type = formats.Find((DiskBasicFormatType)Utils::ToInt(format_name));
			if (!format_name.IsEmpty() && format_type) {
				// フォーマットパラメータを初期値とする
				p.SetFormatType(format_type);
//				p.RequireFileName(format_type->IsFileNameRequired());
				p.SetBasicParamBase(*format_type);
			} else {
				// フォーマットタイプがない
				errmsgs += wxT("\n");
				errmsgs += _("Unknown format type in DiskBasicType : ");
				errmsgs += type_name;
				return false;
			}

			p.AddBasicCategoryName(item->GetAttribute("category"));

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
				} else if (name == "SidesPerDisk") {
					p.SetSidesPerDiskOnBasic(Utils::ToInt(str));
				} else if (name == "SectorsPerTrack") {
					p.LoadNumSectorsMap(itemnode, str);
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
					p.LoadReservedGroupsInTypes(itemnode, locale_name, errmsgs);
				} else if (name == "DirStartSector") {
					p.SetDirStartSector(Utils::ToInt(str));
				} else if (name == "DirEndSector") {
					p.SetDirEndSector(Utils::ToInt(str));
				} else if (name == "DirEntryCount") {
					p.SetDirEntryCount(Utils::ToInt(str));
				} else if (name == "GroupWidth") {
					p.SetGroupWidth(Utils::ToInt(str));
				} else if (name == "GroupsPerDirEntry") {
					p.SetGroupsPerDirEntry(Utils::ToInt(str));
				} else if (name == "ValidDensityType") {
					p.SetValidDensityType(Utils::ToInt(str));
				} else if (name == "SectorSkew") {
					p.SetSectorSkew(Utils::ToInt(str));
				} else if (name == "SectorSkewMap") {
					p.LoadSectorSkewMap(itemnode);
				} else if (name == "SubDirGroupSize") {
					p.SetSubDirGroupSize(Utils::ToInt(str));
				} else if (name == "MediaID") {
					p.SetMediaId(Utils::ToInt(str));
				} else if (name == "DataInverted") {
					p.DataInverted(Utils::ToBool(str));
				} else if (name == "SideReversed") {
					p.SideReversed(Utils::ToBool(str));
				} else if (name == "CanMountEachSides") {
					p.MountEachSides(Utils::ToBool(str));
				} else if (name == "Description") {
					LoadDescription(itemnode, locale_name, desc, desc_locale);
				} else if (name == "Categories") {
					p.LoadCategories(itemnode, locale_name, errmsgs);
				} else {
					bool rc = param_bases.Load(itemnode, name, str, locale_name, p, errmsgs);
					valid = (valid && rc);
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

			if (Find(wxEmptyString, type_name) == NULL) {
				Add(p);
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

/// カテゴリとタイプに一致するパラメータを検索
/// @param [in] n_category  : カテゴリ名 空文字列の場合は検索条件からはずす
/// @param [in] n_basic_type: タイプ名
/// @return 一致したパラメータ
const DiskBasicParam *DiskBasicParams::Find(const wxString &n_category, const wxString &n_basic_type) const
{
	DiskBasicParam *match_item = NULL;
	for(size_t i=0; i<Count(); i++) {
		DiskBasicParam *item = &Item(i);
		if (n_category.IsEmpty() || item->FindBasicCategoryName(n_category)) {
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
const DiskBasicParam *DiskBasicParams::Find(const wxString &n_category, const DiskParamNames &n_basic_types) const
{
	const DiskBasicParam *match_item = NULL;
	for(size_t i=0; i<n_basic_types.Count() && match_item == NULL; i++) {
		match_item = Find(n_category, n_basic_types.Item(i).GetName());
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
const DiskBasicParam *DiskBasicParams::Find(const wxString &n_category, const wxString &n_basic_type, int n_sides, int n_sectors) const
{
	const DiskBasicParam *match_item = NULL;
	// カテゴリ、タイプで一致するか
	match_item = Find(n_category, n_basic_type);
	// カテゴリ、サイド数、セクタ数で一致するか
	if (!match_item) {
		for(size_t i=0; i<Count(); i++) {
			DiskBasicParam *item = &Item(i);
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
size_t DiskBasicParams::FindTypes(const wxArrayInt &n_format_types, DiskBasicParams &n_types) const
{
	n_types.Clear();
	for(size_t n=0; n<Count(); n++) {
		const DiskBasicParam *item = &Item(n);
		for(size_t i=0; i<n_format_types.Count(); i++) {
			const DiskBasicFormat *fmt = item->GetFormatType();
			if (fmt && n_format_types.Item(i) == fmt->GetTypeNumber()) {
				n_types.Add(*item);
			}
		}
	}
	return n_types.Count();
}

/// カテゴリ名に一致するタイプ名リストを検索
/// @param [in]  n_category_name  : カテゴリ名
/// @param [out] n_type_names     : タイプ名リスト
/// @return リストの数
size_t DiskBasicParams::FindNames(const wxString &n_category_name, wxArrayString &n_type_names) const
{
	n_type_names.Clear();
	for(size_t n=0; n<Count(); n++) {
		const DiskBasicParam *item = &Item(n);
		if (item->FindBasicCategoryName(n_category_name)) {
			n_type_names.Add(item->GetBasicTypeName());
		}
	}
	return n_type_names.Count();
}
