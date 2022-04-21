/// @file basictype.h
///
/// @brief disk basic type
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICTYPE_H_
#define _BASICTYPE_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"
#include "basicfat.h"
#include "utils.h"


#define INVALID_GROUP_NUMBER	((wxUint32)-1)

/// データの書き出しや読み込みで使用するテンポラリバッファ
class DiskBasicTempData : public Utils::TempData
{
};

class DiskBasic;
class DiskBasicFat;
class DiskBasicDir;
class DiskBasicDirItem;
class DiskBasicError;
class DiskBasicIdentifiedData;

/// セクタを確保する時のフラグ
enum AllocateGroupFlags {
	ALLOCATE_GROUPS_NEW = 0,
	ALLOCATE_GROUPS_APPEND,
};

/// DISK BASIC 機種依存 個別の処理テンプレート
class DiskBasicType
{
protected:
	DiskBasic  *basic;
	DiskBasicFat *fat;
	DiskBasicDir *dir;

	wxUint32 managed_start_group;	///< 管理エリアの開始グループ番号

	wxUint32 data_start_group;	///< データ開始グループ番号

	int			 free_disk_size;	///< 残りディスクサイズ
	int			 free_groups;		///< 残りグループサイズ
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
	virtual double	ParseParamOnDisk(DiskD88Disk *disk, bool is_formatting) { return 1.0; }
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting) { return 1.0; }
	/// @brief FATの開始位置を得る
	void			GetStartNumOnFat(int &track_num, int &side_num, int &sector_num);
	/// @brief FATの終了位置を得る
	void			GetEndNumOnFat(int &track_num, int &side_num, int &sector_num);

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
	double			CheckDirectory(bool is_root, const DiskBasicGroups &group_items);
	/// @brief ディレクトリが空か
	virtual bool	IsEmptyDirectory(bool is_root, const DiskBasicGroups &group_items);
	/// @brief ディレクトリをアサイン
	bool			AssignDirectory(bool is_root, const DiskBasicGroups &group_items, DiskBasicDirItem *dir_item);
	/// @brief ディレクトリエリアのサイズに達したらアサイン終了するか
	/// @retval  0 : 終了しない
	/// @retval  1 : 強制的に未使用とする アサインは継続
	/// @retval -1 : 強制的にアサイン終了する
	virtual int		FinishAssigningDirectory(int size) const { return 0; }
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
	virtual void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
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
	/// @brief ファイルをセーブする前にデータを変換
	virtual bool	ConvertDataForSave(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream);
	/// @brief グループ確保時に最後のグループ番号を計算する
	virtual wxUint32 CalcLastGroupNumber(wxUint32 group_num, int &size_remain);
	/// @brief ファイルをセーブする前の準備を行う
	virtual bool	PrepareToSaveFile(wxInputStream &istream, int &file_size, const DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo) { return true; }
	/// @brief データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
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

#endif /* _BASICTYPE_H_ */
