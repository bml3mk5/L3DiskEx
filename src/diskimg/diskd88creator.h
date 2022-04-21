/// @file diskd88creator.h
///
/// @brief D88ディスクイメージ作成
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKD88_CREATOR_H_
#define _DISKD88_CREATOR_H_

#include "../common.h"
#include <wx/string.h>


class DiskParam;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskResult;

/// D88ディスクの新規作成
class DiskD88Creator
{
private:
	wxString diskname;
	const DiskParam *param;
	bool write_protect;
	DiskD88File *file;
	DiskResult *result;

	/// セクタデータの作成
	wxUint32 CreateSector(int track_number, int side_number, int sector_number, int sector_size, int sectors_per_track, bool single_density, DiskD88Track *track);
	/// ディスクデータの作成
	wxUint32 CreateDisk(int disk_number, short mod_flags);

public:
	DiskD88Creator(const wxString &diskname, const DiskParam &param, bool write_protect, DiskD88File *file, DiskResult &result);
	~DiskD88Creator();

	/// トラックデータの作成
	wxUint32 CreateTrack(int track_number, int side_number, int offset_pos, wxUint32 offset, DiskD88Disk *disk);

	/// ディスクイメージの新規作成
	int Create();
	/// 新規作成して既存のイメージに追加
	int Add();
};

#endif /* _DISKD88_CREATOR_H_ */
