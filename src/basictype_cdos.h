/// @file basictype_cdos.h
///
/// @brief disk basic type for CDOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_CDOS_H_
#define _BASICTYPE_CDOS_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_mz_base.h"


/** @class DiskBasicTypeCDOS

@brief C-DOSの処理

DiskBasicParam
@li IDString     : FATエリアにあるID
@li	IPLString    : セクタ1のIPL
@li VolumeString : ボリューム名
@li Endian       : 16ビット値のバイトオーダ

*/
class DiskBasicTypeCDOS : public DiskBasicTypeMZBase
{
private:
	DiskBasicTypeCDOS() : DiskBasicTypeMZBase() {}
	DiskBasicTypeCDOS(const DiskBasicType &src) : DiskBasicTypeMZBase(src) {}
public:
	DiskBasicTypeCDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief 使用しているグループの位置を得る
	void		CalcUsedGroupPos(wxUint32 num, int &pos, int &mask);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	bool		CheckFat();
	//@}

	/// @name file chain
	//@{
	/// @brief データサイズ分のグループを確保する
	int			AllocateGroups(DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	//@}

	/// @name directory
	//@{
	/// @brief サブディレクトリを作成できるか
	bool		CanMakeDirectory() const { return false; }
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	bool		AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief データの読み込み/比較処理
	int			AccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	//@}

	/// @name save / write
	//@{
	/// @brief データの書き込み処理
	int			WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	void		GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	void		SetIdentifiedData(const DiskBasicIdentifiedData &data);
};

#endif /* _BASICTYPE_CDOS_H_ */
