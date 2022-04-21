/// @file basicparam.h
///
/// @brief disk basic parameter
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICPARAM_H_
#define _BASICPARAM_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include <wx/variant.h>
#include <wx/hashmap.h>
#include "basiccommon.h"
#include "diskd88.h"


//////////////////////////////////////////////////////////////////////

WX_DECLARE_STRING_HASH_MAP(wxVariant, VariantHash);

//////////////////////////////////////////////////////////////////////

/// 特別な属性などを保持する
class L3Attribute
{
private:
	int idx;
	int type;
	int value;
	int mask;
	wxString name;
	wxString desc;

public:
	L3Attribute();
	L3Attribute(int n_idx, int n_type, int n_value, int n_mask, const wxString &n_name, const wxString &n_desc);

	int GetIndex() const { return idx; }
	int GetType() const { return type; }
	int GetValue() const { return value; }
	int GetMask() const { return mask; }
	const wxString &GetName() const { return name; }
	const wxString &GetDescription() const { return desc; }
};

WX_DECLARE_OBJARRAY(L3Attribute, ArrayOfL3Attribute);

/// 特別な属性のリスト
class L3Attributes : public ArrayOfL3Attribute
{
public:
	L3Attributes();
	/// 属性タイプと値に一致するアイテムを返す
	const L3Attribute *Find(int type, int value) const;
	/// 属性タイプと値に一致するアイテムを返す
	const L3Attribute *Find(int type, int mask, int value) const;
	/// 属性タイプに一致するアイテムを返す
	const L3Attribute *FindType(int type, int mask) const;
	/// 属性値に一致するアイテムを返す
	const L3Attribute *FindValue(int value) const;
	/// 属性名に一致するアイテムを返す
	const L3Attribute *Find(int type, const wxString &name) const;
	/// 属性名に一致するアイテムを返す
	const L3Attribute *Find(const wxString &name) const;
	/// 属性名に一致するアイテムを返す 大文字でマッチング
	const L3Attribute *FindUpperCase(const wxString &name) const;
	/// 属性名と属性タイプに一致するアイテムを返す 大文字でマッチング
	const L3Attribute *FindUpperCase(const wxString &name, int type, int mask) const;
	/// 属性名、属性タイプ、属性値に一致するアイテムを返す 大文字でマッチング
	const L3Attribute *FindUpperCase(const wxString &name, int type, int mask, int value) const;
	/// 属性値に一致するアイテムの位置を返す
	int					GetIndexByValue(int value) const;
	/// 属性値に一致するアイテムの属性値を返す
	int					GetTypeByValue(int value) const;
	/// 位置から属性タイプを返す
	int					GetTypeByIndex(int idx) const;
	/// 位置から属性値を返す
	int					GetValueByIndex(int idx) const;
};

//////////////////////////////////////////////////////////////////////

/// DISK BASICのフォーマットタイプ
class DiskBasicFormat
{
private:
	DiskBasicFormatType type_number;	///< フォーマットタイプ番号
	bool has_volume_name;				///< ボリューム名
	bool has_volume_number;				///< ボリューム番号
	bool has_volume_date;				///< ボリューム日付

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
	L3Attributes special_attrs;			///< 特別な属性
	L3Attributes attrs_by_extension;	///< 拡張子と属性の関係
	wxUint8 fillcode_on_format;			///< フォーマット時に埋めるコード
	wxUint8 fillcode_on_fat;			///< フォーマット時にFAT領域を埋めるコード
	wxUint8 fillcode_on_dir;			///< フォーマット時にディレクトリ領域を埋めるコード
	wxUint8 delete_code;				///< ファイル削除時にセットするコード
	wxUint8 text_terminate_code;		///< テキストの終端コード
	wxString valid_first_chars;			///< ファイル名の先頭に設定できる文字
	wxString valid_chars;				///< ファイル名に設定できる文字
	wxString invalid_chars;				///< ファイル名に設定できない文字
	wxString deduplicate_chars;			///< ファイル名に重複指定できない文字
	bool compare_case_insense;			///< ファイル名比較時に大文字小文字区別しないか
	bool to_upper_before_dialog;		///< ファイル名ダイアログ表示前に大文字に変換するか
	bool to_upper_after_renamed;		///< ファイル名ダイアログ入力後に大文字に変換するか
	bool filename_require;				///< ファイル名が必須か
	bool big_endian;					///< バイトオーダ ビッグエンディアンか

public:
	DiskBasicFormat();
	~DiskBasicFormat() {}

