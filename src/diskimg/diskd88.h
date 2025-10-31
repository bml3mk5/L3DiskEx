/// @file diskd88.h
///
/// @brief D88ディスクイメージ入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKD88_H
#define DISKD88_H

#include "../common.h"
#include <wx/string.h>
#include <wx/filename.h>
//#include <wx/mstream.h>
#include <wx/dynarray.h>
#include <wx/hashmap.h>
#include "diskimage.h"
#include "diskparam.h"
#include "diskresult.h"

#ifdef DISKD88_USE_MEMORY_INPUT_STREAM
class wxMemoryInputStream;
#endif

/// disk density 0: 2D, 1: 2DD, 2: 2HD
struct st_disk_density {
	wxUint8		val;
	const char *name;
};
extern const struct st_disk_density gDiskDensity[];

#define DISKD88_MAX_TRACKS	164
#define HEADER_TYPE_D88 1

// ----------------------------------------------------------------------

#pragma pack(1)
/// D88 disk header
typedef struct st_d88_header {
	wxUint8 diskname[17];		///< disk name
	char reserved1[9];
	wxUint8 write_protect;		///< 0x10 write protected
	wxUint8 disk_density;		///< disk density 00H: 2D, 10H: 2DD, 20H: 2HD
	wxUint32 disk_size;			///< disk size

	wxUint32 offsets[DISKD88_MAX_TRACKS];		///< track table
} d88_header_t;

/// D88 sector id
typedef	struct st_d88_sector_id {
		wxUint8 c;		///< cylinder (track) id (0...)
		wxUint8 h;		///< head (side) id
		wxUint8 r;		///< record (sector) id (1...)
		wxUint8 n;		///< sector size (0:128bytes 1:256bytes 2:512bytes 3:1024bytes)
} d88_sector_id_t;

/// D88 sector header
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

// ----------------------------------------------------------------------

/// セクタデータへのヘッダ部分を渡すクラス
class DiskD88SectorHeader : public DiskImageSectorHeader
{
private:
	d88_sector_header_t *m_header;	///< sector header

public:
	DiskD88SectorHeader();
	~DiskD88SectorHeader();
	int GetHeaderType() const { return HEADER_TYPE_D88; }
	d88_sector_header_t *GetHeader() { return m_header; }
	wxUint32 GetHeaderSize() const { return (wxUint32)sizeof(d88_sector_header_t); }
	void Alloc();
	void Free();
//	void New(d88_sector_header_t *n_header);
	void New(const DiskImageSectorHeader &src);
	void New(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density, int status);
	void Fill(wxUint8 data);
	void Copy(const DiskD88SectorHeader &src);
	bool IsSame(const DiskD88SectorHeader &src) const;
	/// ID Cを返す
	wxUint8	GetIDC() const;
	/// ID Hを返す
	wxUint8	GetIDH() const;
	/// ID Rを返す
	wxUint8	GetIDR() const;
	/// ID Nを返す
	wxUint8	GetIDN() const;
	/// セクタ数を返す
	wxUint16 GetNumberOfSectors() const;
	/// 密度を返す
	wxUint8	GetDensity() const;
	/// 削除フラグを返す
	wxUint8	GetDeleted() const;
	/// ステータスを返す
	wxUint8	GetStatus() const;
	/// サイズを返す
	wxUint16 GetSize() const;
	/// ID Cを設定
	void	SetIDC(wxUint8 val);
	/// ID Hを設定
	void	SetIDH(wxUint8 val);
	/// ID Rを設定
	void	SetIDR(wxUint8 val);
	/// ID Nを設定
	void	SetIDN(wxUint8 val);
	/// セクタ数を設定
	void SetNumberOfSectors(wxUint16 val);
	/// 密度を設定
	void SetDensity(wxUint8 val);
	/// 削除フラグを設定
	void SetDeleted(wxUint8 val);
	/// ステータスを設定
	void SetStatus(wxUint8 val);
	/// サイズを設定
	void SetSize(wxUint16 val);
};

