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
#define FILETYPE_OS9_PERMISSION_MASK 0x3f00000
#define FILETYPE_OS9_PERMISSION_POS  20

/// File Descriptorエリアのポインタ
class DiskBasicDirItemOS9FD
{
private:
	DiskD88Sector		*sector;
	directory_os9_fd_t	*fd;
	bool				fd_ownmake;
	union {
		os9_date_t	date;
		os9_cdate_t	cdate;
	} zero_data;

	DiskBasicDirItemOS9FD(const DiskBasicDirItemOS9FD &src);

public:
	DiskBasicDirItemOS9FD();
	~DiskBasicDirItemOS9FD();
	/// 代入
	DiskBasicDirItemOS9FD &operator=(const DiskBasicDirItemOS9FD &src);
	/// 複製
	void Dup(const DiskBasicDirItemOS9FD &src);
	/// ポインタをセット
	void Set(DiskD88Sector *n_sector, directory_os9_fd_t *n_fd);
	/// FDのメモリ確保
	void Alloc();
	/// FDをクリア
	void Clear();
	/// 有効か
	bool IsValid() const { return (fd != NULL); }
	/// 属性を返す
	wxUint8 GetATT() const;
	/// 属性をセット
	void SetATT(wxUint8 val);
	/// セグメントのLSNを返す
	wxUint32 GetLSN(int idx) const;
	/// セグメントのセクタ数を返す
	wxUint16 GetSIZ(int idx) const;
	/// セグメントにLSNを設定
	void SetLSN(int idx, wxUint32  val);
	/// セグメントにセクタ数を設定
	void SetSIZ(int idx, wxUint16 val);
	/// ファイルサイズを返す
	wxUint32 GetSIZ() const;
	/// ファイルサイズを設定
	void SetSIZ(wxUint32 val);
	/// リンク数を返す
	wxUint8 GetLNK() const;
	/// リンク数を設定
	void SetLNK(wxUint8 val);
	/// 更新日付を返す
	const os9_date_t &GetDAT() const;
	/// 更新日付をセット
	void SetDAT(const os9_date_t &val);
	/// 更新日付をセット
	void SetDAT(const os9_cdate_t &val);
	/// 作成日付を返す
	const os9_cdate_t &GetDCR() const;
	/// 作成日付をセット
	void SetDCR(const os9_cdate_t &val);
	/// 更新にする
	void SetModify();
};

/// ディレクトリ１アイテム OS-9
class DiskBasicDirItemOS9 : public DiskBasicDirItem
{
private:
	DiskBasicDirItemOS9() : DiskBasicDirItem() {}
	DiskBasicDirItemOS9(const DiskBasicDirItemOS9 &src) : DiskBasicDirItem(src) {}

	/// File Descriptorエリアのポインタ
	DiskBasicDirItemOS9FD fd;

	/// ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// ファイル名を格納するバッファサイズを返す
	int		GetFileNameSize(bool *invert = NULL) const;
	/// 属性１を返す
	int		GetFileType1() const;
	/// 属性１のセット
	void	SetFileType1(int val);
	/// 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int		GetFileType1Pos();
	/// 属性からリストの位置を返す(プロパティダイアログ用)
	int		GetFileType2Pos();
	/// インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// ファイル名を設定
	void	SetFileName(const wxUint8 *filename, int length);
	/// ファイル名を得る
	void	GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

	/// 日付を変換
	void			ConvDateToTm(const os9_cdate_t &date, struct tm *tm) const;
	/// 時間を変換
	void			ConvTimeToTm(const os9_date_t &time, struct tm *tm) const;
	/// 日付に変換
	void			ConvTmToDate(const struct tm *tm, os9_cdate_t &date) const;
	/// 時間に変換
	void			ConvTmToTime(const struct tm *tm, os9_date_t &time) const;

public:
	DiskBasicDirItemOS9(DiskBasic *basic);
	DiskBasicDirItemOS9(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemOS9(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// 複製
	void			Dup(const DiskBasicDirItem &src);
	/// ディレクトリアイテムのチェック
	bool			Check(bool &last);

	/// ファイル名に設定できない文字を文字列にして返す
	wxString		InvalidateChars() const;
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
	/// ファイルサイズを返す
	int				GetFileSize() const;
	/// ファイルサイズとグループ数を計算する
	void			CalcFileSize();
	/// 指定ディレクトリのすべてのグループを取得
	void			GetAllGroups(DiskBasicGroups &group_items);

	/// 最初のグループ番号をセット
	void			SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	wxUint32		GetStartGroup() const;
	/// 追加のグループ番号をセット FDセクタへのLSNをセット
	void			SetExtraGroup(wxUint32 val);
	/// 追加のグループ番号を返す FDセクタへのLSNを返す
	wxUint32		GetExtraGroup() const;

	/// アイテムが日時を持っているか
	bool			HasDateTime() const { return true; }
	bool			HasDate() const { return true; }
	bool			HasTime() const { return true; }
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
	/// 日付のタイトル名（ダイアログ用）
	wxString		GetFileDateTimeTitle() const;
	/// 日付を返す
	wxString		GetCDateStr() const;
	/// 日付をセット
	void			SetCDate(const struct tm *tm);

	/// ディレクトリアイテムのサイズ
	size_t			GetDataSize() const;

	/// アイテムを削除できるか
	bool			IsDeletable() const;
	/// ファイル名を編集できるか
	bool			IsFileNameEditable() const;

	/// アイテムをコピー
	void			CopyItem(const DiskBasicDirItem &src);

	/// FDセクタのポインタを返す
	DiskBasicDirItemOS9FD &GetFD() { return fd; }
	/// FDセクタのポインタを返す
	const DiskBasicDirItemOS9FD &GetFD() const { return fd; }

	/// アイテムの属するセクタを変更済みにする
	void			SetModify();

	/// 文字列の最後のMSBをセット
	static size_t	EncodeString(wxUint8 *dst, size_t dlen, const char *src, size_t slen);
	/// 文字列の最後のMSBをクリア
	static size_t	DecodeString(char *dst, size_t dlen, const wxUint8 *src, size_t slen);

	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// ダイアログ内の値を設定
	void	InitializeForAttrDialog(IntNameBox *parent, int showitems, int *user_data);
	/// 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
	//@}
};

#endif /* _BASICDIRITEM_OS9_H_ */
