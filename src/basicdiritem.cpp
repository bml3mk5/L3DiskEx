/// @file basicdiritem.cpp
///
/// @brief disk basic directory item
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem.h"
#include <wx/xml/xml.h>
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "config.h"


//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItem::DiskBasicDirItem()
{
	this->basic = NULL;
	this->type = NULL;

	m_parent = NULL;
	m_children = NULL;
	m_valid_dir = false;

	m_num = 0;
	m_position = 0;
//	m_file_size = 0;
	m_sector = NULL;

	m_data = NULL;
	m_ownmake_data = false;

	m_external_attr = 0;

	m_flags = (VISIBLE_LIST | VISIBLE_TREE);
}
DiskBasicDirItem::DiskBasicDirItem(const DiskBasicDirItem &src)
{
#ifdef COPYABLE_DIRITEM
	Dup(src);
#endif
}
DiskBasicDirItem &DiskBasicDirItem::operator=(const DiskBasicDirItem &src)
{
#ifdef COPYABLE_DIRITEM
	Dup(src);
#endif
	return *this;
}
/// ディレクトリアイテムを作成 DATAは内部で確保
/// @param [in] basic  DISK BASIC
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic)
{
	this->basic = basic;
	this->type = basic->GetType();

	m_parent = NULL;
	m_children = NULL;
	m_valid_dir = false;

	m_num = 0;
	m_position = 0;
//	m_file_size = 0;
	m_sector = NULL;

	m_data = new directory_t;
//	memset(m_data, 0, sizeof(directory_t));
//	if (basic->IsDataInverted()) mem_invert(m_data, sizeof(directory_t));
	m_ownmake_data = true;

	m_external_attr = 0;

	m_flags = (VISIBLE_LIST | VISIBLE_TREE);
}
/// ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
/// @param [in] basic  DISK BASIC
/// @param [in] sector セクタ
/// @param [in] secpos セクタ内の位置
/// @param [in] data   セクタ内のディレクトリエントリ
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
{
	this->basic = basic;
	this->type = basic->GetType();

	m_parent = NULL;
	m_children = NULL;
	m_valid_dir = false;

	m_num = 0;
	m_position = secpos;
//	m_file_size = 0;
	m_sector = sector; // no duplicate

	m_data = (directory_t *)data;	// no duplicate
	m_ownmake_data = false;

	m_external_attr = 0;

	m_flags = (VISIBLE_LIST | VISIBLE_TREE);
}
/// ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
/// @param [in] basic  DISK BASIC
/// @param [in] num    通し番号
/// @param [in] track  トラック番号
/// @param [in] side   サイド番号
/// @param [in] sector セクタ
/// @param [in] secpos セクタ内のディレクトリエントリの位置
/// @param [in] data   セクタ内のディレクトリエントリ
/// @param [out] unuse 未使用か
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
{
	this->basic = basic;
	this->type = basic->GetType();

	m_parent = NULL;
	m_children = NULL;
	m_valid_dir = false;

	m_num = num;
	m_position = secpos;
//	m_file_size = 0;
	m_sector = sector; // no duplicate

	m_data = (directory_t *)data;	// no duplicate
	m_ownmake_data = false;

	m_external_attr = 0;

	m_flags = (VISIBLE_LIST | VISIBLE_TREE);
}
/// デストラクタ
DiskBasicDirItem::~DiskBasicDirItem()
{
	if (m_ownmake_data) {
		delete m_data;
	}
	if (m_children) {
		for(size_t i=0; i<m_children->Count(); i++) {
			DiskBasicDirItem *child = m_children->Item(i);
			delete child;
		}
		m_children->Clear();
		delete m_children;
	}
}

/// アイテムへのポインタを設定
/// @param [in] n_num    通し番号
/// @param [in] n_track  トラック番号
/// @param [in] n_side   サイド番号
/// @param [in] n_sector セクタ
/// @param [in] n_secpos セクタ内のディレクトリエントリの位置
/// @param [in] n_data   ディレクトリアイテム
void DiskBasicDirItem::SetDataPtr(int n_num, int n_track, int n_side, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
{
	m_num = n_num;
	m_position = n_secpos;
	m_sector = n_sector; // no duplicate

	if (m_ownmake_data) {
		delete m_data;
	}
	m_ownmake_data = false;
	m_data = (directory_t *)n_data;
}

#ifdef COPYABLE_DIRITEM
/// 複製
/// データ部分は元がメモリを確保している場合、メモリを確保してコピー
/// 確保していない場合、ポインタをコピー
/// @param [in] src ソース
void DiskBasicDirItem::Dup(const DiskBasicDirItem &src)
{
	this->basic = src.basic;
	this->type = src.type;

	m_parent = src.m_parent;
	if (src.m_children) {
		m_children = new DiskBasicDirItems;
		m_children = src.m_children;	// no duplicate
	} else {
		m_children = NULL;
	}
	m_valid_dir = src.m_valid_dir;

	m_num = src.m_num;
	m_position = src.m_position;
//	m_file_size = src.m_file_size;
	m_groups = src.m_groups;
	m_sector = src.m_sector;	// no duplicate

	if (src.m_ownmake_data) {
		m_data = new directory_t;
		memcpy(m_data, src.m_data, sizeof(directory_t));
	} else {
		m_data = src.m_data;	// no duplicate
	}
	m_ownmake_data = src.m_ownmake_data;

	m_external_attr = src.m_external_attr;

	m_flags = src.m_flags;
}
#endif

/// 子ディレクトリを追加
/// @param [in] newitem 新しいアイテム
void DiskBasicDirItem::AddChild(DiskBasicDirItem *newitem)
{
	if (!m_children) {
		m_children = new DiskBasicDirItems;
	}
	m_children->Add(newitem);
}
/// 子ディレクトリ一覧をクリア
void DiskBasicDirItem::EmptyChildren()
{
	if (m_children) {
		for(size_t i=0; i<m_children->Count(); i++) {
			DiskBasicDirItem *child = m_children->Item(i);
			delete child;
		}
		m_children->Empty();

		delete m_children;
	}
	m_children = NULL;
	m_valid_dir = false;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItem::Check(bool &last)
{
	size_t len = GetDataSize();
	wxUint8 *buf = (wxUint8 *)m_data;
	wxUint8 prev = 0;
	bool valid = false;
	// エントリ内のデータがすべて同じ値だとダメ
	for(size_t i = 0; i < len; i++) {
		if (buf[i] == 0 || buf[i] == 0xff) {
			valid = true;
			break;
		}
		if (i != 0 && buf[i] != prev) {
			valid = true;
			break;
		}
		prev = buf[i];
	}
	return valid;
}

/// 削除
/// @param [in] code : 削除コード(デフォルト0)
/// @return true:OK
bool DiskBasicDirItem::Delete(wxUint8 code)
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data->name[0] = basic->InvertUint8(code);
	Used(false);
	return true;
}

/// 内部変数などを再設定
void DiskBasicDirItem::Refresh()
{
	// フラグを更新
	Used(CheckUsed(false));

	// ファイルサイズを再計算
	CalcFileSize();
}

/// アイテムを削除できるか
/// @return true 削除できる
bool DiskBasicDirItem::IsDeletable() const
{
	// ボリュームラベルは不可
	return GetFileAttr().UnmatchType(FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK, FILE_TYPE_VOLUME_MASK);
}

/// アイテムをロード・エクスポートできるか
/// @return true ロードできる
bool DiskBasicDirItem::IsLoadable() const
{
	// ボリュームラベルは不可
	return IsDeletable();
}

/// アイテムをコピーできるか
/// @return true コピーできる
bool DiskBasicDirItem::IsCopyable() const
{
	// ディレクトリ、ボリュームラベルは不可
	return GetFileAttr().MatchType(FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK, 0);
}

/// アイテムを上書きできるか
/// @return true 上書きできる
bool DiskBasicDirItem::IsOverWritable() const
{
	// ディレクトリ、ボリュームラベルは不可
	return IsCopyable();
}

//
//
//

/// ファイル名を格納する位置を返す
/// @param [in]  num    名前バッファ番号
/// @param [out] size   バッファサイズ
/// @param [out] len    ファイル名として使えるサイズ
/// @return 格納先バッファポインタ
wxUint8 *DiskBasicDirItem::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	size = 0;
	len = 0;
	return NULL;
}

