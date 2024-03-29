/// @file basicdiritem_cpm.cpp
///
/// @brief disk basic directory item for CP/M
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_cpm.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// CP/M属性名
const char *gTypeNameCPM[] = {
	wxTRANSLATE("Read Only"),
	wxTRANSLATE("System"),
	wxTRANSLATE("Archive"),
	NULL
};

const char *gTypeNameCPM_2[] = {
	wxTRANSLATE("Binary"),
	wxTRANSLATE("Ascii"),
	NULL
};
enum en_type_name_cpm_2 {
	TYPE_NAME_CPM_BINARY = 0,
	TYPE_NAME_CPM_ASCII = 1,
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
	// グループ番号の幅
	group_width = basic->GetGroupWidth();
	group_entries = basic->GetGroupsPerDirEntry() >= 8 ? basic->GetGroupsPerDirEntry() : (16 / group_width);
	m_external_attr = GetFileTypeByExt(0, GetFileExtPlainStr());

	next_item = NULL;
}
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	// グループ番号の幅
	group_width = basic->GetGroupWidth();
	group_entries = basic->GetGroupsPerDirEntry() >= 8 ? basic->GetGroupsPerDirEntry() : (16 / group_width);
	m_external_attr = GetFileTypeByExt(0, GetFileExtPlainStr());

	next_item = NULL;
}
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);
	// グループ番号の幅
	group_width = basic->GetGroupWidth();
	group_entries = basic->GetGroupsPerDirEntry() >= 8 ? basic->GetGroupsPerDirEntry() : (16 / group_width);
	m_external_attr = GetFileTypeByExt(0, GetFileExtPlainStr());

	next_item = NULL;

	Used(CheckUsed(n_unuse));
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemCPM::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemCPM::GetFileNamePos(int num, size_t &size, size_t &len) const
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
wxUint8 *DiskBasicDirItemCPM::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data.Data()->ext);
	return m_data.Data()->ext;
}

/// 属性１を返す
int	DiskBasicDirItemCPM::GetFileType1() const
{
	return basic->InvertUint8(m_data.Data()->type);
}

/// 属性２を返す
int	DiskBasicDirItemCPM::GetFileType2() const
{
	int val = 0;
	wxUint8 ext[sizeof(m_data.Data()->ext) + 1];
	basic->InvertMem(m_data.Data()->ext, sizeof(m_data.Data()->ext), ext);

	val |= (ext[0] & 0x80 ? FILE_TYPE_READONLY_MASK : 0);	// read only
	val |= (ext[1] & 0x80 ? FILE_TYPE_SYSTEM_MASK : 0);	// system
	val |= (ext[2] & 0x80 ? FILE_TYPE_ARCHIVE_MASK : 0);	// archive

	wxString extstr = GetFileExtPlainStr();

	val = GetFileTypeByExt(val, extstr);

	val |= m_external_attr;

	return val;
}

/// 拡張子からアスキーorバイナリ属性を判断する
int DiskBasicDirItemCPM::GetFileTypeByExt(int val, const wxString &ext) const
{
	const MyAttribute *sa = basic->GetAttributesByExtension().FindUpperCase(ext, FILE_TYPE_BINARY_MASK, 0x3f);
	if (sa) {
		val |= FILE_TYPE_BINARY_MASK;
	}
	return val;
}

/// 属性１を設定
void DiskBasicDirItemCPM::SetFileType1(int val)
{
	m_data.Data()->type = basic->InvertUint8(val);
}

/// 属性２を設定
void DiskBasicDirItemCPM::SetFileType2(int val)
{
	if (basic->IsDataInverted()) mem_invert(m_data.Data()->ext, sizeof(m_data.Data()->ext));	// invert

	m_data.Data()->ext[0] = (m_data.Data()->ext[0] & 0x7f) | (val & FILE_TYPE_READONLY_MASK ? 0x80 : 0);
	m_data.Data()->ext[1] = (m_data.Data()->ext[1] & 0x7f) | (val & FILE_TYPE_SYSTEM_MASK ? 0x80 : 0);
	m_data.Data()->ext[2] = (m_data.Data()->ext[2] & 0x7f) | (val & FILE_TYPE_ARCHIVE_MASK ? 0x80 : 0);
	m_external_attr = (val & FILE_TYPE_BINARY_MASK);

	if (basic->IsDataInverted()) mem_invert(m_data.Data()->ext, sizeof(m_data.Data()->ext));	// invert
}

