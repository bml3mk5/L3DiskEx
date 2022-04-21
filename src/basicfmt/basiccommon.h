/// @file basiccommon.h
///
/// @brief disk basic common
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICCOMMON_H_
#define _BASICCOMMON_H_


#include "../common.h"
#include <wx/string.h>
#include <wx/dynarray.h>


//////////////////////////////////////////////////////////////////////

#define FILENAME_BUFSIZE	(32)
#define FILEEXT_BUFSIZE		(4)

//////////////////////////////////////////////////////////////////////

/// @brief 共通属性フラグ
enum en_file_type_mask {
	FILE_TYPE_BASIC_MASK	 = 0x000001,
	FILE_TYPE_DATA_MASK		 = 0x000002,
	FILE_TYPE_MACHINE_MASK	 = 0x000004,
	FILE_TYPE_ASCII_MASK	 = 0x000008,
	FILE_TYPE_BINARY_MASK	 = 0x000010,
	FILE_TYPE_RANDOM_MASK	 = 0x000020,
	FILE_TYPE_ENCRYPTED_MASK = 0x000040,
	FILE_TYPE_READWRITE_MASK = 0x000080,
	FILE_TYPE_READONLY_MASK	 = 0x000100,
	FILE_TYPE_HIDDEN_MASK	 = 0x000200,
	FILE_TYPE_SYSTEM_MASK	 = 0x000400,
	FILE_TYPE_VOLUME_MASK	 = 0x000800,
	FILE_TYPE_DIRECTORY_MASK = 0x001000,
	FILE_TYPE_ARCHIVE_MASK	 = 0x002000,
	FILE_TYPE_LIBRARY_MASK	 = 0x004000,
	FILE_TYPE_NONSHARE_MASK	 = 0x008000,
	FILE_TYPE_UNDELETE_MASK	 = 0x010000,
	FILE_TYPE_WRITEONLY_MASK = 0x020000,
	FILE_TYPE_TEMPORARY_MASK = 0x040000,
	FILE_TYPE_INTEGER_MASK	 = 0x080000,
	FILE_TYPE_HARDLINK_MASK	 = 0x100000,
	FILE_TYPE_SOFTLINK_MASK	 = 0x200000,

	FILE_TYPE_EXTENSION_MASK = FILE_TYPE_BASIC_MASK
		| FILE_TYPE_DATA_MASK
		| FILE_TYPE_MACHINE_MASK
		| FILE_TYPE_ASCII_MASK
		| FILE_TYPE_BINARY_MASK
		| FILE_TYPE_RANDOM_MASK
		| FILE_TYPE_INTEGER_MASK,
};
/// @brief 共通属性フラグ位置
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
	FILE_TYPE_HIDDEN_POS	= 9,
	FILE_TYPE_SYSTEM_POS	= 10,
	FILE_TYPE_VOLUME_POS	= 11,
	FILE_TYPE_DIRECTORY_POS = 12,
	FILE_TYPE_ARCHIVE_POS	= 13,
	FILE_TYPE_LIBRARY_POS	= 14,
	FILE_TYPE_NONSHARE_POS	= 15,
	FILE_TYPE_UNDELETE_POS	= 16,
	FILE_TYPE_WRITEONLY_POS	= 17,
	FILE_TYPE_TEMPORARY_POS	= 18,
	FILE_TYPE_INTEGER_POS	= 19,
	FILE_TYPE_HARDLINK_POS	= 20,
	FILE_TYPE_SOFTLINK_POS	= 21,
};

//////////////////////////////////////////////////////////////////////
//
// ディレクトリ
//
//////////////////////////////////////////////////////////////////////

#pragma pack(1)
/// @brief ディレクトリエントリ L3,S1 ５インチ,８インチ(倍密度)
typedef struct st_directory_l3_2d {
	wxUint8  name[8];
	wxUint8  ext[3];
	wxUint8  type;
	wxUint8  type2;
	wxUint8  start_group;
	wxUint16 end_bytes;	///< used size of end cluster (big endien)

	char reserved[16];
} directory_l3_2d_t;

