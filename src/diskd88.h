/// @file diskd88.h
///
/// @brief D88ディスクイメージ入出力
///
#ifndef _DISKD88_H_
#define _DISKD88_H_

#include "common.h"
#include <wx/string.h>
#include <wx/filename.h>
#include <wx/mstream.h>
#include <wx/dynarray.h>
#include <wx/hashmap.h>
#include "diskparam.h"
#include "diskresult.h"

/// disk density 0: 2D, 1: 2DD, 2: 2HD
extern const char *gDiskDensity[];

#define DISKD88_MAX_TRACKS	164

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


WX_DECLARE_HASH_MAP( int, int, wxIntegerHash, wxIntegerEqual, IntHashMap );

/// ハッシュを扱うクラス
class IntHashMapUtil
{
public:
	static void IncleaseValue(IntHashMap &hash_map, int key);
	static int GetMaxKeyOnMaxValue(IntHashMap &hash_map);
	static int MaxValue(int src, int value);
	static int MinValue(int src, int value);
};

/// セクタデータへのポインタを保持するクラス
class DiskD88Sector
{
private:
	int num;		///< sector number(ID Rと同じ)
	bool deleted;	///< deleted mark
	d88_sector_header_t header;
	wxUint8 *data;

	bool modified;	///< 変更したか

//	bool fat_area;	///< FATorディレクトリ管理エリアか？

	d88_sector_header_t header_origin;
	wxUint8 *data_origin;

	DiskD88Sector();
	DiskD88Sector(const DiskD88Sector &src) {}
	DiskD88Sector &operator=(const DiskD88Sector &src) { return *this; }

public:
	DiskD88Sector(int newnum, const d88_sector_header_t &newhdr, wxUint8 *newdata);
	DiskD88Sector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density = false);
	~DiskD88Sector();

	/// セクタのデータを置き換える
	bool	Replace(DiskD88Sector *src_sector);
	/// セクタのデータを埋める
	bool	Fill(wxUint8 code); 
	/// セクタのデータを上書き
	bool	Copy(const wxUint8 *buf, size_t len); 
	/// セクタのデータを上書き
	bool	Copy(const char *buf, size_t len); 
	/// 指定位置のセクタデータを返す
	wxUint8	Get(int pos) const;
	/// セクタサイズを変更
	void	ModifySectorSize(int size);

	/// セクタ番号を返す(ID Rと同じ)
	int		GetSectorNumber() const { return num; }
	/// 削除マークがついているか
	bool	IsDeleted() const { return deleted; }
	/// 削除マークの設定
	void	SetDeletedMark(bool val);
	/// 同じセクタか
	bool	IsSameSector(int sector_number, bool deleted_mark = false);
	/// セクタサイズを返す
	int		GetSectorSize() const;
	/// セクタサイズを設定
	void	SetSectorSize(int val);
	/// セクタサイズ（バッファのサイズ）を返す
	int		GetSectorBufferSize() const { return header.size; }
	/// セクタデータへのポインタを返す
	wxUint8 *GetSectorBuffer() { return data; }
	/// セクタ数を返す
	wxUint16 GetSectorsPerTrack() const { return header.secnums; }
	/// セクタ数を設定
	void	SetSectorsPerTrack(wxUint16 val) { header.secnums = val; }

	/// ヘッダを返す
	d88_sector_header_t	*GetHeader() { return &header; }
	/// ID Cを返す
	wxUint8	GetIDC() const { return header.id.c; }
	/// ID Hを返す
	wxUint8	GetIDH() const { return header.id.h; }
	/// ID Rを返す
	wxUint8	GetIDR() const { return header.id.r; }
	/// ID Nを返す
	wxUint8	GetIDN() const { return header.id.n; }
	/// ID Cを設定
	void	SetIDC(wxUint8 val) { header.id.c = val; }
	/// ID Hを設定
	void	SetIDH(wxUint8 val) { header.id.h = val; }
	/// ID Rを設定
	void	SetIDR(wxUint8 val) { header.id.r = val; }
	/// ID Nを設定
	void	SetIDN(wxUint8 val) { header.id.n = val; }
	/// 単密度か
	bool	IsSingleDensity();
	/// 単密度かを設定
	void	SetSingleDensity(bool val);

	/// 変更されているか
	bool	IsModified() const { return modified; }
	/// 変更済みを設定
	void	SetModify();
	/// 変更済みをクリア
	void	ClearModify();

