/// @file basicdiritem_dos80.h
///
/// @brief disk basic directory item for PC-8001 DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_DOS80_H_
#define _BASICDIRITEM_DOS80_H_

#include "basicdiritem_fat8.h"


//////////////////////////////////////////////////////////////////////

extern const char *gTypeNameDOS80[];
/// @brief PC-8001 DOS 属性
enum en_type_name_dos80 {
	TYPE_NAME_DOS80_BASIC = 0,
	TYPE_NAME_DOS80_MACHINE,
	TYPE_NAME_DOS80_BASIC_MACHINE,
	TYPE_NAME_DOS80_END
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム PC-8001 DOS
///
/// @li DefaultStartAddress   デフォルトロードアドレス
/// @li DefaultExecuteAddress デフォルト実行開始アドレス
class DiskBasicDirItemDOS80 : public DiskBasicDirItemFAT8
{
private:
	DiskBasicDirItemDOS80() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemDOS80(const DiskBasicDirItemDOS80 &src) : DiskBasicDirItemFAT8(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_dos80_t> m_data;
	DiskBasicDirData<directory_dos80_2_t> m_data2;
//	bool m_data2_ownmake;

	DiskBasicGroups m_file_unit[2];
	int m_cached_type;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);
	/// @brief 属性を変換
	int				ConvFileAttrFromTypePos(int t1) const;
	/// @brief 属性を変換
	int				ConvFileAttrToTypePos(int file_type) const;

	/// @brief ファイルサイズの計算
	virtual void	CalcFileSize();

	/// @brief ファイル内部のアドレスを取り出す
	void			TakeAddressesInFile() {}

public:
	DiskBasicDirItemDOS80(DiskBasic *basic);
	DiskBasicDirItemDOS80(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemDOS80(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);
	~DiskBasicDirItemDOS80();

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief 削除
	virtual bool	Delete();

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);

	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetAllGroups(DiskBasicGroups &group_items);
	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const;
	/// @brief アドレスを編集できるか
	virtual bool	IsAddressEditable() const;
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 終了アドレスを返す
	virtual int		GetEndAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);
	/// @brief 終了アドレスをセット
	virtual void	SetEndAddress(int val);
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val);

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief ファイル番号のファイルサイズを得る
	int				GetFileUnitSize(int fileunit_num, wxInputStream &istream, int file_offset);
	/// @brief ファイル番号のファイルへアクセスできるか
	bool			IsValidFileUnit(int fileunit_num);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief ファイル名、属性をコピー
	virtual void	CopyItem(const DiskBasicDirItem &src);

	/// @brief データをインポートする前に必要な処理
	virtual bool	PreImportDataFile(wxString &filename);

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief ダイアログ内の値を設定
	virtual void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログの内容が反映された後の処理 
	void	ComittedAttrInAttrDialog(const IntNameBox *parent, bool status);
	/// @brief ダイアログの終了アドレスを編集できるか
	virtual bool	IsEndAddressEditableInAttrDialog(IntNameBox *parent);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_DOS80_H_ */
