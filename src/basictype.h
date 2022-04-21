/// @file basictype.h
///
/// @brief disk basic type
///
#ifndef _BASICTYPE_H_
#define _BASICTYPE_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "basiccommon.h"
#include "basicfat.h"

#define INVALID_GROUP_NUMBER	((wxUint32)-1)

/// データの書き出しや読み込みで使用するテンポラリバッファ
class DiskBasicTempData {
private:
	wxUint8 data[2048];
	size_t  size;
public:
	DiskBasicTempData();
	~DiskBasicTempData() {}
	void SetSize(size_t val) { size = val < 2048 ? val : 2048; }
	void SetData(const wxUint8 *data, size_t len);
	wxUint8 *GetData() { return data; }
	size_t GetSize() const { return size; } 
};

class DiskBasic;
class DiskBasicFat;
class DiskBasicDir;
class DiskBasicDirItem;

/// DISK BASIC 機種依存 個別の処理テンプレート
class DiskBasicType
{
protected:
	DiskBasic  *basic;
	DiskBasicFat *fat;
	DiskBasicDir *dir;

	int      sector_size;		///< セクタサイズ
//	int      grps_per_track;	///< 1トラックあたりのグループ数
	int      secs_per_group;	///< １グループのセクタ数

	wxUint32 managed_start_group;	///< 管理エリアの開始グループ番号
	wxUint32 end_group;			///< 最終グループ番号

	wxUint32 data_start_group;	///< データ開始グループ番号

	wxUint32 group_final_code;	///< 最終グループのコード(0xc0 - )
	wxUint32 group_system_code;	///< システムで使用するコード(0xfe)
	wxUint32 group_unused_code;	///< 未使用のコード(0xff)

	int			 free_disk_size;	///< 残りディスクサイズ
	int			 free_groups;		///< 残りグループサイズ
	wxArrayInt	 fat_availability;	///< FATの空き状況

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
	/// FAT位置をセット
	virtual void	SetGroupNumber(wxUint32 num, wxUint32 val);
	/// FAT位置を返す
	virtual wxUint32 GetGroupNumber(wxUint32 num);
	/// 使用しているグループ番号か
	virtual bool	IsUsedGroupNumber(wxUint32 num);
	/// 次のグループ番号を得る
	virtual wxUint32 GetNextGroupNumber(wxUint32 num, int sector);
	/// 空きFAT位置を返す
	virtual wxUint32 GetEmptyGroupNumber();
	/// 次の空きFAT位置を返す
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// FATエリアをチェック
	virtual bool	CheckFat();

	/// ディスクから各パラメータを取得
	virtual bool	ParseParamOnDisk(DiskD88Disk *disk) { return true; }

	/// 管理エリアのトラック番号からグループ番号を計算
	virtual wxUint32 CalcManagedStartGroup();
	//@}

	/// @name check / assign directory area
	//@{
	/// ルートディレクトリのチェック
	virtual bool	CheckRootDirectory(int start_sector, int end_sector);
	/// ルートディレクトリをアサイン
	virtual bool	AssignRootDirectory(int start_sector, int end_sector);
	/// ディレクトリのチェック
	virtual bool	CheckDirectory(const DiskBasicGroups &group_items);
	/// ディレクトリが空か
	virtual bool	IsEmptyDirectory(const DiskBasicGroups &group_items);
	/// ディレクトリをアサイン
	virtual bool	AssignDirectory(const DiskBasicGroups &group_items);
	//@}

	/// @name disk size
	//@{
	/// 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize();
	/// 残りディスクサイズをクリア
	void			ClearDiskFreeSize();
	/// 残りディスクサイズを得る(CalcDiskFreeSize()で計算した結果)
	int				GetFreeDiskSize() const { return free_disk_size; }
	/// 残りグループを得る(CalcDiskFreeSize()で計算した結果)
	int				GetFreeGroupSize() const { return free_groups; }
	/// FATの空き状況を配列で返す
	void			GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const;
	//@}

	/// @name file chain
	//@{
	/// データサイズ分のグループを確保する
	virtual int		AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items);

	/// グループ番号から開始セクタ番号を得る
	virtual int		GetStartSectorFromGroup(wxUint32 group_num);
	/// グループ番号から最終セクタ番号を得る
	virtual int		GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// データ領域の開始セクタを計算
	virtual int		CalcDataStartSectorPos();
	/// スキップするトラック番号
	virtual int		CalcSkippedTrack();
	/// サイド番号を逆転するか
	virtual bool	IsSideReversed(int sides_per_disk);
	/// ディスク内のデータが反転しているか
	virtual bool	IsDataInverted();
	//@}

	/// @name directory
	//@{
	/// ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return false; }
	/// サブディレクトリを作成する前の個別処理
	virtual bool	PreProcessOnMakingDirectory(wxString &dir_name) { return true; }
	/// サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item, wxUint32 parent_group_num) {}
	//@}

	/// @name format
	//@{
	/// フォーマットできるか
	virtual bool	IsFormattable() const { return true; }
	/// セクタデータを指定コードで埋める
	virtual void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// セクタデータを埋めた後の個別処理
	virtual void	AdditionalProcessOnFormatted();
	//@}

	/// @name data access (read / verify)
	//@{
	/// ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	/// データの読み込み/比較処理
	virtual int		AccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end);
	//@}

	/// @name save / write
	//@{
	/// 書き込み可能か
	virtual bool	IsWritable() const { return true; }
	/// 最後のグループ番号を計算する
	virtual wxUint32 CalcLastGroupNumber(wxUint32 group_num, int size_remain);
	/// セーブ時にセクタがなかった時の処理
	virtual bool	SetSkipMarkOnErrorSector(DiskBasicDirItem *item, wxUint32 prev_group, wxUint32 group, wxUint32 next_group);
	/// データの書き込み処理
	virtual int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
	/// データの書き込み終了後の処理
	virtual void	AdditionalProcessOnSavedFile(DiskBasicDirItem *item) {}

	/// ファイル名変更後の処理
	virtual void	AdditionalProcessOnRenamedFile(DiskBasicDirItem *item) {}
	//@}

	/// @name delete
	//@{
	/// ファイルを削除できるか
	virtual bool	IsDeletable() const { return true; }
	/// 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
	//@}

	/// @name property
	//@{
	void		SetSectorSize(int val)			{ sector_size = val; }
//	void		SetGrpsPerTrack(int val)		{ grps_per_track = val; }
	void		SetSecsPerGroup(int val)		{ secs_per_group = val; }
	void		SetManagedStartGroup(wxUint32 val) { managed_start_group = val; }
	void		SetEndGroup(wxUint32 val)		{ end_group = val; }
	void		SetGroupFinalCode(wxUint32 val) { group_final_code = val; }
	void		SetGroupSystemCode(wxUint32 val) { group_system_code = val; }
	void		SetGroupUnusedCode(wxUint32 val) { group_unused_code = val; }
	//@}
};

#endif /* _BASICTYPE_H_ */