/// @brief ディレクトリエントリ L3 ３インチ(単密度) / F-BASIC 倍密度
typedef struct st_directory_fat8f {
	wxUint8 name[8];
	wxUint8 ext[3];	/// not used.
	wxUint8 type;
	wxUint8 type2;
	wxUint8 type3;
	wxUint8 start_group;

	char reserved[17];
} directory_fat8f_t;

/// @brief ディレクトリエントリ n88 BASIC (16bytes)
typedef struct st_directory_n88 {
	wxUint8 name[6];
	wxUint8 ext[3];
	wxUint8 type;
	wxUint8 start_group;
	char reserved[5];
} directory_n88_t;

/// @brief ディレクトリエントリ X1 Hu-BASIC (32bytes)
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

/// @brief ディレクトリエントリ MZ DISK BASIC (32bytes)
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

/// @brief ディレクトリエントリ MS-DOS FAT (32bytes)
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

/// @brief ディレクトリエントリ MS-DOS LFN (32bytes)
typedef struct st_directory_ms_lfn {
	wxUint8 order;
	wxUint8 name[10];
	wxUint8 type;
	wxUint8 type2;
	wxUint8 chksum;
	wxUint8 name2[12];
	wxUint16 dummy_group;
	wxUint8 name3[4];
} directory_ms_lfn_t;

/// @brief ディレクトリエントリ Human68K (MS-DOS compatible) (32bytes)
typedef struct st_directory_hu68k {
	wxUint8 name[8];
	wxUint8 ext[3];
	wxUint8 type;
	wxUint8 name2[10];
	wxUint16 wtime;
	wxUint16 wdate;
	wxUint16 start_group;
	wxUint32 file_size;
} directory_hu68k_t;

/// @brief ディレクトリエントリ L-os Angeles (MS-DOS compatible) (32bytes)
typedef struct st_directory_losa {
	wxUint8 name[8];
	wxUint8 ext[3];
	wxUint8 type;
	wxUint8 start_addr[4];
	wxUint8 binary_type;
	wxUint8 exec_addr[4];
	wxUint8 reserved;
	wxUint16 wtime;
	wxUint16 wdate;
	wxUint16 start_group;
	wxUint32 file_size;
} directory_losa_t;

/// @brief ディレクトリエントリ MS-DOS compatible (32bytes)
typedef union un_directory_ms {
	directory_msdos_t  msdos;
	directory_ms_lfn_t mslfn;
	directory_hu68k_t  hu68k;
	directory_losa_t   losa;
} directory_ms_t;

/// @brief ディレクトリエントリ FLEX (24bytes)
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

/// @brief FLEX top of each sector
typedef struct st_flex_ptr {
	wxUint8  next_track;
	wxUint8  next_sector;
	wxUint16 seq_num;
} flex_ptr_t;

/// @brief OS-9 LSN
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

/// @brief OS-9 Segment
typedef struct st_os9_segment {
	os9_lsn_t LSN;
	wxUint16  SIZ;
} os9_segment_t;

/// @brief OS-9 Date Format
typedef struct st_os9_date {
	wxUint8 yy;
	wxUint8 mm;
	wxUint8 dd;
	wxUint8 hh;
	wxUint8 mi;
} os9_date_t;

/// @brief OS-9 Created Date
typedef struct st_os9_cdate {
	wxUint8 yy;
	wxUint8 mm;
	wxUint8 dd;
} os9_cdate_t;

/// @brief ディレクトリエントリ OS-9 (32bytes)
typedef struct st_directory_os9 {
	wxUint8		DE_NAM[28];
	wxUint8		DE_Reserved;
	os9_lsn_t	DE_LSN;	// link to FD
} directory_os9_t;

/// @brief OS-9 File Descriptor
typedef struct st_directory_os9_fd {
	wxUint8		FD_ATT;	// attr
	wxUint16	FD_OWN;	// owner id
	os9_date_t	FD_DAT;	// date
	wxUint8		FD_LNK;	// link count
	wxUint32	FD_SIZ;	// in bytes
	os9_cdate_t	FD_DCR;	// created date
	os9_segment_t FD_SEG[48];	// 5*48=240
} directory_os9_fd_t;

