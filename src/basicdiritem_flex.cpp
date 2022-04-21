/// @file basicdiritem_flex.cpp
///
/// @brief disk basic directory item for FLEX
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_flex.h"
#include <wx/regex.h>
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "utils.h"


//
//
//

/// FLEX属性名
const char *gTypeNameFLEX[] = {
	wxTRANSLATE("Read Only"),
	wxTRANSLATE("Undeletable"),
	wxTRANSLATE("Write Only"),
	wxTRANSLATE("Hidden"),
	wxTRANSLATE("Random Access"),
	NULL
};

//
//
//
DiskBasicDirItemFLEX::DiskBasicDirItemFLEX(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemFLEX::DiskBasicDirItemFLEX(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, secpos, data)
{
}
DiskBasicDirItemFLEX::DiskBasicDirItemFLEX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	Used(CheckUsed(unuse));

	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemFLEX::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data->flex.name);
		return m_data->flex.name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemFLEX::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data->flex.ext);
	return m_data->flex.ext;
}

/// 属性１を返す
int	DiskBasicDirItemFLEX::GetFileType1() const
{
	return m_data->flex.type;
}

/// 属性１を設定
void DiskBasicDirItemFLEX::SetFileType1(int val)
{
	m_data->flex.type = (val & 0xff);
}

/// 属性２を返す
int	DiskBasicDirItemFLEX::GetFileType2() const
{
	return m_data->flex.random_access;
}

/// 属性２を設定
void DiskBasicDirItemFLEX::SetFileType2(int val)
{
	m_data->flex.random_access = (val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemFLEX::CheckUsed(bool unuse)
{
	return (m_data->flex.name[0] != 0 && (m_data->flex.name[0] & 0x80) == 0);
}

/// 削除
/// @param [in] code : 削除コード(デフォルト0)
bool DiskBasicDirItemFLEX::Delete(wxUint8 code)
{
	// 削除はエントリのMSBをセットするだけ
	m_data->flex.name[0] |= 0x80;
	Used(false);
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemFLEX::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;

	if (m_data->flex.name[0] == 0) {
		last = true;
		return valid;
	}
	// 属性 0-3bitはゼロ
	if (m_data->flex.type & 0x0f) {
		valid = false;
	}
	return valid;
}

/// 属性を設定
void DiskBasicDirItemFLEX::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int val = 0;
	if (ftype & FILE_TYPE_READONLY_MASK) {
		val |= FILETYPE_MASK_FLEX_READ_ONLY;
	}
	if (ftype & FILE_TYPE_UNDELETE_MASK) {
		val |= FILETYPE_MASK_FLEX_UNDELETE;
	}
	if (ftype & FILE_TYPE_WRITEONLY_MASK) {
		val |= FILETYPE_MASK_FLEX_WRITE_ONLY;
	}
	if (ftype & FILE_TYPE_HIDDEN_MASK) {
		val |= FILETYPE_MASK_FLEX_HIDDEN;
	}
	if (ftype & FILE_TYPE_RANDOM_MASK) {
		int val2 = file_type.GetOrigin();
		SetFileType2(val2 ? val2 : 0x02);
	}

	SetFileType1(val);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemFLEX::GetFileAttr() const
{
	int val = 0;
	int random = 0;
	int type1 = GetFileType1();
	if (type1 & FILETYPE_MASK_FLEX_READ_ONLY) {
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (type1 & FILETYPE_MASK_FLEX_UNDELETE) {
		val |= FILE_TYPE_UNDELETE_MASK;
	}
	if (type1 & FILETYPE_MASK_FLEX_WRITE_ONLY) {
		val |= FILE_TYPE_WRITEONLY_MASK;
	}
	if (type1 & FILETYPE_MASK_FLEX_HIDDEN) {
		val |= FILE_TYPE_HIDDEN_MASK;
	}
	if (GetFileType2() != 0) {
		val |= FILE_TYPE_RANDOM_MASK;
		random = GetFileType2();
	}
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, random);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemFLEX::GetFileAttrStr() const
{
	wxString str;
	int oval = GetFileType1();
	if (oval & FILETYPE_MASK_FLEX_READ_ONLY) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_READ_ONLY]);
	}
	if (oval & FILETYPE_MASK_FLEX_UNDELETE) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_UNDELETE]);
	}
	if (oval & FILETYPE_MASK_FLEX_WRITE_ONLY) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_WRITE_ONLY]);
	}
	if (oval & FILETYPE_MASK_FLEX_HIDDEN) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_HIDDEN]);
	}
	if (GetFileType2() != 0) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_RANDOM]);
	}
	return str;
}

