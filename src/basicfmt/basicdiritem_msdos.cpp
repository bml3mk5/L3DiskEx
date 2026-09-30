/// @file basicdiritem_msdos.cpp
///
/// @brief disk basic directory item for MS-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_msdos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include "../utils.h"

//#define USE_VFAT_LFN 1

//////////////////////////////////////////////////////////////////////
//
//
//

/// MS-DOS (MSX-DOS)
const name_value_t gTypeNameMS[] = {
	{ "RO", FILE_TYPE_READONLY_MASK },
	{ "Hid", FILE_TYPE_HIDDEN_MASK },
	{ "Sys", FILE_TYPE_SYSTEM_MASK },
	{ "<VOL>", FILE_TYPE_VOLUME_MASK },
	{ "<DIR>", FILE_TYPE_DIRECTORY_MASK },
	{ "Arc", FILE_TYPE_ARCHIVE_MASK },
	{ "(LFN)", FILE_TYPE_READONLY_MASK | FILE_TYPE_HIDDEN_MASK | FILE_TYPE_SYSTEM_MASK | FILE_TYPE_VOLUME_MASK },
	{ NULL, -1 }
};
/// MS-DOS (MSX-DOS)
const name_value_t gTypeNameMS_l[] = {
	{ wxTRANSLATE("Read Only"), FILE_TYPE_READONLY_MASK },
	{ wxTRANSLATE("Hidden"), FILE_TYPE_HIDDEN_MASK },
	{ wxTRANSLATE("System"), FILE_TYPE_SYSTEM_MASK },
	{ wxTRANSLATE("Volume Label"), FILE_TYPE_VOLUME_MASK },
	{ wxTRANSLATE("Directory"), FILE_TYPE_DIRECTORY_MASK },
	{ wxTRANSLATE("Archive"), FILE_TYPE_ARCHIVE_MASK },
	{ wxTRANSLATE("Long File Name"), FILE_TYPE_READONLY_MASK | FILE_TYPE_HIDDEN_MASK | FILE_TYPE_SYSTEM_MASK | FILE_TYPE_VOLUME_MASK },
	{ NULL, -1 }
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
}
DiskBasicDirItemMSDOS::DiskBasicDirItemMSDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	// MS-DOS
	m_data.Attach(n_data);
	Used(CheckUsed(n_unuse));
	Visible((GetFileType1() & FILETYPE_MASK_MS_LFN) != FILETYPE_MASK_MS_LFN);
	n_unuse = (n_unuse || (m_data.Data()->msdos.name[0] == 0));

	// グループ数を計算
	CalcFileSize();

	// カレント or 親ディレクトリはツリーに表示しない
	wxString name = GetFileNamePlainStr();
	VisibleOnTree(!(IsDirectory() && (name == wxT(".") || name == wxT(".."))));
}

