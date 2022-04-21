/// @file basicdiritem_falcom.h
///
/// @brief disk basic directory item for Falcom DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FALCOM_H_
#define _BASICDIRITEM_FALCOM_H_

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Falcom DOS
class DiskBasicDirItemFalcom : public DiskBasicDirItem
{
private:
	DiskBasicDirItemFalcom() : DiskBasicDirItem() {}
	DiskBasicDirItemFalcom(const DiskBasicDirItemFalcom &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_falcom_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

public:
	DiskBasicDirItemFalcom(DiskBasic *basic);
	DiskBasicDirItemFalcom(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemFalcom(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
//	/// @brief アイテムを削除できるか
//	bool			IsDeletable() const { return false; }
	/// @brief アイテムをロード・エクスポートできるか
	bool			IsLoadable() const { return true; }
//	/// @brief アイテムをコピーできるか
//	bool			IsCopyable() const { return false; }
	/// @brief 削除
	virtual bool	Delete();

	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズを返す
	virtual int		GetFileSize() const;
	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);

	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);
	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 最後のグループ番号をセット(機種依存)
	virtual void	SetLastGroup(wxUint32 val);
	/// @brief 最後のグループ番号を返す(機種依存)
	virtual wxUint32 GetLastGroup() const;

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }
	/// @brief アドレスを編集できるか
	virtual bool	IsAddressEditable() const { return false; }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 終了アドレスを返す
	virtual int		GetEndAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;

	/// @brief ファイル名から属性を決定する
	virtual int		ConvFileTypeFromFileName(const wxString &filename) const;
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_FALCOM_H_ */
