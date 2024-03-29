/// @file basicfmt.h
///
/// @brief disk basic
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICFMT_H
#define BASICFMT_H

#include "../common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
//#include <wx/filename.h>
#include "basiccommon.h"
#include "basicparam.h"
#include "basicerror.h"
#include "../charcodes.h"
#include "../result.h"


class wxInputStream;
class wxOutputStream;
class DiskImageSector;
class DiskImageTrack;
class DiskImageDisk;
class DiskBasic;
class DiskBasicFat;
class DiskBasicDir;
class DiskBasicType;
class DiskBasicDirItem;
class DiskBasicDirItems;
class DiskBasicDirItemAttr;
class DiskBasicGroups;
class AttrControls;

//////////////////////////////////////////////////////////////////////

/// フォーマット時やダイアログ表示で渡す値
class DiskBasicIdentifiedData
{
private:
	wxString m_volume_name;			///< ボリューム名
	size_t	 m_volume_name_maxlen;	///< ボリューム名最大長
	int		 m_volume_number;		///< ボリューム番号
	bool	 m_volume_number_hexa;	///< ボリューム番号が16進か
	wxString m_volume_date;			///< ボリューム日付

public:
	DiskBasicIdentifiedData();
	DiskBasicIdentifiedData(const wxString &volume_name, int volume_number, const wxString &volume_date);
	~DiskBasicIdentifiedData() {}

	const wxString &GetVolumeName() const { return m_volume_name; }
	size_t GetVolumeNameMaxLength() const { return m_volume_name_maxlen; }
	int GetVolumeNumber() const { return m_volume_number; }
	bool IsVolumeNumberHexa() const { return m_volume_number_hexa; }
	const wxString &GetVolumeDate() const { return m_volume_date; }

	void SetVolumeName(const wxString &val) { m_volume_name = val; }
	void SetVolumeNameMaxLength(size_t val) { m_volume_name_maxlen = val; }
	void SetVolumeNumber(int val) { m_volume_number = val; }
	void VolumeNumberIsHexa(bool val) { m_volume_number_hexa = val; }
	void SetVolumeDate(const wxString &val) { m_volume_date = val; }
};

//////////////////////////////////////////////////////////////////////

class ArrayOfDiskBasic;

/// DISK BASIC ディスク毎のリスト
class DiskBasics
{
private:
	ArrayOfDiskBasic *basics;

	DiskBasics(const DiskBasics &src) {}
	DiskBasics operator=(const DiskBasics &src) { return *this; }

public:
	DiskBasics();
	~DiskBasics();

	void		Add(DiskBasic *newitem = NULL);
	void		Clear();
	void		Empty();
	DiskBasic	*Item(size_t idx);
	size_t		Count();
	void		RemoveAt(size_t idx);
	void		ClearParseAndAssign(int idx = -1);
};

#ifdef DeleteFile
#undef DeleteFile
#endif
#ifdef GetCurrentDirectory
#undef GetCurrentDirectory
#endif

//////////////////////////////////////////////////////////////////////

/// DISK BASIC 構造解析
class DiskBasic : public DiskParam, public DiskBasicParam
{
private:
	DiskImageDisk *disk;
	bool formatted;
	bool parsed;
	bool assigned;

	DiskBasicFat  *fat;
	DiskBasicDir  *dir;
	DiskBasicType *type;

	int			selected_side;			///< サイド(1S用)

	int			data_start_sector;		///< グループ計算用 データ開始セクタ番号
	int			skipped_track;			///< グループ計算する際にスキップするトラック番号

	wxString	char_code;				///< ファイルなどの文字コード体系
	CharCodes	codes;					///< 文字コード変換

	DiskBasicError errinfo;				///< エラー情報保存用

	/// BASIC種類を設定
	void			CreateType();
	/// 指定のDISK BASICでフォーマットされているかを解析＆チェック
	double			ParseFormattedDisk(DiskImageDisk *newdisk, const DiskBasicParam *match, bool is_formatting); 

	int				MaxRatio(wxArrayDouble &values);

public:
	DiskBasic();
	~DiskBasic();

	/// @name 解析
	//@{
	/// 指定したディスクがDISK BASICかを解析する
	int				ParseBasic(DiskImageDisk *newdisk, int newside, const DiskBasicParam *match, bool is_formatting);
	/// パラメータをクリア
	void			Clear();

	/// 固有のパラメータをセット
	void			AssignParameter();
	/// 現在選択しているディスクのFAT領域をアサイン
	double			AssignFat(bool is_formatting);

