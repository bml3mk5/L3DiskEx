/// @file uibindump.cpp
///
/// @brief バイナリダンプ
///

#include "main.h"
#include "uibindump.h"
#include "charcodes.h"
#include "utils.h"

#define SCROLLBAR_UNIT	2

// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskBinDump, wxScrolledWindow)
//	EVT_SIZE(L3DiskBinDump::OnSize)
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

	SetScrollBarPos(pt.x, pt.y, 0, 0);
}

//void L3DiskBinDump::OnSize(wxSizeEvent& event)
//{
//}

void L3DiskBinDump::SetDatas(const wxUint8 *buf, size_t len)
{
	txtHex->SetValue(L3DiskUtils::DumpBinary(buf,len));	
	txtAsc->SetValue(DumpAscii(buf,len));	
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

wxString L3DiskBinDump::DumpAscii(const wxUint8 *buffer, size_t bufsize)
{
	wxString str;
	gCharCodes.SetMap(wxT("hankaku"));
	for(size_t pos = 0, col = 0; pos < bufsize; pos++, col++) {
		if (col >= 16) {
			str += wxT("\n");
			col = 0;
		}
		wxUint8 c = buffer[pos];
		wxString cstr;
		gCharCodes.FindString(c, cstr, wxT("."));
		str += cstr;
	}
	return str;
}
