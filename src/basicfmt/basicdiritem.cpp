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
#include "../charcodes.h"
#include "../config.h"


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

	m_external_attr = 0;

	m_flags = (VISIBLE_LIST | VISIBLE_TREE);
}
/// ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
/// @param [in]  basic    DISK BASIC
/// @param [in]  n_sector セクタ
/// @param [in]  n_secpos セクタ内の位置
/// @param [in]  n_data   セクタ内のディレクトリエントリ
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
{
	this->basic = basic;
	this->type = basic->GetType();

	m_parent = NULL;
	m_children = NULL;
	m_valid_dir = false;

	m_num = 0;
	m_position = n_secpos;
//	m_file_size = 0;
	m_sector = n_sector; // no duplicate

	m_external_attr = 0;

	m_flags = (VISIBLE_LIST | VISIBLE_TREE);
}
/// ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
/// @param [in]  basic    DISK BASIC
/// @param [in]  n_num    通し番号
/// @param [in]  n_gitem  トラック番号などのデータ
/// @param [in]  n_sector セクタ
/// @param [in]  n_secpos セクタ内のディレクトリエントリの位置
/// @param [in]  n_data   セクタ内のディレクトリエントリ
/// @param [in]  n_next   次のセクタ
/// @param [out] n_unuse  未使用か
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
{
	this->basic = basic;
	this->type = basic->GetType();

	m_parent = NULL;
	m_children = NULL;
	m_valid_dir = false;

	m_num = n_num;
	m_position = n_secpos;
//	m_file_size = 0;
	m_sector = n_sector; // no duplicate

	m_external_attr = 0;

	m_flags = (VISIBLE_LIST | VISIBLE_TREE);
}
/// デストラクタ
DiskBasicDirItem::~DiskBasicDirItem()
{
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
/// @param [in] n_num       通し番号
/// @param [in] n_gitem     トラック番号などのデータ
/// @param [in] n_sector    セクタ
/// @param [in] n_secpos    セクタ内のディレクトリエントリの位置
/// @param [in] n_data      ディレクトリアイテム
/// @param [in] n_next      次のセクタ
void DiskBasicDirItem::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	m_num = n_num;
	m_position = n_secpos;
	m_sector = n_sector; // no duplicate
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

/// 子ディレクトリを作成
void DiskBasicDirItem::CreateChildren()
{
	if (!m_children) {
		m_children = new DiskBasicDirItems;
	}
}

/// 子ディレクトリを追加
/// @param [in] newitem 新しいアイテム
void DiskBasicDirItem::AddChild(DiskBasicDirItem *newitem)
{
	CreateChildren();
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
bool DiskBasicDirItem::CheckData(const wxUint8 *buf, size_t len, bool &last)
{
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
		ToUpper(sname, snlen);
		ToUpper(dname, dnlen);
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
		ToUpper(sname, snlen);
		ToUpper(sext, selen);
		ToUpper(dname, dnlen);
		ToUpper(dext, delen);
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
		ToUpper(sname, snlen);
		ToUpper(sext, selen);
		ToUpper(dname, dnlen);
		ToUpper(dext, delen);
	}
	// 比較
	return (memcmp(sname, dname, snlen > dnlen ? snlen : dnlen) == 0)
		&& ((delen == 0 && selen == 0) || memcmp(sext, dext, selen > delen ? selen : delen) == 0)
		&& (src->GetOptionalName() == GetOptionalName());
}

/// 小文字を大文字にする
/// @param [in,out] str  文字列
/// @param [in]     size 長さ
void DiskBasicDirItem::ToUpper(wxUint8 *str, size_t size) const
{
	to_upper(str, size);
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
///
/// @param[in]  pos         位置(負のときは、外部定義からさがす)
/// @param[in]  list        名＆値リスト
/// @param[out] attr        一致した名前
/// @param[in]  default_pos 一致しないとき設定するデフォルト名のある位置(-1のときは設定しない)
/// @return リストに一致したらtrue
bool DiskBasicDirItem::GetFileAttrName(int pos, const name_value_t *list, wxString &attr, int default_pos) const
{
	bool match = true;
	if (pos >= 0) {
		attr = wxGetTranslation(list[pos].name);
	} else {
		const L3Attribute *sa = basic->GetSpecialAttributes().FindValue(-pos);
		if (sa != NULL) {
			attr = sa->GetName();
		} else {
			match = false;
			if (default_pos >= 0) {
				attr = wxGetTranslation(list[default_pos].name);
			}
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

/// 拡張子から属性を決定できるか
///
/// 決定できる場合、拡張子が２つつながっている場合は末尾の拡張子をとり除く
///
/// インポートするファイルのファイル名に適用
///
/// @param [in] filename   ファイル名
/// @param [in] list       拡張子名＆値リスト
/// @param [in] list_first リストの最初の位置
/// @param [in] list_last  リストの最後の位置
/// @param [out] outfile   編集後のファイル名
/// @param [out] attr      属性
/// @param [out] pos       リストの位置 (ユーザ指定属性の場合はマイナスになる)
/// @par For Example:
///  foobar.aaa.bas (is basic file) -> foobar.aaa (trimming last extension) @n
///  foobar.bas (is basic file) -> foobar.bas (no change filename) @n
///  foobar.bbb.ccc (is unknown attr) -> foobar.bbb.ccc (no change filename) @n
bool DiskBasicDirItem::TrimLastExtensionByExtensionAttr(const wxString &filename, const name_value_t *list, int list_first, int list_last, wxString *outfile, int *attr, int *pos) const
{
	bool match = false;
	int t1 = 0;
	int p1 = -1;
	wxFileName fn(filename);
	// 拡張子リストに一致するか
	int idx = list->IndexOf(list, fn.GetExt().Upper()); 
	if (idx >= list_first && idx <= list_last) {
		match = true;
		t1 = list[idx].value;
		p1 = idx;
	} else {
		const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
		if (sa) {
			match = true;
			t1 = sa->GetType();
		}
	}
	if (match) {
		// 拡張子部分をとり除く
		if (outfile) *outfile = fn.GetName();
		if (attr) *attr = t1;
		if (pos) *pos = p1;
	}
	return match;
}

/// 拡張子から属性を決定できるか 決定できる場合、末尾の拡張子をとり除く
///
/// インポートするファイルのファイル名に適用
///
/// @param [in] filename   ファイル名
/// @param [in] list       拡張子名＆値リスト
/// @param [in] list_first リストの最初の位置
/// @param [in] list_last  リストの最後の位置
/// @param [out] outfile   編集後のファイル名
/// @param [out] attr      属性
/// @param [out] pos       リストの位置 (ユーザ指定属性の場合はマイナスになる)
/// @par For Example:
///  foobar.aaa.bas (is basic file) -> foobar.aaa (trimming last extension) @n
///  foobar.bas (is basic file) -> foobar (trimming last extension) @n
///  foobar.bbb.ccc (is unknown attr) -> foobar.bbb.ccc (no change filename) @n
bool DiskBasicDirItem::IsContainAttrByExtension(const wxString &filename, const name_value_t *list, int list_first, int list_last, wxString *outfile, int *attr, int *pos) const
{
	bool match = false;
	int t1 = 0;
	int p1 = -1;
	wxFileName fn(filename);
	// 拡張子リストに一致するか
	int idx = list->IndexOf(list, fn.GetExt().Upper()); 
	if (idx >= list_first && idx <= list_last) {
		match = true;
		t1 = list[idx].value;
		p1 = idx;
	} else {
		// ユーザ指定の属性か
		const L3Attribute *sa = basic->GetSpecialAttributes().FindUpperCase(fn.GetExt());
		if (sa) {
			match = true;
			t1 = sa->GetValue();
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
		if (pos) *pos = p1;
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
//	ConvertFileNameAfterRenamed(namestr);

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
/// @param [in] format_type    フォーマット種類 DiskBasicFormatType
/// @param [in] file_type      共通属性 enum #en_file_type_mask の組み合わせ
/// @param [in] original_type0 本来の属性
/// @param [in] original_type1 本来の属性 つづき1
/// @param [in] original_type2 本来の属性 つづき2
void DiskBasicDirItem::SetFileAttr(DiskBasicFormatType format_type, int file_type, int original_type0, int original_type1, int original_type2)
{
	SetFileAttr(DiskBasicFileType(format_type, file_type, original_type0, original_type1, original_type2));
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

/// 追加のグループリストをセット(機種依存)
void DiskBasicDirItem::SetExtraGroups(const DiskBasicGroups &grps)
{
}

/// 追加のグループ番号を得る(機種依存)
void DiskBasicDirItem::GetExtraGroups(wxArrayInt &arr) const
{
}

/// 追加のグループリストを返す(機種依存)
void DiskBasicDirItem::GetExtraGroups(DiskBasicGroups &grps) const
{
}

/// @brief 次のグループ番号をセット(機種依存)
/// @param [in] val 番号
void DiskBasicDirItem::SetNextGroup(wxUint32 val)
{
}

/// @brief 次のグループ番号を返す(機種依存)
wxUint32 DiskBasicDirItem::GetNextGroup() const
{
	return INVALID_GROUP_NUMBER;
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

/// 親のグループ番号をセット(機種依存)
/// @param [in] val 番号
void DiskBasicDirItem::SetParentGroup(wxUint32 val)
{
}

/// 親のグループ番号を返す(機種依存)
/// @return 番号
wxUint32 DiskBasicDirItem::GetParentGroup() const
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

/// 作成日付を得る
/// @param [out] tm 日付
void DiskBasicDirItem::GetFileCreateDate(TM &tm) const
{
	tm.ClearDate();
}

/// 作成時間を得る
/// @param [out] tm 時間
void DiskBasicDirItem::GetFileCreateTime(TM &tm) const
{
	tm.ClearTime();
}

/// 作成日時を得る
/// @param [out] tm 日時
void DiskBasicDirItem::GetFileCreateDateTime(TM &tm) const
{
	GetFileCreateDate(tm);
	GetFileCreateTime(tm);
}

/// 作成日時を返す
TM DiskBasicDirItem::GetFileCreateDateTime() const
{
	TM tm;
	GetFileCreateDateTime(tm);
	return tm;
}

/// 作成日時をセット
/// @param [in] tm 日時
void DiskBasicDirItem::SetFileCreateDateTime(const TM &tm)
{
	SetFileCreateDate(tm);
	SetFileCreateTime(tm);
}

/// 作成日付のタイトル名（ダイアログ用）
/// @return タイトル文字列
wxString DiskBasicDirItem::GetFileCreateDateTimeTitle() const
{
	return _("Created Date");
}

/// ファイルの作成日付を文字列にして返す
/// @return 日付文字列
wxString DiskBasicDirItem::GetFileCreateDateStr() const
{
	return wxT("");
}

/// ファイルの作成時間を文字列にして返す
/// @return 時間文字列
wxString DiskBasicDirItem::GetFileCreateTimeStr() const
{
	return wxT("");
}

/// ファイルの作成日時を文字列にして返す
/// @return 日時文字列 ない場合"---"
wxString DiskBasicDirItem::GetFileCreateDateTimeStr() const
{
	wxString str = GetFileCreateDateStr();
	if (!str.IsEmpty()) str += wxT(" ");
	str += GetFileCreateTimeStr();
	if (str.IsEmpty()) str += wxT("---");
	return str;
}

/// 変更日付を得る
/// @param [out] tm 日付
void DiskBasicDirItem::GetFileModifyDate(TM &tm) const
{
	tm.ClearDate();
}

/// 変更時間を得る
/// @param [out] tm 時間
void DiskBasicDirItem::GetFileModifyTime(TM &tm) const
{
	tm.ClearTime();
}

/// 変更日時を得る
/// @param [out] tm 日時
void DiskBasicDirItem::GetFileModifyDateTime(TM &tm) const
{
	GetFileModifyDate(tm);
	GetFileModifyTime(tm);
}

/// 変更日時を返す
TM DiskBasicDirItem::GetFileModifyDateTime() const
{
	TM tm;
	GetFileModifyDateTime(tm);
	return tm;
}

/// 変更日時をセット
/// @param [in] tm 日時
void DiskBasicDirItem::SetFileModifyDateTime(const TM &tm)
{
	SetFileModifyDate(tm);
	SetFileModifyTime(tm);
}

/// 変更日付のタイトル名（ダイアログ用）
/// @return タイトル文字列
wxString DiskBasicDirItem::GetFileModifyDateTimeTitle() const
{
	return _("Modified Date");
}

/// ファイルの変更日付を文字列にして返す
/// @return 日付文字列
wxString DiskBasicDirItem::GetFileModifyDateStr() const
{
	return wxT("");
}

/// ファイルの変更時間を文字列にして返す
/// @return 時間文字列
wxString DiskBasicDirItem::GetFileModifyTimeStr() const
{
	return wxT("");
}

/// ファイルの変更日時を文字列にして返す
/// @return 日時文字列 ない場合"---"
wxString DiskBasicDirItem::GetFileModifyDateTimeStr() const
{
	wxString str = GetFileModifyDateStr();
	if (!str.IsEmpty()) str += wxT(" ");
	str += GetFileModifyTimeStr();
	if (str.IsEmpty()) str += wxT("---");
	return str;
}

/// アクセス日付を得る
/// @param [out] tm 日付
void DiskBasicDirItem::GetFileAccessDate(TM &tm) const
{
	tm.ClearDate();
}

/// アクセス時間を得る
/// @param [out] tm 時間
void DiskBasicDirItem::GetFileAccessTime(TM &tm) const
{
	tm.ClearTime();
}

/// アクセス日時を得る
/// @param [out] tm 日時
void DiskBasicDirItem::GetFileAccessDateTime(TM &tm) const
{
	GetFileAccessDate(tm);
	GetFileAccessTime(tm);
}

/// アクセス日時を返す
TM DiskBasicDirItem::GetFileAccessDateTime() const
{
	TM tm;
	GetFileAccessDateTime(tm);
	return tm;
}

/// アクセス日時をセット
/// @param [in] tm 日時
void DiskBasicDirItem::SetFileAccessDateTime(const TM &tm)
{
	SetFileAccessDate(tm);
	SetFileAccessTime(tm);
}

/// アクセス日付のタイトル名（ダイアログ用）
/// @return タイトル文字列
wxString DiskBasicDirItem::GetFileAccessDateTimeTitle() const
{
	return _("Accessed Date");
}

/// ファイルのアクセス日付を文字列にして返す
/// @return 日付文字列
wxString DiskBasicDirItem::GetFileAccessDateStr() const
{
	return wxT("");
}

/// ファイルのアクセス時間を文字列にして返す
/// @return 時間文字列
wxString DiskBasicDirItem::GetFileAccessTimeStr() const
{
	return wxT("");
}

/// ファイルのアクセス日時を文字列にして返す
/// @return 日時文字列 ない場合"---"
wxString DiskBasicDirItem::GetFileAccessDateTimeStr() const
{
	wxString str = GetFileAccessDateStr();
	if (!str.IsEmpty()) str += wxT(" ");
	str += GetFileAccessTimeStr();
	if (str.IsEmpty()) str += wxT("---");
	return str;
}

/// 日時の表示順序を返す（ダイアログ用）
int DiskBasicDirItem::GetFileDateTimeOrder(int idx) const
{
	return idx;
}

/// 日時を返す（ファイルリスト用）
wxString DiskBasicDirItem::GetFileDateTimeStr() const
{
	return GetFileCreateDateTimeStr();
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

	AddExtensionByFileAttr(GetFileAttr().GetType(), FILE_TYPE_EXTENSION_MASK, filename);
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

	TM ctm;
	GetFileCreateDateTime(ctm);
	if (ctm.IsValidTime()) {
		wxXmlNode *time1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("CreateTime"));
		time1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::FormatHMSStr(ctm)));
	}

	if (ctm.IsValidDate()) {
		wxXmlNode *date1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("CreateDate"));
		date1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::FormatYMDStr(ctm)));
	}

	TM mtm;
	GetFileModifyDateTime(mtm);
	if (mtm.IsValidTime()) {
		wxXmlNode *time1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("ModifyTime"));
		time1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::FormatHMSStr(mtm)));
	}

	if (mtm.IsValidDate()) {
		wxXmlNode *date1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("ModifyDate"));
		date1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::FormatYMDStr(mtm)));
	}

	TM atm;
	GetFileAccessDateTime(atm);
	if (atm.IsValidTime()) {
		wxXmlNode *time1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("AccessTime"));
		time1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::FormatHMSStr(atm)));
	}

	if (atm.IsValidDate()) {
		wxXmlNode *date1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, wxT("AccessDate"));
		date1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), Utils::FormatYMDStr(atm)));
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

	for(int i=2; i>=0; i--) {
		wxString label = wxString::Format(wxT("OriginalType%d"), i);
		wxXmlNode *orig1 = new wxXmlNode(root, wxXML_ELEMENT_NODE, label);
		orig1->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxT(""), wxString::Format(wxT("0x%x"), (int)file_type.GetOrigin(i))));
	}

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
/// @param [out]    attr    アイテム内の属性(日時保持用)
/// @return true:正常  false:ファイル読み込めない
bool DiskBasicDirItem::ReadFileAttrFromXml(const wxString &path, DiskBasicDirItemAttr *attr)
{
	wxXmlDocument doc;

	if (!wxFileName::FileExists(path)) return false;
	if (!doc.Load(path)) return false;

	// 各パラメータをセット
	wxXmlNode *root = doc.GetRoot();
	if (!root) return false;

	int format_type = FORMAT_TYPE_UNKNOWN;
	int file_type = 0;
	int original_type0 = 0;
	int original_type1 = 0;
	int original_type2 = 0;
	int file_size = 0;
	int start_addr = -1;
	int end_addr = -1;
	int exec_addr = -1;
	int external_attr = 0;
	TM tm_ctmp;
	TM tm_mtmp;
	TM tm_atmp;

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
		} else if (name == wxT("OriginalType0")) {
			original_type0 = Utils::ToInt(content);
		} else if (name == wxT("OriginalType1")) {
			original_type1 = Utils::ToInt(content);
		} else if (name == wxT("OriginalType2")) {
			original_type2 = Utils::ToInt(content);
		} else if (name == wxT("Size")) {
			file_size = Utils::ToInt(content);
		} else if (name == wxT("StartAddress")) {
			start_addr = Utils::ToInt(content);
		} else if (name == wxT("EndAddress")) {
			end_addr = Utils::ToInt(content);
		} else if (name == wxT("ExecuteAddress")) {
			exec_addr = Utils::ToInt(content);
		} else if (name == wxT("CreateDate")) {
			Utils::ConvDateStrToTm(content, tm_ctmp);
		} else if (name == wxT("CreateTime")) {
			Utils::ConvTimeStrToTm(content, tm_ctmp);
		} else if (name == wxT("ModifyDate")) {
			Utils::ConvDateStrToTm(content, tm_mtmp);
		} else if (name == wxT("ModifyTime")) {
			Utils::ConvTimeStrToTm(content, tm_mtmp);
		} else if (name == wxT("AccessDate")) {
			Utils::ConvDateStrToTm(content, tm_atmp);
		} else if (name == wxT("AccessTime")) {
			Utils::ConvTimeStrToTm(content, tm_atmp);
		} else if (name == wxT("ExternalAttribute")) {
			external_attr = Utils::ToInt(content);
		}
		node = node->GetNext();
	}

	fnl = str_length(fname, fnl, 0);
	fel = str_length(fext, fel, 0);
	SetNativeFileName(fname, sizeof(fname), fnl, fext, sizeof(fext), fel);
	SetFileAttr((DiskBasicFormatType)format_type, file_type, original_type0, original_type1, original_type2);
	SetFileSize(file_size);
	SetStartAddress(start_addr);
	SetEndAddress(end_addr);
	SetExecuteAddress(exec_addr);
	SetExternalAttr(external_attr);
