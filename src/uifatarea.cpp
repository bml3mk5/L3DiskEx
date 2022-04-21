/// @file uifatarea.cpp
///
/// @brief 使用状況を表示
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uifatarea.h"
#include <wx/dcclient.h>
#include "main.h"
#include "basiccommon.h"


//
// Frame
//

// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskFatAreaFrame, wxFrame)
	EVT_MENU(wxID_CLOSE, L3DiskFatAreaFrame::OnClose)
	EVT_MENU(wxID_EXIT,  L3DiskFatAreaFrame::OnQuit)
wxEND_EVENT_TABLE()

L3DiskFatAreaFrame::L3DiskFatAreaFrame(L3DiskFrame *parent, const wxString& title, const wxSize& size)
       : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, size)
{
#if 0
	// icon
#ifdef __WXMSW__
	SetIcon(wxIcon(_T("small")));
#elif defined(__WXGTK__) || defined(__WXMOTIF__)
	SetIcon(wxIcon(small_xpm));
#endif
#endif

//	// menu
//	wxMenu *menuFile = new wxMenu;
//
//	// file menu
//	menuFile->Append( wxID_EXIT, _("E&xit") );
//
//	// menu bar
//	wxMenuBar *menuBar = new wxMenuBar;
//	menuBar->Append( menuFile, _("&File") );
//
//	SetMenuBar( menuBar );

	// panel
	panel = new L3DiskFatAreaPanel(this);

	wxSize psz = panel->GetSize();
	wxSize csz = GetClientSize();
	SetClientSize(psz.x, csz.y > psz.y ? csz.y : psz.y); 
}

L3DiskFatAreaFrame::~L3DiskFatAreaFrame()
{
	L3DiskFrame *parent = (L3DiskFrame *)m_parent;
	parent->FatAreaWindowClosed();
}

void L3DiskFatAreaFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
	Close();
}

void L3DiskFatAreaFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	Close();
}

void L3DiskFatAreaFrame::SetData(wxUint32 offset, const wxArrayInt *arr)
{
	panel->SetData(offset, arr);
}

void L3DiskFatAreaFrame::ClearData()
{
	panel->ClearData();
}

void L3DiskFatAreaFrame::SetGroup(wxUint32 group_num)
{
	panel->SetGroup(group_num);
}

void L3DiskFatAreaFrame::SetGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums)
{
	panel->SetGroup(group_items, extra_group_nums);
}

void L3DiskFatAreaFrame::UnsetGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums)
{
	panel->UnsetGroup(group_items, extra_group_nums);
}

void L3DiskFatAreaFrame::ClearGroup()
{
	panel->ClearGroup();
}

//
// メインパネル
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskFatAreaPanel, wxScrolled<wxPanel>)
	EVT_PAINT(L3DiskFatAreaPanel::OnPaint)
	EVT_SIZE(L3DiskFatAreaPanel::OnSize)
wxEND_EVENT_TABLE()

L3DiskFatAreaPanel::L3DiskFatAreaPanel(L3DiskFatAreaFrame *parent)
	: wxScrolled<wxPanel>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
//	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    frame = parent;
	offset = 0;

	SetFont(wxFont(8, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

	sq.x = 10;
	sq.y = 10;
	margin = 2;		// margin
	lpadding = 8;	// left padding
	rpadding = lpadding + 16;	// right padding
	ll = 4;			// line length (half width)

	int x,y;
	x = lpadding + ll + margin + (sq.x + margin) * 16 + rpadding;
	y = 240;

	SetSize(x, y);
	SetVirtualSize(x, y * 100);
	SetScrollRate(10, 10);

	ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_DEFAULT);

	// pen color
	pens[FAT_AVAIL_FREE] = *wxBLACK_PEN;
	pens[FAT_AVAIL_SYSTEM] = *wxBLACK_PEN;
	pens[FAT_AVAIL_USED] = *wxBLACK_PEN;
	pens[FAT_AVAIL_USED_FIRST] = *wxBLACK_PEN;
	pens[FAT_AVAIL_USED_LAST] = *wxBLACK_PEN;
	pens[FAT_AVAIL_MISSING] = *wxGREY_PEN;
	pens[FAT_AVAIL_LEAK] = *wxLIGHT_GREY_PEN;

	// brush color
	brushes[FAT_AVAIL_FREE] = *wxWHITE_BRUSH;
	brushes[FAT_AVAIL_SYSTEM] = *wxGREY_BRUSH;
	brushes[FAT_AVAIL_USED] = *wxCYAN_BRUSH;
	brushes[FAT_AVAIL_USED_FIRST] = wxBrush(wxColor(0x00, 0xff, 0x80));
	brushes[FAT_AVAIL_USED_LAST] = wxBrush(wxColor(0x00, 0x80, 0xff));
	brushes[FAT_AVAIL_MISSING] = *wxLIGHT_GREY_BRUSH;
	brushes[FAT_AVAIL_LEAK] = wxBrush(wxColor(0xc0, 0xff, 0xff));

	brush_select = *wxRED_BRUSH;
	brush_extra = wxBrush(wxColor(0xff, 0, 0xff));
}

