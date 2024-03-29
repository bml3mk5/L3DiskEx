/// @file basictype_mz_fdos.h
///
/// @brief disk basic type for MZ Floppy DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_MZ_FDOS_H
#define BASICTYPE_MZ_FDOS_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype_mz_base.h"


/** @class DiskBasicTypeMZFDOS

@brief MZ Floppy DOSの処理

DiskBasicParam 固有のパラメータ
@li	IPLString : セクタ1のIPL

*/
class DiskBasicTypeMZFDOS : public DiskBasicTypeMZBase
{
private:
	DiskBasicTypeMZFDOS() : DiskBasicTypeMZBase() {}
	DiskBasicTypeMZFDOS(const DiskBasicType &src) : DiskBasicTypeMZBase(src) {}
public:
	DiskBasicTypeMZFDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief 使用しているグループの位置を得る
	virtual void	CalcUsedGroupPos(wxUint32 num, int &pos, int &mask);
	/// @brief 次のグループ番号を得る
	virtual wxUint32 GetNextGroupNumber(wxUint32 num, int sector_pos);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	//@}

	/// @name disk size
	//@{
	//@}

	/// @name file chain
	//@{
	/// @brief ファイルをセーブする前の準備を行う
	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief グループを確保して使用中にする
	int			AllocateGroupsSub(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, DiskBasicGroups &group_items, int &file_size, int &groups);
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
	/// @brief データの読み込み/比較処理
	virtual int		AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
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
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* BASICTYPE_MZ_FDOS_H */
