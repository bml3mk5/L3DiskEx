/// @file fileparam.cpp
///
/// @brief ファイルパラメータ
///
#include "fileparam.h"
#include <wx/intl.h>
#include <wx/xml/xml.h>

FileTypes gFileTypes;

//
//
//
FileFormat::FileFormat()
{
}
FileFormat::FileFormat(const wxString &name, const wxString &desc)
{
	this->name = name;
	this->description = desc;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileFormats);

//
//
//
FileParamFormat::FileParamFormat()
{
}
FileParamFormat::FileParamFormat(const wxString &type)
{
	this->type = type;
}
void FileParamFormat::AddHint(const wxString &val)
{
	hints.Add(val);
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileParamFormats);

//
//
//
FileParam::FileParam()
{
	this->ClearFileParam();
}
FileParam::FileParam(const FileParam &src)
{
	this->SetFileParam(src);
}
FileParam::FileParam(const wxString &n_ext, const FileParamFormats &n_formats, const wxString &n_desc)
{
	this->SetFileParam(n_ext, n_formats, n_desc);
}
FileParam &FileParam::operator=(const FileParam &src)
{
	this->SetFileParam(src);
	return *this;
}
void FileParam::SetFileParam(const FileParam &src)
{
	extension = src.extension;
	formats = src.formats;
	description = src.description;
}
void FileParam::SetFileParam(const wxString &n_ext, const FileParamFormats &n_formats, const wxString &n_desc)
{
	extension = n_ext;
	formats = n_formats;
	description = n_desc;

	extension = extension.Lower();
}
void FileParam::ClearFileParam()
{
	extension.Empty();
	formats.Empty();
	description.Empty();
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileParams);

//
//
//
FileTypes::FileTypes()
{
}
bool FileTypes::Load(const wxString &data_path, const wxString &locale_name)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("file_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "FileTypes") return false;

#if 0
	wxXmlNode *prolog = doc.GetDocumentNode()->GetChildren();
	while (prolog) {
	    if (prolog->GetType() == wxXML_PI_NODE && prolog->GetName() == "FileType") {
	        wxString pi = prolog->GetContent();
		}
	}
#endif

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

void FileTypes::MakeWildcard()
{
	wxString d88s, exts;
	for(size_t i=0; i<types.Count(); i++) {
		const FileParam *param = &types.Item(i);
		const FileParamFormats *formats = &param->GetFormats();
		for(size_t j=0; j<formats->Count(); j++) {
			const FileParamFormat *format = &formats->Item(j);
			if (format->GetType() == wxT("d88")) {
				if (!d88s.IsEmpty()) d88s += wxT(";");
				d88s += wxT("*.");
				d88s += param->GetExt();
			}
		}
		if (!exts.IsEmpty()) exts += wxT(";");
		exts += wxT("*.");
		exts += param->GetExt();
	}
	wcard_for_load = _("Supported files");
	wcard_for_load += wxT(" (");
	wcard_for_load += exts;
	wcard_for_load += wxT(")|");
	wcard_for_load += exts;
	wcard_for_load += wxT("|");
	wcard_for_load += _("All files");
	wcard_for_load += wxT(" (*.*)|*.*");

	wcard_for_save = _("Supported files");
	wcard_for_save += wxT(" (");
	wcard_for_save += d88s;
	wcard_for_save += wxT(")|");
	wcard_for_save += d88s;
	wcard_for_save += wxT("|");
	for(size_t i=0; i<types.Count(); i++) {
		const FileParam *param = &types.Item(i);
		const FileParamFormats *formats = &param->GetFormats();
		for(size_t j=0; j<formats->Count(); j++) {
			const FileParamFormat *format = &formats->Item(j);
			if (format->GetType() == wxT("d88") || format->GetType() == wxT("plain")) {
				wcard_for_save += param->GetDescription();
				wcard_for_save += wxT(" (*.");
				wcard_for_save += param->GetExt();
				wcard_for_save += wxT(")|*.");
				wcard_for_save += param->GetExt();
				wcard_for_save += wxT("|");
			}
		}
	}
	wcard_for_save += _("All files");
	wcard_for_save += wxT(" (*.*)|*.*");

}

#if 0
int FileTypes::IndexOf(const wxString &n_ext)
{
	int match = -1;
	wxString ext = n_ext.Lower();
	for(size_t i=0; i<types.Count(); i++) {
		FileParam *item = &types[i];
		if (ext == item->GetExt()) {
			match = (int)i;
			break;
		}
	}
	return match;
}
#endif

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
