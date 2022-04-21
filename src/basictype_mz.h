/// @file basictype_mz.h
///
/// @brief disk basic type for MZ DISK BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_MZ_H_
#define _BASICTYPE_MZ_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_mz_base.h"


/** @class DiskBasicTypeMZ

@brief MZ S-BASICの処理

DiskBasicParam 固有パラメータ
@li	IPLString         : セクタ1のIPL
@li	VolumeString      : ディレクトリ先頭のボリューム番号
@li	SpecialAttributes : 特別な属性 Volume属性にする属性値を指定

*/
class DiskBasicTypeMZ : public DiskBasicTypeMZBase
{
private:
	DiskBasicTypeMZ() : DiskBasicTypeMZBase() {}
	DiskBasicTypeMZ(const DiskBasicType &src) : DiskBasicTypeMZBase(src) {}
public:
	DiskBasicTypeMZ(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FATオフセットを返す
	wxUint32	GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループの位置を得る
	void		CalcUsedGroupPos(wxUint32 num, int &pos, int &mask);
	/// @brief 次のグループ番号を得る
	wxUint32	GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// @brief 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	double 		CheckFat(bool is_formatting);
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリをアサイン
	bool		AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	//@}

	/// @name disk size
	//@{
	/// @brief 残りディスクサイズを計算
	void		CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file chain
	//@{
	/// @brief データサイズ分のグループを確保する
	int			AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief グループを確保して使用中にする
	int			AllocateGroupsSub(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, DiskBasicGroups &group_items, int &file_size, int &groups);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	bool		IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	bool		CanMakeDirectory() const { return true; }
	/// @brief サブディレクトリを作成する前にディレクトリ名を編集する
	bool		RenameOnMakingDirectory(wxString &dir_name);
	/// @brief サブディレクトリを作成した後の個別処理
	void		AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	bool		AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief データの読み込み/比較処理
	int			AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	//@}

	/// @name save / write
	//@{
	/// @brief データの書き込み処理
	int			WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
	/// @brief データの書き込み終了後の処理
	void		AdditionalProcessOnSavedFile(DiskBasicDirItem *item);

	/// @brief ファイル名変更後の処理
	void		AdditionalProcessOnRenamedFile(DiskBasicDirItem *item);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	void		GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	void		SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_MZ_H_ */
