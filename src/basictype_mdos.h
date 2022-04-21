/// @file basictype_mdos.h
///
/// @brief disk basic fat type for MDOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_MDOS_H_
#define _BASICTYPE_MDOS_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat16.h"


/** @class DiskBasicTypeMDOS

@brief MDOS の処理

*/
class DiskBasicTypeMDOS : public DiskBasicTypeFAT16
{
protected:
	DiskBasicTypeMDOS() : DiskBasicTypeFAT16() {}
	DiskBasicTypeMDOS(const DiskBasicType &src) : DiskBasicTypeFAT16(src) {}
public:
	DiskBasicTypeMDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(DiskD88Disk *disk, bool is_formatting);
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	virtual void	GetUsableDiskSize(int &disk_size, int &group_size) const;
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
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
	//@}

	/// @name property
	//@{
	//@}
};

#endif /* _BASICTYPE_MDOS_H_ */
