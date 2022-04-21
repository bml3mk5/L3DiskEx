/// @file basictype_n88.h
///
/// @brief disk basic fat type
///
#ifndef _BASICTYPE_N88_H_
#define _BASICTYPE_N88_H_

#include "common.h"
#include "basiccommon.h"
#include "basictype_fat8.h"

/// N88-BASICの処理
class DiskBasicTypeN88 : public DiskBasicTypeFAT8
{
private:
	DiskBasicTypeN88() : DiskBasicTypeFAT8() {}
	DiskBasicTypeN88(const DiskBasicType &src) : DiskBasicTypeFAT8(src) {}
public:
	DiskBasicTypeN88(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	/// 空きFAT位置を返す
	wxUint32	GetEmptyGroupNumber();
	//@}

	/// @name format
	//@{
	/// セクタデータを指定コードで埋める
	void	FillSector(DiskD88Track *track, DiskD88Sector *sector);
	/// セクタデータを埋めた後の個別処理
	void	AdditionalProcessOnFormatted();
	//@}

	/// @name data access (read / verify)
	//@{
	/// ファイルの最終セクタのデータサイズを求める
	int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// データの書き込み処理
	int		WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end);
	//@}
};

#endif /* _BASICTYPE_N88_H_ */
