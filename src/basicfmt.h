/// @file basicfmt.h
///
/// @brief disk basic
///
#ifndef _BASICFMT_H_
#define _BASICFMT_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/dynarray.h>
#include "diskd88.h"
#include "result.h"

/// タイプ1 0...BASIC 1...DATA 2...MACHINE
extern const wxString gTypeName1[];

#define DISKBASIC_FILETYPE_BASIC	0
#define DISKBASIC_FILETYPE_DATA		1
#define DISKBASIC_FILETYPE_MACHINE	2

/// タイプ2 0...Binary 0xff...Ascii
extern const wxString gTypeName2[];
/// タイプ3 0...Sequential 0xff...Random Access
extern const wxString gTypeName3[];

#define DISKBASIC_DATATYPE_BINARY	0
#define DISKBASIC_DATATYPE_ASCII	1
#define DISKBASIC_DATATYPE_RANDOM	2

/// ディレクトリ
#pragma pack(1)
// ５インチ,８インチ(L3,S1共通)
typedef struct st_directory_2d {
	wxUint8 start_group;
	struct {
		wxUint8 h;
		wxUint8 l;
	} end_bytes;	///< used size of end cluster (big endien)

	char reserved[16];
} directory_2d_t;

// ３インチ(単密度)
typedef struct st_directory_1s {
	wxUint8 type3;
	wxUint8 start_group;

	char reserved[17];
} directory_1s_t;

// ディレクトリ
typedef struct st_directory {
	wxUint8 name[8];
	wxUint8 ext[3];	///< 2D,2HD only (1S is not used.)
	wxUint8 type;
	wxUint8 type2;
	union {
		directory_2d_t d;
		directory_1s_t s;
	};
} directory_t;
#pragma pack()

///
class DiskBasicParam
{
protected:
	wxString basic_type_name;
	int format_type;			///< フォーマット種類(0:片面単密度 1:両面倍密度 3:両面高密度)
	int sectors_per_group;		///< グループ(クラスタ)サイズ
	int manage_track_number;	///< ファイル管理エリア
	int fat_start_sector;		///< FAT領域の開始セクタ
	int fat_end_sector;			///< FAT領域の終了セクタ
	int fat_start_pos;			///< FAT開始位置（バイト）
	int fat_end_group;			///< FAT最大グループ番号
	int dir_start_sector;		///< ディレクトリ開始セクタ
	int dir_end_sector;			///< ディレクトリ終了セクタ
	wxUint8 fillcode_on_format;	///< フォーマット時に埋めるコード
	wxString description;

public:
	DiskBasicParam();
	DiskBasicParam(const DiskBasicParam &src);
	DiskBasicParam(const wxString & n_basic_type_name, int n_format_type, int n_sectors_per_group, int n_manage_track_number, int n_fat_start_sector, int n_fat_end_sector, int n_fat_start_pos, int n_fat_end_group, int n_dir_start_sector, int n_dir_end_sector, wxUint8 n_fillcode_on_format , const wxString & n_description);
	virtual ~DiskBasicParam() {}

	virtual void ClearBasicParam();
	virtual void SetBasicParam(const DiskBasicParam &src);
	virtual const DiskBasicParam &GetBasicParam() const;
	const wxString &GetBasicTypeName() const { return basic_type_name; }
	int GetFormatType() { return format_type; }
	int GetSectorsPerGroup() { return sectors_per_group; }
	int GetManageTrackNumber() { return manage_track_number; }
	int GetFatStartSector() { return fat_start_sector; }
	int GetFatEndSector() { return fat_end_sector; }
	int GetFatStartPos() { return fat_start_pos; }
	int GetFatEndGroup() { return fat_end_group; }
	int GetDirStartSector() { return dir_start_sector; }
	int GetDirEndSector() { return dir_end_sector; }
	wxUint8 GetFillCodeOnFormat() { return fillcode_on_format; }
	virtual const wxString &GetDescription() { return description; }
};

