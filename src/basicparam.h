/// @file basicparam.h
///
/// @brief disk basic parameter
///
#ifndef _BASICPARAM_H_
#define _BASICPARAM_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"
#include "diskd88.h"

/// DISK BASICのパラメータを保持するクラス
class DiskBasicParam
{
private:
	wxString basic_type_name;
	wxString basic_category_name;
	DiskBasicFormatType format_type; ///< フォーマット種類

	int sectors_per_group;		///< グループ(クラスタ)サイズ
	int sides_on_basic;			///< BASICが使用するサイド数
	int sectors_on_basic;		///< BASICで使用するセクタ数/トラック
	int managed_track_number;	///< ファイル管理エリア
	int reserved_sectors;		///< 予約済みセクタ数
	int number_of_fats;			///< ファイル管理エリアの数
	int sectors_per_fat;		///< FAT領域のセクタ数
//	int fat_start_sector;		///< FAT領域の開始セクタ
//	int fat_end_sector;			///< FAT領域の終了セクタ
	int fat_start_pos;			///< FAT開始位置（バイト）
	wxUint32 fat_end_group;		///< FAT最大グループ番号
	int fat_side_number;		///< FAT領域のあるサイド番号
	wxUint32 group_final_code;	///< 最終グループのコード(0xc0 - )
	wxUint32 group_system_code;	///< システムで使用するコード(0xfe)
	wxUint32 group_unused_code;	///< 未使用のコード(0xff)
	wxArrayInt reserved_groups;	///< 予約済みグループ
	int dir_start_sector;		///< ルートディレクトリ開始セクタ
	int dir_end_sector;			///< ルートディレクトリ終了セクタ
	int dir_entry_count;		///< ルートディレクトリエントリ数
	int subdir_group_size;		///< サブディレクトリの初期グループ数
	wxUint8 dir_space_code;		///< ディレクトリの空白
	int dir_start_pos;			///< ルートディレクトリの開始位置（バイト）
	int dir_start_pos_on_sec;	///< ディレクトリのセクタ毎の開始位置
	int groups_per_dir_entry;	///< １ディレクトリエントリで指定できるグループ数 
	int id_sector_pos;			///< ID領域のセクタ位置(0 - )
	wxString id_string;			///< ID領域の先頭コード
	wxString ipl_string;		///< IPL領域の先頭コード
	wxString volume_string;		///< ボリューム名
	wxUint8 fillcode_on_format;	///< フォーマット時に埋めるコード
	wxUint8 fillcode_on_fat;	///< フォーマット時にFAT領域を埋めるコード
	wxUint8 fillcode_on_dir;	///< フォーマット時にディレクトリ領域を埋めるコード
	wxUint8 delete_code;		///< ファイル削除時にセットするコード
	wxUint8 media_id;			///< メディアID
	bool data_inverted;			///< データビットが反転してるか
	bool side_reversed;			///< サイドが反転してるか
	wxString basic_description;	///< 説明

public:
	DiskBasicParam();
	DiskBasicParam(const DiskBasicParam &src);
	DiskBasicParam(
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
		bool				n_data_inverted,
		bool				n_side_reversed,
		const wxString &	n_basic_description
	);
	virtual ~DiskBasicParam() {}

	virtual void		ClearBasicParam();
	virtual void		SetBasicParam(const DiskBasicParam &src);
	virtual const		DiskBasicParam &GetBasicParam() const;

	void				CalcDirStartEndSector(int sector_size);

	const wxString&		GetBasicTypeName() const	{ return basic_type_name; }
	const wxString&		GetBasicCategoryName() const	{ return basic_category_name; }
	DiskBasicFormatType	GetFormatType() const		{ return format_type; }
	int					GetSectorsPerGroup() const	{ return sectors_per_group; }
	int					GetSidesOnBasic() const		{ return sides_on_basic; }
	int					GetSectorsOnBasic() const	{ return sectors_on_basic; }
	int					GetManagedTrackNumber() const	{ return managed_track_number; }
	int					GetReservedSectors() const	{ return reserved_sectors; }
	int					GetNumberOfFats() const		{ return number_of_fats; }
	int					GetSectorsPerFat() const	{ return sectors_per_fat; }
	int					GetFatStartSector() const	{ return (reserved_sectors + 1); }
//	int					GetFatEndSector() const		{ return fat_end_sector; }
	int					GetFatStartPos() const		{ return fat_start_pos; }
	wxUint32			GetFatEndGroup() const		{ return fat_end_group; }
	int					GetFatSideNumber() const	{ return fat_side_number; }
	const wxArrayInt&	GetReservedGroups() const	{ return reserved_groups; }
	wxUint32			GetGroupFinalCode() const	{ return group_final_code; }
	wxUint32			GetGroupSystemCode() const	{ return group_system_code; }
	wxUint32			GetGroupUnusedCode() const	{ return group_unused_code; }
	int					GetDirStartSector() const	{ return dir_start_sector; }
	int					GetDirEndSector() const		{ return dir_end_sector; }
	int					GetDirEntryCount() const	{ return dir_entry_count; }
	int					GetSubDirGroupSize() const	{ return subdir_group_size; }
	wxUint8				GetDirSpaceCode() const		{ return dir_space_code; }
	int					GetDirStartPos() const		{ return dir_start_pos; }
	int					GetDirStartPosOnSector() const	{ return dir_start_pos_on_sec; }
	int					GetGroupsPerDirEntry() const	{ return groups_per_dir_entry; }
	int					GetIdSectorPos() const		{ return id_sector_pos; }
	const wxString&		GetIDString() const			{ return id_string; }
	const wxString&		GetIPLString() const		{ return ipl_string; }
	const wxString&		GetVolumeString() const		{ return volume_string; }
	wxUint8				GetFillCodeOnFormat() const	{ return fillcode_on_format; }
	wxUint8				GetFillCodeOnFAT() const	{ return fillcode_on_fat; }
	wxUint8				GetFillCodeOnDir() const	{ return fillcode_on_dir; }
	wxUint8				GetDeleteCode() const		{ return delete_code; }
	wxUint8				GetMediaId() const			{ return media_id; }
	bool				IsDataInverted() const		{ return data_inverted; }
	bool				IsSideReversed() const		{ return side_reversed; }
	int					GetReversedSideNumber(int side_num) const;

