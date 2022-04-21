/// @file basicdiritem.h
///
/// @brief disk basic directory
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
class DiskBasicError;
class IntNameBox;

#define FILENAME_BUFSIZE	(32)
#define FILEEXT_BUFSIZE		(4)

/// ディレクトリ１アイテム (absolute)
class DiskBasicDirItem
{
protected:
	DiskBasic     *basic;
	DiskBasicType *type;

	int			num;				///< 通し番号
	DiskBasicFormatType format_type;	///< フォーマットタイプ
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
	/// ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// ファイル名を格納するバッファサイズを返す
	virtual int		GetFileNameSize(bool *invert = NULL) const { return 0; }
	/// 拡張子を格納するバッファサイズを返す
	virtual int		GetFileExtSize(bool *invert = NULL) const { return 0; }
	/// 属性１を返す
	virtual int		GetFileType1() const { return 0; }
	/// 属性２を返す
	virtual int		GetFileType2() const { return 0; }
	/// 属性３を返す
	virtual int		GetFileType3() const { return 0; }
	/// 属性１のセット
	virtual void	SetFileType1(int val) {}
	/// 属性２のセット
	virtual void	SetFileType2(int val) {}
	/// 属性３のセット
	virtual void	SetFileType3(int val) {}
	//@}

	/// @name ファイル名へのアクセス
	//@{
	/// ファイル名を設定
	virtual void	SetFileName(const wxUint8 *filename, int length);
	/// 拡張子を設定
	virtual void	SetFileExt(const wxUint8 *fileext, int length);
	/// ファイル名と拡張子を得る
	virtual void	GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;
	/// ファイル名(拡張子除く)を返す
	virtual wxString GetFileNamePlainStr() const;
	/// 拡張子を返す
	virtual wxString GetFileExtPlainStr() const;
	/// ファイル名を変換して内部ファイル名にする "."で拡張子と分別
	virtual void	ToNativeFileNameFromStr(const wxString &filename, wxUint8 *nativename, size_t length);
	/// ファイル名を変換して内部ファイル名にする 検索用
	virtual bool	ToNativeFileName(const wxString &filename, wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen);
	/// 文字列をコード変換して内部ファイル名にする
	bool			ToNativeName(const wxString &src, wxUint8 *dst, size_t len);
	/// 文字列をコード変換して内部ファイル名にする
	bool			ToNativeExt(const wxString &src, wxUint8 *dst, size_t len);
	/// 文字列をバッファにコピー あまりはfillでパディング
	static bool		MemoryCopy(const char *src, size_t flen, char fill, wxUint8 *dst, size_t len);
	/// 文字列をバッファにコピー "."で拡張子とを分ける
	static bool		MemoryCopy(const char *src, size_t flen, size_t elen, char fill, wxUint8 *dst, size_t len);
	//@}
public:
	/// ディレクトリアイテムを作成 DATAは内部で確保
	DiskBasicDirItem(DiskBasic *basic);
	/// ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
	DiskBasicDirItem(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	/// ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
	DiskBasicDirItem(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);
	virtual ~DiskBasicDirItem();

	/// @name 操作
	//@{
	/// このファイルへの書き込み/上書き禁止か
	virtual bool	IsWriteProtected() { return false; }
	/// 複製
	virtual void	Dup(const DiskBasicDirItem &src);
	/// ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// アイテムを削除できるか
	virtual bool	IsDeletable();
	/// 削除
	virtual bool	Delete(wxUint8 code);
	/// ENDマークがあるか(一度も使用していないか)
	virtual	bool	HasEndMark() { return false; }
	/// 次のアイテムにENDマークを入れる
	virtual void	SetEndMark(DiskBasicDirItem *next_item) {}
	/// 内部変数などを再設定
	virtual void	Refresh();
	/// アイテムをコピーできるか
	virtual bool	IsCopyable();
	/// アイテムを上書きできるか
	virtual bool	IsOverWritable();
	//@}

	/// @name ファイル名へのアクセス
	//@{
	/// ファイル名を編集できるか
	virtual bool	IsFileNameEditable() { return true; }
	/// ファイル名を設定 "."で拡張子と分離
	void			SetFileNameStr(const wxString &filename);
	/// ファイル名をそのまま設定
	void			SetFileNamePlain(const wxString &filename);
	/// 拡張子を設定
	void			SetFileExt(const wxString &fileext);
	/// ファイル名をコピー
	void			CopyFileName(const DiskBasicDirItem &src);
	/// ファイル名を返す 名前 + "." + 拡張子
	wxString		GetFileNameStr();
	/// ファイル名を得る 名前 + "." + 拡張子
	void			GetFileName(wxUint8 *filename, size_t length) const;
	/// ファイル名が一致するか
	virtual bool	IsSameFileName(const wxString &filename);
	/// ファイル名(拡張子除く)が一致するか
	virtual bool	IsSameName(const wxString &name);
	/// 同じファイル名か
	virtual bool	IsSameFileName(const DiskBasicDirItem &src);
	/// ファイル名＋拡張子のサイズ
	int				GetFileNameStrSize();
	/// ファイルパスから内部ファイル名を生成する インポート時などのダイアログを出す前
	virtual wxString RemakeFileNameStr(const wxString &filepath);
	/// 内部ファイル名からコード変換して文字列を返す コピー、このアプリからインポート時のダイアログを出す前
	virtual wxString RemakeFileName(const wxUint8 *src, size_t srclen);
	/// ダイアログ入力前のファイル名を変換 大文字にするなど
	virtual void	ConvertToFileNameStr(wxString &filename) {}
	/// ダイアログ入力後のファイル名文字列を変換 大文字にするなど
	virtual void	ConvertFromFileNameStr(wxString &filename) {}
	/// ファイル名に設定できない文字を文字列にして返す
	virtual wxString InvalidateChars();
	/// ファイル名は必須（空文字不可）か
	virtual bool	IsFileNameRequired() { return false; }
	/// ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxString &filename, wxString &errormsg) { return true; }
	//@}

	/// @name 属性へのアクセス
	//@{
	/// 属性を設定
	virtual void	SetFileAttr(int file_type);
	/// 属性を返す
	virtual int		GetFileAttr();
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	virtual int     GetFileType1Pos() { return 0; }
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	virtual int     GetFileType2Pos() { return 0; }
	/// リストの位置から属性を返す(プロパティダイアログ用)
	virtual int     CalcFileTypeFromPos(int pos1, int pos2) { return 0; }
	/// 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr();
	/// 外部属性を設定
	virtual void	SetExternalAttr(int val) { external_attr = val; }
	/// 外部属性を返す
	virtual int		GetExternalAttr() const { return external_attr; }
	/// 通常のファイルか
	virtual bool	IsNormalFile();
	/// ディレクトリか
	virtual bool	IsDirectory();
	//@}

	/// @name ファイルサイズ
	//@{
	/// ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// ファイルサイズを返す
	virtual int		GetFileSize();
	/// ファイルサイズとグループ数を計算する
	virtual void	CalcFileSize() {}
	/// 最終セクタの占有サイズをセット
	virtual void	SetDataSizeOnLastSecotr(int val) {}
	/// 最終セクタの占有サイズを返す
	virtual int		GetDataSizeOnLastSector() { return 0; }
	/// ファイルの終端コードをチェックして必要なサイズを返す
	virtual int		CheckEofCode(wxInputStream *istream, int file_size);
	/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size) { return file_size; }
	/// ファイルサイズが適正か
	virtual bool	IsFileValidSize(int file_type1, int size, int *limit);
	/// ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *dlg, int size, int *limit);
	//@}

