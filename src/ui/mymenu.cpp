/// @file mymenu.cpp
///
/// @brief メニューコントロール
///
#include "mymenu.h"
#include <wx/regex.h>

// メニューコントロール

MyMenu::MyMenu()
	: wxMenu()
{
}

wxMenuItem *MyMenu::Append(int id, const wxString &item, const wxString &helpString, wxItemKind kind)
{
	return wxMenu::Append(id, ConvItemString(item), helpString, kind);
}

wxMenuItem *MyMenu::Append(int id, const wxString &item, wxMenu *subMenu, const wxString &helpString)
{
	return wxMenu::Append(id, ConvItemString(item), subMenu, helpString);
}

wxMenuItem *MyMenu::AppendCheckItem(int id, const wxString &item, const wxString &help)
{
	return wxMenu::AppendCheckItem(id, ConvItemString(item), help);
}

wxMenuItem *MyMenu::AppendRadioItem(int id, const wxString &item, const wxString &help)
{
	return wxMenu::AppendRadioItem(id, ConvItemString(item), help);
}

/// 文字列内にある"(&A)"や"&A"などの文字列をとり除く
wxString MyMenu::ConvItemString(const wxString &str)
{
	wxString nstr = str;
#if defined(__WXOSX__)
	// for MacOSX
	wxRegEx re1(wxT("\\(\\&[0-9A-Za-z]\\)"));
	while(re1.Matches(nstr)) {
		size_t st, len;
		re1.GetMatch(&st, &len, 0);
		nstr = nstr.Mid(0, st) + nstr.Mid(st + len);
	}
	wxRegEx re2(wxT("\\&[0-9A-Za-z]"));
	while(re2.Matches(nstr)) {
		size_t st, len;
		re2.GetMatch(&st, &len, 0);
		nstr = nstr.Mid(0, st) + nstr.Mid(st + len - 1);
	}
#endif
#if defined(__WXOSX__) || defined(__WXGTK__)
	nstr.Replace(wxT("ALT+F4"), wxT("CTRL+Q"));
#endif
	return nstr;
}

// メニューバーコントロール

MyMenuBar::MyMenuBar()
	: wxMenuBar()
{
}

bool MyMenuBar::Append(wxMenu *menu, const wxString &title)
{
	return wxMenuBar::Append(menu, MyMenu::ConvItemString(title));
}