/// 初期状態に戻す
void DiskBasicDirItemMSDOS::Reset()
{
	m_data.Delete();

	DiskBasicDirItem::Reset();

	m_data.Alloc();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemMSDOS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemMSDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// MS-DOS
	int t1 = GetFileType1();
	if (num == 0) {
		size = len = sizeof(m_data.Data()->msdos.name);
		return m_data.Data()->msdos.name;
	} else if (num == 1) {
		if (t1 & FILETYPE_MASK_MS_VOLUME) {
			// ボリュームラベルは拡張子もラベル名とする
			size = len = sizeof(m_data.Data()->msdos.ext);
			return m_data.Data()->msdos.ext;
		}
	}
	size = len = 0;
	return NULL;
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemMSDOS::GetFileExtPos(size_t &len) const
{
	wxUint8 *p = NULL;
	len = 0;
	// ボリュームラベルは拡張子なしにする
	if (!(GetFileType1() & FILETYPE_MASK_MS_VOLUME)) {
		len = sizeof(m_data.Data()->msdos.ext);
		p = m_data.Data()->msdos.ext;
	}
	return p;
}

/// 属性１を返す
int	DiskBasicDirItemMSDOS::GetFileType1() const
{
	return m_data.Data()->msdos.type;
}

/// 属性１を設定
void DiskBasicDirItemMSDOS::SetFileType1(int val)
{
	m_data.Data()->msdos.type = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemMSDOS::CheckUsed(bool unuse)
{
	wxUint8 ch = m_data.Data()->msdos.name[0];
	return (ch != 0 && ch != 0xe5);
}

/// ファイル名を設定
void DiskBasicDirItemMSDOS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	if (length > 0) {
		// 0xe5は削除コードなので0x05に変換(Sjift JISなど2バイト系文字など)
		if (filename[0] == 0xe5) filename[0] = 0x05;
	}

	wxUint8 *n;
	size_t nl = 0;
	size_t s, l;

	int num = 0;
	do {
		s = l = 0;
		n = GetFileNamePos(num, s, l);
		if (n == NULL || s == 0) {
			break;
		}
		if (num > 0) {
			size_t pl = nl;
			size_t ps = s;
			if (nl < length) {
				pl = length;
				ps = s + nl - length;
			}
			if (s + nl > length) {
				memset(&filename[pl], 0, ps);
			}
		}

		if (s > size) s = size;
		memcpy(n, &filename[nl], s);

		nl += s;
		size -= s;
		num++;
	} while(num <= 1);
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemMSDOS::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t nl = 0;
	int num = 0;
	do {
		wxUint8 *dname = &filename[nl];
		size_t s, l;

		n = GetFileNamePos(num, s, l);
		if (n == NULL || s == 0) {
			break;
		}
		if (s > size) s = size;
		memcpy(dname, n, s);

		nl += s;
		size -= s;
		num++;
	} while(num <= 1);

	if (nl > 0) {
		// 0x05を0xe5に変換(Shift JISなど2バイト系文字など)
		if (filename[0] == 0x05) filename[0] = 0xe5;
	}

	length = nl;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemMSDOS::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;
	// ファイルサイズが16MBを超えている
	if (CheckUsed(false) && wxUINT32_SWAP_ON_BE(m_data.Data()->msdos.file_size) > 0xffffff) {
		valid = false;
	}
	return valid;
}

/// 削除できるか
bool DiskBasicDirItemMSDOS::IsDeletable() const
{
	// ".", ".."は不可
	bool valid = true;
	wxString name =	GetFileNamePlainStr();
	if (name == wxT(".") || name == wxT("..")) {
		valid = false;
	}
	return valid;
}

/// 削除
bool DiskBasicDirItemMSDOS::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data.Fill(basic->InvertUint8(basic->GetDeleteCode()), 1);
	Used(false);
	return true;
}

/// ファイル名を編集できるか
bool DiskBasicDirItemMSDOS::IsFileNameEditable() const
{
	// ".", ".."は不可
	return IsDeletable();
}

/// アイテムをロード・エクスポートできるか
/// @return true ロードできる
bool DiskBasicDirItemMSDOS::IsLoadable() const
{
	// ボリュームラベルは不可
	int t1 = GetFileType1();
	bool valid = ((t1 & (FILETYPE_MASK_MS_DIRECTORY | FILETYPE_MASK_MS_VOLUME)) != FILETYPE_MASK_MS_VOLUME);
	// ".", ".."は不可
	valid &= IsDeletable();
	return valid;
}

/// アイテムをコピーできるか
/// @return true コピーできる
bool DiskBasicDirItemMSDOS::IsCopyable() const
{
	// ".", ".."は不可
	return IsDeletable();
}

/// アイテムを上書きできるか
/// @return true 上書きできる
bool DiskBasicDirItemMSDOS::IsOverWritable() const
{
	// ディレクトリ、ボリュームラベルは不可
	int t1 = GetFileType1();
	bool valid = ((t1 & (FILETYPE_MASK_MS_DIRECTORY | FILETYPE_MASK_MS_VOLUME)) == 0);
	// ".", ".."は不可
	valid &= IsDeletable();
	return valid;
}

