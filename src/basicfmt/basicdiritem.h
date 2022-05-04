/// @file basicdiritem.h
///
/// @brief disk basic directory item
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_H_
#define _BASICDIRITEM_H_

#include "../common.h"
#include "basiccommon.h"
#include <wx/string.h>
#include <wx/dynarray.h>
#include "../diskd88.h"


class wxWindow;
class wxControl;
class wxBoxSizer;
class wxSizerFlags;
//class wxChoice;
class wxInputStream;
class DiskBasic;
class DiskBasicType;
class DiskBasicFileName;
class DiskBasicError;
class IntNameBox;

class DiskBasicDirItem;
class DiskBasicDirItems;
class DiskBasicDirItemAttr;


//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリデータ
template <class TYPE>
class DiskBasicDirData
{
private:
	TYPE *m_data;
	bool  m_self;	///< 自分でメモリを確保した時true
public:
	DiskBasicDirData();
	~DiskBasicDirData();
	/// @brief 削除
	void Delete();
	/// @brief メモリ確保
	void Alloc();
	/// @brief ポインタ割り当て
	void Attach(void *data);
	/// @brief コピー
	bool Copy(const void *data, size_t len = 0xfffffff, bool invert = false, size_t start = 0);
	/// @brief 埋める
	bool Fill(int ch, size_t len = 0xfffffff, bool invert = false, size_t start = 0);
	/// @brief 反転
	void Invert(size_t len = 0xfffffff, size_t start = 0);
	/// @brief データを返す
	TYPE *Data() const { return m_data; }
	/// @brief データが有効か
	bool IsValid() const { return (m_data != NULL); }
	/// @brief 自分でメモリを確保したか
	bool IsSelf() const { return m_self; }
};

template <class TYPE>
DiskBasicDirData<TYPE>::DiskBasicDirData()
{
	m_data = NULL;
	m_self = false;
}

template <class TYPE>
DiskBasicDirData<TYPE>::~DiskBasicDirData()
{
	Delete();
}

/// 削除 確保したメモリは開放する
template <class TYPE>
void DiskBasicDirData<TYPE>::Delete()
{
	if (m_self) delete m_data;
	m_data = NULL;
	m_self = false;
}

/// メモリ確保
/// @note メモリの初期化は行わない
template <class TYPE>
void DiskBasicDirData<TYPE>::Alloc()
{
	Delete();
	m_data = new TYPE;
	m_self = true;
}

/// ポインタ割り当て 確保していたメモリは開放する
/// @param[in] data ポインタ
template <class TYPE>
void DiskBasicDirData<TYPE>::Attach(void *data)
{
	Delete();
	m_data = (TYPE *)data;
}

/// データをコピーする
/// @param[in] data   元データ
/// @param[in] len    データサイズ
/// @param[in] invert 反転するか
/// @param[in] start  コピー開始位置
template <class TYPE>
bool DiskBasicDirData<TYPE>::Copy(const void *data, size_t len, bool invert, size_t start)
{
	if (!m_data) return false;
	wxUint8 *src = (wxUint8 *)data;
	wxUint8 *dst = (wxUint8 *)m_data;
	if (len > sizeof(TYPE)) len = sizeof(TYPE);
	memcpy(dst + start, src + start, len - start);
	if (invert) Invert(len, start);
	return true;
}

/// 指定したコードでデータを埋める
/// @param[in] ch     埋める文字
/// @param[in] len    データサイズ
/// @param[in] invert 反転するか
/// @param[in] start  コピー開始位置
template <class TYPE>
bool DiskBasicDirData<TYPE>::Fill(int ch, size_t len, bool invert, size_t start)
{
	if (!m_data) return false;
	wxUint8 *dst = (wxUint8 *)m_data;
	if (len > sizeof(TYPE)) len = sizeof(TYPE);
	memset(dst + start, ch, len - start);
	if (invert) Invert(len, start);
	return true;
}

