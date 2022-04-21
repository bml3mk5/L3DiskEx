/// @file uibindump.cpp
///
/// @brief バイナリダンプ
///

#include "uibindump.h"
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/numformatter.h>
#include "main.h"
#include "fontminibox.h"
#include "utils.h"

#define SCROLLBAR_UNIT	4

//
//
//
//static const int IDT_TOOLBAR = 501;
//static const long TOOLBAR_STYLE = wxTB_FLAT | wxTB_DOCKABLE | wxTB_TEXT;

// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskBinDumpFrame, wxFrame)
	EVT_MENU(wxID_CLOSE, L3DiskBinDumpFrame::OnClose)
	EVT_MENU(IDM_VIEW_INVERT, L3DiskBinDumpFrame::OnViewInvert)
	EVT_MENU_RANGE(IDM_VIEW_CHAR_ASCII, IDM_VIEW_CHAR_SJIS, L3DiskBinDumpFrame::OnViewChar)
	EVT_MENU_RANGE(IDM_VIEW_TEXT, IDM_VIEW_BINARY, L3DiskBinDumpFrame::OnViewTextBinary)
	EVT_MENU(IDM_VIEW_FONT, L3DiskBinDumpFrame::OnViewFont)
wxEND_EVENT_TABLE()

L3DiskBinDumpFrame::L3DiskBinDumpFrame(L3DiskFrame *parent, const wxString& title, const wxSize& size)
       : wxFrame(parent, -1, title, wxDefaultPosition, size, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT)
{
	// menu
	menuFile = new wxMenu;
	menuView = new wxMenu;

	menuFile->Append(wxID_CLOSE, _("&Close"));

	menuView->AppendRadioItem(IDM_VIEW_BINARY, _("Binary"));
	menuView->AppendRadioItem(IDM_VIEW_TEXT, _("Text"));
	menuView->AppendSeparator();
	menuView->AppendRadioItem(IDM_VIEW_CHAR_ASCII, _("Ascii"));
	menuView->AppendRadioItem(IDM_VIEW_CHAR_SJIS, _("Shift JIS"));
	menuView->AppendSeparator();
	menuView->AppendCheckItem(IDM_VIEW_INVERT, _("&Invert Datas"));
	menuView->AppendSeparator();
	menuView->AppendCheckItem(IDM_VIEW_FONT, _("&Font..."));

	// menu bar
	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append( menuFile, _("&File") );
	menuBar->Append( menuView, _("&View") );

	SetMenuBar( menuBar );

	// tool bar
//	RecreateToolbar();

	panel = new L3DiskBinDumpPanel(this, this);

	panel->SetClientSize(parent->GetClientSize());
}

L3DiskBinDumpFrame::~L3DiskBinDumpFrame()
{
	L3DiskFrame *parent = (L3DiskFrame *)m_parent;
	parent->BinDumpWindowClosed();
}

void L3DiskBinDumpFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
	Close();
}

void L3DiskBinDumpFrame::OnViewInvert(wxCommandEvent& event)
{
	int id = event.GetId();
	bool checked = event.IsChecked();
	ToggleControl(id, checked);
	panel->SetDataInvert(checked);
}

void L3DiskBinDumpFrame::OnViewChar(wxCommandEvent& event)
{
	int id = event.GetId();
	bool checked = event.IsChecked();
	ToggleControl(id, checked);
	panel->SetDataChar(id - IDM_VIEW_CHAR_ASCII);
}

void L3DiskBinDumpFrame::OnViewTextBinary(wxCommandEvent& event)
{
	int id = event.GetId();
	bool checked = event.IsChecked();
	ToggleControl(id, checked);
	panel->SetTextBinary(id - IDM_VIEW_TEXT);
}

void L3DiskBinDumpFrame::OnViewFont(wxCommandEvent& event)
{
	ShowDataFontDialog();
}

L3DiskBinDump *L3DiskBinDumpFrame::GetDumpPanel() const
{
	return panel->GetDumpPanel();
}

