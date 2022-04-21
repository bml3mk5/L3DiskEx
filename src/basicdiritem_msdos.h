/// @file basicdiritem_msdos.h
///
/// @brief disk basic directory item for MS-DOS
///
#ifndef _BASICDIRITEM_MSDOS_H_
#define _BASICDIRITEM_MSDOS_H_

#include "basicdiritem.h"

/// MS-DOS 属性名
extern const char *gTypeNameMS[];
extern const char *gTypeNameMS_l[];
enum en_type_name_ms {
	TYPE_NAME_MS_READ_ONLY = 0,
	TYPE_NAME_MS_HIDDEN = 1,
	TYPE_NAME_MS_SYSTEM = 2,
	TYPE_NAME_MS_VOLUME = 3,
	TYPE_NAME_MS_DIRECTORY = 4,
	TYPE_NAME_MS_ARCHIVE = 5
};
enum en_file_type_mask_ms {
	FILETYPE_MASK_MS_READ_ONLY = 0x01,
	FILETYPE_MASK_MS_HIDDEN = 0x02,
	FILETYPE_MASK_MS_SYSTEM = 0x04,
	FILETYPE_MASK_MS_VOLUME = 0x08,
	FILETYPE_MASK_MS_DIRECTORY = 0x10,
	FILETYPE_MASK_MS_ARCHIVE = 0x20,
	FILETYPE_MASK_MS_LFN = 0x0f	// long file name
};

/// ディレクトリ１アイテム MS-DOS
class DiskBasicDirItemMSDOS : public DiskBasicDirItem
{
protected:
	DiskBasicDirItemMSDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemMSDOS(const DiskBasicDirItemMSDOS &src) : DiskBasicDirItem(src) {}

	/// ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// ファイル名を格納するバッファサイズを返す
	virtual int 	GetFileNameSize(bool *invert = NULL) const;
	/// 拡張子を格納するバッファサイズを返す
	virtual int 	GetFileExtSize(bool *invert = NULL) const;
	/// 属性１を返す
	virtual int		GetFileType1() const;
	/// 属性１のセット
	virtual void	SetFileType1(int val);
	/// 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);
	/// ファイル名を設定
	virtual void	SetFileName(const wxUint8 *filename, int length);
	/// ファイル名と拡張子を得る
	virtual void	GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

	/// 日付を変換
	void			ConvDateToTm(wxUint16 date, struct tm *tm);
	/// 時間を変換
	void			ConvTimeToTm(wxUint16 time, struct tm *tm);
	/// 日付に変換
	wxUint16		ConvTmToDate(const struct tm *tm);
	/// 時間に変換
	wxUint16		ConvTmToTime(const struct tm *tm);

public:
	DiskBasicDirItemMSDOS(DiskBasic *basic);
	DiskBasicDirItemMSDOS(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemMSDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// アイテムを削除できるか
	virtual bool	IsDeletable();
	/// ファイル名に設定できない文字を文字列にして返す
	virtual wxString InvalidateChars();
	/// ファイル名は必須（空文字不可）か
	virtual bool	IsFileNameRequired() { return true; }
	/// 属性を設定
	virtual void	SetFileAttr(int file_type);

	/// 属性を返す
	virtual int		GetFileAttr();

	/// 属性からリストの位置を返す(プロパティダイアログ用)
	virtual int		GetFileType1Pos();
	/// リストの位置から属性を返す(プロパティダイアログ用)
	virtual int		CalcFileTypeFromPos(int pos1, int pos2);
	/// 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr();
	/// ファイルサイズをセット
	virtual void	SetFileSize(int val);
	/// ファイルサイズとグループ数を計算する
	virtual void	CalcFileSize();

	/// 指定ディレクトリのすべてのグループを取得
	virtual void	GetAllGroups(DiskBasicGroups &group_items);

	/// 最初のグループ番号をセット
	virtual void	SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup() const;

	/// アイテムが日時を持っているか
	virtual bool 	HasDateTime() const { return true; }
	virtual bool 	HasDate() const { return true; }
	virtual bool 	HasTime() const { return true; }
	/// 日付を返す
	virtual void	GetFileDate(struct tm *tm);
	/// 時間を返す
	virtual void	GetFileTime(struct tm *tm);
	/// 日付を返す
	virtual wxString GetFileDateStr();
	/// 時間を返す
	virtual wxString GetFileTimeStr();
	/// 日付をセット
	virtual void	SetFileDate(const struct tm *tm);
	/// 時間をセット
	virtual void	SetFileTime(const struct tm *tm);
	/// 日付のタイトル名（ダイアログ用）
	virtual wxString GetFileDateTimeTitle();
	/// 日付を返す
	wxString		GetCDateStr();
	/// 時間を返す
	wxString		GetCTimeStr();
	/// 日付を返す
	wxString		GetADateStr();
	/// 日付をセット
	void			SetCDate(const struct tm *tm);
	/// 時間をセット
	void			SetCTime(const struct tm *tm);
	/// 日付をセット
	void			SetADate(const struct tm *tm);

	/// ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize();

	/// ダイアログ入力前のファイル名を変換 大文字にする
	virtual void	ConvertToFileNameStr(wxString &filename);
	/// ダイアログ入力後のファイル名文字列を変換 大文字にする
	virtual void	ConvertFromFileNameStr(wxString &filename);


	/// @name プロパティダイアログ用
	//@{
	/// ダイアログ表示前にファイルの属性を設定
	virtual void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);
	/// ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// 属性1を得る
	virtual int		GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo);
	//@}
};

#endif /* _BASICDIRITEM_MSDOS_H_ */
