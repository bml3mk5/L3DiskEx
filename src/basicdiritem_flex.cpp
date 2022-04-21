/// @file basicdiritem_flex.cpp
///
/// @brief disk basic directory item for FLEX
///
#include "basicdiritem_flex.h"
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
DiskBasicDirItemFLEX::DiskBasicDirItemFLEX(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, data)
{
}
DiskBasicDirItemFLEX::DiskBasicDirItemFLEX(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	used = CheckUsed(unuse);

	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemFLEX::GetFileNamePos(size_t &len, bool *invert) const
{
	len = sizeof(data->flex.name);
	return data->flex.name; 
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemFLEX::GetFileNameSize(bool *invert) const
{
	return (int)sizeof(data->flex.name);
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemFLEX::GetFileExtPos(size_t &len) const
{
	len = sizeof(data->flex.ext);
	return data->flex.ext;
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemFLEX::GetFileExtSize(bool *invert) const
{
	return (int)sizeof(data->flex.ext);
}

/// 属性１を返す
int	DiskBasicDirItemFLEX::GetFileType1() const
{
	return data->flex.type;
}

/// 属性１を設定
void DiskBasicDirItemFLEX::SetFileType1(int val)
{
	data->flex.type = (val & 0xff);
}

/// 属性２を返す
int	DiskBasicDirItemFLEX::GetFileType2() const
{
	return data->flex.random_access;
}

/// 属性２を設定
void DiskBasicDirItemFLEX::SetFileType2(int val)
{
	data->flex.random_access = (val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemFLEX::CheckUsed(bool unuse)
{
	return (data->flex.name[0] != 0 && data->flex.name[0] != 0xff);
}

/// 削除
/// @param [in] code : 削除コード(デフォルト0)
bool DiskBasicDirItemFLEX::Delete(wxUint8 code)
{
	// 削除はエントリの先頭にコードを入れるだけ
	data->flex.name[0] = code;
	used = false;
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemFLEX::Check(bool &last)
{
	if (!data) return false;

	bool valid = true;

	if (data->flex.name[0] == 0) {
		last = true;
		return valid;
	}
	// 属性 0-3bitはゼロ
	if (data->flex.type & 0x0f) {
		valid = false;
	}
	return valid;
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemFLEX::InvalidateChars()
{
	return wxT(" !\"#$%&'()*+,/:;<=>?@[\\]^{|}~");
}

/// ダイアログ入力前のファイル名文字列を変換 大文字にする
void DiskBasicDirItemFLEX::ConvertToFileNameStr(wxString &filename)
{
	filename = filename.Upper();
}

/// ダイアログ入力後のファイル名文字列を変換 大文字にする
void DiskBasicDirItemFLEX::ConvertFromFileNameStr(wxString &filename)
{
	filename = filename.Upper();
}

void DiskBasicDirItemFLEX::SetFileAttr(int file_type)
{
	int val = 0;
	if (file_type & FILE_TYPE_READONLY_MASK) {
		val |= FILETYPE_MASK_FLEX_READ_ONLY;
	}
	if (file_type & FILE_TYPE_UNDELETE_MASK) {
		val |= FILETYPE_MASK_FLEX_UNDELETE;
	}
	if (file_type & FILE_TYPE_WRITEONLY_MASK) {
		val |= FILETYPE_MASK_FLEX_WRITE_ONLY;
	}
	if (file_type & FILE_TYPE_HIDDEN_MASK) {
		val |= FILETYPE_MASK_FLEX_HIDDEN;
	}
	if (file_type & FILE_TYPE_RANDOM_MASK) {
		int val2 = (file_type >> FILETYPE_FLEX_RANDOM_POS);
		SetFileType2(val2 ? val2 : 0x02);
	}

	SetFileType1(val);
}

#if 0
/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemFLEX::ClearData()
{
	if (!data) return;
	memset(data, 0, sizeof(directory_flex_t));
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemFLEX::InitialData()
{
	ClearData();
}
#endif

int DiskBasicDirItemFLEX::GetFileType()
{
	int val = 0;
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
		val |= (GetFileType2() << FILETYPE_FLEX_RANDOM_POS);
	}
	return val;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFLEX::GetFileType1Pos()
{
	return GetFileType();
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFLEX::GetFileType2Pos()
{
	return GetFileType2();
}

int	DiskBasicDirItemFLEX::CalcFileTypeFromPos(int pos1, int pos2)
{
	return (pos1 | pos2);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemFLEX::GetFileAttrStr()
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
	file_size = val;
	val = val / (basic->GetSectorSize() - 4);
	data->flex.total_sectors = wxUINT16_SWAP_ON_LE(val);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemFLEX::CalcFileSize()
{
	if (!used) return;

	// セクタ先頭4バイトを除く
	int sec_size = (basic->GetSectorSize() - 4);

	// ファイルサイズ
	file_size = wxUINT16_SWAP_ON_LE(data->flex.total_sectors) * sec_size;

	int calc_file_size = 0;
	int calc_groups = 0;
//	int calc_last_group = 0;
//	int calc_last_sector = 0;
	bool rc = true;

//	DiskD88Disk *disk = basic->GetDisk();
	directory_flex_t *d = &data->flex;

	int track_num  = d->start_track;
	int sector_num = d->start_sector;

	int limit = basic->GetFatEndGroup() + 1;
	while(limit >= 0) {
		DiskD88Sector *sector = basic->GetSector(track_num, sector_num);
		if (!sector) {
			// error
			rc = false;
			break;
		}
		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
		track_num = p->next_track;
		sector_num = p->next_sector;

		calc_file_size += sec_size;
		calc_groups++;

		if (track_num == 0 || sector_num == 0) {
			// 最終セクタは0パディング部分のサイズを減らす
			wxUint8 *buf = sector->GetSectorBuffer();
			for(int pos = sector->GetSectorSize() - 1; pos >= 4; pos--) {
				if (buf[pos] != 0) break;
				calc_file_size--;
			}
			break;
		}

		limit--;
	}
	if (limit < 0) {
		rc = false;
	}

	// グループ数を計算
	if (rc) {
		file_size = calc_file_size;
		groups = calc_groups;
	}
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemFLEX::GetAllGroups(DiskBasicGroups &group_items)
{
//	bool rc = true;

//	DiskD88Disk *disk = basic->GetDisk();
	directory_flex_t *d = &data->flex;

	int track_num  = d->start_track;
	int sector_num = d->start_sector;
	int next_track_num  = 0;
	int next_sector_num = 0;

	int limit = basic->GetFatEndGroup() + 1;
	while((track_num != 0 || sector_num != 0) && limit >= 0) {
		DiskD88Sector *sector = basic->GetSector(track_num, sector_num);
		if (!sector) {
			// error
//			rc = false;
			break;
		}
		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
		next_track_num = p->next_track;
		next_sector_num = p->next_sector;

		wxUint32 gnum = (wxUint32)basic->GetSectorPosFromNum(track_num, sector_num);
		wxUint32 next_gnum = (wxUint32)basic->GetSectorPosFromNum(next_track_num, next_sector_num);

		int side_num = ((sector_num - 1) / basic->GetSectorsPerTrackOnBasic());
		group_items.Add(gnum, next_gnum, track_num, side_num, sector_num, sector_num);

//		file_size += (sector->GetSectorSize() - 4);
//		groups++;

		track_num = next_track_num;
		sector_num = next_sector_num;

		limit--;
	}

	group_items.SetSize(file_size);

//	if (limit < 0) {
//		rc = false;
//	}
}

void DiskBasicDirItemFLEX::GetFileDate(struct tm *tm)
{
	tm->tm_year = (data->flex.year % 100);
	if (tm->tm_year < 80) tm->tm_year += 100;
	tm->tm_mon = data->flex.month - 1; 
	tm->tm_mday = data->flex.day;
}

void DiskBasicDirItemFLEX::GetFileTime(struct tm *tm)
{
	tm->tm_hour = 0; 
	tm->tm_min = 0;
	tm->tm_sec = 0;
}

wxString DiskBasicDirItemFLEX::GetFileDateStr()
{
	struct tm tm;
	GetFileDate(&tm);
	return L3DiskUtils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemFLEX::GetFileTimeStr()
{
	return wxEmptyString;
}

void DiskBasicDirItemFLEX::SetFileDate(const struct tm *tm)
{
	data->flex.year = (tm->tm_year % 100);
	data->flex.month = tm->tm_mon + 1;
	data->flex.day = tm->tm_mday;
}

void DiskBasicDirItemFLEX::SetFileTime(const struct tm *tm)
{
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemFLEX::GetDataSize()
{
	return sizeof(directory_flex_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemFLEX::SetStartGroup(wxUint32 val)
{
	int trk_num = 0;
	int sec_num = 0;
	basic->GetNumFromSectorPos(val, trk_num, sec_num);
	data->flex.start_track = trk_num;
	data->flex.start_sector = sec_num;
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemFLEX::GetStartGroup() const
{
	wxUint32 val = (wxUint32)basic->GetSectorPosFromNum(data->flex.start_track, data->flex.start_sector);
	return val;
}

/// 書き込み/上書き禁止か
bool DiskBasicDirItemFLEX::IsWriteProtected()
{
	return false;
}
bool DiskBasicDirItemFLEX::IsDeletable()
{
	return false;
}
/// ファイル名を編集できるか
bool DiskBasicDirItemFLEX::IsFileNameEditable()
{
	return true;
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

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] file_type_1    ファイル属性1 GetFileType1Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] file_type_2    ファイル属性2 GetFileType2Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] sizer
/// @param [in] flags
/// @param [in,out] controls   [0]: wxTextCtrl::txtIntNameで予約済み [1]からユーザ設定
/// @param [in,out] user_data  ユーザ定義データ
void DiskBasicDirItemFLEX::CreateControlsForAttrDialog(IntNameBox *parent, int file_type_1, int file_type_2, wxBoxSizer *sizer, wxSizerFlags &flags, AttrControls &controls, int *user_data)
{
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkUndelete;
	wxCheckBox *chkWriteOnly;
	wxCheckBox *chkHidden;
	wxCheckBox *chkRandom;

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
	*user_data = (file_type_1 & FILETYPE_FLEX_RANDOM_MASK);

	controls.Add(chkReadOnly);
	controls.Add(chkUndelete);
	controls.Add(chkWriteOnly);
	controls.Add(chkHidden);
	controls.Add(chkRandom);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemFLEX::ChangeTypeInAttrDialog(AttrControls &controls)
{
}

/// ダイアログ用に属性を設定する
/// インポート時ダイアログ表示前にファイルの属性を設定
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemFLEX::SetFileTypeForAttrDialog(const wxString &name, int &file_type_1, int &file_type_2)
{
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemFLEX::GetFileType1InAttrDialog(const AttrControls &controls) const
{
	wxCheckBox *chkReadOnly = (wxCheckBox *)controls.Item(1);
	wxCheckBox *chkUndelete = (wxCheckBox *)controls.Item(2);
	wxCheckBox *chkWriteOnly = (wxCheckBox *)controls.Item(3);
	wxCheckBox *chkHidden = (wxCheckBox *)controls.Item(4);

	int val = chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkUndelete->GetValue() ? FILE_TYPE_UNDELETE_MASK : 0;
	val |= chkWriteOnly->GetValue() ? FILE_TYPE_WRITEONLY_MASK : 0;
	val |= chkHidden->GetValue() ? FILE_TYPE_HIDDEN_MASK : 0;

	return val;
}

/// 属性2を得る
/// @return CalcFileTypeFromPos()のpos2に渡す値
int DiskBasicDirItemFLEX::GetFileType2InAttrDialog(const AttrControls &controls, const int *user_data) const
{
	wxCheckBox *chkRandom = (wxCheckBox *)controls.Item(5);

	// ユーザ定義データ(ランダムファイル属性値)
	return (chkRandom->GetValue() ? (FILE_TYPE_RANDOM_MASK | *user_data) : 0);
}