void L3DiskBinDumpFrame::SetDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	L3DiskBinDump *dump = GetDumpPanel();
	if (dump) dump->SetDatas(trk, sid, sec, buf, len);
}

void L3DiskBinDumpFrame::AppendDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	L3DiskBinDump *dump = GetDumpPanel();
	if (dump) dump->AppendDatas(trk, sid, sec, buf, len);
}

void L3DiskBinDumpFrame::ClearDatas()
{
	L3DiskBinDump *dump = GetDumpPanel();
	if (dump) dump->ClearDatas();
}

void L3DiskBinDumpFrame::SetTextBinary(int val)
{
	panel->SetTextBinary(val);
}

void L3DiskBinDumpFrame::SetDataInvert(bool val)
{
	ToggleControl(IDM_VIEW_INVERT, val);
	panel->SetDataInvert(val);
}

void L3DiskBinDumpFrame::SetDataChar(int val)
{
	ToggleControl(IDM_VIEW_CHAR_ASCII + val, true);
	panel->SetDataChar(val);
}

void L3DiskBinDumpFrame::SetDataFont(const wxFont &font)
{
	panel->SetDataFont(font);
	L3DiskFrame *parent = (L3DiskFrame *)m_parent;
	parent->SetDumpFont(font);
}

void L3DiskBinDumpFrame::GetDefaultDataFont(wxFont &font)
{
	L3DiskFrame *parent = (L3DiskFrame *)m_parent;
	wxString name = parent->GetDumpFontName();
	int size = parent->GetDumpFontSize();
	if (size == 0) size = 10;
	if (name.IsEmpty()) {
		font = wxFont(size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	} else {
		font = wxFont(size, wxFONTFAMILY_DEFAULT , wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, name);
	}
}

wxFont L3DiskBinDumpFrame::GetDefaultFont() const
{
	return wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
}

wxString L3DiskBinDumpFrame::GetDataFontName() const
{
	return panel->GetDataFontName();
}

int L3DiskBinDumpFrame::GetDataFontSize() const
{
	return panel->GetDataFontSize();
}

void L3DiskBinDumpFrame::ToggleControl(int id, bool checked)
{
	wxMenuBar *menubar = GetMenuBar();
	if (menubar) {
		menubar->Check(id, checked);
	}
}

void L3DiskBinDumpFrame::ShowDataFontDialog()
{
	FontMiniBox dlg(this, wxID_ANY, GetDefaultFont());
	dlg.SetFontName(GetDataFontName());
	dlg.SetFontSize(GetDataFontSize());
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		wxFont new_font(dlg.GetFontSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, dlg.GetFontName());
		SetDataFont(new_font);
	}
}

//
//
//
L3MemoryBuffer::L3MemoryBuffer(const L3MemoryBuffer &src)
	: wxMemoryBuffer(src)
{
	track_number = 0;
	side_number = 0;
	sector_number = 1;
}
L3MemoryBuffer::L3MemoryBuffer(size_t size)
	: wxMemoryBuffer(size)
{
	track_number = 0;
	side_number = 0;
	sector_number = 1;
}

//
//
//
L3DiskBinDumpPanel::L3DiskBinDumpPanel(L3DiskBinDumpFrame *parentframe, wxWindow *parentwindow)
                : wxSplitterWindow(parentwindow, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_THIN_SASH /* | wxSP_NO_XP_THEME */ )
{
	parent = parentwindow;
    frame = parentframe;

	// resize only bottom window when resize parent window.
	SetSashGravity(0.0);

	attr = new L3DiskBinDumpAttr(frame, this);
	dump = new L3DiskBinDump(frame, this);

	wxSize sz = attr->GetSize();

	SplitHorizontally(attr, dump, sz.y);
}

L3DiskBinDumpPanel::~L3DiskBinDumpPanel()
{
}

