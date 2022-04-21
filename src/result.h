/// @file result.h
///
/// @brief 結果保存用
///
#ifndef _RESULTINFO_H_
#define _RESULTINFO_H_

#include "common.h"
#include <wx/string.h>
#include <wx/arrstr.h>

/// 結果保存用のクラス absolute
class ResultInfo
{
protected:
	int valid;
	wxArrayString msgs;
	wxArrayString bufs;

public:
	ResultInfo();
	virtual ~ResultInfo() {}

	virtual void Clear();

	virtual void SetError(int error_number, ...);
	virtual void SetWarn(int error_number, ...);
	virtual void SetInfo(int error_number, ...);
	virtual void SetMessage(int error_number, va_list ap) = 0;
	virtual void GetMessages(wxArrayString &arr);
	virtual const wxArrayString &GetMessages(int maxrow = 20);

	/// 結果レベルをセット
	virtual void SetValid(int val) { valid = val; }
	/// 結果レベルを返す
	/// @retval 0 :正常
	/// @retval -1:エラーあり
	/// @retval 1 :警告あり
	/// @retval 2 :情報あり
	virtual int GetValid() const { return valid; }

	static void ShowMessage(int level, const wxArrayString &msgs);
};

#endif /* _RESULTINFO_H_ */