WX_DECLARE_OBJARRAY(DiskBasicParam, DiskBasicParams);

///
class DiskBasicTypes
{
private:
	DiskBasicParams types;

public:
	DiskBasicTypes();
	~DiskBasicTypes() {}

	bool Load(const wxString &data_path);

	DiskBasicParam *Find(const wxString &n_type_name);
#if 0
	const wxString &GetBasicTypeName(int num) { return types[num].GetBasicTypeName(); }
	const DiskBasicParam &GetBasicParam(int num) const { return types[num]; }
	int GetFormatType(int num) { return types[num].GetFormatType(); }
	int GetSectorsPerGroup(int num) { return types[num].GetSectorsPerGroup(); }
	int GetManageTrackNumber(int num) { return types[num].GetManageTrackNumber(); }
	int GetFatStartSector(int num) { return types[num].GetFatStartSector(); }
	int GetFatEndSector(int num) { return types[num].GetFatEndSector(); }
	int GetFatStartPos(int num) { return types[num].GetFatStartPos(); }
	int GetDirStartSector(int num) { return types[num].GetDirStartSector(); }
	int GetDirEndSector(int num) { return types[num].GetDirEndSector(); }
	const wxString &GetDescription(int num) { return types[num].GetDescription(); }
#endif
};

extern DiskBasicTypes gDiskBasicTypes;

///
class DiskBasicGroupItem
{
public:
	int group;
	int track;
	int side;
	int sector_start;
	int sector_end;
public:
	DiskBasicGroupItem();
	~DiskBasicGroupItem() {}
};

WX_DECLARE_OBJARRAY(DiskBasicGroupItem, DiskBasicGroupItems);

///
class DiskBasicFatBuffer
{
public:
	int      size;
	wxUint8 *buffer;

public:
	DiskBasicFatBuffer();
	DiskBasicFatBuffer(wxUint8 *newbuf, int newsize);
	~DiskBasicFatBuffer() {}
};

WX_DECLARE_OBJARRAY(DiskBasicFatBuffer, DiskBasicFatBuffers);

/// FAT (8bit)
class DiskBasicFat
{
private:
	DiskD88Track *managed_track;
	int format_type;	///< フォーマット種類
	int start_sector;	///< 開始セクタ番号
	int end_sector;		///< 終了セクタ番号
	int start_pos;		///< 開始位置
	int sides;			///< サイド数
	int grps_per_track;	///< 1トラックあたりのグループ数
	int secs_per_group;	///< 1グループ（クラスタ）あたりのセクタ数
	int sector_size;	///< セクタサイズ
	int end_group;		///< 最終グループ番号
	int managed_start_group;	///< 管理エリアの開始グループ番号

	DiskBasicFatBuffers bufs;

	wxUint32 free_disk_size;	///< 残りディスクサイズ
	wxUint32 free_groups;		///< 残りグループサイズ

	wxUint32 file_size_cache;	///< CalcSizeで計算したファイルサイズを保持
	wxUint32 groups_cache;		///< CalcSizeで計算したグループサイズを保持
	wxUint8  last_group_cache;	///< CalcSizeで計算した最終グループ番号を保持
	wxUint8  last_sector_cache;	///< CalcSizeで計算した最終グループを保持(0xc0...)

	void CalcGroupNumberFromManagedTrack();

public:
	DiskBasicFat();
	~DiskBasicFat();

	bool SetStart(DiskD88Track *newtrack, int newfmttype, int newstart, int newend, int newstartpos, int newsides, int newgrpspertrk, int newsecspergrp, int newendgrp);
	void Clear();
	void Empty();

	void SetGroupNumber(wxUint8 num, wxUint8 val);
	wxUint8 GetGroupNumber(wxUint8 num);
	wxUint8 GetEmptyGroupNumber();
	wxUint8 GetNextEmptyGroupNumber(wxUint8 curr_group);

	bool CalcSize(wxUint8 start_group);
	bool CalcFreeSize();

	bool Check();

