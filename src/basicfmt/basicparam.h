/// @file basicparam.h
///
/// @brief disk basic parameter
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICPARAM_H
#define BASICPARAM_H

#include "../common.h"
#include "../parambase.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/variant.h>
#include <wx/hashmap.h>
#include "basiccommon.h"
#include "../diskimg/diskparam.h"
#include "../diskimg/diskimage.h"

class wxXmlNode;

//////////////////////////////////////////////////////////////////////

/// @brief DISK BASICの共通パラメータ
class DiskBasicParamBase
{
protected:
	int sectors_per_group;				///< グループ(クラスタ)サイズ
	wxUint32 group_final_code;			///< 最終グループのコード(0xc0 - )
	wxUint32 group_system_code;			///< システムで使用するコード(0xfe)
	wxUint32 group_unused_code;			///< 未使用のコード(0xff)
	wxUint8 dir_terminate_code;			///< ディレクトリ名の終端コード
	wxUint8 dir_space_code;				///< ディレクトリ名の空白コード
	wxUint8 dir_trimming_code;			///< ディレクトリ名の空白コード（とり除くコード）
	int dir_start_pos;					///< サブディレクトリの開始位置（バイト）
	int dir_start_pos_on_root;			///< ルートディレクトリの開始位置（バイト）
	int dir_start_pos_on_sec;			///< ディレクトリのセクタ毎の開始位置
	int dir_start_pos_on_group;			///< ディレクトリのグループ毎の開始位置
	MyAttributes special_attrs;			///< 特別な属性
	MyAttributes attrs_by_extension;	///< 拡張子と属性の関係
	wxUint8 fillcode_on_format;			///< フォーマット時に埋めるコード
	wxUint8 fillcode_on_fat;			///< フォーマット時にFAT領域を埋めるコード
	wxUint8 fillcode_on_dir;			///< フォーマット時にディレクトリ領域を埋めるコード
	wxUint8 delete_code;				///< ファイル削除時にセットするコード
	wxUint8 text_terminate_code;		///< テキストの終端コード
	wxUint8 extension_pre_code;			///< ファイル名と拡張子の間に付けるコード('.')
	ValidNameRule valid_file_name;		///< ファイル名に設定できるルール
	ValidNameRule valid_volume_name;	///< ボリューム名に設定できるルール
	bool compare_case_insense;			///< ファイル名比較時に大文字小文字区別しないか
	bool to_upper_before_dialog;		///< ファイル名ダイアログ表示前に大文字に変換するか
	bool to_upper_after_renamed;		///< ファイル名ダイアログ入力後に大文字に変換するか
	bool big_endian;					///< バイトオーダ ビッグエンディアンか
	VariantHash various_params;			///< その他固有のパラメータ

	/// @brief 初期化
	void ClearBasicParamBase();

public:
	DiskBasicParamBase();
	virtual ~DiskBasicParamBase() {}

	/// @brief 設定
	void SetBasicParamBase(const DiskBasicParamBase &src);

