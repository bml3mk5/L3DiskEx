/// @file basicdiritem_m68fdos.h
///
/// @brief disk basic directory item for Sord M68 FDOS (KDOS)
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _BASICDIRITEM_M68_FDOS_H_
#define _BASICDIRITEM_M68_FDOS_H_

#include "basicdiritem_mz_base.h"


//////////////////////////////////////////////////////////////////////

enum enTypeNameM68FDOS {
	TYPE_NAME_M68_FDOS_A = 0,
	TYPE_NAME_M68_FDOS_P,
	TYPE_NAME_M68_FDOS_W,
	TYPE_NAME_M68_FDOS_R,
	TYPE_NAME_M68_FDOS_X,
	TYPE_NAME_M68_FDOS_S,
	TYPE_NAME_M68_FDOS_C,
	TYPE_NAME_M68_FDOS_D,
	TYPE_NAME_M68_FDOS_END
};

/// @brief Sord M68 FDOS (KDOS) 属性
enum en_file_type_m68fdos {
	FILETYPE_M68_FDOS_UNKNOWN   = 0,
	FILETYPE_M68_FDOS_D	= 0x0001,	// ? Device
	FILETYPE_M68_FDOS_C	= 0x0080,	// Continuous
	FILETYPE_M68_FDOS_S	= 0x0100,	// ? System call
	FILETYPE_M68_FDOS_X	= 0x0800,	// ? Execute protect
	FILETYPE_M68_FDOS_R	= 0x1000,	// Read Protect
	FILETYPE_M68_FDOS_W	= 0x2000,	// Write Protect
	FILETYPE_M68_FDOS_P = 0x4000,	// Hidden
	FILETYPE_M68_FDOS_A	= 0x8000,	// ? Special
};

enum enExtNameM68FDOS {
	EXT_NAME_M68_FDOS_SAV = 0,
	EXT_NAME_M68_FDOS_END
};

//////////////////////////////////////////////////////////////////////

/// @brief ディレクトリ１アイテム Sord M68 FDOS (KDOS)
class DiskBasicDirItemM68FDOS : public DiskBasicDirItemMZBase
{
private:
	DiskBasicDirItemM68FDOS() : DiskBasicDirItemMZBase() {}
	DiskBasicDirItemM68FDOS(const DiskBasicDirItemM68FDOS &src) : DiskBasicDirItemMZBase(src) {}

	/// @brief ディレクトリデータ
	DiskBasicDirData<directory_m68fdos_t> m_data;

	/// @brief ファイル名を格納する位置を返す
	virtual wxUint8 *GetFileNamePos(int num, size_t &size, size_t &len) const;
	/// @brief 拡張子を格納する位置を返す
	virtual wxUint8 *GetFileExtPos(size_t &len) const;
	/// @brief ファイル名を設定
	virtual void	SetNativeName(wxUint8 *filename, size_t size, size_t length);
	/// @brief 拡張子を設定
	virtual void	SetNativeExt(wxUint8 *fileext, size_t size, size_t length);
	/// @brief ファイル名を得る
	virtual void	GetNativeName(wxUint8 *filename, size_t size, size_t &length) const;
	/// @brief 拡張子を得る
	virtual void	GetNativeExt(wxUint8 *fileext, size_t size, size_t &length) const;
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

	/// @brief リビジョンを返す
	wxString		GetRevisionStr() const;
	/// @brief リビジョンを返す
	wxUint16		GetRevision() const;
	/// @brief リビジョンをセット
	void			SetRevision(wxUint16 val);

	/// @brief 属性を変換
	int				ConvToNativeType(int file_type) const;
	/// @brief 属性1を得る
	int				GetFileType1InAttrDialog(const IntNameBox *parent) const;
	/// @brief 属性2を得る
	int				GetFileType2InAttrDialog(const IntNameBox *parent) const;
	/// @brief インポート時ダイアログ表示前にファイルの属性を設定
	void			SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2);

	/// @brief データ内にファイルサイズをセット
	void			SetFileSizeBase(int val);
	/// @brief データ内のファイルサイズを返す
	int				GetFileSizeBase() const;
	/// @brief グループ取得計算中処理
	void			CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data);
	/// @brief ファイル名をデコード
	static void		DecodeName(wxUint16 code, wxUint8 *name, size_t size);
	/// @brief ファイル名をエンコード
	static wxUint16	EncodeName(const wxUint8 *name, size_t size);

