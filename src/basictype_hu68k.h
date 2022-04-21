/// @file basictype_msdos.h
///
/// @brief disk basic type for Human68k
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_HU68K_H_
#define _BASICTYPE_HU68K_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_msdos.h"


/** @class DiskBasicTypeHU68K

@brief Human68kの処理

DiskBasicParam 固有のパラメータ
@li IPLString : IPL文字列
@li IPLCompareString : OS判定時に使用する
@li IgnoreParameter : セクタ１にあるパラメータを無視するか
@li MediaID : メディアID

*/
class DiskBasicTypeHU68K : public DiskBasicTypeMSDOS
{
private:
	DiskBasicTypeHU68K() : DiskBasicTypeMSDOS() {}
	DiskBasicTypeHU68K(const DiskBasicType &src) : DiskBasicTypeMSDOS(src) {}
public:
	DiskBasicTypeHU68K(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name check / assign FAT area
	//@{
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	double	ParseParamOnDisk(DiskD88Disk *disk, bool is_formatting);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_HU68K_H_ */
