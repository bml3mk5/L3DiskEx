/// @file diskparam.h
///
/// @brief ディスクパラメータ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISK_PARAMETER_H_
#define _DISK_PARAMETER_H_

#include "common.h"
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/dynarray.h>


/// (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
extern const int gSectorSizes[5];

class wxXmlNode;

class DiskParticulars;

//////////////////////////////////////////////////////////////////////

/// @brief トラック＆サイド＆セクタ番号を保持
class SectorParam
{
protected:
	int track_num;
	int side_num;
	int sector_num;

public:
	SectorParam();
	SectorParam(int n_track_num, int n_side_num, int n_sector_num);
	virtual ~SectorParam() {}
	/// @brief 一致するか
	bool operator==(const SectorParam &dst) const;

	/// @brief トラック番号を設定
	void SetTrackNumber(int val) { track_num = val; }
	/// @brief サイド番号を設定
	void SetSideNumber(int val) { side_num = val; }
	/// @brief セクタ番号を設定
	void SetSectorNumber(int val) { sector_num = val; }

	/// @brief トラック番号を返す
	int GetTrackNumber() const { return track_num; }
	/// @brief サイド番号を返す
	int GetSideNumber() const { return side_num; }
	/// @brief セクタ番号を返す
	int GetSectorNumber() const { return sector_num; }
};

//////////////////////////////////////////////////////////////////////

/// @brief 単密度など特殊なトラックやセクタ情報を保持する
class TrackParam : public SectorParam
{
protected:
//	int track_num;
//	int side_num;
//	int sector_num;
	int num_of_tracks;
	int sectors_per_track;
	int sector_size;
	wxUint8 id[4];	///< セクタのID C,H,R,N

public:
	TrackParam();
	TrackParam(int n_track_num, int n_side_num, int n_sector_num, int n_num_of_tracks, int n_sectors_per_track, int n_sector_size);
	virtual ~TrackParam() {}

	/// @brief 一致するか
	bool operator==(const TrackParam &dst) const;
	/// @brief 特殊なトラックか
	bool Match(int n_track_num, int n_side_num) const;

	/// @brief トラックの数を設定
	void SetNumberOfTracks(int val) { num_of_tracks = val; }
	/// @brief セクタの数を設定
	void SetSectorsPerTrack(int val) { sectors_per_track = val; }
	/// @brief セクタサイズを設定
	void SetSectorSize(int val) { sector_size = val; }
	/// @brief IDを設定
	void SetID(int idx, wxUint8 val);

	/// @brief トラックの数を返す
	int GetNumberOfTracks() const { return num_of_tracks; }
	/// @brief セクタの数を返す
	int GetSectorsPerTrack() const { return sectors_per_track; }
	/// @brief セクタサイズを返す
	int GetSectorSize() const { return sector_size; }
	/// @brief IDを返す
	wxUint8 GetID(int idx) const;
	/// @brief IDを返す
	const wxUint8 *GetID() const { return id; }
};

/// @class TrackParams
///
/// @brief TrackParam のリスト
WX_DECLARE_OBJARRAY(TrackParam, TrackParams);

//////////////////////////////////////////////////////////////////////

/// @brief 単密度など特殊なトラックやセクタ情報を保持する
class DiskParticular : public TrackParam
{
private:
	TrackParams excludes;	///< 除外リスト

public:
	DiskParticular();
	DiskParticular(int n_track_num, int n_side_num, int n_sector_num, int n_num_of_tracks, int n_sectors_per_track, int n_sector_size);
	virtual ~DiskParticular() {}

	/// @brief 除外するトラックを追加
	void AddExclude(const TrackParam &n_param);
	/// @brief 除外するトラックを探す
	bool FindExclude(int n_track_num, int n_side_num) const;
	/// @brief 同じトラックやサイドのものをまとめる
	static void UniqueTracks(int tracks, int sides, bool both_sides, DiskParticulars &arr);
};

