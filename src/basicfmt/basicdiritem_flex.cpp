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
#include "../charcodes.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// FLEX属性名
const name_value_t gTypeNameFLEX[] = {
	{ wxTRANSLATE("Read Only"), FILE_TYPE_READONLY_MASK },
	{ wxTRANSLATE("Undeletable"), FILE_TYPE_UNDELETE_MASK },
	{ wxTRANSLATE("Write Only"), FILE_TYPE_WRITEONLY_MASK },
	{ wxTRANSLATE("Hidden"), FILE_TYPE_HIDDEN_MASK },
	{ wxTRANSLATE("Random Access"), FILE_TYPE_RANDOM_MASK },
	{ NULL, -1 }
};

const value_value_t gTypeValueFLEX[] = {
	{ FILE_TYPE_READONLY_MASK, FILETYPE_MASK_FLEX_READ_ONLY },
	{ FILE_TYPE_UNDELETE_MASK, FILETYPE_MASK_FLEX_UNDELETE },
	{ FILE_TYPE_WRITEONLY_MASK, FILETYPE_MASK_FLEX_WRITE_ONLY },
	{ FILE_TYPE_HIDDEN_MASK, FILETYPE_MASK_FLEX_HIDDEN },
	{ -1, -1 }
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemFLEX::DiskBasicDirItemFLEX(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemFLEX::DiskBasicDirItemFLEX(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
}
DiskBasicDirItemFLEX::DiskBasicDirItemFLEX(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);

	Used(CheckUsed(n_unuse));

	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemFLEX::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// 論理セクタ番号からセクタ内のブロック位置を得る
int DiskBasicDirItemFLEX::PhySecPos(int sector_number) const
{
	return (sector_number - 1) % basic->GetGroupsPerSector();
}

/// 論理セクタ番号からセクタ内の位置を得る
int DiskBasicDirItemFLEX::SecBufOfs(int sector_number) const
{
	return PhySecPos(sector_number) * basic->GetSectorSize() / basic->GetGroupsPerSector();
}

/// 論理セクタサイズ
int DiskBasicDirItemFLEX::LogSecSiz(int sector_size) const
{
	return sector_size / basic->GetGroupsPerSector();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemFLEX::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemFLEX::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data.Data()->ext);
	return m_data.Data()->ext;
}

/// 属性１を返す
int	DiskBasicDirItemFLEX::GetFileType1() const
{
	return m_data.Data()->type;
}

/// 属性１を設定
void DiskBasicDirItemFLEX::SetFileType1(int val)
{
	m_data.Data()->type = (val & 0xff);
}

/// 属性２を返す
int	DiskBasicDirItemFLEX::GetFileType2() const
{
	return m_data.Data()->random_access;
}

/// 属性２を設定
void DiskBasicDirItemFLEX::SetFileType2(int val)
{
	m_data.Data()->random_access = (val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemFLEX::CheckUsed(bool unuse)
{
	return (m_data.Data()->name[0] != 0 && (m_data.Data()->name[0] & 0x80) == 0);
}

/// 削除
bool DiskBasicDirItemFLEX::Delete()
{
	// 削除はエントリのMSBをセットするだけ
	m_data.Data()->name[0] |= 0x80;
	Used(false);
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemFLEX::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;

	if (m_data.Data()->name[0] == 0) {
		last = true;
		return valid;
	}
	// 属性 0-3bitはゼロ
	if (m_data.Data()->type & 0x0f) {
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
	for(int i=0; i<=TYPE_NAME_FLEX_HIDDEN; i++) {
		if (ftype & gTypeValueFLEX[i].com_value) {
			val |= gTypeValueFLEX[i].ori_value;
		}
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

	for(int i=0; i<=TYPE_NAME_FLEX_HIDDEN; i++) {
		if (type1 & gTypeValueFLEX[i].ori_value) {
			val |= gTypeValueFLEX[i].com_value;
		}
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
	int val = GetFileAttr().GetType();
	for(int i=0; i<=TYPE_NAME_FLEX_RANDOM; i++) {
		if (val & gTypeNameFLEX[i].value) {
			if (!str.IsEmpty()) str += wxT(", ");
			str += wxGetTranslation(gTypeNameFLEX[i].name);
		}
	}

	return str;
}

/// ファイルサイズをセット
void DiskBasicDirItemFLEX::SetFileSize(int val)
{
	m_groups.SetSize(val);
	int sec_size = LogSecSiz(basic->GetSectorSize()) - 4; 
	val = (val + sec_size - 1) / sec_size;
	m_data.Data()->total_sectors = wxUINT16_SWAP_ON_LE(val);
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
	int sec_size = LogSecSiz(basic->GetSectorSize()) - 4;

	int calc_file_size = 0;
	int calc_groups = 0;

	directory_flex_t *d = m_data.Data();

	int track_num  = d->start_track;
	int sector_num = d->start_sector;
	int next_track_num  = 0;
	int next_sector_num = 0;
	int div_num = 0;

	int random_file = GetFileType2();
	if (random_file > 0) {
		// ランダムアクセスファイル
		for(int idx = 0; idx < random_file; idx++) {
			int gnum = type->GetSectorPosFromNumS(track_num, sector_num);

			DiskD88Sector *sector = basic->GetSectorFromSectorPos(gnum, &div_num);
			if (!sector) {
				// error
				break;
			}
			flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
			next_track_num = p->next_track;
			next_sector_num = p->next_sector;

			m_random_group_nums.Add(gnum);

			track_num = next_track_num;
			sector_num = next_sector_num;
		}
	} else {
		m_random_group_nums.Clear();
	}

	int limit = basic->GetFatEndGroup() + 1;
	while((track_num != 0 || sector_num != 0) && limit >= 0) {
		int gnum = type->GetSectorPosFromNumS(track_num, sector_num);

		DiskD88Sector *sector = basic->GetSectorFromSectorPos(gnum, &div_num);
		if (!sector) {
			// error
			break;
		}
		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer(SecBufOfs(div_num + 1));
		next_track_num = p->next_track;
		next_sector_num = p->next_sector;

		calc_file_size += sec_size;
		calc_groups++;

		wxUint32 next_gnum = (wxUint32)type->GetSectorPosFromNumS(next_track_num, next_sector_num);

		int ptrack_num, pside_num, psector_num, div_nums;
		type->GetNumFromSectorPos(gnum, ptrack_num, pside_num, psector_num, &div_num, &div_nums);
		group_items.Add((wxUint32)gnum, next_gnum, ptrack_num, pside_num, psector_num, psector_num, div_num, div_nums);

//		file_size += (sector->GetSectorSize() - 4);
//		groups++;
		track_num = next_track_num;
		sector_num = next_sector_num;

		limit--;

		if (track_num == 0 || sector_num == 0) {
			// 最終セクタは0パディング部分のサイズを減らす
			wxUint8 *buf = sector->GetSectorBuffer(SecBufOfs(div_num + 1));
			for(int pos = LogSecSiz(sector->GetSectorSize()) - 1; pos >= 4; pos--) {
				if (buf[pos] != 0) break;
				calc_file_size--;
			}
			break;
		}
	}

	group_items.SetNums(calc_groups);

	// ファイルサイズ
//	int inter_file_size = wxUINT16_SWAP_ON_LE(m_data.Data()->total_sectors) * sec_size;
//	if (inter_file_size == 0) {
//		inter_file_size = calc_file_size;
//	}
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(LogSecSiz(basic->GetSectorSize()) * basic->GetSectorsPerGroup());
}

void DiskBasicDirItemFLEX::GetFileCreateDate(TM &tm) const
{
	tm.SetYear(m_data.Data()->year % 100);
	if (tm.GetYear() < 80) tm.AddYear(100);
	tm.SetMonth(m_data.Data()->month - 1); 
	tm.SetDay(m_data.Data()->day);
}

void DiskBasicDirItemFLEX::GetFileCreateTime(TM &tm) const
{
	tm.SetHour(0); 
	tm.SetMinute(0);
	tm.SetSecond(0);
}

wxString DiskBasicDirItemFLEX::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

wxString DiskBasicDirItemFLEX::GetFileCreateTimeStr() const
{
	return wxEmptyString;
}

void DiskBasicDirItemFLEX::SetFileCreateDate(const TM &tm)
{
	if (tm.GetYear() < 0 || tm.GetMonth() < -1) return;

	m_data.Data()->year = (tm.GetYear() % 100);
	m_data.Data()->month = tm.GetMonth() + 1;
	m_data.Data()->day = tm.GetDay();
}

void DiskBasicDirItemFLEX::SetFileCreateTime(const TM &tm)
{
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemFLEX::GetDataSize() const
{
	return sizeof(directory_flex_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemFLEX::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemFLEX::CopyData(const directory_t *val)
{
	return m_data.Copy(val);
}

/// ディレクトリをクリア
void DiskBasicDirItemFLEX::ClearData()
{
	m_data.Fill(0);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemFLEX::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	int trk_num = 0;
	int sec_num = 0;
	type->GetNumFromSectorPosS(val, trk_num, sec_num);
	m_data.Data()->start_track = trk_num;
	m_data.Data()->start_sector = sec_num;
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemFLEX::GetStartGroup(int fileunit_num) const
{
	wxUint32 val = (wxUint32)type->GetSectorPosFromNumS(m_data.Data()->start_track, m_data.Data()->start_sector);
	return val;
}

/// 最後のグループ番号をセット
void DiskBasicDirItemFLEX::SetLastGroup(wxUint32 val)
{
	int trk_num = 0;
	int sec_num = 0;
	type->GetNumFromSectorPosS(val, trk_num, sec_num);
	m_data.Data()->last_track = trk_num;
	m_data.Data()->last_sector = sec_num;
}

/// 最後のグループ番号を返す
wxUint32 DiskBasicDirItemFLEX::GetLastGroup() const
{
	wxUint32 val = (wxUint32)type->GetSectorPosFromNumS(m_data.Data()->last_track, m_data.Data()->last_sector);
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
	m_data.Data()->start_track = val;
}
/// 最初のセクタ番号をセット
void DiskBasicDirItemFLEX::SetStartSector(wxUint8 val)
{
	m_data.Data()->start_sector = val;
}
/// 最初のトラック番号を返す
wxUint8 DiskBasicDirItemFLEX::GetStartTrack() const
{
	return m_data.Data()->start_track;
}
/// 最初のセクタ番号を返す
wxUint8 DiskBasicDirItemFLEX::GetStartSector() const
{
	return m_data.Data()->start_sector;
}
/// 最後のトラック番号をセット
void DiskBasicDirItemFLEX::SetLastTrack(wxUint8 val)
{
	m_data.Data()->last_track = val;
}
/// 最後のセクタ番号をセット
void DiskBasicDirItemFLEX::SetLastSector(wxUint8 val)
{
	m_data.Data()->last_sector = val;
}
/// 最後のトラック番号を返す
wxUint8 DiskBasicDirItemFLEX::GetLastTrack() const
{
	return m_data.Data()->last_track;
}
/// 最後のセクタ番号を返す
wxUint8 DiskBasicDirItemFLEX::GetLastSector() const
{
	return m_data.Data()->last_sector;
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_CHECK_ATTR1 51
//#define IDC_CHECK_UNDELETE 52
//#define IDC_CHECK_WRITEONLY 53
//#define IDC_CHECK_HIDDEN   54
//#define IDC_CHECK_RANDOM   55

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
	wxCheckBox *chkAttr1;
//	wxCheckBox *chkUndelete;
//	wxCheckBox *chkWriteOnly;
//	wxCheckBox *chkHidden;
//	wxCheckBox *chkRandom;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);

	for(int i=0; i<=TYPE_NAME_FLEX_RANDOM; i++) {
		chkAttr1 = new wxCheckBox(parent, IDC_CHECK_ATTR1 + i, wxGetTranslation(gTypeNameFLEX[i].name));
		chkAttr1->SetValue((file_type_1 & gTypeNameFLEX[i].value) != 0);
		staType1->Add(chkAttr1, flags);
	}
	sizer->Add(staType1, flags);

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
	int val = 0;
	for(int i=0; i<=TYPE_NAME_FLEX_RANDOM; i++) {
		wxCheckBox *chkAttr1 = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ATTR1 + i);
		if (chkAttr1->GetValue()) {
			val |= gTypeNameFLEX[i].value;
		}
	}

	// ユーザ定義データ(ランダムファイル属性値)
	int random = 0;
	if (val & FILE_TYPE_RANDOM_MASK) {
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

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemFLEX::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("EXT"), m_data.Data()->ext, sizeof(m_data.Data()->ext));
	vals.Add(wxT("TYPE"), m_data.Data()->type);
	vals.Add(wxT("RESERVED"), m_data.Data()->reserved);
	vals.Add(wxT("START_TRACK"), m_data.Data()->start_track);
	vals.Add(wxT("START_SECTOR"), m_data.Data()->start_sector);
	vals.Add(wxT("LAST_TRACK"), m_data.Data()->last_track);
	vals.Add(wxT("LAST_SECTOR"), m_data.Data()->last_sector);
	vals.Add(wxT("TOTAL_SECTORS"), m_data.Data()->total_sectors, true);
	vals.Add(wxT("RANDOM_ACCESS"), m_data.Data()->random_access);
	vals.Add(wxT("RESERVED2"), m_data.Data()->reserved2);
	vals.Add(wxT("MONTH"), m_data.Data()->month);
	vals.Add(wxT("DAY"), m_data.Data()->day);
	vals.Add(wxT("YEAR"), m_data.Data()->year);
}