/// ファイル名を得る
void DiskBasicDirItemCPM::GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	DiskBasicDirItem::GetNativeFileName(name, nlen, ext, elen);

	// 拡張子部分のMSBは属性ビットなので除く
	for(size_t en = 0; en < elen; en++) {
		ext[en] &= 0x7f;
	}
}

/// 拡張子を返す
/// @return 拡張子
wxString DiskBasicDirItemCPM::GetFileExtPlainStr() const
{
	if (!m_data.IsValid()) return wxT("");

	wxUint8 ext[sizeof(m_data.Data()->ext) + 1];
	basic->InvertMem(m_data.Data()->ext, sizeof(m_data.Data()->ext), ext);
	for(size_t i=0; i<sizeof(m_data.Data()->ext); i++) {
		ext[i] &= 0x7f;
	}
	return wxString(ext, sizeof(m_data.Data()->ext));
}

/// ファイル名を設定
/// @param [in]  filename ファイル名
/// @param [in]  size     バッファサイズ
/// @param [in]  length   長さ
/// @note filename はデータビットが反転している場合あり
void DiskBasicDirItemCPM::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	DiskBasicDirItem::SetNativeName(filename, size, length);

	// 複数ある時
	if (next_item) {
		next_item->SetNativeName(filename, size, length);
	}
}

/// 拡張子を設定
/// @param [in]  fileext  拡張子
/// @param [in]  size     バッファサイズ
/// @param [in]  length   長さ
/// @note fileext はデータビットが反転している場合あり
void DiskBasicDirItemCPM::SetNativeExt(wxUint8 *fileext, size_t size, size_t length)
{
	wxUint8 *e;
	size_t el;
	e = GetFileExtPos(el);

	if (el > size) el = size;

	for(size_t i = 0; i < el; i++) {
		// MSBは属性ビットなのでのこす
		e[i] = (e[i] & 0x80) | (fileext[i] & 0x7f);
	}

	// 複数ある時
	if (next_item) {
		next_item->SetNativeExt(fileext, size, length);
	}
}

/// 使用しているアイテムか
bool DiskBasicDirItemCPM::CheckUsed(bool unuse)
{
	return (GetFileType1() != 0xe5);
}

/// 削除
bool DiskBasicDirItemCPM::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	SetFileType1(basic->GetDeleteCode());
	Used(false);

	// 複数ある時
	if (next_item) {
		next_item->Delete();
	}
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemCPM::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = false;
	// ユーザIDが0～15でファイル名がオール0ならダメ
	if (GetFileType1() < 0x10) {
		wxUint8 name[sizeof(m_data.Data()->name) + 1];
		basic->InvertMem(m_data.Data()->name, sizeof(m_data.Data()->name), name);
		for(size_t n=0; n<sizeof(m_data.Data()->name); n++) {
			if (name[n] != 0) {
				valid = true;
				break;
			}
		}
		if (valid && !last) {
			valid = DiskBasicDirItem::CheckData((const wxUint8 *)m_data.Data(), GetDataSize(), last);
		}
		// グループ番号が超えていたらダメ
		if (valid) {
			for(int i = 0; i < group_entries; i++) {
				if (GetGroup(i) > basic->GetFatEndGroup()) {
					valid = false;
					break;
				}
			}
		}
	} else {
		valid = !CheckUsed(false);
	}
	return valid;
}

/// 属性を設定
void DiskBasicDirItemCPM::SetFileAttr(const DiskBasicFileType &file_type)
{
	SetFileType1(file_type.GetFormat() == basic->GetFormatTypeNumber() ? file_type.GetOrigin() : 0);
	SetFileType2(file_type.GetType());

	// 複数ある場合
	if (next_item) {
		next_item->SetFileAttr(file_type);
	}
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemCPM::GetFileAttr() const
{
	return DiskBasicFileType(basic->GetFormatTypeNumber(), GetFileType2(), GetFileType1());
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemCPM::GetFileAttrStr() const
{
	int val = GetFileType2();
	wxString str;

	if (val & FILE_TYPE_BINARY_MASK) {
		str += wxGetTranslation(gTypeNameCPM_2[TYPE_NAME_CPM_BINARY]);
	} else {
		str += wxGetTranslation(gTypeNameCPM_2[TYPE_NAME_CPM_ASCII]);
	}
	if (val & FILE_TYPE_READONLY_MASK) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_READ_ONLY]);
	}
	if (val & FILE_TYPE_SYSTEM_MASK) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_SYSTEM]);
	}
	if (val & FILE_TYPE_ARCHIVE_MASK) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_ARCHIVE]);
	}
	return str;
}

