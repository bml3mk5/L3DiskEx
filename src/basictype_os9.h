﻿/// @file basictype_os9.h
///
/// @brief disk basic type for OS-9
///
#ifndef _BASICTYPE_OS9_H_
#define _BASICTYPE_OS9_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype.h"

#pragma pack(1)
/// OS-9 Ident LSN = 0(track1, sector1)
typedef struct st_os9_ident {
	os9_lsn_t	DD_TOT;	///< total lsn
	wxUint8		DD_TKS;	///< sectors per track
	wxUint16	DD_MAP;	///< allocation map length
	wxUint16	DD_BIT;	///< sectors per group
	os9_lsn_t	DD_DIR;	///< rootdir lsn
	wxUint16	DD_OWN;	///< owner id
	wxUint8		DD_ATT;	///< disk attr
	wxUint16	DD_DSK;	///< disk ident
	wxUint8		DD_FMT;	///< format, density, number of sides
	wxUint16	DD_SPT;	///< sector per track
	wxUint16	reserved1;
	os9_lsn_t	DD_BT;	///< bootstrap lsn
	wxUint16	DD_BSZ;	///< bootstrap size (in bytes)
	os9_date_t	DD_DAT;	///< creation date
	wxUint8		DD_NAM[32];	///< volume label
	wxUint8		DD_OPT[32];	///< option
	wxUint8		reserved2;
	wxUint32	DD_SYNC;	///< media inegrity code
	wxUint32	DD_MapLSN;	///< bitmap starting sector number
	wxUint16	DD_LSNSize;	///< media logical sector size
	wxUint16	DD_VersID;	///< version id
} os9_ident_t;
#pragma pack()


/// OS-9の処理
class DiskBasicTypeOS9 : public DiskBasicType
{
private:
	os9_ident_t *os9_ident;	///< Identification Sector

	DiskBasicTypeOS9() : DiskBasicType() {}
	DiskBasicTypeOS9(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeOS9(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// FATオフセットを返す
	wxUint32	GetGroupNumber(wxUint32 num);
	/// 使用しているグループ番号か
	bool		IsUsedGroupNumber(wxUint32 num);
	/// 次のグループ番号を得る
	wxUint32	GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
	/// 次の空きFAT位置を返す 未使用
	wxUint32	GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// FATエリアをチェック
	bool		CheckFat();
	/// ルートディレクトリをアサイン
	bool		AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// ディスクから各パラメータを取得
	bool		ParseParamOnDisk(DiskD88Disk *disk);
	//@}

	/// @name check / assign directory area
	//@{
	/// ルートディレクトリのチェック
	bool		CheckRootDirectory(int start_sector, int end_sector);
	/// ルートディレクトリをアサイン
	bool		AssignRootDirectory(int start_sector, int end_sector);
	//@}

	/// @name disk size
	//@{
	/// 残りディスクサイズを計算
	void		CalcDiskFreeSize();
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
	/// データサイズ分のグループを確保する
	int			AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items);

	/// グループ番号から開始セクタ番号を得る
	int			GetStartSectorFromGroup(wxUint32 group_num);
	/// グループ番号から最終セクタ番号を得る
	int			GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// データ領域の開始セクタを計算
	int			CalcDataStartSectorPos();
	//@}

	/// @name directory
	//@{
	/// ルートディレクトリか
	bool		IsRootDirectory(wxUint32 group_num);
	/// サブディレクトリを作成できるか
	bool		CanMakeDirectory() const { return true; }
	/// サブディレクトリを作成する前の準備を行う
	bool		PrepareToMakeDirectory(DiskBasicDirItem *item);
	/// サブディレクトリを作成した後の個別処理
	void		AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item, wxUint32 parent_group_num);
	//@}

	/// @name format
	//@{
	/// フォーマットできるか
	bool		IsFormattable() const { return true; }
	/// セクタデータを指定コードで埋める
	void		FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// セクタデータを埋めた後の個別処理
	void		AdditionalProcessOnFormatted();
	//@}

	/// @name data access (read / verify)
	//@{
	//@}

	/// @name save / write
	//@{
	/// 書き込み可能か
	bool		IsWritable() const { return true; }
	/// ファイルをセーブする前の準備を行う
	bool		PrepareToSaveFile(wxInputStream &istream, const DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
//	/// データの書き込み処理
//	int			WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
	/// データの書き込み終了後の処理
	void		AdditionalProcessOnSavedFile(DiskBasicDirItem *item);

//	/// ファイル名変更後の処理
//	void		AdditionalProcessOnRenamedFile(DiskBasicDirItem *item);
	//@}

	/// @name delete
	//@{
	/// ファイルを削除できるか
	bool		IsDeletable() const { return true; }
	/// 指定したグループ番号のFAT領域を削除する
	void		DeleteGroupNumber(wxUint32 group_num);
	/// ファイル削除後の処理
	bool		AdditionalProcessOnDeletedFile(DiskBasicDirItem *item);
	//@}
};

#endif /* _BASICTYPE_OS9_H_ */
