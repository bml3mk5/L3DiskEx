/// @file basicdiritem_l32D.h
///
/// @brief disk basic directory item for L3/S1 BASIC 2D/2HD
///
#ifndef _BASICDIRITEM_L32D_H_
#define _BASICDIRITEM_L32D_H_

#include "basicdiritem_fat8.h"

/// ディレクトリ１アイテム L3/S1 BASIC 倍密度 2D/2HD
class DiskBasicDirItemL32D : public DiskBasicDirItemFAT8
{
private:
	DiskBasicDirItemL32D() : DiskBasicDirItemFAT8() {}
	DiskBasicDirItemL32D(const DiskBasicDirItemL32D &src) : DiskBasicDirItemFAT8(src) {}

	/// ファイル名を格納する位置を返す
	wxUint8 *GetFileNamePos(size_t &len, bool *invert = NULL) const;
	/// 拡張子を格納する位置を返す
	wxUint8 *GetFileExtPos(size_t &len) const;
	/// ファイル名を格納するバッファサイズを返す
	int		GetFileNameSize(bool *invert = NULL) const;
	/// 拡張子を格納するバッファサイズを返す
	int		GetFileExtSize(bool *invert = NULL) const;
	/// 属性１を返す
	int		GetFileType1() const;
	/// 属性２を返す
	int		GetFileType2() const;
	/// 属性１のセット
	void	SetFileType1(int val);
	/// 属性２のセット
	void	SetFileType2(int val);

public:
	DiskBasicDirItemL32D(DiskBasic *basic);
	DiskBasicDirItemL32D(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data);
	DiskBasicDirItemL32D(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse);

	/// ディレクトリアイテムのチェック
	bool	Check(bool &last);

	/// ファイル名に設定できない文字を文字列にして返す
	wxString	InvalidateChars();

	/// ファイルサイズとグループ数を計算する
	void	CalcFileSize();
	/// 最初のグループ番号をセット
	void	SetStartGroup(wxUint32 val);
	/// 最初のグループ番号を返す
	wxUint32 GetStartGroup() const;

	/// ディレクトリアイテムのサイズ
	size_t	GetDataSize();

	/// 最終セクタの占有サイズをセット
	void	SetDataSizeOnLastSecotr(int val);
	/// 最終セクタの占有サイズを返す
	int		GetDataSizeOnLastSector();

	/// 内部ファイル名からコード変換して文字列を返す コピー、このアプリからインポート時のダイアログを出す前
	wxString RemakeFileName(const wxUint8 *src, size_t srclen);

	/// ダイアログ入力前のファイル名を変換 大文字にする
	void	ConvertToFileNameStr(wxString &filename);

	/// ファイル名に拡張子を付ける
	wxString AddExtensionForAttrDialog(int file_type_1, const wxString &name);
};

#endif /* _BASICDIRITEM_L32D_H_ */
