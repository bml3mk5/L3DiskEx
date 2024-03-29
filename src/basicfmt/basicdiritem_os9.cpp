/// @file basicdiritem_os9.cpp
///
/// @brief disk basic directory item for OS-9
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_os9.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include "../config.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// OS-9属性名
const char *gTypeNameOS9[] = {
	wxTRANSLATE("<DIR>"),
	wxTRANSLATE("Non-sharable"),
	NULL
};
const char gTypeNameOS9_2[] = {
	'X','W','R','x','w','r',0
};
const char *gTypeNameOS9_2l[] = {
	wxTRANSLATE("Execute"),
	wxTRANSLATE("Write"),
	wxTRANSLATE("Read"),
	NULL
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemOS9FD::DiskBasicDirItemOS9FD()
{
	basic = NULL;
	sector = NULL;
	fd = NULL;
	mylsn = (wxUint32)-1;
	fd_ownmake = false;
	memset(&zero_data, 0, sizeof(zero_data));
}
DiskBasicDirItemOS9FD::~DiskBasicDirItemOS9FD()
{
	if (fd_ownmake) delete fd;
}
DiskBasicDirItemOS9FD::DiskBasicDirItemOS9FD(const DiskBasicDirItemOS9FD &src)
{
}
#ifdef COPYABLE_DIRITEM
/// 代入
DiskBasicDirItemOS9FD &DiskBasicDirItemOS9FD::operator=(const DiskBasicDirItemOS9FD &src)
{
	this->Dup(src);
	return *this;
}
/// 複製
void DiskBasicDirItemOS9FD::Dup(const DiskBasicDirItemOS9FD &src)
{
	sector = src.sector;
	mylsn = src.mylsn;
	if (src.fd_ownmake) {
		fd = new directory_os9_fd_t;
		memcpy(&fd, src.fd, sizeof(directory_os9_fd_t));
	} else {
		fd = src.fd;
	}
	fd_ownmake = src.fd_ownmake;
}
#endif
/// ポインタをセット
void DiskBasicDirItemOS9FD::Set(DiskBasic *n_basic, DiskImageSector *n_sector, wxUint32 n_mylsn, directory_os9_fd_t *n_fd)
{
	basic = n_basic;
	sector = n_sector;
	mylsn = n_mylsn;
	if (fd_ownmake) delete fd;
	fd = n_fd;
	fd_ownmake = false;
}
/// FDのメモリを確保
void DiskBasicDirItemOS9FD::Alloc()
{
	if (fd_ownmake) delete fd;
	fd = new directory_os9_fd_t;
	fd_ownmake = true;
	memset(fd, 0, sizeof(directory_os9_fd_t));
}
/// FDをクリア
void DiskBasicDirItemOS9FD::Clear()
{
	if (sector) sector->Fill(0);
	else if (fd) memset(fd, 0, sizeof(directory_os9_fd_t));
}
/// 属性を返す
wxUint8 DiskBasicDirItemOS9FD::GetATT() const
{
	return fd ? fd->FD_ATT : 0;
}
/// 属性をセット
void DiskBasicDirItemOS9FD::SetATT(wxUint8 val)
{
	if (fd) fd->FD_ATT = val;
}
/// セグメントのLSNを返す
wxUint32 DiskBasicDirItemOS9FD::GetLSN(int idx) const
{
	return fd ? GET_OS9_LSN(fd->FD_SEG[idx].LSN) : 0;
}
/// セグメントのセクタ数を返す
wxUint16 DiskBasicDirItemOS9FD::GetSIZ(int idx) const
{
	return fd ? wxUINT16_SWAP_ON_LE(fd->FD_SEG[idx].SIZ) : 0;
}
/// セグメントにLSNを設定
void DiskBasicDirItemOS9FD::SetLSN(int idx, wxUint32 val)
{
	if (fd) SET_OS9_LSN(fd->FD_SEG[idx].LSN, val);
}
/// セグメントにセクタ数を設定
void DiskBasicDirItemOS9FD::SetSIZ(int idx, wxUint16 val)
{
	if (fd) fd->FD_SEG[idx].SIZ = wxUINT16_SWAP_ON_LE(val);
}
/// ファイルサイズを返す
wxUint32 DiskBasicDirItemOS9FD::GetSIZ() const
{
	return fd ? wxUINT32_SWAP_ON_LE(fd->FD_SIZ) : 0;
}
/// ファイルサイズを設定
void DiskBasicDirItemOS9FD::SetSIZ(wxUint32 val)
{
	if (fd) fd->FD_SIZ = wxUINT32_SWAP_ON_LE(val);
}
/// リンク数を返す
wxUint8 DiskBasicDirItemOS9FD::GetLNK() const
{
	return fd ? fd->FD_LNK : 0;
}
/// リンク数を設定
void DiskBasicDirItemOS9FD::SetLNK(wxUint8 val)
{
	if (fd) fd->FD_LNK = val;
}
/// 更新日付を返す
const os9_date_t &DiskBasicDirItemOS9FD::GetDAT() const
{
	return fd ? fd->FD_DAT : zero_data.date;
}
/// 更新日付をセット
void DiskBasicDirItemOS9FD::SetDAT(const os9_date_t &val)
{
	if (fd) fd->FD_DAT = val;
}
/// 更新日付をセット
void DiskBasicDirItemOS9FD::SetDAT(const os9_cdate_t &val)
{
	fd->FD_DAT.yy = val.yy;
	fd->FD_DAT.mm = val.mm;
	fd->FD_DAT.dd = val.dd;
}
/// 作成日付を返す
const os9_cdate_t &DiskBasicDirItemOS9FD::GetDCR() const
{
	return fd ? fd->FD_DCR : zero_data.cdate;
}
/// 作成日付をセット
void DiskBasicDirItemOS9FD::SetDCR(const os9_cdate_t &val)
{
	if (fd) fd->FD_DCR = val;
}
/// 更新にする
void DiskBasicDirItemOS9FD::SetModify()
{
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemOS9::DiskBasicDirItemOS9(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
	fd.Alloc();
}
DiskBasicDirItemOS9::DiskBasicDirItemOS9(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
}
DiskBasicDirItemOS9::DiskBasicDirItemOS9(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);

	Used(CheckUsed(n_unuse));

	// FDセクタへのポインタをセット
	if (IsUsed()) {
		wxUint32 lsn = GET_OS9_LSN(((directory_os9_t *)n_data)->DE_LSN);
		if (lsn != 0) {
			DiskImageSector *sector = basic->GetSectorFromGroup(lsn);
			if (sector) {
				fd.Set(basic, sector, lsn, (directory_os9_fd_t *)sector->GetSectorBuffer());
			}
		}
	}

	CalcFileSize();

	// カレント or 親ディレクトリはツリーに表示しない
	wxString name = GetFileNamePlainStr();
	VisibleOnTree(!(IsDirectory() && (name == wxT(".") || name == wxT(".."))));
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemOS9::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemOS9::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->DE_NAM);
		return m_data.Data()->DE_NAM;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 属性１を返す
int	DiskBasicDirItemOS9::GetFileType1() const
{
	return fd.GetATT();
}

/// 属性１を設定
void DiskBasicDirItemOS9::SetFileType1(int val)
{
	fd.SetATT(val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemOS9::CheckUsed(bool unuse)
{
	return (m_data.Data()->DE_NAM[0] != 0);
}

/// ファイル名を設定
void DiskBasicDirItemOS9::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	size_t s, l;
	wxUint8 *n = GetFileNamePos(0, s, l);
	EncodeString(n, l, (const char *)filename, length);
}

/// 文字列の最後のMSBをセット
size_t DiskBasicDirItemOS9::EncodeString(wxUint8 *dst, size_t dlen, const char *src, size_t slen)
{
	memset(dst, 0, dlen);
	size_t len = dlen > slen ? slen : dlen;
	memcpy(dst, src, len);

	// 文字列の最後にMSBをセット
	for(int i = (int)len - 1; i >= 0; i--) {
		if (dst[i] != 0) {
			dst[i] |= 0x80;
			break;
		}
	}
	return len;
}

/// ファイル名を得る
void DiskBasicDirItemOS9::GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	DiskBasicDirItem::GetNativeFileName(name, nlen, ext, elen);

	// 文字列の最後はMSBがセットされているのでクリア
	nlen = DecodeString((char *)name, nlen, name, nlen);
}

/// 文字列のMSBをクリア
size_t DiskBasicDirItemOS9::DecodeString(char *dst, size_t dlen, const wxUint8 *src, size_t slen)
{
	size_t len = dlen > slen ? slen : dlen;

	// 文字列のMSBをクリア
	bool last = false;
	for(size_t i = 0; i < len; i++) {
		last = ((src[i] & 0x80) != 0);
		dst[i] = (src[i] & 0x7f);
		if (last) {
			dlen = i + 1;
			break;
		}
	}
	for(size_t i = dlen; i < len; i++) {
		dst[i] = 0;
	}
	return dlen;
}

#ifdef COPYABLE_DIRITEM
/// 複製
void DiskBasicDirItemOS9::Dup(const DiskBasicDirItem &src)
{
	DiskBasicDirItem::Dup(src);
	const DiskBasicDirItemOS9 *psrc = (const DiskBasicDirItemOS9 *)&src;
	fd.Dup(psrc->fd);
}
#endif

/// 削除
bool DiskBasicDirItemOS9::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data.Data()->DE_NAM[0] = basic->GetDeleteCode();
	Used(false);
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemOS9::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

//	if (m_data.Data()->DE_Reserved != 0) return false;

	return true;
}