	/// @name グループ番号/論理セクタ番号(LSN)へのアクセス
	//@{
	/// 指定ディレクトリのすべてのグループを取得
	virtual void	GetAllGroups(DiskBasicGroups &group_items) {}
	/// グループ数をセット
	void			SetGroupSize(int val);
	/// グループ数を返す
	int				GetGroupSize();
	/// 最初のグループ番号をセット
	virtual void	SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup() const;
	/// 追加のグループ番号をセット(機種依存)
	virtual void	SetExtraGroup(wxUint32 val);
	/// 追加のグループ番号を返す(機種依存)
	virtual wxUint32 GetExtraGroup() const;
	/// 最後のグループ番号をセット(機種依存)
	virtual void	SetLastGroup(wxUint32 val);
	/// 最後のグループ番号を返す(機種依存)
	virtual wxUint32 GetLastGroup() const;
	//@}

	/// @name 日付、時間
	//@{
	/// アイテムが日時を持っているか
	virtual bool	HasDateTime() const { return false; }
	virtual bool	HasDate() const { return false; }
	virtual bool	HasTime() const { return false; }
	/// 日付を得る
	virtual void	GetFileDate(struct tm *tm);
	/// 時間を得る
	virtual void	GetFileTime(struct tm *tm);
	/// 日時を得る
	virtual void	GetFileDateTime(struct tm *tm);
	/// 日付を返す
	virtual wxString GetFileDateStr();
	/// 時間を返す
	virtual wxString GetFileTimeStr();
	/// 日時を返す
	virtual wxString GetFileDateTimeStr();
	/// 日付をセット
	virtual void	SetFileDate(const struct tm *tm) {}
	/// 時間をセット
	virtual void	SetFileTime(const struct tm *tm) {}
	/// 日時をセット
	virtual void	SetFileDateTime(const struct tm *tm);
	/// 日付のタイトル名（ダイアログ用）
	virtual wxString GetFileDateTimeTitle();
	//@}