	const wxString& GetBasicDescription()			{ return basic_description; }

	void			SetSectorsPerGroup(int val)		{ sectors_per_group = val; }
	void			SetSidesOnBasic(int val)		{ sides_on_basic = val; }
	void			SetManagedTrackNumber(int val)	{ managed_track_number = val; }
	void			SetReservedSectors(int val)		{ reserved_sectors = val; }
	void			SetNumberOfFats(int val)		{ number_of_fats = val; }
	void			SetSectorsPerFat(int val)		{ sectors_per_fat = val; }
	void			SetReservedSector(int val)		{ reserved_sectors = val; }
//	void			SetFatStartSector(int val)		{ reserved_sectors = val - 1; }
//	void			SetFatEndSector(int val)		{ fat_end_sector = val; }
	void			SetFatStartPos(int val)			{ fat_start_pos = val; }
	void			SetFatEndGroup(wxUint32 val)	{ fat_end_group = val; }
	void			SetReservedGroups(const wxArrayInt &val)	{ reserved_groups = val; }
	void			SetGroupFinalCode(wxUint32 val)	{ group_final_code = val; }
	void			SetGroupSystemCode(wxUint32 val) { group_system_code = val; }
	void			SetGroupUnusedCode(wxUint32 val) { group_unused_code = val; }
	void			SetDirStartSector(int val)		{ dir_start_sector = val; }
	void			SetGroupsPerDirEntry(int val)	{ groups_per_dir_entry = val; }
	void			SetIDSectorPos(int val)			{ id_sector_pos = val; }
	void			SetIDString(const wxString &str)		{ id_string = str; }
	void			SetIPLString(const wxString &str)		{ ipl_string = str; }
	void			SetVolumeString(const wxString &str)	{ volume_string = str; }
	void			SetDirEndSector(int val)		{ dir_end_sector = val; }
	void			SetDirEntryCount(int val)		{ dir_entry_count = val; }
	void			SetSubDirGroupSize(int val)		{ subdir_group_size = val; }
	void			SetDirSpaceCode(wxUint8 val)	{ dir_space_code = val; }
	void			SetDirStartPos(int val)			{ dir_start_pos = val; }
	void			SetDirStartPosOnSector(int val)	{ dir_start_pos_on_sec = val; }
	void			SetFillCodeOnFormat(wxUint8 val) { fillcode_on_format = val; }
	void			SetFillCodeOnFAT(wxUint8 val)	{ fillcode_on_fat = val; }
	void			SetFillCodeOnDir(wxUint8 val)	{ fillcode_on_dir = val; }
	void			SetDeleteCode(wxUint8 val)		{ delete_code = val; }
	void			SetMediaId(wxUint8 val)			{ media_id = val; }
	void			DataInverted(bool val)			{ data_inverted = val; }
	void			SideReversed(bool val)			{ side_reversed = val; }

	void			SetBasicDescription(const wxString &str) { basic_description = str; }
};

WX_DECLARE_OBJARRAY(DiskBasicParam, DiskBasicParams);

/// DISK BASICのカテゴリ(メーカ毎、OS毎にまとめる)クラス
class DiskBasicCategory
{
private:
	wxString	basic_type_name;
	wxString	description;

public:
	DiskBasicCategory();
	DiskBasicCategory(const DiskBasicCategory &src);
	DiskBasicCategory(const wxString & n_basic_type_name, const wxString & n_description);
	virtual ~DiskBasicCategory() {}

	const wxString&		GetBasicTypeName() const	{ return basic_type_name; }

	const wxString& GetDescription()				{ return description; }

	void			SetDescription(const wxString &str) { description = str; }
};

WX_DECLARE_OBJARRAY(DiskBasicCategory, DiskBasicCategories);

/// DISK BASICパラメータのテンプレートを提供する
class DiskBasicTemplates
{
private:
	DiskBasicParams types;
	DiskBasicCategories categories;

public:
	DiskBasicTemplates();
	~DiskBasicTemplates() {}

	/// XMLファイル読み込み
	bool Load(const wxString &data_path, const wxString &locale_name);

	/// カテゴリとタイプに一致するパラメータを検索
	DiskBasicParam *FindType(const wxString &n_category, const wxString &n_basic_type);
	/// カテゴリが一致し、タイプリストに含まれるパラメータを検索
	DiskBasicParam *FindType(const wxString &n_category, const wxArrayString &n_basic_types);
	/// カテゴリ、サイド数とセクタ数が一致するパラメータを検索
	DiskBasicParam *FindType(const wxString &n_category, int n_sides, int n_sectors);
	/// DISK BASICフォーマット種類に一致するタイプを検索
	size_t FindTypes(const wxArrayInt &n_format_types, DiskBasicParams &n_types);
	/// カテゴリ番号に一致するタイプ名リストを検索
	size_t FindTypeNames(size_t n_category_index, wxArrayString &n_type_names);
	/// カテゴリ名に一致するタイプ名リストを検索
	size_t FindTypeNames(const wxString &n_category_name, wxArrayString &n_type_names);

	const DiskBasicCategories &GetCategories() { return categories; }
};

extern DiskBasicTemplates gDiskBasicTemplates;

#endif /* _BASICPARAM_H_ */