// ----------------------------------------------------------------------

/// セクタデータへのポインタを保持するクラス
class DiskD88Sector : public DiskImageSector
{
private:
	DiskD88SectorHeader	 m_header;		///< sector header
	wxUint8 			*data;			///< sector data

	DiskD88SectorHeader	 m_header_origin;	///< pre-save header
	wxUint8				*data_origin;		///< pre-save data

	int					 m_rec_crc;		///< recorded CRC

	DiskD88Sector();
	DiskD88Sector(const DiskD88Sector &src) : DiskImageSector(src) {}
	DiskD88Sector &operator=(const DiskD88Sector &src) { return *this; }

public:
	DiskD88Sector(int n_num, const DiskImageSectorHeader &n_header, wxUint8 *n_data);
	DiskD88Sector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density = false, int status = 0);
	~DiskD88Sector();

	/// セクタのデータを置き換える
	bool	Replace(DiskImageSector *src_sector);
	/// セクタのデータを埋める
	bool	Fill(wxUint8 code, int len = -1, int start = 0); 
	/// セクタのデータを上書き
	bool	Copy(const void *buf, int len, int start = 0); 
	/// セクタのデータに指定したバイト列があるか
	int		Find(const void *buf, size_t len); 
	/// 指定位置のセクタデータを返す
	wxUint8	Get(int pos) const;
	/// 指定位置のセクタデータを返す
	wxUint16 Get16(int pos, bool big_endian = false) const;
	/// セクタサイズを変更
	int		ModifySectorSize(int size);

//	/// セクタ番号を返す(ID Rと同じ)
//	int		GetSectorNumber() const { return num; }
	/// セクタ番号を設定
	void	SetSectorNumber(int val);
	/// 削除マークがついているか
	bool	IsDeleted() const;
	/// 削除マークの設定
	void	SetDeletedMark(bool val);
	/// 同じセクタか
	bool	IsSameSector(int sector_number, int density, bool deleted_mark = false);
	/// ヘッダサイズを返す
	int		GetHeaderSize() const;
	/// セクタサイズを返す
	int		GetSectorSize() const;
	/// セクタサイズを設定
	void	SetSectorSize(int val);
	/// セクタサイズ（バッファのサイズ）を返す
	int		GetSectorBufferSize() const;
	/// セクタサイズ（ヘッダ＋バッファのサイズ）を返す
	int		GetSize() const;
	/// セクタデータへのポインタを返す
	wxUint8 *GetSectorBuffer() { return data; }
	/// セクタデータへのポインタを返す
	wxUint8 *GetSectorBuffer(int offset);
	/// セクタ数を返す
	wxUint16 GetSectorsPerTrack() const;
	/// セクタ数を設定
	void	SetSectorsPerTrack(wxUint16 val);
	/// セクタのステータスを返す
	wxUint8 GetSectorStatus() const;
	/// セクタのステータスを設定
	void    SetSectorStatus(wxUint8 val);

	/// ヘッダを返す
	DiskImageSectorHeader *GetHeader() { return &m_header; }
	/// ID Cを返す
	wxUint8	GetIDC() const;
	/// ID Hを返す
	wxUint8	GetIDH() const;
	/// ID Rを返す
	wxUint8	GetIDR() const;
	/// ID Nを返す
	wxUint8	GetIDN() const;
	/// ID Cを設定
	void	SetIDC(wxUint8 val);
	/// ID Hを設定
	void	SetIDH(wxUint8 val);
	/// ID Rを設定
	void	SetIDR(wxUint8 val);
	/// ID Nを設定
	void	SetIDN(wxUint8 val);
	/// 単密度か
	bool	IsSingleDensity();
	/// 単密度かを設定
	void	SetSingleDensity(bool val);

	/// CRCを返す
	int		GetRecordedCRC() const;
	/// CRCを計算する
	int		CalculateCRC();
	/// CRCを設定
	void 	SetRecordedCRC(wxUint16 crc);

	/// 変更されているか
	bool	IsModified() const;
	/// 変更済みを設定
	void	SetModify();
	/// 変更済みをクリア
	void	ClearModify();
};