	/// @brief グループ(クラスタ)サイズ
	int					GetSectorsPerGroup() const	{ return sectors_per_group; }
	/// @brief 最終グループのコード
	wxUint32			GetGroupFinalCode() const	{ return group_final_code; }
	/// @brief システムで使用するコード
	wxUint32			GetGroupSystemCode() const	{ return group_system_code; }
	/// @brief 未使用のコード
	wxUint32			GetGroupUnusedCode() const	{ return group_unused_code; }
	/// @brief ディレクトリ名の終端コード
	wxUint8				GetDirTerminateCode() const	{ return dir_terminate_code; }
	/// @brief ディレクトリ名の空白コード
	wxUint8				GetDirSpaceCode() const		{ return dir_space_code; }
	/// @brief ディレクトリ名の空白コード（とり除くコード）
	wxUint8				GetDirTrimmingCode() const	{ return dir_trimming_code; }
	/// @brief ディレクトリの開始位置（バイト）
	int					GetDirStartPos() const		{ return dir_start_pos; }
	/// @brief ルートディレクトリの開始位置（バイト）
	int					GetDirStartPosOnRoot() const	{ return dir_start_pos_on_root; }
	/// @brief ディレクトリのセクタ毎の開始位置
	int					GetDirStartPosOnSector() const	{ return dir_start_pos_on_sec; }
	/// @brief ディレクトリのグループ毎の開始位置
	int					GetDirStartPosOnGroup() const	{ return dir_start_pos_on_group; }
	/// @brief 特別な属性
	const MyAttributes& GetSpecialAttributes() const	{ return special_attrs; }
	/// @brief 拡張子と属性の関係
	const MyAttributes& GetAttributesByExtension() const { return attrs_by_extension; }
	/// @brief フォーマット時に埋めるコード
	wxUint8				GetFillCodeOnFormat() const	{ return fillcode_on_format; }
	/// @brief フォーマット時にFAT領域を埋めるコード
	wxUint8				GetFillCodeOnFAT() const	{ return fillcode_on_fat; }
	/// @brief フォーマット時にディレクトリ領域を埋めるコード
	wxUint8				GetFillCodeOnDir() const	{ return fillcode_on_dir; }
	/// @brief ファイル削除時にセットするコード
	wxUint8				GetDeleteCode() const		{ return delete_code; }
	/// @brief テキストの終端コード
	wxUint8				GetTextTerminateCode() const	{ return text_terminate_code; }
	/// @brief ファイル名と拡張子の間に付けるコード('.')
	wxUint8				GetExtensionPreCode() const	{ return extension_pre_code; }
	/// @brief ファイル名に設定できるルール
	const ValidNameRule& GetValidFileName() const	{ return valid_file_name; }
	/// @brief ファイル名に設定できるルール
	ValidNameRule&		GetValidFileName()			{ return valid_file_name; }
	/// @brief ボリューム名に設定できるルール
	const ValidNameRule& GetValidVolumeName() const	{ return valid_volume_name; }
	/// @brief ボリューム名に設定できるルール
	ValidNameRule&		GetValidVolumeName()		{ return valid_volume_name; }
	/// @brief ファイル名比較時に大文字小文字区別しないか
	bool				IsCompareCaseInsense() const	{ return compare_case_insense; }
	/// @brief ファイル名ダイアログ表示前に大文字に変換するか
	bool				ToUpperBeforeDialog() const { return to_upper_before_dialog; }
	/// @brief ファイル名ダイアログ入力後に大文字に変換するか
	bool				ToUpperAfterRenamed() const { return to_upper_after_renamed; }
	/// @brief バイトオーダ ビッグエンディアンか
	bool				IsBigEndian() const			{ return big_endian; }
	/// @brief 固有のパラメータ
	const VariantHash &	GetVariousParams() const	{ return various_params; }
	/// @brief 固有のパラメータ
	void				GetVariousParam(const wxString &key, wxVariant &val) const;
	/// @brief 固有のパラメータ
	int					GetVariousIntegerParam(const wxString &key) const;
	/// @brief 固有のパラメータ
	bool				GetVariousBoolParam(const wxString &key) const;
	/// @brief 固有のパラメータ
	wxString			GetVariousStringParam(const wxString &key) const;

