/// @file basicdiritem_s1os.h
///
/// @brief disk basic directory item for S1-OS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICDIRITEM_S1OS_H
#define BASICDIRITEM_S1OS_H

#include "basicdiritem.h"


//////////////////////////////////////////////////////////////////////

enum enTypeNameS1OS_1 {
	TYPE_NAME_S1OS_1_N = 0,
	TYPE_NAME_S1OS_1_D,
	TYPE_NAME_S1OS_1_R,
	TYPE_NAME_S1OS_1_END
};

enum enTypeNameS1OS_2 {
	TYPE_NAME_S1OS_2_N = 0,
	TYPE_NAME_S1OS_2_D,
	TYPE_NAME_S1OS_2_E,
	TYPE_NAME_S1OS_2_END
};

enum enTypeNameS1OS_3 {
	TYPE_NAME_S1OS_3_L = 0,
	TYPE_NAME_S1OS_3_END
};

/// @brief S1-OS 属性
enum en_file_type_s1os {
	FILETYPE_S1OS_N = 'n',	// normal file ?
	FILETYPE_S1OS_D = 'd',	// directory / destroyed
	FILETYPE_S1OS_E = 'e',	// erased
	FILETYPE_S1OS_L = 'l',	// ?
	FILETYPE_S1OS_R = 'r',	// record
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム S1-OS
class DiskBasicDirItemS1OS : public DiskBasicDirItem
{
private:
	DiskBasicDirItemS1OS() : DiskBasicDirItem() {}
	DiskBasicDirItemS1OS(const DiskBasicDirItemS1OS &src) : DiskBasicDirItem(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_s1os_t> m_data;

	DirItemSectorBoundary m_sdata;	///< セクタ内部へのポインタ

	/// @brief ディレクトリエントリを確保
	bool			AllocateItem(const SectorParam *next);

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
	/// @brief 属性１を返す
	virtual int		GetFileType1() const;
	/// @brief 属性２を返す
	virtual int		GetFileType2() const;
	/// @brief 属性３を返す
	virtual int		GetFileType3() const;
	/// @brief 属性１のセット
	virtual void	SetFileType1(int val);
	/// @brief 属性２のセット
	virtual void	SetFileType2(int val);
	/// @brief 属性３のセット
	virtual void	SetFileType3(int val);
	/// @brief 使用しているアイテムか
	virtual bool	CheckUsed(bool unuse);

	/// @brief 属性1を変換
	int				ConvToNativeType1(int file_type) const;
	/// @brief 属性2を変換
	int				ConvToNativeType2(int file_type) const;
	/// @brief 属性1を得る
	int				GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief 属性2を得る
	int				GetFileType2InAttrDialog(const IntNameBox *parent) const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

//	/// @brief データ内にファイルサイズをセット
//	void			SetFileSizeBase(int val);
//	/// @brief データ内のファイルサイズを返す
//	int				GetFileSizeBase() const;
//	/// @brief グループ取得計算中処理
//	void			CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data);

public:
	DiskBasicDirItemS1OS(DiskBasic *basic);
	DiskBasicDirItemS1OS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemS1OS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief 初期状態に戻す
	virtual void	Reset();
	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

	/// @brief 削除
	bool			Delete();

	/// @brief ファイル名に付随する拡張属性を返す
	virtual int		GetOptionalName() const;

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
	/// @brief ファイルサイズを内部形式に変換
	static void ConvFromFileSize(DiskBasic *basic, int file_size, s1os_size_t *s1os_size);
	/// @brief 内部形式のファイルサイズを変換
	static int	ConvToFileSize(DiskBasic *basic, const s1os_size_t *s1os_size);

	/// @brief 指定ディレクトリのすべてのグループを取得
	virtual void	GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief トラック、サイド、セクタ番号を取得
	static void GetCHR(DiskBasic *basic, const s1os_ptr_t *ptr, int &trk_num, int &sid_num, int &sec_num);
	/// @brief トラック、サイド、セクタ番号をセット
	static void SetCHR(DiskBasic *basic, int trk_num, int sid_num, int sec_num, s1os_ptr_t *ptr);

	/// @brief アイテムが日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	virtual bool	HasCreateDate() const { return true; }
	virtual bool	HasCreateTime() const { return true; }
	/// @brief アイテムの時間設定を無視することができるか
	virtual enDateTime CanIgnoreDateTime() const { return DATETIME_ALL; }
	/// @brief 日付を返す
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 時間を返す
	virtual void	GetFileCreateTime(TM &tm) const;
	/// @brief 日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 時間を返す
	virtual wxString GetFileCreateTimeStr() const;
	/// @brief 日付時間をセット
	virtual void	SetFileCreateDateTime(const TM &tm);
	/// 内部形式の日付時間を変換
	static void		ConvFromFileDateTime(DiskBasic *basic, wxUint32 date_time, TM &tm);
	/// 日付時間を内部形式に変換
	static wxUint32	ConvToFileDateTime(DiskBasic *basic, const TM &tm);

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return false; }

	/// @brief ディレクトリアイテムのサイズ
	virtual size_t	GetDataSize() const;
	/// @brief アイテムを返す
	virtual directory_t *GetData() const;
	/// @brief アイテムをコピー
	virtual bool	CopyData(const directory_t *val);
	/// @brief ディレクトリをクリア ファイル新規作成時
	virtual void	ClearData();

	/// @brief データをチェインする必要があるか（非連続データか）
	bool			NeedChainInData() const;
	/// @brief データをエクスポートする前に必要な処理
	virtual bool	PreExportDataFile(wxString &filename);
	/// @brief インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
	virtual bool	PreImportDataFile(wxString &filename);

	/// @brief アイテムの属するセクタを変更済みにする
	virtual void	SetModify();

	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	virtual bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxString &filename, wxString &errormsg);

#ifndef USE_CONSOLE
	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief ダイアログの終了アドレスを編集できるか
	virtual bool	IsEndAddressEditableInAttrDialog(IntNameBox *parent) { return false; }
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
#endif /* USE_CONSOLE */
};

#endif /* BASICDIRITEM_S1OS_H */