//	SetFileCreateDateTime(&tm);
	if (attr) {
		attr->SetCreateDateTime(tm_ctmp);
		attr->SetModifyDateTime(tm_mtmp);
		attr->SetAccessDateTime(tm_atmp);
	}

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

//
//
//

//#include <wx/choice.h>

/// 属性の選択肢を作成する（プロパティダイアログ用）
/// @param[in]  basic     DiskBasic
/// @param[in]  list      拡張子名＆値リスト
/// @param[in]  end_pos   リストの最終位置
/// @param[out] types     選択肢リスト
/// @param[in]  file_type 属性値(optional)
/// @note 属性値を設定した場合、属性値がリストに一致しなければそれを選択肢に追加する。
void DiskBasicDirItem::CreateChoiceForAttrDialog(DiskBasic *basic, const name_value_t *list, int end_pos, wxArrayString &types, int file_type)
{
	bool match_value = false;
	for(size_t i=0; i<(size_t)end_pos; i++) {
		types.Add(wxGetTranslation(list[i].name));
		if (file_type >= 0 && list[i].value == file_type) {
			match_value = true;
		}
	}
	const L3Attributes *attrs = &basic->GetSpecialAttributes();
	for(size_t i=0; i<attrs->Count(); i++) {
		const L3Attribute *attr = &attrs->Item(i);
		wxString str;
		str += attr->GetName();
		str += wxString::Format(wxT(" 0x%02x"), attr->GetValue());
		if (!attr->GetDescription().IsEmpty()) {
			str += wxT(" (");
			str += attr->GetDescription();
			str += wxT(")");
		}
		types.Add(str);
		if (file_type >= 0 && attr->GetValue() == file_type) {
			match_value = true;
		}
	}
	// リストにない属性値を選択肢に追加する
	if (file_type >= 0 && !match_value) {
		types.Add(wxString::Format(wxT("0x%02x"), file_type));
	}
}

