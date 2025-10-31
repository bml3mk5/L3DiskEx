/// @file basictype_hfs.h
///
/// @brief disk basic type for HFS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_HFS_H
#define BASICTYPE_HFS_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


//////////////////////////////////////////////////////////////////////

#pragma pack(1)
// Boot Block Header
typedef struct st_hfs_boot_blk_hdr {
	wxUint8  sig[2];			// boot blocks signature "LK"
	wxUint32 entry;				// entry point to boot code
	wxUint16 version;			// boot blocks version number
	wxUint16 pageFlags;			// used internally
	wxUint8  sysName[16];		// System filename
	wxUint8  shellName[16];		// Finder filename
	wxUint8  dbg1Name[16];		// debugger filename
	wxUint8  dbg2Name[16];		// debugger filename
	wxUint8  screenName[16];	// name of startup screen
	wxUint8  helloName[16];		// name of startup program
	wxUint8  scrapName[16];		// name of system scrap file
	wxUint16 numFCBs;			// number of FCBs to allocate
	wxUint16 numEvts;			// number of event queue elements
	wxUint32 hsize128;			// system heap size on 128K Mac
	wxUint32 hsize256;			// used internally
	wxUint32 sysHeapSize;		// system heap size on all machines
	wxUint16 filler;			// reserved
	wxUint32 sysHeapExtra;		// additional system heap space
	wxUint32 sysHeapFract;		// fraction of RAM for system heap
} hfs_boot_blk_hdr_t;

// Extent Descriptor
typedef struct st_hfs_ext_descriptor {
	wxUint16 start;				// first allocation block
	wxUint16 count;				// number of allocation blocks
} hfs_ext_descriptor_t;

typedef struct st_hfs_ext_data_rec {
   hfs_ext_descriptor_t d[3];	// extent data record
} hfs_ext_data_rec_t;

// Master Directory Block
typedef struct st_hfs_mdb {
	wxUint8	 sig[2];				// volume signature "BD" $D2$D7 (mfs)
	wxUint32 drCrDate;				// date and time of volume creation
	wxUint32 drLsMod;				// date and time of last modification
	wxUint16 drAtrb;				// volume attributes
	wxUint16 drNmFls;				// number of files in root directory
	wxUint16 drVBMSt;				// first block of volume bitmap
	wxUint16 drAllocPtr;			// start of next allocation search
	wxUint16 drNmAlBlks;			// number of allocation blocks in volume
	wxUint32 drAlBlkSiz;			// size (in bytes) of allocation blocks
	wxUint32 drClpSiz;				// default clump size
	wxUint16 drAlBlSt;				// first allocation block in volume
	wxUint32 drNxtCNID;				// next unused catalog node ID
	wxUint16 drFreeBks;				// number of unused allocation blocks
	wxUint8	 drVN[28];				// volume name
	wxUint32 drVolBkUp;				// date and time of last backup
	wxUint16 drVSeqNum;				// volume backup sequence number
	wxUint32 drWrCnt;				// volume write count
	wxUint32 drXTClpSiz;			// clump size for extents overflow file
	wxUint32 drCTClpSiz;			// clump size for catalog file
	wxUint16 drNmRtDirs;			// number of directories in root directory
	wxUint32 drFilCnt;				// number of files in volume
	wxUint32 drDirCnt;				// number of directories in volume
	wxUint32 drFndrInfo[8];			// information used by the Finder
	wxUint16 drVCSize;				// size (in blocks) of volume cache
	wxUint16 drVBMCSize;			// size (in blocks) of volume bitmap cache
	wxUint16 drCtlCSize;			// size (in blocks) of common volume cache
	wxUint32 drXTFlSize;			// size of extents overflow file
	hfs_ext_data_rec_t drXTExtRec;	// extent record for extents overflow file
	wxUint32 drCTFlSize;			// size of catalog file
	hfs_ext_data_rec_t drCTExtRec;	//  extent record for catalog file
} hfs_mdb_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicTypeHFS

@brief HFS の処理

DiskBasicParam
@li DirStartPositionOnSector : ディレクトリエントリの開始位置

*/
class DiskBasicTypeHFS : public DiskBasicType
{
private:
	DiskBasicTypeHFS() : DiskBasicType() {}
	DiskBasicTypeHFS(const DiskBasicType &src) : DiskBasicType(src) {}

	hfs_mdb_t *p_hfs_mdb;	///< Master Directory Blocks

	DiskBasicBitMLMap bitmap;

//	/// @brief トラックマップマスクを設定
//	void		SetTrackMapMask(wxUint32 val);
//	/// @brief トラックマップのビットを変更
//	void		ModifyTrackMap(int track_num, int sector_num, bool use);
//	/// @brief  空いているか
//	bool		IsFreeTrackMap(int track_num, int sector_num) const;
//	/// @brief トラックマップを設定
//	void		SetTrackMap(int track_num, wxUint32 val);
//	/// @brief トラックマップを得る
//	wxUint32	GetTrackMap(int track_num) const;
//	/// @brief  チェインセクタを確保する
//	wxUint32	AllocChainSector(int idx, DiskBasicDirItem *item, wxUint32 curr_group);

public:
	DiskBasicTypeHFS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

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
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリのセクタリストを計算
	virtual bool	CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	/// @brief ディレクトリのチェック
	virtual double	CheckDirectory(bool is_root, const DiskBasicGroups &group_items);
	/// @brief ディレクトリをアサイン
	virtual bool	AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
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
	/// @brief ファイルをセーブする前の準備を行う
	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
//	/// @brief グループをつなげる
//	virtual int		ChainGroups(wxUint32 group_num, wxUint32 append_group_num);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

//	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
//	virtual void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
//	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
//	virtual void	GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num);
//	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
//	virtual int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
//	/// @brief トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
//	virtual int		GetSectorPosFromNumS(int track, int sector_num);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return false; }
	//@}

	/// @name format
	//@{
	/// @brief フォーマット時セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// @brief 書き込み可能か
	virtual bool	SupportWriting() const { return false; }
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
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

#endif /* BASICTYPE_HFS_H */
