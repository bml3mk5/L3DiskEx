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
class DiskBasicFileName;
class DiskBasicError;
class IntNameBox;

class DiskBasicDirItem;
class DiskBasicDirItems;
class DiskBasicDirItemAttr;

//////////////////////////////////////////////////////////////////////

/// ディレクトリ１アイテム (abstract)
class DiskBasicDirItem
{
public:
	enum en_flag_masks {
		USED_ITEM	 = 0x0001,	///< bit0:使用しているか
		VISIBLE_LIST = 0x0002,	///< bit1:リストに表示するか
		VISIBLE_TREE = 0x0004,	///< bit2:ツリーに表示するか
	};

protected:
	DiskBasic     *basic;
	DiskBasicType *type;

	DiskBasicDirItem  *m_parent;	///< 親ディレクトリ
	DiskBasicDirItems *m_children;	///< 子ディレクトリ
	bool		m_valid_dir;		///< 上記ディレクトリツリーが確定しているか

	int			m_num;				///< 通し番号
	int			m_position;			///< セクタ内の位置（バイト）
	int			m_flags;			///< フラグ bit0:使用しているか bit1:リストに表示するか bit2:ツリーに表示するか
	DiskBasicGroups m_groups;		///< 占有グループ
	directory_t	*m_data;			///< セクタ内のデータへのポインタ
	bool		m_ownmake_data;		///< データのメモリは自身で確保したか
	DiskD88Sector *m_sector;		///< ディレクトリのあるセクタ
	int			m_external_attr;	///< ディレクトリエントリ内に持たない属性を保持する(機種依存)

	DiskBasicDirItem();
	DiskBasicDirItem(const DiskBasicDirItem &src);
	DiskBasicDirItem &operator=(const DiskBasicDirItem &src);

#ifdef COPYABLE_DIRITEM
	/// @brief 複製
	virtual void	Dup(const DiskBasicDirItem &src);
#endif

	/// @name 機種依存パラメータへのアクセス(protected)
	//@{
	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
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

