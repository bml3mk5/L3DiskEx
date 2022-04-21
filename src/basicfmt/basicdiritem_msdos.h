/// @file basicdiritem_msdos.h
///
/// @brief disk basic directory item for MS-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_MSDOS_H_
#define _BASICDIRITEM_MSDOS_H_

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

/// @brief MS-DOS 属性名
extern const name_value_t gTypeNameMS[];
extern const name_value_t gTypeNameMS_l[];
enum en_type_name_ms {
	TYPE_NAME_MS_READ_ONLY = 0,
	TYPE_NAME_MS_HIDDEN,
	TYPE_NAME_MS_SYSTEM,
	TYPE_NAME_MS_VOLUME,
	TYPE_NAME_MS_DIRECTORY,
	TYPE_NAME_MS_ARCHIVE,
	TYPE_NAME_MS_LFN,
	TYPE_NAME_MS_END
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

class wxStaticBoxSizer;

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム MS-DOS
class DiskBasicDirItemMSDOS : public DiskBasicDirItem
{
protected:
	DiskBasicDirItemMSDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemMSDOS(const DiskBasicDirItemMSDOS &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_ms_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	void			GetFileAttrStrSub(int ftype, wxString &attr) const;

	/// @brief 日付を変換
	static void		ConvDateToTm(wxUint16 date, TM &tm);
	/// @brief 時間を変換
	static void		ConvTimeToTm(wxUint16 time, TM &tm);
	/// @brief 日付に変換
	static wxUint16	ConvTmToDate(const TM &tm);
	/// @brief 時間に変換
	static wxUint16	ConvTmToTime(const TM &tm);

	/// @brief ダイアログ表示前にファイルの属性を設定
	virtual void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief ダイアログ内の属性部分のレイアウトを作成
	wxStaticBoxSizer *CreateControlsSubForAttrDialog(IntNameBox *parent, int show_flags, wxBoxSizer *sizer, wxSizerFlags &flags, int file_type_1);
	/// @brief 属性を設定する
	void			SetAttrSubInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr) const;

public:
	DiskBasicDirItemMSDOS(DiskBasic *basic);
	DiskBasicDirItemMSDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemMSDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief 削除
	virtual bool	Delete();
	/// @brief ファイル名を編集できるか
	virtual bool	IsFileNameEditable() const;
	/// @brief アイテムをロード・エクスポートできるか
	virtual bool	IsLoadable() const;
	/// @brief アイテムをコピー(内部でDnD)できるか
	virtual bool	IsCopyable() const;
	/// @brief アイテムを上書きできるか
	virtual bool	IsOverWritable() const;
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

	/// @brief アイテムが更新日時を持っているか
	virtual bool 	HasModifyDateTime() const { return true; }
	virtual bool 	HasModifyDate() const { return true; }
	virtual bool 	HasModifyTime() const { return true; }
	/// @brief 更新日付を得る
	virtual void	GetFileModifyDate(TM &tm) const;
	/// @brief 更新時間を得る
	virtual void	GetFileModifyTime(TM &tm) const;
	/// @brief 更新日付を文字列で返す
	virtual wxString GetFileModifyDateStr() const;
	/// @brief 更新時間を文字列で返す
	virtual wxString GetFileModifyTimeStr() const;
	/// @brief 更新日付をセット
	virtual void	SetFileModifyDate(const TM &tm);
	/// @brief 更新時間をセット
	virtual void	SetFileModifyTime(const TM &tm);
	/// @brief 更新日付のタイトル名（ダイアログ用）
	virtual wxString GetFileModifyDateTimeTitle() const;
	/// @brief 日時の表示順序を返す（ダイアログ用）
	virtual int		GetFileDateTimeOrder(int idx) const;
	/// @brief 日時を返す（ファイルリスト用）
	virtual wxString GetFileDateTimeStr() const;

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t	*GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief ファイル名から属性を決定する
	virtual int		ConvFileTypeFromFileName(const wxString &filename) const;


	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム MS-DOS VFAT
class DiskBasicDirItemVFAT : public DiskBasicDirItemMSDOS
{
protected:
	DiskBasicDirItemVFAT() : DiskBasicDirItemMSDOS() {}
	DiskBasicDirItemVFAT(const DiskBasicDirItemVFAT &src) : DiskBasicDirItemMSDOS(src) {}

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;

public:
	DiskBasicDirItemVFAT(DiskBasic *basic);
	DiskBasicDirItemVFAT(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemVFAT(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);
	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;
	/// @brief ファイル名を編集できるか
	virtual bool	IsFileNameEditable() const;

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);

	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;

	/// @brief アイテムが作成日時を持っているか
	virtual bool 	HasCreateDateTime() const { return true; }
	virtual bool 	HasCreateDate() const { return true; }
	virtual bool 	HasCreateTime() const { return true; }
	/// @brief 作成日付を返す
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 作成時間を返す
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 作成日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 作成時間を返す
	virtual wxString GetFileCreateTimeStr() const;
	/// @brief 作成日付をセット
	virtual void	SetFileCreateDate(const TM &tm);
	/// @brief 作成時間をセット
	virtual void	SetFileCreateTime(const TM &tm);

	/// @brief アイテムがアクセス日時を持っているか
	virtual bool 	HasAccessDateTime() const { return true; }
	virtual bool 	HasAccessDate() const { return true; }
	virtual bool 	HasAccessTime() const { return false; }
	/// @brief アクセス日付を返す
	virtual void	GetFileAccessDate(TM &tm) const;
	/// @brief アクセス日付を返す
	virtual wxString GetFileAccessDateStr() const;
	/// @brief アクセス日付をセット
	virtual void	SetFileAccessDate(const TM &tm);

	/// @brief 文字列をバイト列に変換 文字コードは機種依存
	virtual int		ConvStringToChars(const wxString &src, wxUint8 *dst, size_t len) const;
	/// @brief バイト列を文字列に変換 文字コードは機種依存
	virtual void	ConvCharsToString(const wxUint8 *src, size_t len, wxString &dst) const;

	/// @brief その他の属性値を設定する
	virtual void	SetAttr(DiskBasicDirItemAttr &attr);

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_MSDOS_H_ */