/// @brief ディレクトリエントリ CP/M (32bytes)
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

/// @brief ディレクトリエントリ TF-DOS (16bytes)
typedef struct st_directory_tfdos {
	wxUint8  type;
	wxUint8  name[8];	// file name ends with $0D and fills rest with $20
	wxUint16 file_size;
	wxUint16 load_addr;
	wxUint16 exec_addr;
	wxUint8  track;
} directory_tfdos_t;

/// @brief ディレクトリエントリ PC-8001 DOS (New PC.DOS) (16bytes)
typedef struct st_directory_dos80 {
	wxUint8 name[16];
} directory_dos80_t;

/// @brief PC-8001 DOS (New PC.DOS) グループエントリ
typedef struct st_directory_dos80_grp {
	wxUint8  g;
	wxUint16 a;
} directory_dos80_grp_t;

/// @brief ディレクトリエントリ2 PC-8001 DOS (New PC.DOS) (16bytes)
typedef struct st_directory_dos80_2 {
	directory_dos80_grp_t grps[5];
	wxUint8 reserved[1];
} directory_dos80_2_t;

/// @brief ディレクトリエントリ C-DOS (32bytes)
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

/// @brief ディレクトリエントリ MZ Floppy DOS (64bytes)
typedef struct st_directory_mz_fdos {
	wxUint8  type;
	wxUint8  name[17];	// file name has $0D on the end of string
	wxUint16 file_size;
	wxUint16 load_addr;
	wxUint16 exec_addr;
	wxUint16 groups;
	wxUint8  attr[2];		// 0x30 0x53
	wxUint8  password[2];	// 0x00 0x00
	wxUint16 dummy_sector;	// 0x01 0x02

	wxUint8  mmddyy[7];
	wxUint8  reserved2[13];
	wxUint8  track;
	wxUint8  sector;
	wxUint8  reserved3[5];
	wxUint8  seq_num;
	wxUint8  unknown1;		// 0x9x - 0xax
	wxUint8  unknown2;		// 0x15
	wxUint8  data_track;
	wxUint8  data_sector;
} directory_mz_fdos_t;

/// @brief ディレクトリエントリ Frost-DOS (16bytes)
typedef struct st_directory_frost {
	wxUint8  name[6];
	wxUint8  ext[3];
	wxUint8  type;
	wxUint8  track;
	wxUint8  sector;
	wxUint16 load_addr;
	wxUint16 size;
} directory_frost_t;

/// @brief X-DOSセグメント情報
typedef struct st_xdos_seg {
	wxUint8 track;
	wxUint8 sector;
	wxUint8 size;
} xdos_seg_t;

/// @brief ディレクトリエントリ X-DOS X1 (32bytes)
typedef struct st_directory_xdos {
	wxUint16 ftype;		// big endien
	wxUint8  name[16];
	wxUint16 load_addr;
	wxUint16 file_size;
	wxUint16 exec_addr;
	wxUint16 date;
	wxUint16 time;
	wxUint8  attr;		// アトリビュート
	xdos_seg_t start;
} directory_xdos_t;

/// @brief Magical DOS セグメント情報
typedef struct st_magical_seg {
	wxUint8  track;
	wxUint8  sector;
	wxUint8  size;
} magical_seg_t;

/// @brief ディレクトリエントリ Magical DOS
typedef struct st_directory_magical {
	wxUint8  type;
	wxUint8  name[31];
	wxUint8  type2;
	wxUint16 load_addr;
	wxUint16 file_size;
	wxUint16 exec_addr;
	wxUint8  date[2];
	wxUint8  time[2];
	wxUint8  reserved[2];
	magical_seg_t start;
} directory_magical_t;

