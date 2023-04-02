/// @file diskd88.h
///
/// @brief D88ディスクイメージ入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKD88_H_
#define _DISKD88_H_

#include "common.h"
#include <wx/string.h>
#include <wx/filename.h>
//#include <wx/mstream.h>
#include <wx/dynarray.h>
#include <wx/hashmap.h>
#include "diskparam.h"
#include "diskimg/diskresult.h"

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
	int					 num;		///< sector number(ID Rと同じ)
	d88_sector_header_t *header;	///< sector header
	wxUint8 			*data;		///< sector data

//	bool modified;	///< 変更したか

	d88_sector_header_t  header_origin;	///< pre-save header
	wxUint8				*data_origin;	///< pre-save data

	DiskD88Sector();
	DiskD88Sector(const DiskD88Sector &src) {}
	DiskD88Sector &operator=(const DiskD88Sector &src) { return *this; }
	/// 変更されているか
	bool	IsModifiedBase() const;

public:
	DiskD88Sector(int n_num, d88_sector_header_t *n_header, wxUint8 *n_data);
	DiskD88Sector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density = false, int status = 0);
	~DiskD88Sector();

	/// セクタのデータを置き換える
	bool	Replace(DiskD88Sector *src_sector);
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

	/// セクタ番号を返す(ID Rと同じ)
	int		GetSectorNumber() const { return num; }
	/// セクタ番号を設定
	void	SetSectorNumber(int val);
	/// 削除マークがついているか
	bool	IsDeleted() const;
	/// 削除マークの設定
	void	SetDeletedMark(bool val);
	/// 同じセクタか
	bool	IsSameSector(int sector_number, int density, bool deleted_mark = false);
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
	d88_sector_header_t	*GetHeader() { return header; }
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

	/// 変更されているか
	bool	IsModified() const;
	/// 変更済みを設定
	void	SetModify();
	/// 変更済みをクリア
	void	ClearModify();

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
	wxUint32 size;	///< track size
	int interleave;	///< interleave of sector
	DiskD88Sectors *sectors;

	size_t orig_sectors;	///< num of sectors (original / pre save)

	wxUint8 *extra_data;	///< extra data
	size_t extra_size;		///< extra data size

	DiskD88Track() {}
	DiskD88Track(const DiskD88Track &src) {}
	DiskD88Track &operator=(const DiskD88Track &src) { return *this; }

public:
	DiskD88Track(DiskD88Disk *disk);
	DiskD88Track(DiskD88Disk *disk, int newtrknum, int newsidnum, int newoffpos, int newinterleave);
	~DiskD88Track();

	/// セクタを追加する
	size_t	Add(DiskD88Sector *newsec);
	/// トラック内のセクタデータを置き換える
	int		Replace(DiskD88Track *src_track); 
	/// トラックに新規セクタを追加する
	int		AddNewSector(int trknum, int sidnum, int secnum, int secsize, bool sdensity, int status);
	/// トラック内の指定位置のセクタを削除する
	int		DeleteSectorByIndex(int pos);
	/// トラック内の指定セクタを削除する
	int		DeleteSectors(int start_sector_num, int end_sector_num);
	/// トラックサイズの再計算
	wxUint32 Shrink(bool trim_unused_data);
	/// トラックサイズの再計算&オフセット計算
	void	ShrinkAndCalcOffsets(bool trim_unused_data);
	/// 余りバッファ領域のサイズを増やす
	void	IncreaseExtraDataSize(size_t size);
	/// 余りバッファ領域のサイズを減らす
	void	DecreaseExtraDataSize(size_t size);

	/// トラック番号を返す
	int		GetTrackNumber() const { return trk_num; }
	/// トラック番号を設定
	void	SetTrackNumber(int val) { trk_num = val; }
	/// サイド番号を返す
	int		GetSideNumber() const { return sid_num; }
	/// サイド番号を設定
	void	SetSideNumber(int val) { sid_num = val; }

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
	/// インターリーブを計算して設定
	void	CalcInterleave();

	/// セクタリストを返す
	DiskD88Sectors *GetSectors() const { return sectors; }
	/// セクタ数を返す
	int		GetSectorsPerTrack() const;
	/// 指定セクタ番号のセクタを返す
	DiskD88Sector  *GetSector(int sector_number, int density = -1);
	/// 指定位置のセクタを返す
	DiskD88Sector  *GetSectorByIndex(int pos);

	/// トラック内のもっともらしいID Cを返す
	wxUint8	GetMajorIDC() const;
	/// トラック内のもっともらしいID Hを返す
	wxUint8	GetMajorIDH() const;

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

	/// 余分なデータを設定する
	void	SetExtraData(wxUint8 *buf, size_t size);
	/// 余分なデータを返す
	wxUint8 *GetExtraData() const { return extra_data; }
	/// 余分なデータのサイズを返す
	size_t	GetExtraDataSize() const { return extra_size; }

	/// 変更されているか
	bool	IsModified() const;
	/// 変更済みをクリア
	void	ClearModify();

	/// トラック番号とサイド番号の比較
	static int Compare(DiskD88Track *item1, DiskD88Track *item2);
	/// インターリーブを考慮したセクタ番号リストを返す
	static bool CalcSectorNumbersForInterleave(int interleave, size_t sectors_count, wxArrayInt &sector_nums, int sector_offset = 0);
};

WX_DEFINE_ARRAY(DiskD88Track *, DiskD88Tracks);

class DiskD88File;
class DiskBasicParam;
class DiskBasic;
class DiskBasics;

/// １ディスクへのポインタを保持するクラス
class DiskD88Disk : public DiskParam
{
private:
	DiskD88File *parent;
	int num;				///< disk number

