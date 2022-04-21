/// @file basicdiritem_hu68k.h
///
/// @brief disk basic directory item for Human68k
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_HU68K_H_
#define _BASICDIRITEM_HU68K_H_

#include "basicdiritem_msdos.h"


/// ディレクトリ１アイテム Human68k
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
	DiskBasicDirItemHU68K(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemHU68K(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
};

#endif /* _BASICDIRITEM_HU68K_H_ */
