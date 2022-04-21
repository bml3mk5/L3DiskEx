/// @file intnamevalid.cpp
///
/// @brief 内部ファイル名バリデータ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "intnamevalid.h"
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basicdiritem.h"
#include "../utils.h"


#define TEXT_VALIDATOR_TRANS \
_("'%s' should only contain ASCII characters."), \
_("'%s' should only contain alphabetic characters."), \
_("'%s' should only contain alphabetic or numeric characters."), \
_("'%s' should only contain digits."), \
_("'%s' should be numeric."), \
_("'%s' doesn't consist only of valid characters"), \
_("'%s' contains illegal characters")

//////////////////////////////////////////////////////////////////////
//
// 内部ファイル名バリデータ
//
// Attach Events
wxBEGIN_EVENT_TABLE(IntNameValidator, wxValidator)
    EVT_CHAR(IntNameValidator::OnChar)
wxEND_EVENT_TABLE()

IntNameValidator::IntNameValidator(DiskBasicDirItem *item, const wxString &subject, const ValidNameRule &valid_chars)
	: wxValidator()
{
	CreateValidator(item, subject, valid_chars);
}
IntNameValidator::IntNameValidator(size_t maxlen, const wxString &subject, const ValidNameRule &valid_chars)
	: wxValidator()
{
	CreateValidator(maxlen, subject, valid_chars);
}
IntNameValidator::IntNameValidator(const IntNameValidator& val) : wxValidator()
{
    Copy(val);
}

/// バリデータ作成
void IntNameValidator::CreateValidator(DiskBasicDirItem *item, const wxString &subject, const ValidNameRule &valid_chars)
{
	m_subject = subject;
//	m_basic = basic;
	m_item = item;
	m_valchrs = valid_chars.GetValidChars();
	m_invchrs = valid_chars.GetInvalidChars();
	m_dupchrs = valid_chars.GetDeduplicateChars();
	m_fstchrs = valid_chars.GetValidFirstChars();
	m_require = valid_chars.IsNameRequired();
	m_maxlen = item->GetFileNameStrSize();
	if (m_maxlen == 0) m_maxlen = valid_chars.GetMaxLength();
	if (m_maxlen == 0) m_maxlen = 64;
}

/// バリデータ作成
void IntNameValidator::CreateValidator(size_t maxlen, const wxString &subject, const ValidNameRule &valid_chars)
{
	m_subject = subject;
//	m_basic = basic;
	m_item = NULL;
	m_valchrs = valid_chars.GetValidChars();
	m_invchrs = valid_chars.GetInvalidChars();
	m_dupchrs = valid_chars.GetDeduplicateChars();
	m_fstchrs = valid_chars.GetValidFirstChars();
	m_require = valid_chars.IsNameRequired();
	m_maxlen = maxlen;
	if (m_maxlen == 0) m_maxlen = valid_chars.GetMaxLength();
	if (m_maxlen == 0) m_maxlen = 64;
}

/// コピー
bool IntNameValidator::Copy(const IntNameValidator& val)
{
    wxValidator::Copy(val);
	m_subject = val.m_subject;
//	m_basic = val.m_basic;
	m_item = val.m_item;
	m_valchrs = val.m_valchrs;
	m_invchrs = val.m_invchrs;
	m_dupchrs = val.m_dupchrs;
	m_fstchrs = val.m_fstchrs;
	m_require = val.m_require;
	m_maxlen = val.m_maxlen;
	return true;
}

/// 入力フィールドを返す
wxTextEntry *IntNameValidator::GetTextEntry()
{
	if (wxDynamicCast(m_validatorWindow, wxTextCtrl)) {
		return (wxTextCtrl*)m_validatorWindow;
	}

	wxFAIL_MSG(
		"IntNameValidator can only be used with wxTextCtrl"
	);

	return NULL;
}

/// ファイル名の検査
bool IntNameValidator::Validate(wxWindow *parent)
{
	// If window is disabled, simply return
	if ( !m_validatorWindow->IsEnabled() ) return true;

	wxTextEntry * const text = GetTextEntry();
	if ( !text ) return false;

	wxString val(text->GetValue());

	return Validate(parent, val);
}