/// @brief ファイル名を格納する位置を返す
/// @param [in]  num    名前バッファ番号
/// @param [out] len    ファイル名として使えるサイズ
/// @return 格納先バッファポインタ
wxUint8 *DiskBasicDirItem::GetFileNamePos(int num, size_t &len) const
{
	size_t size;
	return GetFileNamePos(num, size, len);
}

/// 拡張子を格納する位置を返す
/// @param [out] len    バッファサイズ
/// @return 格納先バッファポインタ
wxUint8 *DiskBasicDirItem::GetFileExtPos(size_t &len) const
{
	len = 0;
	return NULL;
}

/// ファイル名の通常コードの割合(0.0-1.0)
double DiskBasicDirItem::NormalCodesInFileName() const
{
	int  count = 0;
	bool invert = basic->IsDataInverted();
	size_t l;
	wxUint8 *n = GetFileNamePos(0, l);
	for(size_t i=0; i<l; i++) {
		wxUint8 c = n[i];
		if (invert) {
			c ^= 0xff;
		}
		if (c >= 0x20) {
			count++;
		}
	}
	return l > 0 ? (double)count / (double)l : 0.0;
}

/// ファイル名を設定 "."で拡張子と分離
/// @param [in]  filename   ファイル名(Unicode)
void DiskBasicDirItem::SetFileNameStr(const wxString &filename)
{
	wxUint8 name[FILENAME_BUFSIZE], ext[FILEEXT_BUFSIZE];
	size_t nlen = sizeof(name);
	size_t elen = sizeof(ext);
	ToNativeFileName(filename, name, nlen, ext, elen);
	SetNativeFileName(name, sizeof(name), nlen, ext, sizeof(ext), elen);
}

/// ファイル名をそのまま設定
/// @note 拡張子部分はクリアする
/// @param [in]  filename   ファイル名(Unicode)
void DiskBasicDirItem::SetFileNamePlain(const wxString &filename)
{
	wxUint8 name[FILENAME_BUFSIZE], ext[FILEEXT_BUFSIZE];
	size_t nlen = sizeof(name);
	size_t elen = 0;
	ToNativeFileName(filename, name, nlen, NULL, elen);
	SetNativeFileName(name, sizeof(name), nlen, ext, sizeof(ext), 0);
}

/// 拡張子を設定
/// @param [in]  fileext    拡張子(Unicode)
void DiskBasicDirItem::SetFileExtPlain(const wxString &fileext)
{
	wxUint8 ext[FILEEXT_BUFSIZE];
	size_t nlen = 0;
	size_t elen = sizeof(ext);
	ToNativeFileName(fileext, ext, elen, NULL, nlen);
	SetNativeFileName(NULL, 0, 0, ext, sizeof(ext), elen);
}

/// ファイル名をコピー
/// @param [in]  src ディレクトリアイテム
void DiskBasicDirItem::CopyFileName(const DiskBasicDirItem &src)
{
	wxUint8 name[FILENAME_BUFSIZE], ext[FILEEXT_BUFSIZE];
	size_t nlen = sizeof(name);
	size_t elen;
	// 拡張子があるか
	GetFileExtPos(elen);
	src.GetNativeFileName(name, nlen, ext, elen);
	SetNativeFileName(name, sizeof(name), nlen, ext, sizeof(ext), elen);
}

/// ファイル名を設定
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
/// @note filename はデータビットが反転している場合あり
void DiskBasicDirItem::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		if (ns > size) ns = size;
		memcpy(n, filename, ns);
	}
}

/// 拡張子を設定
/// @param [in,out] fileext  拡張子
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
/// @note fileext はデータビットが反転している場合あり
void DiskBasicDirItem::SetNativeExt(wxUint8 *fileext, size_t size, size_t length)
{
	wxUint8 *e;
	size_t el = 0;
	e = GetFileExtPos(el);
	if (e != NULL && el > 0) {
		if (el > size) el = size;
		memcpy(e, fileext, el);
	}
}

/// ファイル名と拡張子を設定
/// @param [in,out] name  ファイル名
/// @param [in]     nsize ファイル名バッファサイズ
/// @param [in]     nlen  ファイル名長さ
/// @param [in,out] ext   拡張子
/// @param [in]     esize 拡張子バッファサイズ
/// @param [in]     elen  拡張子長さ
/// @note データビットはこの関数で反転させる
void DiskBasicDirItem::SetNativeFileName(wxUint8 *name, size_t nsize, size_t nlen, wxUint8 *ext, size_t esize, size_t elen)
{
	bool invert = basic->IsDataInverted();
	char space = basic->GetDirSpaceCode();	// 空白コード
	char term  = basic->GetDirTerminateCode();	// 終端コード

	if (name) {
		if (nsize > nlen) {
			name[nlen] = (wxUint8)term;
			nlen++;
		}
		if (nsize > nlen) {
			memset(&name[nlen], space, nsize - nlen);
		}
		if (invert) {
			mem_invert(name, nsize);
		}

		SetNativeName(name, nsize, nlen);
	}

	if (ext) {
		if (esize > elen) {
			ext[elen] = (wxUint8)term;
			elen++;
		}
		if (esize > elen) {
			memset(&ext[elen], space, esize - elen);
		}
		if (invert) {
			mem_invert(ext, esize);
		}

		SetNativeExt(ext, esize, elen);
	}
}

