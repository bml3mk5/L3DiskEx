/// @file intnamevalid.h
///
/// @brief 内部ファイル名バリデータ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _INTNAMEVALID_H_
#define _INTNAMEVALID_H_

#include "../common.h"
#include <wx/validate.h>
#include <wx/valtext.h>


class wxTextEntry;
class DiskBasic;
class DiskBasicDirItem;
class ValidNameRule;

//////////////////////////////////////////////////////////////////////

//
//
//

/// 内部ファイル名バリデータ
class IntNameValidator : public wxValidator
{
private:
	wxString m_subject;	///< 対象となる属性名("file name" etc.)
//	DiskBasic *m_basic;
	DiskBasicDirItem *m_item;
	wxString m_valchrs;	///< 入力できる文字
	wxString m_invchrs;	///< 入力できない文字
	wxString m_dupchrs;	///< 重複指定できない文字
	wxString m_fstchrs;	///< ファイル名先頭に入力できる文字
	bool m_require;		///< 名前必須か
	size_t m_maxlen;

	bool ContainsIncludedCharacters(const wxString& val, wxString &invchr) const;
	bool ContainsExcludedCharacters(const wxString& val, wxString &invchr) const;
	bool ContainsDuplicatedCharacters(const wxString& val, wxString &invchr) const;
	bool ContainsIncludedCharactersAtFirst(const wxString& val, wxString &invchr) const;

public:
	IntNameValidator(DiskBasicDirItem *item, const wxString &subject, const ValidNameRule &valid_chars);
	IntNameValidator(size_t maxlen, const wxString &subject, const ValidNameRule &valid_chars);
	IntNameValidator(const IntNameValidator& val);

	void CreateValidator(DiskBasicDirItem *item, const wxString &subject, const ValidNameRule &valid_chars);
	void CreateValidator(size_t maxlen, const wxString &subject, const ValidNameRule &valid_chars);

	wxObject *Clone() const { return new IntNameValidator(*this); }

	bool Copy(const IntNameValidator& val);

	wxTextEntry *GetTextEntry();

	bool Validate(wxWindow *parent);
	bool Validate(wxWindow *parent, const wxString &val);
	bool TransferToWindow();
	bool TransferFromWindow();

	void OnChar(wxKeyEvent& event);

    wxDECLARE_EVENT_TABLE();
};

//////////////////////////////////////////////////////////////////////

/// 日付用バリデータ
class DateTimeValidator : public wxTextValidator
{
private:
	bool m_is_time;
	bool m_require;

public:
	DateTimeValidator(bool is_time, bool required);
	DateTimeValidator(const DateTimeValidator &src);
	wxObject *Clone() const;
	bool Validate(wxWindow *parent);
};

//////////////////////////////////////////////////////////////////////

/// アドレス用バリデータ
class AddressValidator : public wxTextValidator
{
private:
public:
	AddressValidator();
	AddressValidator(const AddressValidator &src);
	wxObject *Clone() const;
	bool Validate(wxWindow *parent);
};

#endif /* _INTNAMEVALID_H_ */

