/// @file diskd88writer.h
///
/// @brief D88ディスクライター
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKD88_WRITER_H
#define DISKD88_WRITER_H

#include "../common.h"
#include "diskwriter.h"


class wxOutputStream;
class DiskWriter;
class DiskImage;
class DiskImageDisk;
class DiskResult;

/// D88形式ディスクライター
class DiskD88Writer : public DiskImageWriter
{
private:
	/// ディスク1つを保存
	int SaveDisk(DiskImageDisk *disk, int side_number, wxOutputStream *ostream);

public:
	DiskD88Writer(DiskWriter *dw_, DiskResult *result_);

	/// ストリームの内容をファイルに保存できるか
	int ValidateDisk(DiskImage *image, int disk_number, int side_number);
	/// ストリームの内容をファイルに保存
	int SaveDisk(DiskImage *image, int disk_number, int side_number, wxOutputStream *ostream);
};

#endif /* DISKD88_WRITER_H */
