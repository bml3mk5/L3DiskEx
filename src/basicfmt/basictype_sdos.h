/// @file basictype_sdos.h
///
/// @brief disk basic type for S-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_SDOS_H
#define BASICTYPE_SDOS_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


/** @class DiskBasicTypeSDOS

@brief S-DOSの処理

DiskBasicParam
@li IPLCompareString : OS判別用検索文字列 セクタ１を検索
@li DefaultStartAddress : BASIC指定時の開始アドレス
@li DefaultExecuteAddress : BASIC指定時の実行アドレス

*/
class DiskBasicTypeSDOS : public DiskBasicType
{
private:
	DiskBasicTypeSDOS() : DiskBasicType() {}
	DiskBasicTypeSDOS(const DiskBasicType &src) : DiskBasicType(src) {}

	wxUint32 m_empty_group_num;	///< 空き開始グループ

public:
	DiskBasicTypeSDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	virtual void	SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
	virtual wxUint32 GetGroupNumber(wxUint32 num) const;
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
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	virtual int		FinishAssigningDirectory(int &pos, int &size, int &size_remain) const;
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
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	//@}

	/// @name directory
	//@{
	//@}

	/// @name format
	//@{
	/// @brief フォーマットできるか
	virtual bool	SupportFormatting() const { return false; }
	/// @brief セクタデータを指定コードで埋める
	virtual void	FillSector(DiskImageTrack *track, DiskImageSector *sector);
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	//@}

	/// @name save / write
	//@{
//	/// @brief ファイルをセーブする前の準備を行う
//	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	//@}

	/// @name delete
	//@{
	/// @brief ファイル削除後の処理
	virtual bool	AdditionalProcessOnDeletedFile(DiskBasicDirItem *item);
	//@}
};

#endif /* BASICTYPE_SDOS_H */