//////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(DiskParticular, ArrayOfDiskParticular);

//////////////////////////////////////////////////////////////////////

/// @brief 単密度など特殊なトラックやセクタ情報を保持するリスト DiskParticular の配列
class DiskParticulars : public ArrayOfDiskParticular
{
public:
	DiskParticulars();

	/// @brief セクタ/トラックのリスト内で最小値を返す
	int GetMinSectorsPerTrack(int default_number) const;
	/// @brief セクタ/トラックのリスト内で最大値を返す
	int GetMaxSectorsPerTrack(int default_number) const;
};

/// @brief 全ての値が一致するか
bool operator==(const DiskParticulars &src, const DiskParticulars &dst);

//////////////////////////////////////////////////////////////////////

/// @brief 各トラックのセクタ数を保持
class NumSectorsParam
{
protected:
	int start_track_num;
	int num_of_tracks;
	int sectors_per_track;

public:
	NumSectorsParam();
	NumSectorsParam(int n_start_track_num, int n_num_of_tracks, int n_sectors_per_track);
	virtual ~NumSectorsParam() {}

	/// @brief 開始トラック番号を設定
	void SetStartTrackNumber(int val) { start_track_num = val; }
	/// @brief トラック数を設定
	void SetNumberOfTracks(int val) { num_of_tracks = val; }
	/// @brief セクタ数/トラックを設定
	void SetSectorsPerTrack(int val) { sectors_per_track = val; }

	/// @brief 開始トラック番号を返す
	int GetStartTrackNumber() const { return start_track_num; }
	/// @brief トラック数を返す
	int GetNumberOfTracks() const { return num_of_tracks; }
	/// @brief セクタ数/トラックを返す
	int GetSectorsPerTrack() const { return sectors_per_track; }
};

//////////////////////////////////////////////////////////////////////

WX_DECLARE_OBJARRAY(NumSectorsParam, ArrayOfNumSectorsParam);

//////////////////////////////////////////////////////////////////////

/// @brief 各トラックのセクタ数を保持しているリスト NumSectorsParam の配列
class NumSectorsParams : public ArrayOfNumSectorsParam
{
public:
	NumSectorsParams();

	/// @brief リスト内でセクタ数の最小値を返す
	int GetMinSectorOfTracks() const;
	/// @brief リスト内でセクタ数の最大値を返す
	int GetMaxSectorOfTracks() const;
};

//////////////////////////////////////////////////////////////////////

/// @brief DISK BASIC 名前リストを保存 
class DiskParamName
{
private:
	wxString name;
	int flags;
public:
	DiskParamName();
	~DiskParamName() {}

	/// @brief 名前を設定
	void SetName(const wxString &val) { name = val; }
	/// @brief 名前を返す
	const wxString &GetName() const { return name; }
	/// @brief フラグを設定
	void SetFlags(int val) { flags = val; }
	/// @brief フラグを返す
	int GetFlags() const { return flags; }
};

//////////////////////////////////////////////////////////////////////

/// @class DiskParamNames
///
/// @brief DiskParamName のリスト
WX_DECLARE_OBJARRAY(DiskParamName, DiskParamNames);

//////////////////////////////////////////////////////////////////////

/// @brief インターリーブ/セクタスキュー
///
/// @note マップがないときは、間隔から計算する。
class SectorInterleave
{
private:
	bool has_map;				///< 固有のマップを使用する場合 true
	wxArrayInt secs;			///< 変換後のセクタ番号
public:
	SectorInterleave();
	~SectorInterleave() {}

	/// @brief インターリーブ間隔を返す
	int Get() const;
	/// @brief インターリーブマップを返す
	int Get(int idx) const;
	/// @brief 固有のマップを持っているか
	bool HasMap() const { return has_map; }
	/// @brief インターリーブ間隔を設定
	void Set(int val);
	/// @brief インターリーブマップを設定
	void Set(const wxArrayInt &val);
};