	/// @brief グループ(クラスタ)サイズ
	void				SetSectorsPerGroup(int val)			{ sectors_per_group = val; }
	/// @brief 最終グループのコード
	void				SetGroupFinalCode(wxUint32 val)		{ group_final_code = val; }
	/// @brief システムで使用するコード
	void				SetGroupSystemCode(wxUint32 val)	{ group_system_code = val; }
	/// @brief 未使用のコード
	void				SetGroupUnusedCode(wxUint32 val)	{ group_unused_code = val; }
	/// @brief ディレクトリ名の終端コード
	void				SetDirTerminateCode(wxUint8 val)	{ dir_terminate_code = val; }
	/// @brief ディレクトリ名の空白コード
	void				SetDirSpaceCode(wxUint8 val)		{ dir_space_code = val; }
	/// @brief ディレクトリ名の空白コード（とり除くコード）
	void				SetDirTrimmingCode(wxUint8 val)		{ dir_trimming_code = val; }
	/// @brief ディレクトリの開始位置（バイト）
	void				SetDirStartPos(int val)				{ dir_start_pos = val; }
	/// @brief ルートディレクトリの開始位置（バイト）
	void				SetDirStartPosOnRoot(int val)		{ dir_start_pos_on_root = val; }
	/// @brief ディレクトリのセクタ毎の開始位置
	void				SetDirStartPosOnSector(int val)		{ dir_start_pos_on_sec = val; }
	/// @brief ディレクトリのグループ毎の開始位置
	void				SetDirStartPosOnGroup(int val)		{ dir_start_pos_on_group = val; }
	/// @brief 特別な属性
	void				SetSpecialAttributes(const MyAttributes& arr)	{ special_attrs = arr; }
	/// @brief 拡張子と属性の関係
	void				SetAttributesByExtension(const MyAttributes& arr) { attrs_by_extension = arr; }
	/// @brief フォーマット時に埋めるコード
	void				SetFillCodeOnFormat(wxUint8 val)	{ fillcode_on_format = val; }
	/// @brief フォーマット時にFAT領域を埋めるコード
	void				SetFillCodeOnFAT(wxUint8 val)		{ fillcode_on_fat = val; }
	/// @brief フォーマット時にディレクトリ領域を埋めるコード
	void				SetFillCodeOnDir(wxUint8 val)		{ fillcode_on_dir = val; }
	/// @brief ファイル削除時にセットするコード
	void				SetDeleteCode(wxUint8 val)			{ delete_code = val; }
	/// @brief テキストの終端コード
	void				SetTextTerminateCode(wxUint8 val)	{ text_terminate_code = val; }
	/// @brief ファイル名と拡張子の間に付けるコード('.')
	void				SetExtensionPreCode(wxUint8 val) 	{ extension_pre_code = val; }
	/// @brief ファイル名に設定できるルール
	void				SetValidFileName(const ValidNameRule &str)	{ valid_file_name = str; }
	/// @brief ボリューム名に設定できるルール
	void				SetValidVolumeName(const ValidNameRule &str) { valid_volume_name = str; }
	/// @brief ファイル名比較時に大文字小文字区別しないか
	void				CompareCaseInsense(bool val) 		{ compare_case_insense = val; }
	/// @brief ファイル名ダイアログ表示前に大文字に変換するか
	void				ToUpperBeforeDialog(bool val)		{ to_upper_before_dialog = val; }
	/// @brief ファイル名ダイアログ入力後に大文字に変換するか
	void 				ToUpperAfterRenamed(bool val)		{ to_upper_after_renamed = val; }
	/// @brief バイトオーダ ビッグエンディアンか
	void				BigEndian(bool val)					{ big_endian = val; }
	/// @brief 固有のパラメータ
	void				SetVariousParam(const wxString &key, const wxVariant &val);
	/// @brief 固有のパラメータ
	void				SetVariousParams(const VariantHash &val) { various_params = val; }
};

//////////////////////////////////////////////////////////////////////

class DiskBasicParamBases : public TemplatesBase
{
private:
	bool m_set_volume_rule;

public:
	DiskBasicParamBases();
	virtual ~DiskBasicParamBases() {}

	/// @brief 共通パラメータ関連のロード
	bool Load(const wxXmlNode *node, const wxString &name, const wxString &value, const wxString &locale_name, DiskBasicParamBase &param, wxString &errmsgs);
};

//////////////////////////////////////////////////////////////////////

/// @brief DISK BASICのフォーマットタイプ
class DiskBasicFormat : public DiskBasicParamBase
{
private:
	DiskBasicFormatType type_number;	///< フォーマットタイプ番号
	bool has_volume_name;				///< ボリューム名
	bool has_volume_number;				///< ボリューム番号
	bool has_volume_date;				///< ボリューム日付

	/// @brief 初期化
	void ClearBasicFormatPrivate();

public:
	DiskBasicFormat();
	~DiskBasicFormat() {}

	/// @brief 初期化
	void ClearBasicFormat();

