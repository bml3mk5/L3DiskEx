/// @file basictype_fat12.h
///
/// @brief disk basic fat type
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


/// FAT12の処理
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
	/// FAT位置をセット
	void			SetGroupNumber(wxUint32 num, wxUint32 val);
	/// FAT位置を返す
	wxUint32		GetGroupNumber(wxUint32 num);
	/// 次の空きFAT位置を返す
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// FATエリアをチェック
	bool			CheckFat();
	/// ディスクから各パラメータを取得
	virtual bool	ParseParamOnDisk(DiskD88Disk *disk);
	/// 管理エリアのトラック番号からグループ番号を計算
	virtual wxUint32 CalcManagedStartGroup();
	//@}

	/// @name disk size
	//@{
	/// 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize();
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
	/// グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// データ領域の開始セクタを計算
	virtual int		CalcDataStartSectorPos();
	//@}

	/// @name directory
	//@{
	/// ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return true; }
	/// サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item, wxUint32 parent_group_num);
	//@}

	/// @name format
	//@{
	/// セクタデータを指定コードで埋める
	virtual void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// セクタデータを埋めた後の個別処理
	virtual void	AdditionalProcessOnFormatted();
	//@}

	/// @name save / write
	//@{
	/// 最後のグループ番号を計算する
	virtual wxUint32 CalcLastGroupNumber(wxUint32 group_num, int size_remain);
	//@}
};

#endif /* _BASICTYPE_FAT12_H_ */
