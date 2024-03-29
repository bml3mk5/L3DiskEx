/// @file uibindump.cpp
///
/// @brief バイナリダンプ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uibindump.h"
#include <wx/statline.h>
#include <wx/radiobut.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/numformatter.h>
#include "mymenu.h"
#include "../main.h"
#include "uimainframe.h"
#include "fontminibox.h"
#include "../utils.h"
#include "../logging.h"


#define SCROLLBAR_UNIT	(4)

extern const char * fd_5inch_16_1_xpm[];

//
//
//
UiDiskBinDumpTextCtrl::UiDiskBinDumpTextCtrl(wxWindow *parent, wxWindowID id)
#ifdef USE_RICH_TEXT_ON_BINDUMP
	: wxRichTextCtrl(parent, id, wxT(""), wxDefaultPosition, wxDefaultSize, wxRE_MULTILINE | wxRE_READONLY)
#else
	: wxTextCtrl(parent, id, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxTE_NO_VSCROLL)
#endif
{
};

//
//
//
// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskBinDumpFrame, wxFrame)
	EVT_MENU(wxID_CLOSE, UiDiskBinDumpFrame::OnClose)
	EVT_MENU(IDM_VIEW_INVERT, UiDiskBinDumpFrame::OnViewInvert)
	EVT_MENU_RANGE(IDM_VIEW_CHAR_0, IDM_VIEW_CHAR_0 + 30, UiDiskBinDumpFrame::OnViewChar)
	EVT_MENU_RANGE(IDM_VIEW_TEXT, IDM_VIEW_BINARY, UiDiskBinDumpFrame::OnViewTextBinary)
	EVT_MENU(IDM_VIEW_FONT, UiDiskBinDumpFrame::OnViewFont)
wxEND_EVENT_TABLE()

UiDiskBinDumpFrame::UiDiskBinDumpFrame(UiDiskFrame *parent, const wxString& title, const wxSize& size)
       : wxFrame(parent, -1, title, wxDefaultPosition, size, wxDEFAULT_FRAME_STYLE | wxFRAME_FLOAT_ON_PARENT)
{
	// icon
#ifdef __WXMSW__
	SetIcon(wxIcon(_T("zz_fd_5inch")));
#elif defined(__WXGTK__) || defined(__WXMOTIF__)
	SetIcon(wxIcon(fd_5inch_16_1_xpm));
#endif

	// menu
	menuFile = new MyMenu;
	menuView = new MyMenu;

	menuFile->Append(wxID_CLOSE, _("&Close"));

	menuView->AppendRadioItem(IDM_VIEW_BINARY, _("Binary"));
	menuView->AppendRadioItem(IDM_VIEW_TEXT, _("Text"));
	menuView->AppendSeparator();
		MyMenu *sm = new MyMenu();
		const CharCodeChoice *choice = gCharCodeChoices.Find(wxT("dump"));
		if (choice) {
			for(size_t i=0; i<choice->Count(); i++) {
				const CharCodeMap *map = choice->Item(i);
				sm->AppendRadioItem( IDM_VIEW_CHAR_0 + (int)i, map->GetDescription() );
			}
		}
	menuView->AppendSubMenu(sm, _("&Charactor Code") );
	menuView->AppendSeparator();
	menuView->AppendCheckItem(IDM_VIEW_INVERT, _("&Invert Datas"));
	menuView->AppendSeparator();
	menuView->Append(IDM_VIEW_FONT, _("&Font..."));

	// menu bar
	MyMenuBar *menuBar = new MyMenuBar;
	menuBar->Append( menuFile, _("&File") );
	menuBar->Append( menuView, _("&View") );

	SetMenuBar( menuBar );

	// tool bar
//	RecreateToolbar();

	panel = new UiDiskBinDumpPanel(this, this);

	panel->SetClientSize(parent->GetClientSize());
}

UiDiskBinDumpFrame::~UiDiskBinDumpFrame()
{
	UiDiskFrame *parent = (UiDiskFrame *)m_parent;
	parent->BinDumpWindowClosed();
}

void UiDiskBinDumpFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
	Close();
}

void UiDiskBinDumpFrame::OnViewInvert(wxCommandEvent& event)
{
	int id = event.GetId();
	bool checked = event.IsChecked();
	ToggleControl(id, checked);
	panel->SetDataInvert(checked);
}

