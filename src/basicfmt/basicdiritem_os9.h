/// @file basicdiritem_os9.h
///
/// @brief disk basic directory item for OS-9
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_OS9_H
#define BASICDIRITEM_OS9_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

/// @brief OS-9属性名
extern const char *gTypeNameOS9[];
extern const char gTypeNameOS9_2[];
extern const char *gTypeNameOS9_2l[];
enum en_type_name_os9 {
	TYPE_NAME_OS9_DIRECTORY = 0,
	TYPE_NAME_OS9_NONSHARE = 1,
};
enum en_file_type_mask_os9 {
	FILETYPE_MASK_OS9_DIRECTORY = 0x80,
	FILETYPE_MASK_OS9_NONSHARE = 0x40,
	FILETYPE_MASK_OS9_PUBLIC_EXEC = 0x20,
	FILETYPE_MASK_OS9_PUBLIC_WRITE = 0x10,
	FILETYPE_MASK_OS9_PUBLIC_READ = 0x08,
	FILETYPE_MASK_OS9_USER_EXEC = 0x04,
	FILETYPE_MASK_OS9_USER_WRITE = 0x02,
	FILETYPE_MASK_OS9_USER_READ = 0x01,
};

//////////////////////////////////////////////////////////////////////

/// @brief OS-9 File Descriptorエリアのポインタ
class DiskBasicDirItemOS9FD
{
private:
	DiskBasic			*basic;
	DiskImageSector		*sector;
	directory_os9_fd_t	*p_fd;
	wxUint32			m_mylsn;
	bool				m_fd_ownmake;
	union {
		os9_date_t	date;
		os9_cdate_t	cdate;
	} zero_data;

	DiskBasicDirItemOS9FD(const DiskBasicDirItemOS9FD &src);

public:
	DiskBasicDirItemOS9FD();
	~DiskBasicDirItemOS9FD();
#ifdef COPYABLE_DIRITEM
	/// @brief 代入
	DiskBasicDirItemOS9FD &operator=(const DiskBasicDirItemOS9FD &src);
	/// @brief 複製
	void Dup(const DiskBasicDirItemOS9FD &src);
#endif
	/// @brief ポインタをセット
	void Set(DiskBasic *n_basic, DiskImageSector *n_sector, wxUint32 n_mylsn, directory_os9_fd_t *n_fd);
	/// @brief FDのメモリ確保
	void Alloc();
	/// @brief FDをクリア
	void Clear();
	/// @brief 有効か
	bool IsValid() const;
	/// @brief FDへのポインタを返す
	const directory_os9_fd_t *GetFD() const;
	/// @brief 自分のLSNを返す
	wxUint32 GetMyLSN() const { return m_mylsn; }
	/// @brief 自分のLSNを設定
	void SetMyLSN(wxUint32 val) { m_mylsn = val; }
	/// @brief 属性を返す
	wxUint8 GetATT() const;
	/// @brief 属性をセット
	void SetATT(wxUint8 val);
	/// @brief ユーザIDを返す
	wxUint16 GetOWN() const;
	/// @brief ユーザIDをセット
	void SetOWN(wxUint16 val);
	/// @brief セグメントのLSNを返す
	wxUint32 GetLSN(int idx) const;
	/// @brief セグメントのセクタ数を返す
	wxUint16 GetSIZ(int idx) const;
	/// @brief セグメントにLSNを設定
	void SetLSN(int idx, wxUint32  val);
	/// @brief セグメントにセクタ数を設定
	void SetSIZ(int idx, wxUint16 val);
	/// @brief ファイルサイズを返す
	wxUint32 GetSIZ() const;
	/// @brief ファイルサイズを設定
	void SetSIZ(wxUint32 val);
	/// @brief リンク数を返す
	wxUint8 GetLNK() const;
	/// @brief リンク数を設定
	void SetLNK(wxUint8 val);
	/// @brief 更新日付を返す
	const os9_date_t &GetDAT() const;
	/// @brief 更新日付をセット
	void SetDAT(const os9_date_t &val);
	/// @brief 更新日付をセット
	void SetDAT(const os9_cdate_t &val);
	/// @brief 作成日付を返す
	const os9_cdate_t &GetDCR() const;
	/// @brief 作成日付をセット
	void SetDCR(const os9_cdate_t &val);
	/// @brief 更新にする
	virtual void	SetModify();
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム OS-9
class DiskBasicDirItemOS9 : public DiskBasicDirItem
{
private:
	DiskBasicDirItemOS9() : DiskBasicDirItem() {}
	DiskBasicDirItemOS9(const DiskBasicDirItemOS9 &src) : DiskBasicDirItem(src) {}

#ifdef COPYABLE_DIRITEM
	/// @brief 複製
	void	Dup(const DiskBasicDirItem &src);
#endif
	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_os9_t> m_data;

