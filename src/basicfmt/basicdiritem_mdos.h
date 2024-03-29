/// @file basicdiritem_mdos.h
///
/// @brief disk basic directory item for MDOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_MDOS_H
#define BASICDIRITEM_MDOS_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム MDOS
class DiskBasicDirItemMDOS : public DiskBasicDirItem
{
private:
	DiskBasicDirItemMDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemMDOS(const DiskBasicDirItemMDOS &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_mdos_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
//	/// @brief 属性１を返す
//	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
//	/// @brief 属性を変換
//	int				ConvFileType1(int file_type) const;
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

//	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
//	int				ConvFileType1Pos(int t1) const;
//	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
//	int				CalcFileTypeFromPos(int pos) const;
//	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
//	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

//	/// @brief ファイル内部のアドレスを取り出す
//	void			TakeAddressesInFile();

public:
	DiskBasicDirItemMDOS(DiskBasic *basic);
	DiskBasicDirItemMDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemMDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief 削除
	virtual bool	Delete();

//	/// @brief ENDマークがあるか(一度も使用していないか)
//	bool			HasEndMark();
//	/// @brief 次のアイテムにENDマークを入れる
//	void			SetEndMark(DiskBasicDirItem *next_item);

//	/// @brief 属性を設定
//	void			SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
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

	/// @brief アイテムが実行アドレスを持っているか
	virtual bool	HasExecuteAddress() const { return false; }

	/// @brief ファイル名から属性を決定する
	virtual int		ConvFileTypeFromFileName(const wxString &filename) const;
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
//	/// @brief ダイアログ内の属性部分のレイアウトを作成
//	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
//	/// @brief 属性を変更した際に呼ばれるコールバック
//	void	ChangeTypeInAttrDialog(IntNameBox *parent);
//	/// @brief 機種依存の属性を設定する
//	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* BASICDIRITEM_MDOS_H */