	d88_header_t *header;	///< disk header
	wxUint32 offset_start;	///< usually header size
	DiskD88Tracks *tracks;
	int max_track_number;

	d88_header_t header_origin;

	bool modified;	///< 変更したか

	DiskParam orig_param;	///< 解析したパラメータ
	bool param_changed;		///< ディスクパラメータを変更したか

	DiskBasics *basics;

	DiskD88Disk() : DiskParam() {}
	DiskD88Disk(const DiskD88Disk &src) : DiskParam(src) {}
	DiskD88Disk &operator=(const DiskD88Disk &src) { return *this; }

public:
	DiskD88Disk(DiskD88File *file, int n_num);
	DiskD88Disk(DiskD88File *file, const wxString &newname, int newnum, const DiskParam &param, bool write_protect);
	DiskD88Disk(DiskD88File *file, int n_num, d88_header_t *n_header);
	~DiskD88Disk();

	/// ディスクにトラックを追加
	size_t	Add(DiskD88Track *newtrk);
	/// ディスクの内容を置き換える
	int		Replace(int side_number, DiskD88Disk *src_disk, int src_side_number);
	/// ディスクにトラックを追加
	int		AddNewTrack(int side_number);
	/// トラックを削除する
	void	DeleteTracks(int start_offset_pos, int end_offset_pos, int side_number);
	/// トラックサイズ＆オフセットの再計算＆ディスクサイズ変更
	size_t	ShrinkTracks(bool trim_unused_data);
	/// オフセットの再計算＆ディスクサイズ変更
	size_t	CalcOffsets();
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

	/// ヘッダを返す
	d88_header_t *GetHeader() { return header; }
	/// ディスクファイルを返す
	DiskD88File   *GetFile() const { return parent; }
	/// トラックリストを返す
	DiskD88Tracks *GetTracks() const { return tracks; }
	/// 指定トラックを返す
	DiskD88Track  *GetTrack(int track_number, int side_number);
	/// 指定トラックを返す
	DiskD88Track  *GetTrack(int index);
	/// 指定オフセット値からトラックを返す
	DiskD88Track  *GetTrackByOffset(wxUint32 offset);
	/// 指定セクタを返す
	DiskD88Sector *GetSector(int track_number, int side_number, int sector_number, int density = -1);
	/// ディスクの中でもっともらしいパラメータを設定
	const DiskParam *CalcMajorNumber();

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
	/// 密度を検索
	static int FindDensity(int val);

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

	/// 変更済みに設定
	void	SetModify();
	/// 変更されているか
	bool	IsModified();
	/// 変更済みをクリア
	void	ClearModify();

	/// トラックが存在するか
	bool	ExistTrack(int side_number);

	/// 変更前パラメータを設定
	void	SetOriginalParam(const DiskParam &val) { orig_param = val; }
	/// 変更前パラメータを返す
	const DiskParam &GetOriginalParam() const { return orig_param; }
	/// パラメータ変更フラグを設定
	void	SetParamChanged(bool val) { param_changed = val; }
	/// パラメータ変更フラグを返す
	bool	GetParamChanged() const { return param_changed; }

	/// DISK BASIC領域を確保
	void	AllocDiskBasics();
	/// DISK BASICを返す
	DiskBasic *GetDiskBasic(int idx);
	/// DISK BASICを返す
	DiskBasics *GetDiskBasics() { return basics; }
	/// DISK BASICをクリア
	void	ClearDiskBasics();
	/// キャラクターコードマップ設定
	void	SetCharCode(const wxString &name);

	/// ディスク番号を比較
	static int Compare(DiskD88Disk *item1, DiskD88Disk *item2);
};

WX_DEFINE_ARRAY(DiskD88Disk *, DiskD88Disks);

/// ディスクライト時のオプション
class DiskWriteOptions
{
protected:
	bool trim_unused_data;
public:
	DiskWriteOptions();
	DiskWriteOptions(
		bool n_trim_unused_data
	);
	virtual ~DiskWriteOptions();
	bool IsTrimUnusedData() const { return trim_unused_data; }
};

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
	int Add(const wxString &filepath, const wxString &file_format, const DiskParam &param_hint);
	/// ファイルを開く
	int Open(const wxString &filepath, const wxString &file_format, const DiskParam &param_hint);
	/// ファイルを開く前のチェック
	int Check(const wxString &filepath, wxString &file_format, DiskParamPtrs &params, DiskParam &manual_param);
	/// 閉じる
	void Close();
	/// ストリームの内容をファイルに保存できるか
	int CanSave(const wxString &file_format);
	/// ストリームの内容をファイルに保存
	int Save(const wxString &filepath, const wxString &file_format, const DiskWriteOptions &options);
	/// ストリームの内容をファイルに保存
	int SaveDisk(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const DiskWriteOptions &options);
	/// ディスクを削除
	bool Delete(int disk_number);
	/// 置換元のディスクを解析
	int ParseForReplace(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const DiskParam &param_hint, DiskD88File &src_file, DiskD88Disk* &tag_disk);
	/// ファイルでディスクを置換
	int ReplaceDisk(int disk_number, int side_number, DiskD88Disk *src_disk, int src_side_number, DiskD88Disk *tag_disk);

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

	/// DISK BASICが一致するか
	bool MatchDiskBasic(const DiskBasic *target);
	/// DISK BASICの解析状態をクリア
	void ClearDiskBasicParseAndAssign(int disk_number, int side_number);
	/// キャラクターコードマップ設定
	void SetCharCode(const wxString &name);

	/// エラーメッセージ
	const wxArrayString &GetErrorMessage(int maxrow = 20);
	/// エラーメッセージを表示
	void  ShowErrorMessage();
	/// エラー警告メッセージを表示
	int   ShowErrWarnMessage();
};

#endif /* _DISKD88_H_ */
