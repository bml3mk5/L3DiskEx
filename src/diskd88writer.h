/// @file diskd88writer.h
///
/// @brief D88ディスクライター
///
#ifndef _DISKD88_WRITER_H_
#define _DISKD88_WRITER_H_

#include "common.h"

class wxOutputStream;
class DiskD88;
class DiskD88Disk;
class DiskResult;

/// D88形式ディスクライター
class DiskD88Writer
{
private:
	DiskResult *result;

	/// ディスク1つを保存
	int SaveDisk(DiskD88Disk *disk, wxOutputStream *ostream);
	/// ディスクの1つサイドだけを保存(1S用)
	int SaveDisk(DiskD88Disk *disk, int side_number, wxOutputStream *ostream);

public:
	DiskD88Writer(DiskResult *result);
	~DiskD88Writer();

//	/// ストリームの内容をファイルに保存
//	int Save(DiskD88 *image, wxOutputStream *ostream);
	/// ストリームの内容をファイルに保存
	int SaveDisk(DiskD88 *image, int disk_number, int side_number, wxOutputStream *ostream);
};

#endif /* _DISKD88_WRITER_H_ */
