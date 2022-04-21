/// @file diskwriter.cpp
///
/// @brief ディスクライター
///
#include "diskwriter.h"
#include <wx/filename.h>
#include <wx/wfstream.h>
#include "diskd88.h"
#include "diskd88writer.h"
#include "diskplainwriter.h"
#include "fileparam.h"
#include "diskresult.h"


DiskWriter::DiskWriter()
{
	this->d88 = NULL;
	this->ostream = NULL;
	this->result = NULL;
}
DiskWriter::DiskWriter(const DiskWriter &src)
{
}

/// @param [in,out] image ディスクイメージ
/// @param [in] path      ファイルパス
/// @param [out] result   結果
DiskWriter::DiskWriter(DiskD88 *image, const wxString &path, DiskResult *result)
{
	this->d88 = image;
	this->file_path = path;
	this->result = result;
	this->Open(path);
}
//DiskWriter::DiskWriter(wxOutputStream *stream, DiskResult &result)
//{
//	this->ostream = stream;
//}

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

// 拡張子をさがす
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

/// ディスクイメージの保存
/// @param [in] file_format ファイルフォーマット
int DiskWriter::Save(const wxString &file_format)
{
	return SaveDisk(-1, -1, file_format);
}

/// ストリームの内容をファイルに保存
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

// 拡張子で保存形式を判定
int DiskWriter::SelectSaveDisk(const wxString &type, int disk_number, int side_number, bool &support)
{
	int rc = -1;
	if (type == wxT("d88")) {
		// d88形式
		DiskD88Writer wr(result);
		rc = wr.SaveDisk(d88, disk_number, side_number, ostream);
		support = true;
//	} else if (type == wxT("cpcdsk")) {
//		// CPC DSK形式
//		DiskDskWriter wr(result);
//		rc = wr.SaveDisk(d88, disk_number, side_number, ostream);
//		support = true;
	} else if (type == wxT("plain")) {
		// ベタ
		DiskPlainWriter wr(result);
		rc = wr.SaveDisk(d88, disk_number, side_number, ostream);
		support = true;
	}
	if (support && rc >= 0) {
		// 保存したファイル名を持っておく
		d88->SetFileName(file_path);
	}
	return rc;
}

