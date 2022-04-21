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


class wxFFile;

/// ロギング メッセージをファイルに保存する
class MyLogging
{
private:
	wxFFile *file;
	int		log_level;

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

	void SetError(const wxString &msg);
	void SetError(const char *format, ...);
	void SetErrorV(const char *format, va_list ap);
	void SetInfo(const wxString &msg);
	void SetInfo(const char *format, ...);
	void SetInfoV(const char *format, va_list ap);
	void SetDebug(const wxString &msg);
	void SetDebug(const char *format, ...);
	void SetDebugV(const char *format, va_list ap);
	
	void SetLogLevel(int val) { log_level = val; }
	int  GetLogLevel() const { return log_level; }
};

extern MyLogging myLog;

#endif /* _MY_LOGGING_H_ */

