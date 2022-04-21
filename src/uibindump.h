/// @file uibindump.h
///
/// @brief バイナリダンプ
///
#ifndef _UIBINDUMP_H_
#define _UIBINDUMP_H_

#include "common.h"
#include <wx/frame.h>
#include <wx/scrolwin.h>
#include <wx/splitter.h>
//#include <wx/toolbar.h>
#include "utils.h"

class wxMenu;
class wxTextCtrl;
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
		IDM_VIEW_CHAR_ASCII,
		IDM_VIEW_CHAR_SJIS,
	};

//	void RecreateToolbar();
//	void PopulateToolbar(wxToolBar* toolBar);

public:
    L3DiskBinDumpFrame(L3DiskFrame *parent, const wxString& title, const wxSize& size);
	~L3DiskBinDumpFrame();

	void OnClose(wxCommandEvent& event);
	void OnViewInvert(wxCommandEvent& event);
	void OnViewChar(wxCommandEvent& event);

	L3DiskBinDumpPanel *GetPanel() const { return panel; }
	L3DiskBinDump *GetDumpPanel() const;
	void SetDataInvert(bool val);
	void SetDataChar(int val);
	void SetDataFont(const wxFont &font);
	wxString GetDataFontName() const;
	int GetDataFontSize() const;

	void ShowDataFontDialog();

	void ToggleControl(int id, bool checked);

	wxDECLARE_EVENT_TABLE();
};

WX_DEFINE_ARRAY(wxMemoryBuffer *, L3MemoryBuffers);

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

	wxRadioButton *radCharAscii;
	wxRadioButton *radCharSJIS;
	wxCheckBox *chkInvert;
	wxButton *btnFont;

public:
	L3DiskBinDumpAttr(L3DiskBinDumpFrame *parentframe, wxWindow *parent);
	~L3DiskBinDumpAttr();

	enum {
		IDC_RADIO_CHAR_ASCII = 1,
		IDC_RADIO_CHAR_SJIS,
		IDC_CHECK_INVERT,
		IDC_BUTTON_FONT,
	};

	void OnCheckChar(wxCommandEvent& event);
	void OnCheckInvert(wxCommandEvent& event);
	void OnClickButton(wxCommandEvent& event);

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

	wxTextCtrl *txtHex;
	wxTextCtrl *txtAsc;

	L3MemoryBuffers buffers;

	int min_x;
	int min_y;
	int txt_height;

	bool data_invert;
	int  data_char;

	L3DiskUtils::Dump dump;

	void SetDatasMain(const wxUint8 *buf, size_t len);
	void AppendDatasMain(const wxUint8 *buf, size_t len);

public:
	L3DiskBinDump(L3DiskBinDumpFrame *parentframe, wxWindow *parent);
	~L3DiskBinDump();

	void ClearBuffer();
	void AppendBuffer(const wxUint8 *buf, size_t len);

	enum {
		IDC_TXT_HEX = 1,
		IDC_TXT_ASC,
	};

	void OnSize(wxSizeEvent& event);
	void OnMouseWheelOnChild(wxMouseEvent& event);

	void SetDatas(const wxUint8 *buf, size_t len);
	void AppendDatas(const wxUint8 *buf, size_t len);
	void ClearDatas();
	void RefreshData();

	void SetScrollBarPos(int new_ux, int new_uy, int new_px, int new_py);

	void SetDataInvert(bool val);
	void SetDataChar(int val);
	void SetDataFont(const wxFont &font);
	wxString GetDataFontName() const;
	int GetDataFontSize() const;

	wxDECLARE_EVENT_TABLE();
};


#endif /* _UIBINDUMP_H_ */
