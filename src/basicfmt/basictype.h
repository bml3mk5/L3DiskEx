/// @file basictype.h
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_H
#define BASICTYPE_H

#include "../common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"
#include "basicfat.h"
#include "../utils.h"


#define INVALID_GROUP_NUMBER	((wxUint32)-1)

class wxInputStream;
class wxOutputStream;
class DiskImageSector;
class DiskImageTrack;
class DiskBasic;
class DiskBasicFat;
class DiskBasicDir;
class DiskBasicDirItem;
class DiskBasicDirItems;
class DiskBasicError;
class DiskBasicIdentifiedData;

/// セクタを確保する時のフラグ
enum AllocateGroupFlags {
	ALLOCATE_GROUPS_NEW = 0,
	ALLOCATE_GROUPS_APPEND,
};

//////////////////////////////////////////////////////////////////////

/// @brief データの書き出しや読み込みで使用するテンポラリバッファ
class DiskBasicTempData : public Utils::TempData
{
};

//////////////////////////////////////////////////////////////////////

/// @brief セクタスキューマップ基底
///
/// @sa SectorsPerTrack , DiskBasicSectorPosTrans
class SectorSkewBase
{
protected:
	int  m_num_secs;	///< セクタ数

public:
	SectorSkewBase();
	SectorSkewBase(const SectorSkewBase &src);
	virtual ~SectorSkewBase() {}
	/// @brief クローン コピー時に使用する
	virtual SectorSkewBase *Clone() const { return new SectorSkewBase(*this); }
	/// @brief スキューマップを削除
	virtual void Delete() {}
	/// @brief スキューマップを作成
	virtual void Create(DiskBasic *basic, int num_secs);
	/// @brief スキューマップをマッピング
	virtual void Mapping(DiskBasic *basic) {}
	/// @brief 物理番号に変換
	virtual int ToPhysical(int val) const { return 0; }
	/// @brief 論理番号に変換
	virtual int ToLogical(int val) const { return 0; }
};

/// @brief セクタスキューマップ
///
/// @sa SectorsPerTrack , DiskBasicSectorPosTrans
class DiskBasicSectorSkew : public SectorSkewBase
{
protected:
	int *m_ltop_map;	///< 論理セクタ番号 から 物理セクタ番号
	int *m_ptol_map;	///< 物理セクタ番号 から 論理セクタ番号

	void MappingFromParam(DiskBasic *basic);
	void MappingFromCalc(DiskBasic *basic, int skew);
public:
	DiskBasicSectorSkew();
	DiskBasicSectorSkew(const DiskBasicSectorSkew &src);
	virtual ~DiskBasicSectorSkew();
	/// @brief クローン コピー時に使用する
	virtual SectorSkewBase *Clone() const { return new DiskBasicSectorSkew(*this); }
	/// @brief スキューマップを削除
	virtual void Delete();
	/// @brief スキューマップを作成
	virtual void Create(DiskBasic *basic, int num_secs);
	/// @brief スキューマップをマッピング
	virtual void Mapping(DiskBasic *basic);
	/// @brief 物理番号に変換
	int ToPhysical(int val) const;
	/// @brief 論理番号に変換
	int ToLogical(int val) const;
};

/// @brief セクタスキューマップ インポート時の空きセクタの割り当て方
class DiskBasicSectorSkewForSave : public DiskBasicSectorSkew
{
public:
	DiskBasicSectorSkewForSave();
	DiskBasicSectorSkewForSave(const DiskBasicSectorSkewForSave &src);
	/// @brief クローン コピー時に使用する
	virtual SectorSkewBase *Clone() const { return new DiskBasicSectorSkewForSave(*this); }
	/// @brief スキューマップをマッピング
	virtual void Mapping(DiskBasic *basic);
};

//////////////////////////////////////////////////////////////////////

