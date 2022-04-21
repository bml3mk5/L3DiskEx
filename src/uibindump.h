/// @file uibindump.h
///
/// @brief バイナリダンプ
///
#ifndef _UIBINDUMP_H_
#define _UIBINDUMP_H_

#include "common.h"
#include <wx/wx.h>

///
class L3DiskBinDump : public wxScrolledWindow
{
public:
	wxWindow *parent;
	L3DiskFrame *frame;

	wxTextCtrl *txtHex;
	wxTextCtrl *txtAsc;

public:
	L3DiskBinDump(L3DiskFrame *parentframe, wxWindow *parent);

	enum {
		IDC_TXT_HEX = 1,
		IDC_TXT_ASC,
	};

//	void OnSize(wxSizeEvent& event);

	void SetDatas(const wxUint8 *buf, size_t len);
	void ClearDatas();

	void SetScrollBarPos(int new_ux, int new_uy, int new_px, int new_py);
	wxString DumpAscii(const wxUint8 *buffer, size_t bufsize);

	wxDECLARE_EVENT_TABLE();
};


#endif /* _UIBINDUMP_H_ */