/// 属性を設定
void DiskBasicDirItemMSDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	// MS-DOS
	SetFileType1((ftype & 0xff00) >> 8);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemMSDOS::GetFileAttr() const
{
	int t1 = GetFileType1();
	return DiskBasicFileType(basic->GetFormatTypeNumber(), t1 << 8, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMSDOS::GetFileAttrStr() const
{
	wxString attr;
	int ftype = GetFileAttr().GetType();
	// MS-DOS
	GetFileAttrStrSub(ftype, attr);
	if (attr.IsEmpty()) {
		attr = wxT("---");
	}
	return attr;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
void DiskBasicDirItemMSDOS::GetFileAttrStrSub(int ftype, wxString &attr) const
{
	for(int i=0; i<=TYPE_NAME_MS_ARCHIVE; i++) {
		if (ftype & gTypeNameMS[i].value) {
			if (!attr.IsEmpty()) attr += wxT(", ");
			attr += wxGetTranslation(gTypeNameMS[i].name);
		}
	}
}

/// ファイルサイズをセット
void DiskBasicDirItemMSDOS::SetFileSize(int val)
{
	m_groups.SetSize(val);
	m_data.Data()->msdos.file_size = wxUINT32_SWAP_ON_BE(val);
}

/// ファイルサイズを返す
int DiskBasicDirItemMSDOS::GetFileSize() const
{
	int val = m_data.Data()->msdos.file_size;
	return wxUINT32_SWAP_ON_BE(val);
}

/// ディレクトリサイズをセット
/// MS-DOSでは常に0をセットする
void DiskBasicDirItemMSDOS::SetDirectorySize(int val)
{
	SetFileSize(0);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemMSDOS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemMSDOS::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	int calc_groups = 0; 
	int calc_file_size = GetFileSize();

	// 12bit FAT
	bool rc = true;
	wxUint32 group_num = GetStartGroup(fileunit_num);
	bool working = (group_num >= 2);
	int remain = (calc_file_size > 0 ? calc_file_size : 0x7ffffff);
	int limit = basic->GetFatEndGroup() + 1;
	while(working) {
		wxUint32 next_group = type->GetGroupNumber(group_num);
		if (next_group == group_num) {
			// 同じポジションならエラー
			rc = false;
		} else if (next_group >= 0xff8) {
			// 最終グループ
			working = false;
		} else if (next_group > (wxUint32)basic->GetFatEndGroup()) {
			// グループ番号がおかしい
			rc = false;
		}
		if (rc) {
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), remain, group_items);
//			m_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			remain -= (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			calc_groups++;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}

	group_items.SetNums(calc_groups);
	// 元のファイルサイズが０ならグループ数から計算したサイズを格納
	group_items.SetSize(calc_file_size > 0 ? calc_file_size : basic->GetSectorSize() * basic->GetSectorsPerGroup() * calc_groups);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
}

/// 更新日付を得る
void DiskBasicDirItemMSDOS::GetFileModifyDate(TM &tm) const
{
	// MS-DOS
	wxUint16 wdate = wxUINT16_SWAP_ON_BE(m_data.Data()->msdos.wdate);
	ConvDateToTm(wdate, tm);
}

/// 更新時間を得る
void DiskBasicDirItemMSDOS::GetFileModifyTime(TM &tm) const
{
	// MS-DOS
	wxUint16 wtime = wxUINT16_SWAP_ON_BE(m_data.Data()->msdos.wtime);
	ConvTimeToTm(wtime, tm);
}

/// 更新日付を文字列で返す
wxString DiskBasicDirItemMSDOS::GetFileModifyDateStr() const
{
	TM tm;
	GetFileModifyDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 更新時間を文字列で返す
wxString DiskBasicDirItemMSDOS::GetFileModifyTimeStr() const
{
	TM tm;
	GetFileModifyTime(tm);
	return Utils::FormatHMSStr(tm);
}

/// 更新日付をセット
void DiskBasicDirItemMSDOS::SetFileModifyDate(const TM &tm)
{
	if (tm.GetYear() >= 0 && tm.GetMonth() >= -1) {
		wxUint16 wdate = ConvTmToDate(tm);
		m_data.Data()->msdos.wdate = wxUINT16_SWAP_ON_BE(wdate);
	}
}

/// 更新時間をセット
void DiskBasicDirItemMSDOS::SetFileModifyTime(const TM &tm)
{
	if (tm.GetHour() >= 0 && tm.GetMinute() >= 0) {
		wxUint16 wtime = ConvTmToTime(tm);
		m_data.Data()->msdos.wtime = wxUINT16_SWAP_ON_BE(wtime); 
	}
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItemMSDOS::GetFileModifyDateTimeTitle() const
{
	return _("Updated Date");
}

/// 日時の表示順序を返す（ダイアログ用）
int DiskBasicDirItemMSDOS::GetFileDateTimeOrder(int idx) const
{
	return idx <= 1 ? 1 - idx : idx;
}

/// 日時を返す（ファイルリスト用）
wxString DiskBasicDirItemMSDOS::GetFileDateTimeStr() const
{
	return GetFileModifyDateTimeStr();
}

/// 日付を変換
void DiskBasicDirItemMSDOS::ConvDateToTm(wxUint16 date, TM &tm)
{
	int yy = ((date & 0xfe00) >> 9) + 80;
//	if (yy >= 128) yy -= 28; 
	tm.SetYear(yy);
	tm.SetMonth(((date & 0x01e0) >> 5) - 1);
	tm.SetDay(date & 0x001f);
}
/// 時間を変換
void DiskBasicDirItemMSDOS::ConvTimeToTm(wxUint16 time, TM &tm)
{
	tm.SetHour((time & 0xf800) >> 11);
	tm.SetMinute((time & 0x07e0) >> 5);
	tm.SetSecond((time & 0x001f) << 1);
}
/// 日付に変換
wxUint16 DiskBasicDirItemMSDOS::ConvTmToDate(const TM &tm)
{
	int yy = tm.GetYear();
//	if (yy >= 100) yy += 28;
	return (wxUint16)
		(((yy - 80) & 0x7f) << 9)
		| (((tm.GetMonth() + 1) & 0xf) << 5)
		| (tm.GetDay() & 0x1f);
}
/// 時間に変換
wxUint16 DiskBasicDirItemMSDOS::ConvTmToTime(const TM &tm)
{
	return (wxUint16)
		((tm.GetHour() & 0x1f) << 11)
		| ((tm.GetMinute() & 0x3f) << 5)
		| ((tm.GetSecond() & 0x3f) >> 1);
}

size_t DiskBasicDirItemMSDOS::GetDataSize() const
{
	return sizeof(directory_ms_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemMSDOS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemMSDOS::CopyData(const directory_t *val)
{
	return m_data.Copy(val, GetDataSize());
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemMSDOS::ClearData()
{
	m_data.Fill(0, GetDataSize(), basic->IsDataInverted());
}

/// 最初のグループ番号を設定
void DiskBasicDirItemMSDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// MS-DOS
	m_data.Data()->msdos.start_group = wxUINT16_SWAP_ON_BE(val);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemMSDOS::GetStartGroup(int fileunit_num) const
{
	// MS-DOS
	return wxUINT16_SWAP_ON_BE(m_data.Data()->msdos.start_group);
}

/// ファイル名から属性を決定する
int DiskBasicDirItemMSDOS::ConvFileTypeFromFileName(const wxString &filename) const
{
	return FILE_TYPE_ARCHIVE_MASK;
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemMSDOS::NeedCheckEofCode()
{
	// テキストファイルかは拡張子で判断する
	bool rc = false;
	const MyAttribute *sa = basic->GetAttributesByExtension().FindUpperCase(GetFileExtPlainStr());
	if (sa) {
		rc = ((sa->GetType() & FILE_TYPE_ASCII_MASK) != 0); 
	}
	return rc;
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合など
int DiskBasicDirItemMSDOS::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイル終端に終端文字があるか
		wxFileOffset curr_pos = istream->TellI();
		istream->SeekI(-1, wxFromEnd);
		if (istream->GetC() != basic->GetTextTerminateCode()) {
			file_size++;
		}
		istream->SeekI(curr_pos);
	}
	return file_size;
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemMSDOS::ValidateFileName(const wxString &filename, wxString &errormsg)
{
	bool valid = true;
	wxString name =	filename;
	// "."で始まる名前は設定できない
	if (name.Left(1) == wxT(".")) {
		errormsg = wxString::Format(wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERRV_CANNOT_SET_NAME]), name);
		valid = false;
	}
	return valid;
}

#ifndef USE_CONSOLE

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_CHECK_ATTR	51
//#define IDC_CHECK_HIDDEN	52
//#define IDC_CHECK_SYSTEM	53
//#define IDC_CHECK_VOLUME	54
//#define IDC_CHECK_DIRECTORY	55
//#define IDC_CHECK_ARCHIVE	56

/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemMSDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = ConvFileTypeFromFileName(name);
	}
	if (show_flags & INTNAME_IMPORT_INTERNAL) {
		// 内部からインポート時
		file_type_1 |= FILE_TYPE_ARCHIVE_MASK;
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemMSDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileAttr().GetType();
	int file_type_2 = 0;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	// 属性チェックボックスを作成
	CreateControlsSubForAttrDialog(parent, show_flags, sizer, flags, file_type_1);
}

/// ダイアログ内の属性部分のレイアウトを作成
wxStaticBoxSizer *DiskBasicDirItemMSDOS::CreateControlsSubForAttrDialog(IntNameBox *parent, int show_flags, wxBoxSizer *sizer, wxSizerFlags &flags, int file_type_1)
{
	wxCheckBox *chkAttr;
//	wxCheckBox *chkHidden;
//	wxCheckBox *chkSystem;
//	wxCheckBox *chkVolume;
//	wxCheckBox *chkDirectory;
//	wxCheckBox *chkArchive;

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxBoxSizer *hbox = NULL;
	for(int i=0; i<=TYPE_NAME_MS_ARCHIVE; i++) {
		if ((i % 2) == 0) {
			hbox = new wxBoxSizer(wxHORIZONTAL);
			staType1->Add(hbox);
		}
		chkAttr = new wxCheckBox(parent, IDC_CHECK_ATTR + i, wxGetTranslation(gTypeNameMS_l[i].name));
		chkAttr->SetValue((file_type_1 & gTypeNameMS_l[i].value) != 0);
		hbox->Add(chkAttr, flags);
	}

	sizer->Add(staType1, flags);

	return staType1;
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemMSDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	// 属性
	SetAttrSubInAttrDialog(parent, attr);
	return true;
}

/// 属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
void DiskBasicDirItemMSDOS::SetAttrSubInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr) const
{
	int val = 0;

	for(int i=0; i<=TYPE_NAME_MS_ARCHIVE; i++) {
		wxCheckBox *chkAttr = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ATTR + i);
		if (chkAttr->GetValue()) {
			val |= gTypeNameMS_l[i].value;
		}
	}

	attr.SetFileAttr(FORMAT_TYPE_UNKNOWN, val);
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemMSDOS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("NAME"), m_data.Data()->msdos.name, sizeof(m_data.Data()->msdos.name));
	vals.Add(wxT("EXT"), m_data.Data()->msdos.ext, sizeof(m_data.Data()->msdos.ext));
	vals.Add(wxT("TYPE"), m_data.Data()->msdos.type);
	vals.Add(wxT("NTRES"), m_data.Data()->msdos.ntres);
	vals.Add(wxT("CTIME_TENTH"), m_data.Data()->msdos.ctime_tenth);
	vals.Add(wxT("CTIME"), m_data.Data()->msdos.ctime);
	vals.Add(wxT("CDATE"), m_data.Data()->msdos.cdate);
	vals.Add(wxT("ADATE"), m_data.Data()->msdos.adate);
	vals.Add(wxT("START_GROUP_HI"), m_data.Data()->msdos.start_group_hi);
	vals.Add(wxT("WTIME"), m_data.Data()->msdos.wtime);
	vals.Add(wxT("WDATE"), m_data.Data()->msdos.wdate);
	vals.Add(wxT("START_GROUP"), m_data.Data()->msdos.start_group);
	vals.Add(wxT("FILE_SIZE"), m_data.Data()->msdos.file_size);
}

#endif /* !USE_CONSOLE */

///
///
///
DiskBasicDirItemVFAT::DiskBasicDirItemVFAT(DiskBasic *basic)
	: DiskBasicDirItemMSDOS(basic)
{
	m_has_lfn = false;
}
DiskBasicDirItemVFAT::DiskBasicDirItemVFAT(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemMSDOS(basic, n_sector, n_secpos, n_data)
{
	m_has_lfn = false;
}
DiskBasicDirItemVFAT::DiskBasicDirItemVFAT(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemMSDOS(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_has_lfn = false;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemVFAT::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// MS-DOS
	int t1 = GetFileType1();
	if (num == 0) {
		if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
			// ロングファイルネーム
			size = len = sizeof(m_data.Data()->mslfn.name);
			return m_data.Data()->mslfn.name;
		} else {
			size = len = sizeof(m_data.Data()->msdos.name);
			return m_data.Data()->msdos.name;
		}
	} else if (num == 1) {
		if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
			// ロングファイルネーム
			size = len = sizeof(m_data.Data()->mslfn.name2);
			return m_data.Data()->mslfn.name2;
		} else if (t1 & FILETYPE_MASK_MS_VOLUME) {
			// ボリュームラベルは拡張子もラベル名とする
			size = len = sizeof(m_data.Data()->msdos.ext);
			return m_data.Data()->msdos.ext;
		}
	} else if (num == 2) {
		if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
			// ロングファイルネーム
			size = len = sizeof(m_data.Data()->mslfn.name3);
			return m_data.Data()->mslfn.name3;
		}
	}
	size = len = 0;
	return NULL;
}

/// ファイル名を設定
void DiskBasicDirItemVFAT::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	if (length > 0) {
		// 0xe5は削除コードなので0x05に変換(Sjift JISなど2バイト系文字など)
		if (filename[0] == 0xe5) filename[0] = 0x05;
	}

	wxUint8 *n;
	size_t nl = 0;
	size_t s, l;

	int num = 0;
	do {
		s = l = 0;
		n = GetFileNamePos(num, s, l);
		if (n == NULL || s == 0) {
			break;
		}
		if (num > 0) {
			size_t pl = nl;
			size_t ps = s;
			if (nl < length) {
				pl = length;
				ps = s + nl - length;
			}
			memset(&filename[pl], 0, ps);
		}

		if (s > size) s = size;
		memcpy(n, &filename[nl], s);

		nl += s;
		size -= s;
		num++;
	} while(num <= 4);
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemVFAT::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
#ifdef USE_VFAT_LFN
	if (m_has_lfn) {
		// ロングファイル名を取得
		if (!GetNativeNameLFN(filename, size, length)) {
			// ロングファイル名を取得出来ないときは、ショートファイル名
			GetNativeNameSFN(filename, size, length);
		}
	} else {
		// LFNエントリ
		GetNativeNameSFN(filename, size, length);
	}
#else
	GetNativeNameSFN(filename, size, length);
#endif
}

/// @brief 拡張子を得る
/// @param [in,out] fileext  拡張子
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
/// @note データビットは反転させたまま
void DiskBasicDirItemVFAT::GetNativeExt(wxUint8 *fileext, size_t size, size_t &length) const
{
#ifdef USE_VFAT_LFN
	if (m_has_lfn) {
		// 拡張子は無視
		length = 0;
	} else {
		DiskBasicDirItemMSDOS::GetNativeExt(fileext, size, length);
	}
#else
	DiskBasicDirItemMSDOS::GetNativeExt(fileext, size, length);
#endif
}

/// ショートファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemVFAT::GetNativeNameSFN(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t nl = 0;
	int num = 0;
	do {
		wxUint8 *dname = &filename[nl];
		size_t s, l;

		n = GetFileNamePos(num, s, l);
		if (n == NULL || s == 0) {
			break;
		}
		if (s > size) s = size;
		memcpy(dname, n, s);

		nl += s;
		size -= s;
		num++;
	} while(num <= 4);

	if (nl > 0) {
		// 0x05を0xe5に変換(Shift JISなど2バイト系文字など)
		if (filename[0] == 0x05) filename[0] = 0xe5;
	}

	length = nl;
}

