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
wxBEGIN_EVENT_TABLE(UiDiskRPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

UiDiskRPanel::UiDiskRPanel(UiDiskFrame *parentframe, wxWindow *parentwindow, int selected_window)
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

	diskattr = new UiDiskDiskAttr(parentframe, this);
	bpanel = new UiDiskRBPanel(parentframe, this, selected_window);

	SplitHorizontally(diskattr, bpanel, 32);

	SetMinimumPaneSize(10);
}

/// パネルの切り替え
/// @param [in] num 0:ファイルリスト 1:RAWディスク
void UiDiskRPanel::ChangePanel(int num)
{
	if (bpanel) bpanel->ChangePanel(num);
}

/// ファイルリストパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
/// @return パネル
UiDiskFileList *UiDiskRPanel::GetFileListPanel(bool inst) const
{
	if (bpanel) return bpanel->GetFileListPanel(inst);
	else return NULL;
}

/// Rawディスクパネルを返す
/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
/// @return パネル
UiDiskRawPanel *UiDiskRPanel::GetRawPanel(bool inst) const
{
	if (bpanel) return bpanel->GetRawPanel(inst);
	else return NULL;
}

/// フォントをセット
/// @param [in] font フォント
void UiDiskRPanel::SetListFont(const wxFont &font)
{
	UiDiskDiskAttr *dattr = GetDiskAttrPanel();
	if (dattr) dattr->SetListFont(font);

	UiDiskFileList *flist = GetFileListPanel(true);
	if (flist) flist->SetListFont(font);

	UiDiskRawPanel *rlist = GetRawPanel(true);
	if (rlist) rlist->SetListFont(font);
}

//
// 右下パネル
//
// Attach Event
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
wxBEGIN_EVENT_TABLE(UiDiskRBPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()
#else
wxBEGIN_EVENT_TABLE(UiDiskRBPanel, wxPanel)
wxEND_EVENT_TABLE()
#endif

#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
UiDiskRBPanel::UiDiskRBPanel(UiDiskFrame *parentframe, UiDiskRPanel *parentwindow, int selected_window)
                : wxSplitterWindow(parentwindow, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_BORDER | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
#else
UiDiskRBPanel::UiDiskRBPanel(UiDiskFrame *parentframe, UiDiskRPanel *parentwindow, int selected_window)
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

	filelist = new UiDiskFileList(parentframe, this);
	rawpanel = new UiDiskRawPanel(parentframe, this);
#ifdef USE_SPLITTER_WINDOW_ON_RBPANEL
//	bindump = new UiDiskBinDump(parentframe, this);
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

void UiDiskRBPanel::ChangePanel(int num)
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
UiDiskFileList *UiDiskRBPanel::GetFileListPanel(bool inst) const
{
	if (filelist && (inst || filelist->IsShown())) return filelist;
	else return NULL;
}

/// @param [in] inst  true:常にポインタを返す / false:リスト非表示ならNULLを返す 
UiDiskRawPanel *UiDiskRBPanel::GetRawPanel(bool inst) const
{
	if (rawpanel && (inst || rawpanel->IsShown())) return rawpanel;
	else return NULL;
}
