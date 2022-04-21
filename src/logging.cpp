/// @file logging.cpp
///
/// @brief メッセージ保存用
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "logging.h"
#include <wx/wx.h>
#include <wx/ffile.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/datetime.h>
#include <wx/regex.h>


MyLogging myLog;

//
//
//
MyLogging::MyLogging()
{
	file = NULL;
#ifdef _DEBUG
	log_level = MyLog_Debug;
#else
	log_level = MyLog_Info;
#endif
}
MyLogging::~MyLogging()
{
	delete file;
}
bool MyLogging::Open(const wxString &file_path, const wxString &file_base_name, const wxString &file_ext)
{
	Close();

	int seq_num = FindFile(file_path, file_base_name, file_ext);
	int seq_next = (seq_num + 1) % 10;

	// 過去のログファイルを削除
	wxString filename = file_path;
	filename += file_base_name;
	filename += wxString::Format(wxT("%d"), seq_next);
	filename += file_ext;

	if (wxFileExists(filename)) {
		wxRemoveFile(filename);
	}

	filename = file_path;
	filename += file_base_name;
	filename += wxString::Format(wxT("%d"), seq_num);
	filename += file_ext;
	file = new wxFFile(filename, wxT("w"));

	return file->IsOpened();
}
void MyLogging::Close()
{
	if (file) {
		file->Close();
		delete file;
		file = NULL;
	}
}
int MyLogging::FindFile(const wxString &file_path, const wxString &file_base_name, const wxString &file_ext)
{
	wxRegEx re(file_base_name + wxT("([0-9])"));

	// ログファイルをさがす
	char seq_nums[10];
	memset(seq_nums, 0, sizeof(seq_nums));
	wxString filespec = file_base_name;
	filespec += wxT("*");
	filespec += file_ext;
	wxString filename;

	wxDir dir(file_path);
	bool exist = dir.GetFirst(&filename, filespec, wxDIR_FILES);
	while(exist) {
		if (re.Matches(filename)) {
			wxString sval = re.GetMatch(filename, 1);
			long val = 0;
			sval.ToLong(&val);
			seq_nums[val]++;
		}
		exist = dir.GetNext(&filename);
	}
	// 空いている番号をさがす
	int decide = 0;
	for(int i=0; i<10; i++) {
		if (seq_nums[i] == 0) {
			decide = i;
			break;
		}
	}
	return decide;
}
void MyLogging::SetMessage(int level, const wxString &msg)
{
	if (!file || level > log_level) return;

	wxDateTime ndt = wxDateTime::Now();
	wxString mmsg;

	mmsg = ndt.FormatISODate();
	mmsg += wxT(" ");
	mmsg += ndt.FormatISOTime();
	mmsg += wxT(" ");
	mmsg += msg;
	mmsg += wxT("\n");

	file->Write(mmsg);
	file->Flush();
}
void MyLogging::SetMessage(int level, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	SetMessageV(level, format, ap);

	va_end(ap);
}
void MyLogging::SetMessageV(int level, const char *format, va_list ap)
{
	if (!file || level > log_level) return;

	wxDateTime ndt = wxDateTime::Now();
	wxString mmsg;

	mmsg = ndt.FormatISODate();
	mmsg += wxT(" ");
	mmsg += ndt.FormatISOTime();
	mmsg += wxT(" ");
	mmsg += wxString::FormatV(format, ap);
	mmsg += wxT("\n");

	file->Write(mmsg);
	file->Flush();
}
void MyLogging::SetError(const wxString &msg)
{
	SetMessage(MyLog_Error, msg);
}
void MyLogging::SetError(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	SetMessageV(MyLog_Error, format, ap);

	va_end(ap);
}
void MyLogging::SetErrorV(const char *format, va_list ap)
{
	SetMessageV(MyLog_Error, format, ap);
}
void MyLogging::SetInfo(const wxString &msg)
{
	SetMessage(MyLog_Info, msg);
}
void MyLogging::SetInfo(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	SetMessageV(MyLog_Info, format, ap);

	va_end(ap);
}
void MyLogging::SetInfoV(const char *format, va_list ap)
{
	SetMessageV(MyLog_Info, format, ap);
}
void MyLogging::SetDebug(const wxString &msg)
{
#ifdef _DEBUG
	SetMessage(MyLog_Debug, msg);
#endif
}
void MyLogging::SetDebug(const char *format, ...)
{
#ifdef _DEBUG
	va_list ap;
	va_start(ap, format);

	SetMessageV(MyLog_Debug, format, ap);

	va_end(ap);
#endif
}
void MyLogging::SetDebugV(const char *format, va_list ap)
{
#ifdef _DEBUG
	SetMessageV(MyLog_Debug, format, ap);
#endif
}
