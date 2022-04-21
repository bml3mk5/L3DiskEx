/// @file basicdiritem.h
///
/// @brief disk basic directory item
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_H_
#define _BASICDIRITEM_H_

#include "common.h"
#include "basiccommon.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "diskd88.h"


class wxWindow;
class wxControl;
class wxBoxSizer;
class wxSizerFlags;
class DiskBasic;
class DiskBasicType;
class DiskBasicGroups;
class DiskBasicFileName;
class DiskBasicError;
class IntNameBox;

#define FILENAME_BUFSIZE	(32)
#define FILEEXT_BUFSIZE		(4)

class DiskBasicDirItem;
class DiskBasicDirItems;

/// ディレクトリ１アイテム (abstract)
class DiskBasicDirItem
{
protected:
	DiskBasic     *basic;
	DiskBasicType *type;

	DiskBasicDirItem  *parent;		///< 親ディレクトリ
	DiskBasicDirItems *children;	///< 子ディレクトリ
	bool		valid_dir;			///< 上記ディレクトリツリーが確定しているか

	int			num;				///< 通し番号
	int			position;			///< セクタ内の位置（バイト）
	bool		used;				///< 使用しているか
	bool		visible;			///< リストに表示するか
	int			file_size;			///< ファイルサイズ
	int			groups;				///< 使用グループ数
	directory_t	*data;				///< セクタ内のデータへのポインタ
	bool		ownmake_data;		///< データのメモリは自身で確保したか
	DiskD88Sector *sector;			///< ディレクトリのあるセクタ
	int			external_attr;		///< ディレクトリエントリ内に持たない属性を保持する(機種依存)

	DiskBasicDirItem();
	DiskBasicDirItem(const DiskBasicDirItem &src);
	DiskBasicDirItem &operator=(const DiskBasicDirItem &src);