	/// @brief File Descriptorエリアのポインタ
	DiskBasicDirItemOS9FD fd;

	/// @brief ユーザID(プロパティダイアログ用)
	wxUint8 m_owner_id;
	/// @brief グループID(プロパティダイアログ用)
	wxUint8 m_group_id;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief ユーザIDを返す
	int				GetUserID() const;
	/// @brief ユーザIDのセット
	void			SetUserID(int val);
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int				GetFileType1Pos();
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1);

	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

	/// @brief 日付を変換
	void			ConvDateToTm(const os9_cdate_t &date, TM &tm) const;
	/// @brief 時間を変換
	void			ConvTimeToTm(const os9_date_t &time, TM &tm) const;
	/// @brief 日付に変換
	void			ConvTmToDate(const TM &tm, os9_cdate_t &date) const;
	/// @brief 時間に変換
	void			ConvTmToTime(const TM &tm, os9_date_t &time) const;

public:
	DiskBasicDirItemOS9(DiskBasic *basic);
	DiskBasicDirItemOS9(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemOS9(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

	/// @brief 削除
	virtual bool	Delete();

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// @brief ファイルサイズを返す
	virtual int		GetFileSize() const;
	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 追加のグループ番号をセット FDセクタへのLSNをセット
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す FDセクタへのLSNを返す
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る
	virtual void	GetExtraGroups(wxArrayInt &arr) const;

	/// @brief チェイン用のセクタをセット
	virtual void	SetChainSector(DiskImageSector *sector, wxUint32 lsn, wxUint8 *data, const DiskBasicDirItem *pitem = NULL);

	/// @brief アイテムが作成日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	virtual bool	HasCreateDate() const { return true; }
	virtual bool	HasCreateTime() const { return false; }
	/// @brief 作成日付を返す
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 作成日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 作成日付をセット
	virtual void	SetFileCreateDate(const TM &tm);

	/// @brief アイテムが更新日時を持っているか
	virtual bool	HasModifyDateTime() const { return true; }
	virtual bool	HasModifyDate() const { return true; }
	virtual bool	HasModifyTime() const { return true; }
	/// @brief 更新日付を返す
	virtual void	GetFileModifyDate(TM &tm) const;
	/// @brief 更新時間を返す
	virtual void	GetFileModifyTime(TM &tm) const;
	/// @brief 更新日付を返す
	virtual wxString GetFileModifyDateStr() const;
	/// @brief 更新時間を返す
	virtual wxString GetFileModifyTimeStr() const;
	/// @brief 更新日付をセット
	virtual void	SetFileModifyDate(const TM &tm);
	/// @brief 更新時間をセット
	virtual void	SetFileModifyTime(const TM &tm);
	/// @brief 日時の表示順序を返す（ダイアログ用）
	virtual int		GetFileDateTimeOrder(int idx) const;
	/// @brief 日時を返す（ファイルリスト用）
	virtual wxString GetFileDateTimeStr() const;

	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief ファイル名を編集できるか
	virtual bool	IsFileNameEditable() const;
	/// @brief アイテムをロード・エクスポートできるか
	virtual bool	IsLoadable() const;
	/// @brief アイテムをコピー(内部でDnD)できるか
	virtual bool	IsCopyable() const;
	/// @brief アイテムを上書きできるか
	virtual bool	IsOverWritable() const;

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();
	/// @brief アイテムをコピー
	virtual void	CopyItem(const DiskBasicDirItem &src);

	/// @brief FDセクタのポインタを返す
	DiskBasicDirItemOS9FD &GetFD() { return fd; }
	/// @brief FDセクタのポインタを返す
	const DiskBasicDirItemOS9FD &GetFD() const { return fd; }

	/// @brief アイテムの属するセクタを変更済みにする
	virtual void	SetModify();

	/// @brief 文字列の最後のMSBをセット
	static size_t	EncodeString(wxUint8 *dst, size_t dlen, const char *src, size_t slen);
	/// @brief 文字列の最後のMSBをクリア
	static size_t	DecodeString(char *dst, size_t dlen, const wxUint8 *src, size_t slen);

	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief データをインポートする前に必要な処理
	virtual bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief その他の属性値を設定する
	virtual void	SetOptionalAttr(DiskBasicDirItemAttr &attr);

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief ダイアログ内の値を設定
	virtual void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* BASICDIRITEM_OS9_H */