/// ファイル名の検査
bool IntNameValidator::Validate(wxWindow *parent, const wxString &val)
{
	wxString errormsg;
	wxString invchr;

	wxString subject = m_subject;

	if ( val.length() > m_maxlen ) {
		subject.MakeCapitalized();
		errormsg = wxString::Format(_("%s is too long."), subject);
	} else if ( !ContainsIncludedCharactersAtFirst(val, invchr) ) {
		errormsg = wxString::Format(_("The char '%s' is not able to use in the first of %s."), invchr, subject);
	} else if ( !ContainsIncludedCharacters(val, invchr) ) {
		errormsg = wxString::Format(_("The char '%s' is not able to use in %s."), invchr, subject);
	} else if ( ContainsExcludedCharacters(val, invchr) ) {
		errormsg = wxString::Format(_("The char '%s' is not able to use in %s."), invchr, subject);
	} else if ( ContainsDuplicatedCharacters(val, invchr) ) {
		errormsg = wxString::Format(_("The char '%s' is duplicated, so is not able to set in %s."), invchr, subject);
	} else if ( m_item ) {
		if (errormsg.empty()) {
			int len = m_item->ConvStringToChars(val, NULL, 32);
			if (len == 0 && m_require) {
				subject.MakeCapitalized();
				errormsg = wxString::Format(_("%s is required."), subject);
			} else if (len < 0) {
				errormsg = wxString::Format(_("Invalid char is contained in %s."), subject);
			}
		}
		if (errormsg.empty()) {
			m_item->ValidateFileName(parent, val, errormsg);
		}
	} else {
		if (errormsg.empty()) {
			if (val.IsEmpty() && m_require) {
				subject.MakeCapitalized();
				errormsg = wxString::Format(_("%s is required."), subject);
			}
		}
	}

	if ( !errormsg.empty() ) {
		if (m_validatorWindow) m_validatorWindow->SetFocus();
		wxMessageBox(errormsg, _("Validation conflict"),
			wxOK | wxICON_EXCLAMATION, parent);

		return false;
	}

    return true;
}

/// 変換可能か 
bool IntNameValidator::TransferToWindow()
{
	return true;
}

/// 変換可能か
bool IntNameValidator::TransferFromWindow()
{
	return true;
}

/// 設定できる文字がファイル名に含まれるか
/// @param[in] val     検査する文字列
/// @param[out] invchr 設定できない文字を返す
/// @return false:設定できない文字がある
bool IntNameValidator::ContainsIncludedCharacters(const wxString& val, wxString &invchr) const
{
	if (m_valchrs.IsEmpty()) return true;

	for (wxString::const_iterator i = val.begin(); i != val.end(); i++) {
		if (m_valchrs.Find(*i) == wxNOT_FOUND) {
			invchr = wxString((wxUniChar)*i);
			return false;
		}
	}
	return true;
}

/// 設定できない文字がファイル名に含まれるか
/// @param[in] val     検査する文字列
/// @param[out] invchr 設定できない文字を返す
/// @return false:設定できない文字がある
bool IntNameValidator::ContainsExcludedCharacters(const wxString& val, wxString &invchr) const
{
	for (wxString::const_iterator i = m_invchrs.begin(); i != m_invchrs.end(); i++) {
		if (val.Find(*i) != wxNOT_FOUND) {
			invchr = wxString((wxUniChar)*i);
			return true;
		}
	}
	return false;
}

/// 重複指定できない文字がファイル名に含まれるか
/// @param[in] val     検査する文字列
/// @param[out] invchr 設定できない文字を返す
/// @return false:設定できない文字がある
bool IntNameValidator::ContainsDuplicatedCharacters(const wxString& val, wxString &invchr) const
{
	int n = 0;
	for (wxString::const_iterator i = m_dupchrs.begin(); i != m_dupchrs.end() && n < 2; i++) {
		wxString str = val;
		int found = wxNOT_FOUND;
		n = 0;
		do {
			found = str.Find(*i);
			if (found >= 0) {
				invchr = wxString((wxUniChar)*i);
				str = str.Mid(found + 1);
				n++;
			} else {
				break;
			}
		} while(!str.IsEmpty());
	}
	return (n >= 2);
}

