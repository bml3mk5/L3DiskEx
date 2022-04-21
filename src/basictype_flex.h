/// @file basictype_flex.h
///
/// @brief disk basic type for FLEX
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_FLEX_H_
#define _BASICTYPE_FLEX_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype.h"


#pragma pack(1)
/// FLEX INIT (track0 sector3)
typedef struct st_flex_sir {
	wxUint8  reserved0[16];
	wxUint8  volume_label[8];
	wxUint8  reserved1[3];
	wxUint16 volume_number;		// big endien
	wxUint8  free_start_track;
	wxUint8  free_start_sector;
	wxUint8  free_last_track;
	wxUint8  free_last_sector;
	wxUint16 free_sector_nums;	// big endien
	wxUint8  cmonth;
	wxUint8  cday;
	wxUint8  cyear;
	wxUint8  max_track;
	wxUint8  max_sector;
} flex_sir_t;
#pragma pack()

/** @class DiskBasicTypeFLEX

@brief FLEXの処理

DiskBasicParam
@li DirStartPositionOnSector : ディレクトリエントリの開始位置

*/
class DiskBasicTypeFLEX : public DiskBasicType
{
private:
	flex_sir_t *flex_sir;	///< SIRエリア

	DiskBasicTypeFLEX() : DiskBasicType() {}
	DiskBasicTypeFLEX(const DiskBasicType &src) : DiskBasicType(src) {}

public:
	DiskBasicTypeFLEX(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FATオフセットを返す
	wxUint32	GetGroupNumber(wxUint32 num) const;
	/// @brief 使用しているグループ番号か
	bool		IsUsedGroupNumber(wxUint32 num);
	/// @brief 次のグループ番号を得る
	wxUint32	GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// @brief 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
	/// @brief 次の空きFAT位置を返す
	wxUint32	GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	double 		CheckFat(bool is_formatting);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	double		ParseParamOnDisk(bool is_formatting);
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリのセクタリストを計算
	bool		CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	void		GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	void		CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
	/// @brief データサイズ分のグループを確保する
	int			AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief グループをつなげる
	int			ChainGroups(wxUint32 group_num, wxUint32 append_group_num);

	/// @brief グループ番号から開始セクタ番号を得る
	int			GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	int			GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	bool		IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	bool		CanMakeDirectory() const { return false; }
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを指定コードで埋める
	void		FillSector(DiskD88Track *track, DiskD88Sector *sector);
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
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	void		DeleteGroupNumber(wxUint32 group_num);
	/// @brief ファイル削除後の処理
	bool		AdditionalProcessOnDeletedFile(DiskBasicDirItem *item);
	/// @brief 空きエリアのチェインを作り直す
	void		RemakeChainOnFreeArea();
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	void		GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	void		SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_FLEX_H_ */
