/// @file diskresult.cpp
///
/// @brief ディスク解析結果
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskresult.h"
#include <wx/intl.h>


const char *gDiskResultMsgs[] = {
	//	ERR_NONE
	(""),
	//	ERR_CANNOT_OPEN
	wxTRANSLATE("Cannot open file."),
	//	ERR_CANNOT_SAVE,
	wxTRANSLATE("Cannot save file."),
	//	ERR_NO_DATA,
	wxTRANSLATE("No data exists."),
	//	ERR_NO_DISK,
	wxTRANSLATE("No disk exists."),
	//	ERR_NO_TRACK,
	wxTRANSLATE("No track exists."),
	//	ERR_NO_FOUND_TRACK,
	wxTRANSLATE("No track found. Run \"Initialize\" to create tracks."),
	//	ERR_REPLACE,
	wxTRANSLATE("Couldn't replace a part of sector."),
	//	ERR_FILE_ONLY_1S,
	wxTRANSLATE("Supported file is only single side and single density (1S)."),
	//	ERR_FILE_SAME,
	wxTRANSLATE("Must be the same disk image type."),
	//	ERR_INTERLEAVE,
	wxTRANSLATE("Couldn't create the disk specified interleave."),
	//	ERR_TOO_LARGE,
	wxTRANSLATE("The file is too large."),
	//	ERR_UNSUPPORTED,
	wxTRANSLATE("Unsupported file."),

	//	ERRV_START  v:1
	wxTRANSLATE("Unknown error. code:%d"),
	//	ERRV_INVALID_DISK  v:1
	wxTRANSLATE("[Disk%d] This is invalid or non supported disk."),
	//  ERRV_DISK_SIZE_ZERO  v:1
	wxTRANSLATE("[Disk%d] Disk size is zero."),
	//	ERRV_DISK_TOO_SMALL  v:1
	wxTRANSLATE("[Disk%d] Disk size is too small."),
	//  ERRV_DISK_TOO_LARGE  v:1
	wxTRANSLATE("[Disk%d] Disk size is invalid or too large."),
	//	ERRV_DISK_HEADER  v:1
	wxTRANSLATE("[Disk%d] Invalid parameter may exists in the disk header."),
	//	ERRV_OVERFLOW_OFFSET  v:4
	wxTRANSLATE("[Disk%d] Overflow offset. This track is ignored. position:%d offset:%d disk size:%d"),
	//	ERRV_OVERFLOW_SIZE  v:2
	wxTRANSLATE("[Disk%d] Overflow disk size. size:%d"),
	//	ERRV_ID_TRACK  v:5
	wxTRANSLATE("[Disk%d] Unmatch id C and track number %d. id[C:%d H:%d R:%d]"),
	//	ERRV_ID_SIDE  v:5
	wxTRANSLATE("[Disk%d] Invalid id H in track %d. id[C:%d H:%d R:%d]"),
	//	ERRV_ID_SECTOR  v:6
	wxTRANSLATE("[Disk%d] Invalid id R in track %d. id[C:%d H:%d R:%d] num of sector:%d"),
	// ERRV_TRACKS_HEADER v:2
	wxTRANSLATE("[Disk%d] Number of track is too large in header. tracks:%d"),
	// ERRV_SIDES_HEADER v:2
	wxTRANSLATE("[Disk%d] Number of side is too large in header. sides:%d"),
	//	ERRV_SECTORS_HEADER  v:2
	wxTRANSLATE("[Disk%d] Invalid number of sector in header. num of sector:%d"),
	//	ERRV_ID_NUM_OF_SECTOR  v:2
	wxTRANSLATE("[Disk%d] Mismatch number of sector in track %d and side %d."),
	// ERRV_TOO_MANY_SECTORS v:5
	wxTRANSLATE("[Disk%d] Too many sectors. Ignore sectors over %d. id[C:%d H:%d] num of sector:%d"),
	// ERRV_SHORT_SECTORS v:5
	wxTRANSLATE("[Disk%d] Number of sector is less than %d. [track:%d side:%d] num of sector:%d"),
	//	ERRV_SECTOR_SIZE_HEADER  v:2
	wxTRANSLATE("[Disk%d] Invalid sector size in header. sector size:%d"),
	//	ERRV_SECTOR_SIZE_SECTOR  v:6
	wxTRANSLATE("[Disk%d] Invalid sector size in sector. id[C:%d H:%d R:%d N:%d] sector size:%d"),
	//	ERRV_DUPLICATE_TRACK  v:4
	wxTRANSLATE("[Disk%d] Duplicate track %d and side %d. Side number change to %d."),
	//	ERRV_DUPLICATE_SECTOR  v:3
	wxTRANSLATE("[Disk%d] Duplicate sector %d. [track:%d side:%d]"),
	// ERRV_NO_SECTOR  v:3
	wxTRANSLATE("[Disk%d] No found sector %d. [track:%d side:%d]"),
	//	ERRV_IGNORE_DATA  v:4
	wxTRANSLATE("[Disk%d] Deleted data found. This sector is ignored. id[C:%d H:%d R:%d]"),
	//	ERRV_TOO_MANY_TRACKS  v:2
	wxTRANSLATE("[Disk%d] Too many tracks. Ignore tracks after %dth."),
	//	ERRV_UNSUPPORTED_TYPE,
	wxTRANSLATE("[Disk%d] Data type %s is unsupported."),
	//	ERRV_END  v:1
	wxTRANSLATE("Unknown error. code:%d"),
	NULL
};

//
//
//
void DiskResult::SetMessageV(int error_number, va_list ap)
{
	wxString msg;

	if (error_number <= 0) {
		return;
	} else if (error_number < ERRV_START) {
		msg = wxGetTranslation(gDiskResultMsgs[error_number]);
	} else if (error_number < ERRV_END) {
		msg = wxString::FormatV(wxGetTranslation(gDiskResultMsgs[error_number]), ap);
	} else {
		msg = wxString::Format(wxGetTranslation(gDiskResultMsgs[ERRV_END]), error_number);
	}
	if (!msg.IsEmpty()) msgs.Add(msg);
}