// ----------------------------------------------------------------------

class DiskD88Disk;

/// トラックデータへのポインタを保持するクラス
class DiskD88Track : public DiskImageTrack
{
private:
	DiskD88Track() {}
	DiskD88Track(const DiskD88Track &src) {}
	DiskD88Track &operator=(const DiskD88Track &src) { return *this; }

public:
	DiskD88Track(DiskImageDisk *disk);
	DiskD88Track(DiskImageDisk *disk, int n_trk_num, int n_sid_num, int n_offset_pos, int n_interleave);
	~DiskD88Track();

	/// インスタンス作成
	DiskImageSector *NewImageSector(int n_num, const DiskImageSectorHeader &n_header, wxUint8 *n_data);
	/// インスタンス作成
	DiskImageSector *NewImageSector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density = false, int status = 0);
};

// ----------------------------------------------------------------------

WX_DEFINE_ARRAY(DiskD88Track *, DiskD88Tracks);

// ----------------------------------------------------------------------

/// １ディスクのヘッダを渡すクラス
class DiskD88DiskHeader : public DiskImageDiskHeader
{
private:
	d88_header_t *m_header;
public:
	DiskD88DiskHeader();
	~DiskD88DiskHeader();
	int GetHeaderType() const { return HEADER_TYPE_D88; }
	d88_header_t *GetHeader() { return m_header; }
	wxUint32 GetHeaderSize() const { return (wxUint32)sizeof(d88_header_t); }
	void Alloc();
	void Free();
//	void New(d88_header_t *n_header);
	void New(const DiskImageDiskHeader &src);
	void Fill(wxUint8 data);
	void Copy(const DiskD88DiskHeader &src);
	void ClearOffsets();
	bool IsSame(const DiskD88DiskHeader &src) const;
	/// ディスク名を返す
	wxString GetName(bool real = false) const;
	/// 書き込み禁止かを返す
	bool IsWriteProtected() const;
	/// 密度を返す
	wxUint8 GetDensity() const;
	/// ディスクサイズを返す
	wxUint32 GetDiskSize() const;
	/// オフセットを返す
	wxUint32 GetOffset(int num) const;
	/// ディスク名を設定
	void SetName(const wxString &val);
	/// ディスク名を設定
	void SetName(const wxUint8 *buf, size_t len);
	/// 書き込み禁止かを設定
	void SetWriteProtect(bool val);
	/// 密度を設定
	void SetDensity(wxUint8 val);
	/// ディスクサイズを設定
	void SetDiskSize(wxUint32 val);
	/// オフセットを設定
	void SetOffset(int num, wxUint32 val);
};

// ----------------------------------------------------------------------

class DiskD88File;
class DiskBasicParam;
class DiskBasic;
class DiskBasics;

/// １ディスクへのポインタを保持するクラス
class DiskD88Disk : public DiskImageDisk
{
private:
	DiskD88DiskHeader m_header;	///< disk header

	DiskD88DiskHeader m_header_origin;

	bool m_modified;	///< 変更したか

	DiskD88Disk() : DiskImageDisk() {}
	DiskD88Disk(const DiskD88Disk &src) : DiskImageDisk(src) {}
	DiskD88Disk &operator=(const DiskD88Disk &src) { return *this; }

public:
	DiskD88Disk(DiskImageFile *file, int n_num);
	DiskD88Disk(DiskImageFile *file, int n_num, const DiskParam &n_param, const wxString &n_diskname, bool n_write_protect);
	DiskD88Disk(DiskImageFile *file, int n_num, const DiskImageDiskHeader &n_header);
	~DiskD88Disk();

