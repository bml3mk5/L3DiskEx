/// @file basicdiritem_falcom.h
///
/// @brief disk basic directory item for Falcom DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FALCOM_H_
#define _BASICDIRITEM_FALCOM_H_

#include "basicdiritem.h"


/// ディレクトリ１アイテム Falcom DOS
class DiskBasicDirItemFalcom : public DiskBasicDirItem
{
private:
	DiskBasicDirItemFalcom() : DiskBasicDirItem() {}
	DiskBasicDirItemFalcom(const DiskBasicDirItemFalcom &src) : DiskBasicDirItem(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

public:
	DiskBasicDirItemFalcom(DiskBasic *basic);
	DiskBasicDirItemFalcom(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemFalcom(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool			Check(bool &last);
//	/// @brief アイテムを削除できるか
//	bool			IsDeletable() const { return false; }
	/// @brief アイテムをロード・エクスポートできるか
	bool			IsLoadable() const { return true; }
//	/// @brief アイテムをコピーできるか
//	bool			IsCopyable() const { return false; }

	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString		GetFileAttrStr() const;

	/// @brief ファイルサイズを返す
	int				GetFileSize() const;
	/// @brief ファイルサイズとグループ数を計算する
	void			CalcFileUnitSize(int fileunit_num);

	/// @brief 指定ディレクトリのすべてのグループを取得
	void			GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);
	/// @brief 最初のグループ番号をセット
	void			SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32		GetStartGroup(int fileunit_num) const;
	/// @brief 最後のグループ番号をセット(機種依存)
	void			SetLastGroup(wxUint32 val);
	/// @brief 最後のグループ番号を返す(機種依存)
	wxUint32		GetLastGroup() const;

	/// @brief ディレクトリアイテムのサイズ
	size_t			GetDataSize() const;

	/// @brief ファイルの終端コードをチェックする必要があるか
	bool			NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	int				RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ディレクトリをクリア ファイル新規作成時
	void			ClearData();

	/// @brief アイテムがアドレスを持っているか
	bool			HasAddress() const { return true; }
	/// @brief アドレスを編集できるか
	bool			IsAddressEditable() const { return false; }
	/// @brief 開始アドレスを返す
	int				GetStartAddress() const;
	/// @brief 終了アドレスを返す
	int				GetEndAddress() const;
	/// @brief 実行アドレスを返す
	int				GetExecuteAddress() const;

	/// @brief ファイル名から属性を決定する
	int				ConvFileTypeFromFileName(const wxString &filename) const;
	/// @brief ファイル名から属性を決定する
	int				ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
	//@}
};

#endif /* _BASICDIRITEM_FALCOM_H_ */