	int GetEndGroup() const { return end_group; }

	wxUint32 GetCachedFileSize() const { return file_size_cache; }
	wxUint32 GetCachedGroupSize() const { return groups_cache; }
	wxUint8 GetCachedLastGroup() const { return last_group_cache; }
	wxUint8 GetCachedLastSector() const { return last_sector_cache; }

	wxUint32 GetFreeDiskSize() const { return free_disk_size; }
	wxUint32 GetFreeGroupSize() const { return free_groups; }
};

/// ディレクトリ１アイテム
class DiskBasicDirItem
{
private:
	int num;			///< 通し番号
	int format_type;	///< フォーマットタイプ
	int track_number;	///< トラック番号
	int side_number;	///< サイド番号
	int position;		///< セクタ内の位置（バイト）
	bool used;			///< 使用しているか
	int file_size;		///< ファイルサイズ
	int groups;			///< 使用グループ数
	directory_t *data;
	DiskD88Sector *sector;

public:
	DiskBasicDirItem();
	DiskBasicDirItem(int newnum, int newtype, int newtrack, int newside, DiskD88Sector *newsec, int newpos, directory_t *newdata, int newfilesize, int newgrps);
	DiskBasicDirItem(const DiskBasicDirItem &src);
	~DiskBasicDirItem();

	bool Delete();

	void SetFileName(const wxString &filename);
	void SetFileNameAndAttr(const wxString &filename, int file_type, int data_type);
	void ClearFileNameAndAttr();
	int GetFileType();
	int GetDataType();
	wxString GetFileNameStr();
	wxString GetFileTypeStr();
	wxString GetDataTypeStr();
	int GetFileSize();
	int GetGroupSize();
	void SetStartGroup(wxUint8 val);
	wxUint8 GetStartGroup() const;
	bool IsUsed() const { return used; }
	void SetDataSizeOnLastSecotr(int val);
	int GetDataSizeOnLastSector();
	bool IsSameFileName(const wxString &filename);
	DiskD88Sector *GetSector() const { return sector; }
	directory_t *GetDirData() const { return data; }

	static bool ConvFromNativeNameWithExtension(int format_type, int file_type, const wxUint8 *src, const wxUint8 *ext, wxString &dst);
	static bool ConvFromNativeName(int format_type, const wxUint8 *src, const wxUint8 *ext, wxString &dst);
	static bool ConvToNativeName(int format_type, const wxString &src, wxUint8 *dst, size_t len);
};

WX_DECLARE_OBJARRAY(DiskBasicDirItem, DiskBasicDirItems);

/// ディレクトリ
class DiskBasicDir
{
private:
	DiskBasicDirItems items;
public:
	DiskBasicDir();
	~DiskBasicDir();

	void Add(int newnum, int newtype, int newtrack, int newside, DiskD88Sector *newsec, int newpos, directory_t *newdata, int newfilesize, int newgrps);
	void Add(const DiskBasicDirItem &item);
	DiskBasicDirItem &Item(size_t idx);
	DiskBasicDirItem &operator[](size_t idx);
	DiskBasicDirItem *ItemPtr(size_t idx);
	DiskBasicDirItem *GetEmptyItemPtr();
	size_t Count();
	void Clear();
	void Empty();
	DiskBasicDirItem *FindFile(const wxString &filename, DiskBasicDirItem *exclude_item = NULL);
	static bool Check(DiskD88Track *managed_track, int start_sector);
};

/// エラー情報
class DiskBasicError : public ResultInfo
{
public:
	DiskBasicError() : ResultInfo() {}

	enum {
		ERR_NONE = 0,
		ERR_SUPPORTED,
		ERR_FORMATTED,
		ERR_UNSELECT_DISK,
		ERR_WRITE_PROTECTED,
		ERR_FILE_NOT_FOUND,
		ERR_DIRECTORY_FULL,
		ERR_DISK_FULL,
		ERR_FILE_TOO_LARGE,
		ERR_NOT_ENOUGH_FREE,
		ERR_VERIFY_FILE,
		ERR_MISMATCH_FILESIZE,
		ERR_NO_TRACK,
		ERR_NO_SECTOR,
		ERR_INVALID_SECTOR,
		ERR_CANNOT_EXPORT,
		ERR_CANNOT_IMPORT,
		ERR_CANNOT_VERIFY,
		ERR_CANNOT_FORMAT,
	};

