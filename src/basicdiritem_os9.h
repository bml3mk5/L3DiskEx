/// @file basicdiritem_os9.h
///
/// @brief disk basic directory item for OS-9
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
public:
	DiskD88Sector		*sector;
	directory_os9_fd_t	*fd;

public:
	DiskBasicDirItemOS9FD();
	~DiskBasicDirItemOS9FD() {}
	/// ポインタをセット
	void Set(DiskD88Sector *n_sector, directory_os9_fd_t *n_fd);
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
	/// ファイルサイズを返す
	wxUint32 GetSIZ() const;
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

	/// ファイル名を設定
	void	SetFileName(const wxUint8 *filename, int length);
	/// ファイル名を得る
	void	GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen);

	/// 日付を変換
	void			ConvDateToTm(const os9_cdate_t &date, struct tm *tm);
	/// 時間を変換
	void			ConvTimeToTm(const os9_date_t &time, struct tm *tm);
	/// 日付に変換
	void			ConvTmToDate(const struct tm *tm, os9_cdate_t &date);
	/// 時間に変換
	void			ConvTmToTime(const struct tm *tm, os9_date_t &time);

public:
	DiskBasicDirItemOS9(DiskBasic *basic);
	DiskBasicDirItemOS9(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemOS9(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool			Check(bool &last);

	/// ファイル名に設定できない文字を文字列にして返す
	wxString		InvalidateChars();
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
	/// 追加のグループ番号をセット FDセクタへのLSNをセット
	void			SetExtraGroup(wxUint32 val);
	/// 追加のグループ番号を返す FDセクタへのLSNを返す
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
	/// 日付のタイトル名（ダイアログ用）
	wxString		GetFileDateTimeTitle();
	/// 日付を返す
	wxString		GetCDateStr();
	/// 日付をセット
	void			SetCDate(const struct tm *tm);

	/// ディレクトリアイテムのサイズ
	size_t			GetDataSize();

	/// 書き込み/上書き禁止か
	bool			IsWriteProtected();
	/// アイテムを削除できるか
	bool			IsDeleteable();
	/// ファイル名を編集できるか
	bool			IsFileNameEditable();

	/// FDセクタのポインタを返す
	DiskBasicDirItemOS9FD &GetFD() { return fd; }

	/// アイテムの属するセクタを変更済みにする
	void			SetModify();


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
	/// 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const AttrControls &controls, DiskBasicError &errinfo);
	//@}
};

#endif /* _BASICDIRITEM_OS9_H_ */