/// 属性を設定
void DiskBasicDirItemOS9::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		t1 = file_type.GetOrigin();
	} else {
		if (ftype & FILE_TYPE_DIRECTORY_MASK) {
			t1 |= FILETYPE_MASK_OS9_DIRECTORY;
		}
		if (ftype & FILE_TYPE_NONSHARE_MASK) {
			t1 |= FILETYPE_MASK_OS9_NONSHARE;
		}
//		t1 |= ((ftype & FILETYPE_OS9_PERMISSION_MASK) >> FILETYPE_OS9_PERMISSION_POS);
		if (ftype & FILE_TYPE_BINARY_MASK) {
			t1 |= FILETYPE_MASK_OS9_PUBLIC_EXEC;
			t1 |= FILETYPE_MASK_OS9_USER_EXEC;
		}
		t1 |= FILETYPE_MASK_OS9_PUBLIC_READ;
		t1 |= FILETYPE_MASK_OS9_USER_WRITE;
		t1 |= FILETYPE_MASK_OS9_USER_READ;
	}
	SetFileType1(t1);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemOS9::GetFileAttr() const
{
	int val = 0;
	int t1 = GetFileType1();
	if (t1 & FILETYPE_MASK_OS9_DIRECTORY) {
		val |= FILE_TYPE_DIRECTORY_MASK;
	}
	if (t1 & FILETYPE_MASK_OS9_NONSHARE) {
		val |= FILE_TYPE_NONSHARE_MASK;
	}
	if ((t1 & (FILETYPE_MASK_OS9_PUBLIC_EXEC | FILETYPE_MASK_OS9_USER_EXEC)) != 0) {
		val |= FILE_TYPE_BINARY_MASK;
	}
//	val |= ((t1 << FILETYPE_OS9_PERMISSION_POS) & FILETYPE_OS9_PERMISSION_MASK);
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemOS9::GetFileAttrStr() const
{
	wxString str;
	if (fd.IsValid()) {
		if (fd.GetATT() & FILETYPE_MASK_OS9_DIRECTORY) {
			if (!str.IsEmpty()) str += wxT(", ");
			str += wxGetTranslation(gTypeNameOS9[TYPE_NAME_OS9_DIRECTORY]);
		}
		if (fd.GetATT() & FILETYPE_MASK_OS9_NONSHARE) {
			if (!str.IsEmpty()) str += wxT(", ");
			str += wxGetTranslation(gTypeNameOS9[TYPE_NAME_OS9_NONSHARE]);
		}
		if (!str.IsEmpty()) str += wxT(", ");
		for(int i=0; i<6; i++) {
			if (fd.GetATT() & (0x20 >> i)) {
				str += wxString(1, gTypeNameOS9_2[i]);
			} else {
				str += wxString(1, '-');
			}
		}
	}
	return str;
}

/// ファイルサイズをセット
void DiskBasicDirItemOS9::SetFileSize(int val)
{
	fd.SetSIZ(val);
	m_groups.SetSize(val);
}

/// ファイルサイズを返す
int DiskBasicDirItemOS9::GetFileSize() const
{
	size_t size = fd.GetSIZ();
	if (size == 0) size = m_groups.GetSize();
	return (int)size;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemOS9::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed() || !fd.IsValid()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemOS9::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	if (!fd.IsValid()) return;

	int calc_groups = 0;
	int calc_file_size = GetFileSize();

	for(int i=0; i<48; i++) {
		wxUint32 lsn = fd.GetLSN(i);
		int      siz = fd.GetSIZ(i);
		if (siz == 0) {
			break;
		}

		if (i != 0) {
			if (group_items.Count() > 0) {
				DiskBasicGroupItem *gitm = &group_items.Last();
				gitm->next = lsn;
			}
		}
		for(int n = 0; n < siz; n++) {
			int track_num = 0;
			int side_num = 0;
			int sector_num = 1;
			wxUint32 next_lsn = (n + 1 != siz ? lsn + n + 1 : 0);
			basic->CalcNumFromSectorPosForGroup(lsn + n, track_num, side_num, sector_num);
			group_items.Add(lsn + n, next_lsn, track_num, side_num, sector_num, sector_num);
			calc_groups++;
			if (calc_groups >= (int)basic->GetFatEndGroup()) {
				// too large block size
				break;
			}
		}
	}
	group_items.SetNums(calc_groups);
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize());
}

