/// @file basictype_m68fdos.h
///
/// @brief disk basic type for Sord M68 FDOS (KDOS)
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_M68_FDOS_H
#define BASICTYPE_M68_FDOS_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype_mz_base.h"


/** @class DiskBasicTypeM68FDOS

@brief Sord M68 FDOS (KDOS) の処理

DiskBasicParam 固有のパラメータ
@li	IPLString : セクタ1のIPL

*/
class DiskBasicTypeM68FDOS : public DiskBasicTypeMZBase
{
private:
	DiskBasicTypeM68FDOS() : DiskBasicTypeMZBase() {}
	DiskBasicTypeM68FDOS(const DiskBasicType &src) : DiskBasicTypeMZBase(src) {}

	/// @brief 使用状況テーブル
	DiskBasicBitMLMap m_bitmap;

public:
	DiskBasicTypeM68FDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief 使用しているグループの位置を得る
	virtual void	CalcUsedGroupPos(wxUint32 num, int &pos, int &mask);
	/// @brief FAT位置をセット
	virtual void	SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
	virtual wxUint32 GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループ番号か
	virtual bool	IsUsedGroupNumber(wxUint32 num);
	/// @brief 次のグループ番号を得る
	virtual wxUint32 GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// @brief 空きFAT位置を返す
	virtual wxUint32 GetEmptyGroupNumber();
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリのセクタリストを計算
	virtual bool	CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	virtual int		FinishAssigningDirectory(int &pos, int &size, int &size_remain) const;
	/// @brief ディレクトリアサインでセクタ毎に位置を調整する
	virtual int     AdjustPositionAssigningDirectory(int pos);
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	virtual void	GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize(bool wrote);
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

#endif /* BASICTYPE_M68_FDOS_H */
