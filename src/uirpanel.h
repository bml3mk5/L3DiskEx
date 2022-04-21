/// @file uirpanel.h
///
/// @brief 右パネル
///
#ifndef _UIRPANEL_H_
#define _UIRPANEL_H_

#include "common.h"
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

	void ChangePanel(int num);

	L3DiskDiskAttr *GetDiskAttrPanel() const { return diskattr; }

	L3DiskFileList *GetFileListPanel(bool inst = false) const;

	L3DiskRawPanel *GetRawPanel(bool inst = false) const;

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
//	L3DiskBinDump  *bindump;
	wxPanel *proppanel;
#endif

public:
	L3DiskRBPanel(L3DiskFrame *parentframe, L3DiskRPanel *parentwindow, int selected_window);

	void ChangePanel(int num);

	L3DiskFileList *GetFileListPanel(bool inst = false) const;
	L3DiskRawPanel *GetRawPanel(bool inst = false) const;

#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
//	L3DiskBinDump *GetBinDumpPanel() const { return bindump; }
//	void SetBinDumpData(const wxUint8 *buf, size_t len);
//	void ClearBinDumpData();
//	void ClearData();
#endif

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(L3DiskRBPanel);
};

#endif /* _UIRBPANEL_H_ */