	/// @name ファイル名へのアクセス(protected)
	//@{
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief 拡張子を設定
	virtual void	SetNativeExt(wxUint8 *fileext, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
	/// @brief 拡張子を得る
	virtual void	GetNativeExt(wxUint8 *fileext, size_t size, size_t &length) const;
	/// @brief ファイル名(拡張子除く)を返す
	virtual wxString GetFileNamePlainStr() const;
	/// @brief 拡張子を返す
	virtual wxString GetFileExtPlainStr() const;
	/// @brief ファイル名を変換して内部ファイル名にする 検索用
	bool			ToNativeFileName(const wxString &filename, wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;
	/// @brief ファイルパスから内部ファイル名を生成する インポート時などのダイアログを出す前
	wxString		RemakeFileNameAndExtStr(const wxString &filepath) const;
	/// @brief ファイルパスから内部ファイル名を生成する インポート時などのダイアログを出す前
	wxString		RemakeFileNameOnlyStr(const wxString &filepath) const;
	/// @brief 属性から拡張子を付加する
	void			AddExtensionByFileAttr(int file_type, int mask, wxString &filename, bool dupli = false) const;
	/// @brief 属性から拡張子を付加する
	void			AddExtensionByFileAttr(int file_type, int mask, wxString &filename, int external, bool dupli = false) const;
	/// @brief 属性の文字列を返す
	bool			GetFileAttrName(int pos1, const char *list[], int unknown_pos, wxString &attr) const;
	/// @brief 拡張子から属性を決定できるか 決定できる場合、末尾の拡張子をとり除く
	bool			TrimExtensionByExtensionAttr(wxString &filename) const;
	/// @brief 拡張子から属性を決定できるか 決定できる場合、拡張子が２つつながっている場合は末尾の拡張子をとり除く
	bool			TrimLastExtensionByExtensionAttr(wxString &filename) const;
	/// @brief 拡張子から属性を決定できるか 決定できる場合、末尾の拡張子をとり除く
	bool			IsContainAttrByExtension(const wxString &filename, const char *list[], int list_first, int list_last, wxString *outfile, int *attr) const;
	//@}

public:
	/// @brief ディレクトリアイテムを作成 DATAは内部で確保
	DiskBasicDirItem(DiskBasic *basic);
	/// @brief ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
	DiskBasicDirItem(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	/// @brief ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
	DiskBasicDirItem(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);
	virtual ~DiskBasicDirItem();

	/// @name ディレクトリツリー
	//@{
	/// @brief 子ディレクトリを追加
	void			AddChild(DiskBasicDirItem *newitem);
	/// @brief 親ディレクトリを返す
	DiskBasicDirItem *GetParent() { return m_parent; }
	/// @brief 親ディレクトリを設定
	void			SetParent(DiskBasicDirItem *newitem) { m_parent = newitem; }
	/// @brief 子ディレクトリ一覧を返す
	DiskBasicDirItems *GetChildren() { return m_children; }
	/// @brief 子ディレクトリ一覧を返す
	const DiskBasicDirItems *GetChildren() const { return m_children; }
	/// @brief 子ディレクトリ一覧をクリア
	void			EmptyChildren();
	/// @brief ディレクトリツリーが確定しているか
	bool			IsValidDirectory() const { return m_valid_dir; }
	/// @brief ディレクトリツリーが確定しているか設定
	void			ValidDirectory(bool val) { m_valid_dir = val; }
	//@}

	/// @name 操作
	//@{
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
	/// @brief アイテムをロード・エクスポートできるか
	virtual bool	IsLoadable() const;
	/// @brief アイテムをコピー(内部でDnD)できるか
	virtual bool	IsCopyable() const;
	/// @brief アイテムを上書きできるか
	virtual bool	IsOverWritable() const;
	//@}

	/// @name ファイル名へのアクセス
	//@{
	/// @brief ファイル名の通常コードの割合(0.0-1.0)
	double			NormalCodesInFileName() const;
	/// @brief ファイル名を編集できるか
	virtual bool	IsFileNameEditable() const { return true; }
	/// @brief ファイル名を設定 "."で拡張子と分離
	void			SetFileNameStr(const wxString &filename);
	/// @brief ファイル名をそのまま設定
	void			SetFileNamePlain(const wxString &filename);
	/// @brief 拡張子を設定
	void			SetFileExtPlain(const wxString &fileext);
	/// @brief ファイル名と拡張子を設定
	virtual void	SetNativeFileName(wxUint8 *name, size_t nsize, size_t nlen, wxUint8 *ext, size_t esize, size_t elen);
	/// @brief ファイル名をコピー
	void			CopyFileName(const DiskBasicDirItem &src);
	/// @brief ファイル名を返す 名前 + "." + 拡張子
	wxString		GetFileNameStr() const;
	/// @brief ファイル名を得る 名前 + "." + 拡張子
	void			GetFileName(wxUint8 *filename, size_t length) const;
	/// @brief ファイル名と拡張子を得る
	virtual void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;
	/// @brief ファイル名(拡張子除く)が一致するか
	bool			IsSameName(const wxString &name, bool icase) const;
	/// @brief ファイル名が一致するか
	virtual bool	IsSameFileName(const DiskBasicFileName &filename, bool icase) const;
	/// @brief 同じファイル名か
	virtual bool	IsSameFileName(const DiskBasicDirItem *src, bool icase) const;
	/// @brief ファイル名＋拡張子のサイズ
	int				GetFileNameStrSize() const;
	/// @brief ダイアログ入力前のファイル名を変換 大文字にするなど
	virtual void	ConvertFileNameBeforeImportDialog(wxString &filename) const {}
//	/// @brief ダイアログ入力後のファイル名文字列を変換 大文字にするなど
//	virtual void	ConvertFileNameAfterRenamed(wxString &filename) const {}
	/// @brief ファイル名に付随する拡張属性を設定
	virtual void	SetOptionalName(int val) {}
	/// @brief ファイル名に付随する拡張属性を返す
	/// @see IsSameFileName()
	virtual int		GetOptionalName() const { return 0; }
	/// @brief 文字列をバッファにコピー あまりはfillでパディング
	static void		MemoryCopy(const wxUint8 *src, size_t ssize, size_t slen, wxUint8 *dst, size_t dsize, size_t &dlen);
	/// @brief 文字列をバッファにコピー "."で拡張子とを分ける
	static void		SplitFileName(const wxUint8 *src, size_t ssize, size_t slen, wxUint8 *dname, size_t dnsize, size_t &dnlen, wxUint8 *dext, size_t desize, size_t &delen);
	//@}

	/// @name 属性へのアクセス
	//@{
	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を設定
	virtual void	SetFileAttr(DiskBasicFormatType format_type, int file_type, int original_type = 0);
	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;
	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;
	/// @brief 外部属性を設定
	virtual void	SetExternalAttr(int val) { m_external_attr = val; }
	/// @brief 外部属性を返す
	virtual int		GetExternalAttr() const { return m_external_attr; }
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
	virtual void	CalcFileUnitSize(int fileunit_num) {}
	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileSize();
	/// @brief ファイルの終端コードをチェックして必要なサイズを返す
	virtual int		CheckEofCode(wxInputStream *istream, int file_size);
	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int		RecalcFileSize(DiskBasicGroups &group_items, int occupied_size) { return occupied_size; }
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size) { return file_size; }
	//@}

	/// @name グループ番号/論理セクタ番号(LSN)へのアクセス
	//@{
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items) {}
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetAllGroups(DiskBasicGroups &group_items);
	/// @brief グループ数をセット
	void			SetGroupSize(int val);
	/// @brief グループ数を返す
	int				GetGroupSize() const;
	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 追加のグループ番号をセット(機種依存)
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す(機種依存)
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る(機種依存)
	virtual void	GetExtraGroups(wxArrayInt &arr) const;
	/// @brief 最後のグループ番号をセット(機種依存)
	virtual void	SetLastGroup(wxUint32 val);
	/// @brief 最後のグループ番号を返す(機種依存)
	virtual wxUint32 GetLastGroup() const;
	/// @brief グループリストの数を返す
	size_t			GetGroupCount() const;
	/// @brief グループリストを返す
	const DiskBasicGroups &GetGroups() const;
	/// @brief グループリストを設定
	void SetGroups(const DiskBasicGroups &vals);
	/// @brief グループリストのアイテムを返す
	DiskBasicGroupItem *GetGroup(size_t idx) const;
	/// @brief チェイン用のセクタをセット(機種依存)
	virtual void	SetChainSector(DiskD88Sector *sector, wxUint8 *data, const DiskBasicDirItem *pitem = NULL) {}
	/// @brief チェイン用のセクタにグループ番号をセット(機種依存)
	virtual void	AddChainGroupNumber(int idx, wxUint32 val) {}
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
	/// @brief 日時を返す
	virtual struct tm GetFileDateTime() const;
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
	/// @brief アイテムが実行アドレスを持っているか
	virtual bool	HasExecuteAddress() const { return HasAddress(); }
	/// @brief アドレスを編集できるか
	virtual bool	IsAddressEditable() const { return HasAddress(); }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const { return -1; }
	/// @brief 終了アドレスを返す
	virtual int		GetEndAddress() const { return -1; }
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const { return -1; }
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val) {}
	/// @brief 終了アドレスをセット
	virtual void	SetEndAddress(int val) {}
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val) {}
	//@}

	/// @name リスト表示、使用しているか
	//@{
	/// @brief 使用しているアイテムかどうかチェック
	virtual bool	CheckUsed(bool unuse) { return false; }
	/// @brief 使用中のアイテムか
	bool			IsUsed() const;
	/// @brief 使用中かをセット
	void			Used(bool val);
	/// @brief リストに表示するアイテムか
	bool			IsVisible() const;
	/// @brief リストに表示するかをセット
	void			Visible(bool val);
	/// @brief 使用中かつリストに表示するアイテムか
	bool			IsUsedAndVisible() const;
	/// @brief ツリーに表示するアイテムか
	bool			IsVisibleOnTree() const;
	/// @brief ツリーに表示するかをセット
	void			VisibleOnTree(bool val);
	//@}

	/// @name ディレクトリデータへのアクセス
	//@{
	/// @brief ファイル名、属性をコピー
	virtual void	CopyItem(const DiskBasicDirItem &src);
	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	directory_t		*GetData() const { return m_data; }
	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, int n_track, int n_side, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();
	/// @brief ディレクトリを初期化 未使用にする
	virtual void	InitialData();
	//@}

	/// @name ファイルインポート・エクスポート
	//@{
	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode() { return false; }
	/// @brief ファイルの終端コードを返す
	virtual wxUint8	GetEofCode() const;
	/// @brief データをチェインする必要があるか（非連続データか）
	virtual bool	NeedChainInData() { return false; }
	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief データをインポートする前に必要な処理
	virtual bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル番号のファイルサイズを得る
	virtual int		GetFileUnitSize(int fileunit_num, wxInputStream &istream, int file_offset);
	/// @brief ファイル番号のファイルへアクセスできるか
	virtual bool	IsValidFileUnit(int fileunit_num);
	/// @brief ファイル名から属性を決定する
	virtual int		ConvFileTypeFromFileName(const wxString &filename) const;
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief ファイル名から拡張属性を決定する
	virtual int		ConvOptionalNameFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	virtual bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief その他の属性値を設定する
	virtual void	SetAttr(DiskBasicDirItemAttr &attr);
	//@}

	/// @name 文字コード
	//@{
	/// 文字列をバイト列に変換 文字コードは機種依存
	virtual int		ConvStringToChars(const wxString &src, wxUint8 *dst, size_t len) const;
	/// バイト列を文字列に変換 文字コードは機種依存
	virtual void	ConvCharsToString(const wxUint8 *src, size_t len, wxString &dst) const;
	//@}

	/// @name 属性ファイル
	//@{
	/// ファイル属性をXMLで出力
	bool WriteFileAttrToXml(const wxString &path);
	/// ファイル属性をXMLから読み込む
	bool ReadFileAttrFromXml(const wxString &path, struct tm *tm);
	//@}

	/// @name その他
	//@{
	/// @brief アイテムを含むセクタを設定
	void			SetSector(DiskD88Sector	*val) { m_sector = val; }
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
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const { return true; }
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit) { return true; }
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg) { return true; }
	/// @brief ファイル名に付随する拡張属性をセットする
	/// @see GetOptionalName()
	virtual int		GetOptionalNameInAttrDialog(const IntNameBox *parent) { return 0; }
	/// @brief ダイアログの内容が反映された後の処理 
	virtual void	ComittedAttrInAttrDialog(const IntNameBox *parent, bool status) {}
	/// @brief ダイアログ用に終了アドレスを返す
	virtual int		GetEndAddressInAttrDialog(IntNameBox *parent) { return GetEndAddress(); }
	/// @brief ダイアログの終了アドレスを編集できるか
	virtual bool	IsEndAddressEditableInAttrDialog(IntNameBox *parent) { return IsAddressEditable(); }
	//@}
};

