/// @file diskd88.h
///
/// @brief D88ディスクイメージ入出力
///
#ifndef _DISKD88_H_
#define _DISKD88_H_

#include "common.h"
#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/dynarray.h>
#include "diskparam.h"
#include "diskd88parser.h"

/// disk density 0: 2D, 1: 2DD, 2: 2HD
extern const wxString gDiskDensity[];

#define DISKD88_MAX_TRACKS	164

#pragma pack(1)
typedef struct st_d88_header {
	wxUint8 diskname[17];		///< disk name
	char reserved1[9];
	wxUint8 write_protect;		///< 0x10 write protected
	wxUint8 disk_density;		///< disk density 00H: 2D, 10H: 2DD, 20H: 2HD
	wxUint32 disk_size;			///< disk size

	wxUint32 offsets[DISKD88_MAX_TRACKS];		///< track table
} d88_header_t;

typedef	struct st_d88_sector_id {
		wxUint8 c;		///< cylinder (track) id (0...)
		wxUint8 h;		///< head (side) id
		wxUint8 r;		///< record (sector) id (1...)
		wxUint8 n;		///< sector size (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
} d88_sector_id_t;

typedef struct st_d88_sector_header {
	d88_sector_id_t id;
	wxUint16 secnums;	///< sector numbers per track
	wxUint8 density;	///< 0x00:double density 0x40:single density
	wxUint8 deleted;	///< 0x10:deleted data
	wxUint8 status;		///< 0x00:no error
	char reserved[5];
	wxUint16 size;		///< sector size (bytes)
} d88_sector_header_t;
#pragma pack()

/// セクタデータへのポインタを保持するクラス
class DiskD88Sector
{
private:
	int num;	///< sector number
	bool deleted;	///< deleted mark
	d88_sector_header_t header;
	wxUint8 *data;

	bool modified;	///< 変更したか

	d88_sector_header_t header_origin;
	wxUint8 *data_origin;

	DiskD88Sector(const DiskD88Sector &src);

public:
	DiskD88Sector();
	DiskD88Sector(int newnum, const d88_sector_header_t &newhdr, wxUint8 *newdata);
	DiskD88Sector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density = false);
	~DiskD88Sector();

	bool Replace(DiskD88Sector *src_sector); 
	int GetSectorNumber() { return num; }
	bool IsDeleted() { return deleted; }
	void SetDeletedMark(bool val);
	bool IsSameSector(int sector_number, bool deleted_mark = false);
	/// セクタサイズ
	int GetSectorSize();
	void SetSectorSize(int val);
	/// セクタサイズ（バッファのサイズ）
	int GetSectorBufferSize() { return header.size; }
	wxUint8 *GetSectorBuffer() { return data; }

	wxUint16 GetSectorsPerTrack() { return header.secnums; }
	void SetSectorsPerTrack(wxUint16 val) { header.secnums = val; }

	d88_sector_header_t *GetHeader() { return &header; }
	wxUint8 GetIDC() { return header.id.c; }
	wxUint8 GetIDH() { return header.id.h; }
	wxUint8 GetIDR() { return header.id.r; }
	wxUint8 GetIDN() { return header.id.n; }
	void SetIDC(wxUint8 val) { header.id.c = val; }
	void SetIDH(wxUint8 val) { header.id.h = val; }
	void SetIDR(wxUint8 val) { header.id.r = val; }
	void SetIDN(wxUint8 val) { header.id.n = val; }
	bool IsSingleDensity();
	void SetSingleDensity(bool val);

	bool IsModified() const { return modified; }
	void SetModify();
	void ClearModify();

	static int Compare(DiskD88Sector *item1, DiskD88Sector *item2);
	static int ConvIDNToSecSize(wxUint8 n);
	static wxUint8 ConvSecSizeToIDN(int size);
};

WX_DEFINE_ARRAY(DiskD88Sector *, DiskD88Sectors);

/// トラックデータへのポインタを保持するクラス
class DiskD88Track
{
private:
	int trk_num;	///< track number
	int sid_num;	///< side number
	int offset_pos;	///< offset position
	wxUint32 offset;	///< offset on diskimage
	wxUint32 size;	///< track size
	DiskD88Sectors *sectors;

	DiskD88Track(const DiskD88Track &src);

public:
	DiskD88Track();
	DiskD88Track(int newtrknum, int newsidnum, int newoffpos, wxUint32 newoffset);
//	DiskD88Track(int newnum, int newsidnum, wxUint32 newoffset, DiskD88Sectors *newsecs);
	~DiskD88Track();

	size_t Add(DiskD88Sector *newsec);
	int Replace(DiskD88Track *src_track); 
	int GetTrackNumber() const { return trk_num; }
	int GetSideNumber() const { return sid_num; }
	void SetSideNumber(int val) { sid_num = val; }
	wxUint32 GetOffset() const { return offset; }
	int GetOffsetPos() const { return offset_pos; }
	int GetMaxSectorNumber();
	int GetMaxSectorSize();
	wxUint32 GetSize() { return size; }
	void SetSize(wxUint32 val) { size = val; }

	DiskD88Sectors *GetSectors() { return sectors; }
	DiskD88Sector  *GetSector(int sector_number);

