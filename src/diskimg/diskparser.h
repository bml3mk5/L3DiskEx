/// @file diskparser.h
///
/// @brief ディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISK_PARSER_H
#define DISK_PARSER_H

#include "../common.h"
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/filename.h>


class wxInputStream;
class wxArrayString;
class DiskImageTrack;
class DiskImageDisk;
class DiskImageFile;
class DiskResult;
class DiskParam;
class DiskParamPtrs;
class FileParamFormat;
class DiskTypeHints;

/// ディスクパーサー
class DiskParser
{
private:
	wxFileName		 m_filepath;
	wxInputStream	*p_stream;
	DiskImageFile	*p_file;
	DiskResult		*p_result;
	wxString		 m_image_type;

	/// ファイルの解析方法を選択
	int SelectPerser(const wxString &type, const DiskParam *disk_param, short mod_flags, bool &support);
	int Parse(const wxString &file_format, const DiskParam &param_hint, short mod_flags);
	/// ファイルの解析方法を選択
	int SelectChecker(const wxString &type, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param, short mod_flags, bool &support);
	int Check(wxString &file_format, DiskParamPtrs &disk_params, DiskParam &manual_param, short mod_flags);

public:
	DiskParser(const wxString &filepath, wxInputStream *stream, DiskImageFile *file, DiskResult &result);
	~DiskParser();

	/// ディスクイメージを新たに解析する
	int Parse(const wxString &file_format, const DiskParam &param_hint);
	/// 指定ディスクを解析してこれを既存のディスクイメージに追加する
	int ParseAdd(const wxString &file_format, const DiskParam &param_hint);
	/// ディスクイメージをチェック
	int Check(wxString &file_format, DiskParamPtrs &disk_params, DiskParam &manual_param);
	/// ディスクイメージのタイプを返す
	const wxString &GetImageType() const { return m_image_type; }
};

/// ディスクパーサー
class DiskImageParser
{
protected:
	DiskImageFile	*p_file;
	short			 m_mod_flags;
	DiskResult		*p_result;

	DiskImageParser() {}
	DiskImageParser(const DiskImageParser &src) {}

public:
	DiskImageParser(DiskImageFile *file, short mod_flags, DiskResult *result);
	virtual ~DiskImageParser();

	/// チェック
	virtual int Check(wxInputStream &istream);
	/// チェック
	virtual int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);
	/// 解析
	virtual int Parse(wxInputStream &istream, const DiskParam *disk_param);
};

#endif /* DISK_PARSER_H */
