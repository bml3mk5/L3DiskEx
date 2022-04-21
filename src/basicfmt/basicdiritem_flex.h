/// @file basicdiritem_flex.h
///
/// @brief disk basic directory item for FLEX
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_FLEX_H_
#define _BASICDIRITEM_FLEX_H_

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

enum en_type_name_flex {
	TYPE_NAME_FLEX_READ_ONLY = 0,
	TYPE_NAME_FLEX_UNDELETE,
	TYPE_NAME_FLEX_WRITE_ONLY,
	TYPE_NAME_FLEX_HIDDEN,
	TYPE_NAME_FLEX_RANDOM
};
/// @brief FLEX属性
enum en_file_type_mask_flex {
	FILETYPE_MASK_FLEX_READ_ONLY = 0x80,
	FILETYPE_MASK_FLEX_UNDELETE = 0x40,
	FILETYPE_MASK_FLEX_WRITE_ONLY = 0x20,
	FILETYPE_MASK_FLEX_HIDDEN = 0x10,
};
#define FILETYPE_FLEX_RANDOM_MASK	0xff00000
#define FILETYPE_FLEX_RANDOM_POS	20

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム FLEX
class DiskBasicDirItemFLEX : public DiskBasicDirItem
{
private:
	DiskBasicDirItemFLEX() : DiskBasicDirItem() {}
	DiskBasicDirItemFLEX(const DiskBasicDirItemFLEX &src) : DiskBasicDirItem(src) {}

	inline int PhySecPos(int sector_number) const;
	inline int SecBufOfs(int sector_number) const;
	inline int LogSecSiz(int sector_size) const;

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_flex_t> m_data;

	/// @brief ランダムアクセスファイルのインデックス(FSM)のグループ番号
	wxArrayInt m_random_group_nums;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

public:
	DiskBasicDirItemFLEX(DiskBasic *basic);
	DiskBasicDirItemFLEX(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemFLEX(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

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
	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);
	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 最後のグループ番号をセット(機種依存)
	virtual void	SetLastGroup(wxUint32 val);
	/// @brief 最後のグループ番号を返す(機種依存)
	virtual wxUint32 GetLastGroup() const;
	/// @brief 追加のグループ番号を得る(機種依存)
	virtual void	GetExtraGroups(wxArrayInt &arr) const;

	/// @brief アイテムが日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	virtual bool	HasCreateDate() const { return true; }
	virtual bool	HasCreateTime() const { return false; }
	/// @brief 日付を返す
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 時間を返す
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 時間を返す
	virtual wxString GetFileCreateTimeStr() const;
	/// @brief 日付をセット
	virtual void	SetFileCreateDate(const TM &tm);
	/// @brief 時間をセット
	virtual void	SetFileCreateTime(const TM &tm);

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;

	/// @brief 最初のトラック番号をセット
	void			SetStartTrack(wxUint8 val);
	/// @brief 最初のセクタ番号をセット
	void			SetStartSector(wxUint8 val);
	/// @brief 最初のトラック番号を返す
	wxUint8			GetStartTrack() const;
	/// @brief 最初のセクタ番号を返す
	wxUint8			GetStartSector() const;
	/// @brief 最後のトラック番号をセット
	void			SetLastTrack(wxUint8 val);
	/// @brief 最後のセクタ番号をセット
	void			SetLastSector(wxUint8 val);
	/// @brief 最後のトラック番号を返す
	wxUint8			GetLastTrack() const;
	/// @brief 最後のセクタ番号を返す
	wxUint8			GetLastSector() const;


	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
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

#endif /* _BASICDIRITEM_FLEX_H_ */
