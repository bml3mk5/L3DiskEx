/// @file basictype_cdos.h
///
/// @brief disk basic type for CDOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_CDOS_H_
#define _BASICTYPE_CDOS_H_

#include "../common.h"
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
	virtual void	CalcUsedGroupPos(wxUint32 num, int &pos, int &mask);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	//@}

	/// @name file chain
	//@{
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	//@}

	/// @name directory
	//@{
	/// @brief サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return false; }
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
	/// @brief 内部ファイルをエクスポートする際に内容を変換
	virtual bool	ConvertDataForLoad(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	/// @brief エクスポートしたファイルをベリファイする際に内容を変換
	virtual bool	ConvertDataForVerify(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	//@}

	/// @name save / write
	//@{
	/// @brief ファイルをセーブする前にデータを変換
	virtual bool	ConvertDataForSave(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
};

#endif /* _BASICTYPE_CDOS_H_ */
