/// @file uimainpanel.cpp
///
/// @brief メインパネル
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uimainpanel.h"

#ifndef USE_CONSOLE

#include "uimainframe.h"
#include "uidisklist.h"
#include "uirpanel.h"
#include "uirawdisk.h"
#include "../diskimg/diskimage.h"

//////////////////////////////////////////////////////////////////////

// ドラッグアンドドロップ時のフォーマットID
wxDataFormat *UiDiskPanelDataFormat = NULL;

//
// メインパネルは分割ウィンドウ
//
// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskPanel, wxSplitterWindow)
wxEND_EVENT_TABLE()

UiDiskPanel::UiDiskPanel(UiDiskFrame *parent)
                : wxSplitterWindow(parent, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_BORDER | wxSP_LIVE_UPDATE |
                                   wxCLIP_CHILDREN /* | wxSP_NO_XP_THEME */ )
{
    frame = parent;

	// resize right window when resize parent window.
	SetSashGravity(0.0);

	// control panel
	lpanel = new UiDiskList(frame, this);
	rpanel = new UiDiskRPanel(frame, this, frame->GetSelectedMode());

	int w = gConfig.GetLPanelWidth();
	SplitVertically(lpanel, rpanel, FromDIP(w));

	SetMinimumPaneSize(10);

	SetDropTarget(new UiDiskPanelDropTarget(parent, this));
}

UiDiskPanel::~UiDiskPanel()
{
	int w = GetSashPosition();
	gConfig.SetLPanelWidth(ToDIP(w));
}

/// 外部からのDnD
/// @param[in] x         ドロップした位置X
/// @param[in] y         ドロップした位置Y
/// @param[in] filenames ドロップしたファイル名一覧
/// @return ドロップ成功/失敗
bool UiDiskPanel::ProcessDroppedFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	if (filenames.Count() == 0) return false;

	int drop_type = 0;

	// 分割位置
	int pos_x = GetSashPosition();
	bool disk_is_empty = (frame->GetDiskImage().CountDisks() == 0);
	bool include_dir = false;
	// 一覧にディレクトリが存在するか
	for(size_t i=0; i<filenames.Count(); i++) {
		if (wxFileName::DirExists(filenames.Item(i))) {
			include_dir = true;
			break;
		}
	}

	if (disk_is_empty) {
		// ディスクイメージを開いていない
		drop_type = 2;
	} else if (x < pos_x) {
		// 右側のツリーにドロップしている
		if (lpanel->HasNodeAtPoint(x, y)) {
			// ツリー内部のディレクトリなどにドロップしている
			drop_type = 1;
		} else {
			// ツリー外
			drop_type = 2;
		}
	}

	bool sts = false;
	switch(drop_type) {
	case 0:
		{
			// ファイルをインポート
			UiDiskFileList *file_list = rpanel->GetFileListPanel();
			if (file_list) {
				sts = file_list->DropDataFiles(this, x, y, filenames, include_dir);
			}
			UiDiskRawPanel *raw_panel = rpanel->GetRawPanel();
			if (raw_panel) {
				for(int n = 0; n < (int)filenames.Count(); n++) {
					wxString filename = filenames.Item(n);
					sts = raw_panel->ShowImportTrackRangeDialog(filename);
				}
			}
		}
		break;
	case 1:
		// 指定フォルダにファイルをインポート
		if (lpanel) {
			sts = lpanel->DropDataFiles(this, x, y, filenames, include_dir);
		}
		break;
	case 2:
		// ディスクイメージ１つだけ開く
		if (!include_dir) {
			frame->OpenDroppedFile(filenames.Item(0));
		}
		break;
	default:
		// nothing to do
		break;
	}
	return sts;
}

//////////////////////////////////////////////////////////////////////
//
// File Drag and Drop
//
UiDiskPanelDropTarget::UiDiskPanelDropTarget(UiDiskFrame *parentframe, UiDiskPanel *parentwindow)
	: wxDropTarget()
{
	parent = parentwindow;
	frame = parentframe;

#ifdef USE_DATA_OBJECT_COMPOSITE
	wxDataObjectComposite* dataobj = new wxDataObjectComposite();

	dataobj->Add(new wxFileDataObject());
	SetDataObject(dataobj);
#else
	SetDataObject(new wxFileDataObject());
#endif
}

wxDragResult UiDiskPanelDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	if ( !GetData() ) return wxDragNone;
#ifdef USE_DATA_OBJECT_COMPOSITE
	bool sts = false;
	wxDataObjectComposite *comobj = (wxDataObjectComposite *)GetDataObject();
	if (comobj) {
		wxDataFormat fmt = comobj->GetReceivedFormat();
		if (fmt.GetType() == wxDF_FILENAME) {
			// エクスプローラからのDnD
			wxFileDataObject *dobj = (wxFileDataObject *)comobj->GetObject(fmt);
			sts = parent->ProcessDroppedFiles(x, y, dobj->GetFilenames());
		}
	}
#else
	bool sts = false;
	wxFileDataObject *dobj = (wxFileDataObject *)GetDataObject();
	if (dobj) {
		wxArrayString filenames;
#ifdef __WXOSX__
		size_t cnt = dobj->GetFilenames().Count();
		if (cnt == 1) {
			// ファイルパスが改行で区切られている可能性がある
			wxString str = dobj->GetFilenames().Item(0);
			filenames = wxSplit(str.Trim(), 0x0d);
		} else {
			filenames = dobj->GetFilenames();
		}
#else
		filenames = dobj->GetFilenames();
#endif
		sts = parent->ProcessDroppedFiles(x, y, filenames);
	}
#endif
	return (sts ? def : wxDragError);
}

#endif /* !USE_CONSOLE */
