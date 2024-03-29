/// @file uibindump.h
///
/// @brief バイナリダンプ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef UIBINDUMP_H
#define UIBINDUMP_H

#include "../common.h"
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include <wx/splitter.h>
#include "../utils.h"


#undef USE_RICH_TEXT_ON_BINDUMP

#ifdef USE_RICH_TEXT_ON_BINDUMP
#include <wx/richtext/richtextctrl.h>
#else
#include <wx/textctrl.h>
#endif

/// テキストコントロール
class UiDiskBinDumpTextCtrl
#ifdef USE_RICH_TEXT_ON_BINDUMP
	: public wxRichTextCtrl
#else
	: public wxTextCtrl
#endif
{
public:
	UiDiskBinDumpTextCtrl(wxWindow *parent, wxWindowID id); 
};

class MyMenu;
class wxCheckBox;
class wxChoice;
class wxRadioButton;
class wxButton;

class UiDiskFrame;
class UiDiskBinDump;
class UiDiskBinDumpAttr;
class UiDiskBinDumpPanel;

/// バイナリダンプウィンドウ
class UiDiskBinDumpFrame: public wxFrame
{
private:
	// gui
	MyMenu *menuFile;
	MyMenu *menuView;

	UiDiskBinDumpPanel *panel;

	enum en_menu_id
	{
		IDM_VIEW_INVERT = 1,
		IDM_VIEW_TEXT,
		IDM_VIEW_BINARY,
		IDM_VIEW_FONT,

		IDM_VIEW_CHAR_0,
	};

public:
    UiDiskBinDumpFrame(UiDiskFrame *parent, const wxString& title, const wxSize& size);
	~UiDiskBinDumpFrame();

	void OnClose(wxCommandEvent& event);
	void OnViewInvert(wxCommandEvent& event);
	void OnViewChar(wxCommandEvent& event);
	void OnViewTextBinary(wxCommandEvent& event);
	void OnViewFont(wxCommandEvent& event);

	UiDiskBinDumpPanel *GetPanel() const { return panel; }
	UiDiskBinDump *GetDumpPanel() const;

	void SetDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	void AppendDatas(int trk, int sid, int sec, const wxUint8 *buf, size_t len);
	void ClearDatas();

	void SetTextBinary(int val);
	void SetDataInvert(bool val);
	void SetDataChar(int sel);
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
class MyMemoryBuffer : public wxMemoryBuffer
{
private:
	int track_number;
	int side_number;
	int sector_number;

public:
	MyMemoryBuffer(const MyMemoryBuffer &src);
	MyMemoryBuffer(size_t size);

	const wxUint8 *GetByteData() const { return (const wxUint8 *)GetData(); }

	int GetTrackNumber() const { return track_number; }
	int GetSideNumber() const { return side_number; }
	int GetSectorNumber() const { return sector_number; }
	void SetTrackNumber(int val) { track_number = val; }
	void SetSideNumber(int val) { side_number = val; }
	void SetSectorNumber(int val) { sector_number = val; }
};

WX_DEFINE_ARRAY(MyMemoryBuffer *, MyMemoryBuffers);

/// バイナリダンプパネル
class UiDiskBinDumpPanel : public wxSplitterWindow
{
private:
	wxWindow *parent;
	UiDiskBinDumpFrame *frame;

	UiDiskBinDumpAttr	*attr;
	UiDiskBinDump		*dump;

public:
	UiDiskBinDumpPanel(UiDiskBinDumpFrame *parentframe, wxWindow *parent);
	~UiDiskBinDumpPanel();

	UiDiskBinDump *GetDumpPanel() const { return dump; }

	void SetTextBinary(int val);
	void SetDataInvert(bool val);
	void SetDataChar(const wxString &name);
	void SetDataFont(const wxFont &font);
	wxString GetDataFontName() const;
	int GetDataFontSize() const;
};

/// バイナリダンプ 属性表示部
class UiDiskBinDumpAttr : public wxPanel
{
private:
	wxWindow *parent;
	UiDiskBinDumpFrame *frame;

	wxRadioButton *radText;
	wxRadioButton *radBinary;

	wxChoice *comCharCode;
	wxCheckBox *chkInvert;
//	wxButton *btnFont;

public:
	UiDiskBinDumpAttr(UiDiskBinDumpFrame *parentframe, wxWindow *parent);
	~UiDiskBinDumpAttr();

	enum {
		IDC_RADIO_TEXT = 1,
		IDC_RADIO_BINARY,
		IDC_COMBO_CHAR_CODE,
		IDC_CHECK_INVERT,
		IDC_BUTTON_FONT,
	};

	void OnCheckTextBinary(wxCommandEvent& event);
	void OnCheckChar(wxCommandEvent& event);
	void OnCheckInvert(wxCommandEvent& event);
	void OnClickButton(wxCommandEvent& event);

	void SetTextBinary(int val);
	void SetDataInvert(bool val);
	void SetDataChar(const wxString &name);

	wxDECLARE_EVENT_TABLE();
};

/// バイナリダンプ データ表示部
class UiDiskBinDump : public wxScrolledWindow
{
private:
	wxWindow *parent;
	UiDiskBinDumpFrame *frame;

	UiDiskBinDumpTextCtrl *txtHex;
	UiDiskBinDumpTextCtrl *txtAsc;

	MyMemoryBuffers buffers;

	int min_x;
	int min_y;
	int txt_height;

	int  text_binary;
	bool data_invert;
	wxString data_char;

	Utils::Dump dump;

	void SetDatasMain(const MyMemoryBuffer *buf);
	void AppendDatasMain(const MyMemoryBuffer *buf);
	void SetDatasBinaryMain(const MyMemoryBuffer *buf, bool append);
	void SetDatasTextMain(const MyMemoryBuffer *buf, bool append);

	void CalcWidthOnTextCtrl(const wxPoint &pv, int def_rows);
	int CalcHeightOnTextCtrl(const UiDiskBinDumpTextCtrl *ctrl, int rows, const wxString &str);

public:
	UiDiskBinDump(UiDiskBinDumpFrame *parentframe, wxWindow *parent);
	~UiDiskBinDump();

	void ClearBuffer();
	MyMemoryBuffer *AppendBuffer(int trk, int sid, int sec, const wxUint8 *buf, size_t len);

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
	void SetDataChar(const wxString &name);
	void SetDataFont(const wxFont &font);
	wxString GetDataFontName() const;
	int GetDataFontSize() const;

	wxDECLARE_EVENT_TABLE();
};


#endif /* UIBINDUMP_H */