/// 作成日付を得る
void DiskBasicDirItemOS9::GetFileCreateDate(TM &tm) const
{
	if (fd.IsValid()) {
		ConvDateToTm(fd.GetDCR(), tm);
	} else {
		tm.SetYear(0);
		tm.SetMonth(0);
		tm.SetDay(0);
	}
}

/// 作成日付を文字列で返す
wxString DiskBasicDirItemOS9::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 作成日付をセット
void DiskBasicDirItemOS9::SetFileCreateDate(const TM &tm)
{
	if (fd.IsValid() && tm.GetYear() >= 0 && tm.GetMonth() >= 0) {
		os9_cdate_t date;
		ConvTmToDate(tm, date);
		fd.SetDCR(date);
	}
}

/// 更新日付を得る
void DiskBasicDirItemOS9::GetFileModifyDate(TM &tm) const
{
	if (fd.IsValid()) {
		ConvDateToTm((const os9_cdate_t &)fd.GetDAT(), tm);
	} else {
		tm.SetYear(0);
		tm.SetMonth(0);
		tm.SetDay(0);
	}
}

/// 更新時間を得る
void DiskBasicDirItemOS9::GetFileModifyTime(TM &tm) const
{
	if (fd.IsValid()) {
		ConvTimeToTm(fd.GetDAT(), tm);
	} else {
		tm.SetHour(0);
		tm.SetMinute(0);
	}
	tm.SetSecond(0);
}