/// @brief ディレクトリエントリ S-DOS (32bytes)
typedef struct st_directory_sdos {
	wxUint8  name[22];
	wxUint8  type;
	wxUint8  track;
	wxUint8  sector;
	wxUint8  size;		// number of sector
	wxUint8  rest_size;
	wxUint16 load_addr;
	wxUint16 exec_addr;
	wxUint8  reserved;
} directory_sdos_t;

/// @brief ディレクトリエントリ C82-BASIC (32bytes)
typedef struct st_directory_fp {
	wxUint8  type;
	wxUint8  name[8];
	wxUint8  ext[3];
	wxUint8  term;
	wxUint8  unknown[7];
	wxUint16 load_addr;
	wxUint16 end_addr;
	wxUint16 exec_addr;
	wxUint16 file_size;
	wxUint8  start_group;
	wxUint8  attr;
	wxUint8  reserved[2];
} directory_fp_t;

/// @brief ディレクトリエントリ MDOS (16bytes)
typedef struct st_directory_mdos {
	wxUint8  name[8];
	wxUint8  ext[3];
	wxUint8  unknown;
	wxUint16 start_group;	// little endien
	wxUint16 file_size;		// big endien
} directory_mdos_t;

/// @brief ディレクトリエントリ Falcom (16bytes)
typedef struct st_directory_falcom {
	wxUint8  name[6];
	wxUint16 exec_addr;
	wxUint16 start_addr;
	wxUint16 end_addr;
	struct {
		wxUint8 track;
		wxUint8 sector;
	} start_group;
	struct {
		wxUint8 track;
		wxUint8 sector;
	} end_group;
} directory_falcom_t;

/// @brief ディレクトリエントリ Apple DOS (35bytes)
typedef struct st_directory_apledos {
	wxUint8 track;
	wxUint8 sector;
	wxUint8 type;
	wxUint8 name[30];
	wxUint16 sector_count;	// size (little endien)
} directory_apledos_t;

/// @brief Apple DOS top of each sector
typedef struct st_apledos_ptr {
	wxUint8  reserved;
	wxUint8  next_track;
	wxUint8  next_sector;
} apledos_ptr_t;

/// @brief ディレクトリエントリ Apple ProDOS (39bytes)
typedef struct st_directory_prodos {
	wxUint8  stype_and_nlen;
	wxUint8  name[15];
	wxUint8  file_type;			// file only
	wxUint16 key_pointer;		// file only
	wxUint16 blocks_used;		// file only
	wxUint8  eof[3];			// file only
	wxUint8  cdate[2];
	wxUint8  ctime[2];
	wxUint8  version;
	wxUint8  min_version;
	wxUint8  access;
	union {
		struct {
			wxUint8  entry_len;
			wxUint8  entries_per_block;
			wxUint16 file_count;
			wxUint16 bitmap_pointer;
			wxUint16 total_blocks;
		} v;
		struct {
			wxUint8  entry_len;
			wxUint8  entries_per_block;
			wxUint16 file_count;
			wxUint16 parent_pointer;
			wxUint8  parent_entry;
			wxUint8  parent_entry_len;
		} sv;
		struct {
			wxUint16 aux_type;			// aux type
			wxUint8  mdate[2];
			wxUint8  mtime[2];
			wxUint16 header_pointer;
		} f;
	};
} directory_prodos_t;

/// @brief トラック＆セクタ Commodore 1541
typedef struct st_c1541_ptr {
	wxUint8  track;
	wxUint8  sector;
} c1541_ptr_t;

/// @brief ディレクトリエントリ Commodore 1541 (32bytes)
typedef struct st_directory_c1541 {
	wxUint8  do_not_write[2];	// first entry only on each sector
	wxUint8  type;
	c1541_ptr_t first_data;
	wxUint8  name[16];
	c1541_ptr_t first_side;		// relative file only
	wxUint8  record_size;		// relative file only
	wxUint8  unused[4];
	c1541_ptr_t replace;
	wxUint16 num_of_blocks;
} directory_c1541_t;