/// データのビットを反転する
/// @param[in] len    データサイズ
/// @param[in] start  コピー開始位置
template <class TYPE>
void DiskBasicDirData<TYPE>::Invert(size_t len, size_t start)
{
	if (len > sizeof(TYPE)) len = sizeof(TYPE);
	mem_invert(m_data + start, len - start);
}

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム (abstract)
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
	bool			GetFileAttrName(int pos, const name_value_t *list, wxString &attr, int default_pos = -1) const;
	/// @brief 拡張子から属性を決定できるか 決定できる場合、末尾の拡張子をとり除く
	bool			TrimExtensionByExtensionAttr(wxString &filename) const;
	/// @brief 拡張子から属性を決定できるか 決定できる場合、拡張子が２つつながっている場合は末尾の拡張子をとり除く
	bool			TrimLastExtensionByExtensionAttr(wxString &filename) const;
	/// @brief 拡張子から属性を決定できるか 決定できる場合、拡張子が２つつながっている場合は末尾の拡張子をとり除く
	bool			TrimLastExtensionByExtensionAttr(const wxString &filename, const name_value_t *list, int list_first, int list_last, wxString *outfile, int *attr, int *pos) const;
	/// @brief 拡張子から属性を決定できるか 決定できる場合、末尾の拡張子をとり除く
	bool			IsContainAttrByExtension(const wxString &filename, const name_value_t *list, int list_first, int list_last, wxString *outfile, int *attr, int *pos) const;
	//@}

	/// @name プロパティダイアログ用(protected)
	//@{
	/// @brief 属性の選択肢を作成する（プロパティダイアログ用）
	static void		CreateChoiceForAttrDialog(DiskBasic *basic, const name_value_t *list, int end_pos, wxArrayString &types, int file_type = -1);
	/// @brief 属性の選択肢を選ぶ（プロパティダイアログ用）
	static int		SelectChoiceForAttrDialog(DiskBasic *basic, int file_type, int end_pos, int unknown_pos);
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	static int		CalcSpecialOriginalTypeFromPos(DiskBasic *basic, int pos, int end_pos);
	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
	static int		CalcSpecialFileTypeFromPos(DiskBasic *basic, int pos, int end_pos);
	//@}

