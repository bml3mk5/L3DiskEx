/// @file diskd88creator.h
///
/// @brief D88ディスクイメージ作成
///
#ifndef _DISKD88_CREATOR_H_
#define _DISKD88_CREATOR_H_

#include "common.h"
#include <wx/wx.h>

class DiskParam;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskD88Result;

/// D88ディスク作成
class DiskD88Creator
{
private:
	wxString diskname;
	const DiskParam *param;
	bool write_protect;
	DiskD88File *file;
	DiskD88Result *result;

public:
	DiskD88Creator(const wxString &diskname, const DiskParam &param, bool write_protect, DiskD88File *file, DiskD88Result &result);
	~DiskD88Creator();

	wxUint32 CreateSector(int track_number, int side_number, int sector_number, DiskD88Track *track);
	wxUint32 CreateTrack(int track_number, int side_number, int offset_pos, wxUint32 offset, DiskD88Disk *disk);
	wxUint32 CreateDisk(int disk_number);

	int Create();
	int Add();
};

#endif /* _DISKD88_CREATOR_H_ */
