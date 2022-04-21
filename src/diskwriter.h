/// @file diskwriter.h
///
/// @brief ディスクライター
///
#ifndef _DISK_WRITER_H_
#define _DISK_WRITER_H_

#include "common.h"
#include <wx/string.h>
#include "diskd88.h"


class wxOutputStream;

/// ディスクライター
class DiskWriter : public DiskWriteOptions
{
private:
	wxString		file_path;
	DiskD88			*d88;
	wxOutputStream	*ostream;
	bool			ownstream;
	DiskResult		*result;

	DiskWriter();
	DiskWriter(const DiskWriter &src);

	// 拡張子をさがす
	int SaveDiskByExt(int disk_number, int side_number, bool &support);
	// 拡張子で保存形式を判定
	int SelectSaveDisk(const wxString &type, int disk_number, int side_number, bool &support);

public:
	DiskWriter(DiskD88 *image, const wxString &path, const DiskWriteOptions &options, DiskResult *result);
	~DiskWriter();

	/// 出力先を開く
	int Open(const wxString &path);
	/// 出力先がオープンしているか
	bool IsOk() const;

	/// ディスクイメージの保存
	int Save(const wxString &file_format = wxEmptyString);
	/// ストリームの内容をファイルに保存
	int SaveDisk(int disk_number, int side_number, const wxString &file_format = wxEmptyString);
};

#endif /* _DISK_WRITER_H_ */
