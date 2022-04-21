/// @file basicdiritem_tfdos.h
///
/// @brief disk basic directory item for TF-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_TFDOS_H_
#define _BASICDIRITEM_TFDOS_H_

#include "basicdiritem_mz_base.h"


//////////////////////////////////////////////////////////////////////

/// @brief TF-DOS 属性位置
enum en_type_name_tfdos {
	TYPE_NAME_TFDOS_UNKNOWN = 0,
	TYPE_NAME_TFDOS_OBJ,
	TYPE_NAME_TFDOS_TEX,
	TYPE_NAME_TFDOS_CMD,
	TYPE_NAME_TFDOS_SYS,
	TYPE_NAME_TFDOS_DAT,
	TYPE_NAME_TFDOS_GRA,
	TYPE_NAME_TFDOS_DBB,
	TYPE_NAME_TFDOS_READ_ONLY,
	TYPE_NAME_TFDOS_HIDDEN,
};
/// @brief TF-DOS 属性値
enum en_file_type_tfdos {
	FILETYPE_TFDOS_OBJ = 0x01,
	FILETYPE_TFDOS_TEX = 0x02,
	FILETYPE_TFDOS_CMD = 0x03,
	FILETYPE_TFDOS_SYS = 0x04,
	FILETYPE_TFDOS_DAT = 0x05,
	FILETYPE_TFDOS_GRA = 0x06,
	FILETYPE_TFDOS_DBB = 0x07,
};
enum en_data_type_mask_tfdos {
	DATATYPE_TFDOS_HIDDEN = 0x40,
	DATATYPE_TFDOS_READ_ONLY = 0x80,
};

//////////////////////////////////////////////////////////////////////

/** @class DiskBasicDirItemTFDOS

@brief ディレクトリ１アイテム TF-DOS

@li m_external_attr : 1:BASE互換  2:BASE互換かを自動判定

*/
class DiskBasicDirItemTFDOS : public DiskBasicDirItemMZBase
{
private:
	DiskBasicDirItemTFDOS() : DiskBasicDirItemMZBase() {}
	DiskBasicDirItemTFDOS(const DiskBasicDirItemTFDOS &src) : DiskBasicDirItemMZBase(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_tfdos_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int			    ConvFileType1Pos(int t1) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int			   ConvFileType2Pos(int t1) const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int				CalcFileTypeFromPos(int pos) const;
	/// @brief 属性1を得る
	int				GetFileType1InAttrDialog(const IntNameBox *parent) const;

	/// @brief データ内にファイルサイズをセット
	void			SetFileSizeBase(int val);
	/// @brief データ内のファイルサイズを返す
	int				GetFileSizeBase() const;

	/// @brief ダイアログでの表示フラグ
	int m_show_flags;

public:
	DiskBasicDirItemTFDOS(DiskBasic *basic);
	DiskBasicDirItemTFDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemTFDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

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
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_TFDOS_H_ */
