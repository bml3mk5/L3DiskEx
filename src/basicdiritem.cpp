/// @file basicdiritem.cpp
///
/// @brief disk basic directory item
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

	this->num = 0;
	this->format_type = FORMAT_TYPE_NONE;
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
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic)
{
	this->basic = basic;
	this->type = basic->GetType();

	this->num = 0;
	this->format_type = basic->GetFormatType();
	this->position = 0;
	this->file_size = 0;
	this->groups = 0;
	this->sector = NULL;

	this->data = new directory_t;
	memset(this->data, 0, sizeof(directory_t));
	this->ownmake_data = true;

	this->external_attr = 0;

	this->used = false;
	this->visible = true;
}
/// ディレクトリアイテムを作成 DATAはディスクイメージをアサイン
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
{
	this->basic = basic;
	this->type = basic->GetType();

	this->num = 0;
	this->format_type = basic->GetFormatType();
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
DiskBasicDirItem::DiskBasicDirItem(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
{
	this->basic = basic;
	this->type = basic->GetType();

	this->num = num;
	this->format_type = basic->GetFormatType();
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
DiskBasicDirItem::~DiskBasicDirItem()
{
	if (ownmake_data) {
		delete data;
	}
}

/// 複製
/// データ部分は元がメモリを確保している場合、メモリを確保してコピー
/// 確保していない場合、ポインタをコピー
/// @param [in] src ソース
void DiskBasicDirItem::Dup(const DiskBasicDirItem &src)
{
	this->basic = src.basic;
	this->type = src.type;

	this->num = src.num;
	this->format_type = src.format_type;
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

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItem::Check(bool &last)
{
	return false;
}

/// 削除
/// @param [in] code : 削除コード(デフォルト0)
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

//
//
//

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItem::GetFileNamePos(size_t &len, bool *invert) const
{
	len = 0;
	return NULL;
}

/// 拡張子を格納する位置を返す
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
void DiskBasicDirItem::SetFileNamePlain(const wxString &filename)
{
	wxUint8 name[FILENAME_BUFSIZE];
	ToNativeName(filename, name, sizeof(name));
	SetFileName(name, sizeof(name));
}

/// 拡張子を設定
void DiskBasicDirItem::SetFileExt(const wxString &fileext)
{
	wxUint8 ext[FILEEXT_BUFSIZE];
	ToNativeExt(fileext, ext, sizeof(ext));
	SetFileExt(ext, sizeof(ext));
}

/// ファイル名をコピー
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
wxString DiskBasicDirItem::GetFileNameStr()
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

/// ファイル名を得る 名前 + "." + 拡張子
/// @note バッファを超える場合は拡張子を追加しない
/// @param [in] filename 出力先バッファ
/// @param [in] length   出力先バッファのサイズ
void DiskBasicDirItem::GetFileName(wxUint8 *filename, size_t length)
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
void DiskBasicDirItem::GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen)
{
	wxUint8 *n, *e;
	size_t nl, el;
	bool invert = false;
	char space = basic->GetDirSpaceCode();	// 空白コード

	n = GetFileNamePos(nl, &invert);
	e = GetFileExtPos(el);
	memset(name, 0, nlen); 
	memset(ext,  0, elen); 
	memcpy(name, n, nl); 
	if (e) memcpy(ext, e, el); 
	if (invert) {
		mem_invert(name, nl);
		mem_invert(ext,  el);
	}
	nl = rtrim(name, nl, space);
	el = rtrim(ext,  el, space);
	nlen = str_shrink(name, nl);
	elen = str_shrink(ext,  el);
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItem::InvalidateChars()
{
	return wxT("\"\\/:;");
}

/// 同じファイル名か
bool DiskBasicDirItem::IsSameFileName(const wxString &filename)
{
	if (!IsUsedAndVisible()) return false;

	wxUint8 sname[FILENAME_BUFSIZE], sext[FILEEXT_BUFSIZE];
	wxUint8 dname[FILENAME_BUFSIZE], dext[FILEEXT_BUFSIZE];
	size_t snlen = sizeof(sname);
	size_t selen = sizeof(sext);
	size_t dnlen = sizeof(dname);
	size_t delen = sizeof(dext);

	// ファイル名を内部ファイル名に変換
	ToNativeFileName(filename, dname, dnlen, dext, delen);
	// このアイテムのファイル名を取得
	GetFileName(sname, snlen, sext, selen);
	// 比較
	return (memcmp(sname, dname, snlen > dnlen ? snlen : dnlen) == 0)
		&& (selen == 0 || memcmp(sext, dext, selen > delen ? selen : delen) == 0);
}

/// 同じファイル名か
bool DiskBasicDirItem::IsSameFileName(const DiskBasicDirItem &src)
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

	return (l > 0 ? memcmp(sname, dname, l) == 0 : false);
}

/// ファイル名＋拡張子のサイズ
int DiskBasicDirItem::GetFileNameStrSize()
{
	int l = GetFileExtSize();
	if (l > 0) l++;
	l += GetFileNameSize();
	return l;
}

/// ファイルパスから内部ファイル名を生成する
/// エクスプローラからインポート時のダイアログを出す前
wxString DiskBasicDirItem::RemakeFileNameStr(const wxString &filepath)
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
/// コピー、このアプリからインポート時のダイアログを出す前
wxString DiskBasicDirItem::RemakeFileName(const wxUint8 *src, size_t srclen)
{
	wxString dst;
	basic->ConvCharsToString(src, srclen, dst);
	dst.Trim(true);
	return dst;
}

/// ファイル名を変換して内部ファイル名にする "."で拡張子と分別 大文字変換（機種依存）
/// ダイアログ入力後のファイル名文字列を変換する
/// @param [in]  filename   ファイル名(Unicode)
/// @param [out] nativename 内部ファイル名バッファ
/// @param [in]  length     バッファサイズ
void DiskBasicDirItem::ToNativeFileNameFromStr(const wxString &filename, wxUint8 *nativename, size_t length)
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
/// @return true OK
bool DiskBasicDirItem::ToNativeFileName(const wxString &filename, wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen)
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

/// 文字列をコード変換して内部ファイル名にする
bool DiskBasicDirItem::ToNativeName(const wxString &src, wxUint8 *dst, size_t len)
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
	valid = MemoryCopy((char *)tmp, l, (char)basic->GetDirSpaceCode(), dst, len);
	if (invert) {
		mem_invert(dst, len);
	}
	return valid;
}

/// 文字列をコード変換して内部ファイル名にする
bool DiskBasicDirItem::ToNativeExt(const wxString &src, wxUint8 *dst, size_t len)
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

/// 文字列をバッファにコピー あまりはfillでパディング
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
/// @param [in] file_type : -1のときは設定しない
void DiskBasicDirItem::SetFileAttr(int file_type)
{
}

/// 属性を返す
/// bit 0 : BASIC
/// bit 1 : Data
/// bit 2 : Machine
/// bit 3 : Ascii
/// bit 4 : Binary
/// bit 5 : Random access
/// bit 6 : Encrypted
/// bit 7 : Read after Write
/// bit 8 : Read Only
/// bit 9 : System
/// bit10 : Hidden
/// bit11 : Volume
/// bit12 : Directory
/// bit13 : Archive
int DiskBasicDirItem::GetFileType()
{
	return 0;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItem::GetFileAttrStr()
{
	return wxT("");
}

/// 通常のファイルか ディレクトリ削除でのチェックで使用
bool DiskBasicDirItem::IsNormalFile()
{
	int type = GetFileType();
	if (type & FILE_TYPE_VOLUME_MASK) {
		// ボリュームラベルは特殊
		return false;
	} else if (type & FILE_TYPE_DIRECTORY_MASK) {
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
bool DiskBasicDirItem::IsDirectory()
{
	int type = GetFileType();
	return ((type & (FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_VOLUME_MASK)) == FILE_TYPE_DIRECTORY_MASK);
}

/// ファイルサイズをセット
void DiskBasicDirItem::SetFileSize(int val)
{
	file_size = val;
}

int DiskBasicDirItem::GetFileSize()
{
	return file_size;
}

void DiskBasicDirItem::SetGroupSize(int val)
{
	groups = val;
}

int DiskBasicDirItem::GetGroupSize()
{
	return groups;
}

/// 最初のグループ番号をセット
void DiskBasicDirItem::SetStartGroup(wxUint32 val)
{
}
/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItem::GetStartGroup() const
{
	return 0;
}

/// 追加のグループ番号をセット(機種依存)
void DiskBasicDirItem::SetExtraGroup(wxUint32 val)
{
}
/// 追加のグループ番号を返す(機種依存)
wxUint32 DiskBasicDirItem::GetExtraGroup() const
{
	return INVALID_GROUP_NUMBER;
}

//wxUint32 DiskBasicDirItem::GetStartGroupNumber(DiskBasicFormatType format_type, const directory_t *data)
//{
//	return 0;
//}

/// 日付を返す
void DiskBasicDirItem::GetFileDate(struct tm *tm)
{
	tm->tm_year = 0;
	tm->tm_mon = 0;
	tm->tm_mday = 0;
}

/// 時間を返す
void DiskBasicDirItem::GetFileTime(struct tm *tm)
{
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
}

/// 日時を返す
void DiskBasicDirItem::GetFileDateTime(struct tm *tm)
{
	GetFileDate(tm);
	GetFileTime(tm);
}

/// 日時をセット
void DiskBasicDirItem::SetFileDateTime(const struct tm *tm)
{
	SetFileDate(tm);
	SetFileTime(tm);
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItem::GetFileDateTimeTitle()
{
	return _("Created Date:");
}

wxString DiskBasicDirItem::GetFileDateStr()
{
	return wxT("");
}

wxString DiskBasicDirItem::GetFileTimeStr()
{
	return wxT("");
}

wxString DiskBasicDirItem::GetFileDateTimeStr()
{
	wxString str = GetFileDateStr();
	if (!str.IsEmpty()) str += wxT(" ");
	str += GetFileTimeStr();
	if (str.IsEmpty()) str += wxT("---");
	return str;
}

/// ファイルの終端コードをチェックして必要なサイズを返す
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

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItem::SetModify()
{
	if (sector) sector->SetModify();
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItem::GetDataSize()
{
	return sizeof(directory_t);
}

/// アイテムへのポインタを設定
void DiskBasicDirItem::SetDataPtr(directory_t *val)
{
	if (ownmake_data) {
		delete data;
	}
	ownmake_data = false;
	data = val;
}

/// アイテムをコピー
/// @return false コピー先なし
bool DiskBasicDirItem::CopyData(const directory_t *val)
{
	if (!data) return false;
	memcpy(data, val, GetDataSize());
	return true;
}

/// ファイル名、属性をコピー
/// @param [in] src ソース
void DiskBasicDirItem::CopyItem(const DiskBasicDirItem &src)
{
	// データはコピーする
	CopyData(src.GetData());
	// その他の属性
	external_attr = src.external_attr;
}

/// 内部メモリを確保してアイテムをコピー
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
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItem::InitialData()
{
	ClearData();
}
