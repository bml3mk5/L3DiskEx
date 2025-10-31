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
#include "diskstrparser.h"
#include "diskg64parser.h"
#include "disk2mgparser.h"
#include "diskadcparser.h"
#include "diskdmkparser.h"
#include "diskjv3parser.h"
#include "diskhfeparser.h"
#include "diskimage.h"
#include "fileparam.h"
#include "diskresult.h"
#include "../logging.h"


/// コンストラクタ
/// @param [in]     filepath    解析するファイルのパス 
/// @param [in]     stream      上記ファイルのストリーム   
/// @param [in,out] file        既存のディスクイメージ
/// @param [out]    result      結果
DiskParser::DiskParser(const wxString &filepath, wxInputStream *stream, DiskImageFile *file, DiskResult &result)
{
	m_filepath = wxFileName(filepath);
	p_stream = stream;
	p_file = file;
	p_result = &result;
}

DiskParser::~DiskParser()
{
}

/// ディスクイメージを新たに解析する
/// @param [in] file_format      ファイルの形式名("d88","plain"など)
/// @param [in] param_hint       ディスクパラメータヒント("plain"時のみ)
int DiskParser::Parse(const wxString &file_format, const DiskParam &param_hint)
{
	return Parse(file_format, param_hint, DiskImageFile::MODIFY_NONE);
}

/// 指定ディスクを解析してこれを既存のディスクイメージに追加する
/// @param [in] file_format      ファイルの形式名("d88","plain"など)
/// @param [in] param_hint       ディスクパラメータヒント("plain"時のみ)
int DiskParser::ParseAdd(const wxString &file_format, const DiskParam &param_hint)
{
	return Parse(file_format, param_hint, DiskImageFile::MODIFY_ADD);
}

/// ディスクイメージの解析
/// @param [in] file_format ファイルの形式名("d88","plain"など)
/// @param [in] param_hint  ディスクパラメータヒント("plain"時のみ)
/// @param [in] mod_flags   オープン/追加 DiskImageFile::Add()
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskParser::Parse(const wxString &file_format, const DiskParam &param_hint, short mod_flags)
{
	bool support = false;
	int rc = -1;

	m_image_type.Empty();
	if (!file_format.IsEmpty()) {
		// ファイル形式の指定あり
		rc = SelectPerser(file_format, &param_hint, mod_flags, support);
		if (rc >= 0) {
			m_image_type = file_format;
		}
	}
	if (!support) {
		p_result->SetError(DiskResult::ERR_UNSUPPORTED);
		return p_result->GetValid();
	}
	return rc;
}

/// ディスクイメージをチェック
/// @param [in,out] file_format  ファイルの形式名("d88","plain"など)
/// @param [out] disk_params     ディスクパラメータの候補
/// @param [out] manual_param    候補がないときのパラメータヒント
int DiskParser::Check(wxString &file_format, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return Check(file_format, disk_params, manual_param, DiskImageFile::MODIFY_NONE);
}

/// ディスクイメージのチェック
/// @param [in,out] file_format  ファイルの形式名("d88","plain"など)
/// @param [out] disk_params     ディスクパラメータの候補
/// @param [out] manual_param    候補がないときのパラメータヒント
/// @param [in] mod_flags        オープン/追加 DiskImageFile::Add()
/// @retval  0 正常
/// @retval -1 エラーあり
int DiskParser::Check(wxString &file_format, DiskParamPtrs &disk_params, DiskParam &manual_param, short mod_flags)
{
	bool support = false;
	int rc = -1;

	if (file_format.IsEmpty()) {
		// ファイル形式の指定がない場合

		// 拡張子で判定
		wxString ext = m_filepath.GetExt();

		// サポートしているファイルか
		const FileParam *fitem = gFileTypes.FindExt(ext);
		if (!fitem) {
			p_result->SetError(DiskResult::ERR_UNSUPPORTED);
			return p_result->GetValid();
		}

		// 指定形式で解析する
		const FileParamFormats *formats = &fitem->GetFormats();
		for(size_t i=0; i<formats->Count(); i++) {
			const FileParamFormat *param_format = &formats->Item(i);
			myLog.SetInfo(wxT("Parsing image: ") + param_format->GetType()); 
			rc = SelectChecker(param_format->GetType(), &param_format->GetHints(), NULL, disk_params, manual_param, mod_flags, support);
			if (rc >= 0) {
				file_format = param_format->GetType();
				break;
			}
		}

	} else {
		// ファイル形式の指定あり
		myLog.SetInfo(wxT("Parsing image: ") + file_format); 

		rc = SelectChecker(file_format, NULL, NULL, disk_params, manual_param, mod_flags, support);

	}
	if (!support) {
		p_result->SetError(DiskResult::ERR_UNSUPPORTED);
		return p_result->GetValid();
	}

	myLog.SetInfo(wxT("Decided image: ") + file_format); 
	return rc;
}

