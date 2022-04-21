/// @file basictype_cpm.h
///
/// @brief disk basic type for CP/M
///
#ifndef _BASICTYPE_CPM_H_
#define _BASICTYPE_CPM_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype.h"

/// CP/Mの処理
class DiskBasicTypeCPM : public DiskBasicType
{
private:
	DiskBasicTypeCPM() : DiskBasicType() {}
	DiskBasicTypeCPM(const DiskBasicType &src) : DiskBasicType(src) {}

public:
	DiskBasicTypeCPM(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// FATオフセットを返す
	wxUint32	GetGroupNumber(wxUint32 num);
	/// 使用しているグループ番号か
	bool		IsUsedGroupNumber(wxUint32 num);
	/// 次のグループ番号を得る
	wxUint32	GetNextGroupNumber(wxUint32 num, int sector_pos);
	/// 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
	/// 次の空きFAT位置を返す 未使用
	wxUint32	GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// FATエリアをチェック
	bool		CheckFat();
	/// ディスクから各パラメータを取得
	bool		ParseParamOnDisk(DiskD88Disk *disk);
	//@}

	/// @name check / assign directory area
	//@{
//	/// ルートディレクトリのチェック
//	bool		CheckRootDirectory(int start_sector, int end_sector);
	/// ルートディレクトリをアサイン
	bool		AssignRootDirectory(int start_sector, int end_sector);
	//@}

	/// @name disk size
	//@{
	/// 残りディスクサイズを計算
	void		CalcDiskFreeSize();
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
	/// データサイズ分のグループを確保する
	int			AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items);

	/// グループ番号から開始セクタ番号を得る
	int			GetStartSectorFromGroup(wxUint32 group_num);
	/// グループ番号から最終セクタ番号を得る
	int			GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);

	/// データ領域の開始セクタを計算
	int			CalcDataStartSectorPos();
	//@}

	/// @name directory
	//@{
	/// ルートディレクトリか
	bool		IsRootDirectory(wxUint32 group_num);
	/// サブディレクトリを作成できるか
	bool		CanMakeDirectory() const { return false; }
	//@}

	/// @name format
	//@{
	/// フォーマットできるか
	bool		IsFormattable() const { return false; }
	/// セクタデータを指定コードで埋める
	void		FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// セクタデータを埋めた後の個別処理
	void		AdditionalProcessOnFormatted();
	//@}

	/// @name data access (read / verify)
	//@{
	/// ファイルの最終セクタのデータサイズを求める
	int			CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// 書き込み可能か
	bool		IsWritable() const { return false; }
	/// データの書き込み処理
	int			WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
	/// データの書き込み終了後の処理
	void		AdditionalProcessOnSavedFile(DiskBasicDirItem *item);

	/// ファイル名変更後の処理
	void		AdditionalProcessOnRenamedFile(DiskBasicDirItem *item);
	//@}

	/// @name delete
	//@{
	/// ファイルを削除できるか
	bool		IsDeletable() const { return false; }
	/// 指定したグループ番号のFAT領域を削除する
	void		DeleteGroupNumber(wxUint32 group_num);
	//@}
};

#endif /* _BASICTYPE_CPM_H_ */
