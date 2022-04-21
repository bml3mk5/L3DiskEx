/// @file basicdiritem_mdos.h
///
/// @brief disk basic directory item for MDOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_MDOS_H_
#define _BASICDIRITEM_MDOS_H_

#include "basicdiritem.h"


/// ディレクトリ１アイテム MDOS
class DiskBasicDirItemMDOS : public DiskBasicDirItem
{
private:
	DiskBasicDirItemMDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemMDOS(const DiskBasicDirItemMDOS &src) : DiskBasicDirItem(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
//	/// @brief 属性１を返す
//	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
//	/// @brief 属性を変換
//	int		ConvFileType1(int file_type) const;
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

//	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
//	int	    ConvFileType1Pos(int t1) const;
//	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
//	int		CalcFileTypeFromPos(int pos) const;
//	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
//	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

//	/// @brief ファイル内部のアドレスを取り出す
//	void	TakeAddressesInFile();

public:
	DiskBasicDirItemMDOS(DiskBasic *basic);
	DiskBasicDirItemMDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemMDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

//	/// @brief ディレクトリアイテムのチェック
//	bool			Check(bool &last);

//	/// @brief ENDマークがあるか(一度も使用していないか)
//	bool			HasEndMark();
//	/// @brief 次のアイテムにENDマークを入れる
//	void			SetEndMark(DiskBasicDirItem *next_item);

//	/// @brief 属性を設定
//	void			SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString		GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	void			SetFileSize(int val);
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


	/// @brief ディレクトリアイテムのサイズ
	size_t			GetDataSize() const;

	/// @brief ファイルの終端コードをチェックする必要があるか
	bool			NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	int				RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ディレクトリをクリア ファイル新規作成時
	void			ClearData();

	/// @brief アイテムが実行アドレスを持っているか
	bool			HasExecuteAddress() const { return false; }

	/// @brief ファイル名から属性を決定する
	int				ConvFileTypeFromFileName(const wxString &filename) const;
	/// @brief ファイル名から属性を決定する
	int				ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
//	/// @brief ダイアログ内の属性部分のレイアウトを作成
//	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
//	/// @brief 属性を変更した際に呼ばれるコールバック
//	void	ChangeTypeInAttrDialog(IntNameBox *parent);
//	/// @brief 機種依存の属性を設定する
//	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	//@}
};

#endif /* _BASICDIRITEM_MDOS_H_ */
