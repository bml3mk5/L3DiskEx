/// @file diskparser.cpp
///
/// @brief ディスクパーサー
///
#include "diskparser.h"
#include <wx/arrstr.h>
#include "diskplainparser.h"
#include "diskd88parser.h"
#include "diskdskparser.h"
#include "diskd88.h"
#include "fileparam.h"
#include "diskresult.h"

/// コンストラクタ
/// @param [in]     filepath    解析するファイルのパス 
/// @param [in]     stream      上記ファイルのストリーム   
/// @param [in,out] file        既存のディスクイメージ
/// @param [out]    result      結果
DiskParser::DiskParser(const wxString &filepath, wxInputStream *stream, DiskD88File *file, DiskResult &result)
{
	this->filepath = wxFileName(filepath);
	this->stream = stream;
	this->file = file;
	this->result = &result;
}

DiskParser::~DiskParser()
{
}

/// ディスクイメージの解析
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] disk_type   ディスクパラメータ種類
/// @param [in] mod_flags   オープン/追加 DiskD88File::Add()
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskParser::Parse(const wxString &file_format, const wxString &disk_type, short mod_flags)
{
	bool support = false;
	int rc = -1;

	if (file_format.IsEmpty()) {
		// ファイル形式の指定がない場合

		// 拡張子で判定
		wxString ext = filepath.GetExt();

		// サポートしているファイルか
		FileParam *fitem = gFileTypes.FindExt(ext);
		if (!fitem) {
			result->SetError(DiskResult::ERR_UNSUPPORTED);
			return result->GetValid();
		}

		// 指定形式で解析する
		const FileParamFormats *formats = &fitem->GetFormats();
		for(size_t i=0; i<formats->Count(); i++) {
			const FileParamFormat *param_format = &formats->Item(i);
			rc = SelectPerser(param_format->GetType(), &param_format->GetHints(), NULL, mod_flags, support);
			if (rc >= 0) {
				break;
			}
		}
	} else {
		// ファイル形式の指定あり

		DiskParam *param = gDiskTypes.Find(disk_type);

		rc = SelectPerser(file_format, NULL, param, mod_flags, support);
	}
	if (!support) {
		result->SetError(DiskResult::ERR_UNSUPPORTED);
		return result->GetValid();
	}
	return rc;
}

/// ファイルの解析方法を選択
int DiskParser::SelectPerser(const wxString &type, const wxArrayString *disk_hints, const DiskParam *disk_param, short mod_flags, bool &support)
{
	int rc = -1;
	if (type == wxT("d88")) {
		// d88形式
		DiskD88Parser ps(file, mod_flags, result);
		rc = ps.Parse(stream);
		support = true;
	} else if (type == wxT("cpcdsk")) {
		// CPC DSK形式
		DiskDskParser ps(file, mod_flags, result);
		if (ps.Check(stream)) {
			rc = ps.Parse(stream);
		}
		support = true;
	} else if (type == wxT("plain")) {
		// ベタ
		DiskPlainParser ps(file, mod_flags, result);
		rc = ps.Parse(stream, disk_hints, disk_param);
		support = true;
	}
	return rc;
}

/// ディスクイメージを新たに解析する
int DiskParser::Parse(const wxString &file_format, const wxString &disk_type)
{
	return Parse(file_format, disk_type, DiskD88File::MODIFY_NONE);
}

/// 指定ディスクを解析してこれを既存のディスクイメージに追加する
int DiskParser::ParseAdd(const wxString &file_format, const wxString &disk_type)
{
	return Parse(file_format, disk_type, DiskD88File::MODIFY_ADD);
}
