/// @file basiccommon.h
///
/// @brief disk basic common
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICCOMMON_H_
#define _BASICCOMMON_H_


#include "common.h"
#include <wx/string.h>

/// 共通属性フラグ
enum en_file_type_mask {
	FILE_TYPE_BASIC_MASK	 = 0x00001,
	FILE_TYPE_DATA_MASK		 = 0x00002,
	FILE_TYPE_MACHINE_MASK	 = 0x00004,
	FILE_TYPE_ASCII_MASK	 = 0x00008,
	FILE_TYPE_BINARY_MASK	 = 0x00010,
	FILE_TYPE_RANDOM_MASK	 = 0x00020,
	FILE_TYPE_ENCRYPTED_MASK = 0x00040,
	FILE_TYPE_READWRITE_MASK = 0x00080,
	FILE_TYPE_READONLY_MASK	 = 0x00100,
	FILE_TYPE_SYSTEM_MASK	 = 0x00200,
	FILE_TYPE_HIDDEN_MASK	 = 0x00400,
	FILE_TYPE_VOLUME_MASK	 = 0x00800,
	FILE_TYPE_DIRECTORY_MASK = 0x01000,
	FILE_TYPE_ARCHIVE_MASK	 = 0x02000,
	FILE_TYPE_NONSHARE_MASK	 = 0x08000,
	FILE_TYPE_UNDELETE_MASK	 = 0x10000,
	FILE_TYPE_WRITEONLY_MASK = 0x20000,
	FILE_TYPE_TEMPORARY_MASK = 0x40000,
};
/// 共通属性フラグ位置
enum en_file_type_pos {
	FILE_TYPE_BASIC_POS		= 0,
	FILE_TYPE_DATA_POS		= 1,
	FILE_TYPE_MACHINE_POS	= 2,
	FILE_TYPE_ASCII_POS		= 3,
	FILE_TYPE_BINARY_POS	= 4,
	FILE_TYPE_RANDOM_POS	= 5,
	FILE_TYPE_ENCRYPTED_POS = 6,
	FILE_TYPE_READWRITE_POS = 7,
	FILE_TYPE_READONLY_POS	= 8,
	FILE_TYPE_SYSTEM_POS	= 9,
	FILE_TYPE_HIDDEN_POS	= 10,
	FILE_TYPE_VOLUME_POS	= 11,
	FILE_TYPE_DIRECTORY_POS = 12,
	FILE_TYPE_ARCHIVE_POS	= 13,
	FILE_TYPE_NONSHARE_POS	= 15,
	FILE_TYPE_UNDELETE_POS	= 16,
	FILE_TYPE_WRITEONLY_POS	= 17,
	FILE_TYPE_TEMPORARY_POS	= 18,
};

//
//
// ディレクトリ
//
//

#pragma pack(1)
/// ディレクトリエントリ L3,S1 ５インチ,８インチ(倍密度)
typedef struct st_directory_l3_2d {
	wxUint8 name[8];
	wxUint8 ext[3];
	wxUint8 type;
	wxUint8 type2;
	wxUint8 start_group;
	struct {
		wxUint8 h;
		wxUint8 l;
	} end_bytes;	///< used size of end cluster (big endien)

	char reserved[16];
} directory_l3_2d_t;

/// ディレクトリエントリ L3 ３インチ(単密度) / F-BASIC 倍密度
typedef struct st_directory_fat8f {
	wxUint8 name[8];
	wxUint8 ext[3];	/// not used.
	wxUint8 type;
	wxUint8 type2;
	wxUint8 type3;
	wxUint8 start_group;

	char reserved[17];
} directory_fat8f_t;

/// ディレクトリエントリ n88 BASIC (16bytes)
typedef struct st_directory_n88 {
	wxUint8 name[6];
	wxUint8 ext[3];
	wxUint8 type;
	wxUint8 start_group;
	char reserved[5];
} directory_n88_t;

