/// @file result.cpp
///
/// @brief 結果保存用
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "result.h"
#include <wx/intl.h>
#include <wx/msgdlg.h>
#include "logging.h"


//
//
//
ResultInfo::ResultInfo()
{
	valid = 0;
	msgs.Empty();
}
void ResultInfo::Clear()
{
	valid = 0;
	msgs.Empty();
}
void ResultInfo::SetError(int error_number, ...)
{
	va_list ap;
	va_start(ap, error_number);

	SetMessage(error_number, ap);

	va_end(ap);

	valid = -1;
}
void ResultInfo::SetWarn(int error_number, ...)
{
	va_list ap;
	va_start(ap, error_number);

	SetMessage(error_number, ap);

	va_end(ap);

	if (valid == 0) valid = 1;
}
void ResultInfo::SetInfo(int error_number, ...)
{
	va_list ap;
	va_start(ap, error_number);

	SetMessage(error_number, ap);

	va_end(ap);

	if (valid == 0) valid = 2;
}
void ResultInfo::GetMessages(wxArrayString &arr)
{
	arr = msgs;
}
const wxArrayString &ResultInfo::GetMessages(int maxrow)
{
	bufs.Empty();
	int level = valid < 0 ? myLog.MyLog_Error : myLog.MyLog_Info;
	if (valid < 2) {
		for(size_t i = 0; i < msgs.Count(); i++) {
			myLog.SetMessage(level, msgs.Item(i));
		}
	}
	if (maxrow >= 0) {
		size_t cnt = msgs.Count();
		for(size_t i = 0; i < (size_t)maxrow && i < cnt; i++) {
			bufs.Add(msgs[i]);
		}
		if ((size_t)maxrow < cnt) {
			bufs.Add(wxString::Format(_("And have more %d messages..."), cnt - (size_t)maxrow));
		}
		return bufs;
	} else {
		return msgs;
	}
}

void ResultInfo::ShowMessage(int level, const wxArrayString &msgs)
{
	wxString msg;
	for(size_t i=0; i<msgs.Count(); i++) {
		msg += msgs[i];
		msg += wxT("\n");
	}
	if (msg.IsEmpty()) return;

	wxString caption;
	int style = wxOK;

	if (level < 0) {
		caption = _("Error");
		style |= wxICON_HAND;
	} else if (level == 1) {
		caption = _("Warning");
		style |= wxICON_EXCLAMATION;
	} else {
		caption = _("Information");
		style |= wxICON_INFORMATION;
	}

	wxMessageBox(msg, caption, style);
}