	/// @name 機種依存パラメータへのアクセス
	//@{
	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief ファイル名を格納するバッファサイズを返す
	virtual int		GetFileNameSize(bool *invert = NULL) const { return 0; }
	/// @brief 拡張子を格納するバッファサイズを返す
	virtual int		GetFileExtSize(bool *invert = NULL) const { return 0; }
	/// @brief 属性１を返す
	virtual int		GetFileType1() const { return 0; }
	/// @brief 属性２を返す
	virtual int		GetFileType2() const { return 0; }
	/// @brief 属性３を返す
	virtual int		GetFileType3() const { return 0; }
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val) = 0;
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val) {}
	/// @brief 属性３のセット
	virtual void	SetFileType3(int val) {}
	//@}

	/// @name ファイル名へのアクセス
	//@{
	/// @brief ファイル名を設定
	virtual void	SetFileName(const wxUint8 *filename, int length);
	/// @brief 拡張子を設定
	virtual void	SetFileExt(const wxUint8 *fileext, int length);
	/// @brief ファイル名と拡張子を得る
	virtual void	GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;
	/// @brief ファイル名(拡張子除く)を返す
	virtual wxString GetFileNamePlainStr() const;
	/// @brief 拡張子を返す
	virtual wxString GetFileExtPlainStr() const;
	/// @brief ファイル名を変換して内部ファイル名にする "."で拡張子と分別
	void			ToNativeFileNameFromStr(const wxString &filename, wxUint8 *nativename, size_t length) const;
	/// @brief ファイル名を変換して内部ファイル名にする 検索用
	bool			ToNativeFileName(const wxString &filename, wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;
	/// @brief 文字列をコード変換して内部ファイル名にする
	bool			ToNativeName(const wxString &src, wxUint8 *dst, size_t len) const;
	/// @brief 文字列をコード変換して内部ファイル名にする
	bool			ToNativeExt(const wxString &src, wxUint8 *dst, size_t len) const;
	//@}

public:
	/// @brief ディレクトリアイテムを作成 DATAは内部で確保
	DiskBasicDirItem(DiskBasic *basic);
	/// @brief ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
	DiskBasicDirItem(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	/// @brief ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
	DiskBasicDirItem(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);
	virtual ~DiskBasicDirItem();

	/// @name ディレクトリツリー
	//@{
	/// @brief 子ディレクトリを追加
	void			AddChild(DiskBasicDirItem *newitem);
	/// @brief 親ディレクトリを返す
	DiskBasicDirItem *GetParent() { return parent; }
	/// @brief 親ディレクトリを設定
	void			SetParent(DiskBasicDirItem *newitem) { parent = newitem; }
	/// @brief 子ディレクトリ一覧を返す
	DiskBasicDirItems *GetChildren() { return children; }
	/// @brief 子ディレクトリ一覧を返す
	const DiskBasicDirItems *GetChildren() const { return children; }
	/// @brief 子ディレクトリ一覧をクリア
	void			EmptyChildren();
	/// @brief ディレクトリツリーが確定しているか
	bool			IsValidDirectory() const { return valid_dir; }
	/// @brief ディレクトリツリーが確定しているか設定
	void			ValidDirectory(bool val) { valid_dir = val; }

	//@}

	/// @name 操作
	//@{
	/// @brief 複製
	virtual void	Dup(const DiskBasicDirItem &src);
	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief 削除
	virtual bool	Delete(wxUint8 code);
	/// @brief ENDマークがあるか(一度も使用していないか)
	virtual	bool	HasEndMark() { return false; }
	/// @brief 次のアイテムにENDマークを入れる
	virtual void	SetEndMark(DiskBasicDirItem *next_item) {}
	/// @brief 内部変数などを再設定
	virtual void	Refresh();
	/// @brief アイテムをコピーできるか
	virtual bool	IsCopyable() const;
	/// @brief アイテムを上書きできるか
	virtual bool	IsOverWritable() const;
	//@}

	/// @name ファイル名へのアクセス
	//@{
	/// @brief ファイル名を編集できるか
	virtual bool	IsFileNameEditable() const { return true; }
	/// @brief ファイル名を設定 "."で拡張子と分離
	void			SetFileNameStr(const wxString &filename);
	/// @brief ファイル名をそのまま設定
	void			SetFileNamePlain(const wxString &filename);
	/// @brief 拡張子を設定
	void			SetFileExt(const wxString &fileext);
	/// @brief ファイル名をコピー
	void			CopyFileName(const DiskBasicDirItem &src);
	/// @brief ファイル名を返す 名前 + "." + 拡張子
	wxString		GetFileNameStr() const;
	/// @brief ファイル名を得る 名前 + "." + 拡張子
	void			GetFileName(wxUint8 *filename, size_t length) const;
	/// @brief ファイル名(拡張子除く)が一致するか
	bool			IsSameName(const wxString &name) const;
	/// @brief ファイル名が一致するか
	virtual bool	IsSameFileName(const DiskBasicFileName &filename) const;
	/// @brief 同じファイル名か
	virtual bool	IsSameFileName(const DiskBasicDirItem &src) const;
	/// @brief ファイル名＋拡張子のサイズ
	int				GetFileNameStrSize() const;
	/// @brief ファイルパスから内部ファイル名を生成する インポート時などのダイアログを出す前
	virtual wxString RemakeFileNameStr(const wxString &filepath) const;
	/// @brief 内部ファイル名からコード変換して文字列を返す コピー、このアプリからインポート時のダイアログを出す前
	virtual wxString RemakeFileName(const wxUint8 *src, size_t srclen) const;
	/// @brief ダイアログ入力前のファイル名を変換 大文字にするなど
	virtual void	ConvertToFileNameStr(wxString &filename) const {}
	/// @brief ダイアログ入力後のファイル名文字列を変換 大文字にするなど
	virtual void	ConvertFromFileNameStr(wxString &filename) const {}
	/// @brief ファイル名に設定できない文字を文字列にして返す
	virtual wxString GetDefaultInvalidateChars() const;
	/// @brief ファイル名は必須（空文字不可）か
	virtual bool	IsFileNameRequired() const { return false; }
	/// @brief ファイル名に付随する拡張属性を返す
	/// @see IsSameFileName()
	virtual int		GetOptionalName() const { return 0; }
	/// @brief 文字列をバッファにコピー あまりはfillでパディング
	static bool		MemoryCopy(const char *src, size_t flen, char fill, wxUint8 *dst, size_t len);
	/// @brief 文字列をバッファにコピー "."で拡張子とを分ける
	static bool		MemoryCopy(const char *src, size_t flen, size_t elen, char fill, wxUint8 *dst, size_t len);
	//@}

	/// @name 属性へのアクセス
	//@{
	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を設定
	void			SetFileAttr(int file_type, int original_type = 0);
	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;
	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;
	/// @brief 外部属性を設定
	virtual void	SetExternalAttr(int val) { external_attr = val; }
	/// @brief 外部属性を返す
	virtual int		GetExternalAttr() const { return external_attr; }
	/// @brief 通常のファイルか
	bool			IsNormalFile() const;
	/// @brief ディレクトリか
	bool			IsDirectory() const;
	//@}

	/// @name ファイルサイズ
	//@{
	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// @brief ファイルサイズを返す
	virtual int		GetFileSize() const;
	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileSize() {}
	/// @brief ファイルの終端コードをチェックして必要なサイズを返す
	virtual int		CheckEofCode(wxInputStream *istream, int file_size);
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size) { return file_size; }
	//@}

	/// @name グループ番号/論理セクタ番号(LSN)へのアクセス
	//@{
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetAllGroups(DiskBasicGroups &group_items) {}
	/// @brief グループ数をセット
	void			SetGroupSize(int val);
	/// @brief グループ数を返す
	int				GetGroupSize() const;
	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(wxUint32 val);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup() const;
	/// @brief 追加のグループ番号をセット(機種依存)
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す(機種依存)
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 最後のグループ番号をセット(機種依存)
	virtual void	SetLastGroup(wxUint32 val);
	/// @brief 最後のグループ番号を返す(機種依存)
	virtual wxUint32 GetLastGroup() const;
	//@}

	/// @name 日付、時間
	//@{
	/// @brief アイテムが日時を持っているか
	virtual bool	HasDateTime() const { return false; }
	/// @brief アイテムが日付を持っているか
	virtual bool	HasDate() const { return false; }
	/// @brief アイテムが時間を持っているか
	virtual bool	HasTime() const { return false; }
	/// @brief アイテムの時間設定を無視することができるか
	virtual bool	CanIgnoreDateTime() const { return false; }
	/// @brief 日付を得る
	virtual void	GetFileDate(struct tm *tm) const;
	/// @brief 時間を得る
	virtual void	GetFileTime(struct tm *tm) const;
	/// @brief 日時を得る
	virtual void	GetFileDateTime(struct tm *tm) const;
	/// @brief 日付を返す
	virtual wxString GetFileDateStr() const;
	/// @brief 時間を返す
	virtual wxString GetFileTimeStr() const;
	/// @brief 日時を返す
	virtual wxString GetFileDateTimeStr() const;
	/// @brief 日付をセット
	virtual void	SetFileDate(const struct tm *tm) {}
	/// @brief 時間をセット
	virtual void	SetFileTime(const struct tm *tm) {}
	/// @brief 日時をセット
	virtual void	SetFileDateTime(const struct tm *tm);
	/// @brief 日付のタイトル名（ダイアログ用）
	virtual wxString GetFileDateTimeTitle() const;
	//@}

	/// @name 開始アドレス、実行アドレス
	//@{
	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return false; }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const { return 0; }
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const { return 0; }
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val) {}
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val) {}
	//@}

	/// @name リスト表示、使用しているか
	//@{
	/// @brief 使用しているアイテムかどうかチェック
	virtual bool	CheckUsed(bool unuse) { return false; }
	/// @brief 使用中のアイテムか
	bool			IsUsed() const { return used; }
	/// @brief 使用中かをセット
	void			Used(bool val) { used = val; }
	/// @brief 表示するアイテムか
	bool			IsVisible() const { return visible; }
	/// @brief 表示するかをセット
	void			Visible(bool val) { visible = val; }
	/// @brief 使用中かつ表示するアイテムか
	bool			IsUsedAndVisible() const { return used && visible; }
	//@}

	/// @name ディレクトリデータへのアクセス
	//@{
	/// @brief ファイル名、属性をコピー
	virtual void	CopyItem(const DiskBasicDirItem &src);
	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	directory_t		*GetData() const { return data; }
	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, int n_track, int n_side, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief 内部メモリを確保してアイテムをコピー
	virtual void	CloneData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();
	/// @brief ディレクトリを初期化 未使用にする
	virtual void	InitialData();
	//@}

	/// @name ファイルインポート・エクスポート
	//@{
	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode() { return false; }
	/// @brief データをチェインする必要があるか（非連続データか）
	virtual bool	NeedChainInData() { return false; }
	/// @brief データをエクスポートする前に必要な処理
	/// @param [in,out] filename ファイル名
	/// @return false このファイルは対象外とする
	/// @see PreDropDataFile()
	virtual bool	PreExportDataFile(wxString &filename) { return true; }
	/// @brief データをDnDで外部へエクスポートする前に必要な処理
	/// @param [in,out] filename ファイル名
	/// @return false このファイルは対象外とする
	/// @see PreExportDataFile()
	virtual bool	PreDropDataFile(wxString &filename) { return true; }
	//@}

	/// @name その他
	//@{
	/// @brief アイテムを含むセクタを設定
	void			SetSector(DiskD88Sector	*val) { sector = val; }
	/// @brief アイテムの属するセクタを変更済みにする
	virtual void	SetModify();
	/// @brief DISK BASICを返す
	DiskBasic		*GetBasic() const { return basic; }
	//@}

	/// @name プロパティダイアログ用　機種依存部分を設定する
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags) {}
	/// @brief ダイアログ内の値を設定
	virtual void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data) {}
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent) {}
	/// @brief ファイル名に拡張子を付ける
	virtual wxString AddExtensionForAttrDialog(int file_type_1, const wxString &name) { return name; }
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo) { return true; }
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit) { return true; }
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg) { return true; }
	/// @brief ファイル名に付随する拡張属性をセットする
	/// @see GetOptionalName()
	virtual int		GetOptionalNameInAttrDialog(const IntNameBox *parent) { return 0; }
	//@}
};

WX_DEFINE_ARRAY(DiskBasicDirItem *, DiskBasicDirItems);

#endif /* _BASICDIRITEM_H_ */