void L3DiskBinDumpPanel::SetTextBinary(int val)
{
	attr->SetTextBinary(val);
	dump->SetTextBinary(val);
}
void L3DiskBinDumpPanel::SetDataInvert(bool val)
{
	attr->SetDataInvert(val);
	dump->SetDataInvert(val);
}
void L3DiskBinDumpPanel::SetDataChar(int val)
{
	attr->SetDataChar(val);
	dump->SetDataChar(val);
}
void L3DiskBinDumpPanel::SetDataFont(const wxFont &font)
{
	dump->SetDataFont(font);
}
wxString L3DiskBinDumpPanel::GetDataFontName() const
{
	return dump->GetDataFontName();
}
int L3DiskBinDumpPanel::GetDataFontSize() const
{
	return dump->GetDataFontSize();
}

//
//
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskBinDumpAttr, wxPanel)
	EVT_RADIOBUTTON(IDC_RADIO_TEXT, L3DiskBinDumpAttr::OnCheckTextBinary)
	EVT_RADIOBUTTON(IDC_RADIO_BINARY, L3DiskBinDumpAttr::OnCheckTextBinary)
	EVT_RADIOBUTTON(IDC_RADIO_CHAR_ASCII, L3DiskBinDumpAttr::OnCheckChar)
	EVT_RADIOBUTTON(IDC_RADIO_CHAR_SJIS, L3DiskBinDumpAttr::OnCheckChar)
	EVT_CHECKBOX(IDC_CHECK_INVERT, L3DiskBinDumpAttr::OnCheckInvert)
	EVT_BUTTON(IDC_BUTTON_FONT, L3DiskBinDumpAttr::OnClickButton)
wxEND_EVENT_TABLE()

L3DiskBinDumpAttr::L3DiskBinDumpAttr(L3DiskBinDumpFrame *parentframe, wxWindow *parentwindow)
                : wxPanel(parentwindow)
{
	parent = parentwindow;
    frame = parentframe;

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 2);
	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrH = new wxBoxSizer(wxHORIZONTAL);

	radBinary = new wxRadioButton(this, IDC_RADIO_BINARY, _("Binary"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	szrH->Add(radBinary, flags);
	radText = new wxRadioButton(this, IDC_RADIO_TEXT, _("Text"));
	szrH->Add(radText, flags);

	radBinary->SetValue(true);

	szrH->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), flags);

	szrH->Add(new wxStaticText(this, wxID_ANY, _("Charactor Code")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL).Border(wxALL, 4));
	radCharAscii = new wxRadioButton(this, IDC_RADIO_CHAR_ASCII, _("Ascii"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	szrH->Add(radCharAscii, flags);
	radCharSJIS = new wxRadioButton(this, IDC_RADIO_CHAR_SJIS, _("Shift JIS"));
	szrH->Add(radCharSJIS, flags);
	
	radCharAscii->SetValue(true);

	szrH->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), flags);

	chkInvert = new wxCheckBox(this, IDC_CHECK_INVERT, _("Invert Datas")); 
	szrH->Add(chkInvert, flags);

	szrH->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), flags);

	btnFont = new wxButton(this, IDC_BUTTON_FONT, _("Font..."));
	szrH->Add(btnFont, flags);

	szrAll->Add(szrH);

	SetSizerAndFit(szrAll);
}

L3DiskBinDumpAttr::~L3DiskBinDumpAttr()
{
}

void L3DiskBinDumpAttr::OnCheckTextBinary(wxCommandEvent& event)
{
	frame->SetTextBinary(event.GetId() - IDC_RADIO_TEXT);
}
void L3DiskBinDumpAttr::OnCheckChar(wxCommandEvent& event)
{
	frame->SetDataChar(event.GetId() - IDC_RADIO_CHAR_ASCII);
}
void L3DiskBinDumpAttr::OnCheckInvert(wxCommandEvent& event)
{
	frame->SetDataInvert(event.IsChecked());
}
void L3DiskBinDumpAttr::OnClickButton(wxCommandEvent& event)
{
	frame->ShowDataFontDialog();
}

