/// @file basictype_prodos.h
///
/// @brief disk basic type for Apple ProDOS 8
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_PRODOS_H
#define BASICTYPE_PRODOS_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


//////////////////////////////////////////////////////////////////////

/// @brief Apple ProDOS ビットマップ
class ProDOSBitmap : public DiskBasicBitMLMap
{
private:
	wxUint32	 m_group_num;

public:
	ProDOSBitmap();
	~ProDOSBitmap() {}

	/// @brief ポインタをセット
	void AddBitmap(DiskImageSector *sector);
	/// @brief 指定位置のビットを変更する
	void Modify(wxUint32 group_num, bool use);
	/// @brief 指定位置が空いているか
	bool IsFree(wxUint32 group_num) const;

	/// @brief ブロック番号をセット
	void SetMyGroupNumber(wxUint32 group_num) { m_group_num = group_num; }
	/// @brief ブロック番号を得る
	wxUint32 GetMyGroupNumber() const { return m_group_num; }
};

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicTypeProDOS

@brief Apple ProDOS 8 / 16 の処理

DiskBasicParam
@li DirStartPositionOnSector : ディレクトリエントリの開始位置

グループ番号(Group Number) := ブロック番号(Block number)
論理セクタ番号(Sector Pos) := グループ番号 * 2 (ProDOS 8)

@sa DiskBasicDirItemProDOS
*/
class DiskBasicTypeProDOS : public DiskBasicType
{
private:
	DiskBasicTypeProDOS() : DiskBasicType() {}
	DiskBasicTypeProDOS(const DiskBasicType &src) : DiskBasicType(src) {}

	DiskBasicSectorSkew sector_skew;	///< セクタスキュー(ProDOS 8のみ)

	ProDOSBitmap bitmap;				///< ビットマップ
	directory_prodos_t *volume;			///< ボリュームヘッダ
	DiskBasicSectorPosTrans sector_map;	///< 可変数セクタマップ

	/// @brief  チェインセクタを確保する
	wxUint32	AllocChainSector(int idx, DiskBasicDirItem *item);

public:
	DiskBasicTypeProDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	~DiskBasicTypeProDOS();

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
	/// @brief Allocation Mapの開始位置を得る（ダイアログ用）
	virtual void	GetStartNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief Allocation Mapの終了位置を得る（ダイアログ用）
	virtual void	GetEndNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief "Allocation Map"タイトル名（ダイアログ用）
	virtual wxString GetTitleForFat() const;
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリをアサイン
	virtual bool	AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	virtual int		FinishAssigningDirectory(int &pos, int &size, int &size_remain) const;
	/// @brief ルートディレクトリのセクタリストを計算
	virtual bool	CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	/// セクタをディレクトリとして初期化
	virtual int		InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &sizeremain, DiskBasicError &errinfo);
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
	/// @brief ディレクトリのグループをつなげる
	int			ChainDirectoryGroups(DiskBasicDirItem *item, DiskBasicGroups &group_items);

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
	virtual bool	CanMakeDirectory() const { return true; }
	/// @brief サブディレクトリのサイズを拡張できるか
	virtual bool	CanExpandDirectory() const { return true; }
	/// @brief サブディレクトリを作成する前にディレクトリ名を編集する
	virtual bool	RenameOnMakingDirectory(wxString &dir_name);
	/// @brief サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
//	/// @brief フォーマット時セクタデータを指定コードで埋める
//	virtual void	FillSector(DiskImageTrack *track, DiskImageSector *sector);
	/// @brief フォーマット時セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
//	/// @brief データの読み込み/比較処理
//	virtual int		AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	/// @brief ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	/// @brief データの書き込み終了後の処理
	virtual void	AdditionalProcessOnSavedFile(DiskBasicDirItem *item);
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

#endif /* BASICTYPE_PRODOS_H */
