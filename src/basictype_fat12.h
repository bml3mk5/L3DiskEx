/// @file basictype_fat12.h
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_FAT12_H_
#define _BASICTYPE_FAT12_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype.h"


#pragma pack(1)
/// FAT BPB
typedef struct st_fat_bpb {
	wxUint8	 BS_JmpBoot[3];
	wxUint8  BS_OEMName[8];
	wxUint16 BPB_BytsPerSec;
	wxUint8  BPB_SecPerClus;
	wxUint16 BPB_RsvdSecCnt;
	wxUint8  BPB_NumFATs;
	wxUint16 BPB_RootEntCnt;
	wxUint16 BPB_TotSec16;
	wxUint8  BPB_Media;
	wxUint16 BPB_FATSz16;
	wxUint16 BPB_SecPerTrk;
	wxUint16 BPB_NumHeads;
	wxUint32 BPB_HiddSec;
} fat_bpb_t;
#pragma pack()


/** @class DiskBasicTypeFAT12

@brief FAT12の処理

*/
class DiskBasicTypeFAT12 : public DiskBasicType
{
protected:
	DiskBasicTypeFAT12() : DiskBasicType() {}
	DiskBasicTypeFAT12(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeFAT12(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeFAT12() {}

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	void			SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
	wxUint32		GetGroupNumber(wxUint32 num) const;
	/// @brief 次の空きFAT位置を返す
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(DiskD88Disk *disk, bool is_formatting);
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// @brief 管理エリアのトラック番号からグループ番号を計算
	virtual wxUint32 CalcManagedStartGroup();
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
	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);
	/// @brief データ領域の開始セクタを計算
	virtual int		CalcDataStartSectorPos();
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return true; }
	/// @brief サブディレクトリを作成する前にディレクトリ名を編集する
	virtual bool	RenameOnMakingDirectory(wxString &dir_name);
	/// @brief サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを指定コードで埋める
	virtual void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	/// @brief BIOS Parameter Block を作成
	bool			CreateBiosParameterBlock(const char *jmp, const char *name, wxUint8 **sec_buf = NULL);
	//@}

	/// @name save / write
	//@{
	/// @brief グループ確保時に最後のグループ番号を計算する
	virtual wxUint32 CalcLastGroupNumber(wxUint32 group_num, int &size_remain);
	//@}
};

#endif /* _BASICTYPE_FAT12_H_ */
