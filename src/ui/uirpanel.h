/// @file uirpanel.h
///
/// @brief 右パネル
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UIRPANEL_H_
#define _UIRPANEL_H_

#include "../common.h"
#include <wx/string.h>
#include <wx/panel.h>
#include <wx/splitter.h>


class UiDiskFrame;
class UiDiskRPanel;
class UiDiskRBPanel;
class UiDiskDiskAttr;
class UiDiskFileList;
class UiDiskRawPanel;

/// 右パネル
class UiDiskRPanel : public wxSplitterWindow
{
private:
	wxWindow *parent;
	UiDiskFrame *frame;

	UiDiskDiskAttr *diskattr;
	UiDiskRBPanel  *bpanel;

public:
	UiDiskRPanel(UiDiskFrame *parentframe, wxWindow *parentwindow, int selected_window);
	/// パネルの切り替え
	void ChangePanel(int num);
	/// ディスク属性パネルを返す
	UiDiskDiskAttr *GetDiskAttrPanel() const { return diskattr; }
	/// ファイルリストパネルを返す
	UiDiskFileList *GetFileListPanel(bool inst = false) const;
	/// Rawディスクパネルを返す
	UiDiskRawPanel *GetRawPanel(bool inst = false) const;
	/// フォントをセット
	void SetListFont(const wxFont &font);

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(UiDiskRPanel);
};

#define USE_SPLITTER_WINDOW_ON_RBPANEL 1

#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
/// 右下パネル
class UiDiskRBPanel : public wxSplitterWindow
#else
/// 右下パネル
class UiDiskRBPanel : public wxPanel
#endif
{
private:
	UiDiskRPanel *parent;
	UiDiskFrame *frame;

	UiDiskFileList *filelist;
	UiDiskRawPanel *rawpanel;
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
	wxPanel *proppanel;
#endif

public:
	UiDiskRBPanel(UiDiskFrame *parentframe, UiDiskRPanel *parentwindow, int selected_window);

	void ChangePanel(int num);

	UiDiskFileList *GetFileListPanel(bool inst = false) const;
	UiDiskRawPanel *GetRawPanel(bool inst = false) const;

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(UiDiskRBPanel);
};

#endif /* _UIRBPANEL_H_ */

