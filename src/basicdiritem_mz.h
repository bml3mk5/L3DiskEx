/// @file basicdiritem_mz.h
///
/// @brief disk basic directory item for MZ DISK BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_MZ_H_
#define _BASICDIRITEM_MZ_H_

#include "basicdiritem_mz_base.h"


/// MZ S-BASIC 属性名
extern const char *gTypeNameMZ[];
enum en_type_name_mz {
	TYPE_NAME_MZ_UNKNOWN = 0,
	TYPE_NAME_MZ_OBJ,
	TYPE_NAME_MZ_BTX,
	TYPE_NAME_MZ_BSD,
	TYPE_NAME_MZ_BRD,
	TYPE_NAME_MZ_DIR,
	TYPE_NAME_MZ_VOL,
	TYPE_NAME_MZ_VOLSWAP,
	TYPE_NAME_MZ_END,
};
extern const char *gTypeNameMZ2[];
enum en_type_name_mz2 {
	TYPE_NAME_MZ2_READ_ONLY = 0,
	TYPE_NAME_MZ2_SEAMLESS,
};
enum en_file_type_mz {
	FILETYPE_MZ_OBJ = 1,
	FILETYPE_MZ_BTX = 2,
	FILETYPE_MZ_BSD = 3,
	FILETYPE_MZ_BRD = 4,
	FILETYPE_MZ_DIR = 0xf,
	FILETYPE_MZ_VOL = 0x80,
	FILETYPE_MZ_VOLSWAP = 0x81,
};
enum en_data_type_mask_mz {
	DATATYPE_MZ_READ_ONLY = 0x01,
	DATATYPE_MZ_SEAMLESS = 0x80,
};
#define DATATYPE_MZ_SEAMLESS_MASK 0xff00000
#define DATATYPE_MZ_SEAMLESS_POS  20

/// ディレクトリ１アイテム MZ DISK BASIC
class DiskBasicDirItemMZ : public DiskBasicDirItemMZBase
{
private:
	DiskBasicDirItemMZ() : DiskBasicDirItemMZBase() {}
	DiskBasicDirItemMZ(const DiskBasicDirItemMZ &src) : DiskBasicDirItemMZBase(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性２を返す
	int		GetFileType2() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 属性２のセット
	void	SetFileType2(int val);
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
	/// @brief ファイルサイズとグループ数を計算する前処理
	void	PreCalcFileSize();
	/// @brief グループ取得計算前処理
	void	PreCalcAllGroups(int &calc_flags, wxUint32 &group_num, int &remain, int &sec_size, void **user_data);
	/// @brief グループ取得計算中処理
	void	CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data);
	/// @brief グループ取得計算後処理
	void	PostCalcAllGroups(void *user_data);

public:
	DiskBasicDirItemMZ(DiskBasic *basic);
	DiskBasicDirItemMZ(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemMZ(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// @brief ファイル名が一致するか
	bool	IsSameFileName(const DiskBasicFileName &filename, bool icase) const;
	/// @brief ファイル名が一致するか
	bool	IsSameFileName(const DiskBasicDirItem *src, bool icase) const;

	/// @brief 属性を設定
	void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief ディレクトリをクリア ファイル新規作成時
	void	ClearData();
	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	void	SetFileSize(int val);

	/// @brief 最初のグループ番号をセット
	void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 追加のグループ番号を返す
	wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る
	void	GetExtraGroups(wxArrayInt &arr) const;

	/// @brief アイテムが日時を持っているか
	bool	HasDateTime() const { return true; }
	bool	HasDate() const { return true; }
	bool	HasTime() const { return true; }
	/// @brief アイテムの時間設定を無視することができるか
	bool	CanIgnoreDateTime() const { return true; }
	/// @brief 日付を返す
	void	GetFileDate(struct tm *tm) const;
	/// @brief 時間を返す
	void	GetFileTime(struct tm *tm) const;
	/// @brief 日付を返す
	wxString GetFileDateStr() const;
	/// @brief 時間を返す
	wxString GetFileTimeStr() const;
	/// @brief 日付をセット
	void	SetFileDate(const struct tm *tm);
	/// @brief 時間をセット
	void	SetFileTime(const struct tm *tm);

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

	/// @brief データをチェインする必要があるか（非連続データか）
	bool	NeedChainInData();
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
	/// @brief ダイアログ内の値を設定
	void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
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

#endif /* _BASICDIRITEM_MZ_H_ */
