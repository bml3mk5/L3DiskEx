/// @file diskplainwriter.h
///
/// @brief べたディスクライター
///
#ifndef _DISKPLAIN_WRITER_H_
#define _DISKPLAIN_WRITER_H_

#include "common.h"

class wxOutputStream;
class DiskD88;
class DiskD88Disk;
class DiskResult;

/// べたディスクライター
class DiskPlainWriter
{
private:
	DiskResult *result;

	/// ディスク1つを保存
	int SaveDisk(DiskD88Disk *disk, wxOutputStream *ostream);
	/// ディスクの1つサイドだけを保存(1S用)
	int SaveDisk(DiskD88Disk *disk, int side_number, wxOutputStream *ostream);

public:
	DiskPlainWriter(DiskResult *result);
	~DiskPlainWriter();

	/// ストリームの内容をファイルに保存
	int SaveDisk(DiskD88 *image, int disk_number, int side_number, wxOutputStream *ostream);
};

#endif /* _DISKPLAIN_WRITER_H_ */