/// ファイル名を返す 名前 + "." + 拡張子
/// @return ファイル名
wxString DiskBasicDirItem::GetFileNameStr() const
{
	wxUint8 name[FILENAME_BUFSIZE], ext[FILEEXT_BUFSIZE];
	size_t nl, el;
	nl = sizeof(name);
	el = sizeof(ext);

	GetNativeFileName(name, nl, ext, el);

	wxString dst;

	ConvCharsToString(name, nl, dst);

	if (el > 0) {
		dst += wxT(".");
		ConvCharsToString(ext, el, dst);
	}

	return dst;
}

/// ファイル名(拡張子除く)を返す
/// @return ファイル名
wxString DiskBasicDirItem::GetFileNamePlainStr() const
{
	wxUint8 name[FILENAME_BUFSIZE];
	size_t nl, el;
	nl = sizeof(name);
	el = 0;

	GetNativeFileName(name, nl, NULL, el);

	wxString dst;

	ConvCharsToString(name, nl, dst);

	return dst;
}

/// 拡張子を返す
/// @return 拡張子
wxString DiskBasicDirItem::GetFileExtPlainStr() const
{
	wxUint8 ext[FILEEXT_BUFSIZE];
	size_t nl, el;
	nl = 0;
	el = sizeof(ext);

	GetNativeFileName(NULL, nl, ext, el);

	wxString dst;

	ConvCharsToString(ext, el, dst);

	return dst;
}

/// ファイル名を得る 名前 + "." + 拡張子
/// @note バッファを超える場合は拡張子を追加しない
/// @param [in] filename 出力先バッファ
/// @param [in] length   出力先バッファのサイズ
void DiskBasicDirItem::GetFileName(wxUint8 *filename, size_t length) const
{
	wxUint8 name[FILENAME_BUFSIZE], ext[FILEEXT_BUFSIZE];
	size_t nl, el;
	nl = sizeof(name);
	el = sizeof(ext);

	GetNativeFileName(name, nl, ext, el);

	memcpy(filename, name, length > nl ? nl : length);

	if (el > 0 && (nl + el + 1) < length) {
		// 拡張子ありの場合"."を追加
		// ただし、バッファを超える場合は拡張子を追加しない
		filename[nl] = '.';
		nl++;
		memcpy(&filename[nl], ext, el);
	}
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
/// @note データビットは反転させたまま
void DiskBasicDirItem::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t s = 0;
	size_t l = 0;

	n = GetFileNamePos(0, s, l);
	if (n != NULL && s > 0) {
		if (s > size) s = size;
		memcpy(filename, n, s);
	}

	length = l;
}

/// 拡張子を得る
/// @param [in,out] fileext  拡張子
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
/// @note データビットは反転させたまま
void DiskBasicDirItem::GetNativeExt(wxUint8 *fileext, size_t size, size_t &length) const
{
	wxUint8 *e = NULL;
	size_t l = 0;

	e = GetFileExtPos(l);
	if (e != NULL && l > 0) {
		memcpy(fileext, e, l);
	}

	length = l;
}

/// ファイル名と拡張子を得る
/// それぞれ空白は右トリミング
/// @param [out]    name     ファイル名バッファ
/// @param [in,out] nlen     上記バッファサイズ / 文字列長さを返す
/// @param [out]    ext      拡張子名バッファ
/// @param [in,out] elen     上記バッファサイズ / 文字列長さを返す
/// @note データビットはこの関数で反転させる
void DiskBasicDirItem::GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	size_t nl, el;
	bool invert = basic->IsDataInverted();
	char trimm = basic->GetDirTrimmingCode();	// とり除くコード
	char space = basic->GetDirSpaceCode();	// 空白コード
	char term  = basic->GetDirTerminateCode();	// 終端コード

	if (name && nlen > 0) {
		memset(name, 0, nlen);

		GetNativeName(name, nlen, nl);

		if (invert) {
			mem_invert(name, nl);
		}
		nl = rtrim(name, nl, trimm);
		nl = rtrim(name, nl, space);
		nl = rtrim(name, nl, term);

		if (IsUsed()) nlen = str_shrink(name, nl);
		else nlen = nl;
	}

	if (ext && elen > 0) {
		memset(ext, 0, elen);

		GetNativeExt(ext, elen, el);

		if (invert) {
			mem_invert(ext, el);
		}
		el = rtrim(ext, el, trimm);
		el = rtrim(ext, el, space);
		el = rtrim(ext, el, term);

		if (IsUsed()) elen = str_shrink(ext,  el);
		else elen = el;
	}
}

/// ファイル名(拡張子除く)が一致するか
/// @param [in] name      ファイル名
/// @param [in] icase     大文字小文字を区別しないか(case insensitive)
bool DiskBasicDirItem::IsSameName(const wxString &name, bool icase) const
{
	if (!IsUsedAndVisible()) return false;

	wxUint8 sname[FILENAME_BUFSIZE];
	wxUint8 dname[FILENAME_BUFSIZE];
	size_t snlen = sizeof(sname);
	size_t dnlen = sizeof(dname);
	size_t selen = 0;
	size_t delen = 0;

	// ファイル名を内部ファイル名に変換
	ToNativeFileName(name, dname, dnlen, NULL, delen);
	// このアイテムのファイル名を取得
	GetNativeFileName(sname, snlen, NULL, selen);
	// 小文字大文字を区別しない場合
	if (icase) {
		to_upper(sname, snlen);
		to_upper(dname, dnlen);
	}
	// 比較
	return (memcmp(sname, dname, snlen > dnlen ? snlen : dnlen) == 0);
}

/// 同じファイル名か
/// ファイル名＋拡張子＋拡張属性で一致するかどうか
/// @param [in] filename ファイル名
/// @param [in] icase    大文字小文字を区別しないか(case insensitive)
/// @see GetOptionalName()
bool DiskBasicDirItem::IsSameFileName(const DiskBasicFileName &filename, bool icase) const
{
	if (!IsUsedAndVisible()) return false;

	wxUint8 sname[FILENAME_BUFSIZE], sext[FILEEXT_BUFSIZE];
	wxUint8 dname[FILENAME_BUFSIZE], dext[FILEEXT_BUFSIZE];
	size_t snlen = sizeof(sname);
	size_t selen = sizeof(sext);
	size_t dnlen = sizeof(dname);
	size_t delen = sizeof(dext);

	// ファイル名を内部ファイル名に変換
	ToNativeFileName(filename.GetName(), dname, dnlen, dext, delen);
	// このアイテムのファイル名を取得
	GetNativeFileName(sname, snlen, sext, selen);
	// 小文字大文字を区別しない場合
	if (icase) {
		to_upper(sname, snlen);
		to_upper(sext, selen);
		to_upper(dname, dnlen);
		to_upper(dext, delen);
	}
	// 比較
	return (memcmp(sname, dname, snlen > dnlen ? snlen : dnlen) == 0)
		&& ((delen == 0 && selen == 0) || memcmp(sext, dext, selen > delen ? selen : delen) == 0)
		&& (GetOptionalName() == filename.GetOptional());
}