public:
	/// @brief ディレクトリアイテムを作成 DATAは内部で確保
	DiskBasicDirItem(DiskBasic *basic);
	/// @brief ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
	DiskBasicDirItem(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	/// @brief ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
	DiskBasicDirItem(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);
	virtual ~DiskBasicDirItem();

	/// @name 初期設定
	//@{
	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);
	//@}

	/// @name ディレクトリツリー
	//@{
	/// @brief 子ディレクトリを作成
	void			CreateChildren();
	/// @brief 子ディレクトリを追加
	void			AddChild(DiskBasicDirItem *newitem);
	/// @brief 親ディレクトリを返す
	DiskBasicDirItem *GetParent() const { return m_parent; }
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
	/// @brief ディレクトリアイテムのチェック（要実装）
	virtual bool	Check(bool &last) = 0;
	/// @brief ディレクトリアイテムのチェック
	static bool		CheckData(const wxUint8 *buf, size_t len, bool &last);
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief 削除（要実装）
	virtual bool	Delete() = 0;
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
	/// @brief ファイル名を返す 名前 + "." + 拡張子 エクスポート時
	wxString		GetFileNameStrForExport() const;
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
	/// @brief 小文字を大文字にする
	virtual void	ToUpper(wxUint8 *str, size_t size) const;
	/// @brief ファイル名＋拡張子のサイズ
	virtual int		GetFileNameStrSize() const;
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
	/// @brief 文字列をバッファにコピー spchr(通常".")で拡張子とを分ける
	static void		SplitFileName(const wxUint8 *src, size_t ssize, size_t slen, wxUint8 *dname, size_t dnsize, size_t &dnlen, wxUint8 *dext, size_t desize, size_t &delen, wxUint8 spchr);
	//@}

	/// @name 属性へのアクセス
	//@{
	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を設定
	virtual void	SetFileAttr(DiskBasicFormatType format_type, int file_type, int original_type0 = 0, int original_type1 = 0, int original_type2 = 0);
	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;
	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;
	/// @brief 外部属性を設定
	void			SetExternalAttr(int val) { m_external_attr = val; }
	/// @brief 外部属性を返す
	int				GetExternalAttr() const { return m_external_attr; }
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
	/// @brief 追加のグループリストをセット(機種依存)
	virtual void	SetExtraGroups(const DiskBasicGroups &grps);
	/// @brief 追加のグループ番号を得る(機種依存)
	/// @note  使用状況の表示で使用する
	virtual void	GetExtraGroups(wxArrayInt &arr) const;
	/// @brief 追加のグループリストを返す(機種依存)
	virtual void	GetExtraGroups(DiskBasicGroups &grps) const;
	/// @brief 次のグループ番号をセット(機種依存)
	virtual void	SetNextGroup(wxUint32 val);
	/// @brief 次のグループ番号を返す(機種依存)
	virtual wxUint32 GetNextGroup() const;
	/// @brief 最後のグループ番号をセット(機種依存)
	virtual void	SetLastGroup(wxUint32 val);
	/// @brief 最後のグループ番号を返す(機種依存)
	virtual wxUint32 GetLastGroup() const;
	/// @brief 親のグループ番号をセット(機種依存)
	virtual void	SetParentGroup(wxUint32 val);
	/// @brief 親のグループ番号を返す(機種依存)
	virtual wxUint32 GetParentGroup() const;
	/// @brief グループリストの数を返す
	size_t			GetGroupCount() const;
	/// @brief グループリストを返す
	const DiskBasicGroups &GetGroups() const;
	/// @brief グループリストを設定
	void SetGroups(const DiskBasicGroups &vals);
	/// @brief グループリストのアイテムを返す
	DiskBasicGroupItem *GetGroup(size_t idx) const;
	/// @brief チェイン用のセクタをクリア(機種依存)
	virtual void	ClearChainSector(const DiskBasicDirItem *pitem = NULL) {}
	/// @brief チェイン用のセクタをセット(機種依存)
	virtual void	SetChainSector(DiskD88Sector *sector, wxUint8 *data, const DiskBasicDirItem *pitem = NULL) {}
	/// @brief チェイン用のセクタをセット(機種依存)
	virtual void	SetChainSector(DiskD88Sector *sector, wxUint32 num, wxUint8 *data, const DiskBasicDirItem *pitem = NULL) {}
	/// @brief チェイン用のセクタをセット(機種依存)
	virtual void	SetChainSector(wxUint32 num, int pos, wxUint8 *data, const DiskBasicDirItem *pitem = NULL) {}
	/// @brief チェイン用のセクタにグループ番号をセット(機種依存)
	virtual void	AddChainGroupNumber(int idx, wxUint32 val) {}
	//@}

	/// @name 日付、時間
	//@{
	/// @brief アイテムが作成日時を持っているか
	virtual bool	HasCreateDateTime() const { return false; }
	/// @brief アイテムが作成日付を持っているか
	virtual bool	HasCreateDate() const { return false; }
	/// @brief アイテムが作成時間を持っているか
	virtual bool	HasCreateTime() const { return false; }
	/// @brief アイテムが変更日時を持っているか
	virtual bool	HasModifyDateTime() const { return false; }
	/// @brief アイテムが変更日付を持っているか
	virtual bool	HasModifyDate() const { return false; }
	/// @brief アイテムが変更時間を持っているか
	virtual bool	HasModifyTime() const { return false; }
	/// @brief アイテムがアクセス日時を持っているか
	virtual bool	HasAccessDateTime() const { return false; }
	/// @brief アイテムがアクセス日付を持っているか
	virtual bool	HasAccessDate() const { return false; }
	/// @brief アイテムがアクセス時間を持っているか
	virtual bool	HasAccessTime() const { return false; }
	///
	enum enDateTime {
		DATETIME_NONE = 0,
		DATETIME_CREATE = 0x01,
		DATETIME_MODIFY = 0x02,
		DATETIME_CREATE_MODIFY = 0x03,
		DATETIME_ACCESS = 0x04,
		DATETIME_CREATE_ACCESS = 0x05,
		DATETIME_MODIFY_ACCESS = 0x06,
		DATETIME_ALL = 0x07
	};
	/// @brief アイテムの時間設定を無視することができるか
	virtual enDateTime CanIgnoreDateTime() const { return DATETIME_NONE; }
	/// @brief 作成日付を得る
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 作成時間を得る
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 作成日時を得る
	virtual void	GetFileCreateDateTime(TM &tm) const;
	/// @brief 作成日時を返す
	virtual TM		GetFileCreateDateTime() const;
	/// @brief 作成日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 作成時間を返す
	virtual wxString GetFileCreateTimeStr() const;
	/// @brief 作成日時を返す
	virtual wxString GetFileCreateDateTimeStr() const;
	/// @brief 変更日付を得る
	virtual void	GetFileModifyDate(TM &tm) const;
	/// @brief 変更時間を得る
	virtual void	GetFileModifyTime(TM &tm) const;
	/// @brief 変更日時を得る
	virtual void	GetFileModifyDateTime(TM &tm) const;
	/// @brief 変更日時を返す
	virtual TM		GetFileModifyDateTime() const;
	/// @brief 変更日付を返す
	virtual wxString GetFileModifyDateStr() const;
	/// @brief 変更時間を返す
	virtual wxString GetFileModifyTimeStr() const;
	/// @brief 変更日時を返す
	virtual wxString GetFileModifyDateTimeStr() const;
	/// @brief アクセス日付を得る
	virtual void	GetFileAccessDate(TM &tm) const;
	/// @brief アクセス時間を得る
	virtual void	GetFileAccessTime(TM &tm) const;
	/// @brief アクセス日時を得る
	virtual void	GetFileAccessDateTime(TM &tm) const;
	/// @brief アクセス日時を返す
	virtual TM		GetFileAccessDateTime() const;
	/// @brief アクセス日付を返す
	virtual wxString GetFileAccessDateStr() const;
	/// @brief アクセス時間を返す
	virtual wxString GetFileAccessTimeStr() const;
	/// @brief アクセス日時を返す
	virtual wxString GetFileAccessDateTimeStr() const;
	/// @brief 作成日付をセット
	virtual void	SetFileCreateDate(const TM &tm) {}
	/// @brief 作成時間をセット
	virtual void	SetFileCreateTime(const TM &tm) {}
	/// @brief 作成日時をセット
	virtual void	SetFileCreateDateTime(const TM &tm);
	/// @brief 作成日付のタイトル名（ダイアログ用）
	virtual wxString GetFileCreateDateTimeTitle() const;
	/// @brief 変更日付をセット
	virtual void	SetFileModifyDate(const TM &tm) {}
	/// @brief 変更時間をセット
	virtual void	SetFileModifyTime(const TM &tm) {}
	/// @brief 変更日時をセット
	virtual void	SetFileModifyDateTime(const TM &tm);
	/// @brief 変更日付のタイトル名（ダイアログ用）
	virtual wxString GetFileModifyDateTimeTitle() const;
	/// @brief アクセス日付をセット
	virtual void	SetFileAccessDate(const TM &tm) {}
	/// @brief アクセス時間をセット
	virtual void	SetFileAccessTime(const TM &tm) {}
	/// @brief アクセス日時をセット
	virtual void	SetFileAccessDateTime(const TM &tm);
	/// @brief アクセス日付のタイトル名（ダイアログ用）
	virtual wxString GetFileAccessDateTimeTitle() const;
	/// @brief 日時の表示順序を返す（ダイアログ用）
	virtual int		GetFileDateTimeOrder(int idx) const;
	/// @brief 日時を返す（ファイルリスト用）
	virtual wxString GetFileDateTimeStr() const;
	/// @brief 日時を指定ファイルに書き込む
	virtual void	WriteFileDateTime(const wxString &path) const;
	/// @brief 指定ファイルから日時を読み込む
	virtual void	ReadFileDateTime(const wxString &path, DiskBasicDirItemAttr &date_time) const;
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
	/// @brief ディレクトリアイテムのサイズ（要実装）
	virtual size_t	GetDataSize() const = 0;
	/// @brief アイテムを返す（要実装）
	virtual directory_t	*GetData() const = 0;
	/// @brief アイテムをコピー（要実装）
	virtual bool	CopyData(const directory_t *val) = 0;
	/// @brief ディレクトリをクリア ファイル新規作成時（要実装）
	virtual void	ClearData() = 0;
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
	virtual bool	NeedChainInData() const { return false; }
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
	virtual void	SetOptionalAttr(DiskBasicDirItemAttr &attr);
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
	bool			WriteFileAttrToXml(const wxString &path);
	/// ファイル属性をXMLから読み込む
	bool			ReadFileAttrFromXml(const wxString &path, DiskBasicDirItemAttr *attr);
	//@}

	/// @name その他
	//@{
	/// @brief アイテムを含むセクタを設定
	void			SetSector(DiskD88Sector	*val) { m_sector = val; }
	/// @brief アイテムの属するセクタを変更済みにする
	virtual void	SetModify();
	/// @brief DISK BASICを返す
	DiskBasic		*GetBasic() const { return basic; }
	/// @brief 通し番号を返す
	int				GetNumber() const { return m_num; }
	/// @brief 通し番号をセット
	void			SetNumber(int val) { m_num = val; }
	/// @brief セクタ内の位置（バイト）を返す
	int				GetPosition() const { return m_position; }
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
	/// @brief プロパティで表示する内部データを設定
	void			SetCommonDataInAttrDialog(KeyValArray &vals) const;
	/// @brief プロパティで表示する内部データを設定
	/// @param[in,out] vals 名前＆値のリスト
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals) {}
	//@}
};