	/// フォーマットタイプ番号
	DiskBasicFormatType GetTypeNumber() const	{ return type_number; }
	/// ボリューム名
	bool			HasVolumeName() const		{ return has_volume_name; }
	/// ボリューム番号
	bool			HasVolumeNumber() const		{ return has_volume_number; }
	/// ボリューム日付
	bool			HasVolumeDate() const		{ return has_volume_date; }
	/// グループ(クラスタ)サイズ
	int					GetSectorsPerGroup() const	{ return sectors_per_group; }
	/// 最終グループのコード
	wxUint32			GetGroupFinalCode() const	{ return group_final_code; }
	/// システムで使用するコード
	wxUint32			GetGroupSystemCode() const	{ return group_system_code; }
	/// 未使用のコード
	wxUint32			GetGroupUnusedCode() const	{ return group_unused_code; }
	/// ディレクトリ名の終端コード
	wxUint8			GetDirTerminateCode() const	{ return dir_terminate_code; }
	/// ディレクトリ名の空白コード
	wxUint8			GetDirSpaceCode() const		{ return dir_space_code; }
	/// ディレクトリ名の空白コード（とり除くコード）
	wxUint8			GetDirTrimmingCode() const		{ return dir_trimming_code; }
	/// ディレクトリの開始位置（バイト）
	int					GetDirStartPos() const		{ return dir_start_pos; }
	/// ルートディレクトリの開始位置（バイト）
	int					GetDirStartPosOnRoot() const	{ return dir_start_pos_on_root; }
	/// ディレクトリのセクタ毎の開始位置
	int					GetDirStartPosOnSector() const	{ return dir_start_pos_on_sec; }
	/// 特別な属性
	const L3Attributes& GetSpecialAttributes() const { return special_attrs; }
	/// 拡張子と属性の関係
	const L3Attributes& GetAttributesByExtension() const { return attrs_by_extension; }
	/// フォーマット時に埋めるコード
	wxUint8				GetFillCodeOnFormat() const	{ return fillcode_on_format; }
	/// フォーマット時にFAT領域を埋めるコード
	wxUint8				GetFillCodeOnFAT() const	{ return fillcode_on_fat; }
	/// フォーマット時にディレクトリ領域を埋めるコード
	wxUint8				GetFillCodeOnDir() const	{ return fillcode_on_dir; }
	/// ファイル削除時にセットするコード
	wxUint8				GetDeleteCode() const		{ return delete_code; }
	/// テキストの終端コード
	wxUint8				GetTextTerminateCode() const	{ return text_terminate_code; }
	/// ファイル名の先頭に設定できる文字
	const wxString&		GetValidFirstChars() const	{ return valid_first_chars; }
	/// ファイル名に設定できる文字
	const wxString&		GetValidChars() const	{ return valid_chars; }
	/// ファイル名に設定できない文字
	const wxString&		GetInvalidChars() const	{ return invalid_chars; }
	/// ファイル名に重複指定できない文字
	const wxString&		GetDeduplicateChars() const	{ return deduplicate_chars; }
	/// ファイル名比較時に大文字小文字区別しないか
	bool				IsCompareCaseInsense() const	{ return compare_case_insense; }
	/// ファイル名ダイアログ表示前に大文字に変換するか
	bool				ToUpperBeforeDialog() const { return to_upper_before_dialog; }
	/// ファイル名ダイアログ入力後に大文字に変換するか
	bool				ToUpperAfterRenamed() const { return to_upper_after_renamed; }
	/// ファイル名が必須か
	bool				IsFileNameRequired() const	{ return filename_require; }
	/// バイトオーダ ビッグエンディアンか
	bool				IsBigEndian() const			{ return big_endian; }