/// SFNのチェックサムを計算する
wxUint8 DiskBasicDirItemVFAT::CalcCheckSumSFN() const
{
	wxUint8 sum = 0;
	for(int i=0; i<11; i++) {
		sum = (sum >> 1) + (sum << 7);
		sum += m_data.Data()->msdos.name[i];	// ext[3]も含む
	}
	return sum;
}

/// ロングファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
/// @return false LFNなし
bool DiskBasicDirItemVFAT::GetNativeNameLFN(wxUint8 *filename, size_t size, size_t &length) const
{
	bool valid = false;
	wxUint8 name_str[512];
	size_t name_len = 0;
	length = 0;

	if (!m_parent) {
		// LFNなし
		return valid;
	}

	// チェックサムを計算
	wxUint8 sum = CalcCheckSumSFN();

	// このエントリが含まれるディレクトリ内のエントリリスト
	DiskBasicDirItems *items = m_parent->GetChildren();
	// LFNエントリはこのエントリの直前にあるはず
	for(int n = m_num - 1, cnt = 1; n >= 0 && cnt <= 20; n--, cnt++) {
		DiskBasicDirItem *item = items->Item(n);
		if (!item) {
			break;
		}
		// 属性がLFNか
		int t1 = item->GetData()->mslfn.type;
		if ((t1 & FILETYPE_MASK_MS_LFN) != FILETYPE_MASK_MS_LFN) {
			// LFNではない
			break;
		}
		// チェックサムが一致するか
		if (item->GetData()->mslfn.chksum != sum) {
			// LFNが不正？
			break;
		}
		// オーダー順が一致するか
		if ((item->GetData()->mslfn.order & 0x3f) != cnt) {
			// LFNが不正？
			break;
		}
		// LFNファイル名取得(UTF16LE)
		memcpy(&name_str[name_len], item->GetData()->mslfn.name, sizeof(item->GetData()->mslfn.name));
		name_len += sizeof(item->GetData()->mslfn.name);
		memcpy(&name_str[name_len], item->GetData()->mslfn.name2, sizeof(item->GetData()->mslfn.name2));
		name_len += sizeof(item->GetData()->mslfn.name2);
		memcpy(&name_str[name_len], item->GetData()->mslfn.name3, sizeof(item->GetData()->mslfn.name3));
		name_len += sizeof(item->GetData()->mslfn.name3);
		name_str[name_len] = 0;
		name_str[name_len+1] = 0;
		// 最終オーダーか
		if (item->GetData()->mslfn.order & 0x40) {
			valid = true;
			break;
		}
	}
	if (valid) {
		// 長さ計算
		for(size_t i=0; i<(name_len+2); i+=2) {
			if ((name_str[i] | name_str[i+1]) == 0) {
				length = i;	// bytes
				break;
			}
		}
		// コピー
		if (size > length) size = length;
		memcpy(filename, name_str, size);
	}
	return valid;
}

