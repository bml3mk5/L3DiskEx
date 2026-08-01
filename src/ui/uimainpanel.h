/// @file uimainpanel.h
///
/// @brief メインパネル
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef UIMAINPANEL_H
#define UIMAINPANEL_H

#include "../common.h"

#ifndef USE_CONSOLE

#include <wx/string.h>
#include <wx/dynarray.h>
#include "../config.h"
#include "../utils.h"

#include <wx/panel.h>
#include <wx/dnd.h>
#include <wx/splitter.h>
#include "uicommon.h"

class UiDiskApp;
class UiDiskFrame;
class UiDiskPanel;
class UiDiskList;
class UiDiskRPanel;

//////////////////////////////////////////////////////////////////////


/// 分割ウィンドウ
class UiDiskPanel : public wxSplitterWindow
{
private:
	UiDiskFrame *frame;

	UiDiskList *lpanel;
	UiDiskRPanel *rpanel;

public:
	UiDiskPanel(UiDiskFrame *parent);
	~UiDiskPanel();

	// event handlers

	UiDiskList *GetLPanel() { return lpanel; }
	UiDiskRPanel *GetRPanel() { return rpanel; }

	bool ProcessDroppedFiles(wxCoord x, wxCoord y, const wxArrayString &filenames);

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(UiDiskPanel);
};

//////////////////////////////////////////////////////////////////////

/// ドラッグ＆ドロップ
class UiDiskPanelDropTarget : public wxDropTarget
{
	UiDiskPanel *parent;
    UiDiskFrame *frame;

public:
    UiDiskPanelDropTarget(UiDiskFrame *parentframe, UiDiskPanel *parentwindow);

	wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);
};

#endif /* !USE_CONSOLE */

#endif /* UIMAINPANEL_H */