//	bool	IsFATArea() const { return fat_area; }
//	void	SetFATArea(bool val) { fat_area = val; }

	/// セクタ内容の比較
	static int Compare(DiskD88Sector *item1, DiskD88Sector *item2);
	/// セクタ番号の比較
	static int CompareIDR(DiskD88Sector **item1, DiskD88Sector **item2);
	/// ID Nからセクタサイズを計算
	static int ConvIDNToSecSize(wxUint8 n);
	/// セクタサイズからID Nを計算
	static wxUint8 ConvSecSizeToIDN(int size);
};

WX_DEFINE_ARRAY(DiskD88Sector *, DiskD88Sectors);

class DiskD88Disk;

/// トラックデータへのポインタを保持するクラス
class DiskD88Track
{
private:
	DiskD88Disk *parent;
	int trk_num;	///< track number
	int sid_num;	///< side number
	int offset_pos;	///< position of offset table in header
//	wxUint32 offset;	///< offset on diskimage
	wxUint32 size;	///< track size
	int interleave;	///< interleave of sector
	DiskD88Sectors *sectors;

	DiskD88Track() {}
	DiskD88Track(const DiskD88Track &src) {}
	DiskD88Track &operator=(const DiskD88Track &src) { return *this; }

public:
	DiskD88Track(DiskD88Disk *disk);
//	DiskD88Track(DiskD88Disk *disk, int newtrknum, int newsidnum, int newoffpos, wxUint32 newoffset, int newinterleave);
	DiskD88Track(DiskD88Disk *disk, int newtrknum, int newsidnum, int newoffpos, int newinterleave);
//	DiskD88Track(DiskD88Disk *disk, int newnum, int newsidnum, wxUint32 newoffset, DiskD88Sectors *newsecs);
	~DiskD88Track();

	/// セクタを追加する
	size_t	Add(DiskD88Sector *newsec);
	/// トラック内のセクタデータを置き換える
	int		Replace(DiskD88Track *src_track); 
	/// トラック内の指定セクタを削除する
	int		DeleteSectors(int start_sector_num, int end_sector_num);
	/// トラックサイズの再計算
	wxUint32 Shrink();
	/// トラック番号を返す
	int		GetTrackNumber() const { return trk_num; }
	/// サイド番号を返す
	int		GetSideNumber() const { return sid_num; }
	/// サイド番号を設定
	void	SetSideNumber(int val) { sid_num = val; }

//	wxUint32	GetOffset() const { return offset; }
//	void		SetOffset(wxUint32 val) { offset = val; }

	/// オフセットを返す
	int		GetOffsetPos() const { return offset_pos; }
	/// トラック内の最小セクタ番号を返す
	int		GetMinSectorNumber() const;
	/// トラック内の最大セクタ番号を返す
	int		GetMaxSectorNumber() const;
	/// トラック内の最大セクタサイズを返す
	int		GetMaxSectorSize() const;
	/// トラックサイズを返す
	wxUint32 GetSize() const { return size; }
	/// トラックサイズを設定
	void	SetSize(wxUint32 val) { size = val; }
	/// インターリーブを返す
	int		GetInterleave() const { return interleave; }
	/// インターリーブを設定
	void	SetInterleave(int val) { interleave = val; }

	/// セクタリストを返す
	DiskD88Sectors *GetSectors() const { return sectors; }
	/// 指定セクタ番号のセクタを返す
	DiskD88Sector  *GetSector(int sector_number);

	/// トラック内のすべてのID Cを変更
	void	SetAllIDC(wxUint8 val);
	/// トラック内のすべてのID Hを変更
	void	SetAllIDH(wxUint8 val);
	/// トラック内のすべてのID Rを変更
	void	SetAllIDR(wxUint8 val);
	/// トラック内のすべてのID Nを変更
	void	SetAllIDN(wxUint8 val);
	/// トラック内のすべての密度を変更
	void	SetAllSingleDensity(bool val);
	/// トラック内のすべてのセクタ数を変更
	void	SetAllSectorsPerTrack(int val);
	/// トラック内のすべてのセクタサイズを変更
	void	SetAllSectorSize(int val);

