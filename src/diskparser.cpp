/// @file diskparser.cpp
///
/// @brief ディスクパーサー
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskparser.h"
#include "diskplainparser.h"
#include "diskd88parser.h"
#include "diskdskparser.h"
#include "diskfdiparser.h"
#include "diskcqmparser.h"
#include "disktd0parser.h"
#include "diskdimparser.h"
#include "diskvfdparser.h"
#include "diskimdparser.h"
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

/// ディスクイメージを新たに解析する
/// @param [in] file_format      ファイルの形式名("d88","plain"など)
/// @param [in] param_hint       ディスクパラメータヒント("plain"時のみ)
int DiskParser::Parse(const wxString &file_format, const DiskParam &param_hint)
{
	return Parse(file_format, param_hint, DiskD88File::MODIFY_NONE);
}

/// 指定ディスクを解析してこれを既存のディスクイメージに追加する
/// @param [in] file_format      ファイルの形式名("d88","plain"など)
/// @param [in] param_hint       ディスクパラメータヒント("plain"時のみ)
int DiskParser::ParseAdd(const wxString &file_format, const DiskParam &param_hint)
{
	return Parse(file_format, param_hint, DiskD88File::MODIFY_ADD);
}

/// ディスクイメージの解析
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] param_hint  ディスクパラメータヒント("plain"時のみ)
/// @param [in] mod_flags   オープン/追加 DiskD88File::Add()
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskParser::Parse(const wxString &file_format, const DiskParam &param_hint, short mod_flags)
{
	bool support = false;
	int rc = -1;

	image_type.Empty();
	if (!file_format.IsEmpty()) {
		// ファイル形式の指定あり

//		DiskParam *param = gDiskTemplates.Find(disk_type);

		rc = SelectPerser(file_format, &param_hint, mod_flags, support);
		if (rc >= 0) {
			image_type = file_format;
		}
	}
	if (!support) {
		result->SetError(DiskResult::ERR_UNSUPPORTED);
		return result->GetValid();
	}
	return rc;
}

/// ディスクイメージをチェック
/// @param [in,out] file_format  ファイルの形式名("d88","plain"など)
/// @param [out] disk_params     ディスクパラメータの候補
/// @param [out] manual_param    候補がないときのパラメータヒント
int DiskParser::Check(wxString &file_format, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return Check(file_format, disk_params, manual_param, DiskD88File::MODIFY_NONE);
}

/// ディスクイメージのチェック
/// @param [in,out] file_format  ファイルの形式名("d88","plain"など)
/// @param [out] disk_params     ディスクパラメータの候補
/// @param [out] manual_param    候補がないときのパラメータヒント
/// @param [in] mod_flags        オープン/追加 DiskD88File::Add()
/// @retval  0 正常
/// @retval -1 エラーあり
int DiskParser::Check(wxString &file_format, DiskParamPtrs &disk_params, DiskParam &manual_param, short mod_flags)
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
			rc = SelectChecker(param_format->GetType(), &param_format->GetHints(), NULL, disk_params, manual_param, mod_flags, support);
			if (rc >= 0) {
				file_format = param_format->GetType();
				break;
			}
		}

	} else {
		// ファイル形式の指定あり

		rc = SelectChecker(file_format, NULL, NULL, disk_params, manual_param, mod_flags, support);

	}
	if (!support) {
		result->SetError(DiskResult::ERR_UNSUPPORTED);
		return result->GetValid();
	}
	return rc;
}

