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
	TYPE_NAME_TFDOS_OBJ = 0,
	TYPE_NAME_TFDOS_TXT,
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
	FILETYPE_TFDOS_TXT = 0x02,
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

/// ディレクトリ１アイテム TF-DOS
class DiskBasicDirItemTFDOS : public DiskBasicDirItemMZBase
{
private:
	DiskBasicDirItemTFDOS() : DiskBasicDirItemMZBase() {}
	DiskBasicDirItemTFDOS(const DiskBasicDirItemTFDOS &src) : DiskBasicDirItemMZBase(src) {}

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// @brief ファイル名を格納するバッファサイズを返す
	int		GetFileNameSize(bool *invert = NULL) const;
	/// @brief 拡張子を格納するバッファサイズを返す
	int		GetFileExtSize(bool *invert = NULL) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType1Pos() const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType2Pos() const;
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	int		CalcFileTypeFromPos(int pos);
	/// @brief 属性1を得る
	int		GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// データ内にファイルサイズをセット
	void	SetFileSizeBase(int val);
	/// データ内のファイルサイズを返す
	int		GetFileSizeBase() const;

public:
	DiskBasicDirItemTFDOS(DiskBasic *basic);
	DiskBasicDirItemTFDOS(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemTFDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// @brief ファイルパスから内部ファイル名を生成する
	wxString RemakeFileNameStr(const wxString &filepath) const;
	/// @brief ダイアログ入力前のファイル名を変換 大文字にするなど
	void	ConvertToFileNameStr(wxString &filename) const;
	/// @brief ファイル名に設定できない文字を文字列にして返す
	wxString InvalidateChars() const;

	/// @brief 属性を設定
	void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief ディレクトリをクリア ファイル新規作成時
	void	ClearData();
	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString GetFileAttrStr() const;

	/// @brief 最初のグループ番号をセット
	void	SetStartGroup(wxUint32 val);
	/// @brief 最初のグループ番号を返す
	wxUint32 GetStartGroup() const;

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

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
	/// @brief ファイルサイズが適正か
	bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief ダイアログ入力後のファイル名チェック
	bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	//@}
};

#endif /* _BASICDIRITEM_TFDOS_H_ */
