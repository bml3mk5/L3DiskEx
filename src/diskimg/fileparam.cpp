/// @file fileparam.cpp
///
/// @brief ファイルパラメータ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "fileparam.h"
#include "diskwriter.h"
#include <wx/intl.h>
#include <wx/xml/xml.h>

FileTypes gFileTypes;

//////////////////////////////////////////////////////////////////////
//
// ファイル形式種類
//
FileFormat::FileFormat()
{
	m_idx = 0;
}
FileFormat::FileFormat(int idx, const wxString &name, const wxString &desc)
{
	m_idx = idx;
	m_name = name;
	m_description = desc;
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileFormats);

//////////////////////////////////////////////////////////////////////
//
// ディスク解析で用いるヒント
//
DiskTypeHint::DiskTypeHint()
{
	m_kind = 0;
}
DiskTypeHint::DiskTypeHint(const wxString &hint)
{
	m_hint = hint;
	m_kind = 0;
}
DiskTypeHint::DiskTypeHint(const wxString &hint, int kind)
{
	m_hint = hint;
	m_kind = kind;
}
/// セット
void DiskTypeHint::Set(const wxString &hint, int kind)
{
	m_hint = hint;
	m_kind = kind;
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskTypeHints);

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
void FileParamFormat::AddHint(const wxString &val, int kind)
{
	m_hints.Add(DiskTypeHint(val, kind));
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
// ワイルドカード保存用
//
WildCard::WildCard()
{
}

WildCard::WildCard(const wxString n_format, const wxString n_ext, const wxString n_card)
{
	m_format = n_format;
	m_ext    = n_ext;
	m_card   = n_card;
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(WildCards);

//////////////////////////////////////////////////////////////////////
//
// ファイル種類
//
FileTypes::FileTypes() : TemplatesBase()
{
}

/// XMLファイルをロード
/// @param[in] data_path   : ファイルパス
/// @param[in] locale_name : ローケル(jaなど)
/// @param[out] errmsgs     エラーメッセージ
/// @return false:エラー
bool FileTypes::Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("file_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "FileTypes") return false;

	int idx = 0;
	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item) {
		if (item->GetName() == "FileFormatType") {
			wxString name = item->GetAttribute("name");
			wxXmlNode *itemnode = item->GetChildren();
			wxString desc, desc_locale;
			while (itemnode) {
				if (itemnode->GetName() == "Description") {
					LoadDescription(itemnode, locale_name, desc, desc_locale);
				}
				itemnode = itemnode->GetNext();
			}
			if (!desc_locale.IsEmpty()) {
				desc = desc_locale;
			}
			FileFormat p(idx, name, desc);
			formats.Add(p);
			idx++;
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
								wxString skind = citemnode->GetAttribute("kind");
								long kind = 0;
								if (!skind.IsEmpty()) {
									skind.ToLong(&kind);
								}
								fmt.AddHint(str, (int)kind);
							}
						}
						citemnode = citemnode->GetNext();
					}
					fmts.Add(fmt);
				} else if (itemnode->GetName() == "Description") {
					LoadDescription(itemnode, locale_name, desc, desc_locale);
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
		for(int n=0; DiskWriter::cFormatTypeNamesForSave[n] != NULL; n++) {
			if (format->GetName() == DiskWriter::cFormatTypeNamesForSave[n]) {
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
			for(int n=0; DiskWriter::cFormatTypeNamesForSave[n] != NULL; n++) {
				if (n == 0 || format->GetType() == DiskWriter::cFormatTypeNamesForSave[n]) {
					wxString ext = param->GetExt().Lower();
					if (exts[n].Index(ext) == wxNOT_FOUND) {
						exts[n].Add(ext);
#if !defined(__WXMSW__)
						ext = param->GetExt().Upper();
						exts[n].Add(ext);
#endif
					}
				}
			}
		}
	}
	wcard_for_load = _("Supported files");
	wcard_for_load += wxT("|");
	for(size_t i=0; i<exts[0].Count(); i++) {
		if (i != 0) wcard_for_load += wxT(";");
		wcard_for_load += wxT("*.");
		wcard_for_load += exts[0].Item(i);
	}
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

	int dir = 1;
#if !defined(__WXMSW__)
	dir = 2;
#endif
	for(int n=1; DiskWriter::cFormatTypeNamesForSave[n] != NULL; n++) {
		for(size_t i=0; i<exts[n].Count(); i+=dir) {
			wxString str;
			str += desc[n];
			str += wxT(" (*.");
			str += exts[n].Item(i);
#if !defined(__WXMSW__)
			str += wxT(";*.");
			str += exts[n].Item(i+1);
#endif
			str += wxT(")|*.");
			str += exts[n].Item(i);
#if !defined(__WXMSW__)
			str += wxT(";*.");
			str += exts[n].Item(i+1);
#endif
			wcard_for_save.Add(WildCard(DiskWriter::cFormatTypeNamesForSave[n], exts[n].Item(i), str));
		}
	}
}

