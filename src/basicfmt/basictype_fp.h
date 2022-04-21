/// @file basictype_fp.h
///
/// @brief disk basic type for C82-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_FP_H_
#define _BASICTYPE_FP_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype_n88.h"


/** @class DiskBasicTypeFP

@brief C82-BASICの処理

DiskBasicParam
@li ReservedGroups : Group 予約済みにするグループ（クラスタ）番号

*/
class DiskBasicTypeFP : public DiskBasicTypeN88
{
private:
	DiskBasicTypeFP() : DiskBasicTypeN88() {}
	DiskBasicTypeFP(const DiskBasicType &src) : DiskBasicTypeN88(src) {}
public:
	DiskBasicTypeFP(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
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
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	//@}
};

#endif /* _BASICTYPE_FP_H_ */