/// 同じファイル名か
/// ファイル名＋拡張子＋拡張属性で一致するかどうか
/// @param [in] src     ディレクトリアイテム
/// @param [in] icase   大文字小文字を区別しないか(case insensitive)
/// @see GetOptionalName()
bool DiskBasicDirItem::IsSameFileName(const DiskBasicDirItem *src, bool icase) const
{
	if (!IsUsedAndVisible()) return false;

	wxUint8 sname[FILENAME_BUFSIZE], sext[FILEEXT_BUFSIZE];
	wxUint8 dname[FILENAME_BUFSIZE], dext[FILEEXT_BUFSIZE];
	size_t snlen = sizeof(sname);
	size_t selen = sizeof(sext);
	size_t dnlen = sizeof(dname);
	size_t delen = sizeof(dext);

	// ファイル名を内部ファイル名に変換
	src->GetNativeFileName(dname, dnlen, dext, delen);
	// このアイテムのファイル名を取得
	GetNativeFileName(sname, snlen, sext, selen);
	// 小文字大文字を区別しない場合
	if (icase) {
		to_upper(sname, snlen);
		to_upper(sext, selen);
		to_upper(dname, dnlen);
		to_upper(dext, delen);
	}
	// 比較
	return (memcmp(sname, dname, snlen > dnlen ? snlen : dnlen) == 0)
		&& ((delen == 0 && selen == 0) || memcmp(sext, dext, selen > delen ? selen : delen) == 0)
		&& (src->GetOptionalName() == GetOptionalName());
}

/// ファイル名＋拡張子のサイズ
/// @return サイズ
int DiskBasicDirItem::GetFileNameStrSize() const
{
	size_t nl = 0;
	size_t el = 0;
	size_t l;
	int num = 0;
	do {
		l = 0;
		GetFileNamePos(num, l);
		if (l == 0) break;
		nl += l;
		num++;
	} while(l > 0);

	GetFileExtPos(el);
	if (el > 0) el++;

	return (int)(nl + el);
}

/// ファイルパスから内部ファイル名を生成する
///
/// インポート時のダイアログを出す前
/// 拡張子がない機種では拡張子部分をファイル名としてそのまま設定
///
/// @param [in] filepath ファイルパス
/// @return ファイル名
wxString DiskBasicDirItem::RemakeFileNameAndExtStr(const wxString &filepath) const
{
	wxString newname;
	wxFileName fn(filepath);
	size_t nl, el;

	GetFileNamePos(0, nl);
	GetFileExtPos(el);

	if (el == 0) {
		newname = fn.GetFullName().Left(nl);
	} else {
		newname = fn.GetName().Left(nl);
		if (!fn.GetExt().IsEmpty()) {
			newname += wxT(".");
			newname += fn.GetExt().Left(el);
		}
	}

	// 大文字にする（機種依存）
	if (basic->ToUpperBeforeDialog()) {
		newname.MakeUpper();
	}
	ConvertFileNameBeforeImportDialog(newname);

	return newname;
}

/// ファイルパスから内部ファイル名を生成する
///
/// インポート時のダイアログを出す前
/// 拡張子がない機種では拡張子はとり除かれる
///
/// @param [in] filepath ファイルパス
/// @return ファイル名
wxString DiskBasicDirItem::RemakeFileNameOnlyStr(const wxString &filepath) const
{
	wxString newname;
	wxFileName fn(filepath);
	size_t nl, el;

	GetFileNamePos(0, nl);
	GetFileExtPos(el);

	newname = fn.GetName().Left(nl);
	if (el > 0) {
		if (!fn.GetExt().IsEmpty()) {
			newname += wxT(".");
			newname += fn.GetExt().Left(el);
		}
	}

	// 大文字にする（機種依存）
	if (basic->ToUpperBeforeDialog()) {
		newname.MakeUpper();
	}
	ConvertFileNameBeforeImportDialog(newname);

	return newname;
}

/// 属性から拡張子を付加する
///
/// エクスポートするファイルのファイル名に適用
///
/// @param [in]     file_type ファイル属性
/// @param [in]     mask      ファイル属性ビットマスク
/// @param [in,out] filename  ファイル名
/// @param [in]     dupli     同じ拡張子名の重複を許すか
void DiskBasicDirItem::AddExtensionByFileAttr(int file_type, int mask, wxString &filename, bool dupli) const
{
	wxFileName fn(filename);
	// 拡張子と属性で登録されているか
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt(), file_type, mask);
	if (!sa) {
		// 属性で登録されているか
		sa = basic->GetAttributesByExtension().FindType(file_type, mask);
	}
	if (sa) {
		// 同じ拡張子でなければ拡張子部分を付加
		if (dupli || fn.GetExt().Upper() != sa->GetName().Upper()) {
			filename += wxT(".");
			if (Utils::IsUpperString(filename)) {
				filename += sa->GetName().Upper();
			} else {
				filename += sa->GetName().Lower();
			}
		}
	}
}

/// 属性から拡張子を付加する
///
/// エクスポートするファイルのファイル名に適用
///
/// @param [in]     file_type ファイル属性
/// @param [in]     mask      ファイル属性ビットマスク
/// @param [in,out] filename  ファイル名
/// @param [in]     external  拡張属性 valueと比較
/// @param [in]     dupli     同じ拡張子名の重複を許すか
void DiskBasicDirItem::AddExtensionByFileAttr(int file_type, int mask, wxString &filename, int external, bool dupli) const
{
	wxFileName fn(filename);
	// 拡張子と属性で登録されているか
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt(), file_type, mask);
	if (!sa) {
		// 属性で登録されているか
		sa = basic->GetAttributesByExtension().Find(file_type, mask, external);
	}
	if (!sa) {
		// 属性で登録されているか
		sa = basic->GetAttributesByExtension().FindType(file_type, mask);
	}
	if (sa) {
		// 同じ拡張子でなければ拡張子部分を付加
		if (dupli || fn.GetExt().Upper() != sa->GetName().Upper()) {
			filename += wxT(".");
			if (Utils::IsUpperString(filename)) {
				filename += sa->GetName().Upper();
			} else {
				filename += sa->GetName().Lower();
			}
		}
	}
}

