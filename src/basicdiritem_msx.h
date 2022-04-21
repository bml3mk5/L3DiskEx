/// @file basicdiritem_msx.h
///
/// @brief disk basic directory item for MSX DISK BASIC / MSX-DOS
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

public:
	DiskBasicDirItemMSX(DiskBasic *basic);
	DiskBasicDirItemMSX(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemMSX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ファイル名に設定できない文字を文字列にして返す
	wxString	InvalidateChars();

	/// 属性の文字列を返す(ファイル一覧画面表示用)
	wxString	GetFileAttrStr();

	/// 日付のタイトル名（ダイアログ用）
	wxString	GetFileDateTimeTitle();

	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int file_type_1, int file_type_2, wxBoxSizer *sizer, wxSizerFlags &flags, AttrControls &controls, int *user_data);
	/// 属性1を得る
	int		GetFileType1InAttrDialog(const AttrControls &controls) const;
	/// 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const AttrControls &controls, DiskBasicError &errinfo);
	//@}
};

#endif /* _BASICDIRITEM_MSX_H_ */
