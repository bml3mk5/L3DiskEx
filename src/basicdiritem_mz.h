/// @file basicdiritem_mz.h
///
/// @brief disk basic directory item for MZ DISK BASIC
///
#ifndef _BASICDIRITEM_MZ_H_
#define _BASICDIRITEM_MZ_H_

#include "basicdiritem.h"

/// MZ S-BASIC 属性名
extern const char *gTypeNameMZ[];
enum en_type_name_mz {
	TYPE_NAME_MZ_OBJ = 0,
	TYPE_NAME_MZ_BTX = 1,
	TYPE_NAME_MZ_BSD = 2,
	TYPE_NAME_MZ_BRD = 3,
	TYPE_NAME_MZ_DIR = 4,
	TYPE_NAME_MZ_VOL = 5,
	TYPE_NAME_MZ_VOL2 = 6,
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
	FILETYPE_MZ_VOL2 = 0x81,
};
enum en_data_type_mask_mz {
	DATATYPE_MZ_READ_ONLY = 0x01,
	DATATYPE_MZ_SEAMLESS = 0x80,
};
#define DATATYPE_MZ_SEAMLESS_MASK 0xff00000
#define DATATYPE_MZ_SEAMLESS_POS  20

/// ディレクトリ１アイテム MZ DISK BASIC
class DiskBasicDirItemMZ : public DiskBasicDirItem
{
private:
	DiskBasicDirItemMZ() : DiskBasicDirItem() {}
	DiskBasicDirItemMZ(const DiskBasicDirItemMZ &src) : DiskBasicDirItem(src) {}

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

public:
	DiskBasicDirItemMZ(DiskBasic *basic);
	DiskBasicDirItemMZ(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemMZ(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool			Check(bool &last);

	/// 削除
	bool			Delete(wxUint8 code);
	/// ファイル名が一致するか
	bool			IsSameFileName(const wxString &filename);
	/// ファイルパスから内部ファイル名を生成する
	wxString		RemakeFileNameStr(const wxString &filepath);
	/// ファイル名に設定できない文字を文字列にして返す
	wxString		InvalidateChars();
	/// ダイアログ入力後のファイル名チェック
	bool			ValidateFileName(const wxString &filename, wxString &errormsg);

	/// 属性を設定
	void			SetFileAttr(int file_type);
	/// ディレクトリをクリア ファイル新規作成時
	void			ClearData();
	/// ディレクトリを初期化 未使用にする
	void			InitialData();
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
	/// ファイルサイズが適正か
	bool			IsFileValidSize(int file_type1, int size, int *limit);

	/// 指定ディレクトリのすべてのグループを取得
	void			GetAllGroups(DiskBasicGroups &group_items);
	/// 最初のグループ番号をセット
	void			SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	wxUint32		GetStartGroup() const;
	/// 追加のグループ番号を返す
	wxUint32		GetExtraGroup() const;

	/// アイテムが日時を持っているか
	bool			HasDateTime() const { return true; }
	bool			HasDate() const { return true; }
	bool			HasTime() const { return true; }
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

	/// アイテムがアドレスを持っているか
	bool			HasAddress() const { return true; }
	/// 開始アドレスを返す
	int				GetStartAddress() const;
	/// 実行アドレスを返す
	int				GetExecuteAddress() const;
	/// 開始アドレスをセット
	void			SetStartAddress(int val);
	/// 実行アドレスをセット
	void			SetExecuteAddress(int val);

	/// ディレクトリアイテムのサイズ
	size_t			GetDataSize();

	/// 書き込み/上書き禁止か
	bool			IsWriteProtected();
	/// アイテムを削除できるか
	bool			IsDeleteable();
	/// ファイル名を編集できるか
	bool			IsFileNameEditable();
	/// データをチェインする必要があるか（非連続データか）
	bool			NeedChainInData();


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

#endif /* _BASICDIRITEM_MZ_H_ */