	/// フォーマットタイプ番号
	void			SetTypeNumber(DiskBasicFormatType val)	{ type_number = val; }
	/// ボリューム名
	void			HasVolumeName(bool val)				{ has_volume_name = val; }
	/// ボリューム番号
	void			HasVolumeNumber(bool val)			{ has_volume_number = val; }
	/// ボリューム日付
	void			HasVolumeDate(bool val)				{ has_volume_date = val; }
	/// グループ(クラスタ)サイズ
	void			SetSectorsPerGroup(int val)			{ sectors_per_group = val; }
	/// 最終グループのコード
	void			SetGroupFinalCode(wxUint32 val)		{ group_final_code = val; }
	/// システムで使用するコード
	void			SetGroupSystemCode(wxUint32 val)	{ group_system_code = val; }
	/// 未使用のコード
	void			SetGroupUnusedCode(wxUint32 val)	{ group_unused_code = val; }
	/// ディレクトリ名の終端コード
	void			SetDirTerminateCode(wxUint8 val)	{ dir_terminate_code = val; }
	/// ディレクトリ名の空白コード
	void			SetDirSpaceCode(wxUint8 val)		{ dir_space_code = val; }
	/// ディレクトリ名の空白コード（とり除くコード）
	void			SetDirTrimmingCode(wxUint8 val)		{ dir_trimming_code = val; }
	/// ディレクトリの開始位置（バイト）
	void			SetDirStartPos(int val)				{ dir_start_pos = val; }
	/// ルートディレクトリの開始位置（バイト）
	void			SetDirStartPosOnRoot(int val)		{ dir_start_pos_on_root = val; }
	/// ディレクトリのセクタ毎の開始位置
	void			SetDirStartPosOnSector(int val)		{ dir_start_pos_on_sec = val; }
	/// 特別な属性
	void			SetSpecialAttributes(const L3Attributes& arr) { special_attrs = arr; }
	/// 拡張子と属性の関係
	void			SetAttributesByExtension(const L3Attributes& arr) { attrs_by_extension = arr; }
	/// フォーマット時に埋めるコード
	void			SetFillCodeOnFormat(wxUint8 val) { fillcode_on_format = val; }
	/// フォーマット時にFAT領域を埋めるコード
	void			SetFillCodeOnFAT(wxUint8 val)	{ fillcode_on_fat = val; }
	/// フォーマット時にディレクトリ領域を埋めるコード
	void			SetFillCodeOnDir(wxUint8 val)	{ fillcode_on_dir = val; }
	/// ファイル削除時にセットするコード
	void			SetDeleteCode(wxUint8 val)		{ delete_code = val; }
	/// テキストの終端コード
	void			SetTextTerminateCode(wxUint8 val)	{ text_terminate_code = val; }
	/// ファイル名の先頭に設定できる文字
	void			SetValidFirstChars(const wxString &str)	{ valid_first_chars = str; }
	/// ファイル名に設定できる文字
	void			SetValidChars(const wxString &str)	{ valid_chars = str; }
	/// ファイル名に設定できない文字
	void			SetInvalidChars(const wxString &str)	{ invalid_chars = str; }
	/// ファイル名に重複指定できない文字
	void			SetDeduplicateChars(const wxString &str)	{ deduplicate_chars = str; }
	/// ファイル名比較時に大文字小文字区別しないか
	void			CompareCaseInsense(bool val) 	{ compare_case_insense = val; }
	/// ファイル名ダイアログ表示前に大文字に変換するか
	void 			ToUpperBeforeDialog(bool val)	{ to_upper_before_dialog = val; }
	/// ファイル名ダイアログ入力後に大文字に変換するか
	void 			ToUpperAfterRenamed(bool val)	{ to_upper_after_renamed = val; }
	/// ファイル名が必須か
	void			RequireFileName(bool val)		{ filename_require = val; }
	/// バイトオーダ ビッグエンディアンか
	void			BigEndian(bool val)				{ big_endian = val; }
};

WX_DECLARE_OBJARRAY(DiskBasicFormat, DiskBasicFormats);

//////////////////////////////////////////////////////////////////////

/// DISK BASICのパラメータを保持するクラス
class DiskBasicParam
{
private:
	wxString basic_type_name;					///< BASIC種類名
	wxString basic_category_name;				///< BASICカテゴリ名
	const DiskBasicFormat *format_type;			///< フォーマット種類

