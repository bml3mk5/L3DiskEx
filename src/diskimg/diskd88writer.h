/// @file diskd88writer.h
///
/// @brief D88ディスクライター
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKD88_WRITER_H_
#define _DISKD88_WRITER_H_

#include "../common.h"
#include "diskwriter.h"


class wxOutputStream;
class DiskWriter;
class DiskD88;
class DiskD88Disk;
class DiskResult;

/// D88形式ディスクライター
class DiskD88Writer : public DiskInhWriterBase
{
private:
	/// ディスク1つを保存
	int SaveDisk(DiskD88Disk *disk, int side_number, wxOutputStream *ostream);

public:
	DiskD88Writer(DiskWriter *dw_, DiskResult *result_);

	/// ストリームの内容をファイルに保存できるか
	int ValidateDisk(DiskD88 *image, int disk_number, int side_number);
	/// ストリームの内容をファイルに保存
	int SaveDisk(DiskD88 *image, int disk_number, int side_number, wxOutputStream *ostream);
};

#endif /* _DISKD88_WRITER_H_ */