	/// @brief フォーマットタイプ番号
	DiskBasicFormatType GetTypeNumber() const	{ return type_number; }
	/// @brief ボリューム名
	bool			HasVolumeName() const		{ return has_volume_name; }
	/// @brief ボリューム番号
	bool			HasVolumeNumber() const		{ return has_volume_number; }
	/// @brief ボリューム日付
	bool			HasVolumeDate() const		{ return has_volume_date; }
//	/// @brief ファイル名が必須か
//	bool				IsFileNameRequired() const	{ return filename_require; }

	/// @brief フォーマットタイプ番号
	void			SetTypeNumber(DiskBasicFormatType val)	{ type_number = val; }
	/// @brief ボリューム名
	void			HasVolumeName(bool val)				{ has_volume_name = val; }
	/// @brief ボリューム番号
	void			HasVolumeNumber(bool val)			{ has_volume_number = val; }
	/// @brief ボリューム日付
	void			HasVolumeDate(bool val)				{ has_volume_date = val; }
//	/// @brief ファイル名が必須か
//	void			RequireFileName(bool val)		{ filename_require = val; }
};

//////////////////////////////////////////////////////////////////////

/// @class ArrayOfDiskBasicFormat
///
/// @brief DiskBasicFormat の配列
WX_DECLARE_OBJARRAY(DiskBasicFormat, ArrayOfDiskBasicFormat);

//////////////////////////////////////////////////////////////////////

/// @brief DiskBasicFormat のリスト
class DiskBasicFormats : public ArrayOfDiskBasicFormat, public TemplatesBase
{
public:
	/// @brief DiskBasicFormatエレメントのロード
	bool Load(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs);
	/// @brief フォーマット種類を検索
	const DiskBasicFormat *Find(DiskBasicFormatType format_type) const;
};

//////////////////////////////////////////////////////////////////////

/// @brief DISK BASICのパラメータを保持するクラス
class DiskBasicParam : public DiskBasicParamBase
{
private:
	wxString basic_type_name;					///< BASIC種類名
	wxArrayString basic_category_names;			///< BASICカテゴリ名
	const DiskBasicFormat *format_type;			///< フォーマット種類

	int format_subtype_number;			///< フォーマットサブタイプ番号
	int sides_on_basic;					///< BASICが使用するサイド数
	int sectors_on_basic;				///< BASICで使用するセクタ数/トラック
	NumSectorsParams sectors_on_basic_list;		///< BASICで使用するセクタ数/トラック
	int sector_number_base;				///< BASICで使用するセクタ番号基準
	int tracks_on_basic;				///< BASICで使用するトラック数/サイド
	int managed_track_number;			///< ファイル管理エリア
	int groups_per_track;				///< トラック当たりのグループ数
	int groups_per_sector;				///< セクタ当たりのグループ数
	int reserved_sectors;				///< 予約済みセクタ数
	int number_of_fats;					///< ファイル管理エリアの数
	int valid_number_of_fats;			///< 有効・使用しているファイル管理エリアの数
	int sectors_per_fat;				///< FAT領域のセクタ数
	int fat_start_pos;					///< FAT開始位置（バイト）
	wxUint32 fat_end_group;				///< FAT最大グループ番号
	int fat_side_number;				///< FAT領域のあるサイド番号
	wxArrayInt reserved_groups;			///< 予約済みグループ
	int dir_start_sector;				///< ルートディレクトリ開始セクタ
	int dir_end_sector;					///< ルートディレクトリ終了セクタ
	int dir_entry_count;				///< ルートディレクトリエントリ数
	int subdir_group_size;				///< サブディレクトリの初期グループ数
	int group_width;					///< グループ幅（バイト）
	int groups_per_dir_entry;			///< １ディレクトリエントリで指定できるグループ数
	int valid_density_type;				///< 有効な密度 0:倍密度 1:単密度
	SectorInterleave sector_skew;		///< ソフトウェアセクタスキュー(セクタ間隔)
	wxUint8 media_id;					///< メディアID
	bool data_inverted;					///< データビットが反転してるか
	bool side_reversed;					///< サイドが反転してるか
	bool mount_each_sides;				///< 片面のみ使用するOSで各面ごとに独立してアクセスできるか
	wxString basic_description;			///< 説明

	/// @brief 初期化
	void	ClearBasicParamPrivate();

public:
	DiskBasicParam();
	virtual ~DiskBasicParam() {}

