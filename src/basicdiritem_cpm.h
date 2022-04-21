/// @file basicdiritem_cpm.h
///
/// @brief disk basic directory item for CP/M
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_CPM_H_
#define _BASICDIRITEM_CPM_H_

#include "basicdiritem.h"


#define SECTOR_UNIT_CPM		128

/// CP/M属性名
extern const char *gTypeNameCPM[];
enum en_type_name_cpm {
	TYPE_NAME_CPM_READ_ONLY = 0,
	TYPE_NAME_CPM_SYSTEM = 1,
	TYPE_NAME_CPM_ARCHIVE = 2,
};

class DiskBasicDirItemCPM;

/** @class DiskBasicDirItemCPM

@brief ディレクトリ１アイテム CP/M

@li m_external_attr : バイナリ属性の時:FILE_TYPE_BINARY_MASK  アスキー属性の時:0

*/
class DiskBasicDirItemCPM : public DiskBasicDirItem
{
protected:
	DiskBasicDirItemCPM() : DiskBasicDirItem() {}
	DiskBasicDirItemCPM(const DiskBasicDirItemCPM &src) : DiskBasicDirItem(src) {}

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int		GetFileType1Pos() const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int		GetFileType2Pos() const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief ファイル名を得る
	void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;
	/// @brief 拡張子を返す
	wxString GetFileExtPlainStr() const;
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief 拡張子を設定
	virtual void	SetNativeExt(wxUint8 *fileext, size_t size, size_t length);

	int group_width;	///< グループ番号の幅(1 = 8ビット, 2 = 16ビット)
	int group_entries;	///< グループ番号のエントリ数(8 or 16)

	DiskBasicDirItemCPM *next_item;	///< 次のエクステントがある場合

	/// @brief 拡張子からバイナリかどうかを判断する
	virtual int	GetFileTypeByExt(int val, const wxString &ext) const;

	/// @brief 最終セクタのサイズを計算してファイルサイズを返す
	virtual int	RecalcFileSize(DiskBasicGroups &group_items, int occupied_size);

public:
	DiskBasicDirItemCPM(DiskBasic *basic);
	DiskBasicDirItemCPM(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemCPM(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

	/// @brief 削除
	virtual bool	Delete(wxUint8 code);

	/// @brief ダイアログ入力前のファイル名を変換 大文字にする
	virtual void	ConvertToFileNameStr(wxString &filename) const;
	/// @brief ダイアログ入力後のファイル名文字列を変換 大文字にする
	virtual void	ConvertFromFileNameStr(wxString &filename) const;
//	/// @brief ファイル名に設定できない文字を文字列にして返す
//	virtual wxString GetDefaultInvalidateChars() const;
	/// @brief ファイル名は必須（空文字不可）か
	virtual bool	IsFileNameRequired() const { return true; }

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

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;

	/// @brief ファイルの終端コードをチェックする必要があるか
	virtual bool	NeedCheckEofCode();
	/// @brief セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
	virtual int		RecalcFileSizeOnSave(wxInputStream *istream, int file_size);

	/// @brief アイテムを削除できるか
	virtual bool	IsDeletable() const;

	/// @brief グループ番号の幅をセット
	void		SetGroupWidth(int val) { group_width = val; }
	/// @brief グループ番号の幅を返す(1 = 8ビット, 2 = 16ビット)
	int			GetGroupWidth() const { return group_width; }
	/// @brief グループ番号のエントリ数を返す
	int			GetGroupEntries() const { return group_entries; }
	/// @brief グループ番号をセット
	void		SetGroup(int pos, wxUint32 val);
	/// @brief グループ番号を返す
	wxUint32	GetGroup(int pos) const;
	/// @brief エクステント番号を返す
	wxUint8		GetExtentNumber() const;
	/// @brief レコード番号を返す
	wxUint8		GetRecordNumber() const;
	/// @brief ファイルサイズからエクステント番号とレコード番号をセット
	void		CalcExtentAndRecordNumber(int val);
	/// @brief 次のアイテムをセット
	void		SetNextItem(DiskBasicDirItem *val) { next_item = (DiskBasicDirItemCPM *)val; }
	/// @brief 次のアイテムを返す
	DiskBasicDirItemCPM *GetNextItem() { return next_item; }
	/// @brief アイテムソート用
	static int  Compare(DiskBasicDirItem **item1, DiskBasicDirItem **item2);
	/// @brief 名前比較
	static int  CompareName(DiskBasicDirItem **item1, DiskBasicDirItem **item2);

	/// @brief ファイル名から属性を決定する
	int			ConvFileTypeFromFileName(const wxString &filename) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	virtual void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	//@}
};

#endif /* _BASICDIRITEM_CPM_H_ */
