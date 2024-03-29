/// @file uifatarea.h
///
/// @brief 使用状況を表示
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UI_FATAREA_H_
#define _UI_FATAREA_H_

#include "../common.h"
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include <wx/dynarray.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include "../basicfmt/basicfat.h"


class UiDiskFrame;
class UiDiskFatAreaFrame;
class UiDiskFatAreaPanel;
class DiskBasicGroups;

/// FAT使用状況ウィンドウ
class UiDiskFatAreaFrame: public wxFrame
{
private:
	UiDiskFatAreaPanel *panel;

public:

    UiDiskFatAreaFrame(UiDiskFrame *parent, const wxString& title, const wxSize& size);
	~UiDiskFatAreaFrame();

	void OnClose(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	void SetData(wxUint32 offset, const wxArrayInt *arr);
	void ClearData();
	void SetGroup(wxUint32 group_num);
	void SetGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums);
	void UnsetGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums);
	void ClearGroup();

	enum en_menu_id
	{
		// menu id
		IDM_EXIT = 1,
	};

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(UiDiskFatAreaFrame);
};


/// FAT使用状況 内部パネル
class UiDiskFatAreaPanel : public wxScrolled<wxPanel>
{
private:
	UiDiskFatAreaFrame *frame;

	wxUint32   offset;	///< 開始グループ番号
	wxArrayInt datas;	///< 各グループの状態

	wxPen   pens[FAT_AVAIL_NULLEND];

	wxBrush brushes[FAT_AVAIL_NULLEND];
	wxBrush brush_select;
	wxBrush brush_extra;

	wxSize sq;
	int margin;
	int lpadding;
	int rpadding;
	int ll;

	void SetGroupBase(wxUint32 group_num, int highlight);
	void UnsetGroupBase(wxUint32 group_num);
	void ClearGroupBase();

public:
	UiDiskFatAreaPanel(UiDiskFatAreaFrame *parent);
	~UiDiskFatAreaPanel();

	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

	void SetData(wxUint32 offset, const wxArrayInt *arr);
	void ClearData();
	void SetGroup(wxUint32 group_num);
	void SetGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums);
	void UnsetGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums);
	void ClearGroup();

	wxDECLARE_EVENT_TABLE();
	wxDECLARE_NO_COPY_CLASS(UiDiskFatAreaPanel);
};


#endif /* _UI_FATAREA_H_ */