/// 属性の文字列を返す
bool DiskBasicDirItem::GetFileAttrName(int pos1, const char *list[], int unknown_pos, wxString &attr) const
{
	bool match = true;
	if (pos1 >= 0) {
		attr = wxGetTranslation(list[pos1]);
	} else {
		const L3Attribute *sa = basic->GetSpecialAttributes().FindValue(-pos1);
		if (sa != NULL) {
			attr = sa->GetName();
		} else {
			match = false;
			attr = wxGetTranslation(list[unknown_pos]);
		}
	}
	return match;
}

/// 拡張子から属性を決定できるか 決定できる場合、末尾の拡張子をとり除く
///
/// インポートするファイルのファイル名に適用
///
/// @par For Example:
///  foobar.aaa.bas (is basic file) -> foobar.aaa (trimming last extension) @n
///  foobar.bas (is basic file) -> foobar (trimming last extension) @n
///  foobar.bbb.ccc (is unknown attr) -> foobar.bbb.ccc (no change filename) @n
bool DiskBasicDirItem::TrimExtensionByExtensionAttr(wxString &filename) const
{
	wxFileName fn(filename);
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
	if (sa) {
		// 拡張子部分をとり除く
		filename = fn.GetName();
	}
	return (sa != NULL);
}

/// 拡張子から属性を決定できるか
///
/// 決定できる場合、拡張子が２つつながっている場合は末尾の拡張子をとり除く
///
/// インポートするファイルのファイル名に適用
///
/// @par For Example:
///  foobar.aaa.bas (is basic file) -> foobar.aaa (trimming last extension) @n
///  foobar.bas (is basic file) -> foobar.bas (no change filename) @n
///  foobar.bbb.ccc (is unknown attr) -> foobar.bbb.ccc (no change filename) @n
bool DiskBasicDirItem::TrimLastExtensionByExtensionAttr(wxString &filename) const
{
	wxFileName fn(filename);
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
	if (sa) {
		// すでに拡張子があれば拡張子部分をとり除く
		wxFileName sfn(fn.GetName());
		if (!sfn.GetExt().IsEmpty()) {
			filename = fn.GetName();
		}
	}
	return (sa != NULL);
}


/// 拡張子から属性を決定できるか 決定できる場合、末尾の拡張子をとり除く
///
/// インポートするファイルのファイル名に適用
///
/// @param [in] filename   ファイル名
/// @param [in] list       リスト
/// @param [in] list_first リストの最初の位置
/// @param [in] list_last  リストの最後の位置
/// @param [out] outfile   編集後のファイル名
/// @param [out] attr      属性 (ユーザ指定属性の場合はマイナスになる)
/// @par For Example:
///  foobar.aaa.bas (is basic file) -> foobar.aaa (trimming last extension) @n
///  foobar.bas (is basic file) -> foobar (trimming last extension) @n
///  foobar.bbb.ccc (is unknown attr) -> foobar.bbb.ccc (no change filename) @n
bool DiskBasicDirItem::IsContainAttrByExtension(const wxString &filename, const char *list[], int list_first, int list_last, wxString *outfile, int *attr) const
{
	bool match = false;
	int t1 = 0;
	wxFileName fn(filename);
	// 拡張子リストに一致するか
	int idx = Utils::IndexOf(list, fn.GetExt().Upper()); 
	if (idx >= list_first && idx <= list_last) {
		match = true;
		t1 = idx;
	} else {
		// ユーザ指定の属性か
		const L3Attribute *sa = basic->GetSpecialAttributes().FindUpperCase(fn.GetExt());
		if (sa) {
			match = true;
			t1 = -sa->GetValue();
		} else if (!sa) {
			// その他
			sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
			if (sa) {
				match = true;
				t1 = sa->GetValue();
			}
		}
	}
	if (match) {
		// 拡張子部分をとり除く
		if (outfile) *outfile = fn.GetName();
		if (attr) *attr = t1;
	}

	return match;
}

/// ファイル名を変換して内部ファイル名にする 検索用
/// @param [in]     filename ファイル名(Unicode)
/// @param [out]    name     内部ファイル名バッファ
/// @param [in,out] nlen     上記バッファサイズ / 文字列長さを返す
/// @param [out]    ext      内部拡張子名バッファ
/// @param [in,out] elen     上記バッファサイズ / 文字列長さを返す
/// @return true OK / false 変換できない文字がある
bool DiskBasicDirItem::ToNativeFileName(const wxString &filename, wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	wxUint8 tmp[FILENAME_BUFSIZE];
	size_t nl = 0;
	size_t el = 0;

	wxString namestr = filename;

	// ダイアログ入力後のファイル名文字列を変換 大文字にする（機種依存）
	if (basic->ToUpperAfterRenamed()) {
		namestr.MakeUpper();
	}
	ConvertFileNameAfterRenamed(namestr);

	int tmplen = ConvStringToChars(namestr, tmp, sizeof(tmp));
	if (tmplen < 0) return false;

	// 拡張子があるか
	if (ext != NULL && elen > 0) {
		GetFileExtPos(elen);
	}
	SplitFileName(tmp, sizeof(tmp), tmplen, name, nlen, nl, ext, elen, el);
	nlen = nl;
	elen = el;

	return true;
}

/// 文字列をバッファにコピー
/// @param [in]   src     ファイル名
/// @param [in]   ssize   ファイル名サイズ
/// @param [in]   slen    ファイル名長さ
/// @param [out]  dst     出力先バッファ
/// @param [in]   dsize   上記バッファサイズ
/// @param [out]  dlen    上記長さ
void DiskBasicDirItem::MemoryCopy(const wxUint8 *src, size_t ssize, size_t slen, wxUint8 *dst, size_t dsize, size_t &dlen)
{
	if (dst == NULL || dsize == 0) {
		dlen = 0;
		return;
	}
	if (dsize > slen) memset(&dst[slen], 0, dsize - slen);
	if (slen > 0) memcpy(dst, src, slen);
	dlen = slen;
}