	/// インスタンス作成
	DiskImageTrack *NewImageTrack();
	/// インスタンス作成
	DiskImageTrack *NewImageTrack(int n_trk_num, int n_sid_num, int n_offset_pos, int n_interleave);

	/// ディスク名を返す
	wxString GetName(bool real = false) const;
	/// ディスク名を設定
	void	SetName(const wxString &val);
	/// ディスク名を設定
	void	SetName(const wxUint8 *buf, size_t len);

	/// ヘッダを返す
	DiskImageDiskHeader *GetHeader() { return &m_header; }

	/// 書き込み禁止かどうかを返す
	bool	IsWriteProtected() const;
	/// 書き込み禁止かどうかを設定
	void	SetWriteProtect(bool val);

	/// 密度を返す
	wxString GetDensityText() const;
	/// 密度を返す
	int		GetDensity() const;
	/// 密度を設定
	void	SetDensity(int val);

	/// ディスクサイズ（ヘッダサイズ含む）
	wxUint32 GetSize() const;
	/// ディスクサイズ（ヘッダサイズ含む）を設定
	void	SetSize(wxUint32 val);
	/// ディスクサイズ（ヘッダサイズを除く）
	wxUint32 GetSizeWithoutHeader() const;
	/// ディスクサイズ（ヘッダサイズを除く）を設定
	void	SetSizeWithoutHeader(wxUint32 val);

	/// オフセット値を返す
	wxUint32 GetOffset(int num) const;
	/// オフセット値を設定
	void	SetOffset(int num, wxUint32 offset);
	/// ヘッダサイズを除いたオフセット値を設定
	void	SetOffsetWithoutHeader(int num, wxUint32 offset);

	/// 作成可能なトラック数を返す (DiskD88::GetCreatableTracks())
	int		GetCreatableTracks() const { return DISKD88_MAX_TRACKS; }

	/// 変更済みに設定
	void	SetModify();
	/// 変更されているか
	bool	IsModified();
	/// 変更済みをクリア
	void	ClearModify();
};

// ----------------------------------------------------------------------

WX_DEFINE_ARRAY(DiskD88Disk *, DiskD88Disks);

// ----------------------------------------------------------------------

/// ディスクイメージへのポインタを保持するクラス
class DiskD88File : public DiskImageFile
{
private:
	DiskD88File();
	DiskD88File(const DiskD88File &src);

public:
	DiskD88File(DiskImage &image);
	~DiskD88File();

	/// インスタンス作成
	DiskImageDisk *NewImageDisk(int n_num);
	/// インスタンス作成
	DiskImageDisk *NewImageDisk(int n_num, const DiskParam &n_param, const wxString &n_diskname, bool n_write_protect);
	/// インスタンス作成
	DiskImageDisk *NewImageDisk(int n_num, const DiskImageDiskHeader &n_header);
};

// ----------------------------------------------------------------------

/// D88ディスクイメージ入出力
class DiskD88 : public DiskImage
{
private:
#ifdef DISKD88_USE_MEMORY_INPUT_STREAM
	wxMemoryInputStream *stream;
	bool OpenStream(wxInputStream &src);
	void CloseStream();
#endif

public:
	DiskD88();
	~DiskD88();

	/// インスタンス作成
	DiskImageFile *NewImageFile();

	/// 作成可能なトラック数を返す
	int	GetCreatableTracks() const { return DISKD88_MAX_TRACKS; }

	/// 密度文字列を返す
	int		GetDensityNames(wxArrayString &arr) const;
	/// 密度リストを検索
	int		FindDensity(int val) const;
	/// 密度リストを検索
	int		FindDensityByIndex(int idx) const;
	/// 密度リストの指定位置の値を返す
	wxUint8	GetDensity(int idx) const;
};

#endif /* DISKD88_H */