/// 更新日付を文字列で返す
wxString DiskBasicDirItemOS9::GetFileModifyDateStr() const
{
	TM tm;
	GetFileModifyDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 更新時間を文字列で返す
wxString DiskBasicDirItemOS9::GetFileModifyTimeStr() const
{
	TM tm;
	GetFileModifyTime(tm);
	return Utils::FormatHMStr(tm);
}

/// 更新日付を設定
void DiskBasicDirItemOS9::SetFileModifyDate(const TM &tm)
{
	if (fd.IsValid() && tm.GetYear() >= 0 && tm.GetMonth() >= -1) {
		os9_cdate_t date;
		ConvTmToDate(tm, date);
		fd.SetDAT(date);
	}
}

/// 更新時間を設定
void DiskBasicDirItemOS9::SetFileModifyTime(const TM &tm)
{
	if (fd.IsValid() && tm.GetHour() >= 0 && tm.GetMinute() >= -1) {
		os9_date_t time = fd.GetDAT();
		ConvTmToTime(tm, time);
		fd.SetDAT(time);
	}
}

/// 日時の表示順序を返す（ダイアログ用）
int DiskBasicDirItemOS9::GetFileDateTimeOrder(int idx) const
{
	return idx <= 1 ? 1 - idx : idx;
}

/// 日時を返す（ファイルリスト用）
wxString DiskBasicDirItemOS9::GetFileDateTimeStr() const
{
	return GetFileModifyDateTimeStr();
}

/// 日付を変換
void DiskBasicDirItemOS9::ConvDateToTm(const os9_cdate_t &date, TM &tm) const
{
	tm.SetYear(date.yy % 100);
	if (tm.GetYear() < 80) tm.AddYear(100);
	tm.SetMonth(date.mm - 1);
	tm.SetDay(date.dd);
}
/// 時間を変換
void DiskBasicDirItemOS9::ConvTimeToTm(const os9_date_t &time, TM &tm) const
{
	tm.SetHour(time.hh);
	tm.SetMinute(time.mi);
}
/// 日付に変換
void DiskBasicDirItemOS9::ConvTmToDate(const TM &tm, os9_cdate_t &date) const
{
	date.yy = (tm.GetYear() % 100);
	date.mm = tm.GetMonth() + 1;
	date.dd = tm.GetDay();
}
/// 時間に変換
void DiskBasicDirItemOS9::ConvTmToTime(const TM &tm, os9_date_t &time) const
{
	time.hh = tm.GetHour();
	time.mi = tm.GetMinute();
}

/// 最初のグループ番号を設定
void DiskBasicDirItemOS9::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
//	if (fd) {
//		SET_OS9_LSN(fd->FD_SEG[0].LSN, val);
//	}
	SET_OS9_LSN(m_data.Data()->DE_LSN, val);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemOS9::GetStartGroup(int fileunit_num) const
{
//	wxUint32 val = 0;
//	if (fd) {
//		val = GET_OS9_LSN(fd->FD_SEG[0].LSN);
//	}
//	return val;
	return GET_OS9_LSN(m_data.Data()->DE_LSN);
}

/// 追加のグループ番号をセット FDセクタへのLSNをセット
void DiskBasicDirItemOS9::SetExtraGroup(wxUint32 val)
{
	SET_OS9_LSN(m_data.Data()->DE_LSN, val);
}

/// 追加のグループ番号を返す FDセクタへのLSNを返す
wxUint32 DiskBasicDirItemOS9::GetExtraGroup() const
{
	return GET_OS9_LSN(m_data.Data()->DE_LSN);
}

/// 追加のグループ番号を得る
void DiskBasicDirItemOS9::GetExtraGroups(wxArrayInt &arr) const
{
	arr.Add((int)GetExtraGroup());
}

/// チェイン用のセクタをセット
/// @param [in] sector セクタ
/// @param [in] lsn    セクタのLSN
/// @param [in] data   セクタ内のバッファ
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemOS9::SetChainSector(DiskImageSector *sector, wxUint32 lsn, wxUint8 *data, const DiskBasicDirItem *pitem)
{
	fd.Set(basic, sector, lsn, (directory_os9_fd_t *)sector->GetSectorBuffer());
	fd.Clear();

	// 属性をコピー
	if (pitem) CopyItem(*pitem);

	// リンク数
	fd.SetLNK(1);
}

bool DiskBasicDirItemOS9::IsDeletable() const
{
	// ".", ".."は不可
	bool valid = true;
	wxString name =	GetFileNamePlainStr();
	if (name == wxT(".") || name == wxT("..")) {
		valid = false;
	}
	return valid;
}
/// ファイル名を編集できるか
bool DiskBasicDirItemOS9::IsFileNameEditable() const
{
	// ".", ".."は不可
	return IsDeletable();
}
/// アイテムをロード・エクスポートできるか
/// @return true ロードできる
bool DiskBasicDirItemOS9::IsLoadable() const
{
	// ".", ".."は不可
	return IsDeletable();
}
/// アイテムをコピーできるか
/// @return true コピーできる
bool DiskBasicDirItemOS9::IsCopyable() const
{
	// ".", ".."は不可
	return IsDeletable();
}
/// アイテムを上書きできるか
/// @return true 上書きできる
bool DiskBasicDirItemOS9::IsOverWritable() const
{
	// ディレクトリは不可
	int t1 = GetFileType1();
	bool valid = ((t1 & FILETYPE_MASK_OS9_DIRECTORY) == 0);
	// ".", ".."は不可
	valid &= IsDeletable();
	return valid;
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemOS9::GetDataSize() const
{
	return sizeof(directory_os9_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemOS9::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemOS9::CopyData(const directory_t *val)
{
	return m_data.Copy(val, GetDataSize());
}

/// ディレクトリをクリア
void DiskBasicDirItemOS9::ClearData()
{
	m_data.Fill(basic->GetDeleteCode(), GetDataSize(), basic->IsDataInverted());
}

/// アイテムをコピー
void DiskBasicDirItemOS9::CopyItem(const DiskBasicDirItem &src)
{
	DiskBasicDirItem::CopyItem(src);
	const DiskBasicDirItemOS9FD *src_fd = &((const DiskBasicDirItemOS9 &)src).GetFD();
	fd.SetATT(src_fd->GetATT());
	fd.SetSIZ(src_fd->GetSIZ());
	fd.SetDAT(src_fd->GetDAT());
	fd.SetDCR(src_fd->GetDCR());
}

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemOS9::SetModify()
{
	DiskBasicDirItem::SetModify();
	fd.SetModify();
}

/// データをエクスポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemOS9::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	if (!IsDirectory()) {
		AddExtensionByFileAttr(GetFileAttr().GetType(), 0x3f, filename);
	}
	return true;
}

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemOS9::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimExtensionByExtensionAttr(filename);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemOS9::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;
	// -- --R -wr
	t1 |= FILETYPE_MASK_OS9_PUBLIC_READ;
	t1 |= FILETYPE_MASK_OS9_USER_WRITE;
	t1 |= FILETYPE_MASK_OS9_USER_READ;
	// 拡張子で実行属性を付ける
	wxFileName fn(filename);
	const MyAttribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt(), FILE_TYPE_BINARY_MASK, FILE_TYPE_BINARY_MASK);
	if (sa) {
		// 実行属性を付ける
		t1 |= FILETYPE_MASK_OS9_PUBLIC_EXEC;
		t1 |= FILETYPE_MASK_OS9_USER_EXEC;
	}

	return t1;
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"
#include "../ui/intnamevalid.h"


#define IDC_CHECK_DIRECTORY 51
#define IDC_CHECK_NONSHARE	52
#define IDC_CHECK_PUB_EXEC	53
#define IDC_CHECK_PUB_WRITE	54
#define IDC_CHECK_PUB_READ	55
#define IDC_CHECK_USR_EXEC	56
#define IDC_CHECK_USR_WRITE	57
#define IDC_CHECK_USR_READ	58
#define IDC_TEXT_CREATEDATE	59

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemOS9::GetFileType1Pos()
{
	return GetFileType1();
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemOS9::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = ConvOriginalTypeFromFileName(name);
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemOS9::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
//	int file_type_2 = 0;

	wxCheckBox *chkDirectory;
	wxCheckBox *chkSharable;

	wxCheckBox *chkPubExec;
	wxCheckBox *chkPubWrite;
	wxCheckBox *chkPubRead;
	wxCheckBox *chkUsrExec;
	wxCheckBox *chkUsrWrite;
	wxCheckBox *chkUsrRead;

//	wxTextCtrl *txtCDate;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

	chkDirectory = new wxCheckBox(parent, IDC_CHECK_DIRECTORY, wxGetTranslation(gTypeNameOS9[TYPE_NAME_OS9_DIRECTORY]));
	chkDirectory->SetValue((file_type_1 & FILETYPE_MASK_OS9_DIRECTORY) != 0);
	hbox->Add(chkDirectory, flags);

	chkSharable = new wxCheckBox(parent, IDC_CHECK_NONSHARE, wxGetTranslation(gTypeNameOS9[TYPE_NAME_OS9_NONSHARE]));
	chkSharable->SetValue((file_type_1 & FILETYPE_MASK_OS9_NONSHARE) != 0);
	hbox->Add(chkSharable, flags);
	staType4->Add(hbox);

	sizer->Add(staType4);

	wxStaticBoxSizer *staType5 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("Permission")), wxVERTICAL);

	wxSizerFlags vcenter = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);
	wxFlexGridSizer *gbox = new wxFlexGridSizer(4, 8, 8);

	gbox->Add(new wxStaticText(parent, wxID_ANY, _("Public")), vcenter);

	chkPubExec = new wxCheckBox(parent, IDC_CHECK_PUB_EXEC, wxGetTranslation(gTypeNameOS9_2l[0]));
	chkPubExec->SetValue((file_type_1 & FILETYPE_MASK_OS9_PUBLIC_EXEC) != 0);
	gbox->Add(chkPubExec);
	chkPubWrite = new wxCheckBox(parent, IDC_CHECK_PUB_WRITE, wxGetTranslation(gTypeNameOS9_2l[1]));
	chkPubWrite->SetValue((file_type_1 & FILETYPE_MASK_OS9_PUBLIC_WRITE) != 0);
	gbox->Add(chkPubWrite);
	chkPubRead = new wxCheckBox(parent, IDC_CHECK_PUB_READ, wxGetTranslation(gTypeNameOS9_2l[2]));
	chkPubRead->SetValue((file_type_1 & FILETYPE_MASK_OS9_PUBLIC_READ) != 0);
	gbox->Add(chkPubRead);
