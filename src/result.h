/// @file result.h
///
/// @brief 結果保存用
///
#ifndef _RESULTINFO_H_
#define _RESULTINFO_H_

#include "common.h"
#include <wx/wx.h>

class ResultInfo
{
protected:
	int valid;	///< エラーあり:-1 警告あり:1
	wxArrayString msgs;
	wxArrayString bufs;

public:
	ResultInfo();
	virtual ~ResultInfo() {}

	virtual void Clear();

	virtual void SetError(int error_number, ...);
	virtual void SetWarn(int error_number, ...);
	virtual void SetMessage(int error_number, va_list ap);
	virtual void GetMessages(wxArrayString &arr);
	virtual const wxArrayString &GetMessages(int maxrow = 30);

	virtual void SetValid(int val) { valid = val; }
	virtual int GetValid() const { return valid; }
};

#endif /* _RESULTINFO_H_ */

