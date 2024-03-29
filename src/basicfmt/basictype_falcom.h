/// @file basictype_falcom.h
///
/// @brief disk basic fat type for Falcom DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_FALCOM_H
#define BASICTYPE_FALCOM_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype.h"


/** @class DiskBasicTypeFalcom

@brief Falcom DOS の処理

*/
class DiskBasicTypeFalcom : public DiskBasicType
{
protected:
	DiskBasicTypeFalcom() : DiskBasicType() {}
	DiskBasicTypeFalcom(const DiskBasicType &src) : DiskBasicType(src) {}
public:
	DiskBasicTypeFalcom(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name access to FAT area
	//@{
	//@}

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	//@}

	/// @name disk size
	//@{
	/// @brief 残りディスクサイズを計算
	virtual void	CalcDiskFreeSize(bool wrote);
	//@}

	/// @name file chain
	//@{
	//@}

	/// @name directory
	//@{
	//@}

	/// @name format
	//@{
	/// @brief フォーマットできるか
	virtual bool	SupportFormatting() const { return false; }
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	//@}

	/// @name data access (read / verify)
	//@{
	/// @brief ファイルの最終セクタのデータサイズを求める
	virtual int		CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size);
	//@}

	/// @name save / write
	//@{
	/// @brief 書き込み可能か
	virtual bool	SupportWriting() const { return false; }
	//@}

	/// @name delete
	//@{
	/// @brief ファイルを削除できるか
	virtual bool	SupportDeleting() const { return false; }
	/// @brief 指定したグループ番号のFAT領域を削除する
	virtual void	DeleteGroupNumber(wxUint32 group_num);
	//@}

	/// @name property
	//@{
	//@}
};

#endif /* BASICTYPE_FALCOM_H */