	/// 変更されているか
	bool	IsModified() const;
	/// 変更済みをクリア
	void	ClearModify();

//	void SetFATArea(bool val);

	/// トラック番号とサイド番号の比較
	static int Compare(DiskD88Track *item1, DiskD88Track *item2);
};

WX_DEFINE_ARRAY(DiskD88Track *, DiskD88Tracks);

class DiskD88File;
class DiskBasicParam;

/// １ディスクへのポインタを保持するクラス
class DiskD88Disk : public DiskParam
{
private:
	DiskD88File *parent;
	int num;	///< disk number

	d88_header_t header;
	wxUint32 offset_start;	///< usually header size
	DiskD88Tracks *tracks;
	int max_track_number;

	d88_header_t header_origin;

//	int max_track_num;
//	wxUint8 *buffer;
//	size_t buffer_size;

	bool modified;	///< 変更したか

	DiskBasicParam *basic_param;

	DiskD88Disk() : DiskParam() {}
	DiskD88Disk(const DiskD88Disk &src) : DiskParam(src) {}
	DiskD88Disk &operator=(const DiskD88Disk &src) { return *this; }

public:
	DiskD88Disk(DiskD88File *file);
	DiskD88Disk(DiskD88File *file, const wxString &newname, int newnum, const DiskParam &param, bool write_protect);
	DiskD88Disk(DiskD88File *file, int newnum, const d88_header_t &newhdr);
//	DiskD88Disk(wxUint8 *newbuf, size_t newbuflen, int newnum);
//	DiskD88Disk(int newnum, d88_header_t *newhdr, DiskD88Tracks *newtrks);
	~DiskD88Disk();

	/// ディスクにトラックを追加
	size_t	Add(DiskD88Track *newtrk);
	/// ディスクの内容を置き換える
	int		Replace(int side_number, DiskD88Disk *src_disk, int src_side_number);
	/// トラックを削除する
	void	DeleteTracks(int start_offset_pos, int end_offset_pos, int side_number);
	/// オフセットの再計算＆ディスクサイズ変更
	size_t	Shrink();
	/// ディスクサイズ計算（ディスクヘッダ分を除く）
	size_t	CalcSizeWithoutHeader();

	/// ディスク番号を返す
	int		GetNumber() const { return num; }
	/// ディスク名を返す
	wxString GetName(bool real = false) const;
	/// ディスク名を設定
	void	SetName(const wxString &val);
	/// ディスク名を設定
	void	SetName(const wxUint8 *buf, size_t len);

//	int		GetDiskType() { return disk_type; }

	/// ヘッダを返す
	d88_header_t *GetHeader() { return &header; }
	/// ディスクファイルを返す
	DiskD88File   *GetFile() const { return parent; }
	/// トラックリストを返す
	DiskD88Tracks *GetTracks() const { return tracks; }
	/// 指定トラックを返す
	DiskD88Track  *GetTrack(int track_number, int side_number);
	/// 指定オフセット値からトラックを返す
	DiskD88Track  *GetTrackByOffset(wxUint32 offset);
	/// 指定セクタを返す
	DiskD88Sector *GetSector(int track_number, int side_number, int sector_number);
	/// ディスクの中でもっともらしいパラメータを設定
	const DiskParam *CalcMajorNumber();

	/// ディスクパラメータを文字列にフォーマットして返す
	wxString GetAttrText() const;

	/// 書き込み禁止かどうかを返す
	bool	IsWriteProtected() const;
	/// 書き込み禁止かどうかを設定
	void	SetWriteProtect(bool val);

	/// 密度を返す
	wxString GetDensityText() const;
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

	/// offset最小値 -> トラックデータの開始位置を返す
	wxUint32 GetOffsetStart() const;
	/// offset最小値 -> トラックデータの開始位置を設定
	void	SetOffsetStart(wxUint32 val);

	/// 最大トラック番号 offsetがNULLでない最大位置を返す
	int		GetMaxTrackNumber() const;
	/// 最大トラック番号 offsetがNULLでない最大位置を設定
	void	SetMaxTrackNumber(int pos);

	/// ディスクの内容を初期化する(0パディング)
	bool	Initialize(int selected_side);
	/// ディスクのトラックを作り直す
	bool	Rebuild(const DiskParam &param, int selected_side);

//	wxUint8 *GetBuffer() { return buffer; }
//	size_t GetBufferSize() { return buffer_size; }