	int format_subtype_number;			///< フォーマットサブタイプ番号
	int sectors_per_group;				///< グループ(クラスタ)サイズ
	int sides_on_basic;					///< BASICが使用するサイド数
	int sectors_on_basic;				///< BASICで使用するセクタ数/トラック
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
	wxUint32 group_final_code;			///< 最終グループのコード(0xc0 - )
	wxUint32 group_system_code;			///< システムで使用するコード(0xfe)
	wxUint32 group_unused_code;			///< 未使用のコード(0xff)
	wxArrayInt reserved_groups;			///< 予約済みグループ
	int dir_start_sector;				///< ルートディレクトリ開始セクタ
	int dir_end_sector;					///< ルートディレクトリ終了セクタ
	int dir_entry_count;				///< ルートディレクトリエントリ数
	int subdir_group_size;				///< サブディレクトリの初期グループ数
	wxUint8 dir_terminate_code;			///< ディレクトリ名の終端コード
	wxUint8 dir_space_code;				///< ディレクトリ名の空白コード
	wxUint8 dir_trimming_code;			///< ディレクトリ名の空白コード（とり除くコード）
	int dir_start_pos;					///< サブディレクトリの開始位置（バイト）
	int dir_start_pos_on_root;			///< ルートディレクトリの開始位置（バイト）
	int dir_start_pos_on_sec;			///< ディレクトリのセクタ毎の開始位置
	int group_width;					///< グループ幅（バイト）
	int groups_per_dir_entry;			///< １ディレクトリエントリで指定できるグループ数
	int sector_skew;					///< ソフトウェアセクタスキュー(セクタ間隔)
	L3Attributes special_attrs;			///< 特別な属性
	L3Attributes attrs_by_extension;	///< 拡張子と属性の関係
	wxUint8 fillcode_on_format;			///< フォーマット時に埋めるコード
	wxUint8 fillcode_on_fat;			///< フォーマット時にFAT領域を埋めるコード
	wxUint8 fillcode_on_dir;			///< フォーマット時にディレクトリ領域を埋めるコード
	wxUint8 delete_code;				///< ファイル削除時にセットするコード
	wxUint8 media_id;					///< メディアID
	wxUint8 text_terminate_code;		///< テキストの終端コード
	wxString valid_first_chars;			///< ファイル名の先頭に設定できる文字
	wxString valid_chars;				///< ファイル名に設定できる文字
	wxString invalid_chars;				///< ファイル名に設定できない文字
	wxString deduplicate_chars;			///< ファイル名に重複指定できない文字
	bool compare_case_insense;			///< ファイル名比較時に大文字小文字区別しないか
	bool to_upper_before_dialog;		///< ファイル名ダイアログ表示前に大文字に変換するか
	bool to_upper_after_renamed;		///< ファイル名ダイアログ入力後に大文字に変換するか
	bool filename_require;				///< ファイル名が必須か
	bool data_inverted;					///< データビットが反転してるか
	bool side_reversed;					///< サイドが反転してるか
	bool big_endian;					///< バイトオーダ ビッグエンディアンか
	bool mount_each_sides;				///< 片面のみ使用するOSで各面ごとに独立してアクセスできるか
	VariantHash various_params;			///< その他固有のパラメータ
	wxString basic_description;			///< 説明

public:
	DiskBasicParam();
	DiskBasicParam(const DiskBasicParam &src);
	virtual ~DiskBasicParam() {}

	virtual void		ClearBasicParam();
	virtual void		SetBasicParam(const DiskBasicParam &src);
	virtual const		DiskBasicParam &GetBasicParam() const;

	void				CalcDirStartEndSector(int sector_size);

