/// @file filedirbox.h
///
/// @brief ファイル＆ディレクトリ選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef FILEDIRBOX_H
#define FILEDIRBOX_H

#include "../common.h"

#ifndef USE_CONSOLE

#include <wx/filedlg.h>
#include <wx/dirdlg.h>

//////////////////////////////////////////////////////////////////////

/// ファイルオープンダイアログ
class UiDiskOpenFileDialog: public wxFileDialog
{
public:
	UiDiskOpenFileDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, const wxString& defaultFile = wxEmptyString, const wxString& wildcard = wxFileSelectorDefaultWildcardStr, long style = 0);

};

//////////////////////////////////////////////////////////////////////

/// ファイルセーブダイアログ
class UiDiskSaveFileDialog: public wxFileDialog
{
public:
	UiDiskSaveFileDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, const wxString& defaultFile = wxEmptyString, const wxString& wildcard = wxFileSelectorDefaultWildcardStr);

};

//////////////////////////////////////////////////////////////////////

/// ディレクトリダイアログ
class UiDiskDirDialog: public wxDirDialog
{
public:
	UiDiskDirDialog(const wxString& message, const wxString& defaultDir = wxEmptyString, long style = wxDD_DEFAULT_STYLE);

};

#endif /* !USE_CONSOLE */

#endif /* FILEDIRBOX_H */