L3DiskFatAreaPanel::~L3DiskFatAreaPanel()
{
}

void L3DiskFatAreaPanel::OnPaint(wxPaintEvent& event)
{
	wxPaintDC dc(this);
	DoPrepareDC(dc);

	wxSize size = GetSize();

	int x = lpadding;
	int y = lpadding;

	dc.SetPen(*wxBLACK_PEN);

	int step = (sq.x + margin) * 4;
	size_t pos = 1;
	for(int px = (lpadding + ll + margin); px < (size.x - rpadding); px += step) {
		int py0 = y - (pos & 1) * ll;
		int py1 = y + ll;
		dc.DrawLine(px, py0, px, py1);
		if ((pos & 3) == 1) {
			wxString str = wxString::Format(wxT("%x"), (int)(pos >> 2));
			wxSize tsz = dc.GetTextExtent(str);
			py0 = y - (tsz.y / 2);
			dc.DrawText(str, px + 2, py0);
		}
		pos++;
	}

	y += (ll + margin);
	int row = 0;
	for(pos = 0; pos < datas.Count(); pos++) {
		int sts = datas.Item(pos);

		dc.SetPen(*wxBLACK_PEN);
		if ((x + sq.x + rpadding) > size.x) {
			y += (sq.y + margin);
			row++;
			x = lpadding;
		}
		if (x == lpadding) {
			if ((row % 4) == 0) {
				int px0 = x - (1 - ((row / 4) & 1)) * ll;
				int px1 = x + ll;
				dc.DrawLine(px0, y, px1, y);
				if ((row & 0xf) == 0) {
					wxString str = wxString::Format(wxT("%x"), (row >> 4));
					wxSize tsz = dc.GetTextExtent(str);
					px1 = px1 + margin - tsz.x;
					dc.DrawText(str, px1, y + 2);
				}
			}
			x += (ll + margin);
		}

		if (sts & 0x10000) {
			// selected (red)
			dc.SetBrush(brush_select);
		} else if (sts & 0x20000) {
			// selected (magenta)
			dc.SetBrush(brush_extra);
		} else {
			if (sts < FAT_AVAIL_NULLEND) {
				dc.SetPen(pens[sts]);
				dc.SetBrush(brushes[sts]);
			}
		}
		dc.DrawRectangle(x, y, sq.x, sq.y);
#if 0
		if (sts == FAT_AVAIL_USED_LAST) {
			dc.SetPen(*wxBLUE_PEN);
			dc.DrawLine(x + sq.x - 2, y, x + sq.x - 2, y + sq.y);
		}
#endif
		x += (sq.x + margin);
	}
	y += (sq.y + margin);

	SetVirtualSize(size.x, y);
}

void L3DiskFatAreaPanel::OnSize(wxSizeEvent& event)
{
	Refresh(true);
}

void L3DiskFatAreaPanel::SetData(wxUint32 offset, const wxArrayInt *arr)
{
	if (arr) {
		this->offset = offset;
		this->datas = *arr;
		Refresh(false);
	}
}

void L3DiskFatAreaPanel::ClearData()
{
	datas.Empty();
	Refresh(true);
}

void L3DiskFatAreaPanel::SetGroupBase(wxUint32 group_num, int highlight)
{
	wxUint32 pos = (group_num + offset);
	if (pos < (wxUint32)datas.Count()) {
		int val = datas.Item(pos);
		val |= highlight;
		datas.Item(pos) = val;
	}
}

void L3DiskFatAreaPanel::UnsetGroupBase(wxUint32 group_num)
{
	wxUint32 pos = (group_num + offset);
	if (pos < (wxUint32)datas.Count()) {
		int val = datas.Item(pos);
		val &= 0xffff;
		datas.Item(pos) = val;
	}
}

void L3DiskFatAreaPanel::ClearGroupBase()
{
	for(size_t pos = 0; pos < datas.Count(); pos++) {
		int val = datas.Item(pos);
		val &= 0xffff;
		datas.Item(pos) = val;
	}
}

void L3DiskFatAreaPanel::SetGroup(wxUint32 group_num)
{
	SetGroupBase(group_num, 0x10000);
	Refresh(false);
}

void L3DiskFatAreaPanel::SetGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums)
{
	for(size_t n = 0; n < group_items.Count(); n++) {
		SetGroupBase(group_items.Item(n).group, 0x10000);
	}
	for(size_t n = 0; n < extra_group_nums.Count(); n++) {
		SetGroupBase(extra_group_nums.Item(n), 0x20000);
	}
	Refresh(false);
}

void L3DiskFatAreaPanel::UnsetGroup(const DiskBasicGroups &group_items, const wxArrayInt &extra_group_nums)
{
	for(size_t n = 0; n < group_items.Count(); n++) {
		UnsetGroupBase(group_items.Item(n).group);
	}
	for(size_t n = 0; n < extra_group_nums.Count(); n++) {
		UnsetGroupBase(extra_group_nums.Item(n));
	}
	Refresh(false);
}

void L3DiskFatAreaPanel::ClearGroup()
{
	ClearGroupBase();
	Refresh(false);
}