	/// BASIC種類名
	const wxString&		GetBasicTypeName() const	{ return basic_type_name; }
	/// BASICカテゴリ名
	const wxString&		GetBasicCategoryName() const	{ return basic_category_name; }
	/// BASIC種類
	const DiskBasicFormat *GetFormatType() const		{ return format_type; }
	/// サブタイプ番号
	int					GetFormatSubTypeNumber() const	{ return format_subtype_number; }
	/// グループ(クラスタ)サイズ
	int					GetSectorsPerGroup() const	{ return sectors_per_group; }
	/// BASICが使用するサイド数
	int					GetSidesPerDiskOnBasic() const	{ return sides_on_basic; }
	/// BASICで使用するセクタ数/トラック
	int					GetSectorsPerTrackOnBasic() const	{ return sectors_on_basic; }
	/// BASICで使用するトラック数/サイド
	int					GetTracksPerSideOnBasic() const	{ return tracks_on_basic; }
	/// ファイル管理エリアのあるトラック番号
	int					GetManagedTrackNumber() const	{ return managed_track_number; }
	/// トラック当たりのグループ数
	int					GetGroupsPerTrack() const	{ return groups_per_track; }
	/// セクタ当たりのグループ数
	int					GetGroupsPerSector() const	{ return groups_per_sector; }
	/// 予約済みセクタ数
	int					GetReservedSectors() const	{ return reserved_sectors; }
	/// ファイル管理エリアの数
	int					GetNumberOfFats() const		{ return number_of_fats; }
	/// 有効・使用しているファイル管理エリアの数
	int					GetValidNumberOfFats() const	{ return valid_number_of_fats; }
	/// FAT領域のセクタ数
	int					GetSectorsPerFat() const	{ return sectors_per_fat; }
	/// FAT開始セクタ
	int					GetFatStartSector() const	{ return (reserved_sectors + 1); }
	/// FAT開始位置（バイト）
	int					GetFatStartPos() const		{ return fat_start_pos; }
	/// FAT最大グループ番号
	wxUint32			GetFatEndGroup() const		{ return fat_end_group; }
	/// FAT領域のあるサイド番号
	int					GetFatSideNumber() const	{ return fat_side_number; }
	/// 予約済みグループ番号
	const wxArrayInt&	GetReservedGroups() const	{ return reserved_groups; }
	/// 最終グループのコード
	wxUint32			GetGroupFinalCode() const	{ return group_final_code; }
	/// システムで使用するコード
	wxUint32			GetGroupSystemCode() const	{ return group_system_code; }
	/// 未使用のコード
	wxUint32			GetGroupUnusedCode() const	{ return group_unused_code; }
	/// ルートディレクトリ開始セクタ
	int					GetDirStartSector() const	{ return dir_start_sector; }
	/// ルートディレクトリ終了セクタ
	int					GetDirEndSector() const		{ return dir_end_sector; }
	/// ルートディレクトリエントリ数
	int					GetDirEntryCount() const	{ return dir_entry_count; }
	/// サブディレクトリの初期グループ数
	int					GetSubDirGroupSize() const	{ return subdir_group_size; }
	/// ディレクトリ名の終端コード
	wxUint8				GetDirTerminateCode() const	{ return dir_terminate_code; }
	/// ディレクトリ名の空白コード
	wxUint8				GetDirSpaceCode() const		{ return dir_space_code; }
	/// ディレクトリ名の空白コード（とり除くコード）
	wxUint8				GetDirTrimmingCode() const	{ return dir_trimming_code; }
	/// ディレクトリの開始位置（バイト）
	int					GetDirStartPos() const		{ return dir_start_pos; }
	/// ルートディレクトリの開始位置（バイト）
	int					GetDirStartPosOnRoot() const	{ return dir_start_pos_on_root; }
	/// ディレクトリのセクタ毎の開始位置
	int					GetDirStartPosOnSector() const	{ return dir_start_pos_on_sec; }
	/// グループ幅（バイト） 
	int					GetGroupWidth() const		{ return group_width; }
	/// １ディレクトリエントリで指定できるグループ数 
	int					GetGroupsPerDirEntry() const	{ return groups_per_dir_entry; }
	/// ソフトウェアセクタスキュー(セクタ間隔)
	int					GetSectorSkew() const		{ return sector_skew; }
	/// 特別な属性
	const L3Attributes& GetSpecialAttributes() const { return special_attrs; }
	/// 拡張子と属性の関係
	const L3Attributes& GetAttributesByExtension() const { return attrs_by_extension; }
	/// フォーマット時に埋めるコード
	wxUint8				GetFillCodeOnFormat() const	{ return fillcode_on_format; }
	/// フォーマット時にFAT領域を埋めるコード
	wxUint8				GetFillCodeOnFAT() const	{ return fillcode_on_fat; }
	/// フォーマット時にディレクトリ領域を埋めるコード
	wxUint8				GetFillCodeOnDir() const	{ return fillcode_on_dir; }
	/// ファイル削除時にセットするコード
	wxUint8				GetDeleteCode() const		{ return delete_code; }
	/// メディアID
	wxUint8				GetMediaId() const			{ return media_id; }
	/// テキストの終端コード
	wxUint8				GetTextTerminateCode() const	{ return text_terminate_code; }
	/// ファイル名の先頭に設定できる文字
	const wxString&		GetValidFirstChars() const	{ return valid_first_chars; }
	/// ファイル名に設定できる文字
	const wxString&		GetValidChars() const	{ return valid_chars; }
	/// ファイル名に設定できない文字
	const wxString&		GetInvalidChars() const	{ return invalid_chars; }
	/// ファイル名に重複指定できない文字
	const wxString&		GetDeduplicateChars() const	{ return deduplicate_chars; }
	/// ファイル名比較時に大文字小文字区別しないか
	bool				IsCompareCaseInsense() const	{ return compare_case_insense; }
	/// ファイル名ダイアログ表示前に大文字に変換するか
	bool				ToUpperBeforeDialog() const { return to_upper_before_dialog; }
	/// ファイル名ダイアログ入力後に大文字に変換するか
	bool				ToUpperAfterRenamed() const { return to_upper_after_renamed; }
	/// ファイル名が必須か
	bool				IsFileNameRequired() const	{ return filename_require; }
	/// データビットが反転してるか
	bool				IsDataInverted() const		{ return data_inverted; }
	/// サイドが反転してるか
	bool				IsSideReversed() const		{ return side_reversed; }
	/// バイトオーダ ビッグエンディアンか
	bool				IsBigEndian() const			{ return big_endian; }
	/// 反転したサイド番号を返す
	int					GetReversedSideNumber(int side_num) const;
	/// 片面のみ使用するOSで各面ごとに独立してアクセスできるか
	bool				CanMountEachSides() const;
	/// 説明
	const wxString&		GetBasicDescription() const	{ return basic_description; }
	/// 固有のパラメータ
	int					GetVariousIntegerParam(const wxString &key) const;
	/// 固有のパラメータ
	bool				GetVariousBoolParam(const wxString &key) const;
	/// 固有のパラメータ
	wxString			GetVariousStringParam(const wxString &key) const;

