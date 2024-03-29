/// @file basicdiritem_hu68k.h
///
/// @brief disk basic directory item for Human68k
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_HU68K_H
#define BASICDIRITEM_HU68K_H

#include "basicdiritem_msdos.h"


//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Human68k
class DiskBasicDirItemHU68K : public DiskBasicDirItemMSDOS
{
protected:
	DiskBasicDirItemHU68K() : DiskBasicDirItemMSDOS() {}
	DiskBasicDirItemHU68K(const DiskBasicDirItemHU68K &src) : DiskBasicDirItemMSDOS(src) {}

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;

	/// @brief ダイアログ表示前にファイルの属性を設定
	virtual void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemHU68K(DiskBasic *basic);
	DiskBasicDirItemHU68K(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemHU68K(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;

	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
};

#endif /* BASICDIRITEM_HU68K_H */