/// ディレクトリアイテムをアサインした後の追加処理 ロングファイル名のチェック
void DiskBasicDirItemVFAT::AdditionalProcessAfterAssigned()
{
	m_has_lfn = false;

#ifdef USE_VFAT_LFN
	if ((m_data.Data()->msdos.type & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		// LFNエントリは無視
		return;
	}

	if (!m_parent) {
		// LFNなし
		return;
	}

	// チェックサムを計算
	wxUint8 sum = CalcCheckSumSFN();

	// このエントリが含まれるディレクトリ内のエントリリスト
	DiskBasicDirItems *items = m_parent->GetChildren();
	// LFNエントリはこのエントリの直前にあるはず
	for(int n = m_num - 1, cnt = 1; n >= 0 && cnt <= 20; n--, cnt++) {
		DiskBasicDirItem *item = items->Item(n);
		if (!item) {
			break;
		}
		// 属性がLFNか
		int t1 = item->GetData()->mslfn.type;
		if ((t1 & FILETYPE_MASK_MS_LFN) != FILETYPE_MASK_MS_LFN) {
			// LFNではない
			break;
		}
		// チェックサムが一致するか
		if (item->GetData()->mslfn.chksum != sum) {
			// LFNが不正？
			break;
		}
		// オーダー順が一致するか
		if ((item->GetData()->mslfn.order & 0x3f) != cnt) {
			// LFNが不正？
			break;
		}
		// 最終オーダーか
		if (item->GetData()->mslfn.order & 0x40) {
			m_has_lfn = true;
			break;
		}
	}
#endif
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemVFAT::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;
	// ファイルサイズが16MBを超えている
	if (CheckUsed(false) && (m_data.Data()->msdos.type & FILETYPE_MASK_MS_LFN) != FILETYPE_MASK_MS_LFN && wxUINT32_SWAP_ON_BE(m_data.Data()->msdos.file_size) > 0xffffff) {
		valid = false;
	}
	return valid;
}

/// 削除できるか
bool DiskBasicDirItemVFAT::IsDeletable() const
{
	bool valid = true;
	int t1 = GetFileType1();
	if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		valid = false;
	} else if (t1 & FILETYPE_MASK_MS_DIRECTORY) {
		wxString name =	GetFileNamePlainStr();
		if (name == wxT(".") || name == wxT("..")) {
			// ディレクトリ ".", ".."は削除不可
			valid = false;
		}
	}
	return valid;
}

