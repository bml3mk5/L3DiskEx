/// @file basicdiritem_cpm.h
///
/// @brief disk basic directory item for CP/M
///
#ifndef _BASICDIRITEM_CPM_H_
#define _BASICDIRITEM_CPM_H_

#include "basicdiritem.h"

#define SECTOR_UNIT_CPM		128

/// CP/M属性名
extern const char *gTypeNameCPM[];
enum en_type_name_cpm {
	TYPE_NAME_CPM_READ_ONLY = 0,
	TYPE_NAME_CPM_SYSTEM = 1,
	TYPE_NAME_CPM_ARCHIVE = 2,
};
#define FILETYPE_CPM_USERID_MASK 0x0f00000
#define FILETYPE_CPM_USERID_POS  20


class DiskBasicDirItemCPM;

/// ディレクトリ１アイテム CP/M
class DiskBasicDirItemCPM : public DiskBasicDirItem
{
private:
	DiskBasicDirItemCPM() : DiskBasicDirItem() {}
	DiskBasicDirItemCPM(const DiskBasicDirItemCPM &src) : DiskBasicDirItem(src) {}

	/// ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
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

	/// ファイル名を得る
	void	GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;
	/// 拡張子を返す
	wxString GetFileExtPlainStr() const;
	/// ファイル名を設定
	void	SetFileName(const wxUint8 *filename, int length);
	/// 拡張子を設定
	void	SetFileExt(const wxUint8 *fileext, int length);

	int group_width;	///< グループ番号の幅(1 = 8ビット, 2 = 16ビット)
	int group_entries;	///< グループ番号のエントリ数(8 or 16)

	DiskBasicDirItemCPM *next_item;	///< 次のエクステントがある場合

	/// 拡張子からバイナリかどうかを判断する
	int		GetFileTypeByExt(int val, const wxString &ext) const;

public:
	DiskBasicDirItemCPM(DiskBasic *basic);
	DiskBasicDirItemCPM(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemCPM(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool			Check(bool &last);

	/// 削除
	bool			Delete(wxUint8 code);

	/// ファイル名に設定できない文字を文字列にして返す
	wxString		InvalidateChars();
	/// ダイアログ入力前のファイル名を変換 大文字にする
	void			ConvertToFileNameStr(wxString &filename);
	/// ダイアログ入力後のファイル名文字列を変換 大文字にする
	void			ConvertFromFileNameStr(wxString &filename);
	/// ファイル名は必須（空文字不可）か
	bool			IsFileNameRequired() { return true; }

	/// 属性を設定
	void			SetFileAttr(int file_type);
	/// 属性を返す
	int				GetFileAttr();

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

	/// ディレクトリアイテムのサイズ
	size_t			GetDataSize();

	/// ファイルの終端コードをチェックする必要があるか
	bool			NeedCheckEofCode();
	/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	int				RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// アイテムを削除できるか
	bool			IsDeletable();
	/// ファイル名を編集できるか
	bool			IsFileNameEditable();

	
	/// グループ番号の幅をセット
	void		SetGroupWidth(int val) { group_width = val; }
	/// グループ番号の幅を返す(1 = 8ビット, 2 = 16ビット)
	int			GetGroupWidth() const { return group_width; }
	/// グループ番号のエントリ数を返す
	int			GetGroupEntries() const { return group_entries; }
	/// グループ番号をセット
	void		SetGroup(int pos, wxUint32 val);
	/// グループ番号を返す
	wxUint32	GetGroup(int pos) const;
	/// エクステント番号を返す
	wxUint8		GetExtentNumber() const;
	/// レコード番号を返す
	wxUint8		GetRecordNumber() const;
	/// ファイルサイズからエクステント番号とレコード番号をセット
	void		CalcExtentAndRecordNumber(int val);
	/// 次のアイテムをセット
	void		SetNextItem(DiskBasicDirItem *val) { next_item = (DiskBasicDirItemCPM *)val; }
	/// 次のアイテムを返す
	DiskBasicDirItemCPM *GetNextItem() { return next_item; }
	/// アイテムソート用
	static int  Compare(DiskBasicDirItem **item1, DiskBasicDirItem **item2); 
	/// 名前比較
	static int  CompareName(DiskBasicDirItem **item1, DiskBasicDirItem **item2); 


	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);
	/// 属性1を得る
	int		GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// 属性2を得る
	int		GetFileType2InAttrDialog(const IntNameBox *parent) const;
	//@}
};

#endif /* _BASICDIRITEM_CPM_H_ */
