/// @file fileparam.cpp
///
/// @brief ファイルパラメータ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "fileparam.h"
#include <wx/intl.h>
#include <wx/xml/xml.h>


FileTypes gFileTypes;

static const char *cFileFormatTypeNamesForSave[] = {
	"", "d88", "plain", NULL
};

//////////////////////////////////////////////////////////////////////
//
// ファイル形式種類
//
FileFormat::FileFormat()
{
}
FileFormat::FileFormat(const wxString &name, const wxString &desc)
{
	m_name = name;
	m_description = desc;
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileFormats);

//////////////////////////////////////////////////////////////////////
//
// ファイル形式パラメータ
//
FileParamFormat::FileParamFormat()
{
}
/// @param[in] type ファイル種類("d88","plain",...)
FileParamFormat::FileParamFormat(const wxString &type)
{
	m_type = type;
}
/// ヒントを追加
void FileParamFormat::AddHint(const wxString &val)
{
	m_hints.Add(val);
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileParamFormats);

//////////////////////////////////////////////////////////////////////
//
// ファイルパラメータ
//
FileParam::FileParam()
{
	this->ClearFileParam();
}
FileParam::FileParam(const FileParam &src)
{
	this->SetFileParam(src);
}
/// @param[in] n_ext     拡張子
/// @param[in] n_formats フォーマットリスト
/// @param[in] n_desc    説明
FileParam::FileParam(const wxString &n_ext, const FileParamFormats &n_formats, const wxString &n_desc)
{
	this->SetFileParam(n_ext, n_formats, n_desc);
}
/// 代入
FileParam &FileParam::operator=(const FileParam &src)
{
	this->SetFileParam(src);
	return *this;
}
/// 設定
/// @param[in] src       元
void FileParam::SetFileParam(const FileParam &src)
{
	m_extension = src.m_extension;
	m_formats = src.m_formats;
	m_description = src.m_description;
}
/// 設定
/// @param[in] n_ext     拡張子
/// @param[in] n_formats フォーマットリスト
/// @param[in] n_desc    説明
void FileParam::SetFileParam(const wxString &n_ext, const FileParamFormats &n_formats, const wxString &n_desc)
{
	m_extension = n_ext;
	m_formats = n_formats;
	m_description = n_desc;

	// 拡張子は小文字にしておく
	m_extension = m_extension.Lower();
}
/// 初期化
void FileParam::ClearFileParam()
{
	m_extension.Empty();
	m_formats.Empty();
	m_description.Empty();
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileParams);

//////////////////////////////////////////////////////////////////////
//
// ファイル種類
//
FileTypes::FileTypes()
{
}

