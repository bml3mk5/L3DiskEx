/// @file diskwriter.cpp
///
/// @brief ディスクライター
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskwriter.h"
#include <wx/filename.h>
#include <wx/wfstream.h>
#include "../diskd88.h"
#include "diskd88writer.h"
#include "diskplainwriter.h"
#include "fileparam.h"
#include "diskresult.h"


//////////////////////////////////////////////////////////////////////
//
//
//
DiskWriter::DiskWriter()
	: DiskWriteOptions()
{
	this->d88 = NULL;
	this->ostream = NULL;
	this->result = NULL;
}
DiskWriter::DiskWriter(const DiskWriter &src)
	: DiskWriteOptions(src)
{
}

/// @param [in,out] image ディスクイメージ
/// @param [in] path      ファイルパス
/// @param [in] options   出力時のオプション
/// @param [out] result   結果
DiskWriter::DiskWriter(DiskD88 *image, const wxString &path, const DiskWriteOptions &options, DiskResult *result)
	: DiskWriteOptions(options)
{
	this->d88 = image;
	this->file_path = path;
	this->result = result;
	this->Open(path);
}

/// @param [in]  image    ディスクイメージ
/// @param [out] result   結果
DiskWriter::DiskWriter(DiskD88 *image, DiskResult *result)
	: DiskWriteOptions()
{
	this->d88 = image;
	this->file_path = wxEmptyString;
	this->result = result;
	this->ostream = NULL;
	this->ownstream = false;
}

DiskWriter::~DiskWriter()
{
	if (ownstream) {
		delete ostream;
	}
}

/// 出力先を開く
/// @param [in] path 出力先ファイルパス
/// @return 結果
int DiskWriter::Open(const wxString &path)
{
	wxFileOutputStream *fstream = new wxFileOutputStream(path);
//	if (!fstream->IsOk()) {
//		result->SetError(DiskResult::ERR_CANNOT_SAVE);
//	}
	ostream = fstream;
	ownstream = true;
	return result->GetValid();
}

/// 出力先がオープンしているか
bool DiskWriter::IsOk() const
{
	return ostream ? ostream->IsOk() : false;
}

/// 拡張子をさがす
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
int DiskWriter::CanSaveDiskByExt(int disk_number, int side_number)
{
	int  rc = 0;

	// ファイル形式の指定がない場合
	wxFileName fpath(file_path);

	// 拡張子で判定
	wxString ext = fpath.GetExt();

	// サポートしているファイルか
	FileParam *fitem = gFileTypes.FindExt(ext);
	if (!fitem) {
		result->SetError(DiskResult::ERR_UNSUPPORTED);
		return result->GetValid();
	}

	// 指定した形式でファイル出力
	const FileParamFormats *formats = &fitem->GetFormats();
	for(size_t i=0; i<formats->Count(); i++) {
		const FileParamFormat *param_format = &formats->Item(i);
		rc = SelectCanSaveDisk(param_format->GetType(), disk_number, side_number);
		if (rc >= 0) {
			break;
		}
	}

	return rc;
}

/// 拡張子をさがす
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [out] support    対応しているフォーマットならtrue
int DiskWriter::SaveDiskByExt(int disk_number, int side_number, bool &support)
{
	int  rc = 0;

	// ファイル形式の指定がない場合
	wxFileName fpath(file_path);

	// 拡張子で判定
	wxString ext = fpath.GetExt();

	// サポートしているファイルか
	FileParam *fitem = gFileTypes.FindExt(ext);
	if (!fitem) {
		result->SetError(DiskResult::ERR_UNSUPPORTED);
		return result->GetValid();
	}

	// 指定した形式でファイル出力
	const FileParamFormats *formats = &fitem->GetFormats();
	for(size_t i=0; i<formats->Count(); i++) {
		const FileParamFormat *param_format = &formats->Item(i);
		rc = SelectSaveDisk(param_format->GetType(), disk_number, side_number, support);
		if (rc >= 0) {
			break;
		}
	}

	return rc;
}

/// ディスクイメージを保存できるか
/// @param [in] file_format ファイルフォーマット
/// @retval 0:できる
/// @retval 1:警告あり
int DiskWriter::CanSave(const wxString &file_format)
{
	return CanSaveDisk(-1, -1, file_format);
}