/// 拡張子をさがす
/// @param[in] n_ext 拡張子("d88"など)
/// @return FileFormat
const FileParam *FileTypes::FindExt(const wxString &n_ext) const
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

/// 拡張子をさがす
/// @param[in] n_ext 拡張子("d88"など)
/// @return リストの位置 ないときwxNOT_FOUND
size_t FileTypes::IndexOfExt(const wxString &n_ext) const
{
	size_t match = wxNOT_FOUND;
	wxString ext = n_ext.Lower();
	for(size_t i=0; i<types.Count(); i++) {
		FileParam *item = &types[i];
		if (ext == item->GetExt()) {
			match = i;
			break;
		}
	}
	return match;
}

/// ディスクイメージフォーマット形式をさがす
/// @param[in] n_name 名前("d88"など)
/// @return FileFormat
const FileFormat *FileTypes::FindFormat(const wxString &n_name) const
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

/// ディスクイメージフォーマット形式をさがす
/// @param[in] idx
/// @return FileFormat
const FileFormat *FileTypes::FindFormat(int idx) const
{
	if (idx < 0 || idx >= (int)formats.Count()) {
		return NULL;
	}
	return &formats[idx];
}

/// ファイルセーブ時の拡張子リスト
/// @param[in] n_format フォーマット種類 ("plain"など)
/// @param[in] n_ext 拡張子 ("d88"など)
/// @note 拡張子を指定した場合そのアイテムがあれば最初に持ってくる
wxString FileTypes::GetWildcardForSave(const wxString &n_format, const wxString &n_ext)
{
	bool first = true;
	size_t match = wxNOT_FOUND;
	wxString str;

	idx_for_save.Clear();
	if (!n_format.IsEmpty() && !n_ext.IsEmpty()) {
		wxString format = n_format.Lower();
		wxString ext = n_ext.Lower();
		// フォーマットと拡張子を指定した場合そのアイテムがあれば最初に持ってくる
		for(size_t i=0; i<wcard_for_save.Count(); i++) {
			WildCard *item = &wcard_for_save.Item(i);
			if (item->GetFormat().Lower() == format && item->GetExt().Lower() == ext) {
				match = i;
				str += item->GetCard();
				idx_for_save.Add((int)i);
				first = false;
				break;
			}
		}
	}
	for(size_t i=0; i<wcard_for_save.Count(); i++) {
		if (match == i) continue;
		if (!first) str += wxT("|");
		str += wcard_for_save.Item(i).GetCard();
		idx_for_save.Add((int)i);
		first = false;
	}
	return str;
}

/// ファイル保存時の保存形式のフォーマットを返す
/// @param[in] index ダイアログ内にある拡張子リストの位置
/// @return FileFormat
/// @attention 事前にGetWildcardForSave()を実行する必要がある
const FileFormat *FileTypes::GetFilterForSave(int index) const
{
	const FileFormat *match = NULL;
	if (index < 0 || index >= (int)idx_for_save.Count()) {
		return match;
	}
	int n = idx_for_save.Item(index);
	match = FindFormat(wcard_for_save.Item(n).GetFormat());

	return match;
}

/// ファイル保存時のフォーマット名を返す
/// @param[in] index ダイアログ内にある拡張子リストの位置
/// @param[out] format フォーマット名
/// @attention 事前にGetWildcardForSave()を実行する必要がある
void FileTypes::GetFormatByIndexForSave(int index, wxString &format) const
{
	if (index < 0 || index >= (int)idx_for_save.Count()) {
		return;
	}
	int n = idx_for_save.Item(index);
	format = wcard_for_save.Item(n).GetFormat();
}

/// ファイル保存時の拡張子を返す
/// @param[in] index ダイアログ内にある拡張子リストの位置
/// @param[out] ext 拡張子
/// @attention 事前にGetWildcardForSave()を実行する必要がある
void FileTypes::GetExtByIndexForSave(int index, wxString &ext) const
{
	if (index < 0 || index >= (int)idx_for_save.Count()) {
		return;
	}
	int n = idx_for_save.Item(index);
	ext = wcard_for_save.Item(n).GetExt();
}