/// 文字列をバッファにコピー "."で拡張子とを分ける
/// @param [in]   src     ファイル名
/// @param [in]   ssize   ファイル名サイズ
/// @param [in]   slen    ファイル名長さ
/// @param [out]  dname   出力先ファイル名バッファ
/// @param [in]   dnsize  上記ファイル名バッファサイズ
/// @param [out]  dnlen   上記ファイル名長さ
/// @param [out]  dext    出力先拡張子バッファ
/// @param [in]   desize  上記拡張子バッファサイズ
/// @param [out]  delen   上記拡張子長さ
void DiskBasicDirItem::SplitFileName(const wxUint8 *src, size_t ssize, size_t slen, wxUint8 *dname, size_t dnsize, size_t &dnlen, wxUint8 *dext, size_t desize, size_t &delen)
{
	// .で分割する
	int pos = mem_rchr(src, slen, '.');
	// 拡張子サイズありで"."があれば分割してそれぞれのバッファに入れる
	if (desize > 0 && pos >= 0) {
		MemoryCopy(src, ssize, pos, dname, dnsize, dnlen);
		pos++;
		MemoryCopy(&src[pos], ssize - pos, slen - pos, dext, desize, delen);
	} else {
		MemoryCopy(src, ssize, slen, dname, dnsize, dnlen);
		MemoryCopy(NULL, 0, 0, dext, desize, delen);
	}
}

//
//
//

/// 属性を設定
/// @param [in] file_type : 属性
void DiskBasicDirItem::SetFileAttr(const DiskBasicFileType &file_type)
{
}

/// 属性を設定
/// @param [in] format_type   フォーマット種類 DiskBasicFormatType
/// @param [in] file_type     共通属性 enum #en_file_type_mask の組み合わせ
/// @param [in] original_type 本来の属性
void DiskBasicDirItem::SetFileAttr(DiskBasicFormatType format_type, int file_type, int original_type)
{
	SetFileAttr(DiskBasicFileType(format_type, file_type, original_type));
}

/// 属性を返す
/// @return 属性
DiskBasicFileType DiskBasicDirItem::GetFileAttr() const
{
	return DiskBasicFileType();
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
/// @return 文字列
wxString DiskBasicDirItem::GetFileAttrStr() const
{
	return wxT("");
}

/// 通常のファイルか ディレクトリ削除でのチェックで使用
/// @return true 通常のファイル
bool DiskBasicDirItem::IsNormalFile() const
{
	DiskBasicFileType attr = GetFileAttr();
	if (attr.IsVolume()) {
		// ボリュームラベルは特殊
		return false;
	} else if (attr.IsDirectory()) {
		// ディレクトリのとき、カレント、親以外なら通常ファイルとする
		wxString name = GetFileNameStr();
		if (name == wxT(".") || name == wxT("..")) {
			return false;
		} else {
			return true;
		}
	}
	return true;
}

/// ディレクトリか
/// @return true ディレクトリ
bool DiskBasicDirItem::IsDirectory() const
{
	return GetFileAttr().IsDirectory();
}

/// ファイルサイズをセット
/// @param [in] val サイズ
void DiskBasicDirItem::SetFileSize(int val)
{
	m_groups.SetSize(val);
}

/// ファイルサイズを返す
/// @return サイズ
int DiskBasicDirItem::GetFileSize() const
{
	return (int)m_groups.GetSize();
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [out] group_items グループリスト
void DiskBasicDirItem::GetAllGroups(DiskBasicGroups &group_items)
{
	group_items.Empty();
	GetUnitGroups(0, group_items);
}

/// グループ数をセット
/// @param [in] val 数
void DiskBasicDirItem::SetGroupSize(int val)
{
	m_groups.SetNums(val);
}

/// グループ数を返す
/// @return 数
int DiskBasicDirItem::GetGroupSize() const
{
	return m_groups.GetNums();
}

/// 最初のグループ番号をセット
/// @param [in] fileunit_num ファイル番号
/// @param [in] val          番号
/// @param [in] size         サイズ(機種依存)
void DiskBasicDirItem::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
}
/// 最初のグループ番号を返す
/// @param [in] fileunit_num ファイル番号
/// @return 番号
wxUint32 DiskBasicDirItem::GetStartGroup(int fileunit_num) const
{
	return 0;
}

/// 追加のグループ番号をセット(機種依存)
/// @param [in] val 番号
void DiskBasicDirItem::SetExtraGroup(wxUint32 val)
{
}
/// 追加のグループ番号を返す(機種依存)
/// @return 番号
wxUint32 DiskBasicDirItem::GetExtraGroup() const
{
	return INVALID_GROUP_NUMBER;
}

/// 追加のグループ番号を得る(機種依存)
void DiskBasicDirItem::GetExtraGroups(wxArrayInt &arr) const
{
}

/// 最後のグループ番号をセット(機種依存)
/// @param [in] val 番号
void DiskBasicDirItem::SetLastGroup(wxUint32 val)
{
}

/// 最後のグループ番号を返す(機種依存)
/// @return 番号
wxUint32 DiskBasicDirItem::GetLastGroup() const
{
	return INVALID_GROUP_NUMBER;
}

/// グループリストの数を返す
size_t DiskBasicDirItem::GetGroupCount() const
{
	return m_groups.Count();
}

/// グループリストを返す
const DiskBasicGroups &DiskBasicDirItem::GetGroups() const
{
	return m_groups;
}

/// グループリストを設定
void DiskBasicDirItem::SetGroups(const DiskBasicGroups &vals)
{
	m_groups = vals;
}

/// グループリストのアイテムを返す
DiskBasicGroupItem *DiskBasicDirItem::GetGroup(size_t idx) const
{
	return m_groups.ItemPtr(idx);
}

/// 日付を得る
/// @param [out] tm 日付
void DiskBasicDirItem::GetFileDate(struct tm *tm) const
{
	tm->tm_year = -1;
	tm->tm_mon = -2;
	tm->tm_mday = -1;
}

/// 時間を得る
/// @param [out] tm 時間
void DiskBasicDirItem::GetFileTime(struct tm *tm) const
{
	tm->tm_hour = -1;
	tm->tm_min = -1;
	tm->tm_sec = -1;
}

/// 日時を得る
/// @param [out] tm 日時
void DiskBasicDirItem::GetFileDateTime(struct tm *tm) const
{
	GetFileDate(tm);
	GetFileTime(tm);
}

/// 日時を返す
struct tm DiskBasicDirItem::GetFileDateTime() const
{
	struct tm tm;
	GetFileDateTime(&tm);
	return tm;
}

/// 日時をセット
/// @param [in] tm 日時
void DiskBasicDirItem::SetFileDateTime(const struct tm *tm)
{
	SetFileDate(tm);
	SetFileTime(tm);
}

/// 日付のタイトル名（ダイアログ用）
/// @return タイトル文字列
wxString DiskBasicDirItem::GetFileDateTimeTitle() const
{
	return _("Created Date:");
}

/// ファイルの日付を文字列にして返す
/// @return 日付文字列
wxString DiskBasicDirItem::GetFileDateStr() const
{
	return wxT("");
}

/// ファイルの時間を文字列にして返す
/// @return 時間文字列
wxString DiskBasicDirItem::GetFileTimeStr() const
{
	return wxT("");
}

/// ファイルの日時を文字列にして返す
/// @return 日時文字列 ない場合"---"
wxString DiskBasicDirItem::GetFileDateTimeStr() const
{
	wxString str = GetFileDateStr();
	if (!str.IsEmpty()) str += wxT(" ");
	str += GetFileTimeStr();
	if (str.IsEmpty()) str += wxT("---");
	return str;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItem::CalcFileSize()
{
	m_groups.Empty();
	CalcFileUnitSize(0);
}

/// ファイルの終端コードをチェックして必要なサイズを返す
/// @param [in] istream   入力ストリーム
/// @param [in] file_size 入力ストリームの元のデータサイズ
/// @return 終端コードを付加したサイズ（元のサイズ+1）
int DiskBasicDirItem::CheckEofCode(wxInputStream *istream, int file_size)
{
	// ファイルの最終が終端記号で終わっているかを調べる
	istream->SeekI(-1, wxFromEnd);
	char c[4];
	istream->Read(c, 1);
	if (c[0] != GetEofCode()) {
		file_size++;
	}
	istream->SeekI(0);
	return file_size;
}

/// アイテムの属するセクタを変更済みにする（未実装）
void DiskBasicDirItem::SetModify()
{
}

/// ディレクトリアイテムのサイズ
/// @return サイズ
size_t DiskBasicDirItem::GetDataSize() const
{
	return sizeof(directory_t);
}

/// アイテムをコピー
/// @param [in] val ディレクトリアイテム
/// @return false コピー先なし
bool DiskBasicDirItem::CopyData(const directory_t *val)
{
	if (!m_data) return false;
	memcpy(m_data, val, GetDataSize());
	return true;
}

/// ファイル名、属性をコピー
/// @param [in] src ディレクトリアイテム
void DiskBasicDirItem::CopyItem(const DiskBasicDirItem &src)
{
	// データはコピーする
	CopyData(src.GetData());
	// グループ
	m_groups = src.m_groups;
	// サイズ
//	m_file_size = src.m_file_size;
	// フラグ
	m_flags = src.m_flags;
	// その他の属性
	m_external_attr = src.m_external_attr;
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItem::ClearData()
{
	if (!m_data) return;
	int c = 0;
	size_t l;
	l = GetDataSize();
	memset(m_data, c, l);
	if (basic->IsDataInverted()) mem_invert(m_data, l);
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItem::InitialData()
{
	ClearData();
}

/// データをエクスポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItem::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	AddExtensionByFileAttr(GetFileAttr().GetType(), 0x3f, filename);
	return true;
}

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItem::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimLastExtensionByExtensionAttr(filename);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイルの終端コードを返す
wxUint8	DiskBasicDirItem::GetEofCode() const
{
	return basic->GetTextTerminateCode();
}

/// ファイル番号のファイルサイズを得る
/// １つのエントリで複数のファイルを管理している場合は要オーバーライド
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] istream      入力ストリーム
/// @param [in]     file_offset  ストリーム内のオフセット
/// @return ファイルサイズ / ない場合 -1
int DiskBasicDirItem::GetFileUnitSize(int fileunit_num, wxInputStream &istream, int file_offset)
{
	if (fileunit_num == 0) {
		return (int)istream.GetLength();
	} else {
		return -1;
	}
}

/// ファイル番号のファイルへアクセスできるか
/// １つのエントリで複数のファイルを管理している場合は要オーバーライド
/// @param [in]     fileunit_num ファイル番号
bool DiskBasicDirItem::IsValidFileUnit(int fileunit_num)
{
	return (fileunit_num == 0);
}

/// ファイル名から属性を決定する
int DiskBasicDirItem::ConvFileTypeFromFileName(const wxString &filename) const
{
	return 0;
}

/// ファイル名から属性を決定する
int DiskBasicDirItem::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	return 0;
}

/// ファイル名から拡張属性を決定する
int DiskBasicDirItem::ConvOptionalNameFromFileName(const wxString &filename) const
{
	return 0;
}

/// 属性値を加工する
///
/// インポートダイアログ入力後に必要なら入力した属性値の加工を行う
bool DiskBasicDirItem::ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	return true;
}