/// ファイル名を編集できるか
bool DiskBasicDirItemVFAT::IsFileNameEditable() const
{
	bool valid = true;
	int t1 = GetFileType1();
	if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		valid = false;
	} else if (t1 & FILETYPE_MASK_MS_DIRECTORY) {
		wxString name =	GetFileNamePlainStr();
		if (name == wxT(".") || name == wxT("..")) {
			// ディレクトリ ".", ".."は削除不可
			valid = false;
		}
	}
	return valid;
}

/// 属性を設定
void DiskBasicDirItemVFAT::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	// MS-DOS
	SetFileType1((ftype & 0xff00) >> 8);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemVFAT::GetFileAttr() const
{
	int t1 = GetFileType1();
	return DiskBasicFileType(basic->GetFormatTypeNumber(), t1 << 8, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemVFAT::GetFileAttrStr() const
{
	wxString attr;
	int ftype = GetFileAttr().GetType();
	// MS-DOS
	if ((GetFileAttr().GetOrigin() & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		if (!attr.IsEmpty()) attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMS[TYPE_NAME_MS_LFN].name);	// long file name
	} else {
		GetFileAttrStrSub(ftype, attr);
	}
	if (attr.IsEmpty()) {
		attr = wxT("---");
	}
	return attr;
}

/// 作成日付を得る
void DiskBasicDirItemVFAT::GetFileCreateDate(TM &tm) const
{
	wxUint16 cdate = wxUINT16_SWAP_ON_BE(m_data.Data()->msdos.cdate);
	ConvDateToTm(cdate, tm);
}

/// 作成時間を得る
void DiskBasicDirItemVFAT::GetFileCreateTime(TM &tm) const
{
	wxUint16 ctime = wxUINT16_SWAP_ON_BE(m_data.Data()->msdos.ctime);
	ConvTimeToTm(ctime, tm);
}

/// 作成日付を文字列で返す
wxString DiskBasicDirItemVFAT::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 作成時間を文字列で返す
wxString DiskBasicDirItemVFAT::GetFileCreateTimeStr() const
{
	TM tm;
	GetFileCreateTime(tm);
	return Utils::FormatHMSStr(tm);
}

/// 作成日付をセット
void DiskBasicDirItemVFAT::SetFileCreateDate(const TM &tm)
{
	if (tm.GetYear() >= 0 && tm.GetMonth() >= -1) {
		wxUint16 cdate = ConvTmToDate(tm);
		m_data.Data()->msdos.cdate = wxUINT16_SWAP_ON_BE(cdate);
	}
}

/// 作成時間をセット
void DiskBasicDirItemVFAT::SetFileCreateTime(const TM &tm)
{
	if (tm.GetHour() >= 0 && tm.GetMinute() >= 0) {
		wxUint16 ctime = ConvTmToTime(tm);
		m_data.Data()->msdos.ctime = wxUINT16_SWAP_ON_BE(ctime); 
	}
}

/// アクセス日付を返す
void DiskBasicDirItemVFAT::GetFileAccessDate(TM &tm) const
{
	wxUint16 adate = wxUINT16_SWAP_ON_BE(m_data.Data()->msdos.adate);
	ConvDateToTm(adate, tm);
}

/// アクセス日付を返す
wxString DiskBasicDirItemVFAT::GetFileAccessDateStr() const
{
	TM tm;
	GetFileAccessDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// アクセス日付をセット
void DiskBasicDirItemVFAT::SetFileAccessDate(const TM &tm)
{
	if (tm.GetYear() >= 0 && tm.GetMonth() >= -1) {
		wxUint16 adate = ConvTmToDate(tm);
		m_data.Data()->msdos.adate = wxUINT16_SWAP_ON_BE(adate);
	}
}

#if 0
/// 最初のグループ番号を設定
void DiskBasicDirItemVFAT::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	// MS-DOS
	m_data.Data()->msdos.start_group = wxUINT16_SWAP_ON_BE(val);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemVFAT::GetStartGroup(int fileunit_num) const
{
	// MS-DOS
	return wxUINT16_SWAP_ON_BE(m_data.Data()->msdos.start_group);
}
#endif

/// 文字列をバイト列に変換 文字コードは機種依存
/// @return >=0 バイト数
int DiskBasicDirItemVFAT::ConvStringToChars(const wxString &src, wxUint8 *dst, size_t len) const
{
	if ((GetFileType1() & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN || m_has_lfn) {
		// ロングファイル名は常にUTF-16
		wxCharBuffer buf = src.mb_str(wxMBConvUTF16());
		if (buf.length() > 0) {
			size_t l = buf.length() > len ? len : buf.length();
			memcpy(dst, buf.data(), l);
			return (int)l;
		} else {
			return 0;
		}
	} else {
		return basic->GetCharCodes().ConvToChars(src, dst, len);
	}
}

/// バイト列を文字列に変換 文字コードは機種依存
void DiskBasicDirItemVFAT::ConvCharsToString(const wxUint8 *src, size_t len, wxString &dst) const
{
	if ((GetFileType1() & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN || m_has_lfn) {
		// ロングファイル名は常にUTF-16
		dst = wxString((const char *)src, wxMBConvUTF16());
	} else {
		basic->GetCharCodes().ConvToString(src, len, dst);
	}
}

#ifndef USE_CONSOLE

//
// ダイアログ用
//

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemVFAT::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileAttr().GetType();
	int file_type_2 = 0;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	// 属性チェックボックスを作成
	CreateControlsSubForAttrDialog(parent, show_flags, sizer, flags, file_type_1);
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemVFAT::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	// 属性
	SetAttrSubInAttrDialog(parent, attr);

	return true;
}

#if 0
/// その他の属性値を設定する
void DiskBasicDirItemVFAT::SetOptionalAttr(DiskBasicDirItemAttr &attr)
{
}
#endif

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemVFAT::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	int t1 = GetFileType1();
	if ((t1 & FILETYPE_MASK_MS_LFN) == FILETYPE_MASK_MS_LFN) {
		vals.Add(wxT("self"), m_data.IsSelf());
		vals.Add(wxT("ORDER"), m_data.Data()->mslfn.order);
		vals.Add(wxT("NAME"), m_data.Data()->mslfn.name, sizeof(m_data.Data()->mslfn.name));
		vals.Add(wxT("TYPE"), m_data.Data()->mslfn.type);
		vals.Add(wxT("TYPE2"), m_data.Data()->mslfn.type2);
		vals.Add(wxT("CHKSUM"), m_data.Data()->mslfn.chksum);
		vals.Add(wxT("NAME2"), m_data.Data()->mslfn.name2, sizeof(m_data.Data()->mslfn.name2));
		vals.Add(wxT("DUMMY_GROUP"), m_data.Data()->mslfn.dummy_group);
		vals.Add(wxT("NAME3"), m_data.Data()->mslfn.name3, sizeof(m_data.Data()->mslfn.name3));
	} else {
		DiskBasicDirItemMSDOS::SetInternalDataInAttrDialog(vals);
	}
}

#endif /* !USE_CONSOLE */
