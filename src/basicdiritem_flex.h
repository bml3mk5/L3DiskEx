/// @file basicdiritem_flex.h
///
/// @brief disk basic directory item for FLEX
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FLEX_H_
#define _BASICDIRITEM_FLEX_H_

#include "basicdiritem.h"


// FLEX属性名
extern const char *gTypeNameFLEX[];
enum en_type_name_flex {
	TYPE_NAME_FLEX_READ_ONLY = 0,
	TYPE_NAME_FLEX_UNDELETE = 1,
	TYPE_NAME_FLEX_WRITE_ONLY = 2,
	TYPE_NAME_FLEX_HIDDEN = 3,
	TYPE_NAME_FLEX_RANDOM = 4,
};
enum en_file_type_mask_flex {
	FILETYPE_MASK_FLEX_READ_ONLY = 0x80,
	FILETYPE_MASK_FLEX_UNDELETE = 0x40,
	FILETYPE_MASK_FLEX_WRITE_ONLY = 0x20,
	FILETYPE_MASK_FLEX_HIDDEN = 0x10,
};
#define FILETYPE_FLEX_RANDOM_MASK	0xff00000
#define FILETYPE_FLEX_RANDOM_POS	20

/// ディレクトリ１アイテム FLEX
class DiskBasicDirItemFLEX : public DiskBasicDirItem
{
private:
	DiskBasicDirItemFLEX() : DiskBasicDirItem() {}
	DiskBasicDirItemFLEX(const DiskBasicDirItemFLEX &src) : DiskBasicDirItem(src) {}

	/// @brief ランダムアクセスファイルのインデックス(FSM)のグループ番号
	wxArrayInt m_random_group_nums;

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 属性２を返す
	int		GetFileType2() const;
	/// @brief 属性２のセット
	void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemFLEX(DiskBasic *basic);
	DiskBasicDirItemFLEX(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemFLEX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool			Check(bool &last);

//	/// @brief ファイル名に設定できない文字を文字列にして返す
//	wxString		GetDefaultInvalidateChars() const;
	/// @brief ダイアログ入力前のファイル名を変換 大文字にする
	void			ConvertToFileNameStr(wxString &filename) const;
	/// @brief ダイアログ入力後のファイル名文字列を変換 大文字にする
	void			ConvertFromFileNameStr(wxString &filename) const;
	/// @brief ファイル名は必須（空文字不可）か
	bool			IsFileNameRequired() const { return true; }

	/// @brief 削除
	bool			Delete(wxUint8 code);

	/// @brief 属性を設定
	void			SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString		GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	void			SetFileSize(int val);
	/// @brief ファイルサイズとグループ数を計算する
	void			CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	void			GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief 最初のグループ番号をセット
	void			SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32		GetStartGroup(int fileunit_num) const;
	/// @brief 最後のグループ番号をセット(機種依存)
	void			SetLastGroup(wxUint32 val);
	/// @brief 最後のグループ番号を返す(機種依存)
	wxUint32		GetLastGroup() const;
	/// @brief 追加のグループ番号を得る(機種依存)
	void			GetExtraGroups(wxArrayInt &arr) const;

	/// @brief アイテムが日時を持っているか
	bool			HasDateTime() const { return true; }
	bool			HasDate() const { return true; }
	bool			HasTime() const { return false; }
	/// @brief 日付を返す
	void			GetFileDate(struct tm *tm) const;
	/// @brief 時間を返す
	void			GetFileTime(struct tm *tm) const;
	/// @brief 日付を返す
	wxString		GetFileDateStr() const;
	/// @brief 時間を返す
	wxString		GetFileTimeStr() const;
	/// @brief 日付をセット
	void			SetFileDate(const struct tm *tm);
	/// @brief 時間をセット
	void			SetFileTime(const struct tm *tm);

	/// @brief ディレクトリアイテムのサイズ
	size_t			GetDataSize() const;

	/// @brief アイテムを削除できるか
	bool			IsDeletable() const;

	/// @brief 最初のトラック番号をセット
	void			SetStartTrack(wxUint8 val);
	/// @brief 最初のセクタ番号をセット
	void			SetStartSector(wxUint8 val);
	/// @brief 最初のトラック番号を返す
	wxUint8			GetStartTrack() const;
	/// @brief 最初のセクタ番号を返す
	wxUint8			GetStartSector() const;
	/// @brief 最後のトラック番号をセット
	void			SetLastTrack(wxUint8 val);
	/// @brief 最後のセクタ番号をセット
	void			SetLastSector(wxUint8 val);
	/// @brief 最後のトラック番号を返す
	wxUint8			GetLastTrack() const;
	/// @brief 最後のセクタ番号を返す
	wxUint8			GetLastSector() const;


	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログ入力後のファイル名チェック
	bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	//@}
};

#endif /* _BASICDIRITEM_FLEX_H_ */