	void SetMessage(int error_number, va_list ap);
};

#ifdef DeleteFile
#undef DeleteFile
#endif

/// DISK BASIC 構造解析
class DiskBasic : public DiskParam, public DiskBasicParam
{
private:
	DiskD88Disk *disk;
	bool formatted;

	DiskBasicFat fat;
	DiskBasicDir directory;

	int selected_side;			///< サイド(1S用)
	DiskD88Track *manage_track[2];	///< 管理エリア

	int free_size;
	int free_groups;
	wxString desc_size;

	DiskBasicError errinfo;

public:
	DiskBasic();
	~DiskBasic();

	/// @name for user interface
	//@{
	int ParseDisk(DiskD88Disk *newdisk, int newside);
	void Clear();
	bool AssignFat();
	bool AssignDirectory();
	bool AssignFatAndDirectory();
	wxString RemakeFileNameStr(const wxString &filepath);
	size_t GetFileNameLength();

	bool LoadFile(const wxString &filename, const wxString &dstpath);
	bool LoadFile(int item_number, const wxString &dstpath);
	bool LoadFile(DiskBasicDirItem *item, const wxString &dstpath);
	bool VerifyFile(DiskBasicDirItem *item, const wxString &srcpath);
	bool AccessData(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream);
	DiskBasicDirItem *FindFile(const wxString &filename, DiskBasicDirItem *exclude_item = NULL);
	bool IsWriteableDisk();
	bool SaveFile(const wxString &srcpath, const wxString &filename, int file_type, int data_type);
	bool SaveFile(const wxUint8 *buffer, size_t buflen, const wxString &filename, int file_type, int data_type);
	bool SaveFile(DiskBasicDirItem *item, wxInputStream *istream, const wxString &filename, int file_type, int data_type);
//	bool DeleteFile(const wxString &filename);
//	bool DeleteFile(int item_number);
	bool DeleteFile(DiskBasicDirItem *item, bool clearmsg = true);
	bool RenameFile(DiskBasicDirItem *item, const wxString &newname, int file_type = -1, int data_type = -1);

	bool FormatDisk();

	void GetAllGroups(const DiskBasicDirItem *item, DiskBasicGroupItems &group_items);

	DiskD88Track *GetTrackFromGroup(wxUint8 group_num, wxUint8 next_group, int *sector_start = NULL, int *sector_end = NULL);
	DiskD88Sector *GetSectorFromGroup(wxUint8 group_num);
	DiskD88Sector *GetSectorFromPosition(size_t position);

	bool GetGroupsFromPosition(size_t position, DiskBasicGroupItems &group_items);
	//@}

	/// @name properties
	//@{
	bool IsFormatted() const { return (disk != NULL && formatted); }
	bool CanUse() const { return (disk != NULL); }
	DiskD88Disk *GetDisk() const { return disk; }
	int GetSelectedSide() const { return selected_side; }
	const wxString &GetDescription();
	DiskBasicDir &GetDirectory() { return directory; }
	bool GetNumsFromGroup(wxUint8 group_num, wxUint8 next_group, int &track_num, int &side_num, int *sector_start, int *sector_end);
//	int GetFormatType() { return basicparam.GetFormatType(); }
	const wxArrayString &GetErrorMessage(int maxrow = 30);
	int GetErrorLevel(void) const;
//	int GetFatEndGroup() { return basicparam.GetFatEndGroup(); }
	int GetNextGroupNumber(int group_num) { return fat.GetGroupNumber((wxUint8)group_num); }
	//@}
};

#endif /* _BASICFMT_H_ */