void UiDiskBinDumpFrame::OnViewChar(wxCommandEvent& event)
{
	int id = event.GetId() - IDM_VIEW_CHAR_0;
	bool checked = event.IsChecked();
	ToggleControl(id, checked);

	wxString name = gCharCodeChoices.GetItemName(wxT("dump"), (size_t)id);
	panel->SetDataChar(name);
}

void UiDiskBinDumpFrame::OnViewTextBinary(wxCommandEvent& event)
{
	int id = event.GetId();
	bool checked = event.IsChecked();
	ToggleControl(id, checked);
	panel->SetTextBinary(id - IDM_VIEW_TEXT);
}

void UiDiskBinDumpFrame::OnViewFont(wxCommandEvent& event)
{
	ShowDataFontDialog();
}

UiDiskBinDump *UiDiskBinDumpFrame::GetDumpPanel() const
{
	return panel->GetDumpPanel();
}

void UiDiskBinDumpFrame::SetDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	UiDiskBinDump *dump = GetDumpPanel();
	if (dump) dump->SetDatas(trk, sid, sec, buf, len);
}

void UiDiskBinDumpFrame::AppendDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	UiDiskBinDump *dump = GetDumpPanel();
	if (dump) dump->AppendDatas(trk, sid, sec, buf, len);
}

void UiDiskBinDumpFrame::ClearDatas()
{
	UiDiskBinDump *dump = GetDumpPanel();
	if (dump) dump->ClearDatas();
}

void UiDiskBinDumpFrame::SetTextBinary(int val)
{
	panel->SetTextBinary(val);
}

void UiDiskBinDumpFrame::SetDataInvert(bool val)
{
	ToggleControl(IDM_VIEW_INVERT, val);
	panel->SetDataInvert(val);
}

void UiDiskBinDumpFrame::SetDataChar(int sel)
{
	ToggleControl(IDM_VIEW_CHAR_0 + sel, true);

	wxString name = gCharCodeChoices.GetItemName(wxT("dump"), (size_t)sel);
	panel->SetDataChar(name);
}

void UiDiskBinDumpFrame::SetDataFont(const wxFont &font)
{
	panel->SetDataFont(font);
	UiDiskFrame *parent = (UiDiskFrame *)GetParent();
	parent->SetIniDumpFont(font);
}

