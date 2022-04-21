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

class DiskParticulars;

/// 単密度など特殊なトラックやセクタ情報を保持する
class TrackParam
{
protected:
	int track_num;
	int side_num;
	int sector_num;
	int sectors_per_track;
	int sector_size;
	wxUint8 id[4];	///< セクタのID C,H,R,N

public:
	TrackParam();
	TrackParam(int n_track_num, int n_side_num, int n_sector_num, int n_sectors_per_track, int n_sector_size);
	virtual ~TrackParam() {}

	bool operator==(const TrackParam &dst) const;
	bool Match(int n_track_num, int n_side_num) const;

	void SetTrackNumber(int val) { track_num = val; }
	void SetSideNumber(int val) { side_num = val; }
	void SetSectorNumber(int val) { sector_num = val; }
	void SetSectorsPerTrack(int val) { sectors_per_track = val; }
	void SetSectorSize(int val) { sector_size = val; }
	void SetID(int idx, wxUint8 val);

	int GetTrackNumber() const { return track_num; }
	int GetSideNumber() const { return side_num; }
	int GetSectorNumber() const { return sector_num; }
	int GetSectorsPerTrack() const { return sectors_per_track; }
	int GetSectorSize() const { return sector_size; }
	wxUint8 GetID(int idx) const;
	const wxUint8 *GetID() const { return id; }
};

WX_DECLARE_OBJARRAY(TrackParam, TrackParams);

/// 単密度など特殊なトラックやセクタ情報を保持する
class DiskParticular : public TrackParam
{
private:
	TrackParams excludes;

public:
	DiskParticular();
	DiskParticular(int n_track_num, int n_side_num, int n_sector_num, int n_sectors_per_track, int n_sector_size);
	virtual ~DiskParticular() {}

	void AddExclude(const TrackParam &n_param);
	bool FindExclude(int n_track_num, int n_side_num) const;

	static void UniqueTracks(int tracks, int sides, bool both_sides, DiskParticulars &arr);
};

WX_DECLARE_OBJARRAY(DiskParticular, DiskParticulars);

bool operator==(const DiskParticulars &src, const DiskParticulars &dst);

/// DISK BASIC 名前リストを保存 
class DiskParamName
{
private:
	wxString name;
	int flags;
public:
	DiskParamName();
	~DiskParamName() {}

	void SetName(const wxString &val) { name = val; }
	const wxString &GetName() const { return name; }
	void SetFlags(int val) { flags = val; }
	int GetFlags() const { return flags; }
};

WX_DECLARE_OBJARRAY(DiskParamName, DiskParamNames);

/// ディスクパラメータ
class DiskParam
{
protected:
	wxString disk_type_name;	///< ディスク種類名 "2D" "2HD" など
	DiskParamNames basic_types;	///< BASIC種類（DiskBasicParamとのマッチングにも使用）
	bool reversible;			///< 裏返し可能 AB面あり（3インチFD）
	int sides_per_disk;			///< サイド数
	int tracks_per_side;		///< トラック数
	int sectors_per_track;		///< セクタ数
	int sector_size;			///< セクタサイズ
	int numbering_sector;		///< セクタ番号の付番方法(0:サイド毎、1:トラック毎)
	int disk_density;			///< 0x00:2D 0x10:2DD 0x20:2HD
	int interleave;				///< セクタの間隔
	DiskParticulars singles;	///< 単密度にするトラック
	DiskParticulars ptracks;	///< 特殊なトラックを定義
	DiskParticulars psectors;	///< 特殊なセクタを定義
	wxString density_name;		///< 密度情報（説明用）
	wxString description;		///< 説明

public:
	DiskParam();
	DiskParam(const DiskParam &src);
	virtual ~DiskParam() {}