/// @brief Amiga block structure head (all Big Endien)
typedef struct st_block_pre {
	wxUint32 type;				///< starting block (T_HEADER:2 / T_LIST:16) 
	wxUint32 header_key;		///< self pointer (except Root)
	wxUint32 high_seq;			///< number of data (File only)
	wxUint32 table_size;		///< hash table size (Root only) (72)
	wxUint32 first_data;		///< first data block pointer (File only)
	wxUint32 check_sum;
	union {
		wxUint32 table[1];		///< block pointer (72 items)
		wxUint8  sym_name[4];	///< symbolic name (Soft link only)
	} u;
} amiga_block_pre_t;

/// @brief Amiga Root Block (above hash_table)
typedef struct st_amiga_root_block_post {
	wxUint32 bm_flag;			///< value is -1 if disk bitmap is valid
	wxUint32 bm_pages[25];		///< blocks of disk bitmap
	wxUint32 bm_ext;			///< ext blocks of disk bitmap
	wxUint32 r_days;			///< root dir modified date
	wxUint32 r_mins;			///< root dir modified time
	wxUint32 r_ticks;			///< root dir modified seconds
	wxUint8  disk_name_len;		///< in bytes
	wxUint8  disk_name[31];
	wxUint32 unused[2];			///< set to 0
	wxUint32 v_days;			///< disk modified date
	wxUint32 v_mins;			///< disk modified time
	wxUint32 v_ticks;			///< disk modified seconds
	wxUint32 c_days;			///< disk creation date
	wxUint32 c_mins;			///< disk creation time
	wxUint32 c_ticks;			///< disk creation seconds
	wxUint32 next_hash;			///< always 0
	wxUint32 parent_dir;		///< always 0
	wxUint32 extension;			///< always 0
	wxUint32 sec_type;			///< always 1
} amiga_root_block_post_t;

/// @brief Amiga File / Directory Header Block (post table)
typedef struct st_amiga_header_post {
	wxUint8  unused0[4];
	wxUint16 uid;				///< user id
	wxUint16 gid;				///< user group
	wxUint32 protect;
	wxUint32 byte_size;			///< file size (File only)
	wxUint8  comment_len;		///< in bytes
	wxUint8  comment[79];
	wxUint8  unused1[12];
	wxUint32 days;				///< modified date
	wxUint32 mins;				///< modified time
	wxUint32 ticks;				///< modified seconds
	wxUint8  name_len;			///< in bytes
	wxUint8  name[31];			///< 0 terminate
	wxUint8  unused2[4];
	wxUint32 real_entry;		///< FFS unused (File only)
	wxUint32 next_link;			///< FFS hardlinks chained list
	wxUint8  unused3[20];
	wxUint32 hash_chain;		///< next entry with same hash
	wxUint32 parent_dir;
	wxUint32 extension;			///< 1st extension block / FFS: cache block
	wxUint32 sec_type;			///< -2 (ST_USERDIR) / -3 (ST_FILE) / -4 (ST_LINKFILE) / 4 (ST_LINKDIR) / 3 (ST_SOFTLINK)
} amiga_header_post_t;

/// @brief Amiga block structure post table
typedef union un_amiga_block_post {
	amiga_root_block_post_t r;
	amiga_header_post_t     h;
} amiga_block_post_t;

/// @brief ディレクトリエントリ Amiga DOS
///
/// AmigaDOSは1セクタ分になるのでブロック番号だけを保持
typedef struct st_directory_amiga {
	wxUint32            block_num;
	amiga_block_pre_t  *pre;
	amiga_block_post_t *post;
} directory_amiga_t;

