/// @file diskimage.h
///
/// @brief ディスクイメージ入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKIMAGE_H
#define DISKIMAGE_H

#include "../common.h"
#include <wx/string.h>
#include <wx/filename.h>
//#include <wx/mstream.h>
#include <wx/dynarray.h>
#include <wx/hashmap.h>
#include "diskparam.h"
#include "diskresult.h"

class DiskBasicParam;
class DiskBasic;
class DiskBasics;
class DiskWriteOptions;
class DiskImageDisk;
class DiskImageFile;
class DiskImage;

// ----------------------------------------------------------------------

WX_DECLARE_HASH_MAP( int, int, wxIntegerHash, wxIntegerEqual, IntHashMap );

/// ハッシュを扱うクラス
class IntHashMapUtil
{
public:
	static void IncleaseValue(IntHashMap &hash_map, int key);
	static int GetMaxKeyOnMaxValue(IntHashMap &hash_map);
	static int GetKeyCount(IntHashMap &hash_map);
	static int MaxValue(int src, int value);
	static int MinValue(int src, int value);
};

// ----------------------------------------------------------------------

/// セクタデータへのヘッダ部分を渡すクラス
class DiskImageSectorHeader
{
public:
	DiskImageSectorHeader() {}
	virtual ~DiskImageSectorHeader() {}
	virtual int GetHeaderType() const = 0;
};

// ----------------------------------------------------------------------

/// セクタデータへのポインタを保持するクラス
class DiskImageSector
{
protected:
	int m_num;		///< sector number(ID Rと同じ)

	DiskImageSector() {}
	DiskImageSector(const DiskImageSector &src) {}
	DiskImageSector &operator=(const DiskImageSector &src) { return *this; }

public:
	DiskImageSector(int n_num);
	virtual ~DiskImageSector();

	/// セクタのデータを置き換える
	virtual bool	Replace(DiskImageSector *src_sector) { return false; }
	/// セクタのデータを埋める
	virtual bool	Fill(wxUint8 code, int len = -1, int start = 0) { return false; }
	/// セクタのデータを上書き
	virtual bool	Copy(const void *buf, int len, int start = 0) { return false; }
	/// セクタのデータに指定したバイト列があるか
	virtual int		Find(const void *buf, size_t len) { return -1; }
	/// 指定位置のセクタデータを返す
	virtual wxUint8	Get(int pos) const { return 0; }
	/// 指定位置のセクタデータを返す
	virtual wxUint16 Get16(int pos, bool big_endian = false) const { return 0; }
	/// セクタサイズを変更
	virtual int		ModifySectorSize(int size) { return 0; }

	/// セクタ番号を返す(ID Rと同じ)
	virtual int		GetSectorNumber() const { return m_num; }
	/// セクタ番号を設定
	virtual void	SetSectorNumber(int val) { m_num = val; }
	/// 削除マークがついているか
	virtual bool	IsDeleted() const { return false; }
	/// 削除マークの設定
	virtual void	SetDeletedMark(bool val) {}
	/// 同じセクタか
	virtual bool	IsSameSector(int sector_number, int density, bool deleted_mark = false) { return false; }
	/// ヘッダサイズを返す
	virtual int		GetHeaderSize() const { return 0; }
	/// セクタサイズを返す
	virtual int		GetSectorSize() const { return 0; }
	/// セクタサイズを設定
	virtual void	SetSectorSize(int val) {}
	/// セクタサイズ（バッファのサイズ）を返す
	virtual int		GetSectorBufferSize() const = 0;
	/// セクタサイズ（ヘッダ＋バッファのサイズ）を返す
	virtual int		GetSize() const { return 0; }
	/// セクタデータへのポインタを返す
	virtual wxUint8 *GetSectorBuffer() = 0;
	/// セクタデータへのポインタを返す
	virtual wxUint8 *GetSectorBuffer(int offset) { return NULL; }
	/// セクタ数を返す
	virtual wxUint16 GetSectorsPerTrack() const { return 0; }
	/// セクタ数を設定
	virtual void	SetSectorsPerTrack(wxUint16 val) {}
	/// セクタのステータスを返す
	virtual wxUint8 GetSectorStatus() const { return 0; }
	/// セクタのステータスを設定
	virtual void    SetSectorStatus(wxUint8 val) {}

