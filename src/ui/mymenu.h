/// @file mymenu.h
///
/// @brief メニューコントロール ラッパークラス
///
#ifndef MYMENU_H
#define MYMENU_H

#include "../common.h"
#include <wx/menu.h>
#include <wx/string.h>

/// メニューコントロール ラッパークラス
class MyMenu : public wxMenu
{
public:
	MyMenu();

	wxMenuItem *Append(int id, const wxString &item=wxEmptyString, const wxString &helpString=wxEmptyString, wxItemKind kind=wxITEM_NORMAL);
	wxMenuItem *Append(int id, const wxString &item, wxMenu *subMenu, const wxString &helpString=wxEmptyString);
	wxMenuItem *AppendCheckItem(int id, const wxString &item, const wxString &help=wxEmptyString);
	wxMenuItem *AppendRadioItem(int id, const wxString &item, const wxString &help=wxEmptyString);

	/// 文字列内にある"(&A)"や"&A"などの文字列をとり除く
	static wxString ConvItemString(const wxString &str);
};

/// メニューバーコントロール ラッパークラス
class MyMenuBar : public wxMenuBar
{
public:
	MyMenuBar();

	bool Append(wxMenu *menu, const wxString &title);
};

#endif /* MYMENU_H */
