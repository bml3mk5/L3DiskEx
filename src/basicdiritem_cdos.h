/// @file basicdiritem_cdos.h
///
/// @brief disk basic directory item for C-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_CDOS_H_
#define _BASICDIRITEM_CDOS_H_

#include "basicdiritem_mz_base.h"


/// C-DOS 属性名
extern const char *gTypeNameCDOS[];
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
enum en_file_type_cdos {
	FILETYPE_CDOS_OBJ = 1,
	FILETYPE_CDOS_TEX = 2,
	FILETYPE_CDOS_CMD = 3,
	FILETYPE_CDOS_SYS = 4,
};
enum en_data_type_mask_cdos {
	DATATYPE_CDOS_READ_ONLY = 0x01
};

/// ディレクトリ１アイテム C-DOS
class DiskBasicDirItemCDOS : public DiskBasicDirItemMZBase
{
private:
	DiskBasicDirItemCDOS() : DiskBasicDirItemMZBase() {}
	DiskBasicDirItemCDOS(const DiskBasicDirItemCDOS &src) : DiskBasicDirItemMZBase(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性２を返す
	int		GetFileType2() const;
	/// @brief 属性３を返す
	int		GetFileType3() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 属性２のセット
	void	SetFileType2(int val);
	/// @brief 属性３のセット
	void	SetFileType3(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief 属性を変換
	int		ConvToNativeType(int file_type) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int		ConvFileType1Pos(int native_type) const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    ConvFileType2Pos(int native_type) const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int		CalcFileTypeFromPos(int pos) const;
	/// @brief 属性1を得る
	int		GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief データ内にファイルサイズをセット
	void	SetFileSizeBase(int val);
	/// @brief データ内のファイルサイズを返す
	int		GetFileSizeBase() const;

public:
	DiskBasicDirItemCDOS(DiskBasic *basic);
	DiskBasicDirItemCDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemCDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

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

	/// @brief アイテムが日時を持っているか
	bool	HasDateTime() const { return true; }
	bool	HasDate() const { return true; }
	/// @brief アイテムの時間設定を無視することができるか
	bool	CanIgnoreDateTime() const { return true; }
	/// @brief 日付を返す
	void	GetFileDate(struct tm *tm) const;
	/// @brief 日付を返す
	wxString GetFileDateStr() const;
	/// @brief 日付をセット
	void	SetFileDate(const struct tm *tm);

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
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	//@}
};

#endif /* _BASICDIRITEM_CDOS_H_ */