	DiskParam &operator=(const DiskParam &src);
	void SetDiskParam(const DiskParam &src);
	void SetDiskParam(int n_sides_per_disk
		, int n_tracks_per_side
		, int n_sectors_per_track
		, int n_sector_size
		, int n_disk_density
		, int n_interleave
		, const DiskParticulars &n_singles
	);
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
		, const DiskParticulars &n_singles
		, const DiskParticulars &n_ptracks
		, const DiskParticulars &n_psectors
		, const wxString &n_density_name
		, const wxString &n_desc
	);

	void ClearDiskParam();
	/// 指定したパラメータで一致するものがあるか
	bool Match(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks);
	/// 指定したパラメータで一致するものがあるか
	bool Match(const DiskParam &param);
	/// 名前を除いて指定したパラメータで一致するものがあるか
	bool MatchExceptName(const DiskParam &param);
	/// 指定したパラメータに近い値で一致するものがあるか
	bool MatchNear(int num, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, bool &last);
	/// 指定したトラック、サイドが単密度か
	bool FindSingleDensity(int track_num, int side_num, int *sectors_per_track = NULL, int *sector_size = NULL) const;
	/// 単密度を持っているか
	int  HasSingleDensity(int *sectors_per_track = NULL, int *sector_size = NULL) const;
	/// ディスクサイズを計算する（ベタディスク用）
	int  CalcDiskSize() const;
	/// 特殊なトラックか
	bool FindParticularTrack(int track_num, int side_num, int &sectors_per_track, int &sector_size) const;
	/// 特殊なセクタか
	bool FindParticularSector(int track_num, int side_num, int sector_num, int &sector_size, const wxUint8 **sector_id = NULL) const;
	/// DISK BASICをさがす
	const DiskParamName *FindBasicType(const wxString &type_name, int flags = -1) const;

	/// ディスク種類名を設定 "2D" "2HD" など
	void SetDiskTypeName(const wxString &str) { disk_type_name = str; }
	/// BASIC種類を設定
	void SetBasicTypes(const DiskParamNames &arr) { basic_types = arr; }
	/// 裏返し可能 AB面ありかどうかを設定
	void Reversible(bool val) { reversible = val; }
	/// サイド数を設定
	void SetSidesPerDisk(int val) { sides_per_disk = val; }
	/// トラック数を設定
	void SetTracksPerSide(int val) { tracks_per_side = val; }
	/// セクタ数を設定
	void SetSectorsPerTrack(int val) { sectors_per_track = val; }
	/// セクタサイズを設定
	void SetSectorSize(int val) { sector_size = val; }
	/// セクタ番号の付番方法(0:サイド毎、1:トラック毎)を設定
	void SetNumberingSector(int val) { numbering_sector = val; }
	/// 密度(0x00:2D 0x10:2DD 0x20:2HD)を設定
	void SetParamDensity(int val) { disk_density = val; }
	/// セクタの間隔を設定
	void SetInterleave(int val) { interleave = val; }
	/// 単密度にするトラックを設定
	void SetSingles(const DiskParticulars &arr) { singles = arr; }
	/// 特殊なトラックを設定
	void SetParticularTracks(const DiskParticulars &arr) { ptracks = arr; }
	/// 特殊なセクタを設定
	void SetParticularSectors(const DiskParticulars &arr) { psectors = arr; }
	/// 密度情報（説明用）を設定
	void SetDensityName(const wxString &str) { density_name = str; }
	/// 説明を設定
	void SetDescription(const wxString &str) { description = str; }

	/// 単密度にするトラックを追加
	void AddSingleDensity(const DiskParticular &val) { singles.Add(val); }
	/// 特殊なトラックを追加
	void AddParticularTrack(const DiskParticular &val) { ptracks.Add(val); }
	/// 特殊なセクタを追加
	void AddParticularSector(const DiskParticular &val) { psectors.Add(val); }

	/// ディスク種類名を返す "2D" "2HD" など
	const wxString &GetDiskTypeName() const { return disk_type_name; }
	/// BASIC種類を返す
	const DiskParamNames &GetBasicTypes() const { return basic_types; }
	/// 裏返し可能 AB面ありかどうかを返す
	bool IsReversible() const { return reversible; }
	/// サイド数を返す
	int GetSidesPerDisk() const { return sides_per_disk; }
	/// トラック数を返す
	int GetTracksPerSide() const { return tracks_per_side; }
	/// セクタ数を返す
	int GetSectorsPerTrack() const { return sectors_per_track; }
	/// セクタサイズを返す
	int GetSectorSize() const { return sector_size; }
	/// セクタ番号の付番方法(0:サイド毎、1:トラック毎)を返す
	int GetNumberingSector() const { return numbering_sector; }
	/// 密度(0x00:2D 0x10:2DD 0x20:2HD)を返す
	int GetParamDensity() const { return disk_density; }
	/// セクタの間隔を返す
	int GetInterleave() const { return interleave; }
	/// 単密度にするトラックを返す
	const DiskParticulars &GetSingles() const { return singles; }
	/// 特殊なトラックを返す
	const DiskParticulars &GetParticularTracks() const { return ptracks; }
	/// 特殊なセクタを返す
	const DiskParticulars &GetParticularSectors() const { return psectors; }
	/// 密度情報（説明用）を返す
	const wxString &GetDensityName() const { return density_name; }
	/// 説明を返す
	const wxString &GetDescription() const { return description; }
	/// ディスクパラメータを含めた説明文を返す
	wxString GetDiskDescription() const;
};

WX_DECLARE_OBJARRAY(DiskParam, DiskParams);

WX_DEFINE_ARRAY(const DiskParam *, DiskParamPtrs);

class wxXmlNode;

/// ディスクパラメータのテンプレートを提供する
class DiskTemplates
{
private:
	DiskParams params;

public:
	DiskTemplates();
	~DiskTemplates() {}

	/// XMLファイルから読み込み
	bool Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs);
	bool LoadDiskBasicTypes(const wxXmlNode *node, DiskParamNames &basic_types, wxString &errmsgs);
	bool LoadSingleDensity(const wxXmlNode *node, DiskParticular &s, wxString &errmsgs);
	bool LoadParticularTrack(const wxXmlNode *node, DiskParticular &d, wxString &errmsgs);
	bool LoadParticularSector(const wxXmlNode *node, DiskParticular &d, wxString &errmsgs);
	bool LoadDescription(const wxXmlNode *node, const wxString &locale_name, wxString &desc, wxString &desc_locale);

	/// タイプ名に一致するテンプレートの番号を返す
	int IndexOf(const wxString &n_type_name) const;
	/// 一致するテンプレートを返す
	const DiskParam *Find(const DiskParam &n_param) const;
	/// タイプ名に一致するテンプレートを返す
	const DiskParam *Find(const wxString &n_type_name) const;
	/// パラメータに一致するテンプレートを返す
	const DiskParam *FindStrict(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks) const;
	/// パラメータに一致するあるいは近い物のテンプレートを返す
	const DiskParam *Find(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const DiskParticulars &n_singles, const DiskParticulars &n_ptracks) const;
	/// テンプレートを返す
	const DiskParam *ItemPtr(size_t index) const { return &params[index]; }
	/// テンプレートを返す
	const DiskParam &Item(size_t index) const { return params[index]; }
	/// テンプレートの数を返す
	size_t Count() const { return params.Count(); }
};

extern DiskTemplates gDiskTemplates;

#endif /* _DISK_PARAMETER_H_ */
