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

//
//
//
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	// グループ番号の幅は最大グループ番号から判定
	group_width = basic->GetFatEndGroup() >= 256 ? 2 : 1;

	next_item = NULL;
}
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, data)
{
	// グループ番号の幅は最大グループ番号から判定
	group_width = basic->GetFatEndGroup() >= 256 ? 2 : 1;

	next_item = NULL;
}
DiskBasicDirItemCPM::DiskBasicDirItemCPM(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	// グループ番号の幅は最大グループ番号から判定
	group_width = basic->GetFatEndGroup() >= 256 ? 2 : 1;

	next_item = NULL;

	used = CheckUsed(unuse);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemCPM::GetFileNamePos(size_t &len, bool *invert) const
{
	len = sizeof(data->cpm.name);
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
	return (int)sizeof(data->cpm.name);
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemCPM::GetFileExtSize(bool *invert) const
{
	return (int)sizeof(data->cpm.ext);
}

/// 属性１を返す
int	DiskBasicDirItemCPM::GetFileType1() const
{
	return data->cpm.type;	// user id
}

/// 属性２を返す
int	DiskBasicDirItemCPM::GetFileType2() const
{
	int val = 0;
	val |= (data->cpm.ext[0] & 0x80 ? FILE_TYPE_READONLY_MASK : 0);	// read only
	val |= (data->cpm.ext[1] & 0x80 ? FILE_TYPE_SYSTEM_MASK : 0);	// system
	val |= (data->cpm.ext[2] & 0x80 ? FILE_TYPE_ARCHIVE_MASK : 0);	// archive
	return val;
}

/// 属性１を設定
void DiskBasicDirItemCPM::SetFileType1(int val)
{
	data->cpm.type = val & 0xff;	// user id
}

/// 属性２を設定
void DiskBasicDirItemCPM::SetFileType2(int val)
{
	data->cpm.ext[0] = (data->cpm.ext[0] & 0x7f) | (val & FILE_TYPE_READONLY_MASK ? 0x80 : 0);
	data->cpm.ext[1] = (data->cpm.ext[1] & 0x7f) | (val & FILE_TYPE_SYSTEM_MASK ? 0x80 : 0);
	data->cpm.ext[2] = (data->cpm.ext[2] & 0x7f) | (val & FILE_TYPE_ARCHIVE_MASK ? 0x80 : 0);
}

/// ファイル名を得る
void DiskBasicDirItemCPM::GetFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen)
{
	DiskBasicDirItem::GetFileName(name, nlen, ext, elen);

	// 拡張子部分のMSBは属性ビットなので除く
	for(size_t en = 0; en < elen; en++) {
		ext[en] &= 0x7f;
	}
}

/// ファイル名を設定
void DiskBasicDirItemCPM::SetFileName(const wxUint8 *filename, int length)
{
	DiskBasicDirItem::SetFileName(filename, length);

	// 複数ある時
	if (next_item) {
		next_item->SetFileName(filename, length);
	}
}

/// 拡張子を設定
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
	return (data->cpm.type != 0xe5);
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

	return true;
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

#if 0
/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemCPM::ClearData()
{
	if (!data) return;
	memset(data, 0, sizeof(directory_cpm_t));
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemCPM::InitialData()
{
	ClearData();
}
#endif

int DiskBasicDirItemCPM::GetFileType()
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
	file_size = val;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemCPM::CalcFileSize()
{
	if (used) {
		int bytes_per_group = basic->GetSectorSize() * basic->GetSectorsPerGroup();

		// グループ数を計算
		int map_size = (3 - group_width) * 8;

		for(int map_pos=0; map_pos < map_size; map_pos++) {
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
			file_size = (GetExtentNumber() * 128 + GetRecordNumber()) * 128;
			// ファイルサイズは1エントリ分にする
			file_size %= (bytes_per_group * map_size);
		}
	}
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemCPM::GetAllGroups(DiskBasicGroups &group_items)
{
	int bytes_per_group = basic->GetSectorSize() * basic->GetSectorsPerGroup();
	// グループ数を計算
	int map_size = (3 - group_width) * 8;
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

/// 最初のグループ番号を設定
void DiskBasicDirItemCPM::SetStartGroup(wxUint32 val)
{
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemCPM::GetStartGroup() const
{
	wxUint32 val = 0;
	val = group_width > 1 ? wxUINT16_SWAP_ON_BE(data->cpm.map.w[0]) : data->cpm.map.b[0];
	return val;
}

/// 書き込み/上書き禁止か
bool DiskBasicDirItemCPM::IsWriteProtected()
{
	return false;
}
bool DiskBasicDirItemCPM::IsDeleteable()
{
	return false;
}
/// ファイル名を編集できるか
bool DiskBasicDirItemCPM::IsFileNameEditable()
{
	return true;
}

/// グループ番号をセット
void DiskBasicDirItemCPM::SetGroup(int pos, wxUint32 val)
{
}

/// グループ番号を返す
wxUint32 DiskBasicDirItemCPM::GetGroup(int pos) const
{
	wxUint32 val = 0;
	if (pos < 0 || pos >= ((3 - group_width) * 8)) return val;

	val = group_width > 1 ? wxUINT16_SWAP_ON_BE(data->cpm.map.w[pos]) : data->cpm.map.b[pos];
	return val;
}

/// エクステント番号を返す
wxUint8 DiskBasicDirItemCPM::GetExtentNumber() const
{
	return data->cpm.extent_num;
}

/// レコード番号を返す
wxUint8 DiskBasicDirItemCPM::GetRecordNumber() const
{
	return data->cpm.record_num;
}

/// アイテムソート用
int DiskBasicDirItemCPM::Compare(DiskBasicDirItem **item1, DiskBasicDirItem **item2)
{
	directory_t *d1 = (*item1)->GetData(); 
	directory_t *d2 = (*item2)->GetData(); 
	// ファイル名＋拡張子＋エクステント番号
	int cmp = 0;
	cmp = memcmp(&d1->cpm, &d2->cpm, 13);
	// ＋レコード番号(逆)
	if (cmp == 0) cmp = memcmp(&d2->cpm.record_num, &d1->cpm.record_num, 1);
	// ＋マップ
	if (cmp == 0) cmp = memcmp(&d1->cpm.map, &d2->cpm.map, 1);
	return cmp;
}

/// 名前比較
int DiskBasicDirItemCPM::CompareName(DiskBasicDirItem **item1, DiskBasicDirItem **item2)
{
	directory_t *d1 = (*item1)->GetData(); 
	directory_t *d2 = (*item2)->GetData(); 
	// ファイル名＋拡張子
	int cmp = 0;
	cmp = memcmp(&d1->cpm.name, &d2->cpm.name, sizeof(d1->cpm.name) + sizeof(d1->cpm.ext));
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

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] file_type_1    ファイル属性1 GetFileType1Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] file_type_2    ファイル属性2 GetFileType2Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] sizer
/// @param [in] flags
/// @param [in,out] controls   [0]: wxTextCtrl::txtIntNameで予約済み [1]からユーザ設定
/// @param [in,out] user_data  ユーザ定義データ
void DiskBasicDirItemCPM::CreateControlsForAttrDialog(IntNameBox *parent, int file_type_1, int file_type_2, wxBoxSizer *sizer, wxSizerFlags &flags, AttrControls &controls, int *user_data)
{
	wxSpinCtrl *spnUserId;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkSystem;
	wxCheckBox *chkArchive;

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
	chkArchive = new wxCheckBox(parent, IDC_CHECK_SYSTEM, wxGetTranslation(gTypeNameCPM[TYPE_NAME_CPM_ARCHIVE]));
	chkArchive->SetValue((file_type_2 & FILE_TYPE_ARCHIVE_MASK) != 0);
	staType4->Add(chkArchive, flags);
	sizer->Add(staType4, flags);

	controls.Add(spnUserId);
	controls.Add(chkReadOnly);
	controls.Add(chkSystem);
	controls.Add(chkArchive);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemCPM::ChangeTypeInAttrDialog(AttrControls &controls)
{
}

/// ダイアログ用に属性を設定する
/// インポート時ダイアログ表示前にファイルの属性を設定
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemCPM::SetFileTypeForAttrDialog(const wxString &name, int &file_type_1, int &file_type_2)
{
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemCPM::GetFileType1InAttrDialog(const AttrControls &controls) const
{
	wxSpinCtrl *spnUserId = (wxSpinCtrl *)controls.Item(1);

	int val = spnUserId->GetValue();
	return val;
}

/// 属性2を得る
/// @return CalcFileTypeFromPos()のpos2に渡す値
int DiskBasicDirItemCPM::GetFileType2InAttrDialog(const AttrControls &controls, const int *user_data) const
{
	wxCheckBox *chkReadOnly = (wxCheckBox *)controls.Item(2);
	wxCheckBox *chkSystem = (wxCheckBox *)controls.Item(3);
	wxCheckBox *chkArchive = (wxCheckBox *)controls.Item(4);

	int val = 0;
	val |= chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkSystem->GetValue() ? FILE_TYPE_SYSTEM_MASK : 0;
	val |= chkArchive->GetValue() ? FILE_TYPE_ARCHIVE_MASK : 0;

	return val;
}