	/// ヘッダを返す
	virtual DiskImageSectorHeader *GetHeader() { return NULL; }
	/// ID Cを返す
	virtual wxUint8	GetIDC() const { return 0; }
	/// ID Hを返す
	virtual wxUint8	GetIDH() const { return 0; }
	/// ID Rを返す
	virtual wxUint8	GetIDR() const { return 0; }
	/// ID Nを返す
	virtual wxUint8	GetIDN() const { return 0; }
	/// ID Cを設定
	virtual void	SetIDC(wxUint8 val) {}
	/// ID Hを設定
	virtual void	SetIDH(wxUint8 val) {}
	/// ID Rを設定
	virtual void	SetIDR(wxUint8 val) {}
	/// ID Nを設定
	virtual void	SetIDN(wxUint8 val) {}
	/// 単密度か
	virtual bool	IsSingleDensity() { return false; }
	/// 単密度かを設定
	virtual void	SetSingleDensity(bool val) {}

	/// CRCを返す
	virtual int		GetRecordedCRC() const { return -1; }
	/// CRCを計算する
	virtual int		CalculateCRC() { return -1; }
	/// CRCを設定
	virtual void 	SetRecordedCRC(wxUint16 crc) {}

	/// 変更されているか
	virtual bool	IsModified() const { return false; }
	/// 変更済みを設定
	virtual void	SetModify() {}
	/// 変更済みをクリア
	virtual void	ClearModify() {}

	/// セクタ内容の比較
	static int		Compare(DiskImageSector *item1, DiskImageSector *item2);
	/// セクタ番号の比較
	static int		CompareIDR(DiskImageSector **item1, DiskImageSector **item2);
	/// ID Nからセクタサイズを計算
	static int		ConvIDNToSecSize(wxUint8 n);
	/// セクタサイズからID Nを計算
	static wxUint8	ConvSecSizeToIDN(int size);
};

// ----------------------------------------------------------------------

WX_DEFINE_ARRAY(DiskImageSector *, DiskImageSectors);

// ----------------------------------------------------------------------

class DiskImageDisk;

/// トラックデータへのポインタを保持するクラス
class DiskImageTrack
{
protected:
	DiskImageDisk *parent;
	int m_trk_num;		///< track number
	int m_sid_num;		///< side number
	int m_offset_pos;	///< position of offset table in header
	wxUint32 m_size;	///< track size
	int m_interleave;	///< interleave of sector
	DiskImageSectors *sectors;

	size_t m_orig_sectors;	///< num of sectors (original / pre save)

	wxUint8 *extra_data;	///< extra data
	size_t extra_size;		///< extra data size

	DiskImageTrack() {}
	DiskImageTrack(const DiskImageTrack &src) {}
	DiskImageTrack &operator=(const DiskImageTrack &src) { return *this; }

public:
	DiskImageTrack(DiskImageDisk *disk);
	DiskImageTrack(DiskImageDisk *disk, int n_trk_num, int n_sid_num, int n_offset_pos, int n_interleave);
	virtual ~DiskImageTrack();

