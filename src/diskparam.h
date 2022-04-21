/// @file diskparam.h
///
/// @brief ディスクパラメータ
///
#ifndef _DISK_PARAMETER_H_
#define _DISK_PARAMETER_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/dynarray.h>

/// (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
extern const int gSectorSizes[5];

///
class SingleDensity
{
public:
	int track_num;
	int side_num;
	SingleDensity(int n_track_num, int n_side_num);
	~SingleDensity() {}

	bool operator==(const SingleDensity &dst) const;
};

WX_DECLARE_OBJARRAY(SingleDensity, SingleDensities);

bool operator==(const SingleDensities &src, const SingleDensities &dst);

/// ディスクパラメータ
class DiskParam
{
protected:
	wxString disk_type_name;	///< "2D" "2HD" など（DiskBasicParamとのマッチングにも使用）
	wxUint32 disk_type;			///< 1: AB面あり（3インチFD）
	int sides_per_disk;			///< サイド数
	int tracks_per_side;		///< トラック数
	int sectors_per_track;		///< セクタ数
	int sector_size;			///< セクタサイズ
	int density;				///< 0:2D 1:2DD 2:2HD
	SingleDensities singles;	///< 単密度にするトラック

public:
	DiskParam();
	DiskParam(const DiskParam &src);
	DiskParam(const wxString &n_type_name, wxUint32 n_disk_type, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, const SingleDensities &n_singles);
	virtual ~DiskParam() {}

	void SetDiskParam(const DiskParam &src);
	void SetDiskParam(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, const SingleDensities &n_singles);
	void SetDiskParam(const wxString &n_type_name, wxUint32 n_disk_type, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density, const SingleDensities &n_singles);
	void SetDiskParam(const wxString &n_type_name, wxUint32 n_disk_type, int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, int n_density);

	void ClearDiskParam();
	bool Match(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, const SingleDensities &n_singles);
	bool Match(const DiskParam &param);
	bool MatchNear(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size);
	bool FindSingleDensity(int track_num, int side_num) const;
	const wxString &GetDiskTypeName() const { return disk_type_name; }
	wxUint32 GetDiskType() const { return disk_type; }
	int GetSidesPerDisk() const { return sides_per_disk; }
	int GetTracksPerSide() const { return tracks_per_side; }
	int GetSectorsPerTrack() const { return sectors_per_track; }
	int GetSectorSize() const { return sector_size; }
	int GetDensity() const { return density; }
	const SingleDensities &GetSingles() const { return singles; }
};

WX_DECLARE_OBJARRAY(DiskParam, DiskParams);

///
class DiskTypes
{
private:
	DiskParams types;

public:
	DiskTypes();
	~DiskTypes() {}

	bool Load(const wxString &data_path);

	int IndexOf(const wxString &n_type_name);
	DiskParam *Find(int n_sides_per_disk, int n_tracks_per_side, int n_sectors_per_track, int n_sector_size, const SingleDensities &n_singles);
	DiskParam *ItemPtr(size_t index) const { return &types[index]; }
	DiskParam &Item(size_t index) const { return types[index]; }
	size_t Count() const { return types.Count(); }
#if 0
	const wxString &GetTypeName(int num) { return types[num].GetTypeName(); }
	int GetSidesPerDisk(int num) const { return types[num].GetSidesPerDisk(); }
	int GetTracksPerSide(int num) const { return types[num].GetTracksPerSide(); }
	int GetSectorsPerTrack(int num) const { return types[num].GetSectorsPerTrack(); }
	int GetSectorSize(int num) const { return types[num].GetSectorSize(); }
#endif
};

extern DiskTypes gDiskTypes;

#endif /* _DISK_PARAMETER_H_ */
