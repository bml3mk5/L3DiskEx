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
	Clear();
}
ResultInfo::ResultInfo(const ResultInfo &src)
{
	valid = src.valid;
	msgs = src.msgs;
	bufs.Empty();
}
ResultInfo &ResultInfo::operator=(const ResultInfo &src)
{
	valid = src.valid;
	msgs = src.msgs;
	bufs.Empty();
	return *this;
}
void ResultInfo::Clear()
{
	valid = 0;
	msgs.Empty();
	bufs.Empty();
}
void ResultInfo::SetError(int error_number, ...)
{
	va_list ap;
	va_start(ap, error_number);
	SetErrorV(error_number, ap);
	va_end(ap);
}
void ResultInfo::SetWarn(int error_number, ...)
{
	va_list ap;
	va_start(ap, error_number);
	SetWarnV(error_number, ap);
	va_end(ap);
}
void ResultInfo::SetInfo(int error_number, ...)
{
	va_list ap;
	va_start(ap, error_number);
	SetInfoV(error_number, ap);
	va_end(ap);
}
void ResultInfo::SetErrorV(int error_number, va_list ap)
{
	SetMessageV(error_number, ap);
	valid = -1;
}
void ResultInfo::SetWarnV(int error_number, va_list ap)
{
	SetMessageV(error_number, ap);
	if (valid == 0) valid = 1;
}
void ResultInfo::SetInfoV(int error_number, va_list ap)
{
	SetMessageV(error_number, ap);
	if (valid == 0) valid = 2;
}
/// メッセージ配列を返す
/// @param[out] arr メッセージの配列
void ResultInfo::GetMessages(wxArrayString &arr)
{
	arr = msgs;
}
/// メッセージをログに出力＆配列で返す
/// @param[in] maxrow 配列の最大行 -1:無限
/// @return メッセージの配列
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
			bufs.Add(wxString::Format(_("And have more %u messages..."), (unsigned int)(cnt - (size_t)maxrow)));
		}
		return bufs;
	} else {
		return msgs;
	}
}

/// 結果ダイアログを表示
void ResultInfo::Show()
{
	ShowMessage(GetValid(), GetMessages());
}

/// 結果ダイアログを表示
/// @param[in] level 0:正常 -1:エラー時 1:警告時
/// @param[in] msgs メッセージ配列
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

/// メッセージダイアログを表示
/// @param[in] code -1:エラー時 1:警告時
/// @param[in] msgs メッセージ配列
/// @retval 0  警告時YESを押下した
/// @retval -1 エラー or 警告時NOを押下した
int ResultInfo::ShowErrWarnMessage(int code, const wxArrayString &msgs)
{
	wxString msg;
	for(size_t i=0; i<msgs.Count(); i++) {
		msg += msgs[i];
		msg += wxT("\n");
	}
	if (msg.IsEmpty()) return 0;

	wxString caption;
	int style = 0;

	if (code < 0) {
		caption = _("Error");
		style = (wxOK | wxICON_HAND);
	} else if (code > 0) {
		caption = _("Warning");
		msg += wxT("\n");
		msg += _("Do you want to continue?");
		style = (wxYES_NO | wxICON_EXCLAMATION);
	} else {
		caption = _("Information");
		style = (wxOK | wxICON_INFORMATION);
	}

	int ans = wxMessageBox(msg, caption, style);
	return (code == 0 || ans == wxYES ? 0 : -1);
}