void UiDiskBinDumpFrame::GetDefaultDataFont(wxFont &font)
{
	UiDiskFrame *parent = (UiDiskFrame *)m_parent;
	wxString name = parent->GetIniDumpFontName();
	int size = parent->GetIniDumpFontSize();
	if (size == 0) size = 10;
	if (name.IsEmpty()) {
		font = wxFont(size, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	} else {
		font = wxFont(size, wxFONTFAMILY_DEFAULT , wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, name);
	}
}

wxFont UiDiskBinDumpFrame::GetDefaultFont() const
{
	return wxFont(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
}

wxString UiDiskBinDumpFrame::GetDataFontName() const
{
	return panel->GetDataFontName();
}

int UiDiskBinDumpFrame::GetDataFontSize() const
{
	return panel->GetDataFontSize();
}

void UiDiskBinDumpFrame::ToggleControl(int id, bool checked)
{
	wxMenuBar *menubar = GetMenuBar();
	if (menubar) {
		menubar->Check(id, checked);
	}
}

void UiDiskBinDumpFrame::ShowDataFontDialog()
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
MyMemoryBuffer::MyMemoryBuffer(const MyMemoryBuffer &src)
	: wxMemoryBuffer(src)
{
	track_number = 0;
	side_number = 0;
	sector_number = 1;
}
MyMemoryBuffer::MyMemoryBuffer(size_t size)
	: wxMemoryBuffer(size)
{
	track_number = 0;
	side_number = 0;
	sector_number = 1;
}

//
//
//
UiDiskBinDumpPanel::UiDiskBinDumpPanel(UiDiskBinDumpFrame *parentframe, wxWindow *parentwindow)
                : wxSplitterWindow(parentwindow, wxID_ANY,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_THIN_SASH /* | wxSP_NO_XP_THEME */ )
{
	parent = parentwindow;
    frame = parentframe;

	// resize only bottom window when resize parent window.
	SetSashGravity(0.0);

	attr = new UiDiskBinDumpAttr(frame, this);
	dump = new UiDiskBinDump(frame, this);

	wxSize sz = attr->GetSize();

	SplitHorizontally(attr, dump, sz.y);
}

UiDiskBinDumpPanel::~UiDiskBinDumpPanel()
{
}

void UiDiskBinDumpPanel::SetTextBinary(int val)
{
	attr->SetTextBinary(val);
	dump->SetTextBinary(val);
}
void UiDiskBinDumpPanel::SetDataInvert(bool val)
{
	attr->SetDataInvert(val);
	dump->SetDataInvert(val);
}
void UiDiskBinDumpPanel::SetDataChar(const wxString &name)
{
	attr->SetDataChar(name);
	dump->SetDataChar(name);
}
void UiDiskBinDumpPanel::SetDataFont(const wxFont &font)
{
	dump->SetDataFont(font);
}
wxString UiDiskBinDumpPanel::GetDataFontName() const
{
	return dump->GetDataFontName();
}
int UiDiskBinDumpPanel::GetDataFontSize() const
{
	return dump->GetDataFontSize();
}

//
//
//
// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskBinDumpAttr, wxPanel)
	EVT_RADIOBUTTON(IDC_RADIO_TEXT, UiDiskBinDumpAttr::OnCheckTextBinary)
	EVT_RADIOBUTTON(IDC_RADIO_BINARY, UiDiskBinDumpAttr::OnCheckTextBinary)
	EVT_CHOICE(IDC_COMBO_CHAR_CODE, UiDiskBinDumpAttr::OnCheckChar)
	EVT_CHECKBOX(IDC_CHECK_INVERT, UiDiskBinDumpAttr::OnCheckInvert)
	EVT_BUTTON(IDC_BUTTON_FONT, UiDiskBinDumpAttr::OnClickButton)
wxEND_EVENT_TABLE()

UiDiskBinDumpAttr::UiDiskBinDumpAttr(UiDiskBinDumpFrame *parentframe, wxWindow *parentwindow)
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
	comCharCode = new wxChoice(this, IDC_COMBO_CHAR_CODE, wxDefaultPosition, wxDefaultSize);
	const CharCodeChoice *choice = gCharCodeChoices.Find(wxT("dump"));
	if (choice) {
		for(size_t i=0; i<choice->Count(); i++) {
			const CharCodeMap *map = choice->Item(i);
			comCharCode->Append( map->GetDescription() );
		}
	}
	comCharCode->SetSelection(0);
	szrH->Add(comCharCode, flags);

	szrH->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), flags);

	chkInvert = new wxCheckBox(this, IDC_CHECK_INVERT, _("Invert Datas")); 
	szrH->Add(chkInvert, flags);

#if 0
	szrH->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), flags);

	btnFont = new wxButton(this, IDC_BUTTON_FONT, _("Font..."));
	szrH->Add(btnFont, flags);
#endif

	szrAll->Add(szrH);

	SetSizerAndFit(szrAll);
}

UiDiskBinDumpAttr::~UiDiskBinDumpAttr()
{
}

void UiDiskBinDumpAttr::OnCheckTextBinary(wxCommandEvent& event)
{
	frame->SetTextBinary(event.GetId() - IDC_RADIO_TEXT);
}
void UiDiskBinDumpAttr::OnCheckChar(wxCommandEvent& event)
{
	int sel = event.GetSelection();
	frame->SetDataChar(sel);
}
void UiDiskBinDumpAttr::OnCheckInvert(wxCommandEvent& event)
{
	frame->SetDataInvert(event.IsChecked());
}
void UiDiskBinDumpAttr::OnClickButton(wxCommandEvent& event)
{
	frame->ShowDataFontDialog();
}

void UiDiskBinDumpAttr::SetTextBinary(int val)
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
void UiDiskBinDumpAttr::SetDataInvert(bool val)
{
	chkInvert->SetValue(val);
}
void UiDiskBinDumpAttr::SetDataChar(const wxString &name)
{
	int sel = gCharCodeChoices.IndexOf(wxT("dump"), name);
	comCharCode->SetSelection(sel);
}