//	staType5->Add(hbox);

	gbox->Add(new wxStaticText(parent, wxID_ANY, _("Owner")), vcenter);

	chkUsrExec = new wxCheckBox(parent, IDC_CHECK_USR_EXEC, wxGetTranslation(gTypeNameOS9_2l[0]));
	chkUsrExec->SetValue((file_type_1 & FILETYPE_MASK_OS9_USER_EXEC) != 0);
	gbox->Add(chkUsrExec);
	chkUsrWrite = new wxCheckBox(parent, IDC_CHECK_USR_WRITE, wxGetTranslation(gTypeNameOS9_2l[1]));
	chkUsrWrite->SetValue((file_type_1 & FILETYPE_MASK_OS9_USER_WRITE) != 0);
	gbox->Add(chkUsrWrite);
	chkUsrRead = new wxCheckBox(parent, IDC_CHECK_USR_READ, wxGetTranslation(gTypeNameOS9_2l[2]));
	chkUsrRead->SetValue((file_type_1 & FILETYPE_MASK_OS9_USER_READ) != 0);
	gbox->Add(chkUsrRead);

	staType5->Add(gbox);
	sizer->Add(staType5, flags);
}

/// ダイアログ内の値を設定
void DiskBasicDirItemOS9::InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data)
{
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemOS9::ChangeTypeInAttrDialog(IntNameBox *parent)
{
}

//#define ATTR_CDATE_IDX 1

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemOS9::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxCheckBox *chkDirectory = (wxCheckBox *)parent->FindWindow(IDC_CHECK_DIRECTORY);
	wxCheckBox *chkSharable = (wxCheckBox *)parent->FindWindow(IDC_CHECK_NONSHARE);

	wxCheckBox *chkPubExec = (wxCheckBox *)parent->FindWindow(IDC_CHECK_PUB_EXEC);
	wxCheckBox *chkPubWrite = (wxCheckBox *)parent->FindWindow(IDC_CHECK_PUB_WRITE);
	wxCheckBox *chkPubRead = (wxCheckBox *)parent->FindWindow(IDC_CHECK_PUB_READ);
	wxCheckBox *chkUsrExec = (wxCheckBox *)parent->FindWindow(IDC_CHECK_USR_EXEC);
	wxCheckBox *chkUsrWrite = (wxCheckBox *)parent->FindWindow(IDC_CHECK_USR_WRITE);
	wxCheckBox *chkUsrRead = (wxCheckBox *)parent->FindWindow(IDC_CHECK_USR_READ);

	int t1 = 0;
	t1 |= (chkDirectory->GetValue() ? FILETYPE_MASK_OS9_DIRECTORY : 0);
	t1 |= (chkSharable->GetValue() ? FILETYPE_MASK_OS9_NONSHARE : 0);
	t1 |= (chkPubExec->GetValue() ? FILETYPE_MASK_OS9_PUBLIC_EXEC : 0);
	t1 |= (chkPubWrite->GetValue() ? FILETYPE_MASK_OS9_PUBLIC_WRITE : 0);
	t1 |= (chkPubRead->GetValue() ? FILETYPE_MASK_OS9_PUBLIC_READ : 0);
	t1 |= (chkUsrExec->GetValue() ? FILETYPE_MASK_OS9_USER_EXEC : 0);
	t1 |= (chkUsrWrite->GetValue() ? FILETYPE_MASK_OS9_USER_WRITE : 0);
	t1 |= (chkUsrRead->GetValue() ? FILETYPE_MASK_OS9_USER_READ : 0);

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t1);

	return true;
}

