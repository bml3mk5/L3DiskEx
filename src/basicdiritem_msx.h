/// @file basicdiritem_msx.h
///
/// @brief disk basic directory item for MSX DISK BASIC / MSX-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_MSX_H_
#define _BASICDIRITEM_MSX_H_

#include "basicdiritem_msdos.h"


/// ディレクトリ１アイテム MSX-DOS
class DiskBasicDirItemMSX : public DiskBasicDirItemMSDOS
{
private:
	DiskBasicDirItemMSX() : DiskBasicDirItemMSDOS() {}
	DiskBasicDirItemMSX(const DiskBasicDirItemMSX &src) : DiskBasicDirItemMSDOS(src) {}

	/// @brief ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemMSX(DiskBasic *basic);
	DiskBasicDirItemMSX(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemMSX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString	GetFileAttrStr() const;

	/// @brief 日付のタイトル名（ダイアログ用）
	wxString	GetFileDateTimeTitle() const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	//@}
};

#endif /* _BASICDIRITEM_MSX_H_ */