	/// BASIC種類名
	void			SetBasicTypeName(const wxString &str)	{ basic_type_name = str; }
	/// BASICカテゴリ名
	void			SetBasicCategoryName(const wxString &str)	{ basic_category_name = str; }
	/// BASIC種類
	void			SetFormatType(const DiskBasicFormat *val)	{ format_type = val; }
	/// サブタイプ番号
	void			SetFormatSubTypeNumber(int val)	{ format_subtype_number = val; }
	/// グループ(クラスタ)サイズ
	void			SetSectorsPerGroup(int val)		{ sectors_per_group = val; }
	/// BASICが使用するサイド数
	void			SetSidesPerDiskOnBasic(int val)		{ sides_on_basic = val; }
	/// BASICで使用するセクタ数/トラック
	void			SetSectorsPerTrackOnBasic(int val)		{ sectors_on_basic = val; }
	/// BASICで使用するトラック数/サイド
	void			SetTracksPerSideOnBasic(int val)		{ tracks_on_basic = val; }
	/// ファイル管理エリアのあるトラック番号
	void			SetManagedTrackNumber(int val)	{ managed_track_number = val; }
	/// トラック当たりのグループ数
	void			SetGroupsPerTrack(int val)		{ groups_per_track = val; }
	/// セクタ当たりのグループ数
	void			SetGroupsPerSector(int val)		{ groups_per_sector = val; }
	/// 予約済みセクタ数
	void			SetReservedSectors(int val)		{ reserved_sectors = val; }
	/// ファイル管理エリアの数
	void			SetNumberOfFats(int val)		{ number_of_fats = val; }
	/// 有効・使用しているファイル管理エリアの数
	void			SetValidNumberOfFats(int val)	{ valid_number_of_fats = val; }
	/// FAT領域のセクタ数
	void			SetSectorsPerFat(int val)		{ sectors_per_fat = val; }
	/// FAT開始位置（バイト）
	void			SetFatStartPos(int val)			{ fat_start_pos = val; }
	/// FAT最大グループ番号
	void			SetFatEndGroup(wxUint32 val)	{ fat_end_group = val; }
	/// FAT領域のあるサイド番号
	void			SetFatSideNumber(int val)		{ fat_side_number = val; }
	/// 予約済みグループ番号
	void			SetReservedGroups(const wxArrayInt &arr)	{ reserved_groups = arr; }
	/// 最終グループのコード
	void			SetGroupFinalCode(wxUint32 val)	{ group_final_code = val; }
	/// システムで使用するコード
	void			SetGroupSystemCode(wxUint32 val) { group_system_code = val; }
	/// 未使用のコード
	void			SetGroupUnusedCode(wxUint32 val) { group_unused_code = val; }
	/// 特別な属性
	void			SetSpecialAttributes(const L3Attributes &arr) { special_attrs = arr; }
	/// 拡張子と属性の関係
	void			SetAttributesByExtension(const L3Attributes& arr) { attrs_by_extension = arr; }
	/// ルートディレクトリ開始セクタ
	void			SetDirStartSector(int val)		{ dir_start_sector = val; }
	/// グループ幅（バイト） 
	void			SetGroupWidth(int val)			{ group_width = val; }
	/// １ディレクトリエントリで指定できるグループ数 
	void			SetGroupsPerDirEntry(int val)	{ groups_per_dir_entry = val; }
	/// ソフトウェアセクタスキュー(セクタ間隔)
	void			SetSectorSkew(int val)			{ sector_skew = val; }
	/// ルートディレクトリ終了セクタ
	void			SetDirEndSector(int val)		{ dir_end_sector = val; }
	/// ルートディレクトリエントリ数
	void			SetDirEntryCount(int val)		{ dir_entry_count = val; }
	/// サブディレクトリの初期グループ数
	void			SetSubDirGroupSize(int val)		{ subdir_group_size = val; }
	/// ディレクトリ名の終端コード
	void			SetDirTerminateCode(wxUint8 val)	{ dir_terminate_code = val; }
	/// ディレクトリ名の空白コード
	void			SetDirSpaceCode(wxUint8 val)	{ dir_space_code = val; }
	/// ディレクトリ名の空白コード（とり除くコード）
	void			SetDirTrimmingCode(wxUint8 val)		{ dir_trimming_code = val; }
	/// ディレクトリの開始位置（バイト）
	void			SetDirStartPos(int val)			{ dir_start_pos = val; }
	/// ルートディレクトリの開始位置（バイト）
	void			SetDirStartPosOnRoot(int val)	{ dir_start_pos_on_root = val; }
	/// ディレクトリのセクタ毎の開始位置
	void			SetDirStartPosOnSector(int val)	{ dir_start_pos_on_sec = val; }
	/// フォーマット時に埋めるコード
	void			SetFillCodeOnFormat(wxUint8 val) { fillcode_on_format = val; }
	/// フォーマット時にFAT領域を埋めるコード
	void			SetFillCodeOnFAT(wxUint8 val)	{ fillcode_on_fat = val; }
	/// フォーマット時にディレクトリ領域を埋めるコード
	void			SetFillCodeOnDir(wxUint8 val)	{ fillcode_on_dir = val; }
	/// ファイル削除時にセットするコード
	void			SetDeleteCode(wxUint8 val)		{ delete_code = val; }
	/// メディアID
	void			SetMediaId(wxUint8 val)			{ media_id = val; }
	/// テキストの終端コード
	void 			SetTextTerminateCode(wxUint8 val)	{ text_terminate_code = val; }
	/// ファイル名の先頭に設定できる文字
	void			SetValidFirstChars(const wxString &str)	{ valid_first_chars = str; }
	/// ファイル名に設定できる文字
	void			SetValidChars(const wxString &str)	{ valid_chars = str; }
	/// ファイル名に設定できない文字
	void			SetInvalidChars(const wxString &str)	{ invalid_chars = str; }
	/// ファイル名に重複指定できない文字
	void			SetDeduplicateChars(const wxString &str)	{ deduplicate_chars = str; }
	/// ファイル名比較時に大文字小文字区別しないか
	void			CompareCaseInsense(bool val) 	{ compare_case_insense = val; }
	/// ファイル名ダイアログ表示前に大文字に変換するか
	void 			ToUpperBeforeDialog(bool val)	{ to_upper_before_dialog = val; }
	/// ファイル名ダイアログ入力後に大文字に変換するか
	void 			ToUpperAfterRenamed(bool val)	{ to_upper_after_renamed = val; }
	/// ファイル名が必須か
	void			RequireFileName(bool val)		{ filename_require = val; }
	/// データビットが反転してるか
	void			DataInverted(bool val)			{ data_inverted = val; }
	/// サイドが反転してるか
	void			SideReversed(bool val)			{ side_reversed = val; }
	/// バイトオーダ ビッグエンディアンか
	void			BigEndian(bool val)				{ big_endian = val; }
	/// 片面のみ使用するOSで各面ごとに独立してアクセスできるか
	void			MountEachSides(bool val)		{ mount_each_sides = val; }
	/// 説明
	void			SetBasicDescription(const wxString &str) { basic_description = str; }
	/// 固有のパラメータ
	void			SetVariousParam(const wxString &key, const wxVariant &val);