	void SetAllIDC(wxUint8 val);
	void SetAllIDH(wxUint8 val);
	void SetAllIDR(wxUint8 val);
	void SetAllIDN(wxUint8 val);
	void SetAllSingleDensity(bool val);

	bool IsModified() const;
	void ClearModify();

	static int Compare(DiskD88Track *item1, DiskD88Track *item2);
};

WX_DEFINE_ARRAY(DiskD88Track *, DiskD88Tracks);

/// １ディスクへのポインタを保持するクラス
class DiskD88Disk : public DiskParam
{
private:
	int num;	///< disk number

	d88_header_t header;
	DiskD88Tracks *tracks;
	int offset_maxpos;

	d88_header_t header_origin;

	//	wxUint8 *buffer;
//	size_t buffer_size;

	bool modified;	///< 変更したか

	DiskD88Disk(const DiskD88Disk &src);

public:
	DiskD88Disk();
	DiskD88Disk(const wxString &newname, int newnum, const DiskParam &param, bool write_protect);
	DiskD88Disk(int newnum, const d88_header_t &newhdr);
//	DiskD88Disk(wxUint8 *newbuf, size_t newbuflen, int newnum);
//	DiskD88Disk(int newnum, d88_header_t *newhdr, DiskD88Tracks *newtrks);
	~DiskD88Disk();

	size_t Add(DiskD88Track *newtrk);
	int Replace(int side_number, DiskD88Disk *src_disk, int src_side_number);
	size_t DeleteTracks(int start_offset_pos, int end_offset_pos);

	int GetNumber() const { return num; }
	wxString GetName(bool real = false);
	void SetName(const wxString &val);
//	int GetDiskType() { return disk_type; }

	d88_header_t *GetHeader() { return &header; }

	DiskD88Tracks *GetTracks() { return tracks; }
	DiskD88Track  *GetTrack(int track_number, int side_number);
	DiskD88Track  *GetTrackByOffset(wxUint32 offset);
	void CalcMajorNumber();

	wxString GetAttrText();

	bool GetWriteProtect();
	void SetWriteProtect(bool val);

	wxString GetDensityText();
	void SetDensity(int val);

	wxUint32 GetSize();
	void SetSize(wxUint32 val);
	wxUint32 GetSizeWithoutHeader();
	void SetSizeWithoutHeader(wxUint32 val);

	wxUint32 GetOffset(int num);
	void SetOffset(int num, wxUint32 offset);
	void SetOffsetWithoutHeader(int num, wxUint32 offset);

	int GetOffsetMaxPos();
	void SetOffsetMaxPos(int pos);

	bool Initialize(int selected_side);
	bool Rebuild(const DiskParam &param, int selected_side);

//	wxUint8 *GetBuffer() { return buffer; }
//	size_t GetBufferSize() { return buffer_size; }

	void SetModify();
	bool IsModified();
	void ClearModify();

	bool ExistTrack(int side_number);

	static int Compare(DiskD88Disk *item1, DiskD88Disk *item2);
};

WX_DEFINE_ARRAY(DiskD88Disk *, DiskD88Disks);

/// ディスクイメージへのポインタを保持するクラス
class DiskD88File
{
private:
	DiskD88Disks *disks;
	bool modified;

	DiskD88File(const DiskD88File &src);

public:
	DiskD88File();
	~DiskD88File();

	size_t Add(DiskD88Disk *newdsk);
	void Clear();
	size_t Count() const;
	bool Delete(size_t idx);

	DiskD88Disks *GetDisks() { return disks; }
	DiskD88Disk  *GetDisk(size_t idx);

	void SetModify();
	bool IsModified();
	void ClearModify();
};

/// D88ディスクイメージ入出力
class DiskD88
{
private:
	wxFileName filename;
	DiskD88File *file;
	DiskD88Result result;

	void NewFile(const wxString &newpath);
	void ClearFile();

#ifdef DISKD88_USE_MEMORY_INPUT_STREAM
	wxMemoryInputStream *stream;
	bool OpenStream(wxInputStream &src);
	void CloseStream();
#endif

public:
	DiskD88();
	~DiskD88();

	int Create(const wxString &diskname, const DiskParam &param, bool write_protect);
	int Add(const wxString &diskname, const DiskParam &param, bool write_protect);
	int Add(const wxString &filepath);
	int Open(const wxString &filepath);
	void Close();
	int Save(const wxString &filepath);
	int SaveDisk(int disk_number, int side_number, const wxString &filepath);
	int SaveDisk(DiskD88Disk *disk, wxOutputStream *stream);
	int SaveDisk(DiskD88Disk *disk, int side_number, wxOutputStream *stream);
	bool Delete(size_t disk_number);
	int ReplaceDisk(int disk_number, int side_number, const wxString &filepath);

	bool SetDiskName(size_t disk_number, const wxString &newname);
	const wxString GetDiskName(size_t disk_number, bool real = false);


	bool IsModified();

	DiskD88File *GetFile() { return file; }
	size_t CountDisks() const;
	DiskD88Disks *GetDisks();
	DiskD88Disk *GetDisk(size_t index);
	int GetDiskTypeNumber(size_t index);

	wxString GetFileName() const;
	wxString GetFileNameBase() const;
	wxString GetFilePath() const;
	wxString GetPath() const;

	const wxArrayString &GetErrorMessage(int maxrow = 30);
};

#endif /* _DISKD88_H_ */