WX_DEFINE_ARRAY(DiskBasicDirItem *, DiskBasicDirItems);

//////////////////////////////////////////////////////////////////////

/// 属性値を一時的に集めておくクラス
class DiskBasicDirItemAttr
{
public:
	enum {
		MAX_DATE_TIME = 4
	};
private:
	bool m_rename;
	DiskBasicFileName m_name;

	bool m_ignore_type;
	DiskBasicFileType m_type;

	int m_start_addr;
	int m_end_addr;
	int m_exec_addr;

	bool m_ignore_date_time;
	TM m_date_time[MAX_DATE_TIME];

	int m_user_data;

public:
	DiskBasicDirItemAttr();
	~DiskBasicDirItemAttr();

	void Renameable(bool val) { m_rename = val; }
	void SetFileName(const DiskBasicFileName &val) { m_name = val; }
	void SetFileName(const wxString &n_name, int n_optional);
	void IgnoreFileAttr(bool val) { m_ignore_type = val; }
	void SetFileAttr(const DiskBasicFileType &val) { m_type = val; }
	void SetFileAttr(DiskBasicFormatType n_format, int n_type, int n_origin = 0);
	void SetFileType(int n_type);
	void SetFileOriginAttr(int n_origin);
	void SetStartAddress(int val) { m_start_addr = val; }
	void SetEndAddress(int val) { m_end_addr = val; }
	void SetExecuteAddress(int val) { m_exec_addr = val; }
	void IgnoreDateTime(bool val) { m_ignore_date_time = val; }
	void SetDateTime(int idx, const struct tm *tm) { m_date_time[idx].Set(tm); }
	void SetUserData(int val) { m_user_data = val; }

	bool IsRenameable() const { return m_rename; }
	DiskBasicFileName &GetFileName() { return m_name; }
	bool DoesIgnoreFileAttr() const { return m_ignore_type; }
	DiskBasicFileType &GetFileAttr() { return m_type; }
	int  GetFileType() const;
	int  GetFileOriginAttr() const;
	int  GetStartAddress() const { return m_start_addr; }
	int  GetEndAddress() const { return m_end_addr; }
	int  GetExecuteAddress() const { return m_exec_addr; }
	bool DoesIgnoreDateTime() const { return m_ignore_date_time; }
	struct tm *GetDateTime(int idx) { return m_date_time[idx].Get(); }
	int  GetUserData() const { return m_user_data; }
};

#endif /* _BASICDIRITEM_H_ */