/// @brief ディレクトリエントリ サイズに注意！
typedef union un_directory {
	wxUint8				name[16];
	directory_l3_2d_t	l3_2d;
	directory_fat8f_t	fat8f;
	directory_msdos_t	msdos;
	directory_ms_lfn_t	mslfn;
	directory_n88_t		n88;
	directory_x1_hu_t	x1hu;
	directory_mz_t		mz;
	directory_flex_t	flex;
	directory_os9_t		os9;
	directory_cpm_t		cpm;
	directory_tfdos_t	tfdos;
	directory_dos80_t	dos80;
	directory_frost_t	frost;
	directory_magical_t magical;
	directory_sdos_t	sdos;
	directory_fp_t		fp;
	directory_xdos_t	xdos;
	directory_cdos_t	cdos;
	directory_mz_fdos_t	fdos;
	directory_hu68k_t	hu68k;
	directory_losa_t	losa;
	directory_mdos_t	mdos;
	directory_falcom_t  falcom;
	directory_apledos_t	apledos;
	directory_prodos_t	prodos;
	directory_c1541_t	c1541;
	directory_amiga_t	amiga;
} directory_t;
#pragma pack()

/// @brief DISK BASIC種類 番号
enum DiskBasicFormatType {
	FORMAT_TYPE_UNKNOWN	= -1,
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
	FORMAT_TYPE_PA		= 11,
	FORMAT_TYPE_SMC		= 12,
	FORMAT_TYPE_FP		= 13,
	FORMAT_TYPE_HU68K	= 14,
	FORMAT_TYPE_APLEDOS	= 15,
	FORMAT_TYPE_PRODOS	= 16,
	FORMAT_TYPE_C1541	= 20,
	FORMAT_TYPE_AMIGA	= 21,
	FORMAT_TYPE_LOSA	= 31,
	FORMAT_TYPE_CDOS2	= 32,
	FORMAT_TYPE_DOS80	= 51,
	FORMAT_TYPE_FROST	= 52,
	FORMAT_TYPE_MAGICAL	= 53,
	FORMAT_TYPE_SDOS	= 54,
	FORMAT_TYPE_MDOS	= 55,
	FORMAT_TYPE_XDOS	= 61,
	FORMAT_TYPE_TFDOS	= 71,
	FORMAT_TYPE_CDOS	= 72,
	FORMAT_TYPE_MZ_FDOS	= 73,
	FORMAT_TYPE_FALCOM	= 91,
};

//////////////////////////////////////////////////////////////////////

/// @brief 名前と値 定数リスト用
typedef struct st_name_value {
	const char *name;
	int        value;

	/// @brief 名前が一致するか
	static int IndexOf(const struct st_name_value *list, const wxString &str);
	/// @brief 値が一致するか
	static int IndexOf(const struct st_name_value *list, int val);
} name_value_t;

//////////////////////////////////////////////////////////////////////

/// @brief 値と値 定数リスト用
typedef struct st_value_value {
	int com_value;
	int ori_value;
} value_value_t;

//////////////////////////////////////////////////////////////////////

/// @brief ファイルプロパティでファイル名変更した時に渡す値
class DiskBasicFileName
{
private:
	wxString	name;		///< ファイル名
	int			optional;	///< 拡張属性 ファイル名が同じでも、この属性が異なれば違うファイルとして扱う

public:
	DiskBasicFileName();
	DiskBasicFileName(const wxString &n_name, int n_optional = 0);
	~DiskBasicFileName();

	/// @brief ファイル名
	const wxString &GetName() const { return name; }
	/// @brief ファイル名
	wxString &GetName() { return name; }
	/// @brief ファイル名
	void SetName(const wxString &val) { name = val; }
	/// @brief 拡張属性 ファイル名が同じでも、この属性が異なれば違うファイルとして扱う
	int GetOptional() const { return optional; }
	/// @brief 拡張属性 ファイル名が同じでも、この属性が異なれば違うファイルとして扱う
	void SetOptional(int val) { optional = val; }
};

//////////////////////////////////////////////////////////////////////

/// @brief 属性保存クラス
class DiskBasicFileType
{
private:
	DiskBasicFormatType format;	///< DISK BASIC種類
	int type;					///< 共通属性 enum #en_file_type_mask の値の組み合わせ
	int origin[3];				///< 本来の属性

public:
	DiskBasicFileType();
	DiskBasicFileType(DiskBasicFormatType n_format, int n_type, int n_origin0 = 0, int n_origin1 = 0, int n_origin2 = 0);
	~DiskBasicFileType();