	/// 説明文でソート
	static int		SortByDescription(const DiskBasicParam **item1, const DiskBasicParam **item2);
};

WX_DECLARE_OBJARRAY(DiskBasicParam, DiskBasicParams);

WX_DEFINE_ARRAY(const DiskBasicParam *, DiskBasicParamPtrs);

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

class wxXmlNode;

/// DISK BASICパラメータのテンプレートを提供する
class DiskBasicTemplates
{
private:
	DiskBasicFormats	formats;
	DiskBasicParams		types;
	DiskBasicCategories	categories;

	bool LoadTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs);
	bool LoadFormats(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs);
	bool LoadCategories(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs);

	bool LoadDescription(const wxXmlNode *node, const wxString &locale_name, wxString &desc, wxString &desc_locale);

	bool LoadReservedGroupsInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs, wxArrayInt &reserved_groups);
	bool LoadL3AttributesInTypes(const wxXmlNode *node, const wxString &locale_name, wxString &errmsgs, L3Attributes &attrs);
	bool LoadL3Attribute(const wxXmlNode *node, const wxString &locale_name, int type, L3Attributes &attrs);

	bool LoadFileNameChars(const wxXmlNode *node, wxString &val_chars, wxString &inv_chars, wxString &dup_chars, wxString &fst_chars, wxString &errmsgs);
	bool LoadFileNameCompareCase(const wxXmlNode *node, bool &val);

	bool LoadVariousParam(const wxXmlNode *node, const wxString &val, wxVariant &nval);