/// XMLファイルをロード
/// @param[in] data_path   : ファイルパス
/// @param[in] locale_name : ローケル(jaなど)
/// @return false:エラー
bool FileTypes::Load(const wxString &data_path, const wxString &locale_name)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("file_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "FileTypes") return false;

	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item) {
		if (item->GetName() == "FileFormatType") {
			wxString name = item->GetAttribute("name");
			wxXmlNode *itemnode = item->GetChildren();
			wxString desc, desc_locale;
			while (itemnode) {
				if (itemnode->GetName() == "Description") {
					if (itemnode->HasAttribute("lang")) {
						wxString lang = itemnode->GetAttribute("lang");
						if (locale_name.Find(lang) != wxNOT_FOUND) {
							desc_locale = itemnode->GetNodeContent();
						}
					} else {
						desc = itemnode->GetNodeContent();
					}
				}
				itemnode = itemnode->GetNext();
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			FileFormat p(name, desc);
			formats.Add(p);
		} else if (item->GetName() == "FileType") {
			wxString ext  = item->GetAttribute("ext");
			wxXmlNode *itemnode = item->GetChildren();
			FileParamFormats fmts;
			wxString desc, desc_locale, str;
			while (itemnode) {
				if (itemnode->GetName() == "Format") {
					wxString type = itemnode->GetAttribute("type");
					FileParamFormat fmt(type);
					wxXmlNode *citemnode = itemnode->GetChildren();
					while(citemnode) {
						if (citemnode->GetName() == "DiskTypeHint") {
							str = citemnode->GetNodeContent();
							str = str.Trim(false).Trim(true);
							if (!str.IsEmpty()) {
								fmt.AddHint(str);
							}
						}
						citemnode = citemnode->GetNext();
					}
					fmts.Add(fmt);
				} else if (itemnode->GetName() == "Description") {
					if (itemnode->HasAttribute("lang")) {
						wxString lang = itemnode->GetAttribute("lang");
						if (locale_name.Find(lang) != wxNOT_FOUND) {
							desc_locale = itemnode->GetNodeContent();
						}
					} else {
						desc = itemnode->GetNodeContent();
					}
				}
				itemnode = itemnode->GetNext();
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			FileParam p(ext, fmts, desc);
			types.Add(p);
		}
		item = item->GetNext();
	}

	MakeWildcard();

	return true;
}

/// ファイルダイアログ用の拡張子選択リストを作成する
void FileTypes::MakeWildcard()
{
	wxString desc[3];
	for(size_t i=0; i<formats.Count(); i++) {
		const FileFormat *format = &formats.Item(i);
		for(int n=0; cFileFormatTypeNamesForSave[n] != NULL; n++) {
			if (format->GetName() == cFileFormatTypeNamesForSave[n]) {
				desc[n] = format->GetDescription();
				break;
			}
		}
	}

	wxArrayString exts[3];
	for(size_t i=0; i<types.Count(); i++) {
		const FileParam *param = &types.Item(i);
		const FileParamFormats *formats = &param->GetFormats();
		for(size_t j=0; j<formats->Count(); j++) {
			const FileParamFormat *format = &formats->Item(j);
			for(int n=0; cFileFormatTypeNamesForSave[n] != NULL; n++) {
				if (n == 0 || format->GetType() == cFileFormatTypeNamesForSave[n]) {
					wxString ext = wxT("*.");
					ext += param->GetExt().Lower();
					if (exts[n].Index(ext) == wxNOT_FOUND) {
						exts[n].Add(ext);
#if !defined(__WXMSW__)
						ext = wxT("*.");
						ext += param->GetExt().Upper();
						exts[n].Add(ext);
#endif
					}
				}
			}
		}
	}
	wcard_for_load = _("Supported files");
//	wcard_for_load += wxT(" (");
//	wcard_for_load += wxJoin(exts[0], ';');
//	wcard_for_load += wxT(")|");
	wcard_for_load += wxT("|");
	wcard_for_load += wxJoin(exts[0], ';');
#if defined(__WXMSW__)
	for(size_t i=0; i<types.Count(); i++) {
		const FileParam *param = &types.Item(i);
		wcard_for_load += wxT("|");
		wcard_for_load += param->GetDescription();
//		wcard_for_load += wxT(" (");
		wxString ext = wxT("*.");
		ext += param->GetExt().Lower();
//		wcard_for_load += ext;
//		wcard_for_load += wxT(")|");
		wcard_for_load += wxT("|");
		wcard_for_load += ext;
	}
#endif
	wcard_for_load += wxT("|");
	wcard_for_load += _("All files");
//	wcard_for_load += wxT(" (*.*)|*.*");
	wcard_for_load += wxT("|*.*");

	exts_for_save.Clear();
	for(int n=1; cFileFormatTypeNamesForSave[n] != NULL; n++) {
		exts_for_save.Add(n);
		if (n > 1) wcard_for_save += wxT("|");
		wcard_for_save += desc[n];
		wcard_for_save += wxT(" (");
		wcard_for_save += wxJoin(exts[n], ';');
		wcard_for_save += wxT(")|");
		wcard_for_save += wxJoin(exts[n], ';');
	}
	int dir = 1;
#if !defined(__WXMSW__)
	dir = 2;
#endif
	for(int n=1; cFileFormatTypeNamesForSave[n] != NULL; n++) {
		for(size_t i=0; i<exts[n].Count(); i+=dir) {
			exts_for_save.Add(n);
			wcard_for_save += wxT("|");
			wcard_for_save += desc[n];
			wcard_for_save += wxT(" (");
			wcard_for_save += exts[n].Item(i);
#if !defined(__WXMSW__)
			wcard_for_save += wxT(";");
			wcard_for_save += exts[n].Item(i+1);
#endif
			wcard_for_save += wxT(")|");
			wcard_for_save += exts[n].Item(i);
#if !defined(__WXMSW__)
			wcard_for_save += wxT(";");
			wcard_for_save += exts[n].Item(i+1);
#endif
		}
	}
}

/// 拡張子をさがす
/// @param[in] n_ext 拡張子(".d88"など)
/// @return FileFormat
FileParam *FileTypes::FindExt(const wxString &n_ext)
{
	FileParam *match = NULL;
	wxString ext = n_ext.Lower();
	for(size_t i=0; i<types.Count(); i++) {
		FileParam *item = &types[i];
		if (ext == item->GetExt()) {
			match = item;
			break;
		}
	}
	return match;
}

/// ディスクイメージフォーマット形式をさがす
/// @param[in] n_name 名前("d88"など)
/// @return FileFormat
FileFormat *FileTypes::FindFormat(const wxString &n_name)
{
	FileFormat *match = NULL;
	for(size_t i=0; i<formats.Count(); i++) {
		FileFormat *item = &formats[i];
		if (n_name == item->GetName()) {
			match = item;
			break;
		}
	}
	return match;
}

/// ファイル保存時の保存形式のフォーマットを返す
/// @param[in] index ダイアログ内にある拡張子リストの位置
/// @return FileFormat
FileFormat *FileTypes::GetFilterForSave(int index)
{
	FileFormat *match = NULL;
	if (index < 0 || index >= (int)exts_for_save.Count()) {
		return match;
	}
	int n = exts_for_save.Item(index);
	if (n <= 0 || n + 1 >= (int)(sizeof(cFileFormatTypeNamesForSave) / sizeof(const char *))) {
		return match;
	}
	match = FindFormat(cFileFormatTypeNamesForSave[n]);

	return match;
}
