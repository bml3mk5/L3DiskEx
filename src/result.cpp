/// @file result.cpp
///
/// @brief 結果保存用
///

#include "result.h"


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
void ResultInfo::SetMessage(int error_number, va_list ap)
{
}
void ResultInfo::GetMessages(wxArrayString &arr)
{
	arr = msgs;
}
const wxArrayString &ResultInfo::GetMessages(int maxrow)
{
	bufs.Empty();
	if (maxrow >= 0) {
		size_t cnt = msgs.Count();
		for(size_t i = 0; i < (size_t)maxrow && i < cnt; i++) bufs.Add(msgs[i]);
		if ((size_t)maxrow < cnt) bufs.Add(wxString::Format(_("And have more %d messages..."), cnt - (size_t)maxrow));
		return bufs;
	} else {
		return msgs;
	}
}