//////////////////////////////////////////////////////////////////////

/// @class DiskBasicDirItems
///
/// @brief DiskBasicDirItem のポインタリスト
WX_DEFINE_ARRAY(DiskBasicDirItem *, DiskBasicDirItems);

//////////////////////////////////////////////////////////////////////

/// @brief 属性値を一時的に集めておくクラス
class DiskBasicDirItemAttr
{
private:
	bool m_rename;
	DiskBasicFileName m_name;

	bool m_ignore_type;
	DiskBasicFileType m_type;

	int m_start_addr;
	int m_end_addr;
	int m_exec_addr;

	bool m_ignore_date_time;
	TM  m_create_date_time;
	TM  m_modify_date_time;
	TM  m_access_date_time;
	TM  m_extern_date_time;

//	int m_user_data[2];

public:
	DiskBasicDirItemAttr();
	~DiskBasicDirItemAttr();

	/// @brief ファイル名を変更できるか
	void Renameable(bool val) { m_rename = val; }
	/// @brief ファイル名をセット
	void SetFileName(const DiskBasicFileName &val) { m_name = val; }
	/// @brief ファイル名をセット
	void SetFileName(const wxString &n_name, int n_optional);
	/// @brief 属性を無視するか
	void IgnoreFileAttr(bool val) { m_ignore_type = val; }
	/// @brief 属性をセット
	void SetFileAttr(const DiskBasicFileType &val) { m_type = val; }
	/// @brief 属性をセット
	void SetFileAttr(DiskBasicFormatType n_format, int n_type, int n_origin0 = 0, int n_origin1 = 0, int n_origin2 = 0);
	/// @brief 共通属性をセット
	void SetFileType(int n_type);
	/// @brief 独自属性をセット
	void SetFileOriginAttr(int idx, int n_origin);
	/// @brief 独自属性をセット
	void SetFileOriginAttr(int n_origin);
	/// @brief 開始アドレスをセット
	void SetStartAddress(int val) { m_start_addr = val; }
	/// @brief 終了アドレスをセット
	void SetEndAddress(int val) { m_end_addr = val; }
	/// @brief 実行アドレスをセット
	void SetExecuteAddress(int val) { m_exec_addr = val; }
	/// @brief 日時を無視するか
	void IgnoreDateTime(bool val) { m_ignore_date_time = val; }
	/// @brief 作成日時をセット
	void SetCreateDateTime(const TM &tm) { m_create_date_time = tm; }
	/// @brief 更新日時をセット
	void SetModifyDateTime(const TM &tm) { m_modify_date_time = tm; }
	/// @brief アクセス日時をセット
	void SetAccessDateTime(const TM &tm) { m_access_date_time = tm; }
	/// @brief その他日時をセット
	void SetExternDateTime(const TM &tm) { m_extern_date_time = tm; }
//	/// @brief 独自のデータをセット
//	void SetUserData(int val, int val2 = 0) { m_user_data[0] = val; m_user_data[1] = val2; }

