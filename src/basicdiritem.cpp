/// @file basicdiritem.cpp
///
/// @brief disk basic directory item
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"


//
//
//
DiskBasicDirItem::DiskBasicDirItem()
{
	this->basic = NULL;
	this->type = NULL;

	this->parent = NULL;
	this->children = NULL;
	this->valid_dir = false;

	this->num = 0;
	this->position = 0;
	this->file_size = 0;
	this->groups = 0;
	this->sector = NULL;

	this->data = NULL;
	this->ownmake_data = false;

	this->external_attr = 0;

	this->used = false;
	this->visible = true;
}
DiskBasicDirItem::DiskBasicDirItem(const DiskBasicDirItem &src)
{
	this->Dup(src);
}
DiskBasicDirItem &DiskBasicDirItem::operator=(const DiskBasicDirItem &src)
{
	this->Dup(src);
	return *this;
}
/// ディレクトリアイテムを作成 DATAは内部で確保
/// @param [in] basic  DISK BASIC
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic)
{
	this->basic = basic;
	this->type = basic->GetType();

	this->parent = NULL;
	this->children = NULL;
	this->valid_dir = false;

	this->num = 0;
	this->position = 0;
	this->file_size = 0;
	this->groups = 0;
	this->sector = NULL;

	this->data = new directory_t;
	memset(this->data, 0, sizeof(directory_t));
	if (basic->IsDataInverted()) mem_invert(this->data, sizeof(directory_t));
	this->ownmake_data = true;

	this->external_attr = 0;

	this->used = false;
	this->visible = true;
}
/// ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
/// @param [in] basic  DISK BASIC
/// @param [in] sector セクタ
/// @param [in] data   セクタ内のディレクトリエントリ
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
{
	this->basic = basic;
	this->type = basic->GetType();

	this->parent = NULL;
	this->children = NULL;
	this->valid_dir = false;

	this->num = 0;
	this->position = 0;
	this->file_size = 0;
	this->groups = 0;
	this->sector = sector; // no duplicate

	this->data = (directory_t *)data;	// no duplicate
	this->ownmake_data = false;

	this->external_attr = 0;

	this->used = false;
	this->visible = true;
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

	this->parent = NULL;
	this->children = NULL;
	this->valid_dir = false;

	this->num = num;
	this->position = secpos;
	this->file_size = 0;
	this->groups = 0;
	this->sector = sector; // no duplicate

	this->data = (directory_t *)data;	// no duplicate
	this->ownmake_data = false;

	this->external_attr = 0;

	this->used = false;
	this->visible = true;
}
/// デストラクタ
DiskBasicDirItem::~DiskBasicDirItem()
{
	if (ownmake_data) {
		delete data;
	}
	if (children) {
		for(size_t i=0; i<children->Count(); i++) {
			DiskBasicDirItem *child = children->Item(i);
			delete child;
		}
		children->Clear();
		delete children;
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
	this->num = n_num;
	this->position = n_secpos;
	this->sector = n_sector; // no duplicate

	if (this->ownmake_data) {
		delete this->data;
	}
	this->ownmake_data = false;
	this->data = (directory_t *)n_data;
}

/// 複製
/// データ部分は元がメモリを確保している場合、メモリを確保してコピー
/// 確保していない場合、ポインタをコピー
/// @param [in] src ソース
void DiskBasicDirItem::Dup(const DiskBasicDirItem &src)
{
	this->basic = src.basic;
	this->type = src.type;

	this->parent = src.parent;
	if (src.children) {
		this->children = new DiskBasicDirItems;
		this->children = src.children;	// no duplicate
	} else {
		this->children = NULL;
	}
	this->valid_dir = src.valid_dir;

	this->num = src.num;
	this->position = src.position;
	this->file_size = src.file_size;
	this->groups = src.groups;
	this->sector = src.sector;	// no duplicate

	if (src.ownmake_data) {
		this->data = new directory_t;
		memcpy(this->data, src.data, sizeof(directory_t));
	} else {
		this->data = src.data;	// no duplicate
	}
	this->ownmake_data = src.ownmake_data;

	this->external_attr = src.external_attr;

	this->used = src.used;
	this->visible = src.visible;
}

/// 子ディレクトリを追加
/// @param [in] newitem 新しいアイテム
void DiskBasicDirItem::AddChild(DiskBasicDirItem *newitem)
{
	if (!children) {
		children = new DiskBasicDirItems;
	}
	children->Add(newitem);
}
/// 子ディレクトリ一覧をクリア
void DiskBasicDirItem::EmptyChildren()
{
	if (children) {
		for(size_t i=0; i<children->Count(); i++) {
			DiskBasicDirItem *child = children->Item(i);
			delete child;
		}
		children->Empty();
	}
	valid_dir = false;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItem::Check(bool &last)
{
	size_t len = GetDataSize();
	wxUint8 *buf = (wxUint8 *)data;
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
	data->name[0] = code;
	used = false;
	return true;
}

/// 内部変数などを再設定
void DiskBasicDirItem::Refresh()
{
	used = CheckUsed(false);
}

/// アイテムを削除できるか
/// @return true 削除できる
bool DiskBasicDirItem::IsDeletable() const
{
	// ボリュームラベルは不可
	return GetFileAttr().UnmatchType(FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK, FILE_TYPE_VOLUME_MASK);
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
	return GetFileAttr().MatchType(FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK, 0);
}

//
//
//

/// ファイル名を格納する位置を返す
/// @param [out] len    バッファサイズ
/// @param [out] invert データを反転する必要があるか
/// @return 格納先バッファポインタ
wxUint8 *DiskBasicDirItem::GetFileNamePos(size_t &len, bool *invert) const
{
	len = 0;
	return NULL;
}

/// 拡張子を格納する位置を返す
/// @param [out] len    バッファサイズ
/// @return 格納先バッファポインタ
wxUint8 *DiskBasicDirItem::GetFileExtPos(size_t &len) const
{
	len = 0;
	return NULL;
}

/// ファイル名を設定 "."で拡張子と分離
/// @param [in]  filename   ファイル名(Unicode)
void DiskBasicDirItem::SetFileNameStr(const wxString &filename)
{
	wxUint8 name[FILENAME_BUFSIZE];
	ToNativeFileNameFromStr(filename, name, sizeof(name));
	SetFileName(name, sizeof(name));
}

/// ファイル名をそのまま設定
/// @param [in]  filename   ファイル名(Unicode)
void DiskBasicDirItem::SetFileNamePlain(const wxString &filename)
{
	wxUint8 name[FILENAME_BUFSIZE];
	ToNativeName(filename, name, sizeof(name));
	SetFileName(name, sizeof(name));
}

/// 拡張子を設定
/// @param [in]  fileext    拡張子(Unicode)
void DiskBasicDirItem::SetFileExt(const wxString &fileext)
{
	wxUint8 ext[FILEEXT_BUFSIZE];
	ToNativeExt(fileext, ext, sizeof(ext));
	SetFileExt(ext, sizeof(ext));
}

/// ファイル名をコピー
/// @param [in]  src ディレクトリアイテム
void DiskBasicDirItem::CopyFileName(const DiskBasicDirItem &src)
{
	wxUint8 sname[FILENAME_BUFSIZE], dname[FILENAME_BUFSIZE];
	wxUint8 *sn, *dn;
	size_t sl, dl;
	bool sinvert = false;
	bool dinvert = false;
	char space = basic->GetDirSpaceCode();	// 空白コード

	sn = src.GetFileNamePos(sl);
	sl += src.GetFileExtSize(&sinvert);
	memcpy(sname, sn, sl);
	if (sinvert) mem_invert(sname, sl);
	sl = rtrim(sname, sl, space);
	sl = str_shrink(sname, sl);

	dn = GetFileNamePos(dl);
	dl += GetFileExtSize(&dinvert);

	MemoryCopy((char *)sname, sl, space, dname, dl);

	if (dinvert) mem_invert(dname, dl);

	memcpy(dn, dname, dl);
}

/// ファイル名を設定
/// @param [in]  filename ファイル名
/// @param [in]  length   長さ
/// @note filename はデータビットが反転している場合あり
void DiskBasicDirItem::SetFileName(const wxUint8 *filename, int length)
{
	wxUint8 *n;
	size_t l;
	n = GetFileNamePos(l);
	l += GetFileExtSize();

	if (l > (size_t)length) l = (size_t)length;

	if (l > 0) {
		memset(n, 0, l);
		memcpy(n, filename, l);
	}
}

/// 拡張子を設定
/// @param [in]  fileext  拡張子
/// @param [in]  length   長さ
/// @note fileext はデータビットが反転している場合あり
void DiskBasicDirItem::SetFileExt(const wxUint8 *fileext, int length)
{
	wxUint8 *e;
	size_t el;
	e = GetFileExtPos(el);

	if (el > (size_t)length) el = (size_t)length;

	if (el > 0) {
		memset(e, 0, el);
		memcpy(e, fileext, el);
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

	GetFileName(name, nl, ext, el);

	wxString dst;

	basic->ConvCharsToString(name, nl, dst);

	if (el > 0) {
		dst += wxT(".");
		basic->ConvCharsToString(ext, el, dst);
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

	GetFileName(name, nl, NULL, el);

	wxString dst;

	basic->ConvCharsToString(name, nl, dst);

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

	GetFileName(NULL, nl, ext, el);

	wxString dst;

	basic->ConvCharsToString(ext, el, dst);

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

	GetFileName(name, nl, ext, el);

	memcpy(filename, name, length > nl ? nl : length);

	if (el > 0 && (nl + el + 1) < length) {
		// 拡張子ありの場合"."を追加
		// ただし、バッファを超える場合は拡張子を追加しない
		filename[nl] = '.';
		nl++;
		memcpy(&filename[nl], ext, el);
	}
}

/// ファイル名と拡張子を得る
/// それぞれ空白は右トリミング
/// @param [out]    name     ファイル名バッファ
/// @param [in,out] nlen     上記バッファサイズ / 文字列長さを返す
/// @param [out]    ext      拡張子名バッファ
/// @param [in,out] elen     上記バッファサイズ / 文字列長さを返す
void DiskBasicDirItem::GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	wxUint8 *n, *e;
	size_t nl, el;
	bool invert = false;
	char space = basic->GetDirSpaceCode();	// 空白コード

	n = GetFileNamePos(nl, &invert);
	e = GetFileExtPos(el);
	if (name && nlen > 0) {
		memset(name, 0, nlen); 
		if (n) memcpy(name, n, nl); 
		if (invert) {
			mem_invert(name, nl);
		}
		nl = rtrim(name, nl, space);
		nlen = str_shrink(name, nl);
	}
	if (ext && elen > 0) {
		memset(ext,  0, elen); 
		if (e) memcpy(ext, e, el); 
		if (invert) {
			mem_invert(ext,  el);
		}
		el = rtrim(ext,  el, space);
		elen = str_shrink(ext,  el);
	}
}

/// ファイル名に設定できない文字を文字列にして返す
/// @return 文字列
wxString DiskBasicDirItem::InvalidateChars() const
{
	return wxT("\"\\/:;");
}

/// ファイル名(拡張子除く)が一致するか
/// @param [in] name ファイル名
bool DiskBasicDirItem::IsSameName(const wxString &name) const
{
	if (!IsUsedAndVisible()) return false;

	wxUint8 sname[FILENAME_BUFSIZE];
	wxUint8 dname[FILENAME_BUFSIZE];
	size_t snlen = sizeof(sname);
	size_t dnlen = sizeof(dname);
	size_t selen = 0;

	// ファイル名を内部ファイル名に変換
	ToNativeName(name, dname, dnlen);
	dnlen = rtrim(dname, dnlen, basic->GetDirSpaceCode());
	dnlen = str_shrink(dname, dnlen);
	// このアイテムのファイル名を取得
	GetFileName(sname, snlen, NULL, selen);
	// 比較
	return (memcmp(sname, dname, snlen > dnlen ? snlen : dnlen) == 0);
}

/// 同じファイル名か
/// ファイル名＋拡張子＋拡張属性で一致するかどうか
/// @param [in] filename ファイル名
/// @see GetOptionalName()
bool DiskBasicDirItem::IsSameFileName(const DiskBasicFileName &filename) const
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
	GetFileName(sname, snlen, sext, selen);
	// 比較
	return (memcmp(sname, dname, snlen > dnlen ? snlen : dnlen) == 0)
		&& (selen == 0 || memcmp(sext, dext, selen > delen ? selen : delen) == 0)
		&& (GetOptionalName() == filename.GetOptional());
}

/// 同じファイル名か
/// ファイル名＋拡張子＋拡張属性で一致するかどうか
/// @param [in] src ディレクトリアイテム
/// @see GetOptionalName()
bool DiskBasicDirItem::IsSameFileName(const DiskBasicDirItem &src) const
{
	if (!IsUsedAndVisible()) return false;

	wxUint8 sname[FILENAME_BUFSIZE], dname[FILENAME_BUFSIZE];
	wxUint8 *sn, *dn;
	size_t sl, dl, l;
	bool sinvert = false;
	bool dinvert = false;
	char space = basic->GetDirSpaceCode();	// 空白コード

	sn = src.GetFileNamePos(sl);
	sl += src.GetFileExtSize(&sinvert);
	memcpy(sname, sn, sl);
	if (sinvert) mem_invert(sname, sl);
	sl = rtrim(sname, sl, space);
	sl = str_shrink(sname, sl);

	dn = GetFileNamePos(dl);
	dl += GetFileExtSize(&dinvert);
	memcpy(dname, dn, dl);
	if (dinvert) mem_invert(dname, dl);
	dl = rtrim(dname, dl, space);
	dl = str_shrink(dname, dl);

	l = (sl > dl ? sl : dl);

	return (l > 0 ? (memcmp(sname, dname, l) == 0) && (src.GetOptionalName() == GetOptionalName()) : false);
}

/// ファイル名＋拡張子のサイズ
/// @return サイズ
int DiskBasicDirItem::GetFileNameStrSize() const
{
	int l = GetFileExtSize();
	if (l > 0) l++;
	l += GetFileNameSize();
	return l;
}

/// ファイルパスから内部ファイル名を生成する
///
/// エクスプローラからインポート時のダイアログを出す前
/// 拡張子がない機種では拡張子はとり除かれる
///
/// @param [in] filepath ファイルパス
/// @return ファイル名
wxString DiskBasicDirItem::RemakeFileNameStr(const wxString &filepath) const
{
	wxString newname;
	wxFileName fn(filepath);

	int nl = GetFileNameSize();
	int el = GetFileExtSize();

	newname = fn.GetName().Left(nl);
	if (el > 0) {
		if (!fn.GetExt().IsEmpty()) {
			newname += wxT(".");
			newname += fn.GetExt().Left(el);
		}
	}

	// 大文字にする（機種依存）
	ConvertToFileNameStr(newname);

	return newname;
}

/// 内部ファイル名からコード変換して文字列を返す
///
/// コピー、このアプリからインポート時のダイアログを出す前
///
/// @param [in] src    ファイル名
/// @param [in] srclen ファイル名長さ
/// @return ファイル名
wxString DiskBasicDirItem::RemakeFileName(const wxUint8 *src, size_t srclen) const
{
	wxString dst;
	basic->ConvCharsToString(src, srclen, dst);
	dst.Trim(true);
	return dst;
}

/// ファイル名を変換して内部ファイル名にする "."で拡張子と分別 大文字変換（機種依存）
///
/// ダイアログ入力後のファイル名文字列を変換する
///
/// @param [in]  filename   ファイル名(Unicode)
/// @param [out] nativename 内部ファイル名バッファ
/// @param [in]  length     バッファサイズ
void DiskBasicDirItem::ToNativeFileNameFromStr(const wxString &filename, wxUint8 *nativename, size_t length) const
{
	char tmp[FILENAME_BUFSIZE];
	bool invert = false;
	size_t nl, el;

	wxString namestr = filename;

	// ダイアログ入力後のファイル名文字列を変換 大文字にする（機種依存）
	ConvertFromFileNameStr(namestr);

	if (!basic->ConvStringToChars(namestr, (wxUint8 *)tmp, sizeof(tmp))) return;

	nl = GetFileNameSize(&invert);
	el = GetFileExtSize();
	MemoryCopy(tmp, nl, el, (char)basic->GetDirSpaceCode(), nativename, length);
	if (invert) {
		mem_invert(nativename, length);
	}
	return;
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
	char tmp[FILENAME_BUFSIZE];
	bool invert = false;
	bool valid = true;
	size_t nl, el;
	char space = basic->GetDirSpaceCode();	// 空白コード

	wxString namestr = filename;

	// ダイアログ入力後のファイル名文字列を変換 大文字にする（機種依存）
	ConvertFromFileNameStr(namestr);

	if (!basic->ConvStringToChars(namestr, (wxUint8 *)tmp, sizeof(tmp))) return false;

	nl = GetFileNameSize(&invert);
	el = GetFileExtSize();
	valid = MemoryCopy((char *)tmp, nl, el, (char)basic->GetDirSpaceCode(), name, nlen);
	memset(ext, 0, elen);
	if (el > 0) {
		memcpy(ext, &name[nl], el);
	}
	nl = rtrim(name, nl, space);
	el = rtrim(ext,  el, space);
	nlen = str_shrink(name, nl);
	elen = str_shrink(ext,  el);
	return valid;
}

/// 文字列をコード変換して内部ファイル名(名前)にする
/// @param [in]   src     ファイル名(Unicode)
/// @param [out]  dst     内部ファイル名バッファ
/// @param [in]   len     上記バッファサイズ
/// @return true OK / false 変換できない文字がある
bool DiskBasicDirItem::ToNativeName(const wxString &src, wxUint8 *dst, size_t len) const
{
	wxUint8 tmp[FILENAME_BUFSIZE];
	bool valid = true;
	bool invert = false;
	size_t l;

	wxString namestr = src;

	// ダイアログ入力後のファイル名文字列を変換 大文字にする（機種依存）
	ConvertFromFileNameStr(namestr);

	if (!basic->ConvStringToChars(namestr, (wxUint8 *)tmp, sizeof(tmp))) return false;

	l = GetFileNameSize(&invert);
	l += GetFileExtSize();
	valid = MemoryCopy((char *)tmp, l, (char)basic->GetDirSpaceCode(), dst, len);
	if (invert) {
		mem_invert(dst, len);
	}
	return valid;
}

/// 文字列をコード変換して内部ファイル名(拡張子)にする
/// @param [in]   src     ファイル名(Unicode)
/// @param [out]  dst     内部ファイル名バッファ
/// @param [in]   len     上記バッファサイズ
/// @return true OK / false 変換できない文字がある
bool DiskBasicDirItem::ToNativeExt(const wxString &src, wxUint8 *dst, size_t len) const
{
	wxUint8 tmp[FILEEXT_BUFSIZE];
	bool valid = true;
	bool invert = false;
	size_t el;

	wxString extstr = src;

	// ダイアログ入力後のファイル名文字列を変換 大文字にする（機種依存）
	ConvertFromFileNameStr(extstr);

	if (!basic->ConvStringToChars(extstr, (wxUint8 *)tmp, sizeof(tmp))) return false;

	el = GetFileExtSize(&invert);
	valid = MemoryCopy((char *)tmp, el, (char)basic->GetDirSpaceCode(), dst, len);
	if (invert) {
		mem_invert(dst, len);
	}
	return valid;
}

/// 文字列をバッファにコピー 余りはfillで埋める
/// @param [in]   src     ファイル名
/// @param [in]   flen    ファイル名長さ
/// @param [in]   fill    余り部分を埋める文字
/// @param [out]  dst     出力先バッファ
/// @param [in]   len     上記バッファサイズ
/// @return true OK / false 出力先バッファが足りない
bool DiskBasicDirItem::MemoryCopy(const char *src, size_t flen, char fill, wxUint8 *dst, size_t len)
{
	size_t l;
	if (len < flen) return false;
	l = strlen(src);
	if (l > flen) l = flen;
	memset(dst, fill, len);
	memcpy(dst, src, l);
	return true;
}

/// 文字列をバッファにコピー "."で拡張子とを分ける
/// @param [in]   src     ファイル名
/// @param [in]   flen    ファイル名長さ
/// @param [in]   elen    拡張子長さ
/// @param [in]   fill    余り部分をサプレスする文字
/// @param [out]  dst     出力先バッファ
/// @param [in]   len     上記バッファサイズ
/// @return true OK / false 出力先バッファが足りない
bool DiskBasicDirItem::MemoryCopy(const char *src, size_t flen, size_t elen, char fill, wxUint8 *dst, size_t len)
{
	size_t l;
	if (len <= flen + elen) return false;
	memset(dst, fill, len);
	// .で分割する
	const char *p = strrchr(src, '.');
	if (elen > 0 && p != NULL) {
		l = (p - src);
		if (l > flen) l = flen;
		memcpy(dst, src, l);
		l = strlen(p+1);
		if (l > elen) l = elen;
		memcpy(&dst[flen], p+1, l);
	} else {
		l = strlen(src);
		if (l > flen) l = flen;
		memcpy(dst, src, l);
	}
	return true;
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
/// @param [in] file_type     共通属性 enum #en_file_type_mask の組み合わせ
/// @param [in] original_type 本来の属性
void DiskBasicDirItem::SetFileAttr(int file_type, int original_type)
{
	SetFileAttr(DiskBasicFileType(basic->GetFormatTypeNumber(), file_type, original_type));
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
	file_size = val;
}

/// ファイルサイズを返す
/// @return サイズ
int DiskBasicDirItem::GetFileSize() const
{
	return file_size;
}

/// グループ数をセット
/// @param [in] val 数
void DiskBasicDirItem::SetGroupSize(int val)
{
	groups = val;
}

/// グループ数を返す
/// @return 数
int DiskBasicDirItem::GetGroupSize() const
{
	return groups;
}

/// 最初のグループ番号をセット
/// @param [in] val 番号
void DiskBasicDirItem::SetStartGroup(wxUint32 val)
{
}
/// 最初のグループ番号を返す
/// @return 番号
wxUint32 DiskBasicDirItem::GetStartGroup() const
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

/// 日付を得る
/// @param [out] tm 日付
void DiskBasicDirItem::GetFileDate(struct tm *tm) const
{
	tm->tm_year = 0;
	tm->tm_mon = 0;
	tm->tm_mday = 0;
}

/// 時間を得る
/// @param [out] tm 時間
void DiskBasicDirItem::GetFileTime(struct tm *tm) const
{
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
}

/// 日時を得る
/// @param [out] tm 日時
void DiskBasicDirItem::GetFileDateTime(struct tm *tm) const
{
	GetFileDate(tm);
	GetFileTime(tm);
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
	if (c[0] != 0x1a) {
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
	if (!data) return false;
	memcpy(data, val, GetDataSize());
	return true;
}

/// ファイル名、属性をコピー
/// @param [in] src ディレクトリアイテム
void DiskBasicDirItem::CopyItem(const DiskBasicDirItem &src)
{
	// データはコピーする
	CopyData(src.GetData());
	// その他の属性
	external_attr = src.external_attr;
}

/// 内部メモリを確保してアイテムをコピー
/// @param [in] val ディレクトリアイテム
void DiskBasicDirItem::CloneData(const directory_t *val)
{
	if (!ownmake_data) {
		data = new directory_t;
	}
	ownmake_data = true;
	memcpy(data, val, GetDataSize());
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItem::ClearData()
{
	if (!data) return;
	int c = 0;
	size_t l;
	l = GetDataSize();
	memset(data, c, l);
	if (basic->IsDataInverted()) mem_invert(data, l);
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItem::InitialData()
{
	ClearData();
}