/// 設定できる文字がファイル名の先頭に含まれるか
/// @param[in] val     検査する文字列
/// @param[out] invchr 設定できない文字を返す
/// @return false:設定できない文字がある
bool IntNameValidator::ContainsIncludedCharactersAtFirst(const wxString& val, wxString &invchr) const
{
	if (m_fstchrs.IsEmpty()) return true;
	if (val.IsEmpty()) return true;

	wxString::const_iterator i = val.begin();
	if (m_fstchrs.Find(*i) == wxNOT_FOUND) {
		invchr = wxString((wxUniChar)*i);
		return false;
	}
	return true;
}

/// キー押下時処理
void IntNameValidator::OnChar(wxKeyEvent& event)
{
	if (!m_validatorWindow) {
		event.Skip();
		return;
	}
	int keyCode = event.GetKeyCode();

	if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode >= WXK_START) {
		event.Skip();
		return;
	}

	wxString str((wxUniChar)keyCode, 1);
	wxString invchr;
	if (!ContainsIncludedCharacters(str, invchr)) {
		if ( !wxValidator::IsSilent() )	wxBell();
		return;
	} else if (ContainsExcludedCharacters(str, invchr)) {
		if ( !wxValidator::IsSilent() )	wxBell();
		return;
	} else {
		event.Skip();
	}
}

//////////////////////////////////////////////////////////////////////
//
// 日付用バリデータ
//
DateTimeValidator::DateTimeValidator(bool is_time, bool required)
	: wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
{
	m_is_time = is_time;
	m_require = required;
	SetCharIncludes("0123456789/:.-");
}
DateTimeValidator::DateTimeValidator(const DateTimeValidator &src)
	: wxTextValidator(src)
{
	m_is_time = src.m_is_time;
	m_require = src.m_require;
}

wxObject *DateTimeValidator::Clone() const
{
	return (new DateTimeValidator(*this));
}

bool DateTimeValidator::Validate(wxWindow *parent)
{
    // If window is disabled, simply return
    if (!m_validatorWindow->IsEnabled()) return true;

    wxTextEntry * const text = GetTextEntry();
    if (!text) return true;
	if (!text->IsEditable()) return true;

	wxString val(text->GetValue());
	wxString errormsg;
	TM tm;
	bool valid = true;
	if (!val.IsEmpty() && m_require) {
		if (m_is_time) {
			valid = Utils::ConvTimeStrToTm(val, tm);
			if (!valid) {
				errormsg = _("Invalid format is contained in date or time.");
			}
		} else {
			valid = Utils::ConvDateStrToTm(val, tm);
			if (!valid) {
				errormsg = _("Invalid format is contained in date or time.");
			}
		}
	}
	if ( !errormsg.empty() ) {
		m_validatorWindow->SetFocus();
		wxMessageBox(errormsg, _("Validation conflict"),
						wxOK | wxICON_WARNING, parent);
		// エラーにしない
//		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////
//
// アドレス用バリデータ
//
AddressValidator::AddressValidator()
	: wxTextValidator(wxFILTER_INCLUDE_CHAR_LIST)
{
	SetCharIncludes("0123456789abcdefABCDEF");
}
AddressValidator::AddressValidator(const AddressValidator &src)
	: wxTextValidator(src)
{
}

wxObject *AddressValidator::Clone() const
{
	return (new AddressValidator(*this));
}

bool AddressValidator::Validate(wxWindow *parent)
{
    // If window is disabled, simply return
    if (!m_validatorWindow->IsEnabled()) return true;

    wxTextEntry * const text = GetTextEntry();
    if (!text) return true;
	if (!text->IsEditable()) return true;

	wxString val(text->GetValue());
	wxString errormsg;
	bool valid = true;
	if (!val.IsEmpty()) {
		if (!valid) {
			errormsg = _("Invalid format is contained in address.");
		}
	}
	if ( !errormsg.empty() ) {
		m_validatorWindow->SetFocus();
		wxMessageBox(errormsg, _("Validation conflict"),
						wxOK | wxICON_WARNING, parent);
		// エラーにしない
//		return false;
	}
	return true;
}