/// その他の属性値を設定する
void DiskBasicDirItemOS9::SetOptionalAttr(DiskBasicDirItemAttr &attr)
{
//	SetCDate(attr.GetCreateDateTime());
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemOS9::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	bool valid = true;
	wxString name =	filename;
	// ".",".."は設定できない
	if (name == wxT(".") || name == wxT("..")) {
		errormsg = wxString::Format(wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERRV_CANNOT_SET_NAME]), name);
		valid = false;
	}
	return valid;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemOS9::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());

	vals.Add(wxT("DE_NAM"), m_data.Data()->DE_NAM, sizeof(m_data.Data()->DE_NAM));
	vals.Add(wxT("DE_Reserved"), m_data.Data()->DE_Reserved);
	vals.Add(wxT("DE_LSN"), &m_data.Data()->DE_LSN, sizeof(m_data.Data()->DE_LSN));

	const DiskBasicDirItemOS9FD *cfd = &GetFD();
	if (!cfd->IsValid()) return;

	const directory_os9_fd_t *fd = cfd->GetFD();

	vals.Add(wxT("FD_ATT"), fd->FD_ATT);
	vals.Add(wxT("FD_OWN"), fd->FD_OWN, true);
	vals.Add(wxT("FD_DAT"), &fd->FD_DAT, sizeof(fd->FD_DAT));
	vals.Add(wxT("FD_LNK"), fd->FD_LNK);
	vals.Add(wxT("FD_SIZ"), fd->FD_SIZ, true);
	vals.Add(wxT("FD_DCR"), &fd->FD_DCR, sizeof(fd->FD_DCR));
}