	/// 現在選択しているディスクのルートディレクトリ構造をチェック
	double			CheckRootDirectory(bool is_formatting);
	/// 現在選択しているディスクのルートディレクトリをアサイン
	bool			AssignRootDirectory();
	/// 現在選択しているディスクのFATとルートディレクトリをアサイン
	bool			AssignFatAndDirectory();
	/// 解析済みをクリア
	void			ClearParseAndAssign();
	/// 解析済みか
	bool			IsParsed() const { return parsed; }
	/// アサイン済みか
	bool			IsAssigned() const { return assigned; }
	//@}
	/// @name 読み出し
	//@{
	/// ロードできるか
	bool			IsLoadableFile(DiskBasicDirItem *item);
	/// 指定したディレクトリ位置のファイルをロード
	bool			LoadFile(int item_number, const wxString &dstpath);
	/// 指定したディレクトリアイテムのファイルをロード
	bool			LoadFile(DiskBasicDirItem *item, const wxString &dstpath);
	/// 指定したストリームにファイルをロード
	bool			LoadFile(DiskBasicDirItem *item, wxOutputStream &ostream);

	/// 指定したアイテムのファイルをベリファイ
	int				VerifyFile(DiskBasicDirItem *item, const wxString &srcpath);

	/// 指定したストリームにファイルをロード
	bool			LoadData(DiskBasicDirItem *item, wxOutputStream &ostream, size_t *outsize = NULL);
	/// 指定したアイテムのファイルをベリファイ
	int				VerifyData(DiskBasicDirItem *item, wxInputStream &istream);
	/// ディスクデータにアクセス（ロード/ベリファイで使用）
	int				AccessUnitData(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, size_t *outsize = NULL);
	//@}
	/// @name 比較・チェック
	//@{
	/// 同じファイル名が既に存在して上書き可能か
	int				IsFileNameDuplicated(const DiskBasicDirItem *dir_item, const DiskBasicFileName &filename, DiskBasicDirItem *exclude_item = NULL, DiskBasicDirItem **next_item = NULL);
	/// 同じファイル名が既に存在して上書き可能か
	int				IsFileNameDuplicated(const DiskBasicDirItem *dir_item, const DiskBasicDirItem *target_item, DiskBasicDirItem *exclude_item = NULL, DiskBasicDirItem **next_item = NULL);

	/// 指定ファイルのサイズをチェック
	bool			CheckFile(const wxString &srcpath, int *file_size);

	/// DISK BASICで使用できる残りディスクサイズ内か
	bool			HasFreeDiskSize(int size);
	/// DISK BASICで使用できる残りディスクサイズを返す
	int				GetFreeDiskSize() const;

