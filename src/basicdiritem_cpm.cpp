/// @file basicdiritem_cpm.cpp
///
/// @brief disk basic directory item for CP/M
///
#include "basicdiritem_cpm.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"

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

//
//
//
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	// グループ番号の幅は最大グループ番号から判定
	group_width = basic->GetFatEndGroup() >= 256 ? 2 : 1;
	group_entries = basic->GetGroupsPerDirEntry() >= 8 ? basic->GetGroupsPerDirEntry() : (16 / group_width);
	external_attr = GetFileTypeByExt(0, GetFileExtPlainStr());

	next_item = NULL;
}
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, data)
{
	// グループ番号の幅は最大グループ番号から判定
	group_width = basic->GetFatEndGroup() >= 256 ? 2 : 1;
	group_entries = basic->GetGroupsPerDirEntry() >= 8 ? basic->GetGroupsPerDirEntry() : (16 / group_width);
	external_attr = GetFileTypeByExt(0, GetFileExtPlainStr());

	next_item = NULL;
}
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	// グループ番号の幅は最大グループ番号から判定
	group_width = basic->GetFatEndGroup() >= 256 ? 2 : 1;
	group_entries = basic->GetGroupsPerDirEntry() >= 8 ? basic->GetGroupsPerDirEntry() : (16 / group_width);
	external_attr = GetFileTypeByExt(0, GetFileExtPlainStr());

	next_item = NULL;

	used = CheckUsed(unuse);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemCPM::GetFileNamePos(size_t &len, bool *invert) const
{
	len = sizeof(data->cpm.name);
	if (invert) *invert = basic->IsDataInverted();
	return data->cpm.name; 
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemCPM::GetFileExtPos(size_t &len) const
{
	len = sizeof(data->cpm.ext);
	return data->cpm.ext; 
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemCPM::GetFileNameSize(bool *invert) const
{
	if (invert) *invert = basic->IsDataInverted();
	return (int)sizeof(data->cpm.name);
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemCPM::GetFileExtSize(bool *invert) const
{
	if (invert) *invert = basic->IsDataInverted();
	return (int)sizeof(data->cpm.ext);
}

/// 属性１を返す
int	DiskBasicDirItemCPM::GetFileType1() const
{
	return basic->InvertUint8(data->cpm.type);
}

/// 属性２を返す
int	DiskBasicDirItemCPM::GetFileType2() const
{
	int val = 0;
	wxUint8 ext[sizeof(data->cpm.ext) + 1];
	basic->InvertMem(data->cpm.ext, sizeof(data->cpm.ext), ext);

	val |= (ext[0] & 0x80 ? FILE_TYPE_READONLY_MASK : 0);	// read only
	val |= (ext[1] & 0x80 ? FILE_TYPE_SYSTEM_MASK : 0);	// system
	val |= (ext[2] & 0x80 ? FILE_TYPE_ARCHIVE_MASK : 0);	// archive

	wxString extstr = GetFileExtPlainStr();

	val = GetFileTypeByExt(val, extstr);

	val |= external_attr;

	return val;
}

int DiskBasicDirItemCPM::GetFileTypeByExt(int val, const wxString &ext) const
{
	if (ext.Upper() == wxT("COM")) {
		val |= FILE_TYPE_BINARY_MASK;
	}
	return val;
}

/// 属性１を設定
void DiskBasicDirItemCPM::SetFileType1(int val)
{
	data->cpm.type = basic->InvertUint8(val);
}

/// 属性２を設定
void DiskBasicDirItemCPM::SetFileType2(int val)
{
	if (basic->IsDataInverted()) mem_invert(data->cpm.ext, sizeof(data->cpm.ext));	// invert

	data->cpm.ext[0] = (data->cpm.ext[0] & 0x7f) | (val & FILE_TYPE_READONLY_MASK ? 0x80 : 0);
	data->cpm.ext[1] = (data->cpm.ext[1] & 0x7f) | (val & FILE_TYPE_SYSTEM_MASK ? 0x80 : 0);
	data->cpm.ext[2] = (data->cpm.ext[2] & 0x7f) | (val & FILE_TYPE_ARCHIVE_MASK ? 0x80 : 0);
	external_attr = (val & FILE_TYPE_BINARY_MASK);

	if (basic->IsDataInverted()) mem_invert(data->cpm.ext, sizeof(data->cpm.ext));	// invert
}

/// ファイル名を得る
void DiskBasicDirItemCPM::GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	DiskBasicDirItem::GetFileName(name, nlen, ext, elen);

	// 拡張子部分のMSBは属性ビットなので除く
	for(size_t en = 0; en < elen; en++) {
		ext[en] &= 0x7f;
	}
}

/// 拡張子を返す
/// @return 拡張子
wxString DiskBasicDirItemCPM::GetFileExtPlainStr() const
{
	if (!data) return wxT("");

	wxUint8 ext[sizeof(data->cpm.ext) + 1];
	basic->InvertMem(data->cpm.ext, sizeof(data->cpm.ext), ext);
	for(size_t i=0; i<sizeof(data->cpm.ext); i++) {
		ext[i] &= 0x7f;
	}
	return wxString(ext, sizeof(data->cpm.ext));
}

/// ファイル名を設定
/// @param [in]  filename ファイル名
/// @param [in]  length   長さ
/// @note filename はデータビットが反転している場合あり
void DiskBasicDirItemCPM::SetFileName(const wxUint8 *filename, int length)
{
	DiskBasicDirItem::SetFileName(filename, length);

	// 複数ある時
	if (next_item) {
		next_item->SetFileName(filename, length);
	}
}

/// 拡張子を設定
/// @param [in]  fileext  拡張子
/// @param [in]  length   長さ
/// @note fileext はデータビットが反転している場合あり
void DiskBasicDirItemCPM::SetFileExt(const wxUint8 *fileext, int length)
{
	wxUint8 *e;
	size_t l, el;
	GetFileNamePos(l);
	e = GetFileExtPos(el);

	if (el > (size_t)length) el = (size_t)length;

	for(size_t i = 0; i < el; i++) {
		// MSBは属性ビットなのでのこす
		e[i] = (e[i] & 0x80) | (fileext[i] & 0x7f);
	}

	// 複数ある時
	if (next_item) {
		next_item->SetFileExt(fileext, length);
	}
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemCPM::InvalidateChars()
{
	return wxT(" \"*,:;<=>?[\\]");
}

/// ダイアログ入力前のファイル名文字列を変換 大文字にする
void DiskBasicDirItemCPM::ConvertToFileNameStr(wxString &filename)
{
	filename = filename.Upper();
}

/// ダイアログ入力後のファイル名文字列を変換 大文字にする
void DiskBasicDirItemCPM::ConvertFromFileNameStr(wxString &filename)
{
	filename = filename.Upper();
}

/// 使用しているアイテムか
bool DiskBasicDirItemCPM::CheckUsed(bool unuse)
{
	return (GetFileType1() != 0xe5);
}

/// 削除
/// @param [in] code : 削除コード(デフォルト0)
bool DiskBasicDirItemCPM::Delete(wxUint8 code)
{
	// 削除はエントリの先頭にコードを入れるだけ
	SetFileType1(code);
	used = false;

	// 複数ある時
	if (next_item) {
		next_item->Delete(code);
	}
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemCPM::Check(bool &last)
{
	if (!data) return false;

	bool valid = true;
	// ユーザIDが0～15でファイル名がオール0ならダメ
	if (GetFileType1() < 0x10) {
		wxUint8 name[sizeof(data->cpm.name) + 1];
		memcpy(name, data->cpm.name, sizeof(data->cpm.name));
		if (basic->IsDataInverted()) mem_invert(name, sizeof(data->cpm.name));
		valid = false;
		for(size_t n=0; n<sizeof(data->cpm.name); n++) {
			if (name[n] != 0) {
				valid = true;
				break;
			}
		}
	}
	return valid;
}

void DiskBasicDirItemCPM::SetFileAttr(int file_type)
{
	SetFileType1((file_type & FILETYPE_CPM_USERID_MASK) >> FILETYPE_CPM_USERID_POS);
	SetFileType2(file_type);

	// 複数ある場合
	if (next_item) {
		next_item->SetFileAttr(file_type);
	}
}

int DiskBasicDirItemCPM::GetFileAttr()
{
	int val = GetFileType1();	// user id
	val <<= FILETYPE_CPM_USERID_POS;
	val &= FILETYPE_CPM_USERID_MASK;
	val |= GetFileType2();		// attr in extend
	return val;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemCPM::GetFileType1Pos()
{
	return GetFileType1();
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemCPM::GetFileType2Pos()
{
	return GetFileType2();
}

int	DiskBasicDirItemCPM::CalcFileTypeFromPos(int pos1, int pos2)
{
	return (((pos1 << FILETYPE_CPM_USERID_POS) & FILETYPE_CPM_USERID_MASK) | pos2);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemCPM::GetFileAttrStr()
{
	int val = GetFileType2();
	wxString str;

	if (val & FILE_TYPE_BINARY_MASK) {
		str += wxGetTranslation(gTypeNameCPM_2[TYPE_NAME_CPM_BINARY]);
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
	file_size = ((val / SECTOR_UNIT_CPM) * SECTOR_UNIT_CPM);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemCPM::CalcFileSize()
{
	if (used) {
		file_size = 0;

		int bytes_per_group = basic->GetSectorSize() * basic->GetSectorsPerGroup();

		for(int map_pos=0; map_pos < group_entries; map_pos++) {
			if (GetGroup(map_pos) == 0) break;
			file_size += bytes_per_group;
			groups++;
		}

		if (next_item) {
			// ファイルには続きがある
			next_item->CalcFileSize();
			file_size += next_item->GetFileSize();
			groups += next_item->GetGroupSize();
		} else {
			// ファイル終り
			file_size = (GetExtentNumber() * SECTOR_UNIT_CPM + GetRecordNumber()) * SECTOR_UNIT_CPM;
			// ファイルサイズは1エントリ分にする
			int group_size = (bytes_per_group * group_entries);
			if (file_size > 0) file_size = ((file_size + group_size - 1) % group_size) + 1;
		}
	}
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemCPM::GetAllGroups(DiskBasicGroups &group_items)
{
	int bytes_per_group = basic->GetSectorSize() * basic->GetSectorsPerGroup();
	// グループ数を計算
	int map_size = GetGroupEntries();
	int remain_size = file_size;

	for(int map_pos=0; map_pos < map_size; map_pos++) {
		wxUint32 group_num = GetGroup(map_pos);
		if (group_num == 0) break;

		basic->GetNumsFromGroup(group_num, 0, basic->GetSectorSize(), remain_size, group_items);
		remain_size -= bytes_per_group;
	}

	if (next_item) {
		// ファイルには続きがある
		next_item->GetAllGroups(group_items);
	}
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemCPM::GetDataSize()
{
	return sizeof(directory_cpm_t);
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemCPM::NeedCheckEofCode()
{
	return (external_attr == 0);
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemCPM::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		// ただし、ファイルサイズが128バイトと合うなら終端記号は不要
		if ((file_size % SECTOR_UNIT_CPM) != 0) {
			file_size = CheckEofCode(istream, file_size);
		}
	}
	return file_size;
}

/// 最初のグループ番号を設定
void DiskBasicDirItemCPM::SetStartGroup(wxUint32 val)
{
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemCPM::GetStartGroup() const
{
	wxUint32 val = 0;
	val = group_width > 1 ? wxUINT16_SWAP_ON_BE(data->cpm.map.w[0]) : data->cpm.map.b[0];
	if (basic->IsDataInverted()) val ^= (group_width > 1 ? 0xffff : 0xff);	// invert
	return val;
}

#if 0
/// 書き込み/上書き禁止か
bool DiskBasicDirItemCPM::IsWriteProtected()
{
	return false;
}
#endif

bool DiskBasicDirItemCPM::IsDeletable()
{
	return true;
}
/// ファイル名を編集できるか
bool DiskBasicDirItemCPM::IsFileNameEditable()
{
	return true;
}

/// グループ番号をセット
void DiskBasicDirItemCPM::SetGroup(int pos, wxUint32 val)
{
	if (pos < 0 || pos >= group_entries) return;

	if (basic->IsDataInverted()) val = ~val;	// invert

	if (group_width > 1) {
		data->cpm.map.w[pos] = wxUINT16_SWAP_ON_BE(val);
	} else {
		data->cpm.map.b[pos] = (wxUint8)val;
	}
}

/// グループ番号を返す
wxUint32 DiskBasicDirItemCPM::GetGroup(int pos) const
{
	wxUint32 val = 0;
	if (pos < 0 || pos >= group_entries) return val;

	val = group_width > 1 ? wxUINT16_SWAP_ON_BE(data->cpm.map.w[pos]) : data->cpm.map.b[pos];
	if (basic->IsDataInverted()) val ^= (group_width > 1 ? 0xffff : 0xff);	// invert
	return val;
}

/// エクステント番号を返す
wxUint8 DiskBasicDirItemCPM::GetExtentNumber() const
{
	wxUint8 num = data->cpm.extent_num;
	if (basic->IsDataInverted()) num ^= 0xff;	// invert
	return num;
}

/// レコード番号を返す
wxUint8 DiskBasicDirItemCPM::GetRecordNumber() const
{
	wxUint8 num = data->cpm.record_num;
	if (basic->IsDataInverted()) num ^= 0xff;	// invert
	return num;
}

/// ファイルサイズからエクステント番号とレコード番号をセット
void DiskBasicDirItemCPM::CalcExtentAndRecordNumber(int val)
{
//	int limit_size = (basic->GetSectorSize() * basic->GetSectorsPerGroup() * group_entries);

	if (val == 0) {
		data->cpm.extent_num = 0;
		data->cpm.record_num = 0;
	} else {
		val = (val + SECTOR_UNIT_CPM - 1) / SECTOR_UNIT_CPM;
		if ((val % SECTOR_UNIT_CPM) == 0) {
			data->cpm.extent_num = (val / SECTOR_UNIT_CPM) - 1;
			data->cpm.record_num = SECTOR_UNIT_CPM;
		} else {
			data->cpm.extent_num = (val / SECTOR_UNIT_CPM);
			data->cpm.record_num = ((val - 1) % SECTOR_UNIT_CPM) + 1;
		}
	}
	if (basic->IsDataInverted()) {
		data->cpm.extent_num ^= 0xff;	// invert
		data->cpm.record_num ^= 0xff;	// invert
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

//
// ダイアログ用
//

#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_SPIN_USERID		51
#define IDC_CHECK_READONLY	52
#define IDC_CHECK_SYSTEM	53
#define IDC_CHECK_ARCHIVE	54
#define IDC_RADIO_BINASC	55

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemCPM::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_INVALID_FILE_TYPE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		wxString ext = name.Right(4);

		file_type_2 = GetFileTypeByExt(external_attr, ext);
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

	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_READ_ONLY]));
	chkReadOnly->SetValue((file_type_2 & FILE_TYPE_READONLY_MASK) != 0);
	staType4->Add(chkReadOnly, flags);
	chkSystem = new wxCheckBox(parent, IDC_CHECK_SYSTEM, wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_SYSTEM]));
	chkSystem->SetValue((file_type_2 & FILE_TYPE_SYSTEM_MASK) != 0);
	staType4->Add(chkSystem, flags);
	chkArchive = new wxCheckBox(parent, IDC_CHECK_ARCHIVE, wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_ARCHIVE]));
	chkArchive->SetValue((file_type_2 & FILE_TYPE_ARCHIVE_MASK) != 0);
	staType4->Add(chkArchive, flags);
	sizer->Add(staType4, flags);

}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemCPM::ChangeTypeInAttrDialog(IntNameBox *parent)
{
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemCPM::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxSpinCtrl *spnUserId = (wxSpinCtrl *)parent->FindWindow(IDC_SPIN_USERID);

	int val = spnUserId->GetValue();
	return val;
}

/// 属性2を得る
/// @return CalcFileTypeFromPos()のpos2に渡す値
int DiskBasicDirItemCPM::GetFileType2InAttrDialog(const IntNameBox *parent) const
{
	wxRadioBox *radBinAsc = (wxRadioBox *)parent->FindWindow(IDC_RADIO_BINASC);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkSystem = (wxCheckBox *)parent->FindWindow(IDC_CHECK_SYSTEM);
	wxCheckBox *chkArchive = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ARCHIVE);

	int val = 0;
	val |= chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkSystem->GetValue() ? FILE_TYPE_SYSTEM_MASK : 0;
	val |= chkArchive->GetValue() ? FILE_TYPE_ARCHIVE_MASK : 0;
	val |= radBinAsc->GetSelection() == TYPE_NAME_CPM_BINARY ? FILE_TYPE_BINARY_MASK : 0;

	return val;
}
