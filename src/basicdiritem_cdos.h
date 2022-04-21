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

	/// ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(size_t &size, size_t &len) const;
//	/// ファイル名を格納するバッファサイズを返す
//	int		GetFileNameSize(bool *invert = NULL) const;
//	/// 拡張子を格納するバッファサイズを返す
//	int		GetFileExtSize(bool *invert = NULL) const;
	/// 属性１を返す
	int		GetFileType1() const;
	/// 属性２を返す
	int		GetFileType2() const;
	/// 属性３を返す
	int		GetFileType3() const;
	/// 属性１のセット
	void	SetFileType1(int val);
	/// 属性２のセット
	void	SetFileType2(int val);
	/// 属性３のセット
	void	SetFileType3(int val);
	/// 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// 属性を変換
	int		ConvToNativeType(int file_type) const;
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int		ConvFileType1Pos(int native_type) const;
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int	    ConvFileType2Pos(int native_type) const;
	/// リストの位置から属性を返す(プロパティダイアログ用)
	int		CalcFileTypeFromPos(int pos);
	/// 属性1を得る
	int		GetFileType1InAttrDialog(const IntNameBox *parent) const;
//	/// 属性2を得る
//	int		GetFileType2InAttrDialog(const IntNameBox *parent) const;
	/// インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// データ内にファイルサイズをセット
	void	SetFileSizeBase(int val);
	/// データ内のファイルサイズを返す
	int		GetFileSizeBase() const;

public:
	DiskBasicDirItemCDOS(DiskBasic *basic);
	DiskBasicDirItemCDOS(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemCDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool	Check(bool &last);

//	/// 削除
//	bool	Delete(wxUint8 code);
//	/// ファイル名が一致するか
//	bool	IsSameFileName(const DiskBasicFileName &filename) const;
//	/// ファイルパスから内部ファイル名を生成する
//	wxString RemakeFileNameStr(const wxString &filepath) const;
	/// ファイル名に設定できない文字を文字列にして返す
	wxString GetDefaultInvalidateChars() const;

	/// 属性を設定
	void	SetFileAttr(const DiskBasicFileType &file_type);
	/// ディレクトリをクリア ファイル新規作成時
	void	ClearData();
//	/// ディレクトリを初期化 未使用にする
//	void	InitialData();
	/// 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// 属性の文字列を返す(ファイル一覧画面表示用)
	wxString GetFileAttrStr() const;

//	/// ファイルサイズをセット
//	void	SetFileSize(int val);
//	/// ファイルサイズとグループ数を計算する
//	void	CalcFileSize();

//	/// 指定ディレクトリのすべてのグループを取得
//	void	GetAllGroups(DiskBasicGroups &group_items);
	/// 最初のグループ番号をセット
	void	SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	wxUint32 GetStartGroup() const;
//	/// 追加のグループ番号を返す
//	wxUint32 GetExtraGroup() const;

	/// アイテムが日時を持っているか
	bool	HasDateTime() const { return true; }
	bool	HasDate() const { return true; }
	/// 日付を返す
	void	GetFileDate(struct tm *tm) const;
	/// 日付を返す
	wxString GetFileDateStr() const;
	/// 日付をセット
	void	SetFileDate(const struct tm *tm);

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

//	/// アイテムを削除できるか
//	bool	IsDeletable() const;
//	/// ファイル名を編集できるか
//	bool	IsFileNameEditable() const;
//	/// データをチェインする必要があるか（非連続データか）
//	bool	NeedChainInData();


	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
//	/// 属性を変更した際に呼ばれるコールバック
//	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
	/// ファイルサイズが適正か
	bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
//	/// ダイアログ入力後のファイル名チェック
//	bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	//@}
};

#endif /* _BASICDIRITEM_CDOS_H_ */
