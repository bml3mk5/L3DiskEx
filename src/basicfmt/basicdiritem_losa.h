/// @file basicdiritem_losa.h
///
/// @brief disk basic directory item for L-os Angeles
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_LOSA_H
#define BASICDIRITEM_LOSA_H

#include "basicdiritem_msdos.h"


//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム L-os Angeles (MS-DOS compatible)
class DiskBasicDirItemLOSA : public DiskBasicDirItemMSDOS
{
protected:
	DiskBasicDirItemLOSA() : DiskBasicDirItemMSDOS() {}
	DiskBasicDirItemLOSA(const DiskBasicDirItemLOSA &src) : DiskBasicDirItemMSDOS(src) {}

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;

	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);

public:
	DiskBasicDirItemLOSA(DiskBasic *basic);
	DiskBasicDirItemLOSA(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemLOSA(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;


	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* BASICDIRITEM_LOSA_H */
