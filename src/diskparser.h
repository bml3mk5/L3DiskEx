/// @file diskparser.h
///
/// @brief ディスクイメージパーサ
///
#ifndef _DISK_PARSER_H_
#define _DISK_PARSER_H_

#include "common.h"
#include <wx/string.h>
#include <wx/filename.h>

class wxInputStream;
class wxArrayString;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskResult;
class DiskParam;
class FileParamFormat;

/// ディスクパーサー
class DiskParser
{
private:
	wxFileName		 filepath;
	wxInputStream	*stream;
	DiskD88File		*file;
	DiskResult	*result;

	/// ファイルの解析方法を選択
	int SelectPerser(const wxString &type, const wxArrayString *disk_hints, const DiskParam *disk_param, short mod_flags, bool &support);
	int Parse(const wxString &file_format, const wxString &disk_type, short mod_flags);

public:
	DiskParser(const wxString &filepath, wxInputStream *stream, DiskD88File *file, DiskResult &result);
	~DiskParser();

	/// ディスクイメージを新たに解析する
	int Parse(const wxString &file_format, const wxString &disk_type);
	/// 指定ディスクを解析してこれを既存のディスクイメージに追加する
	int ParseAdd(const wxString &file_format, const wxString &disk_type);
};

#endif /* _DISK_PARSER_H_ */