/// ファイルの解析方法を選択
/// @param [in] type             ファイルの形式名("d88","plain"など)
/// @param [in] disk_param       ディスクパラメータ("plain"時のみ)
/// @param [in] mod_flags        オープン/追加 DiskImageFile::Add()
/// @param [out] support         サポートしているファイルか
/// @retval  1 警告
/// @retval  0 正常
/// @retval -1 エラー
int DiskParser::SelectPerser(const wxString &type, const DiskParam *disk_param, short mod_flags, bool &support)
{
	int rc = -1;
	if (type == wxT("d88")) {
		// d88形式
		DiskD88Parser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream);
		support = true;
	} else if (type == wxT("cpcdsk")) {
		// CPC DSK形式
		DiskDskParser ps(p_file, mod_flags, p_result);
		if (ps.Check(*p_stream) >= 0) {
			rc = ps.Parse(*p_stream);
		}
		support = true;
	} else if (type == wxT("fdi")) {
		// FDI形式
		DiskFDIParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream, disk_param);
		support = true;
	} else if (type == wxT("cqmimg")) {
		// CopyQM IMG形式
		DiskCQMParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream, disk_param);
		support = true;
	} else if (type == wxT("teletd0")) {
		// Teledisk TD0形式
		DiskTD0Parser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream);
		support = true;
	} else if (type == wxT("difcdim")) {
		// DIFC.X DIM形式
		DiskDIMParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream, disk_param);
		support = true;
	} else if (type == wxT("v98fdd")) {
		// Virtual98 FDD形式
		DiskVFDParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream);
		support = true;
	} else if (type == wxT("imd")) {
		// IMageDisk IMD形式
		DiskIMDParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream);
		support = true;
	} else if (type == wxT("dskstr")) {
		// DSKSTR 形式
		DiskSTRParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream);
		support = true;
	} else if (type == wxT("g64")) {
		// Commodore VICE emu G64 形式
		DiskG64Parser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream);
		support = true;
	} else if (type == wxT("2mg")) {
		// Apple 2MG 形式
		Disk2MGParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream, disk_param);
		support = true;
	} else if (type == wxT("adc")) {
		// Apple Disk Copy 4 形式
		DiskADCParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream, disk_param);
		support = true;
	} else if (type == wxT("dmk")) {
		// TRS-80 DMK 形式
		DiskDmkParser ps(p_file, mod_flags, p_result);
		if (ps.Check(*p_stream) >= 0) {
			rc = ps.Parse(*p_stream);
		}
		support = true;
	} else if (type == wxT("jv3")) {
		// TRS-80 JV3 形式
		DiskJV3Parser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream);
		support = true;
	} else if (type == wxT("hfe")) {
		// HxC HFE 形式
		DiskHfeParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream);
		support = true;
	} else if (type == wxT("plain")) {
		// ベタ
		DiskPlainParser ps(p_file, mod_flags, p_result);
		rc = ps.Parse(*p_stream, disk_param);
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
/// @param [in] mod_flags        オープン/追加 DiskImageFile::Add()
/// @param [out] support         サポートしているファイルか
/// @retval  1 候補がないので改めてディスク種類を選択してもらう
/// @retval  0 候補あり正常
/// @retval -1 エラー終了
int DiskParser::SelectChecker(const wxString &type, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param, short mod_flags, bool &support)
{
	int rc = -1;
	if (type == wxT("d88")) {
		// d88形式
		DiskD88Parser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("cpcdsk")) {
		// CPC DSK形式
		DiskDskParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("fdi")) {
		// FDI形式
		DiskFDIParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	} else if (type == wxT("cqmimg")) {
		// CopyQM IMG形式
		DiskCQMParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	} else if (type == wxT("teletd0")) {
		// Teledisk TD0形式
		DiskTD0Parser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("difcdim")) {
		// DIFC.X DIM形式
		DiskDIMParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	} else if (type == wxT("v98fdd")) {
		// Virtual98 FDD形式
		DiskVFDParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("imd")) {
		// IMageDisk IMD形式
		DiskIMDParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("dskstr")) {
		// DSKSTR 形式
		DiskSTRParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("g64")) {
		// Commodore VICE emu G64 形式
		DiskG64Parser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("2mg")) {
		// Apple 2mg 形式
		Disk2MGParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	} else if (type == wxT("adc")) {
		// Apple Disk Copy 4 形式
		DiskADCParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	} else if (type == wxT("dmk")) {
		// TRS-80 DMK 形式
		DiskDmkParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("jv3")) {
		// TRS-80 JV3 形式
		DiskJV3Parser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("hfe")) {
		// HxC HFE 形式
		DiskHfeParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream);
		support = true;
	} else if (type == wxT("plain")) {
		// ベタ
		DiskPlainParser ps(p_file, mod_flags, p_result);
		rc = ps.Check(*p_stream, disk_hints, disk_param, disk_params, manual_param);
		support = true;
	}
	return rc;
}

// ----------------------------------------------------------------------
//
//
//
DiskImageParser::DiskImageParser(DiskImageFile *file, short mod_flags, DiskResult *result)
{
	p_file = file;
	m_mod_flags = mod_flags;
	p_result = result;
}

DiskImageParser::~DiskImageParser()
{
}

/// ファイルイメージを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param ディスクパラメータ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskImageParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	return p_result->GetValid();
}

/// チェック
/// @param [in] istream       解析対象データ
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskImageParser::Check(wxInputStream &istream)
{
	return p_result->GetValid();
}

/// チェック
/// @param [in] istream       解析対象データ
/// @param [in] disk_hints    ディスクパラメータヒント("2D"など)
/// @param [in] disk_param    ディスクパラメータ disk_hints指定時はNullable
/// @param [out] disk_params  ディスクパラメータの候補
/// @param [out] manual_param 候補がないときのパラメータヒント
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskImageParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	return p_result->GetValid();
}
