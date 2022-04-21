/// @file fileparam.h
///
/// @brief ファイルパラメータ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _FILE_PARAMETER_H_
#define _FILE_PARAMETER_H_

#include "common.h"
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/dynarray.h>


/// ファイル形式種類
class FileFormat
{
private:
	wxString		name;			///< ファイル種類
	wxString		description;	///< 説明

public:
	FileFormat();
	FileFormat(const wxString &name, const wxString &desc);
	~FileFormat() {}

	void SetName(const wxString &val) { name = val; }
	const wxString &GetName() const { return name; }
	const wxString &GetDescription() const { return description; }
};

WX_DECLARE_OBJARRAY(FileFormat, FileFormats);

/// ファイル形式パラメータ
class FileParamFormat
{
private:
	wxString		type;	///< ファイル種類
	wxArrayString	hints;	///< ディスク解析で用いるヒント

public:
	FileParamFormat();
	FileParamFormat(const wxString &type);
	~FileParamFormat() {}
	void AddHint(const wxString &val);

	void SetType(const wxString &val) { type = val; }
	const wxString &GetType() const { return type; }
	const wxArrayString &GetHints() const { return hints; }
};

WX_DECLARE_OBJARRAY(FileParamFormat, FileParamFormats);

/// ファイルパラメータ
class FileParam
{
protected:
	wxString			extension;		///< 拡張子
	FileParamFormats	formats;		///< フォーマットリスト
	wxString			description;	///< 説明

public:
	FileParam();
	FileParam(const FileParam &src);
	FileParam(const wxString &n_ext, const FileParamFormats &n_formats, const wxString &n_desc);
	virtual ~FileParam() {}

	FileParam &operator=(const FileParam &src);
	void SetFileParam(const FileParam &src);
	void SetFileParam(const wxString &n_ext, const FileParamFormats &n_formats, const wxString &n_desc);

	void ClearFileParam();

	const wxString &GetExt() const { return extension; }
	const FileParamFormats &GetFormats() const { return formats; }
	const wxString &GetDescription() const { return description; }
};

WX_DECLARE_OBJARRAY(FileParam, FileParams);

/// ファイル種類
class FileTypes
{
private:
	FileFormats formats;
	FileParams types;

	wxString wcard_for_load;	///< ファイルダイアログ用 ワイルドカード
	wxString wcard_for_save;	///< ファイルダイアログ用 ワイルドカード

	void MakeWildcard();

public:
	FileTypes();
	~FileTypes() {}

	bool Load(const wxString &data_path, const wxString &locale_name);

	FileParam *FindExt(const wxString &n_ext);
	FileFormat *FindFormat(const wxString &n_name);

	FileParam *ItemPtr(size_t index) const { return &types[index]; }
	FileParam &Item(size_t index) const { return types[index]; }
	size_t Count() const { return types.Count(); }

	const wxString &GetWildcardForLoad() const { return wcard_for_load; }
	const wxString &GetWildcardForSave() const { return wcard_for_save; }
	FileFormat *GetFilterForSave(int index);

	const FileFormats &GetFormats() const { return formats; }
};

extern FileTypes gFileTypes;

#endif /* _FILE_PARAMETER_H_ */
