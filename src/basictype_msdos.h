/// @file basictype_msdos.h
///
/// @brief disk basic type for MS-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_MSDOS_H_
#define _BASICTYPE_MSDOS_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat12.h"


/** @class DiskBasicTypeMSDOS

@brief MS-DOSの処理

DiskBasicParam 固有のパラメータ
@li MediaID : メディアID
@li IgnoreParameter : セクタ1のパラメータを無視するか

*/
class DiskBasicTypeMSDOS : public DiskBasicTypeFAT12
{
protected:
	DiskBasicTypeMSDOS() : DiskBasicTypeFAT12() {}
	DiskBasicTypeMSDOS(const DiskBasicType &src) : DiskBasicTypeFAT12(src) {}

	/// ボリュームラベルを更新 なければ作成
	bool			ModifyOrMakeVolumeLabel(const wxString &filename);

public:
	DiskBasicTypeMSDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name check / assign FAT area
	//@{
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(DiskD88Disk *disk, bool is_formatting);
	//@}

	/// @name directory
	//@{
	/// @brief サブディレクトリのサイズを拡張できるか
	virtual bool	CanExpandDirectory() const { return true; }
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_MSDOS_H_ */