	/// @brief DISK BASIC種類
	DiskBasicFormatType GetFormat() const { return format; }
	/// @brief DISK BASIC種類
	void SetFormat(DiskBasicFormatType val) { format = val; }
	/// @brief 共通属性 enum #en_file_type_mask の値の組み合わせ
	int GetType() const { return type; }
	/// @brief 共通属性 enum #en_file_type_mask の値の組み合わせ
	void SetType(int val) { type = val; }
	/// @brief 本来の属性
	int GetOrigin(int idx = 0) const { return origin[idx]; }
	/// @brief 本来の属性
	void SetOrigin(int val) { origin[0] = val; }
	/// @brief 本来の属性
	void SetOrigin(int idx, int val) { origin[idx] = val; }

	/// @brief 共通属性が一致するか
	bool MatchType(int mask, int value) const;
	/// @brief 共通属性が一致しないか
	bool UnmatchType(int mask, int value) const;

	/// @brief 共通属性がアスキー属性か
	bool IsAscii() const;
	/// @brief 共通属性がボリューム属性か
	bool IsVolume() const;
	/// @brief 共通属性がディレクトリ属性か
	bool IsDirectory() const;
};

//////////////////////////////////////////////////////////////////////

/// @brief グループ番号に対応する機種依存データを保持
///
/// @sa DiskBasicGroupItem
class DiskBasicGroupUserData
{
public:
	DiskBasicGroupUserData() {}
	virtual ~DiskBasicGroupUserData() {}
	virtual DiskBasicGroupUserData *Clone() const { return new DiskBasicGroupUserData(*this); }
};

//////////////////////////////////////////////////////////////////////

/// @brief グループ番号に対応するパラメータを保持
///
/// @sa DiskBasicGroups
class DiskBasicGroupItem
{
public:
	wxUint32 group;			///< グループ番号
	wxUint32 next;			///< 次のグループ番号
	int track;				///< トラック番号
	int side;				///< サイド番号
	int sector_start;		///< グループ内の開始セクタ番号
	int sector_end;			///< グループ内の終了セクタ番号
	int div_num;			///< １グループがセクタ内に複数ある時の分割位置
	int div_nums;			///< １グループがセクタ内に複数ある時の分割数
	DiskBasicGroupUserData *user_data;	///< 機種依存データ
public:
	DiskBasicGroupItem();
	DiskBasicGroupItem(const DiskBasicGroupItem &);
	DiskBasicGroupItem(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div = 0, int n_divs = 1, DiskBasicGroupUserData *n_user = NULL);
	DiskBasicGroupItem(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, DiskBasicGroupUserData *n_user = NULL);
	~DiskBasicGroupItem();
	/// @brief 代入
	DiskBasicGroupItem &operator=(const DiskBasicGroupItem &);
	/// @brief データセット
	void Set(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div = 0, int n_divs = 1, DiskBasicGroupUserData *n_user = NULL);
	/// @brief データセット
	void Set(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, DiskBasicGroupUserData *n_user = NULL);
	/// @brief 比較
	static int Compare(DiskBasicGroupItem **item1, DiskBasicGroupItem **item2);
};

/// @class DiskBasicGroupItems
///
/// @brief グループ番号 DiskBasicGroupItem のリスト
WX_DECLARE_OBJARRAY(DiskBasicGroupItem, DiskBasicGroupItems);

//////////////////////////////////////////////////////////////////////

/// @brief グループ番号のリストを保持
///
/// ディスク内ファイルのチェインをこのリストに保持する
///
/// @sa DiskBasicGroupItem , DiskBasicDirItem
class DiskBasicGroups
{
private:
	DiskBasicGroupItems	items;			///< グループ番号のリスト
	int					nums;			///< グループ数
	size_t				size;			///< グループ内の占有サイズ
	size_t				size_per_group;	///< １グループのサイズ

public:
	DiskBasicGroups();
	~DiskBasicGroups() {}

