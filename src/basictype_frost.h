/// @file basictype_frost.h
///
/// @brief disk basic type for Frost-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_FROST_H_
#define _BASICTYPE_FROST_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat8.h"


/** @class DiskBasicTypeFROST

@brief Frost-DOSの処理

DiskBasicParam
@li ReservedGroups : Group 予約済みにするグループ（クラスタ）番号

*/
class DiskBasicTypeFROST : public DiskBasicTypeFAT8
{
private:
	DiskBasicTypeFROST() : DiskBasicTypeFAT8() {}
	DiskBasicTypeFROST(const DiskBasicType &src) : DiskBasicTypeFAT8(src) {}
public:
	DiskBasicTypeFROST(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
	wxUint32	GetGroupNumber(wxUint32 num) const;
	/// @brief 次の空きFAT位置を返す
	wxUint32	GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	double	ParseParamOnDisk(bool is_formatting);
	/// @brief FATエリアをチェック
	double 	CheckFat(bool is_formatting);
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	void	GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	void	CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file chain
	//@{
	/// @brief 未使用が連続している位置をさがす
	wxUint32	FindContinuousArea(wxUint32 group_size);
	/// @brief データサイズ分のグループを確保する
	int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief グループ番号から開始セクタ番号を得る
	int		GetStartSectorFromGroup(wxUint32 group_num);

	/// @brief トラック＋セクタ番号から論理セクタ番号を得る
	wxUint32 ConvSectorPosFromTrackSector(wxUint32 trk_sec) const;
	/// @brief 論理セクタ番号からトラック＋セクタ番号を得る
	wxUint32 ConvTrackSectorFromSectorPos(wxUint32 pos) const;

	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
	void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを指定コードで埋める
	void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// @brief セクタデータを埋めた後の個別処理
	bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	//@}

	/// @name save / write
	//@{
	/// @brief グループ確保時に最後のグループ番号を計算する
	wxUint32 CalcLastGroupNumber(wxUint32 group_num, int &size_remain);
	/// @brief データの書き込み処理
	int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_FROST_H_ */
