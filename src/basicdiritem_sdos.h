/// @file basicdiritem_sdos.h
///
/// @brief disk basic directory item for S-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_SDOS_H_
#define _BASICDIRITEM_SDOS_H_

#include "basicdiritem.h"


// S-DOS

extern const char *gTypeNameSDOS_1[];
enum en_type_name_sdos_1 {
	TYPE_NAME_SDOS_BAS1 = 0,
	TYPE_NAME_SDOS_BAS2,
	TYPE_NAME_SDOS_DAT,
	TYPE_NAME_SDOS_OBJ,
	TYPE_NAME_SDOS_UNKNOWN,
};
enum en_file_type_sdos {
	FILETYPE_SDOS_BAS1 = 0x00,	// N mode
	FILETYPE_SDOS_BAS2 = 0x01,	// n88 mode (Sn88-DOS only)
	FILETYPE_SDOS_DAT = 0x02,	// except exec address
	FILETYPE_SDOS_OBJ = 0x0e,	// include exec address
	FILETYPE_SDOS_UNKNOWN = 0xff,
};

struct st_sdos_sdata {
	directory_sdos_t *data;
	int size;
	int pos;
};

enum en_directory_sdos_pos {
	DIR_SDOS_POS_NAME = 0,
	DIR_SDOS_POS_TYPE = 22,
	DIR_SDOS_POS_TRACK = 23,
	DIR_SDOS_POS_SECTOR = 24,
	DIR_SDOS_POS_SIZE = 25,
	DIR_SDOS_POS_REST_SIZE = 26,
	DIR_SDOS_POS_LOAD_ADDR = 27,
	DIR_SDOS_POS_EXEC_ADDR = 29,
	DIR_SDOS_POS_RESERVED = 31,
};

/// ディレクトリ１アイテム Magical DOS
class DiskBasicDirItemSDOS : public DiskBasicDirItem
{
private:
	DiskBasicDirItemSDOS() : DiskBasicDirItem() {}
	DiskBasicDirItemSDOS(const DiskBasicDirItemSDOS &src) : DiskBasicDirItem(src) {}

	struct st_sdos_sdata sdata[2];	///< セクタ内部へのポインタ

	/// @brief ディレクトリエントリを確保
	bool	AllocateItem();

	/// @brief ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 属性１を返す
	int		GetFileType1() const;
	/// @brief 属性１のセット
	void	SetFileType1(int val);
	/// @brief 使用しているアイテムか
	bool	CheckUsed(bool unuse);

	/// @brief グループ数を返す
	wxUint32 GetGroupSize() const;

	/// @brief ファイル名を設定
	void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名と拡張子を得る
	void	GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const;

	/// @brief グループを追加する
	void	AddGroups(wxUint32 group_num, wxUint32 next_group, DiskBasicGroups &group_items);

	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType1Pos() const;
	/// @brief 属性からリストの位置を返す(プロパティダイアログ用)
	int	    GetFileType2Pos() const;
//	/// @brief リストの位置から属性を返す(プロパティダイアログ用)
//	int		CalcFileTypeFromPos(int pos);
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void	SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief 未使用領域の設定
	void	SetUnknownData();

public:
	DiskBasicDirItemSDOS(DiskBasic *basic);
	DiskBasicDirItemSDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data);
	DiskBasicDirItemSDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// @brief アイテムへのポインタを設定
	void	SetDataPtr(int n_num, int n_track, int n_side, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);

	/// @brief ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// @brief 削除
	bool	Delete(wxUint8 code);
//	/// @brief ENDマークがあるか(一度も使用していないか)
//	bool	HasEndMark();
	/// @brief 次のアイテムにENDマークを入れる
	void	SetEndMark(DiskBasicDirItem *next_item);

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

	/// @brief アイテムがアドレスを持っているか
	bool	HasAddress() const { return true; }
	/// @brief 開始アドレスを返す
	int		GetStartAddress() const;
	/// @brief 実行アドレスを返す
	int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	void	SetStartAddress(int val);
	/// @brief 実行アドレスをセット
	void	SetExecuteAddress(int val);

	/// @brief ディレクトリアイテムのサイズ
	size_t	GetDataSize() const;

	/// @brief ファイルの終端コードをチェックする必要があるか
	bool	NeedCheckEofCode();

	/// @brief アイテムをコピー
	bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	void	ClearData();

	/// @brief データをインポートする前に必要な処理
	bool	PreImportDataFile(wxString &filename);
	/// @brief ファイル名から属性を決定する
	int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 属性を変更した際に呼ばれるコールバック
	void	ChangeTypeInAttrDialog(IntNameBox *parent);
	/// @brief 機種依存の属性を設定する
	bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	//@}
};

#endif /* _BASICDIRITEM_SDOS_H_ */
