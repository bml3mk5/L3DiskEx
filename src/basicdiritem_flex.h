/// @file basicdiritem_flex.h
///
/// @brief disk basic directory item for FLEX
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
};
enum en_file_type_mask_flex {
	FILETYPE_MASK_FLEX_READ_ONLY = 0x80,
	FILETYPE_MASK_FLEX_UNDELETE = 0x40,
	FILETYPE_MASK_FLEX_WRITE_ONLY = 0x20,
	FILETYPE_MASK_FLEX_HIDDEN = 0x10,
};

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
	/// 使用しているアイテムか
	bool	CheckUsed(bool unuse);

public:
	DiskBasicDirItemFLEX(DiskBasic *basic);
	DiskBasicDirItemFLEX(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemFLEX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool			Check(bool &last);

	/// ファイル名に設定できない文字を文字列にして返す
	wxString		InvalidateChars();
	/// ダイアログ入力前のファイル名を変換 大文字にする
	void			ConvertToFileNameStr(wxString &filename);
	/// ダイアログ入力後のファイル名文字列を変換 大文字にする
	void			ConvertFromFileNameStr(wxString &filename);
	/// ファイル名は必須（空文字不可）か
	bool			IsFileNameRequired() { return true; }

	/// 削除
	bool			Delete(wxUint8 code);

	/// 属性を設定
	void			SetFileAttr(int file_type);
//	/// ディレクトリをクリア ファイル新規作成時
//	void			ClearData();
//	/// ディレクトリを初期化 未使用にする
//	void			InitialData();
	/// 属性を返す
	int				GetFileType();

	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int			    GetFileType1Pos();
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int			    GetFileType2Pos();
	/// リストの位置から属性を返す(プロパティダイアログ用)
	int				CalcFileTypeFromPos(int pos1, int pos2);
	/// 属性の文字列を返す(ファイル一覧画面表示用)
	wxString		GetFileAttrStr();

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

	/// アイテムが日時を持っているか
	bool			HasDateTime() const { return true; }
	bool			HasDate() const { return true; }
	bool			HasTime() const { return false; }
	/// 日付を返す
	void			GetFileDate(struct tm *tm);
	/// 時間を返す
	void			GetFileTime(struct tm *tm);
	/// 日付を返す
	wxString		GetFileDateStr();
	/// 時間を返す
	wxString		GetFileTimeStr();
	/// 日付をセット
	void			SetFileDate(const struct tm *tm);
	/// 時間をセット
	void			SetFileTime(const struct tm *tm);

	/// ディレクトリアイテムのサイズ
	size_t			GetDataSize();

	/// 書き込み/上書き禁止か
	bool			IsWriteProtected();
	/// アイテムを削除できるか
	bool			IsDeleteable();
	/// ファイル名を編集できるか
	bool			IsFileNameEditable();


	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int file_type_1, int file_type_2, wxBoxSizer *sizer, wxSizerFlags &flags, AttrControls &controls, int *user_data);
	/// 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(AttrControls &controls);
	/// インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(const wxString &name, int &file_type_1, int &file_type_2);
	/// 属性1を得る
	int		GetFileType1InAttrDialog(const AttrControls &controls) const;
	/// 属性2を得る
	int		GetFileType2InAttrDialog(const AttrControls &controls, const int *user_data) const;
	//@}
};

#endif /* _BASICDIRITEM_FLEX_H_ */
