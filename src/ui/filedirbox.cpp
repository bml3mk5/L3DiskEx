/// @file filedirbox.cpp
///
/// @brief ファイル＆ディレクトリ選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "filedirbox.h"

#ifndef USE_CONSOLE

#if defined(__WXOSX__)
#include <wx/sysopt.h>
#endif

#define MYDISK_TRANS \
	_("can't open file '%s'") \
	_("can't create file '%s'") \
	_("can't close file descriptor %d") \
	_("can't read from file descriptor %d") \
	_("can't write to file descriptor %d") \
	_("can't flush file descriptor %d") \
	_("can't seek on file descriptor %d") \
	_("can't get seek position on file descriptor %d")

//////////////////////////////////////////////////////////////////////
//
// File Open Dialog
//
UiDiskOpenFileDialog::UiDiskOpenFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard, long style)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wildcard, style | wxFD_OPEN)
{
}

//////////////////////////////////////////////////////////////////////
//
// File Save Dialog
//
UiDiskSaveFileDialog::UiDiskSaveFileDialog(const wxString& message, const wxString& defaultDir, const wxString& defaultFile, const wxString& wildcard)
            : wxFileDialog(NULL, message, defaultDir, defaultFile, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT)
{
#if defined(__WXOSX__)
	wxSystemOptions::SetOption(wxOSX_FILEDIALOG_ALWAYS_SHOW_TYPES, 1);
#endif
}

//////////////////////////////////////////////////////////////////////
//
// Dir Dialog
//
UiDiskDirDialog::UiDiskDirDialog(const wxString& message, const wxString& defaultDir, long style)
            : wxDirDialog(NULL, message, defaultDir, style)
{
}

#endif /* !USE_CONSOLE */