/// ディレクトリエントリ X1 Hu-BASIC
typedef struct st_directory_x1_hu {
	wxUint8 type;
	wxUint8 name[13];
	wxUint8 ext[3];
	wxUint8 password;
	wxUint16 file_size;
	wxUint16 load_addr;
	wxUint16 exec_addr;
	wxUint8 date[3];	// yymwdd yy:BCD 00-99  m:HEX 0-C w:WEEK HEX 0(SUN)-7(SAT) dd:BCD
	wxUint8 time[2];	// hhmi BCD
	wxUint8 start_group_h;
	wxUint16 start_group_l;
} directory_x1_hu_t;

/// ディレクトリエントリ MZ DISK BASIC
typedef struct st_directory_mz {
	wxUint8 type;
	wxUint8 name[17];	// file name has $0D on the end of string
	wxUint8 type2;
	wxUint8 reserved;
	wxUint16 file_size;
	wxUint16 load_addr;
	wxUint16 exec_addr;
	wxUint8 date_time[4];
	wxUint16 start_sector;
} directory_mz_t;

/// ディレクトリエントリ MS-DOS FAT
typedef struct st_directory_ms_dos {
	wxUint8 name[8];
	wxUint8 ext[3];
	wxUint8 type;
	wxUint8 ntres;
	wxUint8  ctime_tenth;
	wxUint16 ctime;
	wxUint16 cdate;
	wxUint16 adate;
	wxUint16 start_group_hi;
	wxUint16 wtime;
	wxUint16 wdate;
	wxUint16 start_group;
	wxUint32 file_size;
} directory_msdos_t;

/// ディレクトリエントリ FLEX
typedef struct st_directory_flex {
	wxUint8 name[8];
	wxUint8 ext[3];
	wxUint8 type;
	wxUint8 reserved;
	wxUint8 start_track;
	wxUint8 start_sector;
	wxUint8 last_track;
	wxUint8 last_sector;
	wxUint16 total_sectors;
	wxUint8 random_access;
	wxUint8 reserved2;
	wxUint8 month;
	wxUint8 day;
	wxUint8 year;
} directory_flex_t;

/// FLEX top of each sector
typedef struct st_flex_ptr {
	wxUint8  next_track;
	wxUint8  next_sector;
	wxUint16 seq_num;
} flex_ptr_t;

/// OS-9 LSN
typedef struct st_os9_lsn {
	wxUint8 h;
	wxUint8 m;
	wxUint8 l;
} os9_lsn_t;

#define GET_OS9_LSN(lsn) (((wxUint32)(lsn.h) << 16) | ((wxUint32)(lsn.m) << 8) | lsn.l)
#define SET_OS9_LSN(lsn, val) { \
	lsn.h = (((val) & 0xff0000) >> 16); \
	lsn.m = (((val) & 0xff00) >> 8); \
	lsn.l = ((val) & 0xff); \
}

/// OS-9 Segment
typedef struct st_os9_segment {
	os9_lsn_t LSN;
	wxUint16  SIZ;
} os9_segment_t;

/// OS-9 Date Format
typedef struct st_os9_date {
	wxUint8 yy;
	wxUint8 mm;
	wxUint8 dd;
	wxUint8 hh;
	wxUint8 mi;
} os9_date_t;

/// OS-9 Created Date
typedef struct st_os9_cdate {
	wxUint8 yy;
	wxUint8 mm;
	wxUint8 dd;
} os9_cdate_t;

/// ディレクトリエントリ OS-9
typedef struct st_directory_os9 {
	wxUint8		DE_NAM[28];
	wxUint8		DE_Reserved;
	os9_lsn_t	DE_LSN;	// link to FD
} directory_os9_t;

/// OS-9 File Descriptor
typedef struct st_directory_os9_fd {
	wxUint8		FD_ATT;	// attr
	wxUint16	FD_OWN;	// owner id
	os9_date_t	FD_DAT;	// date
	wxUint8		FD_LNK;	// link count
	wxUint32	FD_SIZ;	// in bytes
	os9_cdate_t	FD_DCR;	// created date
	os9_segment_t FD_SEG[48];	// 5*48=240
} directory_os9_fd_t;