	/// インスタンス作成
	virtual DiskImageSector *NewImageSector(int n_num, const DiskImageSectorHeader &n_header, wxUint8 *n_data) = 0;
	/// インスタンス作成
	virtual DiskImageSector *NewImageSector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density = false, int status = 0) = 0;

	/// セクタを追加する
	virtual size_t	Add(DiskImageSector *newsec);
	/// トラック内のセクタデータを置き換える
	virtual int		Replace(DiskImageTrack *src_track); 
	/// トラックに新規セクタを追加する
	virtual int		AddNewSector(int trknum, int sidnum, int secnum, int secsize, bool sdensity, int status);
	/// トラック内の指定位置のセクタを削除する
	virtual int		DeleteSectorByIndex(int pos);
	/// トラック内の指定セクタを削除する
	virtual int		DeleteSectors(int start_sector_num, int end_sector_num);
	/// トラックサイズの再計算
	virtual wxUint32 Shrink(bool trim_unused_data);
	/// トラックサイズの再計算&オフセット計算
	virtual void	ShrinkAndCalcOffsets(bool trim_unused_data);
	/// 余りバッファ領域のサイズを増やす
	virtual void	IncreaseExtraDataSize(size_t size);
	/// 余りバッファ領域のサイズを減らす
	virtual void	DecreaseExtraDataSize(size_t size);

	/// トラック番号を返す
	virtual int		GetTrackNumber() const { return m_trk_num; }
	/// トラック番号を設定
	virtual void	SetTrackNumber(int val) { m_trk_num = val; }
	/// サイド番号を返す
	virtual int		GetSideNumber() const { return m_sid_num; }
	/// サイド番号を設定
	virtual void	SetSideNumber(int val) { m_sid_num = val; }

	/// オフセットを返す
	virtual int		GetOffsetPos() const { return m_offset_pos; }
	/// トラック内の最小セクタ番号を返す
	virtual int		GetMinSectorNumber() const;
	/// トラック内の最大セクタ番号を返す
	virtual int		GetMaxSectorNumber() const;
	/// トラック内の最大セクタサイズを返す
	virtual int		GetMaxSectorSize() const;
	/// トラックサイズを返す
	virtual wxUint32 GetSize() const { return m_size; }
	/// トラックサイズを設定
	virtual void	SetSize(wxUint32 val) { m_size = val; }
	/// インターリーブを返す
	virtual int		GetInterleave() const { return m_interleave; }
	/// インターリーブを設定
	virtual void	SetInterleave(int val) { m_interleave = val; }
	/// インターリーブを計算して設定
	virtual void	CalcInterleave();

	/// セクタリストを返す
	virtual DiskImageSectors *GetSectors() const { return sectors; }
	/// セクタ数を返す
	virtual int		GetSectorsPerTrack() const;
	/// 指定セクタ番号のセクタを返す
	virtual DiskImageSector  *GetSector(int sector_number, int density = -1);
	/// 指定位置のセクタを返す
	virtual DiskImageSector  *GetSectorByIndex(int pos);

	/// トラック内のもっともらしいID Cを返す
	virtual wxUint8	GetMajorIDC() const;
	/// トラック内のもっともらしいID Hを返す
	virtual wxUint8	GetMajorIDH() const;

	/// トラック内のすべてのID Cを変更
	virtual void	SetAllIDC(wxUint8 val);
	/// トラック内のすべてのID Hを変更
	virtual void	SetAllIDH(wxUint8 val);
	/// トラック内のすべてのID Rを変更
	virtual void	SetAllIDR(wxUint8 val);
	/// トラック内のすべてのID Nを変更
	virtual void	SetAllIDN(wxUint8 val);
	/// トラック内のすべての密度を変更
	virtual void	SetAllSingleDensity(bool val);
	/// トラック内のすべてのセクタ数を変更
	virtual void	SetAllSectorsPerTrack(int val);
	/// トラック内のすべてのセクタサイズを変更
	virtual void	SetAllSectorSize(int val);

	/// 余分なデータを設定する
	virtual void	SetExtraData(wxUint8 *buf, size_t size);
	/// 余分なデータを返す
	virtual wxUint8 *GetExtraData() const { return extra_data; }
	/// 余分なデータのサイズを返す
	virtual size_t	GetExtraDataSize() const { return extra_size; }

	/// 変更されているか
	virtual bool	IsModified() const;
	/// 変更済みをクリア
	virtual void	ClearModify();

	/// ディスクを返す
	virtual DiskImageDisk *GetDisk() const { return parent; }

	/// トラック番号とサイド番号の比較
	static int Compare(DiskImageTrack *item1, DiskImageTrack *item2);
	/// インターリーブを考慮したセクタ番号リストを返す
	static bool CalcSectorNumbersForInterleave(int interleave, size_t sectors_count, wxArrayInt &sector_nums, int sector_offset = 0);
};

