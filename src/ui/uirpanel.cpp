/// @file uirpanel.cpp
///
/// @brief 右パネル
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uirpanel.h"
#include "../main.h"
#include "uidiskattr.h"
#include "uirawdisk.h"
#include "uifilelist.h"


//
// 右パネル上下
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskRPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

L3DiskRPanel::L3DiskRPanel(L3DiskFrame *parentframe, wxWindow *parentwindow, int selected_window)
                : wxSplitterWindow(parentwindow, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_BORDER | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
	parent = parentwindow;
    frame = parentframe;

	// fit size
	SetSize(parentwindow->GetSize());

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	diskattr = new L3DiskDiskAttr(parentframe, this);
	bpanel = new L3DiskRBPanel(parentframe, this, selected_window);

	SplitHorizontally(diskattr, bpanel, 32);

	SetMinimumPaneSize(10);
}

/// パネルの切り替え
/// @param [in] num 0:ファイルリスト 1:RAWディスク
void L3DiskRPanel::ChangePanel(int num)
{
	if (bpanel) bpanel->ChangePanel(num);
}

/// ファイルリストパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
/// @return パネル
L3DiskFileList *L3DiskRPanel::GetFileListPanel(bool inst) const
{
	if (bpanel) return bpanel->GetFileListPanel(inst);
	else return NULL;
}

/// Rawディスクパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
/// @return パネル
L3DiskRawPanel *L3DiskRPanel::GetRawPanel(bool inst) const
{
	if (bpanel) return bpanel->GetRawPanel(inst);
	else return NULL;
}

/// フォントをセット
/// @param [in] font フォント
void L3DiskRPanel::SetListFont(const wxFont &font)
{
	L3DiskDiskAttr *dattr = GetDiskAttrPanel();
	if (dattr) dattr->SetListFont(font);

	L3DiskFileList *flist = GetFileListPanel(true);
	if (flist) flist->SetListFont(font);

	L3DiskRawPanel *rlist = GetRawPanel(true);
	if (rlist) rlist->SetListFont(font);
}

//
// 右下パネル
//
// Attach Event
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
wxBEGIN_EVENT_TABLE(L3DiskRBPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()
#else
wxBEGIN_EVENT_TABLE(L3DiskRBPanel, wxPanel)
wxEND_EVENT_TABLE()
#endif

#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
L3DiskRBPanel::L3DiskRBPanel(L3DiskFrame *parentframe, L3DiskRPanel *parentwindow, int selected_window)
                : wxSplitterWindow(parentwindow, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_BORDER | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
#else
L3DiskRBPanel::L3DiskRBPanel(L3DiskFrame *parentframe, L3DiskRPanel *parentwindow, int selected_window)
				: wxPanel(parentwindow, wxID_ANY,
				   wxDefaultPosition, wxDefaultSize	)
#endif
{
	parent = parentwindow;
    frame = parentframe;

	// fit size
	SetSize(parentwindow->GetSize());

#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
	// resize right window when resize parent window.
	SetSashGravity(0.0);
#endif

	filelist = new L3DiskFileList(parentframe, this);
	rawpanel = new L3DiskRawPanel(parentframe, this);
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
//	bindump = new L3DiskBinDump(parentframe, this);
	proppanel = new wxPanel(this);
#endif

	switch(selected_window) {
	case 1:
		// RAWモード
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
//		SplitVertically(rawpanel, bindump, 500);
		SplitVertically(rawpanel, proppanel, 600);
		Unsplit(proppanel);
#endif
		filelist->Hide();
		break;
	default:
		// BASICモード
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
//		SplitVertically(filelist, bindump, 500);
		SplitVertically(filelist, proppanel, 600);
		Unsplit(proppanel);
#endif
		rawpanel->Hide();
		break;
	}

#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
	SetMinimumPaneSize(10);
#endif
}

void L3DiskRBPanel::ChangePanel(int num)
{
	switch(num) {
	case 1:
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
		if (GetWindow1() == filelist) {
			ReplaceWindow(filelist, rawpanel);
//			SplitVertically(rawpanel, proppanel, 640);
			rawpanel->Show();
			filelist->Hide();
			filelist->ClearAttr();
			filelist->ClearFiles();
		}
#else
		rawpanel->Show();
		filelist->Hide();
#endif
		break;
	default:
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
		if (GetWindow1() == rawpanel) {
			ReplaceWindow(rawpanel, filelist);
//			Unsplit(proppanel);
			filelist->Show();
			rawpanel->Hide();
			rawpanel->ClearTrackListData();
			rawpanel->ClearSectorListData();
		}
#else
		filelist->Show();
		rawpanel->Hide();
#endif
		break;
	}
}

/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
L3DiskFileList *L3DiskRBPanel::GetFileListPanel(bool inst) const
{
	if (filelist && (inst || filelist->IsShown())) return filelist;
	else return NULL;
}

/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
L3DiskRawPanel *L3DiskRBPanel::GetRawPanel(bool inst) const
{
	if (rawpanel && (inst || rawpanel->IsShown())) return rawpanel;
	else return NULL;
}