//////////////////////////////////////////////////////////////////////

/// @brief ディスクパラメータ
class DiskParam
{
protected:
	wxString disk_type_name;	///< ディスク種類名 "2D" "2HD" など
	DiskParamNames basic_types;	///< BASIC種類（DiskBasicParamとのマッチングにも使用）
	bool reversible;			///< 裏返し可能 AB面あり（L3用3インチFDなど）
	int sides_per_disk;			///< サイド数
	int tracks_per_side;		///< トラック数
	int sectors_per_track;		///< セクタ数
	int sector_size;			///< セクタサイズ
	int numbering_sector;		///< セクタ番号の付番方法(0:サイド毎、1:トラック毎)
	int disk_density;			///< 0x00:2D 0x10:2DD 0x20:2HD
	int interleave;				///< セクタの間隔
	int track_number_base;		///< 開始トラック番号
	int sector_number_base;		///< 開始セクタ番号
	bool variable_secs_per_trk;	///< セクタ数がトラックごとに異なる
	DiskParticulars singles;	///< 単密度にするトラック
	DiskParticulars ptracks;	///< 特殊なトラックを定義
	DiskParticulars psectors;	///< 特殊なセクタを定義
	wxString density_name;		///< 密度情報（説明用）
	wxString description;		///< 説明

public:
	DiskParam();
	DiskParam(const DiskParam &src);
	virtual ~DiskParam() {}

	/// @brief 代入
	DiskParam &operator=(const DiskParam &src);
	/// @brief 設定
	void SetDiskParam(const DiskParam &src);
	/// @brief 主要パラメータだけ設定
	void SetDiskParam(int n_sides_per_disk
		, int n_tracks_per_side
		, int n_sectors_per_track
		, int n_sector_size
		, int n_disk_density
		, int n_interleave
		, const DiskParticulars &n_singles
		, const DiskParticulars &n_ptracks
	);
	/// @brief 全パラメータを設定
	void SetDiskParam(const wxString &n_type_name
		, const DiskParamNames &n_basic_types
		, bool n_reversible
		, int n_sides_per_disk
		, int n_tracks_per_side
		, int n_sectors_per_track
		, int n_sector_size
		, int n_numbering_sector
		, int n_disk_density
		, int n_interleave
		, int n_track_number_base
		, int n_sector_number_base
		, bool n_variable_secs_per_trk
		, const DiskParticulars &n_singles
		, const DiskParticulars &n_ptracks
		, const DiskParticulars &n_psectors
		, const wxString &n_density_name
		, const wxString &n_desc
	);

	/// @brief 初期化
	void ClearDiskParam();
	/// @brief 指定したパラメータで一致するものがあるか
	bool Match(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size
		, int n_interleave, int n_track_number_base, int n_sector_number_base, int n_numbering_sector
		, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks);
	/// @brief 指定したパラメータで一致するものがあるか
	bool Match(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size);
	/// @brief 指定したパラメータで一致するものがあるか
	bool Match(const DiskParam &param);
	/// @brief 名前を除いて指定したパラメータで一致するものがあるか
	bool MatchExceptName(const DiskParam &param);
	/// @brief 指定したパラメータに近い値で一致するものがあるか
	bool MatchNear(int num, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, bool &last);
	/// @brief 指定したトラック、サイドが単密度か
	bool FindSingleDensity(int track_num, int side_num, int *sectors_per_track = NULL, int *sector_size = NULL) const;
	/// @brief 単密度を持っているか
	int  HasSingleDensity(int *sectors_per_track = NULL, int *sector_size = NULL) const;
	/// @brief ディスクサイズを計算する（ベタディスク用）
	int  CalcDiskSize() const;
	/// @brief 特殊なトラックか
	bool FindParticularTrack(int track_num, int side_num, int &sectors_per_track, int &sector_size) const;
	/// @brief 特殊なセクタか
	bool FindParticularSector(int track_num, int side_num, int sector_num, int &sector_size, const wxUint8 **sector_id = NULL) const;
	/// @brief DISK BASICをさがす
	const DiskParamName *FindBasicType(const wxString &type_name, int flags = -1) const;

