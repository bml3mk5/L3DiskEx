/// @file diskparam.h
///
/// @brief ディスクパラメータ
///
#ifndef _DISK_PARAMETER_H_
#define _DISK_PARAMETER_H_

#include "common.h"
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/dynarray.h>

/// (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
extern const int gSectorSizes[5];

class SingleDensities;

/// 単密度情報を保持する
class SingleDensity
{
private:
	int track_num;
	int side_num;
	int sectors_per_track;
	int sector_size;

	SingleDensity() {}

public:
	SingleDensity(int n_track_num, int n_side_num, int n_sectors_per_track, int n_sector_size);
	~SingleDensity() {}

	bool operator==(const SingleDensity &dst) const;
	bool Match(int n_track_num, int n_side_num) const;

	int GetTrackNumber() const { return track_num; }
	int GetSideNumber() const { return side_num; }
	int GetSectorsPerTrack() const { return sectors_per_track; }
	int GetSectorSize() const { return sector_size; }

	static void Unique(int tracks, int sides, bool single_type, SingleDensities &arr);
};

WX_DECLARE_OBJARRAY(SingleDensity, SingleDensities);

bool operator==(const SingleDensities &src, const SingleDensities &dst);

/// ディスクパラメータ
class DiskParam
{
protected:
	wxString disk_type_name;	///< "2D" "2HD" など
//	wxUint32 disk_type;			///< 1: AB面あり（3インチFD）
	wxArrayString basic_types;	///< BASIC種類（DiskBasicParamとのマッチングにも使用）
	bool reversible;			///< 裏返し可能 AB面あり（3インチFD）
	int sides_per_disk;			///< サイド数
	int tracks_per_side;		///< トラック数
	int sectors_per_track;		///< セクタ数
	int sector_size;			///< セクタサイズ
	int numbering_sector;		///< セクタ番号の付番方法(0:サイド毎、1:トラック毎)
	int density;				///< 0:2D 1:2DD 2:2HD
	int interleave;				///< セクタの間隔
	SingleDensities singles;	///< 単密度にするトラック
	wxString density_name;		///< 密度情報（説明用）
	wxString description;		///< 説明

public:
	DiskParam();
	DiskParam(const DiskParam &src);
	DiskParam(const wxString &n_type_name
//		, wxUint32 n_disk_type
		, const wxArrayString &n_basic_types
		, bool n_reversible
		, int n_sides_per_disk
		, int n_tracks_per_side
		, int n_sectors_per_track
		, int n_sector_size
		, int n_numbering_sector
		, int n_density
		, int n_interleave
		, const SingleDensities &n_singles
		, const wxString &n_density_name
		, const wxString &n_desc
	);
	virtual ~DiskParam() {}

	DiskParam &operator=(const DiskParam &src);
	void SetDiskParam(const DiskParam &src);
	void SetDiskParam(int n_sides_per_disk
		, int n_tracks_per_side
		, int n_sectors_per_track
		, int n_sector_size
		, int n_density
		, int n_interleave
		, const SingleDensities &n_singles
	);
	void SetDiskParam(const wxString &n_type_name
//		, wxUint32 n_disk_type
		, const wxArrayString &n_basic_types
		, bool n_reversible
		, int n_sides_per_disk
		, int n_tracks_per_side
		, int n_sectors_per_track
		, int n_sector_size
		, int n_numbering_sector
		, int n_density
		, int n_interleave
		, const SingleDensities &n_singles
		, const wxString &n_density_name
		, const wxString &n_desc
	);

	void ClearDiskParam();
	/// 指定したパラメータで一致するものがあるか
	bool Match(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const SingleDensities &n_singles);
	/// 指定したパラメータで一致するものがあるか
	bool Match(const DiskParam &param);
	/// 指定したパラメータに近い値で一致するものがあるか
	bool MatchNear(int num, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const SingleDensities &n_singles, bool &last);
	/// 指定したトラック、サイドが単密度か
	bool FindSingleDensity(int track_num, int side_num, int *sectors_per_track = NULL, int *sector_size = NULL) const;
	/// 単密度を持っているか
	int  HasSingleDensity(int *sectors_per_track = NULL, int *sector_size = NULL) const;
	/// ディスクサイズを計算する（ベタディスク用）
	int  CalcDiskSize() const;

	const wxString &GetDiskTypeName() const { return disk_type_name; }
//	wxUint32 GetDiskType() const { return disk_type; }
	const wxArrayString &GetBasicTypes() const { return basic_types; }
	bool IsReversible() const { return reversible; }
	int GetSidesPerDisk() const { return sides_per_disk; }
	int GetTracksPerSide() const { return tracks_per_side; }
	int GetSectorsPerTrack() const { return sectors_per_track; }
	int GetSectorSize() const { return sector_size; }
	int GetNumberingSector() const { return numbering_sector; }
	int GetParamDensity() const { return density; }
	int GetInterleave() const { return interleave; }
	void SetInterleave(int val) { interleave = val; }
	const SingleDensities &GetSingles() const { return singles; }
	const wxString &GetDensityName() const { return density_name; }
	const wxString &GetDescription() const { return description; }
	wxString GetDiskDescription() const;
};

WX_DECLARE_OBJARRAY(DiskParam, DiskParams);

WX_DEFINE_ARRAY(const DiskParam *, DiskParamPtrs);

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

	/// タイプ名に一致するテンプレートの番号を返す
	int IndexOf(const wxString &n_type_name);
	/// タイプ名に一致するテンプレートを返す
	DiskParam *Find(const wxString &n_type_name);
	/// パラメータに一致するあるいは近い物のテンプレートを返す
	DiskParam *Find(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_interleave, int n_numbering_sector, const SingleDensities &n_singles);

	DiskParam *ItemPtr(size_t index) const { return &params[index]; }
	DiskParam &Item(size_t index) const { return params[index]; }
	size_t Count() const { return params.Count(); }
};

extern DiskTemplates gDiskTemplates;

#endif /* _DISK_PARAMETER_H_ */
