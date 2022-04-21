/// @file basicdiritem_os9.h
///
/// @brief disk basic directory item for OS-9
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_OS9_H_
#define _BASICDIRITEM_OS9_H_

#include "basicdiritem.h"


/// OS-9属性名
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

/// File Descriptorエリアのポインタ
class DiskBasicDirItemOS9FD
{
private:
	DiskBasic			*basic;
	DiskD88Sector		*sector;
	directory_os9_fd_t	*fd;
	wxUint32			mylsn;
	bool				fd_ownmake;
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
	void Set(DiskBasic *n_basic, DiskD88Sector *n_sector, wxUint32 n_mylsn, directory_os9_fd_t *n_fd);
	/// @brief FDのメモリ確保
	void Alloc();
	/// @brief FDをクリア
	void Clear();
	/// @brief 有効か
	bool IsValid() const { return (fd != NULL); }
	/// @brief 自分のLSNを返す
	wxUint32 GetMyLSN() const { return mylsn; }
	/// @brief 自分のLSNを設定
	void SetMyLSN(wxUint32 val) { mylsn = val; }
	/// @brief 属性を返す
	wxUint8 GetATT() const;
	/// @brief 属性をセット
	void SetATT(wxUint8 val);
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
	void SetModify();
};

/// ディレクトリ１アイテム OS-9
class DiskBasicDirItemOS9 : public DiskBasicDirItem
{
private:
	DiskBasicDirItemOS9() : DiskBasicDirItem() {}
	DiskBasicDirItemOS9(const DiskBasicDirItemOS9 &src) : DiskBasicDirItem(src) {}

#ifdef COPYABLE_DIRITEM
	/// @brief 複製
	void	Dup(const DiskBasicDirItem &src);
#endif

	/// @brief File Descriptorエリアのポインタ
	DiskBasicDirItemOS9FD fd;

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int		GetFileType1Pos();
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1);

	/// @brief ファイル名を設定
	void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名を得る
	void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

	/// @brief 日付を変換
	void	ConvDateToTm(const os9_cdate_t &date, struct tm *tm) const;
	/// @brief 時間を変換
	void	ConvTimeToTm(const os9_date_t &time, struct tm *tm) const;
	/// @brief 日付に変換
	void	ConvTmToDate(const struct tm *tm, os9_cdate_t &date) const;
	/// @brief 時間に変換
	void	ConvTmToTime(const struct tm *tm, os9_date_t &time) const;

public:
	DiskBasicDirItemOS9(DiskBasic *basic);
	DiskBasicDirItemOS9(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemOS9(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// @brief 削除
	bool	Delete(wxUint8 code);

	/// @brief 属性を設定
	void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を返す
	DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	wxString GetFileAttrStr() const;

	/// @brief ファイルサイズをセット
	void	SetFileSize(int val);
	/// @brief ファイルサイズを返す
	int		GetFileSize() const;
	/// @brief ファイルサイズとグループ数を計算する
	void	CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief 最初のグループ番号をセット
	void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 追加のグループ番号をセット FDセクタへのLSNをセット
	void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す FDセクタへのLSNを返す
	wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る
	void	GetExtraGroups(wxArrayInt &arr) const;

	/// @brief チェイン用のセクタをセット
	void	SetChainSector(DiskD88Sector *sector, wxUint32 lsn, wxUint8 *data, const DiskBasicDirItem *pitem = NULL);

	/// @brief アイテムが日時を持っているか
	bool	HasDateTime() const { return true; }
	bool	HasDate() const { return true; }
	bool	HasTime() const { return true; }
	/// @brief 日付を返す
	void	GetFileDate(struct tm *tm) const;
	/// @brief 時間を返す
	void	GetFileTime(struct tm *tm) const;
	/// @brief 日付を返す
	wxString GetFileDateStr() const;
	/// @brief 時間を返す
	wxString GetFileTimeStr() const;
	/// @brief 日付をセット
	void	SetFileDate(const struct tm *tm);
	/// @brief 時間をセット
	void	SetFileTime(const struct tm *tm);
	/// @brief 日付のタイトル名（ダイアログ用）
	wxString GetFileDateTimeTitle() const;
	/// @brief 日付を返す
	wxString GetCDateStr() const;
	/// @brief 日付をセット
	void	SetCDate(const struct tm *tm);

	/// @brief ディレクトリアイテムのサイズ
	size_t	GetDataSize() const;

	/// @brief アイテムを削除できるか
	bool	IsDeletable() const;
	/// @brief ファイル名を編集できるか
	bool	IsFileNameEditable() const;
	/// @brief アイテムをロード・エクスポートできるか
	bool	IsLoadable() const;
	/// @brief アイテムをコピー(内部でDnD)できるか
	bool	IsCopyable() const;
	/// @brief アイテムを上書きできるか
	bool	IsOverWritable() const;

	/// @brief アイテムをコピー
	void	CopyItem(const DiskBasicDirItem &src);

	/// @brief FDセクタのポインタを返す
	DiskBasicDirItemOS9FD &GetFD() { return fd; }
	/// @brief FDセクタのポインタを返す
	const DiskBasicDirItemOS9FD &GetFD() const { return fd; }

	/// @brief アイテムの属するセクタを変更済みにする
	void	SetModify();

	/// @brief 文字列の最後のMSBをセット
	static size_t	EncodeString(wxUint8 *dst, size_t dlen, const char *src, size_t slen);
	/// @brief 文字列の最後のMSBをクリア
	static size_t	DecodeString(char *dst, size_t dlen, const wxUint8 *src, size_t slen);

	/// @brief データをエクスポートする前に必要な処理
	bool	PreExportDataFile(wxString &filename);
	/// @brief データをインポートする前に必要な処理
	bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief その他の属性値を設定する
	void	SetAttr(DiskBasicDirItemAttr &attr);

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief ダイアログ内の値を設定
	void	InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data);
	/// @brief 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログ入力後のファイル名チェック
	bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	//@}
};

#endif /* _BASICDIRITEM_OS9_H_ */
