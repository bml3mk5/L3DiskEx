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


class L3DiskFrame;
class L3DiskRPanel;
class L3DiskRBPanel;
class L3DiskDiskAttr;
class L3DiskFileList;
class L3DiskRawPanel;

/// 右パネル
class L3DiskRPanel : public wxSplitterWindow
{
private:
	wxWindow *parent;
	L3DiskFrame *frame;

	L3DiskDiskAttr *diskattr;
	L3DiskRBPanel  *bpanel;

public:
	L3DiskRPanel(L3DiskFrame *parentframe, wxWindow *parentwindow, int selected_window);
	/// パネルの切り替え
	void ChangePanel(int num);
	/// ディスク属性パネルを返す
	L3DiskDiskAttr *GetDiskAttrPanel() const { return diskattr; }
	/// ファイルリストパネルを返す
	L3DiskFileList *GetFileListPanel(bool inst = false) const;
	/// Rawディスクパネルを返す
	L3DiskRawPanel *GetRawPanel(bool inst = false) const;
	/// フォントをセット
	void SetListFont(const wxFont &font);

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRPanel);
};

#define USE_SPLITTER_WINDOW_ON_RBPANEL 1

#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
/// 右下パネル
class L3DiskRBPanel : public wxSplitterWindow
#else
/// 右下パネル
class L3DiskRBPanel : public wxPanel
#endif
{
private:
	L3DiskRPanel *parent;
	L3DiskFrame *frame;

	L3DiskFileList *filelist;
	L3DiskRawPanel *rawpanel;
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
	wxPanel *proppanel;
#endif

public:
	L3DiskRBPanel(L3DiskFrame *parentframe, L3DiskRPanel *parentwindow, int selected_window);

	void ChangePanel(int num);

	L3DiskFileList *GetFileListPanel(bool inst = false) const;
	L3DiskRawPanel *GetRawPanel(bool inst = false) const;

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRBPanel);
};

#endif /* _UIRBPANEL_H_ */