/// その他の属性値を設定する
void DiskBasicDirItem::SetAttr(DiskBasicDirItemAttr &attr)
{
}

/// 文字列をバイト列に変換 文字コードは機種依存
/// @return >=0 バイト数
int DiskBasicDirItem::ConvStringToChars(const wxString &src, wxUint8 *dst, size_t len) const
{
	return basic->GetCharCodes().ConvToChars(src, dst, len);
}

/// バイト列を文字列に変換 文字コードは機種依存
void DiskBasicDirItem::ConvCharsToString(const wxUint8 *src, size_t len, wxString &dst) const
{
	basic->GetCharCodes().ConvToString(src, len, dst);
}


/// ファイル属性をXMLで出力
/// @param [in] path 出力先XMLファイルパス
bool DiskBasicDirItem::WriteFileAttrToXml(const wxString &path)
{
	wxXmlDocument doc;

	wxXmlNode *root = new wxXmlNode(NULL, wxXML_ELEMENT_NODE, wxT("FileInformation"));
	doc.SetRoot(root);

	DiskBasicFileType file_type = GetFileAttr();

	int val = 0;
	wxXmlNode *external1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("ExternalAttribute"));
	external1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("0x%x"), (int)GetExternalAttr())));

	struct tm tm;
	GetFileDateTime(&tm);
	if (tm.tm_hour >= 0 && tm.tm_min >= 0) {
		wxXmlNode *time1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("Time"));
		time1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::FormatHMSStr(&tm)));
	}

	if (tm.tm_mon >= -1 && tm.tm_mday >= 0) {
		wxXmlNode *date1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("Date"));
		date1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::FormatYMDStr(&tm)));
	}

	val = GetExecuteAddress();
	if (val >= 0) {
		wxXmlNode *exec1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("ExecuteAddress"));
		exec1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("0x%x"), val)));
	}

	val = GetEndAddress();
	if (val >= 0) {
		wxXmlNode *end1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("EndAddress"));
		end1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("0x%x"), val)));
	}

	val = GetStartAddress();
	if (val >= 0) {
		wxXmlNode *start1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("StartAddress"));
		start1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("0x%x"), val)));
	}

	wxXmlNode *size1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("Size"));
	size1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("%d"), (int)GetFileSize())));

	wxXmlNode *orig1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("OriginalType"));
	orig1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("0x%x"), (int)file_type.GetOrigin())));

	wxXmlNode *type1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("Type"));
	type1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("0x%x"), (int)file_type.GetType())));

	wxUint8 fname[FILENAME_BUFSIZE], fext[FILEEXT_BUFSIZE];
	size_t fnl = sizeof(fname);
	memset(fname, 0, fnl);
	size_t fel = sizeof(fext);
	memset(fext, 0, fel);
	GetNativeFileName(fname, fnl, fext, fel);
	wxXmlNode *ext1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("Ext"));
	ext1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::EncodeEscape(fext, fel)));
	wxXmlNode *name1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("Name"));
	name1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::EncodeEscape(fname, fnl)));

	wxXmlNode *fmt1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("Format"));
	fmt1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("%d"), (int)file_type.GetFormat())));

	return doc.Save(path);
}

