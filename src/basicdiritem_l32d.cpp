/// @file basicdiritem_l32d.cpp
///
/// @brief disk basic directory item for L3/S1 BASIC 2D/2HD
///
#include "basicdiritem_l32d.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"

///
///
///
DiskBasicDirItemL32D::DiskBasicDirItemL32D(DiskBasic *basic)
	: DiskBasicDirItemFAT8(basic)
{
}
DiskBasicDirItemL32D::DiskBasicDirItemL32D(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItemFAT8(basic, sector, data)
{
}
DiskBasicDirItemL32D::DiskBasicDirItemL32D(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemFAT8(basic, num, track, side, sector, secpos, data, unuse)
{
	// L3 2D

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemL32D::Check(bool &last)
{
	if (!data) return false;

	bool valid = true;
	directory_l3_2d_t *p = (directory_l3_2d_t *)data;
	if (p->name[0] == 0xff) {
		last = true;
		return valid;
	}
	// 属性に想定外の値がある場合はエラー
	if (p->type2 != 0 && p->type2 != 0xff) {
		valid = false;
	}
	if (p->name[0] == 0xff) {
		last = true;
	}
	return valid;
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemL32D::InvalidateChars()
{
	return wxT("\":()");
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemL32D::GetFileNamePos(size_t &len, bool *invert) const
{
	// L3 2D
	len = sizeof(data->l3_2d.name);
	return data->l3_2d.name;
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemL32D::GetFileExtPos(size_t &len) const
{
	len = sizeof(data->l3_2d.ext);
	return data->l3_2d.ext;
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemL32D::GetFileNameSize(bool *invert) const
{
	return (int)sizeof(data->l3_2d.name);
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemL32D::GetFileExtSize(bool *invert) const
{
	return (int)sizeof(data->l3_2d.ext);
}

/// 属性１を返す
int	DiskBasicDirItemL32D::GetFileType1() const
{
	return data->l3_2d.type;
}

/// 属性２を返す
int	DiskBasicDirItemL32D::GetFileType2() const
{
	return data->l3_2d.type2;
}

/// 属性１を設定
void DiskBasicDirItemL32D::SetFileType1(int val)
{
	data->l3_2d.type = val & 0xff;
}

/// 属性２を設定
void DiskBasicDirItemL32D::SetFileType2(int val)
{
	data->l3_2d.type2 = val & 0xff;
}

/// ディレクトリのサイズ
size_t DiskBasicDirItemL32D::GetDataSize()
{
	return sizeof(directory_l3_2d_t);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemL32D::CalcFileSize()
{
	DiskBasicDirItemFAT8::CalcFileSize();

	// 最終セクタのサイズを足す
	if (used && file_size >= 0) {
		file_size = file_size - basic->GetSectorSize() + (int)this->data->l3_2d.end_bytes.h * 256 + this->data->l3_2d.end_bytes.l;
	}
}

/// 最初のグループ番号を設定
void DiskBasicDirItemL32D::SetStartGroup(wxUint32 val)
{
	// L3/S1 2D/2HD
	data->l3_2d.start_group = (val & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemL32D::GetStartGroup() const
{
	// L3/S1 2D/2HD
	return data->l3_2d.start_group;
}

/// 最終セクタのサイズ(2Dのときのみ有効)
void DiskBasicDirItemL32D::SetDataSizeOnLastSecotr(int val)
{
	// L3/S1 2D/2HD
	data->l3_2d.end_bytes.h = ((val & 0xff00) >> 8);
	data->l3_2d.end_bytes.l = (val & 0x00ff);
}
/// 最終セクタのサイズ(2Dのときのみ有効)
int DiskBasicDirItemL32D::GetDataSizeOnLastSector()
{
	// L3/S1 2D/2HD
	return (int)data->l3_2d.end_bytes.h * 256 + data->l3_2d.end_bytes.l;
}

/// ダイアログ入力前のファイル名を変換 大文字にする
void DiskBasicDirItemL32D::ConvertToFileNameStr(wxString &filename)
{
	filename = filename.Upper();
}

/// 内部ファイル名から変換＆拡張子を付加
/// コピー、このアプリからインポート時のダイアログを出す前
wxString DiskBasicDirItemL32D::RemakeFileName(const wxUint8 *src, size_t srclen)
{
	wxString dst;
	basic->ConvCharsToString(src, srclen, dst);
	dst.Trim(true);

	// 拡張子がないとき
	if (dst.Find('.', true) == wxNOT_FOUND) {
		switch(GetFileType1Pos()) {
		case TYPE_NAME_1_BASIC:
			dst += wxT(".BAS");
			break;
		case TYPE_NAME_1_DATA:
			dst += wxT(".DAT");
			break;
		case TYPE_NAME_1_MACHINE:
			dst += wxT(".BIN");
			break;
		}
	}

	return dst;
}

/// 拡張子を追加
wxString DiskBasicDirItemL32D::AddExtensionForAttrDialog(int file_type_1, const wxString &name)
{
	// L3/S1 BASIC
	// 拡張子を自動で付加する
	wxString newname;

	int len = (int)name.Length();
	wxString ext = name.Right(4).Upper();
	bool match = (ext == wxT(".BAS") || ext == wxT(".DAT") || ext == wxT(".BIN"));
	if (match) {
		len -= 4;
		if (len >= 0) newname = name.Left(len);
		else newname.Empty();
	} else {
		len = name.Find('.', false);
		if (len >= 0) {
			return name;
		} else {
			newname = name;
		}
	}
	switch(file_type_1) {
	case 1:
		newname += wxT(".DAT");
		break;
	case 2:
		newname += wxT(".BIN");
		break;
	default:
		newname += wxT(".BAS");
		break;
	}
	return newname;
}