public:
	DiskBasicDirItemM68FDOS(DiskBasic *basic);
	DiskBasicDirItemM68FDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data);
	DiskBasicDirItemM68FDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse);

	/// @brief アイテムへのポインタを設定
	virtual void	SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next = NULL);;

	/// @brief ディレクトリアイテムのチェック
	virtual bool	Check(bool &last);

	/// @brief 削除
	bool			Delete();

	/// @brief 属性を設定
	virtual void	SetFileAttr(const DiskBasicFileType &file_type);
	/// @brief 属性を返す
	virtual DiskBasicFileType GetFileAttr() const;

	/// @brief 属性の文字列を返す(ファイル一覧画面表示用)
	virtual wxString GetFileAttrStr() const;

	/// @brief ファイルサイズを返す
	virtual int		GetFileSize() const;
	/// @brief ファイルサイズとグループ数を計算する
	virtual void	CalcFileUnitSize(int fileunit_num);

	/// @brief 最初のグループ番号をセット
	virtual void	SetStartGroup(int fileunit_num, wxUint32 val, int size = 0);
	/// @brief 最初のグループ番号を返す
	virtual wxUint32 GetStartGroup(int fileunit_num) const;
	/// @brief 追加のグループ番号をセット
	virtual void	SetExtraGroup(wxUint32 val);
	/// @brief 追加のグループ番号を返す
	virtual wxUint32 GetExtraGroup() const;
	/// @brief 追加のグループ番号を得る
	virtual void	GetExtraGroups(wxArrayInt &arr) const;

	/// @brief アイテムが日時を持っているか
	virtual bool	HasCreateDateTime() const { return true; }
	virtual bool	HasCreateDate() const { return true; }
	virtual bool	HasCreateTime() const { return false; }
	/// @brief アイテムの時間設定を無視することができるか
	virtual enDateTime CanIgnoreDateTime() const { return DATETIME_ALL; }
	/// @brief 日付を返す
	virtual void	GetFileCreateDate(TM &tm) const;
	/// @brief 日付を返す
	virtual wxString GetFileCreateDateStr() const;
	/// @brief 日付をセット
	virtual void	SetFileCreateDate(const TM &tm);

	/// @brief アイテムがアドレスを持っているか
	virtual bool	HasAddress() const { return true; }
	/// @brief 開始アドレスを返す
	virtual int		GetStartAddress() const;
	/// @brief 終了アドレスを返す
	virtual int		GetEndAddress() const;
	/// @brief 実行アドレスを返す
	virtual int		GetExecuteAddress() const;
	/// @brief 開始アドレスをセット
	virtual void	SetStartAddress(int val);
	/// @brief 実行アドレスをセット
	virtual void	SetExecuteAddress(int val);

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

	/// @brief ファイル名から属性を決定する
	virtual int		ConvOriginalTypeFromFileName(const wxString &filename) const;
	/// @brief 属性値を加工する
	virtual bool	ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;

	/// @name プロパティダイアログ用
	//@{
	/// @brief ダイアログ内の属性部分のレイアウトを作成
	virtual void	CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags);
	/// @brief 機種依存の属性を設定する
	virtual bool	SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const;
	/// @brief ファイルサイズが適正か
	virtual bool	IsFileValidSize(const IntNameBox *parent, int size, int *limit);
	/// @brief ダイアログ入力後のファイル名チェック
	virtual bool	ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg);
	/// @brief ダイアログの終了アドレスを編集できるか
	virtual bool	IsEndAddressEditableInAttrDialog(IntNameBox *parent) { return false; }
	/// @brief プロパティで表示する内部データを設定
	virtual void	SetInternalDataInAttrDialog(KeyValArray &vals);
	//@}
};

#endif /* _BASICDIRITEM_M68_FDOS_H_ */
