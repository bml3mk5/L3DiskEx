/// @file uibindump.h
///
/// @brief バイナリダンプ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UIBINDUMP_H_
#define _UIBINDUMP_H_

#include "common.h"
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include <wx/splitter.h>
#include "utils.h"


#undef USE_RICH_TEXT_ON_BINDUMP

#ifdef USE_RICH_TEXT_ON_BINDUMP
#include <wx/richtext/richtextctrl.h>
#else
#include <wx/textctrl.h>
#endif

/// テキストコントロール
class L3DiskBinDumpTextCtrl
#ifdef USE_RICH_TEXT_ON_BINDUMP
	: public wxRichTextCtrl
#else
	: public wxTextCtrl
#endif
{
public:
	L3DiskBinDumpTextCtrl(wxWindow *parent, wxWindowID id); 
};

class wxMenu;
class wxCheckBox;
class wxRadioButton;
class wxButton;

class L3DiskFrame;
class L3DiskBinDump;
class L3DiskBinDumpAttr;
class L3DiskBinDumpPanel;

/// バイナリダンプウィンドウ
class L3DiskBinDumpFrame: public wxFrame
{
private:
	// gui
	wxMenu *menuFile;
	wxMenu *menuView;

	L3DiskBinDumpPanel *panel;

	enum en_menu_id
	{
		IDM_VIEW_INVERT = 1,
		IDM_VIEW_TEXT,
		IDM_VIEW_BINARY,
		IDM_VIEW_CHAR_ASCII,
		IDM_VIEW_CHAR_SJIS,
		IDM_VIEW_FONT,
	};

public:
    L3DiskBinDumpFrame(L3DiskFrame *parent, const wxString& title, const wxSize& size);
	~L3DiskBinDumpFrame();

	void OnClose(wxCommandEvent& event);
	void OnViewInvert(wxCommandEvent& event);
	void OnViewChar(wxCommandEvent& event);
	void OnViewTextBinary(wxCommandEvent& event);
	void OnViewFont(wxCommandEvent& event);

	L3DiskBinDumpPanel *GetPanel() const { return panel; }
	L3DiskBinDump *GetDumpPanel() const;

	void SetDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	void AppendDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	void ClearDatas();

	void SetTextBinary(int val);
	void SetDataInvert(bool val);
	void SetDataChar(int val);
	void SetDataFont(const wxFont &font);
	void GetDefaultDataFont(wxFont &font);
	wxFont GetDefaultFont() const;
	wxString GetDataFontName() const;
	int GetDataFontSize() const;

	void ShowDataFontDialog();

	void ToggleControl(int id, bool checked);

	wxDECLARE_EVENT_TABLE();
};

/// データバッファ
class L3MemoryBuffer : public wxMemoryBuffer
{
private:
	int track_number;
	int side_number;
	int sector_number;

public:
	L3MemoryBuffer(const L3MemoryBuffer &src);
	L3MemoryBuffer(size_t size);

	const wxUint8 *GetByteData() const { return (const wxUint8 *)GetData(); }

	int GetTrackNumber() const { return track_number; }
	int GetSideNumber() const { return side_number; }
	int GetSectorNumber() const { return sector_number; }
	void SetTrackNumber(int val) { track_number = val; }
	void SetSideNumber(int val) { side_number = val; }
	void SetSectorNumber(int val) { sector_number = val; }
};

WX_DEFINE_ARRAY(L3MemoryBuffer *, L3MemoryBuffers);

/// バイナリダンプパネル
class L3DiskBinDumpPanel : public wxSplitterWindow
{
private:
	wxWindow *parent;
	L3DiskBinDumpFrame *frame;

	L3DiskBinDumpAttr	*attr;
	L3DiskBinDump		*dump;

public:
	L3DiskBinDumpPanel(L3DiskBinDumpFrame *parentframe, wxWindow *parent);
	~L3DiskBinDumpPanel();

	L3DiskBinDump *GetDumpPanel() const { return dump; }

	void SetTextBinary(int val);
	void SetDataInvert(bool val);
	void SetDataChar(int val);
	void SetDataFont(const wxFont &font);
	wxString GetDataFontName() const;
	int GetDataFontSize() const;
};

/// バイナリダンプ 属性表示部
class L3DiskBinDumpAttr : public wxPanel
{
private:
	wxWindow *parent;
	L3DiskBinDumpFrame *frame;

	wxRadioButton *radText;
	wxRadioButton *radBinary;

	wxRadioButton *radCharAscii;
	wxRadioButton *radCharSJIS;
	wxCheckBox *chkInvert;
	wxButton *btnFont;

public:
	L3DiskBinDumpAttr(L3DiskBinDumpFrame *parentframe, wxWindow *parent);
	~L3DiskBinDumpAttr();

	enum {
		IDC_RADIO_TEXT = 1,
		IDC_RADIO_BINARY,
		IDC_RADIO_CHAR_ASCII,
		IDC_RADIO_CHAR_SJIS,
		IDC_CHECK_INVERT,
		IDC_BUTTON_FONT,
	};

	void OnCheckTextBinary(wxCommandEvent& event);
	void OnCheckChar(wxCommandEvent& event);
	void OnCheckInvert(wxCommandEvent& event);
	void OnClickButton(wxCommandEvent& event);

	void SetTextBinary(int val);
	void SetDataInvert(bool val);
	void SetDataChar(int val);

	wxDECLARE_EVENT_TABLE();
};

/// バイナリダンプ データ表示部
class L3DiskBinDump : public wxScrolledWindow
{
private:
	wxWindow *parent;
	L3DiskBinDumpFrame *frame;

	L3DiskBinDumpTextCtrl *txtHex;
	L3DiskBinDumpTextCtrl *txtAsc;

	L3MemoryBuffers buffers;

	int min_x;
	int min_y;
	int txt_height;

	int  text_binary;
	bool data_invert;
	int  data_char;

	Utils::Dump dump;

	void SetDatasMain(const L3MemoryBuffer *buf);
	void AppendDatasMain(const L3MemoryBuffer *buf);
	void SetDatasBinaryMain(const L3MemoryBuffer *buf, bool append);
	void SetDatasTextMain(const L3MemoryBuffer *buf, bool append);

	void CalcWidthOnTextCtrl(const wxPoint &pv, int def_rows);
	int CalcHeightOnTextCtrl(const L3DiskBinDumpTextCtrl *ctrl, int rows, const wxString &str);

public:
	L3DiskBinDump(L3DiskBinDumpFrame *parentframe, wxWindow *parent);
	~L3DiskBinDump();

	void ClearBuffer();
	L3MemoryBuffer *AppendBuffer(int trk, int sid, int sec, const wxUint8 *buf, size_t len);

	enum {
		IDC_TXT_HEX = 1,
		IDC_TXT_ASC,
	};

	void OnSize(wxSizeEvent& event);
	void OnMouseWheelOnChild(wxMouseEvent& event);

	void SetDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	void AppendDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	void ClearDatas();
	void RefreshData();

	void SetScrollBarPos(int new_ux, int new_uy, int new_px, int new_py);

	void SetTextBinary(int val);
	void SetDataInvert(bool val);
	void SetDataChar(int val);
	void SetDataFont(const wxFont &font);
	wxString GetDataFontName() const;
	int GetDataFontSize() const;

	wxDECLARE_EVENT_TABLE();
};


#endif /* _UIBINDUMP_H_ */