public:
	DiskBasicTemplates();
	~DiskBasicTemplates() {}

	/// XMLファイル読み込み
	bool Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs);

	/// カテゴリとタイプに一致するパラメータを検索
	const DiskBasicParam *FindType(const wxString &n_category, const wxString &n_basic_type) const;
	/// カテゴリが一致し、タイプリストに含まれるパラメータを検索
	const DiskBasicParam *FindType(const wxString &n_category, const DiskParamNames &n_basic_types) const;
	/// カテゴリ、タイプ、サイド数とセクタ数が一致するパラメータを検索
	const DiskBasicParam *FindType(const wxString &n_category, const wxString &n_basic_type, int n_sides, int n_sectors) const;
	/// DISK BASICフォーマット種類に一致するタイプを検索
	size_t FindTypes(const wxArrayInt &n_format_types, DiskBasicParams &n_types) const;
	/// カテゴリ番号に一致するタイプ名リストを検索
	size_t FindTypeNames(size_t n_category_index, wxArrayString &n_type_names) const;
	/// カテゴリ名に一致するタイプ名リストを検索
	size_t FindTypeNames(const wxString &n_category_name, wxArrayString &n_type_names) const;
	/// フォーマット種類を検索
	const DiskBasicFormat *FindFormat(DiskBasicFormatType format_type) const;
	/// タイプリストと一致するパラメータを得る
	size_t FindParams(const DiskParamNames &n_type_names, DiskBasicParamPtrs &params) const;
	/// カテゴリを検索
	const DiskBasicCategory *FindCategory(const wxString &n_category) const;

	const DiskBasicCategories &GetCategories() const { return categories; }
};

extern DiskBasicTemplates gDiskBasicTemplates;

#endif /* _BASICPARAM_H_ */
