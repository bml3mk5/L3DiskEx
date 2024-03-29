/// @file basictype_cpm.h
///
/// @brief disk basic type for CP/M
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_CPM_H
#define BASICTYPE_CPM_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


/** @class DiskBasicTypeCPM

@brief CP/Mの処理

@li AttributesByExtension バイナリとして扱う拡張子

*/
class DiskBasicTypeCPM : public DiskBasicType
{
protected:
	DiskBasicTypeCPM() : DiskBasicType() {}
	DiskBasicTypeCPM(const DiskBasicType &src) : DiskBasicType(src) {}

	DiskBasicSectorSkew sector_skew;	///< ソフトセクタスキュー

public:
	DiskBasicTypeCPM(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeCPM();

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
	/// @brief 次の空きFAT位置を返す 未使用
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリをアサイン
	virtual bool	AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
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
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// @brief データ領域の開始セクタを計算
	virtual int		CalcDataStartSectorPos();

	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
	virtual void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
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
	/// @brief セクタデータを指定コードで埋める
	virtual void	FillSector(DiskImageTrack *track, DiskImageSector *sector);
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
	/// @brief ファイルをセーブする前の準備を行う
	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
	//@}
};

#endif /* BASICTYPE_CPM_H */
