/// @file basicdiritem_l32D.h
///
/// @brief disk basic directory item for L3/S1 BASIC 2D/2HD
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_L32D_H_
#define _BASICDIRITEM_L32D_H_

#include "basicdiritem_fat8.h"


//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム L3/S1 BASIC 倍密度 2D/2HD
class DiskBasicDirItemL32D : public DiskBasicDirItemFAT8
{
private:
	DiskBasicDirItemL32D() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemL32D(const DiskBasicDirItemL32D &src) : DiskBasicDirItemFAT8(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_l3_2d_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性２を返す
	virtual int	GetFileType2() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);

	/// @brief 最終セクタの占有サイズをセット
	void			SetDataSizeOnLastSecotr(int val);
	/// @brief 最終セクタの占有サイズを返す
	int				GetDataSizeOnLastSector() const;

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);
	/// @brief ファイル名に拡張子を付ける
	virtual wxString AddExtension(int file_type_1, const wxString &name) const;

public:
	DiskBasicDirItemL32D(DiskBasic *basic);
	DiskBasicDirItemL32D(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemL32D(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 削除
	virtual bool	Delete();

	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);
	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
};

#endif /* _BASICDIRITEM_L32D_H_ */
