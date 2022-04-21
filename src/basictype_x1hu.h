/// @file basictype_x1hu.h
///
/// @brief disk basic fat type for X1 Hu-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_X1HU_H_
#define _BASICTYPE_X1HU_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype.h"


/** @class DiskBasicTypeX1HU

@brief X1 Hu-BASICの処理

DiskBasicParam
@li IPLString : セクタ1のIPL

*/
class DiskBasicTypeX1HU : public DiskBasicType
{
private:
	DiskBasicTypeX1HU() : DiskBasicType() {}
	DiskBasicTypeX1HU(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeX1HU(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// FAT位置を返す
	wxUint32	GetGroupNumber(wxUint32 num) const;
	/// 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
	/// 次の空き位置を返す
	wxUint32	GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// FATエリアをチェック
	bool		CheckFat();
	//@}

	/// @name disk size
	//@{
	/// 使用可能なディスクサイズを得る
	void		GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// 残りディスクサイズを計算
	void		CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file chain
	//@{
	/// グループ番号から開始セクタ番号を得る
	int			GetStartSectorFromGroup(wxUint32 group_num);
	/// グループ番号から最終セクタ番号を得る
	int			GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);
	//@}

	/// @name directory
	//@{
	/// ルートディレクトリか
	bool		IsRootDirectory(wxUint32 group_num);
	/// サブディレクトリを作成できるか
	bool		CanMakeDirectory() const { return true; }
	/// サブディレクトリを作成する前にディレクトリ名を編集する
	bool		RenameOnMakingDirectory(wxString &dir_name);
	/// サブディレクトリを作成した後の個別処理
	void		AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// セクタデータを埋めた後の個別処理
	bool		AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// ファイルの最終セクタのデータサイズを求める
	int			CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// 最後のグループ番号を計算する
	wxUint32	CalcLastGroupNumber(wxUint32 group_num, int size_remain);
	//@}
};

#endif /* _BASICTYPE_X1HU_H_ */
