/// @file basicerror.cpp
///
/// @brief disk basic error messages
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicerror.h"
#include <wx/intl.h>
#include <wx/wfstream.h>
#include <wx/numformatter.h>


const char *gDiskBasicErrorMsgs[] = {
	//	ERR_NONE
	"",
	//	ERR_SUPPORTED
	wxTRANSLATE("Unsupported disk for DISK BASIC."),
	//	ERR_FORMATTED
	wxTRANSLATE("It may be unformatted disk for DISK BASIC."),
	//	ERR_UNSELECT_DISK
	wxTRANSLATE("Select a disk."),
	//	ERR_WRITE_PROTECTED
	wxTRANSLATE("Write protected."),
	//	ERR_WRITE_UNSUPPORTED
	wxTRANSLATE("Write process is unsupported."),
	//	ERR_FILE_NOT_FOUND
	wxTRANSLATE("File not found."),
	//	ERR_FILE_ALREADY_EXIST
	wxTRANSLATE("File already exist."),
	//	ERR_DIRECTORY_FULL
	wxTRANSLATE("Directory full."),
	//	ERR_DISK_FULL
	wxTRANSLATE("Disk full."),
	//	ERR_FILE_TOO_LARGE
	wxTRANSLATE("File is too large."),
	//	ERR_NOT_ENOUGH_FREE
	wxTRANSLATE("There is not enough free space."),
	//	ERR_CANNOT_EXPORT
	wxTRANSLATE("Cannot export."),
	//	ERR_CANNOT_IMPORT
	wxTRANSLATE("Cannot import."),
	//	ERR_CANNOT_VERIFY
	wxTRANSLATE("Cannot verify."),
	//	ERR_CANNOT_FORMAT
	wxTRANSLATE("Cannot format."),
	//	ERR_FORMATTING
	wxTRANSLATE("Error occurred while formatting."),
	//	ERR_FORMAT_UNSUPPORTED
	wxTRANSLATE("Format process is unsupported."),
	//	ERR_DELETE_UNSUPPORTED
	wxTRANSLATE("Delete process is unsupported."),
	//	ERR_CANNOT_IMPORT_DIRECTORY
	wxTRANSLATE("Cannot import directory."),
	//	ERR_CANNOT_MAKE_DIRECTORY
	wxTRANSLATE("Cannot make directory."),
	//	ERR_MAKING_DIRECTORY
	wxTRANSLATE("Error occured on making a sub directory."),
	//	ERR_IN_FAT_AREA
	wxTRANSLATE("Invalid structure exists in FAT area."),
	//	ERR_IN_DIRECTORY_AREA
	wxTRANSLATE("Invalid structure exists in directory area."),
	//	ERR_IN_PARAMETER_AREA
	wxTRANSLATE("Invalid structure exists in parameter area."),
	//	ERR_INVALID_IN_PARAMETER_AREA
	wxTRANSLATE("Invalid value exists in parameter area. Use default settings."),
	//	ERR_FILENAME_EMPTY
	wxTRANSLATE("File name is required."),
	//	ERR_FILEEXT_EMPTY
	wxTRANSLATE("File extension is required."),
	//  ERR_END_ADDR_TOO_SMALL
	wxTRANSLATE("End address is smaller than load address."),
	//	ERR_PATH_TOO_DEEP
	wxTRANSLATE("Path is too deep."),

	//	ERRV_START v:1
	wxTRANSLATE("Unknown error. code:%d"),
	//	ERRV_VERIFY_FILE  v:4
	wxTRANSLATE("Verify error. group:%d track:%d side:%d sector:%d"),
	//	ERRV_MISMATCH_FILESIZE  v:2
	wxTRANSLATE("Mismatch file size. origin:%d now:%d"),
	//	ERRV_NO_TRACK  v:3
	wxTRANSLATE("No track exists. group:%d track:%d side:%d"),
	//	ERRV_NO_SECTOR  v:4
	wxTRANSLATE("No sector exists. group:%d track:%d side:%d sector:%d"),
	//	ERRV_INVALID_SECTOR  v:5
	wxTRANSLATE("Invalid sector. group:%d track:%d side:%d sector:%d sector_size:%d"),
	//	ERRV_NOTHING_IN_TRACK  v:2
	wxTRANSLATE("There is nothing in track. track:%d side:%d"),
	//	ERRV_NUM_OF_SECTORS_IN_TRACK  v:2
	wxTRANSLATE("Number of sectors is short in track. track:%d side:%d"),
	//	ERRV_NO_SECTOR_IN_TRACK  v:3
	wxTRANSLATE("No sector exists in track. track:%d side:%d sector:%d"),
	//	ERRV_INVALID_VALUE_IN  v:1
	wxTRANSLATE("Invalid value in %ls."),
	//  ERRV_CANNOT_EDIT_NAME  v:1
	wxTRANSLATE("Cannot edit the name '%ls'."),
	//  ERRV_CANNOT_SET_NAME v:1
	wxTRANSLATE("Cannot set the name '%ls'."),
	//	ERRV_CANNOT_EXPORT v:1
	wxTRANSLATE("Cannot export '%ls'."),
	//	ERRV_CANNOT_DELETE v:1
	wxTRANSLATE("Cannot delete '%ls'."),
	//	ERRV_CANNOT_DELETE_DIRECTORY v:1
	wxTRANSLATE("Cannot delete '%ls' which is not empty."),
	//	ERRV_ALREADY_DELETED v:1
	wxTRANSLATE("File '%ls' is already deleted."),
	//	ERRV_ALREADY_EXISTS v:1
	wxTRANSLATE("File '%ls' already exists."),

	//	ERRV_END  v:1
	wxTRANSLATE("Unknown error. code:%d"),
	NULL
};

//
//
//
void DiskBasicError::SetMessage(int error_number, va_list ap)
{
	wxString msg;
	if (error_number <= 0) {
		return;
	} else if (error_number < ERRV_START) {
		msg = wxGetTranslation(gDiskBasicErrorMsgs[error_number]);
	} else if (error_number < ERRV_END) {
		msg = wxString::FormatV(wxGetTranslation(gDiskBasicErrorMsgs[error_number]), ap);
	} else {
		msg = wxString::Format(wxGetTranslation(gDiskBasicErrorMsgs[ERRV_END]), error_number);
	}

	if (msgs.Index(msg) == wxNOT_FOUND) {
		msgs.Add(msg);
	}
}