// ----------------------------------------------------------------------

WX_DEFINE_ARRAY(DiskImageTrack *, DiskImageTracks);

// ----------------------------------------------------------------------

/// １ディスクのヘッダを渡すクラス
class DiskImageDiskHeader
{
public:
	DiskImageDiskHeader() {}
	virtual ~DiskImageDiskHeader() {}
	virtual int GetHeaderType() const = 0;
	/// ディスク名を返す
	virtual wxString GetName(bool real = false) const { return wxEmptyString; }
	/// 書き込み禁止かを返す
	virtual bool IsWriteProtected() const { return false; }
};

// ----------------------------------------------------------------------

/// １ディスクへのポインタを保持するクラス
class DiskImageDisk : public DiskParam
{
protected:
	DiskImageFile *parent;
	int m_num;					///< disk number
	wxString m_name;			///< disk name
	bool m_write_protect;		///< write protected ?

	wxUint32 m_offset_start;	///< usually header size

	DiskImageTracks *tracks;
	int m_max_track_number;

	const DiskParam *p_temp_param;	///< 解析でテンプレートがあった場合セット
	DiskParam orig_param;		///< 解析したパラメータ
	bool m_param_changed;		///< ディスクパラメータを変更したか

	DiskBasics *basics;

	DiskImageDisk() : DiskParam() {}
	DiskImageDisk(const DiskImageDisk &src) : DiskParam(src) {}
	DiskImageDisk &operator=(const DiskImageDisk &src) { return *this; }

public:
	DiskImageDisk(DiskImageFile *file, int n_num);
	DiskImageDisk(DiskImageFile *file, int n_num, const DiskParam &n_param, const wxString &n_diskname, bool n_write_protect);
	DiskImageDisk(DiskImageFile *file, int n_num, const DiskImageDiskHeader &n_header);
	virtual ~DiskImageDisk();

	/// インスタンス作成
	virtual DiskImageTrack *NewImageTrack() = 0;
	/// インスタンス作成
	virtual DiskImageTrack *NewImageTrack(int n_trk_num, int n_sid_num, int n_offset_pos, int n_interleave) = 0;

	/// ディスクにトラックを追加
	virtual size_t	Add(DiskImageTrack *newtrk);
	/// ディスクの内容を置き換える
	virtual int		Replace(int side_number, DiskImageDisk *src_disk, int src_side_number);
	/// ディスクにトラックを追加
	virtual int		AddNewTrack(int side_number);
	/// トラックを削除する
	virtual void	DeleteTracks(int start_offset_pos, int end_offset_pos, int side_number);
	/// トラックサイズ＆オフセットの再計算＆ディスクサイズ変更
	virtual size_t	ShrinkTracks(bool trim_unused_data);
	/// オフセットの再計算＆ディスクサイズ変更
	virtual size_t	CalcOffsets();
	/// ディスクサイズ計算（ディスクヘッダ分を除く）
	virtual size_t	CalcSizeWithoutHeader();

	/// ディスク番号を返す
	virtual int		GetNumber() const { return m_num; }
	/// ディスク名を返す
	virtual wxString GetName(bool real = false) const { return wxEmptyString; }
	/// ディスク名を設定
	virtual void	SetName(const wxString &val) {}
	/// ディスク名を設定
	virtual void	SetName(const wxUint8 *buf, size_t len) {}