	/// @brief ディスク種類名を設定 "2D" "2HD" など
	void SetDiskTypeName(const wxString &str) { disk_type_name = str; }
	/// @brief BASIC種類を設定
	void SetBasicTypes(const DiskParamNames &arr) { basic_types = arr; }
	/// @brief 裏返し可能 AB面ありかどうかを設定
	void Reversible(bool val) { reversible = val; }
	/// @brief サイド数を設定
	void SetSidesPerDisk(int val) { sides_per_disk = val; }
	/// @brief トラック数を設定
	void SetTracksPerSide(int val) { tracks_per_side = val; }
	/// @brief セクタ数を設定
	void SetSectorsPerTrack(int val) { sectors_per_track = val; }
	/// @brief セクタサイズを設定
	void SetSectorSize(int val) { sector_size = val; }
	/// @brief セクタ番号の付番方法(0:サイド毎、1:トラック毎)を設定
	void SetNumberingSector(int val) { numbering_sector = val; }
	/// @brief 密度(0x00:2D 0x10:2DD 0x20:2HD)を設定
	void SetParamDensity(int val) { disk_density = val; }
	/// @brief セクタの間隔を設定
	void SetInterleave(int val) { interleave = val; }
	/// @brief 開始トラック番号を設定
	void SetTrackNumberBaseOnDisk(int val) { track_number_base = val; }
	/// @brief 開始セクタ番号を設定
	void SetSectorNumberBaseOnDisk(int val) { sector_number_base = val; }
	/// @brief セクタ数がトラックごとに異なる
	void VariableSectorsPerTrack(bool val) { variable_secs_per_trk = val; }
	/// @brief 単密度にするトラックを設定
	void SetSingles(const DiskParticulars &arr) { singles = arr; }
	/// @brief 特殊なトラックを設定
	void SetParticularTracks(const DiskParticulars &arr) { ptracks = arr; }
	/// @brief 特殊なセクタを設定
	void SetParticularSectors(const DiskParticulars &arr) { psectors = arr; }
	/// @brief 密度情報（説明用）を設定
	void SetDensityName(const wxString &str) { density_name = str; }
	/// @brief 説明を設定
	void SetDescription(const wxString &str) { description = str; }

	/// @brief 単密度にするトラックを追加
	void AddSingleDensity(const DiskParticular &val) { singles.Add(val); }
	/// @brief 特殊なトラックを追加
	void AddParticularTrack(const DiskParticular &val) { ptracks.Add(val); }
	/// @brief 特殊なセクタを追加
	void AddParticularSector(const DiskParticular &val) { psectors.Add(val); }

	/// @brief ディスク種類名を返す "2D" "2HD" など
	const wxString &GetDiskTypeName() const { return disk_type_name; }
	/// @brief BASIC種類を返す
	const DiskParamNames &GetBasicTypes() const { return basic_types; }
	/// @brief 裏返し可能 AB面ありかどうかを返す
	bool IsReversible() const { return reversible; }
	/// @brief サイド数を返す
	int GetSidesPerDisk() const { return sides_per_disk; }
	/// @brief トラック数を返す
	int GetTracksPerSide() const { return tracks_per_side; }
	/// @brief セクタ数を返す
	int GetSectorsPerTrack() const { return sectors_per_track; }
	/// @brief セクタサイズを返す
	int GetSectorSize() const { return sector_size; }
	/// @brief セクタ番号の付番方法(0:サイド毎、1:トラック毎)を返す
	int GetNumberingSector() const { return numbering_sector; }
	/// @brief 密度(0x00:2D 0x10:2DD 0x20:2HD)を返す
	int GetParamDensity() const { return disk_density; }
	/// @brief セクタの間隔を返す
	int GetInterleave() const { return interleave; }
	/// @brief 開始トラック番号を返す
	int GetTrackNumberBaseOnDisk() const { return track_number_base; }
	/// @brief 開始セクタ番号を返す
	int GetSectorNumberBaseOnDisk() const { return sector_number_base; }
	/// @brief セクタ数がトラックごとに異なる
	bool IsVariableSectorsPerTrack() const { return variable_secs_per_trk; }
	/// @brief 単密度にするトラックを返す
	const DiskParticulars &GetSingles() const { return singles; }
	/// @brief 特殊なトラックを返す
	const DiskParticulars &GetParticularTracks() const { return ptracks; }
	/// @brief 特殊なセクタを返す
	const DiskParticulars &GetParticularSectors() const { return psectors; }
	/// @brief 密度情報（説明用）を返す
	const wxString &GetDensityName() const { return density_name; }
	/// @brief 説明を返す
	const wxString &GetDescription() const { return description; }
	/// @brief ディスクパラメータを含めた説明文を返す
	wxString GetDiskDescription() const;
};