/// ファイル属性をXMLから読み込む
/// @param [in]     path    XMLファイルパス
/// @param [out]    tm      アイテム内の日時
/// @return true:正常  false:ファイル読み込めない
bool DiskBasicDirItem::ReadFileAttrFromXml(const wxString &path, struct tm *tm)
{
	wxXmlDocument doc;

	if (!wxFileName::FileExists(path)) return false;
	if (!doc.Load(path)) return false;

	// 各パラメータをセット
	wxXmlNode *root = doc.GetRoot();
	if (!root) return false;

	int format_type = FORMAT_TYPE_UNKNOWN;
	int file_type = 0;
	int original_type = 0;
	int file_size = 0;
	int start_addr = -1;
	int end_addr = -1;
	int exec_addr = -1;
	int external_attr = 0;
	struct tm tm_tmp;
	tm_tmp.tm_year = -1;
	tm_tmp.tm_mon = -2;
	tm_tmp.tm_mday = -1;
	tm_tmp.tm_hour = -1;
	tm_tmp.tm_min = -1;
	tm_tmp.tm_sec = -1;

	wxUint8 fname[FILENAME_BUFSIZE], fext[FILEEXT_BUFSIZE];
	size_t fnl = sizeof(fname);
	size_t fel = sizeof(fext);
	memset(fname, 0, fnl);
	memset(fext, 0, fel);

	wxXmlNode *node = root->GetChildren();
	while(node) {
		wxString name = node->GetName();
		wxString content = node->GetNodeContent();
		if (name == wxT("Format")) {
			format_type = Utils::ToInt(content);
		} else if (name == wxT("Name")) {
			Utils::DecodeEscape(content, fname, fnl);
		} else if (name == wxT("Ext")) {
			Utils::DecodeEscape(content, fext, fel);
		} else if (name == wxT("Type")) {
			file_type = Utils::ToInt(content);
		} else if (name == wxT("OriginalType")) {
			original_type = Utils::ToInt(content);
		} else if (name == wxT("Size")) {
			file_size = Utils::ToInt(content);
		} else if (name == wxT("StartAddress")) {
			start_addr = Utils::ToInt(content);
		} else if (name == wxT("EndAddress")) {
			end_addr = Utils::ToInt(content);
		} else if (name == wxT("ExecuteAddress")) {
			exec_addr = Utils::ToInt(content);
		} else if (name == wxT("Date")) {
			Utils::ConvDateStrToTm(content, &tm_tmp);
		} else if (name == wxT("Time")) {
			Utils::ConvTimeStrToTm(content, &tm_tmp);
		} else if (name == wxT("ExternalAttribute")) {
			external_attr = Utils::ToInt(content);
		}
		node = node->GetNext();
	}

	fnl = str_length(fname, fnl, 0);
	fel = str_length(fext, fel, 0);
	SetNativeFileName(fname, sizeof(fname), fnl, fext, sizeof(fext), fel);
	SetFileAttr((DiskBasicFormatType)format_type, file_type, original_type);
	SetFileSize(file_size);
	SetStartAddress(start_addr);
	SetEndAddress(end_addr);
	SetExecuteAddress(exec_addr);
	SetExternalAttr(external_attr);
//	SetFileDateTime(&tm);
	if (tm) *tm = tm_tmp;

	return true;
}

/// 使用中のアイテムか
bool DiskBasicDirItem::IsUsed() const
{
	return ((m_flags & USED_ITEM) != 0);
//	return used;
}

/// 使用中かをセット
void DiskBasicDirItem::Used(bool val)
{
	m_flags = val ? (m_flags | USED_ITEM) : (m_flags & ~USED_ITEM);
}

/// リストに表示するアイテムか
bool DiskBasicDirItem::IsVisible() const
{
	return ((m_flags & VISIBLE_LIST) != 0);
}

/// リストに表示するかをセット
void DiskBasicDirItem::Visible(bool val)
{
	m_flags = val ? (m_flags | VISIBLE_LIST) : (m_flags & ~VISIBLE_LIST);
}

/// 使用中かつリストに表示するアイテムか
bool DiskBasicDirItem::IsUsedAndVisible() const
{
	return ((m_flags & (USED_ITEM | VISIBLE_LIST)) == (USED_ITEM | VISIBLE_LIST));
}

/// ツリーに表示するアイテムか
bool DiskBasicDirItem::IsVisibleOnTree() const
{
	return ((m_flags & VISIBLE_TREE) != 0);
}

/// ツリーに表示するかをセット
void DiskBasicDirItem::VisibleOnTree(bool val)
{
	m_flags = val ? (m_flags | VISIBLE_TREE) : (m_flags & ~VISIBLE_TREE);
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemAttr::DiskBasicDirItemAttr()
{
	m_rename = false;
	m_ignore_type = false;

	m_start_addr = -1;
	m_end_addr = -1;
	m_exec_addr = -1;

	m_ignore_date_time = false;

	m_user_data = 0;
}
DiskBasicDirItemAttr::~DiskBasicDirItemAttr()
{
}
void DiskBasicDirItemAttr::SetFileName(const wxString &n_name, int n_optional)
{
	m_name.SetName(n_name);
	m_name.SetOptional(n_optional);
}
void DiskBasicDirItemAttr::SetFileAttr(DiskBasicFormatType n_format, int n_type, int n_origin)
{
	m_type.SetFormat(n_format);
	m_type.SetType(n_type);
	m_type.SetOrigin(n_origin);
}
void DiskBasicDirItemAttr::SetFileType(int n_type)
{
	m_type.SetType(n_type);
}
int DiskBasicDirItemAttr::GetFileType() const
{
	return m_type.GetType();
}
void DiskBasicDirItemAttr::SetFileOriginAttr(int n_origin)
{
	m_type.SetOrigin(n_origin);
}
int DiskBasicDirItemAttr::GetFileOriginAttr() const
{
	return m_type.GetOrigin();
}
