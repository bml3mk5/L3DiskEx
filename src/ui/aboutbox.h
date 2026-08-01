/// @file aboutbox.h
///
/// @brief Aboutダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef ABOUTBOX_H
#define ABOUTBOX_H

#include "../common.h"

#ifndef USE_CONSOLE

#include <wx/dialog.h>

//////////////////////////////////////////////////////////////////////

/// About dialog
class UiDiskAbout : public wxDialog
{
public:
	UiDiskAbout(wxWindow* parent, wxWindowID id);
};

#endif /* !USE_CONSOLE */

#endif /* ABOUTBOX_H */

