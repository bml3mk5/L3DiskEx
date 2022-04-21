/// @file basicdiritem_tfdos.h
///
/// @brief disk basic directory item for TF-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_TFDOS_H_
#define _BASICDIRITEM_TFDOS_H_

#include "basicdiritem_mz_base.h"


/// TF-DOS 属性名
extern const char *gTypeNameTFDOS[];
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

/** @class DiskBasicDirItemTFDOS

@brief ディレクトリ１アイテム TF-DOS

@li m_external_attr : 1:BASE互換  2:BASE互換かを自動判定

*/
class DiskBasicDirItemTFDOS : public DiskBasicDirItemMZBase
{
private:
	DiskBasicDirItemTFDOS() : DiskBasicDirItemMZBase() {}
	DiskBasicDirItemTFDOS(const DiskBasicDirItemTFDOS &src) : DiskBasicDirItemMZBase(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    ConvFileType1Pos(int t1) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    ConvFileType2Pos(int t1) const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int		CalcFileTypeFromPos(int pos) const;
	/// @brief 属性1を得る
	int		GetFileType1InAttrDialog(const IntNameBox *parent) const;

	/// @brief データ内にファイルサイズをセット
	void	SetFileSizeBase(int val);
	/// @brief データ内のファイルサイズを返す
	int		GetFileSizeBase() const;

	/// @brief ダイアログでの表示フラグ
	int m_show_flags;

public:
	DiskBasicDirItemTFDOS(DiskBasic *basic);
	DiskBasicDirItemTFDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemTFDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// @brief ダイアログ入力前のファイル名を変換 大文字にするなど
	void	ConvertToFileNameStr(wxString &filename) const;
//	/// @brief ファイル名に設定できない文字を文字列にして返す
//	wxString GetDefaultInvalidateChars() const;

	/// @brief 属性を設定
	void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief ディレクトリをクリア ファイル新規作成時
	void	ClearData();
	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString GetFileAttrStr() const;

	/// @brief 最初のグループ番号をセット
	void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief アイテムがアドレスを持っているか
	bool	HasAddress() const { return true; }
	/// @brief 開始アドレスを返す
	int		GetStartAddress() const;
	/// @brief 実行アドレスを返す
	int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	void	SetStartAddress(int val);
	/// @brief 実行アドレスをセット
	void	SetExecuteAddress(int val);

	/// @brief ディレクトリアイテムのサイズ
	size_t	GetDataSize() const;

	/// @brief データをエクスポートする前に必要な処理
	bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	bool	PreImportDataFile(wxString &filename);

	/// @brief ファイル名から属性を決定する
	int		ConvOriginalTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief ダイアログ入力後のファイル名チェック
	bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	//@}
};

#endif /* _BASICDIRITEM_TFDOS_H_ */