/// ディレクトリエントリ CP/M
typedef struct st_directory_cpm {
	wxUint8	type;	// user id
	wxUint8 name[8];
	wxUint8 ext[3];
	wxUint8 extent_num;
	wxUint8 reserved[2];
	wxUint8 record_num;
	union {
		wxUint8  b[16];
		wxUint16 w[8];
	} map; // array of allocation block number
} directory_cpm_t;

/// ディレクトリエントリ TF-DOS (16bytes)
typedef struct st_directory_tfdos {
	wxUint8  type;
	wxUint8  name[8];	// file name ends with $0D and fills rest with $20
	wxUint16 file_size;
	wxUint16 load_addr;
	wxUint16 exec_addr;
	wxUint8  track;
} directory_tfdos_t;

/// ディレクトリエントリ C-DOS (32bytes)
typedef struct st_directory_cdos {
	wxUint8  type;
	wxUint8  name[17];	// file name ends with $0D
	wxUint8  type2;
	wxUint8  byte_order;
	wxUint16 file_size;
	wxUint16 load_addr;
	wxUint16 exec_addr;
	wxUint8  yy;
	wxUint8  mm;
	wxUint8  dd;
	wxUint8  reserved2;
	wxUint8  track;
	wxUint8  sector;
} directory_cdos_t;

/// ディレクトリエントリ サイズに注意！
typedef union un_directory {
	wxUint8				name[16];
	directory_l3_2d_t	l3_2d;
	directory_fat8f_t	fat8f;
	directory_msdos_t	msdos;
	directory_n88_t		n88;
	directory_x1_hu_t	x1hu;
	directory_mz_t		mz;
	directory_flex_t	flex;
	directory_os9_t		os9;
	directory_cpm_t		cpm;
	directory_tfdos_t	tfdos;
	directory_cdos_t	cdos;
} directory_t;
#pragma pack()

/// DISK BASIC種類 番号
enum DiskBasicFormatType {
	FORMAT_TYPE_NONE	= -1,
	FORMAT_TYPE_L3_1S	= 0,
	FORMAT_TYPE_L3S1_2D = 1,
	FORMAT_TYPE_FM		= 2,
	FORMAT_TYPE_MSDOS	= 3,
	FORMAT_TYPE_MSX		= 4,
	FORMAT_TYPE_N88		= 5,
	FORMAT_TYPE_X1HU	= 6,
	FORMAT_TYPE_MZ		= 7,
	FORMAT_TYPE_FLEX	= 8,
	FORMAT_TYPE_OS9		= 9,
	FORMAT_TYPE_CPM		= 10,
	FORMAT_TYPE_TFDOS	= 71,
	FORMAT_TYPE_CDOS	= 72,
};

/// 属性保存クラス
class DiskBasicFileType
{
private:
	DiskBasicFormatType format;	///< DISK BASIC種類
	int type;					///< 共通属性 enum #en_file_type_mask の値の組み合わせ
	int origin;					///< 本来の属性

public:
	DiskBasicFileType();
	DiskBasicFileType(DiskBasicFormatType n_format, int n_type, int n_origin = 0);
	~DiskBasicFileType();

	DiskBasicFormatType GetFormat() const { return format; }
	void SetFormat(DiskBasicFormatType val) { format = val; }
	int GetType() const { return type; }
	void SetType(int val) { type = val; }
	int GetOrigin() const { return origin; }
	void SetOrigin(int val) { origin = val; }

	bool MatchType(int mask, int value) const;
	bool UnmatchType(int mask, int value) const;

	bool IsAscii() const;
	bool IsVolume() const;
	bool IsDirectory() const;
};

//
//
//

enum en_fat_availability {
	FAT_AVAIL_FREE = 0,
	FAT_AVAIL_SYSTEM,
	FAT_AVAIL_USED,
	FAT_AVAIL_USED_LAST,
	FAT_AVAIL_MISSING
};

#endif /* _BASICCOMMON_H_ */
