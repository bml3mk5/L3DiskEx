/// @file uicommon.h
///
/// @brief UI 共通
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _UICOMMON_H_
#define _UICOMMON_H_

#include "../common.h"


#define USE_TREE_CTRL_ON_DISK_LIST
#define USE_LIST_CTRL_ON_FILE_LIST
#define USE_LIST_CTRL_ON_TRACK_LIST
#define USE_LIST_CTRL_ON_SECTOR_LIST


/// リストコントロールのカラムの初期値
struct st_list_columns {
	const char *name;		///< 名前
	const char *label;		///< ラベル名
	bool		have_icon;	///< アイコンを持つか
	int         width;		///< 幅
	wxAlignment align;		///< アライン
	bool		sortable;	///< ソート可能か
};

#endif /* _UICOMMON_H_ */

