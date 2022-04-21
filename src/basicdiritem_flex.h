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

	/// ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// ファイル名を格納するバッファサイズを返す
	int		GetFileNameSize(bool *invert = NULL) const;
	/// 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// 拡張子を格納するバッファサイズを返す
	int		GetFileExtSize(bool *invert = NULL) const;
	/// 属性１を返す
	int		GetFileType1() const;
	/// 属性１のセット
	void	SetFileType1(int val);
	/// 属性２を返す
	int		GetFileType2() const;
	/// 属性２のセット
	void	SetFileType2(int val);
	/// 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemFLEX(DiskBasic *basic);
	DiskBasicDirItemFLEX(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemFLEX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool			Check(bool &last);

	/// ファイル名に設定できない文字を文字列にして返す
	wxString		InvalidateChars() const;
	/// ダイアログ入力前のファイル名を変換 大文字にする
	void			ConvertToFileNameStr(wxString &filename) const;
	/// ダイアログ入力後のファイル名文字列を変換 大文字にする
	void			ConvertFromFileNameStr(wxString &filename) const;
	/// ファイル名は必須（空文字不可）か
	bool			IsFileNameRequired() const { return true; }

	/// 削除
	bool			Delete(wxUint8 code);

	/// 属性を設定
	void			SetFileAttr(const DiskBasicFileType &file_type);
	/// 属性を返す
	DiskBasicFileType GetFileAttr() const;

//	/// リストの位置から属性を返す(プロパティダイアログ用)
//	int				CalcFileTypeFromPos(int pos1, int pos2);
	/// 属性の文字列を返す(ファイル一覧画面表示用)
	wxString		GetFileAttrStr() const;

	/// ファイルサイズをセット
	void			SetFileSize(int val);
	/// ファイルサイズとグループ数を計算する
	void			CalcFileSize();
	/// 指定ディレクトリのすべてのグループを取得
	void			GetAllGroups(DiskBasicGroups &group_items);

	/// 最初のグループ番号をセット
	void			SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	wxUint32		GetStartGroup() const;
	/// 最後のグループ番号をセット(機種依存)
	void			SetLastGroup(wxUint32 val);
	/// 最後のグループ番号を返す(機種依存)
	wxUint32		GetLastGroup() const;

	/// アイテムが日時を持っているか
	bool			HasDateTime() const { return true; }
	bool			HasDate() const { return true; }
	bool			HasTime() const { return false; }
	/// 日付を返す
	void			GetFileDate(struct tm *tm) const;
	/// 時間を返す
	void			GetFileTime(struct tm *tm) const;
	/// 日付を返す
	wxString		GetFileDateStr() const;
	/// 時間を返す
	wxString		GetFileTimeStr() const;
	/// 日付をセット
	void			SetFileDate(const struct tm *tm);
	/// 時間をセット
	void			SetFileTime(const struct tm *tm);

	/// ディレクトリアイテムのサイズ
	size_t			GetDataSize() const;

	/// アイテムを削除できるか
	bool			IsDeletable() const;
//	/// ファイル名を編集できるか
//	bool			IsFileNameEditable() const;


	/// 最初のトラック番号をセット
	void			SetStartTrack(wxUint8 val);
	/// 最初のセクタ番号をセット
	void			SetStartSector(wxUint8 val);
	/// 最初のトラック番号を返す
	wxUint8			GetStartTrack() const;
	/// 最初のセクタ番号を返す
	wxUint8			GetStartSector() const;
	/// 最後のトラック番号をセット
	void			SetLastTrack(wxUint8 val);
	/// 最後のセクタ番号をセット
	void			SetLastSector(wxUint8 val);
	/// 最後のトラック番号を返す
	wxUint8			GetLastTrack() const;
	/// 最後のセクタ番号を返す
	wxUint8			GetLastSector() const;


	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
	//@}
};

#endif /* _BASICDIRITEM_FLEX_H_ */