/// ファイルの解析方法を選択
/// @param [in] type             ファイルの形式名("d88","plain"など)
/// @param [in] disk_param       ディスクパラメータ("plain"時のみ)
/// @param [in] mod_flags        オープン/追加 DiskD88File::Add()
/// @param [out] support         サポートしているファイルか
/// @retval  1 警告
/// @retval  0 正常
/// @retval -1 エラー
int DiskParser::SelectPerser(const wxString &type, const DiskParam *disk_param, short mod_flags, bool &support)
{
	int rc = -1;
	if (type == wxT("d88")) {
		// d88形式
		DiskD88Parser ps(file, mod_flags, result);
		rc = ps.Parse(*stream);
		support = true;
	} else if (type == wxT("cpcdsk")) {
		// CPC DSK形式
		DiskDskParser ps(file, mod_flags, result);
		if (ps.Check(*stream) >= 0) {
			rc = ps.Parse(*stream);
		}
		support = true;
	} else if (type == wxT("fdi")) {
		// FDI形式
		DiskFDIParser ps(file, mod_flags, result);
		rc = ps.Parse(*stream, disk_param);
		support = true;
	} else if (type == wxT("cqmimg")) {
		// CopyQM IMG形式
		DiskCQMParser ps(file, mod_flags, result);
		rc = ps.Parse(*stream, disk_param);
		support = true;
	} else if (type == wxT("teletd0")) {
		// Teledisk TD0形式
		DiskTD0Parser ps(file, mod_flags, result);
		rc = ps.Parse(*stream);
		support = true;
	} else if (type == wxT("difcdim")) {
		// DIFC.X DIM形式
		DiskDIMParser ps(file, mod_flags, result);
		rc = ps.Parse(*stream, disk_param);
		support = true;
	} else if (type == wxT("v98fdd")) {
		// Virtual98 FDD形式
		DiskVFDParser ps(file, mod_flags, result);
		rc = ps.Parse(*stream);
		support = true;
	} else if (type == wxT("imd")) {
		// IMageDisk IMD形式
		DiskIMDParser ps(file, mod_flags, result);
		rc = ps.Parse(*stream);
		support = true;
	} else if (type == wxT("plain")) {
		// ベタ
		DiskPlainParser ps(file, mod_flags, result);
		rc = ps.Parse(*stream, disk_param);
		support = true;
	}
	return rc;
}

/// ファイルのチェック方法を選択
/// @param [in] type             ファイルの形式名("d88","plain"など)
/// @param [in] disk_hints       ディスクパラメータヒント("plain"時のみ)
/// @param [in] disk_param       ディスクパラメータ("plain"時のみ)
/// @param [out] disk_params     ディスクパラメータの候補
/// @param [out] manual_param    候補がないときのパラメータヒント
/// @param [in] mod_flags        オープン/追加 DiskD88File::Add()
/// @param [out] support         サポートしているファイルか
/// @retval  1 候補がないので改めてディスク種類を選択してもらう
/// @retval  0 候補あり正常
/// @retval -1 エラー終了
int DiskParser::SelectChecker(const wxString &type, const wxArrayString *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param, short mod_flags, bool &support)
{
	int rc = -1;
	if (type == wxT("d88")) {
		// d88形式
		DiskD88Parser ps(file, mod_flags, result);
		rc = ps.Check(*stream);
		support = true;
	} else if (type == wxT("cpcdsk")) {
		// CPC DSK形式
		DiskDskParser ps(file, mod_flags, result);
		rc = ps.Check(*stream);
		support = true;
	} else if (type == wxT("fdi")) {
		// FDI形式
		DiskFDIParser ps(file, mod_flags, result);
		rc = ps.Check(*this, *stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	} else if (type == wxT("cqmimg")) {
		// CopyQM IMG形式
		DiskCQMParser ps(file, mod_flags, result);
		rc = ps.Check(*this, *stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	} else if (type == wxT("teletd0")) {
		// Teledisk TD0形式
		DiskTD0Parser ps(file, mod_flags, result);
		rc = ps.Check(*this, *stream);
		support = true;
	} else if (type == wxT("difcdim")) {
		// DIFC.X DIM形式
		DiskDIMParser ps(file, mod_flags, result);
		rc = ps.Check(*this, *stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	} else if (type == wxT("v98fdd")) {
		// Virtual98 FDD形式
		DiskVFDParser ps(file, mod_flags, result);
		rc = ps.Check(*stream);
		support = true;
	} else if (type == wxT("imd")) {
		// IMageDisk IMD形式
		DiskIMDParser ps(file, mod_flags, result);
		rc = ps.Check(*this, *stream);
		support = true;
	} else if (type == wxT("plain")) {
		// ベタ
		DiskPlainParser ps(file, mod_flags, result);
		rc = ps.Check(*this, *stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	}
	return rc;
}