//
//
//
// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskBinDump, wxScrolledWindow)
	EVT_SIZE(UiDiskBinDump::OnSize)
wxEND_EVENT_TABLE()

UiDiskBinDump::UiDiskBinDump(UiDiskBinDumpFrame *parentframe, wxWindow *parentwindow)
       : wxScrolledWindow(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	parent   = parentwindow;
	frame    = parentframe;

	data_invert = false;
	data_char = gCharCodeChoices.GetItemName(wxT("dump"), 0);

	wxFont font;
	frame->GetDefaultDataFont(font);

	txtHex = new UiDiskBinDumpTextCtrl(this, IDC_TXT_HEX);
	txtHex->SetFont(font);
	txtHex->Bind(wxEVT_MOUSEWHEEL, &UiDiskBinDump::OnMouseWheelOnChild, this);

	txtAsc = new UiDiskBinDumpTextCtrl(this, IDC_TXT_ASC);
	txtAsc->SetFont(font);
	txtAsc->Bind(wxEVT_MOUSEWHEEL, &UiDiskBinDump::OnMouseWheelOnChild, this);

	txt_height = 0;
	text_binary = 1;

	CalcWidthOnTextCtrl(wxPoint(0, 0), 35);

	parent->SetClientSize(min_x, min_y);
}

UiDiskBinDump::~UiDiskBinDump()
{
	ClearBuffer();
}

void UiDiskBinDump::OnSize(wxSizeEvent& event)
{
}

void UiDiskBinDump::OnMouseWheelOnChild(wxMouseEvent& event)
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

void UiDiskBinDump::ClearBuffer()
{
	for(size_t i=0; i<buffers.Count(); i++) {
		MyMemoryBuffer *p = buffers.Item(i);
		delete p;
	}
	buffers.Clear();
}

MyMemoryBuffer *UiDiskBinDump::AppendBuffer(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	MyMemoryBuffer *p = new MyMemoryBuffer(len);
	p->SetTrackNumber(trk);
	p->SetSideNumber(sid);
	p->SetSectorNumber(sec);
	p->AppendData(buf, len);
	buffers.Add(p);
	return p;
}

/// ダンプにデータを表示
void UiDiskBinDump::SetDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	ClearBuffer();
	SetDatasMain(AppendBuffer(trk, sid, sec, buf, len));
}

/// ダンプにデータを表示
void UiDiskBinDump::SetDatasMain(const MyMemoryBuffer *buf)
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

/// ダンプにデータを表示（追記）
void UiDiskBinDump::AppendDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len)
{
	AppendDatasMain(AppendBuffer(trk, sid, sec, buf, len));
}

/// ダンプにデータを表示（追記）
void UiDiskBinDump::AppendDatasMain(const MyMemoryBuffer *buf)
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

/// バイナリダンプ時のデータ設定
void UiDiskBinDump::SetDatasBinaryMain(const MyMemoryBuffer *buf, bool append)
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

	int yy = CalcHeightOnTextCtrl(txtAsc, rows, stra);

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

/// テキストダンプ時のデータ設定
void UiDiskBinDump::SetDatasTextMain(const MyMemoryBuffer *buf, bool append)
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

	int yy = CalcHeightOnTextCtrl(txtHex, rows, str);

	txt_height = yy;

	if (min_y > yy) {
		yy = min_y;
	}

	cszH.y = yy;

	txtHex->SetSize(cszH);

	SetScrollBarPos(min_x, yy, -1, -1);

	Thaw();
}

