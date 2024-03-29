/// @file diskplainwriter.h
///
/// @brief べたディスクライター
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKPLAIN_WRITER_H
#define DISKPLAIN_WRITER_H

#include "../common.h"
#include "diskwriter.h"


class wxOutputStream;
class DiskWriter;
class DiskImage;
class DiskImageDisk;
class DiskResult;

/// べたディスクライター
class DiskPlainWriter : public DiskImageWriter
{
private:
	/// ディスク1つを保存
	int SaveDisk(DiskImageDisk *disk, int side_number, wxOutputStream *ostream);

public:
	DiskPlainWriter(DiskWriter *dw_, DiskResult *result_);

	/// ストリームの内容をファイルに保存
	int SaveDisk(DiskImage *image, int disk_number, int side_number, wxOutputStream *ostream);
};

#endif /* DISKPLAIN_WRITER_H */
