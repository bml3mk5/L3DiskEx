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
#include <wx/dynarray.h>

#define FILENAME_BUFSIZE	(32)
#define FILEEXT_BUFSIZE		(4)

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
	FILE_TYPE_LIBRARY_MASK	 = 0x04000,
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
	FILE_TYPE_LIBRARY_POS	= 14,
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

/// ディレクトリエントリ X1 Hu-BASIC (32bytes)
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

/// ディレクトリエントリ MZ DISK BASIC (32bytes)
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

/// ディレクトリエントリ MS-DOS FAT (32bytes)
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

/// ディレクトリエントリ MS-DOS LFN (32bytes)
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

/// ディレクトリエントリ Human68K (MS-DOS compatible) (32bytes)
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

/// ディレクトリエントリ L-os Angeles (MS-DOS compatible) (32bytes)
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

/// ディレクトリエントリ FLEX (24bytes)
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

/// ディレクトリエントリ OS-9 (32bytes)
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

/// ディレクトリエントリ CP/M (32bytes)
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

/// ディレクトリエントリ PC-8001 DOS (New PC.DOS) (16bytes)
typedef struct st_directory_dos80 {
	wxUint8 name[16];
} directory_dos80_t;

/// PC-8001 DOS (New PC.DOS) グループエントリ
typedef struct st_directory_dos80_grp {
	wxUint8  g;
	wxUint16 a;
} directory_dos80_grp_t;

/// ディレクトリエントリ2 PC-8001 DOS (New PC.DOS) (16bytes)
typedef struct st_directory_dos80_2 {
	directory_dos80_grp_t grps[5];
	wxUint8 reserved[1];
} directory_dos80_2_t;

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

/// ディレクトリエントリ MZ Floppy DOS (64bytes)
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

/// ディレクトリエントリ Frost-DOS (16bytes)
typedef struct st_directory_frost {
	wxUint8  name[6];
	wxUint8  ext[3];
	wxUint8  type;
	wxUint8  track;
	wxUint8  sector;
	wxUint16 load_addr;
	wxUint16 size;
} directory_frost_t;

/// X-DOSセグメント情報
typedef struct st_xdos_seg {
	wxUint8 track;
	wxUint8 sector;
	wxUint8 size;
} xdos_seg_t;

/// ディレクトリエントリ X-DOS X1 (32bytes)
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

/// Magical DOS セグメント情報
typedef struct st_magical_seg {
	wxUint8  track;
	wxUint8  sector;
	wxUint8  size;
} magical_seg_t;

/// ディレクトリエントリ parts Magical DOS
struct st_directory_magical_p0 {
	wxUint8  type;
	wxUint8  name[15];
};
struct st_directory_magical_p1 {
	wxUint8  name[16];
};
struct st_directory_magical_p2 {
	wxUint8  type2;
	wxUint16 load_addr;
	wxUint16 file_size;
	wxUint16 exec_addr;
	wxUint8  date[2];
	wxUint8  time[2];
	wxUint8  reserved[2];
	magical_seg_t start;
};

/// ディレクトリエントリ Magical DOS (48bytes)
struct st_directory_magical {
	struct st_directory_magical_p0 p0;
	struct st_directory_magical_p1 p1;
	struct st_directory_magical_p2 p2;
};

/// Magical DOSのエントリはセクタをまたぐので
/// ポインタでアクセス可能にする
typedef union un_directory_magical {
	struct st_directory_magical s;
	wxUint8 p[3][16];
} directory_magical_t;

/// ディレクトリエントリ S-DOS (32bytes)
struct st_directory_sdos {
	wxUint8  name[22];
	wxUint8  type;
	wxUint8  track;
	wxUint8  sector;
	wxUint8  size;		// number of sector
	wxUint8  rest_size;
	wxUint16 load_addr;
	wxUint16 exec_addr;
	wxUint8  reserved;
};

typedef union un_directory_sdos {
	struct st_directory_sdos s;
	wxUint8 p[32];
} directory_sdos_t;