	/// @brief 初期化
	virtual void		ClearBasicParam();
	/// @brief 設定
	virtual void		SetBasicParam(const DiskBasicParam &src);
	/// @brief パラメータを返す
	virtual const		DiskBasicParam &GetBasicParam() const;
	/// @brief 開始終了セクタを計算
	void				CalcDirStartEndSector(int sector_size);

	/// @brief BASIC種類名
	const wxString&		GetBasicTypeName() const	{ return basic_type_name; }
	/// @brief BASICカテゴリ名
	const wxArrayString& GetBasicCategoryNames() const	{ return basic_category_names; }
	/// @brief BASICカテゴリ名
	wxArrayString& GetBasicCategoryNames()				{ return basic_category_names; }
	/// @brief BASIC種類
	const DiskBasicFormat *GetFormatType() const		{ return format_type; }
	/// @brief サブタイプ番号
	int					GetFormatSubTypeNumber() const	{ return format_subtype_number; }
	/// @brief BASICが使用するサイド数
	int					GetSidesPerDiskOnBasic() const	{ return sides_on_basic; }
	/// @brief BASICで使用するセクタ数/トラック
	int					GetSectorsPerTrackOnBasic() const	{ return sectors_on_basic; }
	/// @brief BASICで使用するセクタ番号基準
	int					GetSectorNumberBaseOnBasic() const { return sector_number_base; }
	/// @brief BASICで使用するトラック数/サイド
	int					GetTracksPerSideOnBasic() const	{ return tracks_on_basic; }
	/// @brief ファイル管理エリアのあるトラック番号
	int					GetManagedTrackNumber() const	{ return managed_track_number; }
	/// @brief トラック当たりのグループ数
	int					GetGroupsPerTrack() const	{ return groups_per_track; }
	/// @brief セクタ当たりのグループ数
	int					GetGroupsPerSector() const	{ return groups_per_sector; }
	/// @brief 予約済みセクタ数
	int					GetReservedSectors() const	{ return reserved_sectors; }
	/// @brief ファイル管理エリアの数
	int					GetNumberOfFats() const		{ return number_of_fats; }
	/// @brief 有効・使用しているファイル管理エリアの数
	int					GetValidNumberOfFats() const	{ return valid_number_of_fats; }
	/// @brief FAT領域のセクタ数
	int					GetSectorsPerFat() const	{ return sectors_per_fat; }
	/// @brief FAT開始セクタ
	int					GetFatStartSector() const	{ return (reserved_sectors + 1); }
	/// @brief FAT開始位置（バイト）
	int					GetFatStartPos() const		{ return fat_start_pos; }
	/// @brief FAT最大グループ番号
	wxUint32			GetFatEndGroup() const		{ return fat_end_group; }
	/// @brief FAT領域のあるサイド番号
	int					GetFatSideNumber() const	{ return fat_side_number; }
	/// @brief 予約済みグループ番号
	const wxArrayInt&	GetReservedGroups() const	{ return reserved_groups; }
	/// @brief ルートディレクトリ開始セクタ
	int					GetDirStartSector() const	{ return dir_start_sector; }
	/// @brief ルートディレクトリ終了セクタ
	int					GetDirEndSector() const		{ return dir_end_sector; }
	/// @brief ルートディレクトリエントリ数
	int					GetDirEntryCount() const	{ return dir_entry_count; }
	/// @brief サブディレクトリの初期グループ数
	int					GetSubDirGroupSize() const	{ return subdir_group_size; }
	/// @brief グループ幅（バイト） 
	int					GetGroupWidth() const		{ return group_width; }
	/// @brief １ディレクトリエントリで指定できるグループ数 
	int					GetGroupsPerDirEntry() const	{ return groups_per_dir_entry; }
	/// @brief 有効な密度
	int					GetValidDensityType() const	{ return valid_density_type; }
	/// @brief ソフトウェアセクタスキュー(セクタ間隔)
	int					GetSectorSkew() const		{ return sector_skew.Get(); }
	/// @brief ソフトウェアセクタスキュー(セクタ間隔) 固有のマップ
	int					GetSectorSkewMap(int idx) const		{ return sector_skew.Get(idx); }
	/// @brief ソフトウェアセクタスキュー(セクタ間隔) 固有のマップを持っているか
	bool				HasSectorSkewMap() const		{ return sector_skew.HasMap(); }
	/// @brief メディアID
	wxUint8				GetMediaId() const			{ return media_id; }
//	/// @brief ファイル名が必須か
//	bool				IsFileNameRequired() const	{ return filename_require; }
	/// @brief データビットが反転してるか
	bool				IsDataInverted() const		{ return data_inverted; }
	/// @brief サイドが反転してるか
	bool				IsSideReversed() const		{ return side_reversed; }
	/// @brief 反転したサイド番号を返す
	int					GetReversedSideNumber(int side_num) const;
	/// @brief 片面のみ使用するOSで各面ごとに独立してアクセスできるか
	bool				CanMountEachSides() const;
	/// @brief 説明
	const wxString&		GetBasicDescription() const	{ return basic_description; }