/// ファイルサイズをセット
void DiskBasicDirItemFLEX::SetFileSize(int val)
{
	m_groups.SetSize(val);
	int sec_size = (basic->GetSectorSize() - 4); 
	val = (val + sec_size - 1) / sec_size;
	m_data->flex.total_sectors = wxUINT16_SWAP_ON_LE(val);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemFLEX::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemFLEX::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	// セクタ先頭4バイトを除く
	int sec_size = (basic->GetSectorSize() - 4);

	int calc_file_size = 0;
	int calc_groups = 0;

	directory_flex_t *d = &m_data->flex;

	int track_num  = d->start_track;
	int sector_num = d->start_sector;
	int next_track_num  = 0;
	int next_sector_num = 0;

	int random_file = GetFileType2();
	if (random_file > 0) {
		// ランダムアクセスファイル
		for(int idx = 0; idx < random_file; idx++) {
			DiskD88Sector *sector = basic->GetSector(track_num, sector_num);
			if (!sector) {
				// error
				break;
			}
			flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
			next_track_num = p->next_track;
			next_sector_num = p->next_sector;

			wxUint32 gnum = (wxUint32)type->GetSectorPosFromNumS(track_num, sector_num);
			m_random_group_nums.Add((int)gnum);

			track_num = next_track_num;
			sector_num = next_sector_num;
		}
	} else {
		m_random_group_nums.Clear();
	}

	int limit = basic->GetFatEndGroup() + 1;
	while((track_num != 0 || sector_num != 0) && limit >= 0) {
		DiskD88Sector *sector = basic->GetSector(track_num, sector_num);
		if (!sector) {
			// error
			break;
		}
		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
		next_track_num = p->next_track;
		next_sector_num = p->next_sector;

		calc_file_size += sec_size;
		calc_groups++;

		wxUint32 gnum = (wxUint32)type->GetSectorPosFromNumS(track_num, sector_num);
		wxUint32 next_gnum = (wxUint32)type->GetSectorPosFromNumS(next_track_num, next_sector_num);

		int side_num = ((sector_num - 1) / basic->GetSectorsPerTrackOnBasic());
		group_items.Add(gnum, next_gnum, track_num, side_num, sector_num, sector_num);

//		file_size += (sector->GetSectorSize() - 4);
//		groups++;
		track_num = next_track_num;
		sector_num = next_sector_num;

		limit--;

		if (track_num == 0 || sector_num == 0) {
			// 最終セクタは0パディング部分のサイズを減らす
			wxUint8 *buf = sector->GetSectorBuffer();
			for(int pos = sector->GetSectorSize() - 1; pos >= 4; pos--) {
				if (buf[pos] != 0) break;
				calc_file_size--;
			}
			break;
		}
	}

	group_items.SetNums(calc_groups);

	// ファイルサイズ
//	int inter_file_size = wxUINT16_SWAP_ON_LE(m_data->flex.total_sectors) * sec_size;
//	if (inter_file_size == 0) {
//		inter_file_size = calc_file_size;
//	}
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());
}

void DiskBasicDirItemFLEX::GetFileDate(struct tm *tm) const
{
	tm->tm_year = (m_data->flex.year % 100);
	if (tm->tm_year < 80) tm->tm_year += 100;
	tm->tm_mon = m_data->flex.month - 1; 
	tm->tm_mday = m_data->flex.day;
}

void DiskBasicDirItemFLEX::GetFileTime(struct tm *tm) const
{
	tm->tm_hour = 0; 
	tm->tm_min = 0;
	tm->tm_sec = 0;
}