//////////////////////////////////////////////////////////////////////

/// @class DiskParams
///
/// @brief DiskParam のリスト
WX_DECLARE_OBJARRAY(DiskParam, DiskParams);

/// @class DiskParamPtrs
///
/// @brief DiskParam のポインタリスト
WX_DEFINE_ARRAY(const DiskParam *, DiskParamPtrs);

//////////////////////////////////////////////////////////////////////

/// @brief ディスクパラメータのテンプレートを提供する
class DiskTemplates
{
private:
	DiskParams params;

public:
	DiskTemplates();
	~DiskTemplates() {}

	/// @brief XMLファイルから読み込み
	bool Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs);
	/// @brief DiskBasicTypesエレメントをロード
	bool LoadDiskBasicTypes(const wxXmlNode *node, DiskParamNames &basic_types, wxString &errmsgs);
	/// @brief SingleDensityエレメントをロード
	bool LoadSingleDensity(const wxXmlNode *node, DiskParticular &s, wxString &errmsgs);
	/// @brief ParticularTrackエレメントをロード
	bool LoadParticularTrack(const wxXmlNode *node, DiskParticular &d, wxString &errmsgs);
	/// @brief ParticularSectorエレメントをロード
	bool LoadParticularSector(const wxXmlNode *node, DiskParticular &d, wxString &errmsgs);
	/// @brief DensityName/Descriptionエレメントをロード
	bool LoadDescription(const wxXmlNode *node, const wxString &locale_name, wxString &desc, wxString &desc_locale);

	/// @brief タイプ名に一致するテンプレートの番号を返す
	int IndexOf(const wxString &n_type_name) const;
	/// @brief 一致するテンプレートを返す
	const DiskParam *Find(const DiskParam &n_param) const;
	/// @brief タイプ名に一致するテンプレートを返す
	const DiskParam *Find(const wxString &n_type_name) const;
	/// @brief パラメータに一致するテンプレートを返す
	const DiskParam *FindStrict(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size
		, int n_interleave, int n_track_number_base, int n_sector_number_base, int n_numbering_sector
		, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks) const;
	/// @brief パラメータに一致するあるいは近い物のテンプレートを返す
	const DiskParam *Find(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size
		, int n_interleave, int n_track_number_base, int n_sector_number_base, int n_numbering_sector
		, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks) const;
	/// @brief パラメータに一致するテンプレートのリストを返す
	int Find(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, DiskParamPtrs &n_list, bool n_separator = false) const;
	/// @brief テンプレートを返す
	const DiskParam *ItemPtr(size_t index) const { return &params[index]; }
	/// @brief テンプレートを返す
	const DiskParam &Item(size_t index) const { return params[index]; }
	/// @brief テンプレートの数を返す
	size_t Count() const { return params.Count(); }
};

extern DiskTemplates gDiskTemplates;

#endif /* _DISK_PARAMETER_H_ */
