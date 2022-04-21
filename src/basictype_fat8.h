/// @file basictype_fat8.h
///
/// @brief disk basic fat type
///
#ifndef _BASICTYPE_FAT8_H_
#define _BASICTYPE_FAT8_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype.h"

/// FAT8の処理
class DiskBasicTypeFAT8 : public DiskBasicType
{
protected:
	DiskBasicTypeFAT8() : DiskBasicType() {}
	DiskBasicTypeFAT8(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeFAT8(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeFAT8() {}

	/// @name access to FAT area
	//@{
	/// FAT位置をセット
	virtual void	SetGroupNumber(wxUint32 num, wxUint32 val);
	/// FAT位置を返す
	virtual wxUint32 GetGroupNumber(wxUint32 num) const;
	//@}

	/// @name file size
	//@{
	//@}

	/// @name file chain
	//@{
	//@}

	/// @name format
	//@{
	/// セクタデータを指定コードで埋める
	virtual void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// セクタデータを埋めた後の個別処理 FAT予約済みをセット
	virtual bool	AdditionalProcessOnFormatted();
	//@}

	/// @name save / write
	//@{
	/// 最後のグループ番号を計算する
	virtual wxUint32 CalcLastGroupNumber(wxUint32 group_num, int size_remain);
	//@}
};

/// FAT8の処理 (F-BASIC, L3 1S)
class DiskBasicTypeFAT8F : public DiskBasicTypeFAT8
{
protected:
	DiskBasicTypeFAT8F() : DiskBasicTypeFAT8() {}
	DiskBasicTypeFAT8F(const DiskBasicType &src) : DiskBasicTypeFAT8(src) {}
public:
	DiskBasicTypeFAT8F(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);
	virtual ~DiskBasicTypeFAT8F() {}

	/// @name access to FAT area
	//@{
	/// 次の空き位置を返す
	virtual wxUint32 GetNextEmptyGroupNumber(wxUint32 curr_group);
	//@}

	/// @name file chain
	//@{
	/// スキップするトラック番号
	virtual int CalcSkippedTrack();
	//@}

	/// @name data access (read / verify)
	//@{
	/// ファイルの最終セクタのデータサイズを求める
	virtual int CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}
};

#endif /* _BASICTYPE_FAT8_H_ */