void L3DiskBinDumpAttr::SetTextBinary(int val)
{
	switch(val) {
	case 1:
		radBinary->SetValue(true);
		break;
	default:
		radText->SetValue(true);
		break;
	}
}
void L3DiskBinDumpAttr::SetDataInvert(bool val)
{
	chkInvert->SetValue(val);
}
void L3DiskBinDumpAttr::SetDataChar(int val)
{
	switch(val) {
	case 1:
		radCharSJIS->SetValue(true);
		break;
	default:
		radCharAscii->SetValue(true);
		break;
	}
}
//
//
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskBinDump, wxScrolledWindow)
	EVT_SIZE(L3DiskBinDump::OnSize)
wxEND_EVENT_TABLE()

L3DiskBinDump::L3DiskBinDump(L3DiskBinDumpFrame *parentframe, wxWindow *parentwindow)
       : wxScrolledWindow(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	parent   = parentwindow;
	frame    = parentframe;

	data_invert = false;
	data_char = 0;

	wxFont font;
	frame->GetDefaultDataFont(font);

	wxPoint pt(0, 0);
	wxSize sz;

	txtHex = new wxTextCtrl(this, IDC_TXT_HEX, wxT(""), pt, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxTE_NO_VSCROLL);
	txtHex->SetFont(font);
	sz = txtHex->GetTextExtent(wxString((char)'0', 60));
	sz.y *= 35;
	txtHex->SetSize(sz);
	txtHex->Bind(wxEVT_MOUSEWHEEL, &L3DiskBinDump::OnMouseWheelOnChild, this);
	pt.x += sz.x;

	txtAsc = new wxTextCtrl(this, IDC_TXT_ASC, wxT(""), pt, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxTE_NO_VSCROLL);
	txtAsc->SetFont(font);
	sz = txtAsc->GetTextExtent(wxString((char)'0', 30));
	sz.y *= 35;
	txtAsc->SetSize(sz);
	txtAsc->Bind(wxEVT_MOUSEWHEEL, &L3DiskBinDump::OnMouseWheelOnChild, this);
	pt.x += sz.x;

	pt.y += sz.y;

	min_x = pt.x;
	min_y = pt.y;

	txt_height = 0;
	text_binary = 1;

	parent->SetClientSize(min_x, min_y);
	SetScrollBarPos(min_x, min_y, 0, 0);
}

L3DiskBinDump::~L3DiskBinDump()
{
	ClearBuffer();
}

void L3DiskBinDump::ClearBuffer()
{
	for(size_t i=0; i<buffers.Count(); i++) {
		L3MemoryBuffer *p = buffers.Item(i);
		delete p;
	}
	buffers.Clear();
}

L3MemoryBuffer *L3DiskBinDump::AppendBuffer(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	L3MemoryBuffer *p = new L3MemoryBuffer(len);
	p->SetTrackNumber(trk);
	p->SetSideNumber(sid);
	p->SetSectorNumber(sec);
	p->AppendData(buf, len);
	buffers.Add(p);
	return p;
}

void L3DiskBinDump::OnSize(wxSizeEvent& event)
{
}

void L3DiskBinDump::OnMouseWheelOnChild(wxMouseEvent& event)
{
	wxPoint pt = GetViewStart();
	int sx, sy;
	GetScrollPixelsPerUnit(&sx, &sy);
#ifdef __WXOSX__
	Scroll(wxDefaultCoord, pt.y - (event.GetWheelRotation() / sy));
#else
	Scroll(wxDefaultCoord, pt.y - (event.GetWheelRotation() / sy));
#endif
}

void L3DiskBinDump::SetDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	ClearBuffer();
	SetDatasMain(AppendBuffer(trk, sid, sec, buf, len));
}

void L3DiskBinDump::SetDatasMain(const L3MemoryBuffer *buf)
{
	switch(text_binary) {
	case 1:
		SetDatasBinaryMain(buf, false);
		break;
	default:
		SetDatasTextMain(buf, false);
		break;
	}
}