	/// ヘッダを返す
	virtual DiskImageDiskHeader *GetHeader() { return NULL; }
	/// ディスクファイルを返す
	virtual DiskImageFile   *GetFile() const { return parent; }
	/// トラックリストを返す
	virtual DiskImageTracks *GetTracks() const { return tracks; }
	/// 指定トラックを返す
	virtual DiskImageTrack  *GetTrack(int track_number, int side_number);
	/// 指定トラックを返す
	virtual DiskImageTrack  *GetTrack(int index);
	/// 指定オフセット値からトラックを返す
	virtual DiskImageTrack  *GetTrackByOffset(wxUint32 offset);
	/// 指定セクタを返す
	virtual DiskImageSector *GetSector(int track_number, int side_number, int sector_number, int density = -1);
	/// ディスクの中でもっともらしいパラメータを設定
	virtual const DiskParam *CalcMajorNumber();

	/// 書き込み禁止かどうかを返す
	virtual bool	IsWriteProtected() const { return true; }
	/// 書き込み禁止かどうかを設定
	virtual void	SetWriteProtect(bool val) {}

	/// 密度を返す
	virtual wxString GetDensityText() const { return wxEmptyString; }
	/// 密度を返す
	virtual int		GetDensity() const { return 0; }
	/// 密度を設定
	virtual void	SetDensity(int val) {}

	/// ディスクサイズ（ヘッダサイズ含む）
	virtual wxUint32 GetSize() const { return 0; }
	/// ディスクサイズ（ヘッダサイズ含む）を設定
	virtual void	SetSize(wxUint32 val) {}
	/// ディスクサイズ（ヘッダサイズを除く）
	virtual wxUint32 GetSizeWithoutHeader() const { return 0; }
	/// ディスクサイズ（ヘッダサイズを除く）を設定
	virtual void	SetSizeWithoutHeader(wxUint32 val) {}

	/// オフセット値を返す
	virtual wxUint32 GetOffset(int num) const { return 0; }
	/// オフセット値を設定
	virtual void	SetOffset(int num, wxUint32 offset) {}
	/// ヘッダサイズを除いたオフセット値を設定
	virtual void	SetOffsetWithoutHeader(int num, wxUint32 offset) {}

	/// offset最小値 -> トラックデータの開始位置を返す
	virtual wxUint32 GetOffsetStart() const { return m_offset_start; }
	/// offset最小値 -> トラックデータの開始位置を設定
	virtual void	SetOffsetStart(wxUint32 val) { m_offset_start = val; }

	/// 最大トラック番号 offsetがNULLでない最大位置を返す
	virtual int		GetMaxTrackNumber() const { return m_max_track_number; }
	/// 最大トラック番号 offsetがNULLでない最大位置を設定
	virtual void	SetMaxTrackNumber(int pos) { m_max_track_number = pos; }

	/// 作成可能なトラック数を返す (DiskImage::GetCreatableTracks())
	virtual int		GetCreatableTracks() const { return 0; }

	/// ディスクの内容を初期化する(0パディング)
	virtual bool	Initialize(int selected_side);
	/// ディスクのトラックを作り直す
	virtual bool	Rebuild(const DiskParam &param, int selected_side);

	/// 変更済みに設定
	virtual void	SetModify();
	/// 変更されているか
	virtual bool	IsModified();
	/// 変更済みをクリア
	virtual void	ClearModify();

	/// トラックが存在するか
	virtual bool	ExistTrack(int side_number);

	/// テンプレートパラメータを設定
	virtual void	SetTemplateParam(const DiskParam *val) { p_temp_param = val; }
	/// テンプレートパラメータを返す
	virtual const DiskParam *GetTemplateParam() const { return p_temp_param; }