/// 属性の選択肢を選ぶ（プロパティダイアログ用）
/// @param[in]  basic       DiskBasic
/// @param[in]  sel_pos     選択位置(負の場合は、外部設定の属性位置)
/// @param[in]  end_pos     リストの最終位置
/// @param[in]  unknown_pos "不明"の位置
/// @return 選択肢の位置
int  DiskBasicDirItem::SelectChoiceForAttrDialog(DiskBasic *basic, int sel_pos, int end_pos, int unknown_pos)
{
	if (sel_pos < 0) {
		const L3Attributes *attrs = &basic->GetSpecialAttributes();
		int n_type = attrs->GetIndexByValue(-sel_pos);
		if (n_type >= 0) {
			sel_pos = n_type + end_pos;
		}
	}
	if (sel_pos < 0) {
		sel_pos = unknown_pos;
	}

	return sel_pos;
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int DiskBasicDirItem::CalcSpecialOriginalTypeFromPos(DiskBasic *basic, int pos, int end_pos)
{
	int val = -1;
	if (pos >= end_pos) {
		pos -= end_pos;
		const L3Attributes *attrs = &basic->GetSpecialAttributes();
		int count = (int)attrs->Count();
		if (pos < count) {
			val = attrs->GetValueByIndex(pos);
		}
	}
	return val;
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int DiskBasicDirItem::CalcSpecialFileTypeFromPos(DiskBasic *basic, int pos, int end_pos)
{
	int t = 0;
	if (pos >= end_pos) {
		pos -= end_pos;
		const L3Attributes *attrs = &basic->GetSpecialAttributes();
		int count = (int)attrs->Count();
		if (pos < count) {
			t = attrs->GetTypeByIndex(pos);
		}
	}
	return t;
}

/// プロパティで表示する内部データを設定
void DiskBasicDirItem::SetCommonDataInAttrDialog(KeyValArray &vals) const
{
	vals.Add(wxT("num"), m_num);
	vals.Add(wxT("position"), (wxUint32)m_position);
	vals.Add(wxT("flags"), (wxUint32)m_flags);
	vals.Add(wxT("external_attr"), (wxUint32)m_external_attr);
}

//////////////////////////////////////////////////////////////////////
//
// 属性値を一時的に集めておくクラス
//
DiskBasicDirItemAttr::DiskBasicDirItemAttr()
{
	m_rename = false;
	m_ignore_type = false;

	m_start_addr = -1;
	m_end_addr = -1;
	m_exec_addr = -1;

	m_ignore_date_time = false;

//	m_user_data[0] = 0;
//	m_user_data[1] = 0;
}
DiskBasicDirItemAttr::~DiskBasicDirItemAttr()
{
}
/// ファイル名をセット
/// @param[in] n_name     ファイル名
/// @param[in] n_optional ファイル名の属性
void DiskBasicDirItemAttr::SetFileName(const wxString &n_name, int n_optional)
{
	m_name.SetName(n_name);
	m_name.SetOptional(n_optional);
}
/// 属性をセット
/// @param[in] n_format  フォーマットタイプ
/// @param[in] n_type    共通属性
/// @param[in] n_origin0 独自属性
/// @param[in] n_origin1 独自属性 つづき1
/// @param[in] n_origin2 独自属性 つづき2
void DiskBasicDirItemAttr::SetFileAttr(DiskBasicFormatType n_format, int n_type, int n_origin0, int n_origin1, int n_origin2)
{
	m_type.SetFormat(n_format);
	m_type.SetType(n_type);
	m_type.SetOrigin(0, n_origin0);
	m_type.SetOrigin(1, n_origin1);
	m_type.SetOrigin(2, n_origin2);
}
/// 共通属性をセット
/// @param[in] n_type    共通属性
void DiskBasicDirItemAttr::SetFileType(int n_type)
{
	m_type.SetType(n_type);
}
/// 共通属性を得る
int DiskBasicDirItemAttr::GetFileType() const
{
	return m_type.GetType();
}
/// 独自属性をセット
/// @param[in] idx      0..2
/// @param[in] n_origin 独自属性
void DiskBasicDirItemAttr::SetFileOriginAttr(int idx, int n_origin)
{
	m_type.SetOrigin(idx, n_origin);
}
/// 独自属性をセット
/// @param[in] n_origin 独自属性
void DiskBasicDirItemAttr::SetFileOriginAttr(int n_origin)
{
	m_type.SetOrigin(n_origin);
}
/// 独自属性を得る
/// @param[in] idx      0..2
int DiskBasicDirItemAttr::GetFileOriginAttr(int idx) const
{
	return m_type.GetOrigin(idx);
}

//////////////////////////////////////////////////////////////////////
//
// セクタをまたぐディレクトリアイテムを処理
//
DirItemSectorBoundary::DirItemSectorBoundary()
{
	Clear();
}

DirItemSectorBoundary::~DirItemSectorBoundary()
{
}

/// 初期化
void DirItemSectorBoundary::Clear()
{
	for(int i=0; i<2; i++) {
		s[i].data = NULL;
		s[i].size = 0;
		s[i].pos  = 0xffff;
	}
}

/// アイテムの位置情報をセット
/// @param[in] basic      Disk Basic
/// @param[in] sector     セクタ
/// @param[in] position   アイテムの位置
/// @param[in] item_data  アイテムデータ
/// @param[in] item_size  アイテムサイズ
/// @param[in] next       次のセクタ
/// @return セクタまたぎがある場合true
bool DirItemSectorBoundary::Set(DiskBasic *basic, DiskD88Sector *sector, int position, directory_t *item_data, size_t item_size, const SectorParam *next)
{
	if (!sector) return false;

	int spos = position;
	int ssize = sector->GetSectorSize();
	wxUint8 *sptr = (wxUint8 *)item_data;
	if (item_data) {
		s[0].data = sptr;
		s[0].size = (int)item_size;
		s[0].pos  = 0;
	}
	if (spos + (int)item_size >= ssize) {
		// セクタまたぎ
		s[1].pos = ssize - spos;
		s[1].size = s[0].size - s[1].pos;
		s[0].size = s[1].pos;

		// 次のセクタ
		if (next) {
			DiskBasicType *type = basic->GetType();
			int nsector_pos = type->GetSectorPosFromNum(next->GetTrackNumber(), next->GetSideNumber(), next->GetSectorNumber());
			DiskD88Sector *nsector = basic->GetSector(next->GetTrackNumber(), next->GetSideNumber(), next->GetSectorNumber());
			if (nsector) {
				sptr = nsector->GetSectorBuffer();
				ssize = nsector->GetSectorSize();
				spos = basic->GetDirStartPosOnSector();
				spos += (nsector_pos % basic->GetSectorsPerGroup()) == 0 ? basic->GetDirStartPosOnGroup() : 0;
				s[1].data = (sptr + spos - s[1].pos);
			}
		}

		return true;
	}
	return false;
}

/// コピー
/// @param[out] dst_item アイテムデータ
void DirItemSectorBoundary::CopyTo(directory_t *dst_item)
{
	wxUint8 *dst = (wxUint8 *)dst_item;
	for(int i=0; i<2; i++) {
		wxUint8 *src = s[i].data;
		if (dst && src) {
			memcpy(&dst[s[i].pos], &src[s[i].pos], s[i].size);
		}
	}
}

/// コピー
/// @param[in] src_item アイテムデータ
void DirItemSectorBoundary::CopyFrom(const directory_t *src_item)
{
	wxUint8 *src = (wxUint8 *)src_item;
	for(int i=0; i<2; i++) {
		wxUint8 *dst = s[i].data;
		if (dst && src) {
			memcpy(&dst[s[i].pos], &src[s[i].pos], s[i].size);
		}
	}
}
