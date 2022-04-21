/// @file basicdiritem_fat8.cpp
///
/// @brief disk basic directory item
///
#include "basicdiritem_fat8.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"

//
//
//

/// L3/S1/F BASIC タイプ1 0...BASIC 1...DATA 2...MACHINE
const char *gTypeName1[] = {
	wxTRANSLATE("BASIC"),
	wxTRANSLATE("Data"),
	wxTRANSLATE("Machine"),
	("???"),
	NULL
};
/// L3/S1/F BASIC タイプ2 0...Binary 1...Ascii 2...Random Access
const char *gTypeName2[] = {
	wxTRANSLATE("Binary"),
	wxTRANSLATE("Ascii"),
	wxTRANSLATE("Random Access"),
	NULL
};

///
///
///
DiskBasicDirItemFAT8::DiskBasicDirItemFAT8(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemFAT8::DiskBasicDirItemFAT8(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, data)
{
}
DiskBasicDirItemFAT8::DiskBasicDirItemFAT8(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	used = CheckUsed(unuse);
}

/// 使用しているアイテムか
bool DiskBasicDirItemFAT8::CheckUsed(bool unuse)
{
	return (data->name[0] != 0 && data->name[0] != 0xff);
}

/// 属性を設定する
void DiskBasicDirItemFAT8::SetFileAttr(int file_type)
{
	if (file_type == -1) return;

	SetFileType1(
		file_type & FILE_TYPE_BASIC_MASK ? TYPE_NAME_1_BASIC : (
		file_type & FILE_TYPE_DATA_MASK ? TYPE_NAME_1_DATA : (
		file_type & FILE_TYPE_MACHINE_MASK ? TYPE_NAME_1_MACHINE : (
		0))));

	SetFileType3(0);
	if (file_type & FILE_TYPE_BINARY_MASK) {
		SetFileType2(0);
	} else if (file_type & FILE_TYPE_ASCII_MASK) {
		SetFileType2(0xff);
	} else if (file_type & FILE_TYPE_RANDOM_MASK) {
		SetFileType2(0xff);
		SetFileType3(0xff);
	}
}

int DiskBasicDirItemFAT8::GetFileType()
{
	int t1 = GetFileType1();
	int val = (t1 >= TYPE_NAME_1_BASIC && t1 <= TYPE_NAME_1_MACHINE ? 1 << t1 : 0);
	int t2 = GetFileType2();
	int t3 = GetFileType3();
	val |= (t2 & 1 ? (t3 & 1 ? FILE_TYPE_RANDOM_MASK : FILE_TYPE_ASCII_MASK) : FILE_TYPE_BINARY_MASK);
	return val;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFAT8::GetFileType1Pos()
{
	int t1 = GetFileType1();
	if (t1 < TYPE_NAME_1_BASIC || t1 > TYPE_NAME_1_MACHINE) {
		t1 = TYPE_NAME_1_UNKNOWN;
	}
	return t1;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFAT8::GetFileType2Pos()
{
	int t2 = GetFileType2();
	int t3 = GetFileType3();
	t2 = (t2 & 1 ? (t3 & 1 ? TYPE_NAME_2_RANDOM : TYPE_NAME_2_ASCII) : TYPE_NAME_2_BINARY);
	return t2;
}

int	DiskBasicDirItemFAT8::CalcFileTypeFromPos(int pos1, int pos2)
{
	int val = (pos1 >= TYPE_NAME_1_BASIC && pos1 <= TYPE_NAME_1_MACHINE ? 1 << pos1 : 0);
	switch(pos2) {
	case TYPE_NAME_2_BINARY:
		val |= FILE_TYPE_BINARY_MASK;
		break;
	case TYPE_NAME_2_ASCII:
		val |= FILE_TYPE_ASCII_MASK;
		break;
	case TYPE_NAME_2_RANDOM:
		val |= FILE_TYPE_RANDOM_MASK;
		break;
	}
	return val;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemFAT8::GetFileAttrStr()
{
	wxString attr;
	attr = wxGetTranslation(gTypeName1[GetFileType1Pos()]);
	attr += wxT(" - ");
	attr += wxGetTranslation(gTypeName2[GetFileType2Pos()]);
	return attr;
}

/// ファイルサイズを計算
void DiskBasicDirItemFAT8::CalcFileSize()
{
	if (!used) return;

	int calc_file_size = 0;
	int calc_groups = 0; 
	wxUint32 last_group = GetStartGroup();
//	int last_sector = 0;

	// 8bit FAT
	bool rc = true;
	wxUint32 group_num = last_group;
	bool working = true;
	int limit = basic->GetFatEndGroup() + 1;
	while(working) {
		wxUint32 next_group = type->GetGroupNumber(group_num);
		if (next_group == group_num) {
			// 同じポジションならエラー
			rc = false;
		} else if (next_group >= basic->GetGroupSystemCode()) {
			// システム領域はエラー(0xfe - )
			rc = false;
		} else if (next_group >= basic->GetGroupFinalCode()) {
			// 最終グループ(0xc1 - )
//			last_sector = next_group;
			calc_file_size += (basic->GetSectorSize() * (next_group - basic->GetGroupFinalCode() + 1));
			calc_groups++;
			working = false;
		} else if (next_group > (wxUint32)basic->GetFatEndGroup()) {
			// グループ番号がおかしい
			rc = false;
		} else {
			calc_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			calc_groups++;
			last_group = group_num;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}

	if (rc) {
		file_size = calc_file_size;
		groups = calc_groups;
	}
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemFAT8::GetAllGroups(DiskBasicGroups &group_items)
{
//	file_size = 0;
//	groups = 0; 

	// 8bit FAT
	bool rc = true;
	wxUint32 group_num = GetStartGroup();
	bool working = true;
	int limit = basic->GetFatEndGroup() + 1;
	while(working) {
		wxUint32 next_group = type->GetGroupNumber(group_num);
		if (next_group == group_num) {
			// 同じポジションならエラー
			rc = false;
		} else if (next_group >= basic->GetGroupSystemCode()) {
			// システム領域はエラー(0xfe - )
			rc = false;
		} else if (next_group >= basic->GetGroupFinalCode()) {
			// 最終グループ(0xc1 - )
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), 0, group_items);
//			file_size += (sector_size * (next_group - group_final_code + 1));
//			groups++;
			working = false;
		} else if (next_group > (wxUint32)basic->GetFatEndGroup()) {
			// グループ番号がおかしい
			rc = false;
		} else {
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), 0, group_items);
//			file_size += (sector_size * secs_per_group);
//			groups++;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}

	group_items.SetSize(file_size);

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
}

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_RADIO_TYPE1 51
#define IDC_RADIO_TYPE2 52

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] file_type_1    ファイル属性1 GetFileType1Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] file_type_2    ファイル属性2 GetFileType2Pos() / インポート時 SetFileTypeForAttrDialog()で設定
/// @param [in] sizer
/// @param [in] flags
/// @param [in,out] controls   [0]: wxTextCtrl::txtIntNameで予約済み [1]からユーザ設定
/// @param [in,out] user_data  ユーザ定義データ
void DiskBasicDirItemFAT8::CreateControlsForAttrDialog(IntNameBox *parent, int file_type_1, int file_type_2, wxBoxSizer *sizer, wxSizerFlags &flags, AttrControls &controls, int *user_data)
{
	wxRadioBox *radType1;
	wxRadioBox *radType2;

	wxArrayString types1;
	for(size_t i=0; i<=TYPE_NAME_1_MACHINE; i++) {
		types1.Add(wxGetTranslation(gTypeName1[i]));
	}
	radType1 = new wxRadioBox(parent, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 0, wxRA_SPECIFY_COLS);
	radType1->SetSelection(file_type_1);
	sizer->Add(radType1, flags);

	wxArrayString types2;
	for(size_t i=0; i<=TYPE_NAME_2_RANDOM; i++) {
		types2.Add(wxGetTranslation(gTypeName2[i]));
	}

	if (format_type == FORMAT_TYPE_L3S1_2D) {
		// 2D
		if (file_type_2 == TYPE_NAME_2_RANDOM) {
			// 1Sから2Dへのコピーでランダムアクセスのデータはアスキーとする。
			file_type_2 = TYPE_NAME_2_ASCII;
		}
		types2.RemoveAt(TYPE_NAME_2_RANDOM);
	}

	radType2 = new wxRadioBox(parent, IDC_RADIO_TYPE2, _("Data Type"), wxDefaultPosition, wxDefaultSize, types2, 0, wxRA_SPECIFY_ROWS);
	radType2->SetSelection(file_type_2);
	sizer->Add(radType2, flags);

	// event handler
	parent->Bind(wxEVT_RADIOBOX, &IntNameBox::OnChangeType1, parent, IDC_RADIO_TYPE1);


	controls.Add(radType1);
	controls.Add(radType2);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemFAT8::ChangeTypeInAttrDialog(AttrControls &controls)
{
	wxTextCtrl *txtIntName = (wxTextCtrl *)controls.Item(0);
	wxRadioBox *radType1 = (wxRadioBox *)controls.Item(1);
	wxRadioBox *radType2 = (wxRadioBox *)controls.Item(2);

	int selected_idx = 0;
	if (radType1) {
		selected_idx = radType1->GetSelection();
	}

	if (radType2) {
		int cnt = (int)radType2->GetCount();
		int cur_pos = radType2->GetSelection();

		radType2->Enable(0, true);
		radType2->Enable(1, true);
		if (cnt > 2) radType2->Enable(2, true);

		if (selected_idx == 0) {
			// BASIC
			if (cnt > 2) {
				if (cur_pos == 2) {
					radType2->SetSelection(0);
				}
				radType2->Enable(2, false);	// ランダムアクセス指定不可
			}
		} else if (selected_idx == 1) {
			// データ
			if (cur_pos == 0) {
				radType2->SetSelection(1);
			}
			radType2->Enable(0, false);	// バイナリ指定不可
		} else if (selected_idx == 2) {
			// 機械語
			radType2->SetSelection(0);
			radType2->Enable(1, false);	// アスキー指定不可
			if (cnt > 2) {
				radType2->Enable(2, false);	// ランダムアクセス指定不可
			}
		}
		// 拡張子を付加
		txtIntName->SetValue(AddExtensionForAttrDialog(selected_idx, txtIntName->GetValue()));
	}
}

/// ダイアログ用に属性を設定する
/// インポート時ダイアログ表示前にファイルの属性を設定
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemFAT8::SetFileTypeForAttrDialog(const wxString &name, int &file_type_1, int &file_type_2)
{
	// 拡張子で属性を設定する
	wxString ext = name.Right(4).Upper();
	if (ext == wxT(".BAS")) {
		file_type_1 = TYPE_NAME_1_BASIC;
		file_type_2 = TYPE_NAME_2_BINARY;
	} else if (ext == wxT(".DAT") || ext == wxT(".TXT")) {
		file_type_1 = TYPE_NAME_1_DATA;
		file_type_2 = TYPE_NAME_2_ASCII;
	} else if (ext == wxT(".BIN")) {
		file_type_1 = TYPE_NAME_1_MACHINE;
		file_type_2 = TYPE_NAME_2_BINARY;
	}
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemFAT8::GetFileType1InAttrDialog(const AttrControls &controls) const
{
	wxRadioBox *radType1 = (wxRadioBox *)controls.Item(1);

	return radType1->GetSelection();
}

/// 属性2を得る
/// @return CalcFileTypeFromPos()のpos2に渡す値
int DiskBasicDirItemFAT8::GetFileType2InAttrDialog(const AttrControls &controls, const int *user_data) const
{
	wxRadioBox *radType2 = (wxRadioBox *)controls.Item(2);

	return radType2->GetSelection();
}

///
///
///
DiskBasicDirItemFAT8F::DiskBasicDirItemFAT8F(DiskBasic *basic)
	: DiskBasicDirItemFAT8(basic)
{
}
DiskBasicDirItemFAT8F::DiskBasicDirItemFAT8F(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItemFAT8(basic, sector, data)
{
}
DiskBasicDirItemFAT8F::DiskBasicDirItemFAT8F(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemFAT8(basic, num, track, side, sector, secpos, data, unuse)
{
	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemFAT8F::Check(bool &last)
{
	if (!data) return false;

	bool valid = true;
	directory_fat8f_t *p = (directory_fat8f_t *)data;
	if (p->name[0] == 0xff) {
		last = true;
		return valid;
	}
	// 属性に想定外の値がある場合はエラー
	if (p->type2 != 0 && p->type2 != 0xff) {
		valid = false;
	} else if (p->type3 != 0 && p->type3 != 0xff) {
		valid = false;
	}
	return valid;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemFAT8F::GetFileNamePos(size_t &len, bool *invert) const
{
	// 8chars
	len = sizeof(data->fat8f.name);
	return data->fat8f.name;
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemFAT8F::GetFileNameSize(bool *invert) const
{
	return (int)sizeof(data->fat8f.name);
}

/// 属性１を返す
int	DiskBasicDirItemFAT8F::GetFileType1() const
{
	return data->fat8f.type;
}

/// 属性２を返す
int	DiskBasicDirItemFAT8F::GetFileType2() const
{
	return data->fat8f.type2;
}

/// 属性３を返す
int DiskBasicDirItemFAT8F::GetFileType3() const
{
	return data->fat8f.type3;
}

/// 属性１を設定
void DiskBasicDirItemFAT8F::SetFileType1(int val)
{
	data->fat8f.type = val & 0xff;
}

/// 属性２を設定
void DiskBasicDirItemFAT8F::SetFileType2(int val)
{
	data->fat8f.type2 = val & 0xff;
}

/// 属性３を設定
void DiskBasicDirItemFAT8F::SetFileType3(int val)
{
	data->fat8f.type3 = val & 0xff;
}

/// ディレクトリのサイズ
size_t DiskBasicDirItemFAT8F::GetDataSize()
{
	return sizeof(directory_fat8f_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemFAT8F::SetStartGroup(wxUint32 val)
{
	data->fat8f.start_group = (val & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemFAT8F::GetStartGroup() const
{
	return data->fat8f.start_group;
}

/// ファイルのサイズ
void DiskBasicDirItemFAT8F::SetFileSize(int val)
{
	// ファイルサイズはセクタサイズ境界で丸める
	int sector_size = basic->GetSectorSize();
	file_size = (((val - 1) / sector_size) + 1) * sector_size;
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemFAT8F::NeedCheckEofCode()
{
	// ランダムアクセス時は除く
	return (GetFileType3() != 0xff);
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemFAT8F::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		file_size = CheckEofCode(istream, file_size);
	}
	return file_size;
}
