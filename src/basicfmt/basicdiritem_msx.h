/// @file basicdiritem_msx.h
///
/// @brief disk basic directory item for MSX DISK BASIC / MSX-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_MSX_H
#define BASICDIRITEM_MSX_H

#include "basicdiritem_msdos.h"


//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム MSX-DOS
class DiskBasicDirItemMSX : public DiskBasicDirItemMSDOS
{
private:
	DiskBasicDirItemMSX() : DiskBasicDirItemMSDOS() {}
	DiskBasicDirItemMSX(const DiskBasicDirItemMSX &src) : DiskBasicDirItemMSDOS(src) {}

	/// @brief ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemMSX(DiskBasic *basic);
	DiskBasicDirItemMSX(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemMSX(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief 日付のタイトル名（ダイアログ用）
	virtual wxString GetFileCreateDateTimeTitle() const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	//@}
};

#endif /* BASICDIRITEM_MSX_H */
