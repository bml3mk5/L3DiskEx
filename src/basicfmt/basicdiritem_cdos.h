/// @file basicdiritem_cdos.h
///
/// @brief disk basic directory item for C-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_CDOS_H_
#define _BASICDIRITEM_CDOS_H_

#include "basicdiritem_mz_base.h"


//////////////////////////////////////////////////////////////////////

enum en_type_name_cdos {
	TYPE_NAME_CDOS_UNKNOWN = 0,
	TYPE_NAME_CDOS_OBJ,
	TYPE_NAME_CDOS_TEX,
	TYPE_NAME_CDOS_CMD,
	TYPE_NAME_CDOS_SYS,
	TYPE_NAME_CDOS_END
};
extern const char *gTypeNameCDOS2[];
enum en_type_name_cdos2 {
	TYPE_NAME_CDOS2_READ_ONLY = 0
};
/// @brief C-DOS 属性
enum en_file_type_cdos {
	FILETYPE_CDOS_OBJ = 1,
	FILETYPE_CDOS_TEX = 2,
	FILETYPE_CDOS_CMD = 3,
	FILETYPE_CDOS_SYS = 4,
};
enum en_data_type_mask_cdos {
	DATATYPE_CDOS_READ_ONLY = 0x01
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム C-DOS
class DiskBasicDirItemCDOS : public DiskBasicDirItemMZBase
{
private:
	DiskBasicDirItemCDOS() : DiskBasicDirItemMZBase() {}
	DiskBasicDirItemCDOS(const DiskBasicDirItemCDOS &src) : DiskBasicDirItemMZBase(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_cdos_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性３を返す
	virtual int		GetFileType3() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 属性３のセット
	virtual void	SetFileType3(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 属性を変換
	int				ConvToNativeType(int file_type) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				ConvFileType1Pos(int native_type) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int			    ConvFileType2Pos(int native_type) const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int				CalcFileTypeFromPos(int pos) const;
	/// @brief 属性1を得る
	int				GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief データ内にファイルサイズをセット
	void			SetFileSizeBase(int val);
	/// @brief データ内のファイルサイズを返す
	int				GetFileSizeBase() const;

public:
	DiskBasicDirItemCDOS(DiskBasic *basic);
	DiskBasicDirItemCDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemCDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

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

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief アイテムが日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	virtual bool	HasCreateDate() const { return true; }
	/// @brief アイテムの時間設定を無視することができるか
	virtual bool	CanIgnoreDateTime() const { return true; }
	/// @brief 日付を返す
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 日付をセット
	virtual void	SetFileCreateDate(const TM &tm);

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
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	virtual bool	PreImportDataFile(wxString &filename);

	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_CDOS_H_ */