	/// @brief ファイル名を変更できるか
	bool IsRenameable() const { return m_rename; }
	/// @brief ファイル名を得る
	DiskBasicFileName &GetFileName() { return m_name; }
	/// @brief 属性を無視するか
	bool DoesIgnoreFileAttr() const { return m_ignore_type; }
	/// @brief 属性を得る
	DiskBasicFileType &GetFileAttr() { return m_type; }
	/// @brief 共通属性を得る
	int  GetFileType() const;
	/// @brief 独自属性を得る
	int  GetFileOriginAttr(int idx = 0) const;
	/// @brief 開始アドレスを得る
	int  GetStartAddress() const { return m_start_addr; }
	/// @brief 終了アドレスを得る
	int  GetEndAddress() const { return m_end_addr; }
	/// @brief 実行アドレスを得る
	int  GetExecuteAddress() const { return m_exec_addr; }
	/// @brief 日時を無視するか
	bool DoesIgnoreDateTime() const { return m_ignore_date_time; }
	/// @brief 作成日時を得る
	TM &GetCreateDateTime() { return m_create_date_time; }
	/// @brief 更新日時を得る
	TM &GetModifyDateTime() { return m_modify_date_time; }
	/// @brief アクセス日時を得る
	TM &GetAccessDateTime() { return m_access_date_time; }
	/// @brief その他日時を得る
	TM &GetExternDateTime() { return m_extern_date_time; }
//	/// @brief 独自のデータを得る
//	int  GetUserData(int idx = 0) const { return idx < 2 ? m_user_data[idx] : 0; }
};

//////////////////////////////////////////////////////////////////////

/// @brief セクタをまたぐディレクトリアイテムを処理
class DirItemSectorBoundary
{
private:
	struct {
		wxUint8 *data;
		int		 size;
		int		 pos;
	} s[2];

public:
	DirItemSectorBoundary();
	~DirItemSectorBoundary();
	/// @brief 初期化
	void Clear();
	/// @brief アイテムの位置情報をセット
	bool Set(DiskBasic *basic, DiskD88Sector *sector, int position, directory_t *item_data, size_t item_size, const SectorParam *next);
	/// @brief コピー
	void CopyTo(directory_t *dst_item);
	/// @brief コピー
	void CopyFrom(const directory_t *src_item);
};

#endif /* _BASICDIRITEM_H_ */