wxString DiskBasicDirItemFLEX::GetFileDateStr() const
{
	struct tm tm;
	GetFileDate(&tm);
	return Utils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemFLEX::GetFileTimeStr() const
{
	return wxEmptyString;
}

void DiskBasicDirItemFLEX::SetFileDate(const struct tm *tm)
{
	if (tm->tm_year < 0 || tm->tm_mon < -1) return;

	m_data->flex.year = (tm->tm_year % 100);
	m_data->flex.month = tm->tm_mon + 1;
	m_data->flex.day = tm->tm_mday;
}

void DiskBasicDirItemFLEX::SetFileTime(const struct tm *tm)
{
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemFLEX::GetDataSize() const
{
	return sizeof(directory_flex_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemFLEX::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	int trk_num = 0;
	int sec_num = 0;
	type->GetNumFromSectorPosS(val, trk_num, sec_num);
	m_data->flex.start_track = trk_num;
	m_data->flex.start_sector = sec_num;
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemFLEX::GetStartGroup(int fileunit_num) const
{
	wxUint32 val = (wxUint32)type->GetSectorPosFromNumS(m_data->flex.start_track, m_data->flex.start_sector);
	return val;
}

/// 最後のグループ番号をセット
void DiskBasicDirItemFLEX::SetLastGroup(wxUint32 val)
{
	int trk_num = 0;
	int sec_num = 0;
	type->GetNumFromSectorPosS(val, trk_num, sec_num);
	m_data->flex.last_track = trk_num;
	m_data->flex.last_sector = sec_num;
}

/// 最後のグループ番号を返す
wxUint32 DiskBasicDirItemFLEX::GetLastGroup() const
{
	wxUint32 val = (wxUint32)type->GetSectorPosFromNumS(m_data->flex.last_track, m_data->flex.last_sector);
	return val;
}

/// 追加のグループ番号を得る(機種依存)
void DiskBasicDirItemFLEX::GetExtraGroups(wxArrayInt &arr) const
{
	arr = m_random_group_nums;
}

bool DiskBasicDirItemFLEX::IsDeletable() const
{
	return true;
}

/// 最初のトラック番号をセット
void DiskBasicDirItemFLEX::SetStartTrack(wxUint8 val)
{
	m_data->flex.start_track = val;
}
/// 最初のセクタ番号をセット
void DiskBasicDirItemFLEX::SetStartSector(wxUint8 val)
{
	m_data->flex.start_sector = val;
}
/// 最初のトラック番号を返す
wxUint8 DiskBasicDirItemFLEX::GetStartTrack() const
{
	return m_data->flex.start_track;
}
/// 最初のセクタ番号を返す
wxUint8 DiskBasicDirItemFLEX::GetStartSector() const
{
	return m_data->flex.start_sector;
}
/// 最後のトラック番号をセット
void DiskBasicDirItemFLEX::SetLastTrack(wxUint8 val)
{
	m_data->flex.last_track = val;
}
/// 最後のセクタ番号をセット
void DiskBasicDirItemFLEX::SetLastSector(wxUint8 val)
{
	m_data->flex.last_sector = val;
}
/// 最後のトラック番号を返す
wxUint8 DiskBasicDirItemFLEX::GetLastTrack() const
{
	return m_data->flex.last_track;
}
/// 最後のセクタ番号を返す
wxUint8 DiskBasicDirItemFLEX::GetLastSector() const
{
	return m_data->flex.last_sector;
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_CHECK_READONLY 51
#define IDC_CHECK_UNDELETE 52
#define IDC_CHECK_WRITEONLY 53
#define IDC_CHECK_HIDDEN   54
#define IDC_CHECK_RANDOM   55

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemFLEX::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemFLEX::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileAttr().GetType();
	int file_type_2 = GetFileType2();
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkUndelete;
	wxCheckBox *chkWriteOnly;
	wxCheckBox *chkHidden;
	wxCheckBox *chkRandom;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_READ_ONLY]));
	chkReadOnly->SetValue((file_type_1 & FILE_TYPE_READONLY_MASK) != 0);
	staType4->Add(chkReadOnly, flags);
	chkUndelete = new wxCheckBox(parent, IDC_CHECK_UNDELETE, wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_UNDELETE]));
	chkUndelete->SetValue((file_type_1 & FILE_TYPE_UNDELETE_MASK) != 0);
	staType4->Add(chkUndelete, flags);
	chkWriteOnly = new wxCheckBox(parent, IDC_CHECK_WRITEONLY, wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_WRITE_ONLY]));
	chkWriteOnly->SetValue((file_type_1 & FILE_TYPE_WRITEONLY_MASK) != 0);
	staType4->Add(chkWriteOnly, flags);
	chkHidden = new wxCheckBox(parent, IDC_CHECK_HIDDEN, wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_HIDDEN]));
	chkHidden->SetValue((file_type_1 & FILE_TYPE_HIDDEN_MASK) != 0);
	staType4->Add(chkHidden, flags);
	chkRandom = new wxCheckBox(parent, IDC_CHECK_RANDOM, wxGetTranslation(gTypeNameFLEX[TYPE_NAME_FLEX_RANDOM]));
	chkRandom->SetValue((file_type_1 & FILE_TYPE_RANDOM_MASK) != 0);
	staType4->Add(chkRandom, flags);
	sizer->Add(staType4, flags);

	// ユーザ定義データ(ランダムファイル属性値)
	parent->SetUserData(file_type_2);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemFLEX::ChangeTypeInAttrDialog(IntNameBox *parent)
{
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemFLEX::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkUndelete = (wxCheckBox *)parent->FindWindow(IDC_CHECK_UNDELETE);
	wxCheckBox *chkWriteOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_WRITEONLY);
	wxCheckBox *chkHidden = (wxCheckBox *)parent->FindWindow(IDC_CHECK_HIDDEN);
	wxCheckBox *chkRandom = (wxCheckBox *)parent->FindWindow(IDC_CHECK_RANDOM);

	int val = chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkUndelete->GetValue() ? FILE_TYPE_UNDELETE_MASK : 0;
	val |= chkWriteOnly->GetValue() ? FILE_TYPE_WRITEONLY_MASK : 0;
	val |= chkHidden->GetValue() ? FILE_TYPE_HIDDEN_MASK : 0;

	// ユーザ定義データ(ランダムファイル属性値)
	int random = 0;
	if (chkRandom->GetValue()) {
		val |= FILE_TYPE_RANDOM_MASK;
		random = parent->GetUserData();
	}

	attr.SetFileAttr(basic->GetFormatTypeNumber(), val, random);

	return true;
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemFLEX::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	wxFileName fn(filename);
	if (fn.GetExt().IsEmpty()) {
		errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILEEXT_EMPTY]);
		return false;
	}
	return true;
}