	/// 変更済みに設定
	void	SetModify();
	/// 変更されているか
	bool	IsModified();
	/// 変更済みをクリア
	void	ClearModify();

	/// トラックが存在するか
	bool	ExistTrack(int side_number);

//	void	SetMaxTrackNumber(int track_number);
//	int		GetMaxTrackNumber();

	/// DISK BASICパラメータを設定
	void	SetDiskBasicParam(DiskBasicParam *val) { basic_param = val; }
	/// DISK BASICパラメータを返す
	DiskBasicParam *GetDiskBasicParam() { return basic_param; }
//	void	SetFATArea(bool val);

	/// ディスク番号を比較
	static int Compare(DiskD88Disk *item1, DiskD88Disk *item2);
};

WX_DEFINE_ARRAY(DiskD88Disk *, DiskD88Disks);

/// ディスクイメージへのポインタを保持するクラス
class DiskD88File
{
private:
	DiskD88Disks *disks;
	wxArrayShort *mods;	///< 変更フラグ 追加したかどうか

	wxString basic_type_hint;	///< BASIC種類ヒント

	DiskD88File(const DiskD88File &src);

public:
	DiskD88File();
	~DiskD88File();

	/// 変更フラグ 追加したかどうか
	enum {
		MODIFY_NONE = 0,
		MODIFY_ADD = 1
	};

	size_t Add(DiskD88Disk *newdsk, short mod_flags);
	void Clear();
	size_t Count() const;
	bool Delete(size_t idx);

	DiskD88Disks *GetDisks() { return disks; }
	DiskD88Disk  *GetDisk(size_t idx);

//	void SetModify();
	bool IsModified();
	void ClearModify();

	const wxString &GetBasicTypeHint() const { return basic_type_hint; }
	void SetBasicTypeHint(const wxString &val) { basic_type_hint = val; }
};

/// D88ディスクイメージ入出力
class DiskD88
{
private:
	wxFileName filename;
	DiskD88File *file;
	DiskResult result;

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

	/// 新規作成
	int Create(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint);
	/// 追加で新規作成
	int Add(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint);
	/// ファイルを追加
	int Add(const wxString &filepath, const wxString &file_format, const wxString &disk_type);
	/// ファイルを開く
	int Open(const wxString &filepath, const wxString &file_format, const wxString &disk_type);
	/// 閉じる
	void Close();
	/// ストリームの内容をファイルに保存
	int Save(const wxString &filepath);
	/// ストリームの内容をファイルに保存
	int SaveDisk(int disk_number, int side_number, const wxString &filepath);
	/// ディスクを削除
	bool Delete(size_t disk_number);
	/// ファイルでディスクを置換
	int ReplaceDisk(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const wxString &disk_type);

	/// ディスク名を設定
	bool SetDiskName(size_t disk_number, const wxString &newname);
	/// ディスク名を返す
	wxString GetDiskName(size_t disk_number, bool real = false) const;

	/// ディスクを変更したか
	bool IsModified();

	/// ディスクファイルを返す
	DiskD88File			*GetFile() { return file; }
	/// ディスクファイルを返す
	const DiskD88File	*GetFile() const { return file; }
	/// ディスク枚数
	size_t CountDisks() const;
	/// ディスク一覧を返す
	DiskD88Disks *GetDisks();
	/// 指定した位置のディスクを返す
	DiskD88Disk			*GetDisk(size_t index);
	/// 指定した位置のディスクを返す
	const DiskD88Disk	*GetDisk(size_t index) const;
	/// 指定した位置のディスクのタイプ
	int GetDiskTypeNumber(size_t index) const;

	/// ファイル名を返す
	wxString GetFileName() const;
	/// ファイル名ベースを返す
	wxString GetFileNameBase() const;
	/// ファイルパスを返す
	wxString GetFilePath() const;
	/// パスを返す
	wxString GetPath() const;

	/// ファイル名を設定
	void SetFileName(const wxString &path);

	/// エラーメッセージ
	const wxArrayString &GetErrorMessage(int maxrow = 20);
	/// エラーメッセージを表示
	void  ShowErrorMessage();
};

#endif /* _DISKD88_H_ */
