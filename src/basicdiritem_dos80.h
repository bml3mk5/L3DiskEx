/// @file basicdiritem_dos80.h
///
/// @brief disk basic directory item for PC-8001 DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_DOS80_H_
#define _BASICDIRITEM_DOS80_H_

#include "basicdiritem_fat8.h"


/// PC-8001 DOS 属性名
extern const char *gTypeNameDOS80[];
enum en_type_name_dos80 {
	TYPE_NAME_DOS80_BASIC = 0,
	TYPE_NAME_DOS80_MACHINE,
	TYPE_NAME_DOS80_BASIC_MACHINE,
	TYPE_NAME_DOS80_END
};

/// ディレクトリ１アイテム PC-8001 DOS
class DiskBasicDirItemDOS80 : public DiskBasicDirItemFAT8
{
private:
	DiskBasicDirItemDOS80() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemDOS80(const DiskBasicDirItemDOS80 &src) : DiskBasicDirItemFAT8(src) {}

	directory_dos80_2_t *data2;
	bool data2_ownmake;

	DiskBasicGroups m_file_unit[2];
	int cached_type;

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);
	/// @brief 属性を変換
	int		ConvFileAttrFromTypePos(int t1) const;
	/// @brief 属性を変換
	int		ConvFileAttrToTypePos(int file_type) const;

	/// @brief ファイルサイズの計算
	void	CalcFileSize();

	/// @brief ファイル内部のアドレスを取り出す
	void	TakeAddressesInFile() {}

public:
	DiskBasicDirItemDOS80(DiskBasic *basic);
	DiskBasicDirItemDOS80(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemDOS80(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);
	~DiskBasicDirItemDOS80();

	/// @brief ディレクトリアイテムのチェック
	bool			Check(bool &last);

	/// @brief 属性を設定
	void			SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString		GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	void			SetFileSize(int val);

	/// @brief 指定ディレクトリのすべてのグループを取得
	void			GetAllGroups(DiskBasicGroups &group_items);
	/// @brief 最初のグループ番号をセット
	void			SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32		GetStartGroup(int fileunit_num) const;

	/// @brief アイテムがアドレスを持っているか
	bool			HasAddress() const;
	/// @brief アドレスを編集できるか
	bool			IsAddressEditable() const;
	/// @brief 開始アドレスを返す
	int				GetStartAddress() const;
	/// @brief 終了アドレスを返す
	int				GetEndAddress() const;
	/// @brief 実行アドレスを返す
	int				GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	void			SetStartAddress(int val);
	/// @brief 終了アドレスをセット
	void			SetEndAddress(int val);
	/// @brief 実行アドレスをセット
	void			SetExecuteAddress(int val);

	/// @brief ディレクトリアイテムのサイズ
	size_t			GetDataSize() const;
	/// @brief アイテムへのポインタを設定
	void			SetDataPtr(int n_num, int n_track, int n_side, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);

	/// @brief ファイルの終端コードをチェックする必要があるか
	bool			NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	int				RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ファイル番号のファイルサイズを得る
	int				GetFileUnitSize(int fileunit_num, wxInputStream &istream, int file_offset);
	/// @brief ファイル番号のファイルへアクセスできるか
	bool			IsValidFileUnit(int fileunit_num);

	/// @brief ファイル名、属性をコピー
	void			CopyItem(const DiskBasicDirItem &src);
	/// @brief ディレクトリをクリア ファイル新規作成時
	void			ClearData();

	/// @brief データをインポートする前に必要な処理
	bool			PreImportDataFile(wxString &filename);
//	/// @brief ファイル名から属性を決定する
//	int				ConvFileTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief ダイアログ内の値を設定
	void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
	/// @brief 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログの内容が反映された後の処理 
	void	ComittedAttrInAttrDialog(const IntNameBox *parent, bool status);
	/// @brief ダイアログの終了アドレスを編集できるか
	bool	IsEndAddressEditableInAttrDialog(IntNameBox *parent);
	//@}
};

#endif /* _BASICDIRITEM_DOS80_H_ */
