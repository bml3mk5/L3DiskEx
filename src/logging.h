/// @file logging.h
///
/// @brief メッセージ保存用
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _MY_LOGGING_H_
#define _MY_LOGGING_H_

#include "common.h"
#include <stdarg.h>
#include <wx/string.h>

class wxFile;

/// ロギング メッセージをファイルに保存する
class MyLogging
{
private:
	wxFile  *p_file;
	wxString m_file_path;
	int      m_log_level;

	int FindFile(const wxString &file_path, const wxString &file_base_name, const wxString &file_ext);

public:
	MyLogging();
	~MyLogging();
	
	enum en_mylog_level {
		MyLog_Error = 1,
		MyLog_Info,
		MyLog_Debug
	};

	bool Open(const wxString &file_path, const wxString &file_base_name, const wxString &file_ext);
	void Close();

	void SetMessage(int level, const wxString &msg);
	void SetMessage(int level, const char *format, ...);
	void SetMessageV(int level, const char *format, va_list ap);
	void SetMessage(int level, const wchar_t *format, ...);
	void SetMessageV(int level, const wchar_t *format, va_list ap);

	void SetError(const wxString &msg);
	void SetError(const char *format, ...);
	void SetErrorV(const char *format, va_list ap);
	void SetError(const wchar_t *format, ...);
	void SetErrorV(const wchar_t *format, va_list ap);
	void SetInfo(const wxString &msg);
	void SetInfo(const char *format, ...);
	void SetInfoV(const char *format, va_list ap);
	void SetInfo(const wchar_t *format, ...);
	void SetInfoV(const wchar_t *format, va_list ap);
	void SetDebug(const wxString &msg);
	void SetDebug(const char *format, ...);
	void SetDebugV(const char *format, va_list ap);
	void SetDebug(const wchar_t *format, ...);
	void SetDebugV(const wchar_t *format, va_list ap);

	bool GetLog(wxString &text);
	
	void SetLogLevel(int val) { m_log_level = val; }
	int  GetLogLevel() const { return m_log_level; }
	const wxString &GetFilePath() const { return m_file_path; }
};

extern MyLogging myLog;

#endif /* _MY_LOGGING_H_ */