	/// @brief BASIC種類名
	void			SetBasicTypeName(const wxString &str)	{ basic_type_name = str; }
	/// @brief BASICカテゴリ名
	void			SetBasicCategoryNames(const wxArrayString &arr)	{ basic_category_names = arr; }
	/// @brief BASIC種類
	void			SetFormatType(const DiskBasicFormat *val)	{ format_type = val; }
	/// @brief サブタイプ番号
	void			SetFormatSubTypeNumber(int val)	{ format_subtype_number = val; }
	/// @brief BASICが使用するサイド数
	void			SetSidesPerDiskOnBasic(int val)		{ sides_on_basic = val; }
	/// @brief BASICで使用するセクタ数/トラック
	void			SetSectorsPerTrackOnBasic(int val)		{ sectors_on_basic = val; }
	/// @brief BASICで使用するセクタ番号基準
	void			SetSectorNumberBaseOnBasic(int val)		{ sector_number_base = val; }
	/// @brief BASICで使用するトラック数/サイド
	void			SetTracksPerSideOnBasic(int val)		{ tracks_on_basic = val; }
	/// @brief ファイル管理エリアのあるトラック番号
	void			SetManagedTrackNumber(int val)	{ managed_track_number = val; }
	/// @brief トラック当たりのグループ数
	void			SetGroupsPerTrack(int val)		{ groups_per_track = val; }
	/// @brief セクタ当たりのグループ数
	void			SetGroupsPerSector(int val)		{ groups_per_sector = val; }
	/// @brief 予約済みセクタ数
	void			SetReservedSectors(int val)		{ reserved_sectors = val; }
	/// @brief ファイル管理エリアの数
	void			SetNumberOfFats(int val)		{ number_of_fats = val; }
	/// @brief 有効・使用しているファイル管理エリアの数
	void			SetValidNumberOfFats(int val)	{ valid_number_of_fats = val; }
	/// @brief FAT領域のセクタ数
	void			SetSectorsPerFat(int val)		{ sectors_per_fat = val; }
	/// @brief FAT開始位置（バイト）
	void			SetFatStartPos(int val)			{ fat_start_pos = val; }
	/// @brief FAT最大グループ番号
	void			SetFatEndGroup(wxUint32 val)	{ fat_end_group = val; }
	/// @brief FAT領域のあるサイド番号
	void			SetFatSideNumber(int val)		{ fat_side_number = val; }
	/// @brief 予約済みグループ番号
	void			SetReservedGroups(const wxArrayInt &arr)	{ reserved_groups = arr; }
	/// @brief ルートディレクトリ開始セクタ
	void			SetDirStartSector(int val)		{ dir_start_sector = val; }
	/// @brief グループ幅（バイト） 
	void			SetGroupWidth(int val)			{ group_width = val; }
	/// @brief １ディレクトリエントリで指定できるグループ数 
	void			SetGroupsPerDirEntry(int val)	{ groups_per_dir_entry = val; }
	/// @brief 有効な密度
	void 			SetValidDensityType(int val)	{ valid_density_type = val; }
	/// @brief ソフトウェアセクタスキュー(セクタ間隔)
	void			SetSectorSkew(int val)			{ sector_skew.Set(val); }
	/// @brief ソフトウェアセクタスキュー(セクタ間隔)
	void			SetSectorSkewMap(const wxArrayInt &arr)		{ sector_skew.Set(arr); }
	/// @brief ルートディレクトリ終了セクタ
	void			SetDirEndSector(int val)		{ dir_end_sector = val; }
	/// @brief ルートディレクトリエントリ数
	void			SetDirEntryCount(int val)		{ dir_entry_count = val; }
	/// @brief サブディレクトリの初期グループ数
	void			SetSubDirGroupSize(int val)		{ subdir_group_size = val; }
	/// @brief メディアID
	void			SetMediaId(wxUint8 val)			{ media_id = val; }
//	/// @brief ファイル名が必須か
//	void			RequireFileName(bool val)		{ filename_require = val; }
	/// @brief データビットが反転してるか
	void			DataInverted(bool val)			{ data_inverted = val; }
	/// @brief サイドが反転してるか
	void			SideReversed(bool val)			{ side_reversed = val; }
	/// @brief 片面のみ使用するOSで各面ごとに独立してアクセスできるか
	void			MountEachSides(bool val)		{ mount_each_sides = val; }
	/// @brief 説明
	void			SetBasicDescription(const wxString &str) { basic_description = str; }

