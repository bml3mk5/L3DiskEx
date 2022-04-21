/// @file basicdiritem_l32D.h
///
/// @brief disk basic directory item for L3/S1 BASIC 2D/2HD
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_L32D_H_
#define _BASICDIRITEM_L32D_H_

#include "basicdiritem_fat8.h"


/// ディレクトリ１アイテム L3/S1 BASIC 倍密度 2D/2HD
class DiskBasicDirItemL32D : public DiskBasicDirItemFAT8
{
private:
	DiskBasicDirItemL32D() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemL32D(const DiskBasicDirItemL32D &src) : DiskBasicDirItemFAT8(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性２を返す
	int		GetFileType2() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 属性２のセット
	void	SetFileType2(int val);

	/// @brief 最終セクタの占有サイズをセット
	void	SetDataSizeOnLastSecotr(int val);
	/// @brief 最終セクタの占有サイズを返す
	int		GetDataSizeOnLastSector() const;

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);
	/// @brief ファイル名に拡張子を付ける
	wxString AddExtension(int file_type_1, const wxString &name) const;

public:
	DiskBasicDirItemL32D(DiskBasic *basic);
	DiskBasicDirItemL32D(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemL32D(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// @brief ファイルサイズをセット
	void	SetFileSize(int val);
	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);
	/// @brief 最初のグループ番号をセット
	void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief ディレクトリアイテムのサイズ
	size_t	GetDataSize() const;

//	/// @brief ダイアログ入力後のファイル名文字列を変換
//	void	ConvertFileNameAfterRenamed(wxString &filename) const;

	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
};

#endif /* _BASICDIRITEM_L32D_H_ */
