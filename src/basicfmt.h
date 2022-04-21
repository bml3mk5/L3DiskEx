/// @file basicfmt.h
///
/// @brief disk basic
///
#ifndef _BASICFMT_H_
#define _BASICFMT_H_

#include "common.h"
#include <wx/string.h>
#include <wx/dynarray.h>
//#include <wx/filename.h>
#include "basiccommon.h"
#include "basicparam.h"
#include "basicerror.h"
#include "diskd88.h"
#include "charcodes.h"
#include "result.h"

class DiskBasicFat;
class DiskBasicDir;
class DiskBasicType;
class DiskBasicDirItem;
//class DiskBasicGroupItems;
class DiskBasicGroups;
class AttrControls;

/// DISK BASIC 構造解析
class DiskBasic : public DiskParam, public DiskBasicParam
{
private:
	DiskD88Disk *disk;
	bool formatted;

	DiskBasicFat  *fat;
	DiskBasicDir  *dir;
	DiskBasicType *type;

	int			selected_side;			///< サイド(1S用)

	int			sectors_on_basic;		///< BASICで使用するセクタ数

	int			data_start_sector;		///< グループ計算用 データ開始セクタ番号
	int			skipped_track;			///< グループ計算する際にスキップするトラック番号
//	bool		reverse_side;			///< グループ計算する際にサイド番号を逆転するか
	int			char_code;				///< ファイルなどの文字コード
	CharCodes	codes;

	wxString	desc_size;				///< 空きサイズを文字列に成形したもの

	DiskBasicError errinfo;				///< エラー情報保存用

	/// BASIC種類を設定
	void			CreateType();
	/// 指定のDISK BASICでフォーマットされているかを解析＆チェック
	int				ParseFormattedDisk(DiskD88Disk *newdisk, DiskBasicParam *match, bool is_formatting); 

public:
	DiskBasic();
	~DiskBasic();

	/// @name 解析
	//@{
	/// 指定したディスクがDISK BASICかを解析する
	int				ParseDisk(DiskD88Disk *newdisk, int newside, bool is_formatting);
	/// パラメータをクリア
	void			Clear();

	/// 現在選択しているディスクのFAT領域をアサイン
	bool			AssignFat();

	/// 現在選択しているディスクのルートディレクトリ構造をチェック
	bool			CheckRootDirectory();
	/// 現在選択しているディスクのルートディレクトリをアサイン
	bool			AssignRootDirectory();
	/// 現在選択しているディスクのFATとルートディレクトリをアサイン
	bool			AssignFatAndDirectory();

	//@}
	/// @name 読み出し
	//@{
//	/// 指定したファイル名のファイルをロード
//	bool			LoadFile(const wxString &filename, const wxString &dstpath);
	/// 指定したディレクトリ位置のファイルをロード
	bool			LoadFile(int item_number, const wxString &dstpath);
	/// 指定したディレクトリアイテムのファイルをロード
	bool			LoadFile(DiskBasicDirItem *item, const wxString &dstpath);

	/// 指定したアイテムのファイルをベリファイ
	bool			VerifyFile(DiskBasicDirItem *item, const wxString &srcpath);
	/// ディスクデータにアクセス（ロード/ベリファイで使用）
	bool			AccessData(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream);
	//@}
	/// @name 比較・チェック
	//@{
	/// 同じファイル名のアイテムをさがす
	DiskBasicDirItem *FindFile(const wxString &filename, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item);

	/// 指定ファイルのサイズをチェック
	bool			CheckFile(const wxString &srcpath, int *file_size);
	/// 指定ファイルのサイズをチェック
	bool			CheckFile(const wxUint8 *buffer, size_t buflen);