	/// 変更前パラメータを設定
	virtual void	SetOriginalParam(const DiskParam &val) { orig_param = val; }
	/// 変更前パラメータを返す
	virtual const DiskParam &GetOriginalParam() const { return orig_param; }
	/// パラメータ変更フラグを設定
	virtual void	SetParamChanged(bool val) { m_param_changed = val; }
	/// パラメータ変更フラグを返す
	virtual bool	GetParamChanged() const { return m_param_changed; }

	/// DISK BASIC領域を確保
	virtual void	AllocDiskBasics();
	/// DISK BASICを返す
	virtual DiskBasic *GetDiskBasic(int idx);
	/// DISK BASICを返す
	virtual DiskBasics *GetDiskBasics() { return basics; }
	/// DISK BASICをクリア
	virtual void	ClearDiskBasics();
	/// キャラクターコードマップ設定
	virtual void	SetCharCode(const wxString &name);

	/// ディスク番号を比較
	static int Compare(DiskImageDisk *item1, DiskImageDisk *item2);
};

// ----------------------------------------------------------------------

WX_DEFINE_ARRAY(DiskImageDisk *, DiskImageDisks);

// ----------------------------------------------------------------------

/// ディスクイメージへのポインタを保持するクラス
class DiskImageFile
{
protected:
	DiskImage *p_image;		///< イメージ
	DiskImageDisks *disks;	///< ディスク
	wxArrayShort *mods;		///< 変更フラグ 追加したかどうか

	wxString m_basic_type_hint;	///< BASIC種類ヒント

	DiskImageFile();
	DiskImageFile(const DiskImageFile &src);

public:
	DiskImageFile(DiskImage &image);
	virtual ~DiskImageFile();

	/// インスタンス作成
	virtual DiskImageDisk *NewImageDisk(int n_num) = 0;
	/// インスタンス作成
	virtual DiskImageDisk *NewImageDisk(int n_num, const DiskParam &n_param, const wxString &n_diskname, bool n_write_protect) = 0;
	/// インスタンス作成
	virtual DiskImageDisk *NewImageDisk(int n_num, const DiskImageDiskHeader &n_header) = 0;

	/// 変更フラグ 追加したかどうか
	enum en_mod_flags {
		MODIFY_NONE = 0,
		MODIFY_ADD = 1
	};

	/// ディスクを追加
	virtual size_t Add(DiskImageDisk *newdsk, short mod_flags);
	/// 全ディスクを削除
	virtual void Clear();
	/// ディスク数を返す
	virtual size_t Count() const;
	/// ディスクを削除
	virtual bool Delete(size_t idx);

	/// ディスクを返す
	virtual DiskImageDisks *GetDisks() { return disks; }
	/// ディスクを返す
	virtual DiskImageDisk  *GetDisk(size_t idx);

	virtual bool IsModified();
	virtual void ClearModify();

	virtual const wxString &GetBasicTypeHint() const { return m_basic_type_hint; }
	virtual void SetBasicTypeHint(const wxString &val) { m_basic_type_hint = val; }

	/// イメージを返す
	DiskImage &GetImage() { return *p_image; }
};

// ----------------------------------------------------------------------

/// ディスクイメージ入出力
class DiskImage
{
protected:
	wxFileName m_filename;
	DiskImageFile *p_file;
	DiskResult m_result;
	wxString m_format_type;

	virtual void NewFile(const wxString &filepath);
	virtual void ClearFile();

public:
	DiskImage();
	virtual ~DiskImage();

	/// インスタンス作成
	virtual DiskImageFile *NewImageFile() = 0;

