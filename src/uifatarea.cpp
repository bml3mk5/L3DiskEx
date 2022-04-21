/// @file uifatarea.cpp
///
/// @brief FATの使用状況を表示
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

void L3DiskFatAreaFrame::SetGroup(const DiskBasicGroups *group_items, wxUint32 extra_group_num)
{
	panel->SetGroup(group_items, extra_group_num);
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
	: wxScrolled<wxPanel>(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL)
//	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
    frame = parent;
	offset = 0;

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

	ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_ALWAYS);
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
		dc.DrawLine(px, y - (pos & 1) * ll, px, y + ll);
		pos++;
	}

	y += (ll + margin);
	int row = 0;
	for(pos = 0; pos < datas.Count(); pos++) {
		if ((x + sq.x + rpadding) > size.x) {
			y += (sq.y + margin);
			row++;
			x = lpadding;
		}
		if (x == lpadding) {
			if ((row % 4) == 0) {
				dc.DrawLine(x - (1 - ((row / 4) & 1)) * ll, y, x + ll, y);
			}
			x += (ll + margin);
		}
		int sts = datas.Item(pos);
		switch(sts & 0xffff0000) {
		case 0x10000:
			dc.SetBrush(*wxRED_BRUSH);
			break;
		case 0x20000:
			// magenta
			dc.SetBrush(wxBrush(wxColor(0xff, 0, 0xff)));
			break;
		default:
			switch(sts) {
			case FAT_AVAIL_USED:
				dc.SetBrush(*wxCYAN_BRUSH);
				break;
			case FAT_AVAIL_USED_LAST:
				dc.SetBrush(*wxBLUE_BRUSH);
				break;
			case FAT_AVAIL_SYSTEM:
				dc.SetBrush(*wxGREY_BRUSH);
				break;
			case FAT_AVAIL_FREE:
				dc.SetBrush(*wxWHITE_BRUSH);
				break;
			}
		}
		dc.DrawRectangle(x, y, sq.x, sq.y);
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
	ClearGroupBase();
	SetGroupBase(group_num, 0x10000);
	Refresh(false);
}

void L3DiskFatAreaPanel::SetGroup(const DiskBasicGroups *group_items, wxUint32 extra_group_num)
{
	ClearGroupBase();
	for(size_t n = 0; n < group_items->Count(); n++) {
		SetGroupBase(group_items->Item(n).group, 0x10000);
	}
	SetGroupBase(extra_group_num, 0x20000);
	Refresh(false);
}

void L3DiskFatAreaPanel::ClearGroup()
{
	ClearGroupBase();
	Refresh(false);
}