	/// DISK BASICで使用できる残りディスクサイズ
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
	bool			SaveFile(const wxString &srcpath, const DiskBasicDirItem *pitem, DiskBasicDirItem **nitem = NULL);
	/// バッファデータをディスクイメージにセーブ
	bool			SaveFile(const wxUint8 *buffer, size_t buflen, const DiskBasicDirItem *pitem, DiskBasicDirItem **nitem = NULL);
	/// ストリームデータをディスクイメージにセーブ
	bool			SaveFile(wxInputStream &istream, const DiskBasicDirItem *pitem, DiskBasicDirItem **nitem = NULL);
	//@}
	/// @name 削除
	//@{
	/// ファイルを削除できるか
	bool			IsDeletableFiles();
	/// 指定したファイルを削除できるか
	bool			IsDeletableFile(DiskBasicDirItem *item, DiskBasicGroups &group_items, bool clearmsg = true);
	/// ファイルを削除
	bool			DeleteItem(DiskBasicDirItem *item, bool clearmsg = true);
	/// ファイルを削除
	bool			DeleteItem(DiskBasicDirItem *item, const DiskBasicGroups &group_items);
	//@}
	/// @name 属性変更
	//@{
	/// ファイル名や属性を更新できるか
	bool			CanRenameFile(DiskBasicDirItem *item);
	/// ファイル名を更新
	bool			RenameFile(DiskBasicDirItem *item, const wxString &newname);
	/// 属性を更新
	bool			ChangeAttr(DiskBasicDirItem *item, int start_addr, int exec_addr, const struct tm *tm);
	//@}
	/// @name フォーマット
	//@{
	/// DISK BASIC用にフォーマットされているか
	bool			IsFormatted() const;
	/// DISK BASIC用にフォーマットできるか
	bool			IsFormattable();
	/// ディスクを論理フォーマット
	bool			FormatDisk();
	//@}
	/// @name ディレクトリ操作
	//@{
	/// ディレクトリを変更
	bool			ChangeDirectory(DiskBasicDirItem *item);
	/// サブディレクトリの作成できるか
	bool			CanMakeDirectory() const;
	/// サブディレクトリの作成
	bool			MakeDirectory(const wxString &filename, DiskBasicDirItem **nitem = NULL);
	//@}
	/// @name ディレクトリアイテム操作
	//@{
	/// ディレクトリアイテムの作成 使用後はdeleteすること
	DiskBasicDirItem *CreateDirItem();
	/// ディレクトリアイテムの作成 使用後はdeleteすること
	DiskBasicDirItem *CreateDirItem(DiskD88Sector *sector, wxUint8 *data);
	/// ディレクトリのアイテムを取得
	DiskBasicDirItem *GetDirItem(size_t pos);
	/// ディレクトリアイテムの位置から開始セクタを返す
	DiskD88Sector	*GetSectorFromPosition(size_t position, wxUint32 *start_group);
	/// ディレクトリアイテムの位置から属している全グループを返す
	bool			GetGroupsFromPosition(size_t position, DiskBasicGroups &group_items);
	//@}
	/// @name グループ番号
	//@{
	/// グループ番号から開始セクタを返す
	DiskD88Sector	*GetSectorFromGroup(wxUint32 group_num);
	/// グループ番号から開始セクタを返す
	DiskD88Sector	*GetSectorFromGroup(wxUint32 group_num, int &track_num, int &side_num);
//	/// 指定ディレクトリのすべてのグループを取得
//	void			GetAllGroups(DiskBasicDirItem *item, DiskBasicGroups &group_items);
	/// グループ番号からトラック番号、サイド番号、セクタ番号を計算してリストに入れる
	bool			GetNumsFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_size, int remain_size, DiskBasicGroups &items, int *end_sector = NULL);
	/// グループ番号からトラック、サイド、セクタの各番号を計算(グループ計算用)
	bool			CalcStartNumFromGroupNum(wxUint32 group_num, int &track_start, int &side_start, int &sector_start);
	/// セクタ位置(トラック0,サイド0のセクタを0とした位置)からトラック、サイド、セクタの各番号を計算(グループ計算用)
	void			CalcNumFromSectorPosForGroup(int sector_pos, int &track_num, int &side_num, int &sector_num);
	/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0のセクタを0とした位置)を計算(グループ計算用)
	int				CalcSectorPosFromNumForGroup(int track_num, int side_num, int sector_num);
	//@}
	/// @name ユーティリティ
	//@{
	/// 管理エリアのサイド番号、セクタ番号、トラックを得る
	DiskD88Track	*GetManagedTrack(int sector_pos, int *side_num = NULL, int *sector_num = NULL);
	/// 管理エリアのトラック番号、サイド番号、セクタ番号、セクタポインタを得る
	DiskD88Sector	*GetManagedSector(int sector_pos, int *track_num = NULL, int *side_num = NULL, int *sector_num = NULL);

	/// トラックを返す
	DiskD88Track	*GetTrack(int track_num, int side_num);
	/// セクタ返す
	DiskD88Sector	*GetSector(int track_num, int side_num, int sector_num);
	/// セクタ返す
	DiskD88Sector	*GetSector(int track_num, int sector_num, int *side_num = NULL);

	/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラックを返す
	DiskD88Track	*GetTrackFromSectorPos(int sector_pos, int &sector_num);
	/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からセクタを返す
	DiskD88Sector	*GetSectorFromSectorPos(int sector_pos);

	/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
	void			GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num);
	/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
	void			GetNumFromSectorPos(int sector_pos, int &track_num, int &sector_num);
	/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	int				GetSectorPosFromNum(int track, int side, int sector_num);
	/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
	int				GetSectorPosFromNum(int track, int sector_num);
	//@}
	/// @name キャラクターコード
	//@{
	/// キャラクターコードの文字体系を設定
	void			SetCharCode(int val);
	/// 現在のキャラクターコードの文字体系を返す
	int				GetCharCode() const { return char_code; }
	/// キャラクターコードの文字体系
	CharCodes		&GetCharCodes() { return codes; }

	/// 文字列をバイト列に変換 文字コードは機種依存
	bool			ConvStringToChars(const wxString &src, wxUint8 *dst, size_t len);
	/// バイト列を文字列に変換 文字コードは機種依存
	void			ConvCharsToString(const wxUint8 *src, size_t len, wxString &dst);
	//@}
	/// @name プロパティ
	//@{
	/// DISK使用可能か
	bool			CanUse() const { return (disk != NULL); }
	/// DISKイメージを返す
	DiskD88Disk		*GetDisk() const { return disk; }
	/// 選択中のサイドを返す
	int				GetSelectedSide() const { return selected_side; }
	/// DISK BASICの説明を取得
	const wxString &GetDescription();
	/// FATクラス
	DiskBasicFat	*GetFat() { return fat; }
	/// DIRクラス
	DiskBasicDir	*GetDir() { return dir; }
	/// TYPEクラス
	DiskBasicType	*GetType() { return type; }

	/// セクタ数/トラックを返す(実際のディスクイメージと異なる場合があるため)
	int				GetSectorsPerTrackOnBasic() const { return sectors_on_basic; }
	/// セクタ数/トラックを設定(実際のディスクイメージと異なる場合があるため)
	void			SetSectorsPerTrackOnBasic(int val) { sectors_on_basic = val; }

//	/// ディスク内のデータが反転しているか
//	bool			IsDataInverted() const;
	/// 必要ならデータを反転する
	wxUint8			InvertUint8(wxUint8 val) const;
	/// 必要ならデータを反転する
	wxUint16		InvertUint16(wxUint16 val) const;
	/// 必要ならデータを反転する
	wxUint32		InvertUint32(wxUint32 val) const;
	/// 必要ならデータを反転する
	void			InvertMem(void *val, size_t len) const;
	/// 必要ならデータを反転する
	void			InvertMem(const wxUint8 *src, size_t len, wxUint8 *dst) const;
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
	//@}
};

#endif /* _BASICFMT_H_ */