/// テキストコントロールの幅を計算して設定する
/// @param [in] pv       ビューポイント
/// @param [in] def_rows 初期行数
void UiDiskBinDump::CalcWidthOnTextCtrl(const wxPoint &pv, int def_rows)
{
	wxPoint pt;
	wxSize hsz, asz, htz, atz;

	htz = txtHex->GetTextExtent(wxString((char)'0', 64));
	if (def_rows >= 0) {
		txtHex->SetPosition(wxPoint(0, 0));
		hsz.x = htz.x;
		hsz.y = htz.y * def_rows;
	} else {
		hsz = txtHex->GetSize();
		hsz.x = htz.x;
	}

	atz = txtAsc->GetTextExtent(wxString((char)'0', 30));
	if (def_rows >= 0) {
		asz.x = atz.x;
		asz.y = atz.y * def_rows;
	} else {
		asz = txtAsc->GetSize();
		asz.x = atz.x;
	}

	if (def_rows >= 0) {
		pt.x = hsz.x;
		pt.y = 0;
	} else {
//		pt = txtAsc->GetPosition();
		pt.x = htz.x - pv.x;
		pt.y = -pv.y;
	}
	txtAsc->SetPosition(pt);

	min_x = hsz.x + asz.x;
	if (def_rows >= 0) {
		min_y = asz.y;
	}

	if (text_binary == 0) {
		// text
		hsz.x += asz.x;
	}

	txtHex->SetSize(hsz);
	txtAsc->SetSize(asz);

	SetScrollBarPos(min_x, asz.y, pv.x, pv.y);
}

/// テキストコントロールの高さを計算し設定する
/// @param [in] ctrl テキストコントロール
/// @param [in] rows 行数
/// @param [in] str  文字列
int UiDiskBinDump::CalcHeightOnTextCtrl(const UiDiskBinDumpTextCtrl *ctrl, int rows, const wxString &str)
{
	int yy = 0;
	wxSize sz;

#if defined(__WXOSX__)
	sz = ctrl->GetTextExtent(str);
	yy = txt_height + ((double)sz.y * 1.4) * rows;

#elif defined(__WXGTK__)
	sz = ctrl->GetTextExtent(wxT("#\n") + str);
	wxSize tz = ctrl->GetSizeFromTextSize(sz);
	yy = txt_height + tz.y;

//	myLog.SetInfo("sz.y:%d tz.y:%d", sz.y, tz.y);
#else
	wxTextPos last_pos = ctrl->GetLastPosition();
#ifdef USE_RICH_TEXT_ON_BINDUMP
	long pz_col = 0;
	long pz_row = 0;
	ctrl->PositionToXY(last_pos, &pz_col, &pz_row);
	sz = ctrl->GetTextExtent(str);
	wxSize tz = ctrl->GetSizeFromTextSize(sz);
	yy = (int)(pz_row * (tz.y >= 0 ? tz.y : (sz.y + 8)));
#else
	wxPoint pz = ctrl->PositionToCoords(last_pos);
	sz = ctrl->GetTextExtent(str);
	wxSize tz = ctrl->GetSizeFromTextSize(sz);
	yy = pz.y + tz.y;

//	myLog.SetInfo("pz.y:%d sz.y:%d tz.y:%d", pz.y, sz.y, tz.y);
#endif
#endif
	return yy;
}

void UiDiskBinDump::RefreshData()
{
	for(size_t i=0; i<buffers.Count(); i++) {
		MyMemoryBuffer *p = buffers.Item(i);
		if (i == 0) {
			SetDatasMain(p);
		} else {
			AppendDatasMain(p);
		}
	}
}

void UiDiskBinDump::ClearDatas()
{
	ClearBuffer();

	txtHex->Clear();
	txtAsc->Clear();
}

void UiDiskBinDump::SetTextBinary(int val)
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

void UiDiskBinDump::SetDataInvert(bool val)
{
	if (data_invert != val) {
		data_invert = val;
		RefreshData();
	}
}

void UiDiskBinDump::SetDataChar(const wxString &name)
{
	if (data_char != name) {
		data_char = name;
		RefreshData();
	}
}

/// フォントを設定
void UiDiskBinDump::SetDataFont(const wxFont &font)
{
	wxPoint pv;

	pv = GetViewStart();
	pv.x *= SCROLLBAR_UNIT;
	pv.y *= SCROLLBAR_UNIT;

	txtHex->SetFont(font);
	txtAsc->SetFont(font);

	CalcWidthOnTextCtrl(pv, -1);

	RefreshData();
}

wxString UiDiskBinDump::GetDataFontName() const
{
	wxFont font = txtHex->GetFont();
	return font.GetFaceName();
}
int UiDiskBinDump::GetDataFontSize() const
{
	wxFont font = txtHex->GetFont();
	return font.GetPointSize();
}

// スクロールバーを設定
void UiDiskBinDump::SetScrollBarPos(int new_ux, int new_uy, int new_px, int new_py)
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