/// @brief トラックごとのセクタ数を保持
///
/// @sa DiskBasicSectorPosTrans
class SectorsPerTrack
{
private:
	int m_num_of_tracks;		///< トラック数
	int m_num_of_sectors;		///< セクタ数(両サイド通して)
	int m_total_sectors;		///< 合計セクタ数
	SectorSkewBase *m_ssmap;	///< セクタスキュー
public:
	SectorsPerTrack();
	SectorsPerTrack(const SectorsPerTrack &src);
	SectorsPerTrack(int num_of_tracks, int num_of_sectors, int total_sectors);
	~SectorsPerTrack();
	/// @brief トラック数を返す
	int GetNumOfTracks() const { return m_num_of_tracks; }
	/// @brief セクタ数を返す
	int GetNumOfSectors() const { return m_num_of_sectors; }
	/// @brief 合計セクタ数を返す
	int GetTotalSectors() const { return m_total_sectors; }
	/// @brief スキューマップを設定
	void SetSectorSkewMap(SectorSkewBase *map) { m_ssmap = map; }
	/// @brief スキューマップを返す
	const SectorSkewBase *GetSectorSkewMap() const { return m_ssmap; }
};

WX_DECLARE_OBJARRAY(SectorsPerTrack, ArrayOfSectorsPerTrack);

/// @brief セクタ位置変換マップリスト SectorsPerTrack の配列
///
/// トラック毎にセクタ数が異なる場合、このセクタ数リストを使って
/// セクタ位置を求める。
class DiskBasicSectorPosTrans : public ArrayOfSectorsPerTrack
{
public:
	virtual void Create(DiskBasic *basic);
	/// @brief セクタスキューマップを作る
	///
	/// @note 使用する場合は派生クラスを作る
	virtual void CreateSectorSkewMap(DiskBasic *basic) {}
	/// @brief 指定トラックのアイテムを返す
	const SectorsPerTrack *FindByTrackNum(int track_num) const;
	/// @brief 全トラックのセクタ数を返す
	int GetTotalSectors() const;
	/// @brief 論理セクタ位置(最初のトラック＆セクタを0とした通し番号)からトラック、セクタの各番号を得る サイド番号はセクタ番号の通し番号に変換
	void GetNumFromSectorPos(int sector_pos, int &track_num, int &sector_num, int &num_of_sectors) const;
	/// @brief トラック、セクタの各番号からセクタ位置(最初のトラック＆セクタを0とした通し番号)を得る サイド番号はセクタ番号の通し番号に変換
	int GetSectorPosFromNum(int track_num, int sector_num, int &num_of_sectors) const;
};

//////////////////////////////////////////////////////////////////////

/// @brief DISK BASIC 機種依存 個別の処理テンプレート
///
/// 抽象クラス
class DiskBasicType
{
protected:
	DiskBasic  *basic;
	DiskBasicFat *fat;
	DiskBasicDir *dir;

	wxUint32 managed_start_group;	///< 管理エリアの開始グループ番号

	wxUint32 data_start_group;	///< データ開始グループ番号

//	int			 free_disk_size;	///< 残りディスクサイズ
//	int			 free_groups;		///< 残りグループサイズ
	DiskBasicAvailabillity fat_availability;	///< 使用状況(FAT,グループ単位)

	/// ファイルアクセス時のテンポラリバッファ
	DiskBasicTempData temp;

