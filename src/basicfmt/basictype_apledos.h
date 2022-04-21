/// @file basictype_apledos.h
///
/// @brief disk basic type for Apple DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_APLEDOS_H_
#define _BASICTYPE_APLEDOS_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


//////////////////////////////////////////////////////////////////////

#pragma pack(1)
/// Apple DOS Volume table of contents (VTOC)
typedef struct st_apledos_vtoc {
	wxUint8  reserved0;
	wxUint8  dir_start_track;
	wxUint8  dir_start_sector;
	wxUint8  release_number;
	wxUint8  reserved1[2];
	wxUint8	 volume_number;
	wxUint8  reserved2[32];
	wxUint8  chain_size;	// max number of trk/sec list (normally 122)
	wxUint8  reserved3[8];
	wxUint16 track_bit_mask[2];	// big endien
	wxUint8  tracks_per_disk;
	wxUint8  sectors_per_track;
	wxUint16 sector_size;	// little endien
	wxUint16 track_map[50][2];	// big endien
} apledos_vtoc_t;
#pragma pack()

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicTypeAppleDOS

@brief Apple DOS 3.x の処理

DiskBasicParam
@li DirStartPositionOnSector : ディレクトリエントリの開始位置

*/
class DiskBasicTypeAppleDOS : public DiskBasicType
{
private:
	DiskBasicTypeAppleDOS() : DiskBasicType() {}
	DiskBasicTypeAppleDOS(const DiskBasicType &src) : DiskBasicType(src) {}

	apledos_vtoc_t *apledos_vtoc;

	/// @brief トラックマップマスクを設定
	void		SetTrackMapMask(wxUint32 val);
	/// @brief トラックマップのビットを変更
	void		ModifyTrackMap(int track_num, int sector_num, bool use);
	/// @brief  空いているか
	bool		IsFreeTrackMap(int track_num, int sector_num) const;
	/// @brief トラックマップを設定
	void		SetTrackMap(int track_num, wxUint32 val);
	/// @brief トラックマップを得る
	wxUint32	GetTrackMap(int track_num) const;
	/// @brief  チェインセクタを確保する
	wxUint32	AllocChainSector(int idx, DiskBasicDirItem *item, wxUint32 curr_group);

public:
	DiskBasicTypeAppleDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

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
	/// @brief 次の空きFAT位置を返す
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリのセクタリストを計算
	virtual bool	CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
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
	/// @brief ファイルをセーブする前の準備を行う
	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
//	/// @brief グループをつなげる
//	virtual int		ChainGroups(wxUint32 group_num, wxUint32 append_group_num);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
	virtual void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
	virtual void	GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num);
	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
	/// @brief トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNumS(int track, int sector_num);
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
	/// @brief フォーマット時セクタデータを埋めた後の個別処理
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

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
	/// @brief ファイル削除後の処理
	virtual bool	AdditionalProcessOnDeletedFile(DiskBasicDirItem *item);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_APLEDOS_H_ */