/// ディレクトリエントリ C82-BASIC (32bytes)
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

/// ディレクトリエントリ MDOS (16bytes)
typedef struct st_directory_mdos {
	wxUint8  name[8];
	wxUint8  ext[3];
	wxUint8  unknown;
	wxUint16 start_group;	// little endien
	wxUint16 file_size;		// big endien
} directory_mdos_t;

/// ディレクトリエントリ Falcom (16bytes)
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

/// ディレクトリエントリ サイズに注意！
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
} directory_t;
#pragma pack()

/// DISK BASIC種類 番号
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

/// ファイルプロパティでファイル名変更した時に渡す値
class DiskBasicFileName
{
private:
	wxString	name;		///< ファイル名
	int			optional;	///< 拡張属性 ファイル名が同じでも、この属性が異なれば違うファイルとして扱う

public:
	DiskBasicFileName();
	DiskBasicFileName(const wxString &n_name, int n_optional = 0);
	~DiskBasicFileName();

	const wxString &GetName() const { return name; }
	wxString &GetName() { return name; }
	void SetName(const wxString &val) { name = val; }
	int GetOptional() const { return optional; }
	void SetOptional(int val) { optional = val; }
};

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

/// グループ番号に対応するパラメータを保持
class DiskBasicGroupItem
{
public:
	wxUint32 group;
	wxUint32 next;
	int track;
	int side;
	int sector_start;
	int sector_end;
	int div_num;
	int div_nums;
public:
	DiskBasicGroupItem();
	DiskBasicGroupItem(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div = 0, int n_divs = 1);
	~DiskBasicGroupItem() {}
	void Set(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div = 0, int n_divs = 1);
	static int Compare(DiskBasicGroupItem **item1, DiskBasicGroupItem **item2);
};

WX_DECLARE_OBJARRAY(DiskBasicGroupItem, DiskBasicGroupItems);

//////////////////////////////////////////////////////////////////////

/// グループ番号のリストを保持
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

	void	Add(wxUint32 n_group, wxUint32 n_next, int n_track, int n_side, int n_start, int n_end, int n_div = 0, int n_divs = 1);
	void	Add(const DiskBasicGroupItem &item);
	void	Add(const DiskBasicGroups &n_items);
	/// リストをクリア
	void	Empty();
	/// リストの数を返す
	size_t	Count() const;
	/// リストの最後を返す
	DiskBasicGroupItem &Last() const;
	/// リストアイテムを返す
	DiskBasicGroupItem &Item(size_t idx) const;
	/// リストアイテムを返す
	DiskBasicGroupItem *ItemPtr(size_t idx) const;
	/// リストを返す
	const DiskBasicGroupItems &GetItems() const { return items; }

	/// グループ数を返す
	int		GetNums() const { return nums; }
	/// 占有サイズを返す
	size_t	GetSize() const { return size; }
	/// １グループのサイズを返す
	size_t	GetSizePerGroup() const { return size_per_group; }

	/// グループ数を設定
	void	SetNums(int val) { nums = val; }
	/// 占有サイズを設定
	void	SetSize(size_t val) { size = val; }
	/// １グループのサイズを設定
	void	SetSizePerGroup(size_t val) { size_per_group = val; }

	/// グループ数を足す
	int		AddNums(int val);
	/// 占有サイズを足す
	int		AddSize(int val);

	/// グループ番号でソート
	void SortItems();
};

//////////////////////////////////////////////////////////////////////
//
//
//

enum en_fat_availability {
	FAT_AVAIL_FREE = 0,
	FAT_AVAIL_SYSTEM,
	FAT_AVAIL_USED,
	FAT_AVAIL_USED_FIRST,
	FAT_AVAIL_USED_LAST,
	FAT_AVAIL_MISSING,
	FAT_AVAIL_LEAK,
	FAT_AVAIL_NULLEND
};

#endif /* _BASICCOMMON_H_ */