/// ファイルサイズをセット
void DiskBasicDirItemCPM::SetFileSize(int val)
{
	m_groups.SetSize(val);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemCPM::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed())  return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemCPM::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	int calc_file_size = 0;
	int calc_groups = 0;

	int bytes_per_group = basic->GetSectorSize() * basic->GetSectorsPerGroup();

	// グループ数を計算
	int map_size = GetGroupEntries();
	int group_size = (bytes_per_group * map_size);
	int remain_size = (GetExtentNumber() * SECTOR_UNIT_CPM + GetRecordNumber()) * SECTOR_UNIT_CPM;
	// ファイルサイズは1エントリ分にする
	remain_size = ((remain_size + group_size - 1) % group_size) + 1;

	for(int map_pos=0; map_pos < map_size; map_pos++) {
		wxUint32 group_num = GetGroup(map_pos);
		if (group_num == 0) break;
		basic->GetNumsFromGroup(group_num, 0, basic->GetSectorSize(), remain_size, group_items);

		calc_file_size += (remain_size < bytes_per_group ? remain_size : bytes_per_group);
		calc_groups++;

		remain_size -= bytes_per_group;
	}
	group_items.AddSize(calc_file_size);
	group_items.AddNums(calc_groups);

	if (next_item) {
		// ファイルには続きがある
		next_item->GetUnitGroups(fileunit_num, group_items);
	} else {
		// ファイル終り

		// グループサイズ
		group_items.SetSizePerGroup(bytes_per_group);
		// 最終セクタのサイズを計算
		group_items.SetSize(RecalcFileSize(group_items, (int)group_items.GetSize()));
	}
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有サイズ
/// @return 計算後のファイルサイズ
int DiskBasicDirItemCPM::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
	if (group_items.Count() == 0) return occupied_size;

	DiskBasicGroupItem *litem = &group_items.Last();
	DiskImageSector *sector = basic->GetSector(litem->track, litem->side, litem->sector_end);
	if (!sector) return occupied_size;

	int sector_size = sector->GetSectorSize();
	int remain_size = ((occupied_size + SECTOR_UNIT_CPM - 1) % SECTOR_UNIT_CPM) + 1;
	int unit_pos = (((occupied_size + sector_size - 1) % sector_size) / SECTOR_UNIT_CPM);
	wxUint8 *buf = sector->GetSectorBuffer();
	buf += (unit_pos * SECTOR_UNIT_CPM);
	remain_size = type->CalcDataSizeOnLastSector(this, NULL, NULL, buf, SECTOR_UNIT_CPM, remain_size);

	occupied_size = occupied_size - SECTOR_UNIT_CPM + remain_size;

	return occupied_size;
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemCPM::GetDataSize() const
{
	return sizeof(directory_cpm_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemCPM::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemCPM::CopyData(const directory_t *val)
{
	return m_data.Copy(val);
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemCPM::ClearData()
{
	m_data.Fill(0, GetDataSize(), basic->IsDataInverted());
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemCPM::NeedCheckEofCode()
{
	return (m_external_attr == 0);
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemCPM::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		// ただし、ファイルサイズが128バイトと合うなら終端記号は不要
		if ((file_size % SECTOR_UNIT_CPM) != 0) {
			file_size = CheckEofCode(istream, file_size);
			file_size--;
		}
	}
	return file_size;
}

/// 最初のグループ番号を設定
void DiskBasicDirItemCPM::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemCPM::GetStartGroup(int fileunit_num) const
{
	wxUint32 val = 0;
	val = group_width > 1 ? wxUINT16_SWAP_ON_BE(m_data.Data()->map.w[0]) : m_data.Data()->map.b[0];
	if (basic->IsDataInverted()) val ^= (group_width > 1 ? 0xffff : 0xff);	// invert
	return val;
}

bool DiskBasicDirItemCPM::IsDeletable() const
{
	return true;
}

/// グループ番号をセット
void DiskBasicDirItemCPM::SetGroup(int pos, wxUint32 val)
{
	if (pos < 0 || pos >= group_entries) return;

	if (basic->IsDataInverted()) val = ~val;	// invert

	if (group_width > 1) {
		m_data.Data()->map.w[pos] = wxUINT16_SWAP_ON_BE(val);
	} else {
		m_data.Data()->map.b[pos] = (wxUint8)val;
	}
}

/// グループ番号を返す
wxUint32 DiskBasicDirItemCPM::GetGroup(int pos) const
{
	wxUint32 val = 0;
	if (pos < 0 || pos >= group_entries) return val;

	val = group_width > 1 ? wxUINT16_SWAP_ON_BE(m_data.Data()->map.w[pos]) : m_data.Data()->map.b[pos];
	if (basic->IsDataInverted()) val ^= (group_width > 1 ? 0xffff : 0xff);	// invert
	return val;
}

/// エクステント番号を返す
wxUint8 DiskBasicDirItemCPM::GetExtentNumber() const
{
	wxUint8 num = m_data.Data()->extent_num;
	if (basic->IsDataInverted()) num ^= 0xff;	// invert
	return num;
}

/// レコード番号を返す
wxUint8 DiskBasicDirItemCPM::GetRecordNumber() const
{
	wxUint8 num = m_data.Data()->record_num;
	if (basic->IsDataInverted()) num ^= 0xff;	// invert
	return num;
}

/// ファイルサイズからエクステント番号とレコード番号をセット
void DiskBasicDirItemCPM::CalcExtentAndRecordNumber(int val)
{
//	int limit_size = (basic->GetSectorSize() * basic->GetSectorsPerGroup() * group_entries);

	if (val == 0) {
		m_data.Data()->extent_num = 0;
		m_data.Data()->record_num = 0;
	} else {
		val = (val + SECTOR_UNIT_CPM - 1) / SECTOR_UNIT_CPM;
		if ((val % SECTOR_UNIT_CPM) == 0) {
			m_data.Data()->extent_num = (val / SECTOR_UNIT_CPM) - 1;
			m_data.Data()->record_num = SECTOR_UNIT_CPM;
		} else {
			m_data.Data()->extent_num = (val / SECTOR_UNIT_CPM);
			m_data.Data()->record_num = ((val - 1) % SECTOR_UNIT_CPM) + 1;
		}
	}
	if (basic->IsDataInverted()) {
		m_data.Data()->extent_num ^= 0xff;	// invert
		m_data.Data()->record_num ^= 0xff;	// invert
	}
}

/// アイテムソート用
int DiskBasicDirItemCPM::Compare(DiskBasicDirItem **item1, DiskBasicDirItem **item2)
{
	directory_cpm_t d1, d2;
	memcpy(&d1, (*item1)->GetData(), sizeof(directory_cpm_t));
	memcpy(&d2, (*item2)->GetData(), sizeof(directory_cpm_t));
	if ((*item1)->GetBasic()->IsDataInverted()) {
		mem_invert(&d1, sizeof(directory_cpm_t));
		mem_invert(&d2, sizeof(directory_cpm_t));
	}
	int cmp = 0;
	// ユーザID＋ファイル名＋拡張子＋エクステント番号
	cmp = memcmp(&d1, &d2, sizeof(d1.type)+sizeof(d1.name)+sizeof(d1.ext)+sizeof(d1.extent_num));
	// ＋レコード番号(逆)
	if (cmp == 0) cmp = memcmp(&d2.record_num, &d1.record_num, 1);
	// ＋マップ
	if (cmp == 0) cmp = memcmp(&d1.map, &d2.map, 1);
	return cmp;
}

/// 名前比較
int DiskBasicDirItemCPM::CompareName(DiskBasicDirItem **item1, DiskBasicDirItem **item2)
{
	directory_cpm_t d1, d2;
	memcpy(&d1, (*item1)->GetData(), sizeof(directory_cpm_t));
	memcpy(&d2, (*item2)->GetData(), sizeof(directory_cpm_t));
	if ((*item1)->GetBasic()->IsDataInverted()) {
		mem_invert(&d1, sizeof(directory_cpm_t));
		mem_invert(&d2, sizeof(directory_cpm_t));
	}
	int cmp = 0;
	// ファイル名＋拡張子
	cmp = memcmp(&d1.name, &d2.name, sizeof(d1.name) + sizeof(d1.ext));
	return cmp;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemCPM::ConvFileTypeFromFileName(const wxString &filename) const
{
	int ftype = 0;
	// 拡張子で属性を設定する
	wxFileName fn(filename);
	ftype = GetFileTypeByExt(m_external_attr, fn.GetExt());
	return ftype;
}

//
// ダイアログ用
//

#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_SPIN_USERID		51
#define IDC_CHECK_READONLY	52
#define IDC_CHECK_SYSTEM	53
#define IDC_CHECK_ARCHIVE	54
#define IDC_RADIO_BINASC	55

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemCPM::GetFileType1Pos() const
{
	return GetFileType1();
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemCPM::GetFileType2Pos() const
{
	return GetFileType2();
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemCPM::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_2 = ConvFileTypeFromFileName(name);
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemCPM::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();
	wxRadioBox *radBinAsc;
	wxSpinCtrl *spnUserId;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkSystem;
	wxCheckBox *chkArchive;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString choices;
	for(int i=0; gTypeNameCPM_2[i] != NULL; i++) {
		choices.Add(wxGetTranslation(gTypeNameCPM_2[i]));
	}
	radBinAsc = new wxRadioBox(parent, IDC_RADIO_BINASC, _("Select File Type"), wxDefaultPosition, wxDefaultSize, choices, 0, wxRA_SPECIFY_COLS);
	radBinAsc->SetSelection(file_type_2 & FILE_TYPE_BINARY_MASK ? TYPE_NAME_CPM_BINARY : TYPE_NAME_CPM_ASCII);
	sizer->Add(radBinAsc, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxStaticText(parent, wxID_ANY, _("User ID")), wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL));
	spnUserId = new wxSpinCtrl(parent, IDC_SPIN_USERID, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_LEFT, 0, 15, file_type_1);
	hbox->Add(spnUserId, flags);
	staType4->Add(hbox);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_READ_ONLY]));
	chkReadOnly->SetValue((file_type_2 & FILE_TYPE_READONLY_MASK) != 0);
	hbox->Add(chkReadOnly, flags);
	chkSystem = new wxCheckBox(parent, IDC_CHECK_SYSTEM, wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_SYSTEM]));
	chkSystem->SetValue((file_type_2 & FILE_TYPE_SYSTEM_MASK) != 0);
	hbox->Add(chkSystem, flags);
	chkArchive = new wxCheckBox(parent, IDC_CHECK_ARCHIVE, wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_ARCHIVE]));
	chkArchive->SetValue((file_type_2 & FILE_TYPE_ARCHIVE_MASK) != 0);
	hbox->Add(chkArchive, flags);

	staType4->Add(hbox);
	sizer->Add(staType4, flags);

}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemCPM::ChangeTypeInAttrDialog(IntNameBox *parent)
{
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemCPM::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxSpinCtrl *spnUserId = (wxSpinCtrl *)parent->FindWindow(IDC_SPIN_USERID);
	wxRadioBox *radBinAsc = (wxRadioBox *)parent->FindWindow(IDC_RADIO_BINASC);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkSystem = (wxCheckBox *)parent->FindWindow(IDC_CHECK_SYSTEM);
	wxCheckBox *chkArchive = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ARCHIVE);

	int user_id = spnUserId->GetValue();
//	val = (val << FILETYPE_CPM_USERID_POS) & FILETYPE_CPM_USERID_MASK;
	int val = chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkSystem->GetValue() ? FILE_TYPE_SYSTEM_MASK : 0;
	val |= chkArchive->GetValue() ? FILE_TYPE_ARCHIVE_MASK : 0;
	val |= radBinAsc->GetSelection() == TYPE_NAME_CPM_BINARY ? FILE_TYPE_BINARY_MASK : 0;

	attr.SetFileAttr(basic->GetFormatTypeNumber(), val, user_id);

	return true;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemCPM::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("TYPE"), m_data.Data()->type);
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("EXT"), m_data.Data()->ext, sizeof(m_data.Data()->ext));
	vals.Add(wxT("EXTENT_NUM"), m_data.Data()->extent_num);
	vals.Add(wxT("RESERVED"), m_data.Data()->reserved, sizeof(m_data.Data()->reserved));
	vals.Add(wxT("RECORD_NUM"), m_data.Data()->record_num);
	vals.Add(wxT("MAP"), m_data.Data()->map.b, sizeof(m_data.Data()->map.b));
}
