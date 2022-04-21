/// @file basictype_x1hu.h
///
/// @brief disk basic fat type for X1 Hu-BASIC
///
#ifndef _BASICTYPE_X1HU_H_
#define _BASICTYPE_X1HU_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype.h"

/// X1 Hu-BASICの処理
class DiskBasicTypeX1HU : public DiskBasicType
{
private:
	DiskBasicTypeX1HU() : DiskBasicType() {}
	DiskBasicTypeX1HU(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeX1HU(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// FAT位置をセット
	void		SetGroupNumber(wxUint32 num, wxUint32 val);
	/// FAT位置を返す
	wxUint32	GetGroupNumber(wxUint32 num);
	/// 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
	/// 次の空き位置を返す
	wxUint32	GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name check / assign FAT area
	//@{
	/// FATエリアをチェック
	bool		CheckFat();
	//@}

	/// @name disk size
	//@{
	/// 残りディスクサイズを計算
	void		CalcDiskFreeSize();
	//@}

	/// @name file chain
	//@{
	/// グループ番号から開始セクタ番号を得る
	int			GetStartSectorFromGroup(wxUint32 group_num);
	/// グループ番号から最終セクタ番号を得る
	int			GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size);
	//@}

	/// @name directory
	//@{
	/// ルートディレクトリか
	bool		IsRootDirectory(wxUint32 group_num);
	/// サブディレクトリを作成できるか
	bool		CanMakeDirectory() const { return true; }
	/// サブディレクトリを作成する前の個別処理
	bool		PreProcessOnMakingDirectory(wxString &dir_name);
	/// サブディレクトリを作成した後の個別処理
	void		AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item, wxUint32 parent_group_num);
	//@}

	/// @name format
	//@{
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
	/// 最後のグループ番号を計算する
	wxUint32	CalcLastGroupNumber(wxUint32 group_num, int size_remain);

//	size_t		WriteLastData(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, size_t size, size_t last_size, int sector_num, wxUint32 next_group, int sector_end, wxUint16 &next_sector);
	//@}
};

#endif /* _BASICTYPE_X1HU_H_ */