	/// @brief BASICで使用するセクタ数/トラック
	NumSectorsParams &SectorsPerTrackOnBasicList() { return sectors_on_basic_list; }
	/// @brief BASICカテゴリ名を追加
	void			AddBasicCategoryName(const wxString &str);
	/// @brief BASICカテゴリ名を返す
	wxString		GetBasicCategoryName() const;
	/// @brief BASICカテゴリ名が存在するか
	bool			FindBasicCategoryName(const wxString &str) const;

	/// @brief ReservedGroupsエレメントをロード
	bool LoadReservedGroupsInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs);
	/// @brief SectorSkewMapエレメントをロード
	bool LoadSectorSkewMap(const wxXmlNode *node);
	/// @brief SectorsPerTrackエレメントをロード
	bool LoadNumSectorsMap(const wxXmlNode *node, const wxString &val);
	/// @brief Categoriesエレメントのロード
	bool LoadCategories(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs);

	/// @brief 説明文でソート
	static int		SortByDescription(const DiskBasicParam **item1, const DiskBasicParam **item2);
};

//////////////////////////////////////////////////////////////////////

/// @class ArrayOfDiskBasicParam
///
/// @brief DiskBasicParam の配列
WX_DECLARE_OBJARRAY(DiskBasicParam, ArrayOfDiskBasicParam);

//////////////////////////////////////////////////////////////////////

/// @class DiskBasicParamPtrs
///
/// @brief DiskBasicParam のポインタリスト
WX_DEFINE_ARRAY(const DiskBasicParam *, DiskBasicParamPtrs);

//////////////////////////////////////////////////////////////////////

/// @brief DiskBasicParam のリスト
class DiskBasicParams : public ArrayOfDiskBasicParam, public TemplatesBase
{
public:
	/// @brief DiskBasicTypeエレメントのロード
	bool Load(const wxXmlNode *node, const wxString &locale_name, const DiskBasicFormats &formats, wxString &errmsgs);
	/// @brief カテゴリとタイプに一致するパラメータを検索
	const DiskBasicParam *Find(const wxString &n_category, const wxString &n_basic_type) const;
	/// @brief カテゴリが一致し、タイプリストに含まれるパラメータを検索
	const DiskBasicParam *Find(const wxString &n_category, const DiskParamNames &n_basic_types) const;
	/// @brief カテゴリ、タイプ、サイド数とセクタ数が一致するパラメータを検索
	const DiskBasicParam *Find(const wxString &n_category, const wxString &n_basic_type, int n_sides, int n_sectors) const;
	/// @brief DISK BASICフォーマット種類に一致するタイプを検索
	size_t FindTypes(const wxArrayInt &n_format_types, DiskBasicParams &n_types) const;
	/// @brief カテゴリ名に一致するタイプ名リストを検索
	size_t FindNames(const wxString &n_category_name, wxArrayString &n_type_names) const;
};

#endif /* BASICPARAM_H */
