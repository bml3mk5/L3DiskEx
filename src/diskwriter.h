/// @file diskwriter.h
///
/// @brief ディスクライター
///
/// @author Copyright (c) Sasaji. All rights reserved.
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
	int CanSaveDiskByExt(int disk_number, int side_number);
	// 拡張子で保存形式を判定
	int SelectCanSaveDisk(const wxString &file_format, int disk_number, int side_number);
	// 拡張子をさがす
	int SaveDiskByExt(int disk_number, int side_number, bool &support);
	// 拡張子で保存形式を判定
	int SelectSaveDisk(const wxString &file_format, int disk_number, int side_number, bool &support);

public:
	DiskWriter(DiskD88 *image, const wxString &path, const DiskWriteOptions &options, DiskResult *result);
	DiskWriter(DiskD88 *image, DiskResult *result);
	~DiskWriter();

	/// 出力先を開く
	int Open(const wxString &path);
	/// 出力先がオープンしているか
	bool IsOk() const;

	/// ディスクイメージを保存できるか
	int CanSave(const wxString &file_format);
	/// ストリームの内容をファイルに保存できるか
	int CanSaveDisk(int disk_number, int side_number, const wxString &file_format);
	/// ディスクイメージの保存
	int Save(const wxString &file_format);
	/// ストリームの内容をファイルに保存
	int SaveDisk(int disk_number, int side_number, const wxString &file_format);
};

/// 形式ごとのディスクライター
class DiskInhWriterBase
{
protected:
	DiskWriter *dw;
	DiskResult *result;

public:
	DiskInhWriterBase(DiskWriter *dw_, DiskResult *result_);
	virtual ~DiskInhWriterBase();

	/// ストリームの内容をファイルに保存できるか
	virtual int ValidateDisk(DiskD88 *image, int disk_number, int side_number);
	/// ストリームの内容をファイルに保存
	virtual int SaveDisk(DiskD88 *image, int disk_number, int side_number, wxOutputStream *ostream);
};

#endif /* _DISK_WRITER_H_ */
