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
	TYPE_NAME_MZ_OBJ = 0,
	TYPE_NAME_MZ_BTX = 1,
	TYPE_NAME_MZ_BSD = 2,
	TYPE_NAME_MZ_BRD = 3,
	TYPE_NAME_MZ_DIR = 4,
	TYPE_NAME_MZ_VOL = 5,
	TYPE_NAME_MZ_VOLSWAP = 6,
	TYPE_NAME_MZ_READ_ONLY = 7,
	TYPE_NAME_MZ_SEAMLESS = 8,
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

	/// ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// ファイル名を格納するバッファサイズを返す
	int		GetFileNameSize(bool *invert = NULL) const;
	/// 拡張子を格納するバッファサイズを返す
	int		GetFileExtSize(bool *invert = NULL) const;
	/// 属性１を返す
	int		GetFileType1() const;
	/// 属性２を返す
	int		GetFileType2() const;
	/// 属性１のセット
	void	SetFileType1(int val);
	/// 属性２のセット
	void	SetFileType2(int val);
	/// 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType1Pos() const;
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType2Pos() const;
	/// リストの位置から属性を返す(プロパティダイアログ用)
	int		CalcFileTypeFromPos(int pos);
	/// 属性1を得る
	int		GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// データ内にファイルサイズをセット
	void	SetFileSizeBase(int val);
	/// データ内のファイルサイズを返す
	int		GetFileSizeBase() const;
	/// ファイルサイズとグループ数を計算する前処理
	void	PreCalcFileSize();
	/// グループ取得計算前処理
	void	PreCalcAllGroups(int &calc_flags, wxUint32 &group_num, int &remain, int &sec_size, void **user_data);
	/// グループ取得計算中処理
	void	CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data);
	/// グループ取得計算後処理
	void	PostCalcAllGroups(void *user_data);

public:
	DiskBasicDirItemMZ(DiskBasic *basic);
	DiskBasicDirItemMZ(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemMZ(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// ファイル名が一致するか
	bool	IsSameFileName(const DiskBasicFileName &filename) const;
	/// ファイルパスから内部ファイル名を生成する
	wxString RemakeFileNameStr(const wxString &filepath) const;
	/// ファイル名に設定できない文字を文字列にして返す
	wxString GetDefaultInvalidateChars() const;

	/// 属性を設定
	void	SetFileAttr(const DiskBasicFileType &file_type);
	/// ディレクトリをクリア ファイル新規作成時
	void	ClearData();
	/// 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// 属性の文字列を返す(ファイル一覧画面表示用)
	wxString GetFileAttrStr() const;

	/// ファイルサイズをセット
	void	SetFileSize(int val);

	/// 最初のグループ番号をセット
	void	SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	wxUint32 GetStartGroup() const;
	/// 追加のグループ番号を返す
	wxUint32 GetExtraGroup() const;

	/// アイテムが日時を持っているか
	bool	HasDateTime() const { return true; }
	bool	HasDate() const { return true; }
	bool	HasTime() const { return true; }
	/// アイテムの時間設定を無視することができるか
	bool	CanIgnoreDateTime() const { return true; }
	/// 日付を返す
	void	GetFileDate(struct tm *tm) const;
	/// 時間を返す
	void	GetFileTime(struct tm *tm) const;
	/// 日付を返す
	wxString GetFileDateStr() const;
	/// 時間を返す
	wxString GetFileTimeStr() const;
	/// 日付をセット
	void	SetFileDate(const struct tm *tm);
	/// 時間をセット
	void	SetFileTime(const struct tm *tm);

	/// アイテムがアドレスを持っているか
	bool	HasAddress() const { return true; }
	/// 開始アドレスを返す
	int		GetStartAddress() const;
	/// 実行アドレスを返す
	int		GetExecuteAddress() const;
	/// 開始アドレスをセット
	void	SetStartAddress(int val);
	/// 実行アドレスをセット
	void	SetExecuteAddress(int val);

	/// ディレクトリアイテムのサイズ
	size_t	GetDataSize() const;

	/// データをチェインする必要があるか（非連続データか）
	bool	NeedChainInData();


	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// ダイアログ内の値を設定
	void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
	/// 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
	/// ファイルサイズが適正か
	bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// ダイアログ入力後のファイル名チェック
	bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	//@}
};

#endif /* _BASICDIRITEM_MZ_H_ */
