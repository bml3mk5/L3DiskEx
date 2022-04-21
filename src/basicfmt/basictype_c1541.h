/// @file basictype_c1541.h
///
/// @brief disk basic type for Commodore 1541
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_C1541_H_
#define _BASICTYPE_C1541_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


//////////////////////////////////////////////////////////////////////

#pragma pack(1)
/// @brief C1541 BITMAP
typedef struct st_c1541_map {
	wxUint8  remain;	// free blocks
	wxUint8  bits[3];	// Little Endien : Byte0 LSB -> MSB -> byte 1 LSB -> MSB 
} c1541_map_t;

/// @brief C1541 BAM
typedef struct st_c1541_bam {
	c1541_ptr_t start_dir;
	wxUint8  format_type;
	wxUint8  unused;
	c1541_map_t map[35];
	wxUint8  disk_name[18];
	wxUint16 disk_id;
	wxUint8  space0;
	wxUint8  dos_version;
	wxUint8  dos_format;
	wxUint8  space1;
	wxUint8  reserved[85];
} c1541_bam_t;

//////////////////////////////////////////////////////////////////////

/// @brief C1541 side sector
typedef struct st_c1541_side_sector {
	c1541_ptr_t next;
	wxUint8  side_num;
	wxUint8  record_length;
	c1541_ptr_t side_pos[6];
	c1541_ptr_t data_pos[120];
} c1541_side_sector_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

/// @brief C1541 BAM ビットマップ
class C1541Bitmap
{
private:
	wxUint32 m_my_group_num;
	c1541_bam_t *m_bam;			///< Block Availablity Map

public:
	C1541Bitmap();
	~C1541Bitmap() {}

	/// BAMセクタのデータポインタをセット
	void SetBitmap(c1541_bam_t *bam) { m_bam = bam; }
	/// グループ番号をセット
	void SetMyGroupNumber(wxUint32 val) { m_my_group_num = val; }
	/// グループ番号を返す
	wxUint32 GetMyGroupNumber() const { return m_my_group_num; }
	/// @brief 指定位置のビットを変更する
	void Modify(int track_num, int sector_num, bool use);
	/// @brief 指定位置が空いているか
	bool IsFree(int track_num, int sector_num) const;
	/// @brief 指定トラックをすべて未使用にする
	void FreeTrack(int track_num, int num_of_sector);
	/// @brief ディスク名を返す
	size_t GetDiskName(wxUint8 *buf, size_t len) const;
	/// @brief ディスク名サイズを返す
	size_t GetDiskNameSize() const;
	/// @brief ディスク名を設定
	void SetDiskName(const wxUint8 *buf, size_t len);
	/// @brief ディスクIDを返す
	int  GetDiskID() const;
	/// @brief ディスクIDを設定
	void SetDiskID(int val);
};

//////////////////////////////////////////////////////////////////////

/// @brief C1541 セクタ位置変換マップリスト
class C1541SectorPosTrans : public DiskBasicSectorPosTrans
{
public:
	void CreateSectorSkewMap(DiskBasic *basic);
};

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicTypeC1541

@brief Commodore 1541 の処理

DiskBasicParam
@li SectorSkewForSave : ファイルインポート時の空きセクタの埋め方

*/
class DiskBasicTypeC1541 : public DiskBasicType
{
private:
	DiskBasicTypeC1541() : DiskBasicType() {}
	DiskBasicTypeC1541(const DiskBasicType &src) : DiskBasicType(src) {}

	C1541Bitmap c1541_bam;			///< Block Availablity Map
	C1541SectorPosTrans sector_map;	///< 可変数セクタマップ

	/// @brief 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumberM(int method);
	/// @brief 次の空きFAT位置を返す
	wxUint32	GetDirNextEmptyGroupNumber(wxUint32 curr_group);
	/// @brief 最終グループをつなげる
	int			ChainLastGroup(wxUint32 group_num, int remain);

public:
	DiskBasicTypeC1541(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	~DiskBasicTypeC1541();

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	virtual void	SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FATオフセットを返す
	virtual wxUint32 GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループ番号か
	virtual bool	IsUsedGroupNumber(wxUint32 num);
	/// @brief 次のグループ番号を得る
	virtual wxUint32 GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// @brief 空きFAT位置を返す
	virtual wxUint32 GetEmptyGroupNumber();
	/// @brief 次の空きFAT位置を返す
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	/// @brief Allocation Mapの開始位置を得る（ダイアログ用）
	virtual void	GetStartNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief Allocation Mapの終了位置を得る（ダイアログ用）
	virtual void	GetEndNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief "Allocation Map"タイトル名（ダイアログ用）
	virtual wxString GetTitleForFat() const;
	//@}

	/// @name check / assign directory area
	//@{
//	/// @brief ルートディレクトリをアサイン
//	virtual bool	AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief ルートディレクトリのセクタリストを計算
	virtual bool	CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	/// セクタをディレクトリとして初期化
	virtual int		InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &sizeremain, DiskBasicError &errinfo);
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	virtual void	GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
//	/// @brief ファイルをセーブする前の準備を行う
//	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief グループをつなげる
	virtual int		ChainGroups(wxUint32 group_num, wxUint32 append_group_num);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
	virtual void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
	virtual void	GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num);
	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
	/// @brief トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNumS(int track, int sector_num);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// @brief ルートディレクトリのサイズを拡張できるか
	virtual bool	CanExpandRootDirectory() const { return true; }
	//@}

	/// @name format
	//@{
	/// @brief フォーマット時セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief データの読み込み/比較処理
	virtual int		AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	/// @brief ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	/// @brief データの書き込み終了後の処理
	virtual void	AdditionalProcessOnSavedFile(DiskBasicDirItem *item);
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
	/// @brief ファイル削除後の処理
	virtual bool	AdditionalProcessOnDeletedFile(DiskBasicDirItem *item);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_C1541_H_ */
