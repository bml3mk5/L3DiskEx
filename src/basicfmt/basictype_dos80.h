/// @file basictype_dos80.h
///
/// @brief disk basic fat type for PC-8001 DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_DOS80_H
#define BASICTYPE_DOS80_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype_fat8.h"


/** @class DiskBasicTypeDOS80

@brief PC-8001 DOSの処理

DiskBasicParam
@li CanMountEachSides : 表/裏面を別々に扱うか
@li ReservedGroups : Group 予約済みにするグループ（クラスタ）番号
@li DefaultStartAddress : BASIC指定時の開始アドレス
@li DefaultExecuteAddress : BASIC指定時の実行アドレス

*/
class DiskBasicTypeDOS80 : public DiskBasicTypeFAT8
{
private:
	DiskBasicTypeDOS80() : DiskBasicTypeFAT8() {}
	DiskBasicTypeDOS80(const DiskBasicType &src) : DiskBasicTypeFAT8(src) {}
public:
	DiskBasicTypeDOS80(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// 空きFAT位置を返す
	virtual wxUint32 GetEmptyGroupNumber();
	//@}

	/// @name check / assign FAT area
	//@{
	/// FATエリアをチェック
	virtual double CheckFat(bool is_formatting);
	//@}

	/// @name format
	//@{
	/// セクタデータを指定コードで埋める
	virtual void	FillSector(DiskImageTrack *track, DiskImageSector *sector);
	/// セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	//@}

	/// @name delete
	//@{
	//@}
};

#endif /* BASICTYPE_DOS80_H */
