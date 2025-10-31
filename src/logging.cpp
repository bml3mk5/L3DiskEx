/// @file logging.cpp
///
/// @brief メッセージ保存用
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "logging.h"
#include <wx/wx.h>
#include <wx/file.h>
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
	p_file = NULL;
#ifdef _DEBUG
	m_log_level = MyLog_Debug;
#else
	m_log_level = MyLog_Info;
#endif
}
MyLogging::~MyLogging()
{
	delete p_file;
}
bool MyLogging::Open(const wxString &file_path, const wxString &file_base_name, const wxString &file_ext)
{
	Close();

	int seq_num = FindFile(file_path, file_base_name, file_ext);
	int seq_next = (seq_num + 1) % 10;

	// 過去のログファイルを削除
	wxString fullpath = file_path;
	wxString filename;
	filename = file_base_name;
	filename += wxString::Format(wxT("%d"), seq_next);
	filename += file_ext;
	fullpath += filename;

	if (wxFileExists(fullpath)) {
		wxRemoveFile(fullpath);
	}

	// 新規作成
	fullpath = file_path;
	filename = file_base_name;
	filename += wxString::Format(wxT("%d"), seq_num);
	filename += file_ext;
	fullpath += filename;
	p_file = new wxFile(fullpath, wxFile::write);

	bool rc = p_file->IsOpened();
	if (rc) {
		m_file_path = fullpath;

		// 一度閉じて再度RWで開く
		p_file->Close();
		p_file->Open(fullpath, wxFile::read_write);

		SetInfo(wxT("Opened ") + filename);
	} else {
		m_file_path.Clear();
	}
	return rc;
}
void MyLogging::Close()
{
	if (p_file) {
		p_file->Close();
		delete p_file;
		p_file = NULL;
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
	if (!p_file || level > m_log_level) return;

	wxDateTime ndt = wxDateTime::Now();
	wxString mmsg;

	mmsg = ndt.FormatISODate();
	mmsg += wxT(" ");
	mmsg += ndt.FormatISOTime();
	mmsg += wxT(" ");
	mmsg += msg;
	mmsg += wxT("\n");

	p_file->Write(mmsg);
	p_file->Flush();
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
	if (!p_file || level > m_log_level) return;

	wxDateTime ndt = wxDateTime::Now();
	wxString mmsg;

	mmsg = ndt.FormatISODate();
	mmsg += wxT(" ");
	mmsg += ndt.FormatISOTime();
	mmsg += wxT(" ");
	mmsg += wxString::FormatV(format, ap);
	mmsg += wxT("\n");

	p_file->Write(mmsg);
	p_file->Flush();
}
void MyLogging::SetMessage(int level, const wchar_t *format, ...)
{
	va_list ap;
	va_start(ap, format);

	SetMessageV(level, format, ap);

	va_end(ap);
}
void MyLogging::SetMessageV(int level, const wchar_t *format, va_list ap)
{
	if (!p_file || level > m_log_level) return;

	wxDateTime ndt = wxDateTime::Now();
	wxString mmsg;

	mmsg = ndt.FormatISODate();
	mmsg += wxT(" ");
	mmsg += ndt.FormatISOTime();
	mmsg += wxT(" ");
	mmsg += wxString::FormatV(format, ap);
	mmsg += wxT("\n");

	p_file->Write(mmsg);
	p_file->Flush();
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
void MyLogging::SetError(const wchar_t *format, ...)
{
	va_list ap;
	va_start(ap, format);

	SetMessageV(MyLog_Error, format, ap);

	va_end(ap);
}
void MyLogging::SetErrorV(const wchar_t *format, va_list ap)
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
void MyLogging::SetInfo(const wchar_t *format, ...)
{
	va_list ap;
	va_start(ap, format);

	SetMessageV(MyLog_Info, format, ap);

	va_end(ap);
}
void MyLogging::SetInfoV(const wchar_t *format, va_list ap)
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
void MyLogging::SetDebug(const wchar_t *format, ...)
{
#ifdef _DEBUG
	va_list ap;
	va_start(ap, format);

	SetMessageV(MyLog_Debug, format, ap);

	va_end(ap);
#endif
}
void MyLogging::SetDebugV(const wchar_t *format, va_list ap)
{
#ifdef _DEBUG
	SetMessageV(MyLog_Debug, format, ap);
#endif
}

bool MyLogging::GetLog(wxString &text)
{
	if (!p_file) return false;

	p_file->Seek(0);
	bool rc = p_file->ReadAll(&text);
	p_file->SeekEnd();
	return rc;
}