	/// @name 開始アドレス、実行アドレス
	//@{
	/// アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return false; }
	/// 開始アドレスを返す
	virtual int		GetStartAddress() const { return 0; }
	/// 実行アドレスを返す
	virtual int		GetExecuteAddress() const { return 0; }
	/// 開始アドレスをセット
	virtual void	SetStartAddress(int val) {}
	/// 実行アドレスをセット
	virtual void	SetExecuteAddress(int val) {}
	//@}

	/// @name リスト表示、使用しているか
	//@{
	/// 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse) { return false; }
	/// 使用中のアイテムか
	bool			IsUsed() const { return used; }
	/// 使用中かをセット
	void			Used(bool val) { used = val; }
	/// 表示するアイテムか
	bool			IsVisible() const { return visible; }
	/// 表示するかをセット
	void			Visible(bool val) { visible = val; }
	/// 使用中かつ表示するアイテムか
	bool			IsUsedAndVisible() const { return used && visible; }
	//@}

	/// @name ディレクトリデータへのアクセス
	//@{
	/// ファイル名、属性をコピー
	virtual void	CopyItem(const DiskBasicDirItem &src);
	/// ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize();
	/// アイテムを返す
	directory_t		*GetData() const { return data; }
	/// アイテムへのポインタを設定
	void			SetDataPtr(directory_t *val);
	/// アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// 内部メモリを確保してアイテムをコピー
	virtual void	CloneData(const directory_t *val);
	/// ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();
	/// ディレクトリを初期化 未使用にする
	virtual void	InitialData();
	//@}

	/// @name ファイルインポート・エクスポート
	//@{
	/// ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode() { return false; }
	/// データをチェインする必要があるか（非連続データか）
	virtual bool	NeedChainInData() { return false; }
	/// データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename) { return true; }
	//@}

	/// @name その他
	//@{
	/// アイテムを含むセクタを設定
	void			SetSector(DiskD88Sector	*val) { sector = val; }
	/// アイテムの属するセクタを変更済みにする
	virtual void	SetModify();
	/// DISK BASICを返す
	DiskBasic		*GetBasic() const { return basic; }
	//@}

	/// @name プロパティダイアログ用　機種依存部分を設定する
	//@{
	/// ダイアログ表示前にファイルの属性を設定
	virtual void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2) {}
	/// ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags) {}
	/// ダイアログ内の値を設定
	virtual void	InitializeForAttrDialog(IntNameBox *parent, int showitems, int *user_data) {}
	/// 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent) {}
	/// ファイル名に拡張子を付ける
	virtual wxString AddExtensionForAttrDialog(int file_type_1, const wxString &name) { return name; }
	/// 属性1を得る
	virtual int		GetFileType1InAttrDialog(const IntNameBox *parent) const { return 0; }
	/// 属性2を得る
	virtual int		GetFileType2InAttrDialog(const IntNameBox *parent) const { return 0; }
	/// 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
	//@}
};

WX_DEFINE_ARRAY(DiskBasicDirItem *, DiskBasicDirItems);

#endif /* _BASICDIRITEM_H_ */