	DiskBasicType() {}
	DiskBasicType(const DiskBasicType &) {}
	DiskBasicType &operator=(const DiskBasicType &) { return *this; }
public:
	DiskBasicType(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicType();

	/// @name access to FAT area
	//@{
	/// @brief FAT位置をセット
	virtual void	SetGroupNumber(wxUint32 num, wxUint32 val);
	/// @brief FAT位置を返す
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
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	/// @param[in] is_formatting フォーマット中か
	virtual double	ParseParamOnDisk(bool is_formatting) { return 1.0; }
	/// @brief FATエリアをチェック
	/// @param[in] is_formatting フォーマット中か
	virtual double 	CheckFat(bool is_formatting) { return 1.0; }
	/// @brief FATの開始位置を得る（ダイアログ用）
	virtual void	GetStartNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief FATの終了位置を得る（ダイアログ用）
	virtual void	GetEndNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief "FAT"などのタイトル名（ダイアログ用）
	virtual wxString GetTitleForFat() const;

	/// @brief 管理エリアのトラック番号からグループ番号を計算
	virtual wxUint32 CalcManagedStartGroup();
	//@}

	/// @name check / assign directory area
	//@{
	/// @brief ルートディレクトリのセクタリストを計算
	virtual bool	CalcGroupsOnRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items);
	/// @brief ルートディレクトリのチェック
	double			CheckRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, bool is_formatting);
	/// @brief ルートディレクトリをアサイン
	virtual bool	AssignRootDirectory(int start_sector, int end_sector, DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief ディレクトリのチェック
	virtual double	CheckDirectory(bool is_root, const DiskBasicGroups &group_items);
	/// @brief ディレクトリが空か
	virtual bool	IsEmptyDirectory(bool is_root, const DiskBasicGroups &group_items);
	/// @brief ディレクトリをアサイン
	virtual bool	AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief セクタをディレクトリとして初期化
	virtual int		InitializeSectorsAsDirectory(DiskBasicGroups &group_items, int &file_size, int &sizeremain, DiskBasicError &errinfo);
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	/// @param[in,out] pos         ディレクトリの位置
	/// @param[in,out] size        ディレクトリのセクタサイズ
	/// @param[in,out] size_remain ディレクトリの残りサイズ
	/// @retval  0 : 終了しない
	/// @retval  1 : 強制的に未使用とする アサインは継続
	/// @retval -1 : 現グループでアサイン終了。次のグループから継続
	/// @retval -2 : 強制的にアサイン終了する
	virtual int		FinishAssigningDirectory(int &pos, int &size, int &size_remain) const { return 0; }
	/// @brief ディレクトリアサインでセクタ毎に位置を調整する
	/// @param[in] pos ディレクトリの位置
	/// @return 調整後のディレクトリの位置
	virtual int     AdjustPositionAssigningDirectory(int pos) { return pos; }
	/// @brief ルートディレクトリの開始位置を得る
	void			GetStartNumOnRootDirectory(int &track_num, int &side_num, int &sector_num);
	/// @brief ルートディレクトリの終了位置を得る
	void			GetEndNumOnRootDirectory(int &track_num, int &side_num, int &sector_num);
	//@}

	/// @name disk size
	//@{
	/// @brief 使用可能なディスクサイズを得る
	virtual void	GetUsableDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize(bool wrote);
	/// @brief 残りディスクサイズをクリア
	void			ClearDiskFreeSize();
	/// @brief 残りディスクサイズを得る(CalcDiskFreeSize()で計算した結果)
	void			GetFreeDiskSize(int &disk_size, int &group_size) const;
	/// @brief 残りディスクサイズを得る(CalcDiskFreeSize()で計算した結果)
	int				GetFreeDiskSize() const;
	/// @brief 残りグループ数を得る(CalcDiskFreeSize()で計算した結果)
	int				GetFreeGroupSize() const;
	/// @brief FATの空き状況を配列で返す
	void			GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const;
	//@}

	/// @name file chain
	//@{
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief データサイズ分のグループを確保する
	virtual int		AllocateGroups(DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items);
	/// @brief グループをつなげる
	virtual int		ChainGroups(wxUint32 group_num, wxUint32 append_group_num);

	/// @brief グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// @brief グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// @brief データ領域の開始セクタを計算
	virtual int		CalcDataStartSectorPos();
	/// @brief スキップするトラック番号
	virtual int		CalcSkippedTrack();

	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
	virtual void	GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
	virtual void	GetNumFromSectorPosS(int sector_pos, int &track_num, int &sector_num);
	/// @brief セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
	virtual void	GetNumFromSectorPosT(int sector_pos, int &track_num, int &sector_num);
	/// @brief トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNum(int track, int side, int sector_num, int div_num = 0, int div_nums = 1);
	/// @brief トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNumS(int track, int sector_num);
	/// @brief トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	virtual int		GetSectorPosFromNumT(int track, int sector_num);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return false; }
	/// @brief ルートディレクトリのサイズを拡張できるか
	virtual bool	CanExpandRootDirectory() const { return false; }
	/// @brief サブディレクトリのサイズを拡張できるか
	virtual bool	CanExpandDirectory() const { return false; }
	/// @brief サブディレクトリを作成する前にディレクトリ名を編集する
	virtual bool	RenameOnMakingDirectory(wxString &dir_name) { return true; }
	/// @brief サブディレクトリを作成する前の準備を行う
	virtual bool	PrepareToMakeDirectory(DiskBasicDirItem *item) { return true; }
	/// @brief 未使用のディレクトリアイテムを返す
	virtual DiskBasicDirItem *GetEmptyDirectoryItem(DiskBasicDirItem *parent, DiskBasicDirItems *items, DiskBasicDirItem *pitem, DiskBasicDirItem **next_item);
	/// @brief ディレクトリアイテムをリリース（機種依存）
	virtual void	ReleaseDirectoryItem(DiskBasicDirItem *item) {}
	/// @brief サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item) {}
	/// @brief ディレクトリ拡張後の個別処理
	virtual bool	AdditionalProcessOnExpandedDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item) { return true; }
	//@}

	/// @name format
	//@{
	/// @brief フォーマットできるか
	virtual bool	SupportFormatting() const { return true; }
	/// @brief セクタデータを指定コードで埋める
	virtual void	FillSector(DiskImageTrack *track, DiskImageSector *sector);
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	/// @brief データの読み込み/比較の前処理
	virtual bool	PrepareToAccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, int &file_size, DiskBasicGroups &group_items, DiskBasicError &errinfo) { return true; }
	/// @brief データの読み込み/比較処理
	virtual int		AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	/// @brief 内部ファイルをエクスポートする際に内容を変換
	virtual bool	ConvertDataForLoad(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	/// @brief エクスポートしたファイルをベリファイする際に内容を変換
	virtual bool	ConvertDataForVerify(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	//@}

	/// @name save / write
	//@{
	/// @brief 書き込み可能か
	virtual bool	SupportWriting() const { return true; }
	/// @brief 指定したサイズが十分書き込めるか
	virtual bool	IsEnoughFileSize(int size) const { return true; }
	/// @brief ファイルをセーブする前にデータを変換
	virtual bool	ConvertDataForSave(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	/// @brief グループ確保時に最後のグループ番号を計算する
	virtual wxUint32 CalcLastGroupNumber(wxUint32 group_num, int &size_remain);
	/// @brief ファイルをセーブする前の準備を行う
	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo) { return true; }
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num);
	/// @brief データの書き込み終了後の処理
	virtual void	AdditionalProcessOnSavedFile(DiskBasicDirItem *item) {}

	/// @brief ファイル名変更後の処理
	virtual void	AdditionalProcessOnRenamedFile(DiskBasicDirItem *item) {}
	/// @brief 属性変更後の処理
	virtual void	AdditionalProcessOnChangedAttr(DiskBasicDirItem *item) {}
	//@}

	/// @name delete
	//@{
	/// @brief ファイルを削除できるか
	virtual bool	SupportDeleting() const { return true; }
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroups(const DiskBasicGroups &group_items);
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
	/// @brief ファイル削除後の処理
	virtual bool	AdditionalProcessOnDeletedFile(DiskBasicDirItem *item) { return true; }
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const {}
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data) {}
	/// @brief 管理エリアの開始グループをセット
	void			SetManagedStartGroup(wxUint32 val) { managed_start_group = val; }
	//@}
};

#endif /* BASICTYPE_H */