	/// FATエリアの空き状況を取得
	void			GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const;
	//@}
	/// @name 書き込み
	//@{
	/// 書き込みできるか
	bool			IsWritableIntoDisk();
	/// 指定ファイルをディスクイメージにセーブ
	bool			SaveFile(const wxString &srcpath, DiskBasicDirItem *dir_item, DiskBasicDirItem *pitem, DiskBasicDirItem **nitem = NULL);
	/// バッファデータをディスクイメージにセーブ
	bool			SaveFile(const wxUint8 *buffer, size_t buflen, DiskBasicDirItem *dir_item, DiskBasicDirItem *pitem, DiskBasicDirItem **nitem = NULL);
	/// ストリームデータをディスクイメージにセーブ
	bool			SaveFile(wxInputStream &istream, DiskBasicDirItem *dir_item, DiskBasicDirItem *pitem, DiskBasicDirItem **nitem = NULL);
	/// ストリームデータをディスクイメージにセーブ
	bool			SaveData(wxInputStream &istream, DiskBasicDirItem *pitem, DiskBasicDirItem *item, DiskBasicGroups &group_items, int &file_size);
	/// ストリームデータをディスクイメージにセーブ
	bool			SaveUnitData(int fileunit_num, wxInputStream &istream, int isize, DiskBasicDirItem *pitem, DiskBasicDirItem *item, DiskBasicGroups &group_items, int &file_size);
	//@}
	/// @name 削除
	//@{
	/// ファイルを削除できるか
	bool			IsDeletableFiles();
	/// 指定したファイルを削除できるか
	int				IsDeletableFile(DiskBasicDirItem *item, bool clearmsg = true);
	/// 指定したディレクトリが空か
	bool			IsEmptyDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, bool clearmsg = true);
	/// ファイルを削除
	bool			DeleteFile(DiskBasicDirItem *item, bool clearmsg = true);
	/// ファイルを削除
	bool			DeleteFile(DiskBasicDirItem *item, const DiskBasicGroups &group_items);
	//@}
	/// @name 属性変更
	//@{
	/// ファイル名や属性を更新できるか
	bool			CanRenameFile(DiskBasicDirItem *item, bool showmsg = true);
	/// ファイル名を更新
	bool			RenameFile(DiskBasicDirItem *item, const wxString &newname);
	/// 属性を更新
	bool			ChangeAttr(DiskBasicDirItem *item, DiskBasicDirItemAttr &attr);
	//@}
	/// @name フォーマット
	//@{
	/// DISK BASIC用にフォーマットされているか
	bool			IsFormatted() const;
	/// DISK BASIC用にフォーマットできるか
	bool			IsFormattable();
	/// ディスクを論理フォーマット
	int				FormatDisk(const DiskBasicIdentifiedData &data);
	//@}
	/// @name ディレクトリ操作
	//@{
	/// ルートディレクトリを返す
	DiskBasicDirItem *GetRootDirectory();
	/// ルートディレクトリ内の一覧を返す
	DiskBasicDirItems *GetRootDirectoryItems(DiskBasicDirItem **dir_item = NULL);
	/// カレントディレクトリを返す
	DiskBasicDirItem *GetCurrentDirectory();
	/// カレントディレクトリ内の一覧を返す
	DiskBasicDirItems *GetCurrentDirectoryItems(DiskBasicDirItem **dir_item = NULL);
	/// ディレクトリをアサイン
	bool			AssignDirectory(DiskBasicDirItem *dir_item);
	/// ディレクトリを読み直す
	bool			ReassignDirectory(DiskBasicDirItem *dir_item);
	/// ディレクトリを変更
	bool			ChangeDirectory(DiskBasicDirItem * &dst_item);
	/// サブディレクトリの作成できるか
	bool			CanMakeDirectory() const;
	/// サブディレクトリの作成
	int				MakeDirectory(DiskBasicDirItem *dir_item, const wxString &filename, bool ignore_datetime, DiskBasicDirItem **nitem = NULL);
	/// ディレクトリのサイズを拡張
	bool			ExpandDirectory(DiskBasicDirItem *dir_item);
	//@}
	/// @name ディレクトリアイテム操作
	//@{
	/// ディレクトリアイテムの作成 使用後はdeleteすること
	DiskBasicDirItem *CreateDirItem();
	/// ディレクトリアイテムの作成 使用後はdeleteすること
	DiskBasicDirItem *CreateDirItem(DiskImageSector *sector, int secpos, wxUint8 *data);
	/// ディレクトリのアイテムを取得
	DiskBasicDirItem *GetDirItem(size_t pos);
	/// ディレクトリアイテムの位置から開始セクタを返す
	DiskImageSector	*GetSectorFromPosition(size_t position, wxUint32 *start_group);
	//@}
	/// @name グループ番号
	//@{
	/// グループ番号から開始セクタを返す
	DiskImageSector	*GetSectorFromGroup(wxUint32 group_num, int *div_num = NULL, int *div_nums = NULL);
	/// グループ番号から開始セクタを返す
	DiskImageSector	*GetSectorFromGroup(wxUint32 group_num, int &track_num, int &side_num, int *div_num = NULL, int *div_nums = NULL);
	/// グループ番号からトラック番号、サイド番号、セクタ番号を計算してリストに入れる
	bool			GetNumsFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_size, int remain_size, DiskBasicGroups &items, int *end_sector = NULL);
	/// グループ番号からトラック、サイド、セクタの各番号を計算(グループ計算用)
	bool			CalcStartNumFromGroupNum(wxUint32 group_num, int &track_start, int &side_start, int &sector_start, int *div_num = NULL, int *div_nums = NULL);
	/// セクタ位置(トラック0,サイド0のセクタを0とした位置)からトラック、サイド、セクタの各番号を計算(グループ計算用)
	void			CalcNumFromSectorPosForGroup(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
	/// セクタ位置(トラック0のセクタ1を0とした位置)からトラック、セクタの各番号を計算(グループ計算用)
	void			CalcNumFromSectorPosTForGroup(int sector_pos, int &track_num, int &sector_num);
	/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0のセクタを0とした位置)を計算(グループ計算用)
	int				CalcSectorPosFromNumForGroup(int track_num, int side_num, int sector_num, int div_num = 0, int div_nums = 1);
	/// トラック、セクタの各番号からセクタ位置(トラック0のセクタ1を0とした位置)を計算(グループ計算用)
	int				CalcSectorPosFromNumTForGroup(int track_num, int sector_num);
	//@}
	/// @name ユーティリティ
	//@{
	/// 管理エリアのサイド番号、セクタ番号、トラックを得る
	DiskImageTrack	*GetManagedTrack(int sector_pos, int *side_num = NULL, int *sector_num = NULL, int *div_num = NULL, int *div_nums = NULL);
	/// 管理エリアのトラック番号、サイド番号、セクタ番号、セクタポインタを得る
	DiskImageSector	*GetManagedSector(int sector_pos, int *track_num = NULL, int *side_num = NULL, int *sector_num = NULL, int *div_num = NULL, int *div_nums = NULL);

	/// トラックを返す
	DiskImageTrack	*GetTrack(int track_num, int side_num);
	/// セクタ返す
	DiskImageSector	*GetSector(int track_num, int side_num, int sector_num);
	/// セクタ返す
	DiskImageSector	*GetSector(int track_num, int sector_num, int *side_num = NULL);

	/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラックを返す
	DiskImageTrack	*GetTrackFromSectorPos(int sector_pos, int &sector_num, int *div_num = NULL, int *div_nums = NULL);
	/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からセクタを返す
	DiskImageSector	*GetSectorFromSectorPos(int sector_pos, int &track_num, int &side_num, int *div_num = NULL, int *div_nums = NULL);
	/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からセクタを返す
	DiskImageSector	*GetSectorFromSectorPos(int sector_pos, int *div_num = NULL, int *div_nums = NULL);

	/// 開始セクタ番号を返す
	int				GetSectorNumberBase() const;
	//@}
	/// @name キャラクターコード
	//@{
	/// キャラクターコードの文字体系を設定
	void			SetCharCode(const wxString &name);
	/// 現在のキャラクターコードの文字体系を返す
	const wxString &GetCharCode() const { return char_code; }
	/// キャラクターコードの文字体系
	CharCodes		&GetCharCodes() { return codes; }
	//@}
	/// @name プロパティ
	//@{
	/// DISK使用可能か
	bool			CanUse() const { return (disk != NULL); }
	/// DISKイメージを返す
	DiskImageDisk	*GetDisk() const { return disk; }
	/// DISKイメージの番号を返す
	int				GetDiskNumber() const;
	/// 選択中のサイドを設定
	void			SetSelectedSide(int val) { selected_side = val; }
	/// 選択中のサイドを返す
	int				GetSelectedSide() const { return selected_side; }
	/// 選択中のサイド文字列を返す
	wxString		GetSelectedSideStr() const;
	/// DISK BASICの説明を取得
	wxString		GetDescriptionDetails() const;
	/// FATクラス
	DiskBasicFat	*GetFat() { return fat; }
	/// DIRクラス
	DiskBasicDir	*GetDir() { return dir; }
	/// TYPEクラス
	DiskBasicType	*GetType() { return type; }

	/// 必要ならデータを反転する
	wxUint8			InvertUint8(wxUint8 val) const;
	/// 必要ならデータを反転する
	wxUint16		InvertUint16(wxUint16 val) const;
	/// 必要ならデータを反転する＆エンディアンを考慮
	wxUint16		InvertAndOrderUint16(wxUint16 val) const;
	/// 必要ならデータを反転する
	wxUint32		InvertUint32(wxUint32 val) const;
	/// 必要ならデータを反転する＆エンディアンを考慮
	wxUint32		InvertAndOrderUint32(wxUint32 val) const;
	/// 必要ならデータを反転する
	void			InvertMem(void *val, size_t len) const;
	/// 必要ならデータを反転する
	void			InvertMem(const wxUint8 *src, size_t len, wxUint8 *dst) const;
	/// エンディアンを考慮した値を返す
	wxUint16		OrderUint16(wxUint16 val) const;
	/// エンディアンを考慮した値を返す
	wxUint32		OrderUint32(wxUint32 val) const;

	/// DISK BASIC種類番号を返す
	DiskBasicFormatType GetFormatTypeNumber() const;
	//@}
	/// @name エラーメッセージ
	//@{
	/// エラーメッセージ
	const wxArrayString &GetErrorMessage(int maxrow = 20);
	/// エラー有無
	int				GetErrorLevel(void) const;
	/// エラー情報
	DiskBasicError	&GetErrinfo() { return errinfo; }
	/// エラーメッセージを表示
	void			ShowErrorMessage();
	/// エラーメッセージをクリア
	void			ClearErrorMessage();
	//@}
};

WX_DEFINE_ARRAY(DiskBasic *, ArrayOfDiskBasic);

#endif /* BASICFMT_H */