	/// 新規作成
	virtual int Create(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint);
	/// 追加で新規作成
	virtual int Add(const wxString &diskname, const DiskParam &param, bool write_protect, const wxString &basic_hint);
	/// ファイルを追加
	virtual int Add(const wxString &filepath, const wxString &file_format, const DiskParam &param_hint);
	/// ファイルを開く
	virtual int Open(const wxString &filepath, const wxString &file_format, const DiskParam &param_hint);
	/// ファイルを開く前のチェック
	virtual int Check(const wxString &filepath, wxString &file_format, DiskParamPtrs &params, DiskParam &manual_param);
	/// 閉じる
	virtual void Close();
	/// ストリームの内容をファイルに保存できるか
	virtual int CanSave(const wxString &file_format);
	/// ストリームの内容をファイルに保存
	virtual int Save(const wxString &filepath, const wxString &file_format, const DiskWriteOptions &options);
	/// ストリームの内容をファイルに保存
	virtual int SaveDisk(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const DiskWriteOptions &options);
	/// ディスクを削除
	virtual bool Delete(int disk_number);
	/// 置換元のディスクを解析
	virtual int ParseForReplace(int disk_number, int side_number, const wxString &filepath, const wxString &file_format, const DiskParam &param_hint, DiskImageFile &src_file, DiskImageDisk* &tag_disk);
	/// ファイルでディスクを置換
	virtual int ReplaceDisk(int disk_number, int side_number, DiskImageDisk *src_disk, int src_side_number, DiskImageDisk *tag_disk);

	/// ディスク名を設定
	virtual bool SetDiskName(size_t disk_number, const wxString &newname);
	/// ディスク名を返す
	virtual wxString GetDiskName(size_t disk_number, bool real = false) const;

	/// ディスクを変更したか
	virtual bool IsModified();

	/// ディスクファイルを返す
	virtual DiskImageFile			*GetFile() { return p_file; }
	/// ディスクファイルを返す
	virtual const DiskImageFile	*GetFile() const { return p_file; }
	/// ディスク枚数
	virtual size_t CountDisks() const;
	/// ディスク一覧を返す
	virtual DiskImageDisks *GetDisks();
	/// 指定した位置のディスクを返す
	virtual DiskImageDisk			*GetDisk(size_t index);
	/// 指定した位置のディスクを返す
	virtual const DiskImageDisk	*GetDisk(size_t index) const;
	/// 指定した位置のディスクのタイプ
	virtual int GetDiskTypeNumber(size_t index) const;

	/// 作成可能なトラック数を返す
	virtual int	GetCreatableTracks() const { return 0; }

	/// ファイル名を返す
	virtual wxString GetFileName() const;
	/// ファイル拡張子を返す
	virtual wxString GetFileExt() const;
	/// ファイル名ベースを返す
	virtual wxString GetFileNameBase() const;
	/// ファイルパスを返す
	virtual wxString GetFilePath() const;
	/// パスを返す
	virtual wxString GetPath() const;

	/// ファイル名を設定
	virtual void SetFileName(const wxString &path);
	/// ファイル拡張子を設定
	virtual void SetFileExt(const wxString &ext);

	/// ファイルのフォーマット種類を返す
	virtual const wxString &GetFormatType() const;
	/// ファイルのフォーマット種類を設定
	virtual void SetFormatType(const wxString &format_type);

	/// DISK BASICが一致するか
	virtual bool MatchDiskBasic(const DiskBasic *target);
	/// DISK BASICの解析状態をクリア
	virtual void ClearDiskBasicParseAndAssign(int disk_number, int side_number);
	/// キャラクターコードマップ設定
	virtual void SetCharCode(const wxString &name);

	/// 密度文字列を返す
	virtual int	 GetDensityNames(wxArrayString &arr) const { return 0; }
	/// 密度リストを検索
	virtual int	 FindDensity(int val) const { return -1; }
	/// 密度リストを検索
	virtual int  FindDensityByIndex(int idx) const { return -1; }
	/// 密度リストの指定位置の値を返す
	virtual wxUint8 GetDensity(int idx) const { return 0; }

	/// エラーメッセージ
	virtual const wxArrayString &GetErrorMessage(int maxrow = 20);
	/// エラーメッセージを表示
	virtual void  ShowErrorMessage();
	/// エラー警告メッセージを表示
	virtual int   ShowErrWarnMessage();
};

#endif /* DISKIMAGE_H */
