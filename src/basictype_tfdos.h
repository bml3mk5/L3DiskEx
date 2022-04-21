/// @file basictype_tfdos.h
///
/// @brief disk basic type for TF-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_TFDOS_H_
#define _BASICTYPE_TFDOS_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_mz_base.h"


/** @class DiskBasicTypeTFDOS

@brief TF-DOSの処理

DiskBasicParam
@li IDString  : セクタ1のIPL
@li VolumeString : FAT領域にあるボリューム名
@li ReservedGroups : 使用済みにするトラック

*/
class DiskBasicTypeTFDOS : public DiskBasicTypeMZBase
{
private:
	// BASEコンパチファイルかどうか
	bool is_base_compatible;

	DiskBasicTypeTFDOS() : DiskBasicTypeMZBase() {}
	DiskBasicTypeTFDOS(const DiskBasicType &src) : DiskBasicTypeMZBase(src) {}
public:
	DiskBasicTypeTFDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
	wxUint32	GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループ番号か
	bool		IsUsedGroupNumber(wxUint32 num);
	/// @brief 次のグループ番号を得る
	wxUint32	GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// @brief 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
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

	/// @brief グループ番号から最終セクタ番号を得る
	int			GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	bool		IsRootDirectory(wxUint32 group_num);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	bool		AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
//	/// @brief データの読み込み/比較の前処理
//	bool		PrepareToAccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, int &file_size, DiskBasicGroups &group_items, DiskBasicError &errinfo);
	/// @brief データの読み込み/比較処理
	int			AccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	/// @brief 内部ファイルをエクスポートする際に内容を変換
	bool		ConvertDataForLoad(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	/// @brief エクスポートしたファイルをベリファイする際に内容を変換
	bool		ConvertDataForVerify(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	//@}

	/// @name save / write
	//@{
	/// @brief ファイルをセーブする前にデータを変換
	bool		ConvertDataForSave(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
//	/// @brief ファイルをセーブする前の準備を行う
//	bool		PrepareToSaveFile(wxInputStream &istream, int &file_size, const DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データの書き込み処理
	int			WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	void		GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	void		SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_TFDOS_H_ */
