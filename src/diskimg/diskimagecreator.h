/// @file diskimagecreator.h
///
/// @brief ディスクイメージ作成
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKIMAGE_CREATOR_H
#define DISKIMAGE_CREATOR_H

#include "../common.h"
#include <wx/string.h>


class DiskParam;
class DiskImageTrack;
class DiskImageDisk;
class DiskImageFile;
class DiskResult;

/// ディスクイメージの新規作成
class DiskImageCreator
{
private:
	wxString		 m_diskname;
	const DiskParam *p_param;
	bool			 m_write_protect;
	DiskImageFile	*p_file;
	DiskResult		*p_result;

	/// セクタデータの作成
	wxUint32 CreateSector(int track_number, int side_number, int sector_number, int sector_size, int sectors_per_track, DiskImageTrack *track);
	/// ディスクデータの作成
	wxUint32 CreateDisk(int disk_number, short mod_flags);

public:
	DiskImageCreator(const wxString &diskname, const DiskParam &param, bool write_protect, DiskImageFile *file, DiskResult &result);
	~DiskImageCreator();

	/// トラックデータの作成
	wxUint32 CreateTrack(int track_number, int side_number, int offset_pos, wxUint32 offset, DiskImageDisk *disk);

	/// ディスクイメージの新規作成
	int Create();
	/// 新規作成して既存のイメージに追加
	int Add();
};

#endif /* DISKIMAGE_CREATOR_H */
