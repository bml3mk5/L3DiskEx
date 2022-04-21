/// @file uibindump.cpp
///
/// @brief バイナリダンプ
///

#include "main.h"
#include "uibindump.h"
#include "utils.h"

#define SCROLLBAR_UNIT	2

// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskBinDump, wxScrolledWindow)
	EVT_SIZE(L3DiskBinDump::OnSize)
wxEND_EVENT_TABLE()

L3DiskBinDump::L3DiskBinDump(L3DiskFrame *parentframe, wxWindow *parentwindow)
       : wxScrolledWindow(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	parent   = parentwindow;
	frame    = parentframe;

	wxFont fontFixed(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);

	wxPoint pt(0, 0);
	wxSize  sz(320, 300);

	txtHex = new wxTextCtrl(this, IDC_TXT_HEX, wxT(""), pt, sz, wxTE_MULTILINE | wxTE_READONLY | wxTE_NO_VSCROLL);
	txtHex->SetFont(fontFixed);
	sz = txtHex->GetTextExtent(wxString((char)'0', 60));
	sz.y *= 35;
	txtHex->SetSize(sz);

	pt.x += sz.x;
	sz.x = 160;
	txtAsc = new wxTextCtrl(this, IDC_TXT_ASC, wxT(""), pt, sz, wxTE_MULTILINE | wxTE_READONLY | wxTE_NO_VSCROLL);
	txtAsc->SetFont(fontFixed);
	sz = txtHex->GetTextExtent(wxString((char)'0', 30));
	sz.y *= 35;
	txtAsc->SetSize(sz);

	pt.x += sz.x;
	pt.y += sz.y;

	min_x = pt.x;
	min_y = pt.y;

	SetScrollBarPos(pt.x, pt.y, 0, 0);
}

void L3DiskBinDump::OnSize(wxSizeEvent& event)
{
	wxSize sz, cszH, cszA;
	wxPoint pt = GetViewStart();
	int sx, sy;
	GetScrollPixelsPerUnit(&sx, &sy);
	sz = GetClientSize();
	cszH = txtHex->GetSize();
	cszA = txtAsc->GetSize();

	int yy = pt.y * sy + sz.y;

	if (yy > min_y) {
		if (pt.y > 0) {
			Scroll(0, (min_y - sz.y) / sy);
		} else {
			cszH.y = sz.y;
			cszA.y = sz.y;
		}
	} else {
		cszH.y = min_y;
		cszA.y = min_y;
	}

	txtHex->SetSize(cszH);
	txtAsc->SetSize(cszA);
}

void L3DiskBinDump::SetDatas(const wxUint8 *buf, size_t len)
{
	wxString str;
	wxSize sz, cszH, cszA;
	int rows = L3DiskUtils::DumpBinary(buf,len,str);
	rows+=2;

	txtHex->SetValue(str);	
	txtAsc->SetValue(L3DiskUtils::DumpAscii(buf,len));

	sz = txtHex->GetTextExtent(str);
	cszH = txtHex->GetSize();
	cszA = txtAsc->GetSize();

	if (min_y < sz.y * rows) {
		min_y = sz.y * rows;

		cszH.y = min_y;
		cszA.y = min_y;

		txtHex->SetSize(cszH);
		txtAsc->SetSize(cszA);

		SetScrollBarPos(min_x, min_y, 0, 0);
	}
}

void L3DiskBinDump::AppendDatas(const wxUint8 *buf, size_t len)
{
	wxString str;
	wxSize sz, cszH, cszA;
	int rows = L3DiskUtils::DumpBinary(buf,len,str);
	rows+=2;

	txtHex->AppendText(str);	
	txtAsc->AppendText(L3DiskUtils::DumpAscii(buf,len));

	sz = txtHex->GetTextExtent(str);
	cszH = txtHex->GetSize();
	cszA = txtAsc->GetSize();

	if (min_y < sz.y * rows) {
		min_y = sz.y * rows;

		cszH.y = min_y;
		cszA.y = min_y;

		txtHex->SetSize(cszH);
		txtAsc->SetSize(cszA);

		SetScrollBarPos(min_x, min_y, 0, 0);
	}
}

void L3DiskBinDump::ClearDatas()
{
	txtHex->Clear();
	txtAsc->Clear();
}

// スクロールバーを設定
void L3DiskBinDump::SetScrollBarPos(int new_ux, int new_uy, int new_px, int new_py)
{
	int ux, uy, px, py, sx, sy;
	GetVirtualSize(&ux, &uy);
	GetViewStart(&px, &py);
	px *= SCROLLBAR_UNIT;
	py *= SCROLLBAR_UNIT;
	GetClientSize(&sx, &sy);
	if (new_ux < sx) new_ux = sx;
	if (new_uy < sy) new_uy = sy;
	if (ux != new_ux || uy != new_uy || px != new_px || py != new_py) {
		SetScrollbars(SCROLLBAR_UNIT, SCROLLBAR_UNIT
			, new_ux / SCROLLBAR_UNIT, new_uy / SCROLLBAR_UNIT
			, new_px / SCROLLBAR_UNIT, new_py / SCROLLBAR_UNIT, true);
	}
}
