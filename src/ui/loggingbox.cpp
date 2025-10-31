/// @file loggingbox.cpp
///
/// @brief ロギングダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "loggingbox.h"
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include "uimainframe.h"
#include "../logging.h"

// Attach Event
BEGIN_EVENT_TABLE(LoggingBox, wxDialog)
	EVT_SIZE(LoggingBox::OnSize)
	EVT_CLOSE(LoggingBox::OnClose)
	EVT_BUTTON(wxID_CLOSE, LoggingBox::OnButtonClose)
	EVT_BUTTON(IDC_BUTTON_UPDATE, LoggingBox::OnButtonUpdate)
END_EVENT_TABLE()

LoggingBox::LoggingBox(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, _("Log"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER | wxMAXIMIZE_BOX | wxMINIMIZE_BOX | wxDIALOG_NO_PARENT)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *szr = new wxBoxSizer(wxHORIZONTAL);
	wxButton *btn = new wxButton(this, IDC_BUTTON_UPDATE, _("Update"));
	szr->Add(btn);
	szr->AddStretchSpacer();
	btn = new wxButton(this, wxID_CLOSE, _("Close"));
	szr->Add(btn);
//	wxSizer *szrButtons = CreateButtonSizer(wxCLOSE);
//	szr->Add(szrButtons, wxSizerFlags().Align(wxALIGN_RIGHT));
	szriHed = szrAll->Add(szr, flags);

	wxSize sz(320,200);
	txtLogging = new wxTextCtrl(this, IDC_TEXT_LOGGING, wxEmptyString, wxDefaultPosition, sz, wxTE_MULTILINE | wxTE_READONLY);
	szriTxt = szrAll->Add(txtLogging, flags);

	SetSizerAndFit(szrAll);
}

LoggingBox::~LoggingBox()
{
	UiDiskFrame *parent = (UiDiskFrame *)m_parent;
	parent->LoggingWindowClosed();
}

/// リサイズ
void LoggingBox::OnSize(wxSizeEvent& event)
{
	if (!GetSizer()) {
		event.Skip();
		return;
	}

	wxSize szCli = GetClientSize();

	wxSize szHed = szriHed->CalcMin();
	wxSize szTxt = szriTxt->CalcMin();

	// コントロールの幅
	szHed.SetWidth(szCli.GetWidth());
	szTxt.SetWidth(szCli.GetWidth());

	// テキストボックスの高さを求める
	wxPoint ptTxt(0, szHed.GetHeight());
	szTxt.SetHeight(szCli.GetHeight() - szHed.GetHeight());

	// サイズの変更
	wxPoint pt;
	szriHed->SetDimension(pt, szHed);
	szriTxt->SetDimension(ptTxt, szTxt);
}

/// ウィンドウ閉じる
void LoggingBox::OnClose(wxCloseEvent& WXUNUSED(event))
{
	Destroy();
}

/// 閉じるボタン押下
void LoggingBox::OnButtonClose(wxCommandEvent& WXUNUSED(event))
{
	Destroy();
}

/// 更新ボタン押下
void LoggingBox::OnButtonUpdate(wxCommandEvent& WXUNUSED(event))
{
	if (!txtLogging) return;

	wxString text;
	myLog.GetLog(text);
	txtLogging->SetValue(text);
	txtLogging->ShowPosition(txtLogging->GetLastPosition());
}