/// ストリームの内容をファイルに保存できるか
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] file_format ファイルフォーマット
/// @retval 0:できる
/// @retval 1:警告あり
int DiskWriter::CanSaveDisk(int disk_number, int side_number, const wxString &file_format)
{
	int  rc = 0;
	if (file_format.IsEmpty()) {
		// ファイル形式の指定がない場合
		rc = CanSaveDiskByExt(disk_number, side_number);
	} else {
		// ファイル形式の指定あり
		rc = SelectCanSaveDisk(file_format, disk_number, side_number);
	}
	return rc;
}

/// ディスクイメージの保存
/// @param [in] file_format ファイルフォーマット
int DiskWriter::Save(const wxString &file_format)
{
	return SaveDisk(-1, -1, file_format);
}

/// ストリームの内容をファイルに保存
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [in] file_format ファイルフォーマット
int DiskWriter::SaveDisk(int disk_number, int side_number, const wxString &file_format)
{
	int  rc = 0;
	bool support = false;

	if (!IsOk()) {
		result->SetError(DiskResult::ERR_CANNOT_SAVE);
		return result->GetValid();
	}

	if (file_format.IsEmpty()) {
		// ファイル形式の指定がない場合
		rc = SaveDiskByExt(disk_number, side_number, support);
	} else {
		// ファイル形式の指定あり
		rc = SelectSaveDisk(file_format, disk_number, side_number, support);
	}
	if (!support) {
		result->SetError(DiskResult::ERR_UNSUPPORTED);
		return result->GetValid();
	}
	return rc;
}

/// 拡張子で保存形式を判定＆保存できるか
/// @param [in] file_format ファイルフォーマット
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
int DiskWriter::SelectCanSaveDisk(const wxString &file_format, int disk_number, int side_number)
{
	int rc = -1;
	if (file_format == wxT("d88")) {
		// d88形式
		DiskD88Writer wr(this, result);
		rc = wr.ValidateDisk(d88, disk_number, side_number);
//	} else if (file_format == wxT("cpcdsk")) {
//		// CPC DSK形式
//		DiskDskWriter wr(result);
//		rc = wr.ValidateDisk(d88, disk_number, side_number);
	} else if (file_format == wxT("plain")) {
		// ベタ
		DiskPlainWriter wr(this, result);
		rc = wr.ValidateDisk(d88, disk_number, side_number);
	}
	return rc;
}


/// 拡張子で保存形式を判定
/// @param [in] file_format ファイルフォーマット
/// @param [in] disk_number ディスク番号
/// @param [in] side_number サイド番号
/// @param [out] support    対応しているフォーマットならtrue
int DiskWriter::SelectSaveDisk(const wxString &file_format, int disk_number, int side_number, bool &support)
{
	int rc = -1;
	if (file_format == wxT("d88")) {
		// d88形式
		DiskD88Writer wr(this, result);
		rc = wr.SaveDisk(d88, disk_number, side_number, ostream);
		support = true;
//	} else if (file_format == wxT("cpcdsk")) {
//		// CPC DSK形式
//		DiskDskWriter wr(result);
//		rc = wr.SaveDisk(d88, disk_number, side_number, ostream);
//		support = true;
	} else if (file_format == wxT("plain")) {
		// ベタ
		DiskPlainWriter wr(this, result);
		rc = wr.SaveDisk(d88, disk_number, side_number, ostream);
		support = true;
	}
	if (support && rc >= 0) {
		// 保存したファイル名を持っておく
		d88->SetFileName(file_path);
	}
	return rc;
}

//////////////////////////////////////////////////////////////////////
//
// 形式ごとの保存
//
DiskInhWriterBase::DiskInhWriterBase(DiskWriter *dw_, DiskResult *result_)
{
	this->dw = dw_;
	this->result = result_;
}

DiskInhWriterBase::~DiskInhWriterBase()
{
}

/// ストリームの内容をファイルに保存できるか
/// @param [in,out] image ディスクイメージ
/// @param [in]     disk_number ディスク番号(0-) / -1のときは全体 
/// @param [in]     side_number サイド番号(0-) / -1のときは両面 
/// @retval  0 正常
int DiskInhWriterBase::ValidateDisk(DiskD88 *image, int disk_number, int side_number)
{
	result->Clear();

	return 0;
}

/// ストリームの内容をファイルに保存
/// @param [in,out] image ディスクイメージ
/// @param [in]     disk_number ディスク番号(0-) / -1のときは全体 
/// @param [in]     side_number サイド番号(0-) / -1のときは両面 
/// @param [out]    ostream     出力先
/// @retval  0 正常
int DiskInhWriterBase::SaveDisk(DiskD88 *image, int disk_number, int side_number, wxOutputStream *ostream)
{
	result->Clear();

	return 0;
}