	/// @brief 追加
	void	Add(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div = 0, int n_divs = 1, DiskBasicGroupUserData *n_user = NULL);
	/// @brief 追加
	void	Add(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, DiskBasicGroupUserData *n_user = NULL);
	/// @brief 追加
	void	Add(const DiskBasicGroupItem &n_item);
	/// @brief 追加
	void	Add(const DiskBasicGroups &n_items);
	/// @brief リストをクリア
	void	Empty();
	/// @brief リストの数を返す
	size_t	Count() const;
	/// @brief リストの最後を返す
	DiskBasicGroupItem &Last() const;
	/// @brief リストアイテムを返す
	DiskBasicGroupItem &Item(size_t idx) const;
	/// @brief リストアイテムを返す
	DiskBasicGroupItem *ItemPtr(size_t idx) const;
	/// @brief リストを返す
	const DiskBasicGroupItems &GetItems() const { return items; }

	/// @brief グループ数を返す
	int		GetNums() const { return nums; }
	/// @brief 占有サイズを返す
	size_t	GetSize() const { return size; }
	/// @brief １グループのサイズを返す
	size_t	GetSizePerGroup() const { return size_per_group; }

	/// @brief グループ数を設定
	void	SetNums(int val) { nums = val; }
	/// @brief 占有サイズを設定
	void	SetSize(size_t val) { size = val; }
	/// @brief １グループのサイズを設定
	void	SetSizePerGroup(size_t val) { size_per_group = val; }

	/// @brief グループ数を足す
	int		AddNums(int val);
	/// @brief 占有サイズを足す
	int		AddSize(int val);

	/// @brief グループ番号でソート
	void SortItems();
};

//////////////////////////////////////////////////////////////////////

/// @brief 汎用リスト用アイテム
///
/// @sa KeyValArray
class KeyValItem
{
public:
	wxString m_key;
	wxUint8 *m_value;
	size_t   m_size;
	enum en_type {
		TYPE_UNKNOWN = 0,
		TYPE_INTEGER,
		TYPE_UINT8,
		TYPE_UINT16,
		TYPE_UINT32,
		TYPE_STRING,
		TYPE_BOOL
	} m_type;
public:
	KeyValItem();
	KeyValItem(const wxString &key, int val);
	KeyValItem(const wxString &key, wxUint8 val, bool invert = false);
	KeyValItem(const wxString &key, wxUint16 val, bool big_endien = false, bool invert = false);
	KeyValItem(const wxString &key, wxUint32 val, bool big_endien = false, bool invert = false);
	KeyValItem(const wxString &key, const void *val, size_t size, bool invert = false);
	KeyValItem(const wxString &key, bool val);
	~KeyValItem();

	void Clear();
	void Set(const wxString &key, int val);
	void Set(const wxString &key, wxUint8 val, bool invert = false);
	void Set(const wxString &key, wxUint16 val, bool big_endien = false, bool invert = false);
	void Set(const wxString &key, wxUint32 val, bool big_endien = false, bool invert = false);
	void Set(const wxString &key, const void *val, size_t size, bool invert = false);
	void Set(const wxString &key, bool val);

	wxString GetValueString() const;

	const wxString &Key() const { return m_key; }

	static int Compare(KeyValItem **item1, KeyValItem **item2);
};

WX_DEFINE_ARRAY(KeyValItem *, ArrayOfKeyValItem);

/// @brief 汎用リスト KeyValItem の配列
class KeyValArray : public ArrayOfKeyValItem
{
private:
	void DeleteAll();

public:
	KeyValArray();
	~KeyValArray();

	void Clear();
	void Empty();
	void Add(const wxString &key, int val);
	void Add(const wxString &key, wxUint8 val, bool invert = false);
	void Add(const wxString &key, wxUint16 val, bool big_endien = false, bool invert = false);
	void Add(const wxString &key, wxUint32 val, bool big_endien = false, bool invert = false);
	void Add(const wxString &key, const void *val, size_t size, bool invert = false);
	void Add(const wxString &key, bool val);
};

#endif /* _BASICCOMMON_H_ */