void L3DiskBinDump::SetDatasBinaryMain(const L3MemoryBuffer *buf, bool append)
{
	wxString str, stra;
	wxSize sz, tz, cszH, cszA;

	Freeze();

	int rows = dump.Binary(buf->GetByteData(),buf->GetDataLen(),str,data_invert);
	dump.Ascii(buf->GetByteData(),buf->GetDataLen(),data_char,stra,data_invert);

	if (append) {
		txtHex->AppendText(wxT("\n"));
		txtAsc->AppendText(wxT("\n"));	
	} else {
		txtHex->Clear();
		txtAsc->Clear();
		txt_height = 0;
	}

	txtHex->AppendText(wxString::Format(wxT("# C:%d H:%d R:%d\n"),buf->GetTrackNumber(),buf->GetSideNumber(),buf->GetSectorNumber()));	
	txtHex->AppendText(str);
	txtAsc->AppendText(wxT("#\n"));
	txtAsc->AppendText(stra);

	rows++;

	txtHex->ShowPosition(0);
	txtAsc->ShowPosition(0);

	cszH = txtHex->GetSize();
	cszA = txtAsc->GetSize();

#ifdef __WXOSX__
	sz = txtAsc->GetTextExtent(str);
	int yy = txt_height + ((double)sz.y * 1.4) * rows;
#else
	wxPoint pz = txtAsc->PositionToCoords(txtAsc->GetLastPosition());
	sz = txtAsc->GetTextExtent(stra);
	tz = txtAsc->GetSizeFromTextSize(sz);
	int yy = pz.y + tz.y;
#endif

	txt_height = yy;

	if (min_y > yy) {
		yy = min_y;
	}

	cszH.y = yy;
	cszA.y = yy;

	txtHex->SetSize(cszH);
	txtAsc->SetSize(cszA);

	SetScrollBarPos(min_x, yy, -1, -1);

	Thaw();
}

void L3DiskBinDump::SetDatasTextMain(const L3MemoryBuffer *buf, bool append)
{
	wxString str;
	wxSize sz, tz, cszH;

	Freeze();

	int rows = dump.Text(buf->GetByteData(),buf->GetDataLen(),data_char,str,data_invert);

	if (append) {
		txtHex->AppendText(str);
	} else {
		txtHex->SetValue(str);
		txtAsc->Clear();
		txt_height = 0;
	}

	txtHex->ShowPosition(0);
	txtAsc->ShowPosition(0);

	cszH = txtHex->GetSize();

#ifdef __WXOSX__
	sz = txtHex->GetTextExtent(str);
	int yy = txt_height + ((double)sz.y * 1.4) * rows;
#else
	wxPoint pz = txtHex->PositionToCoords(txtHex->GetLastPosition());
	sz = txtHex->GetTextExtent(str);
	tz = txtHex->GetSizeFromTextSize(sz);
	int yy = pz.y + tz.y;
#endif

	txt_height = yy;

	if (min_y > yy) {
		yy = min_y;
	}

	cszH.y = yy;

	txtHex->SetSize(cszH);

	SetScrollBarPos(min_x, yy, -1, -1);

	Thaw();
}

void L3DiskBinDump::AppendDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	AppendDatasMain(AppendBuffer(trk, sid, sec, buf, len));
}

void L3DiskBinDump::AppendDatasMain(const L3MemoryBuffer *buf)
{
	switch(text_binary) {
	case 1:
		SetDatasBinaryMain(buf, true);
		break;
	default:
		SetDatasTextMain(buf, true);
		break;
	}
}

#if 0
void L3DiskBinDump::AppendDatasBinaryMain(const L3MemoryBuffer *buf)
{
	wxString str, stra;
	wxSize sz, tz, cszH, cszA;

	Freeze();

	int rows = dump.DumpBinary(buf->GetByteData(),buf->GetDataLen(),str,data_invert);
	dump.DumpAscii(buf->GetByteData(),buf->GetDataLen(),data_char,stra,data_invert);

	txtHex->AppendText(wxT("\n"));	
	txtHex->AppendText(wxString::Format(wxT("# C:%d H:%d R:%d\n"),buf->GetTrackNumber(),buf->GetSideNumber(),buf->GetSectorNumber()));	
	txtHex->AppendText(str);
	txtAsc->AppendText(wxT("\n"));	
	txtAsc->AppendText(wxT("#\n"));	
	txtAsc->AppendText(stra);

	rows++;

	txtHex->ShowPosition(0);
	txtAsc->ShowPosition(0);

	sz = txtAsc->GetTextExtent(str);
	tz = txtAsc->GetSizeFromTextSize(sz);
	cszH = txtHex->GetSize();
	cszA = txtAsc->GetSize();

#ifdef __WXOSX__
	int yy = txt_height + ((double)sz.y * 1.4) * rows;
#else
	int yy = txt_height + sz.y * rows + (tz.y - sz.y) * rows / 2;
#endif

	txt_height = yy;

	if (min_y > yy) {
		yy = min_y;
	}

	cszH.y = yy;
	cszA.y = yy;

	txtHex->SetSize(cszH);
	txtAsc->SetSize(cszA);

	SetScrollBarPos(min_x, yy, -1, -1);

	Thaw();
}
#endif

