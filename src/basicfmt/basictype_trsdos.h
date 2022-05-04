/// @file basictype_trsdos.h
///
/// @brief disk basic type for Tandy TRSDOS 2.x / 1.3
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_TRSDOS_H_
#define _BASICTYPE_TRSDOS_H_

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


//////////////////////////////////////////////////////////////////////

/// @brief TRSDOS GAT (Granule Allocation Table)
class TRSDOS_GAT
{
private:
	wxUint8 *m_buffer;
	size_t	 m_size;
	int		 m_groups_per_track;

public:
	TRSDOS_GAT();
	TRSDOS_GAT(wxUint8 *n_buffer, size_t n_size, int n_groups_per_track);
	~TRSDOS_GAT() {}

	/// @brief 指定位置のビットを変更する
	void Modify(wxUint32 num, bool val);
	/// @brief 指定位置のビットがセットされているか
	bool IsSet(wxUint32 num) const;
	/// @brief 指定位置のビット位置を計算
	void GetPos(wxUint32 num, wxUint32 &pos, wxUint32 &bit) const;
	/// @brief バッファを返す
	wxUint8 *GetBuffer() { return m_buffer; }
	/// @brief バッファサイズを返す
	size_t GetSize() const { return m_size; }
};

//////////////////////////////////////////////////////////////////////

/// @brief TRSDOS HIT (Hash Index Table)
class TRSDOS_HIT
{
private:
	wxUint8 *m_hit_buffer;	///< バッファポインタ（セクタ内の開始ポインタ）
	size_t   m_hit_size;	///< バッファサイズ

public:
	TRSDOS_HIT();
	~TRSDOS_HIT() {}

	/// @brief メモリをアサイン
	void AssignHIT(wxUint8 *n_buffer, size_t n_size);
	/// @brief ハッシュを得る
	wxUint8 GetHI(wxUint32 pos);
	/// @brief ハッシュをセット
	void SetHI(wxUint32 pos, wxUint8 val);
	/// @brief ハッシュを削除
	void DeleteHI(wxUint32 pos);
};

//////////////////////////////////////////////////////////////////////

class DiskBasicDirItemTRSDOS;

/** @class DiskBasicTypeTRSDOS

@brief TRSDOS 2.x の処理

@sa DiskBasicDirItemTRSDOS
*/
class DiskBasicTypeTRSDOS : public DiskBasicType, public TRSDOS_HIT
{
protected:
	DiskBasicTypeTRSDOS() : DiskBasicType(), TRSDOS_HIT() {}
	DiskBasicTypeTRSDOS(const DiskBasicType &src) : DiskBasicType(src), TRSDOS_HIT() {}

	TRSDOS_GAT gat_table;	///< GAT Granule Allocate Table
	TRSDOS_GAT tlt_table;	///< TLT Track Lock-out Table

//	/// @brief  チェインセクタを確保する
//	wxUint32	AllocChainSector(int idx, DiskBasicDirItem *item);

public:
	DiskBasicTypeTRSDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	~DiskBasicTypeTRSDOS();

	/// @brief ハッシュの格納位置からセクタ番号を得る
	virtual void GetFromHIPosition(wxUint32 pos, int &sector_num, int &pos_in_sector) {}

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
//	/// @brief ファイルをセーブする前の準備を行う
//	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo);
//	/// @brief データサイズ分のグループを確保する
//	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
//	/// @brief ディレクトリのグループをつなげる
//	int			ChainDirectoryGroups(DiskBasicDirItem *item, DiskBasicGroups &group_items);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

//	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
//	virtual void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
//	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
//	virtual void	GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num);
//	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
//	virtual int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
//	/// @brief トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
//	virtual int		GetSectorPosFromNumS(int track, int sector_num);
	//@}

	/// @name directory
	//@{
//	/// @brief ルートディレクトリか
//	virtual bool	IsRootDirectory(wxUint32 group_num);
//	/// @brief サブディレクトリを作成できるか
//	virtual bool	CanMakeDirectory() const { return true; }
//	/// @brief サブディレクトリのサイズを拡張できるか
//	virtual bool	CanExpandDirectory() const { return true; }
//	/// @brief サブディレクトリを作成する前にディレクトリ名を編集する
//	virtual bool	RenameOnMakingDirectory(wxString &dir_name);
//	/// @brief サブディレクトリを作成した後の個別処理
//	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
//	/// @brief 未使用のディレクトリアイテムを返す
//	virtual DiskBasicDirItem *GetEmptyDirectoryItem(DiskBasicDirItem *parent, DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item);
	//@}

	/// @name format
	//@{
//	/// @brief フォーマット時セクタデータを指定コードで埋める
//	virtual void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
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
//	/// @brief データの書き込み終了後の処理
//	virtual void	AdditionalProcessOnSavedFile(DiskBasicDirItem *item);
	//@}

	/// @name delete
	//@{
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
//	/// @brief ファイル削除後の処理
//	virtual bool	AdditionalProcessOnDeletedFile(DiskBasicDirItem *item);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicTypeTRSD23

@brief TRSDOS 2.x の処理

@sa DiskBasicDirItemTRSD23
*/
class DiskBasicTypeTRSD23 : public DiskBasicTypeTRSDOS
{
protected:
	DiskBasicTypeTRSD23() : DiskBasicTypeTRSDOS() {}
	DiskBasicTypeTRSD23(const DiskBasicType &src) : DiskBasicTypeTRSDOS() {}

	/// @brief Overflowエントリを作成する
	int			CreateOverflowEntry(DiskBasicDirItemTRSDOS **ptitem, int &pos);

public:
	DiskBasicTypeTRSD23(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @brief ハッシュの格納位置からセクタ番号を得る
	virtual void GetFromHIPosition(wxUint32 pos, int &sector_num, int &pos_in_sector);
	/// @brief ハッシュの格納位置を得る
	static wxUint32 GetHIPosition(int sector_num, int pos_in_sector);
	/// @brief ハッシュを計算
	static wxUint8 ComputeHI(const wxUint8 *name);

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリをアサイン
	virtual bool	AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	//@}
	/// @name file chain
	//@{
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	//@}
	/// @name directory
	//@{
	/// @brief 未使用のディレクトリアイテムを返す
	virtual DiskBasicDirItem *GetEmptyDirectoryItem(DiskBasicDirItem *parent, DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item);
	//@}
	/// @name format
	//@{
	/// @brief フォーマット時セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}
};

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicTypeTRSD13

@brief TRSDOS 1.3 の処理

@sa DiskBasicDirItemTRSD13
*/
class DiskBasicTypeTRSD13 : public DiskBasicTypeTRSDOS
{
protected:
	DiskBasicTypeTRSD13() : DiskBasicTypeTRSDOS() {}
	DiskBasicTypeTRSD13(const DiskBasicType &src) : DiskBasicTypeTRSDOS() {}

	/// @brief ハッシュの格納位置からセクタ番号を得る
	virtual void GetFromHIPosition(wxUint32 pos, int &sector_num, int &pos_in_sector);

public:
	DiskBasicTypeTRSD13(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @brief ハッシュの格納位置を得る
	static wxUint32 GetHIPosition(int pos);

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリをアサイン
	virtual bool	AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	virtual int		FinishAssigningDirectory(int &pos, int &size, int &size_remain) const;
	/// @brief ディレクトリアサインでセクタ毎に位置を調整する
	virtual int     AdjustPositionAssigningDirectory(int pos);
	//@}
	/// @name file chain
	//@{
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	//@}
	/// @name format
	//@{
	/// @brief フォーマット時セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* _BASICTYPE_TRSDOS_H_ */