void L3DiskBinDump::RefreshData()
{
	for(size_t i=0; i<buffers.Count(); i++) {
		L3MemoryBuffer *p = buffers.Item(i);
		if (i == 0) {
			SetDatasMain(p);
		} else {
			AppendDatasMain(p);
		}
	}
}

void L3DiskBinDump::ClearDatas()
{
	ClearBuffer();

	txtHex->Clear();
	txtAsc->Clear();
}

void L3DiskBinDump::SetTextBinary(int val)
{
	if (text_binary != val) {
		wxSize cszH = txtHex->GetSize();
		wxSize cszA = txtAsc->GetSize();
		if (val == 0) {
			txtAsc->Show(false);
			cszH.SetWidth(cszH.GetWidth() + cszA.GetWidth());
			txtHex->SetSize(cszH);
		} else {
			cszH.SetWidth(cszH.GetWidth() - cszA.GetWidth());
			txtHex->SetSize(cszH);
			txtAsc->Show(true);
		}
		text_binary = val;
		RefreshData();
	}
}

void L3DiskBinDump::SetDataInvert(bool val)
{
	if (data_invert != val) {
		data_invert = val;
		RefreshData();
	}
}

void L3DiskBinDump::SetDataChar(int val)
{
	if (data_char != val) {
		data_char = val;
		RefreshData();
	}
}
void L3DiskBinDump::SetDataFont(const wxFont &font)
{
	wxPoint pt, pv;
	wxSize hsz, asz, sz;

	pv = GetViewStart();
	pv.x *= SCROLLBAR_UNIT;
	pv.y *= SCROLLBAR_UNIT;

	txtHex->SetFont(font);
	hsz = txtHex->GetSize();
	sz = txtHex->GetTextExtent(wxString((char)'0', 60));
	hsz.x = sz.x;

	txtAsc->SetFont(font);
	pt = txtAsc->GetPosition();
	pt.x = sz.x - pv.x;
	pt.y = -pv.y;
	txtAsc->SetPosition(pt);

	asz = txtAsc->GetSize();
	sz = txtAsc->GetTextExtent(wxString((char)'0', 30));
	asz.x = sz.x;

	min_x = hsz.x + asz.x;

	if (text_binary == 0) {
		// text
		hsz.x += asz.x;
	}

	txtHex->SetSize(hsz);
	txtAsc->SetSize(asz);

	SetScrollBarPos(min_x, asz.y, pv.x, pv.y);

	RefreshData();
}

wxString L3DiskBinDump::GetDataFontName() const
{
	wxFont font = txtHex->GetFont();
	return font.GetFaceName();
}
int L3DiskBinDump::GetDataFontSize() const
{
	wxFont font = txtHex->GetFont();
	return font.GetPointSize();
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
	if (new_px < 0) new_px = px;
	if (new_py < 0) new_py = py;
	if (ux != new_ux || uy != new_uy || px != new_px || py != new_py) {
		SetScrollbars(SCROLLBAR_UNIT, SCROLLBAR_UNIT
			, new_ux / SCROLLBAR_UNIT, new_uy / SCROLLBAR_UNIT
			, new_px / SCROLLBAR_UNIT, new_py / SCROLLBAR_UNIT, true);
	}
}
