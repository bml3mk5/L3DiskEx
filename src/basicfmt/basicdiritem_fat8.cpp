/// @file basicdiritem_fat8.cpp
///
/// @brief disk basic directory item
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_fat8.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../config.h"
#include "../charcodes.h"


//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemFAT8::DiskBasicDirItemFAT8(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_start_address = -1;
	m_end_address = -1;
	m_exec_address = -1;
}
DiskBasicDirItemFAT8::DiskBasicDirItemFAT8(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_start_address = -1;
	m_end_address = -1;
	m_exec_address = -1;
}
DiskBasicDirItemFAT8::DiskBasicDirItemFAT8(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_start_address = -1;
	m_end_address = -1;
	m_exec_address = -1;

	Used(CheckUsed(n_unuse));
}

/// 属性を設定する
void DiskBasicDirItemFAT8::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	SetFileType1(
		ftype & FILE_TYPE_BASIC_MASK ? TYPE_NAME_1_BASIC : (
		ftype & FILE_TYPE_DATA_MASK ? TYPE_NAME_1_DATA : (
		ftype & FILE_TYPE_MACHINE_MASK ? TYPE_NAME_1_MACHINE : (
		0))));

	SetFileType3(0);
	if (ftype & FILE_TYPE_BINARY_MASK) {
		SetFileType2(0);
	} else if (ftype & FILE_TYPE_ASCII_MASK) {
		SetFileType2(0xff);
	} else if (ftype & FILE_TYPE_RANDOM_MASK) {
		SetFileType2(0xff);
		SetFileType3(0xff);
	}
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemFAT8::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = (t1 >= TYPE_NAME_1_BASIC && t1 <= TYPE_NAME_1_MACHINE ? 1 << t1 : 0);
	int t2 = GetFileType2();
	int t3 = GetFileType3();
	val |= (t2 & 1 ? (t3 & 1 ? FILE_TYPE_RANDOM_MASK : FILE_TYPE_ASCII_MASK) : FILE_TYPE_BINARY_MASK);
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t3 << 16 | t2 << 8 | t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemFAT8::GetFileAttrStr() const
{
	wxString attr;
	attr = wxGetTranslation(gTypeName1[GetFileType1Pos()]);
	attr += wxT(" - ");
	attr += wxGetTranslation(gTypeName2[GetFileType2Pos()]);
	return attr;
}

/// ファイルサイズを計算
void DiskBasicDirItemFAT8::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemFAT8::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
//	file_size = 0;
//	groups = 0; 
	int calc_file_size = 0;
	int calc_groups = 0; 

	// 8bit FAT
	bool rc = true;
	wxUint32 group_num = GetStartGroup(fileunit_num);
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
			calc_file_size += (basic->GetSectorSize() * (next_group - basic->GetGroupFinalCode() + 1));
			calc_groups++;
			calc_file_size = RecalcFileSize(group_items, calc_file_size);
			working = false;
		} else if (next_group > basic->GetFatEndGroup()) {
			// グループ番号がおかしい
			rc = false;
		} else {
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), 0, group_items);
			calc_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			calc_groups++;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}

	group_items.AddNums(calc_groups);
//	m_file_size += calc_file_size;
	group_items.AddSize(calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
	if (rc) {
		// ファイル内部のアドレスを得る
		TakeAddressesInFile();
	}
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有しているファイルサイズ
/// @return 計算後のファイルサイズ
int	DiskBasicDirItemFAT8::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
	if (group_items.Count() == 0) return occupied_size;

	DiskBasicGroupItem *litem = &group_items.Last();
	DiskImageSector *sector = basic->GetSector(litem->track, litem->side, litem->sector_end);
	if (!sector) return occupied_size;

	int sector_size = sector->GetSectorSize();
	int remain_size = ((occupied_size + sector_size - 1) % sector_size) + 1;
	remain_size = type->CalcDataSizeOnLastSector(this, NULL, NULL, sector->GetSectorBuffer(), sector_size, remain_size);

	occupied_size = occupied_size - sector_size + remain_size;
	return occupied_size;
}

/// ファイル内部のアドレスを取り出す
void DiskBasicDirItemFAT8::TakeAddressesInFile()
{
	if (m_groups.Count() == 0 || GetFileType1() != TYPE_NAME_1_MACHINE) {
		m_start_address = -1;
		m_end_address = -1;
		m_exec_address = -1;
		return;
	}

	DiskBasicGroupItem *item = &m_groups.Item(0);
	DiskImageSector *sector = basic->GetSector(item->track, item->side, item->sector_start);
	if (!sector) return;

	bool is_bigendian = basic->IsBigEndian();

	// 開始アドレス
	m_start_address = (int)sector->Get16(3, is_bigendian);
	// 終了アドレス
	m_end_address = (int)sector->Get16(1, is_bigendian) + m_start_address - 1;

	item = &m_groups.Last();
	sector = basic->GetSector(item->track, item->side, item->sector_end);
	if (!sector) return;
	// 実行アドレス
	int remain_size = m_groups.GetSize() % sector->GetSectorSize();
	if (remain_size >= 2) {
		m_exec_address = (int)sector->Get16(remain_size - 2, is_bigendian);
	} else {
		DiskImageSector *psector = basic->GetSector(item->track, item->side, item->sector_end - 1);
		if (psector) {
			if (remain_size >= 1) {
				m_exec_address = sector->Get(0) | (int)psector->Get(psector->GetSectorSize() - 1) << 8;
				if (is_bigendian) m_exec_address = wxUINT32_SWAP_ON_BE(m_exec_address);
			} else {
				m_exec_address = (int)psector->Get16(psector->GetSectorSize() - 2, is_bigendian);
			}
		}
	}
}

/// 開始アドレスを返す
int DiskBasicDirItemFAT8::GetStartAddress() const
{
	return m_start_address;
}

/// 終了アドレスを返す
int DiskBasicDirItemFAT8::GetEndAddress() const
{
	return m_end_address;
}

/// 実行アドレスを返す
int DiskBasicDirItemFAT8::GetExecuteAddress() const
{
	return m_exec_address;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemFAT8::ConvFileTypeFromFileName(const wxString &filename) const
{
	int ftype = 0;
	// 拡張子で属性を設定する
	wxFileName fn(filename);
	const MyAttribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
	if (sa) {
		ftype = sa->GetType();
	}
	return ftype;
}

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFAT8::GetFileType1Pos() const
{
	int t1 = GetFileType1();
	if (t1 < TYPE_NAME_1_BASIC || t1 > TYPE_NAME_1_MACHINE) {
		t1 = TYPE_NAME_1_UNKNOWN;
	}
	return t1;
}

// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemFAT8::GetFileType2Pos() const
{
	int t2 = GetFileType2();
	int t3 = GetFileType3();
	t2 = (t2 & 1 ? (t3 & 1 ? TYPE_NAME_2_RANDOM : TYPE_NAME_2_ASCII) : TYPE_NAME_2_BINARY);
	return t2;
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemFAT8::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		wxFileName fn(name);
		const MyAttribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
		if (!sa) return;

		int ftype = sa->GetType();
		if ((ftype & (FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK)) == (FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK)) {
			file_type_1 = TYPE_NAME_1_BASIC;
			file_type_2 = TYPE_NAME_2_BINARY;
		} else if ((ftype & (FILE_TYPE_BASIC_MASK | FILE_TYPE_ASCII_MASK)) == (FILE_TYPE_BASIC_MASK | FILE_TYPE_ASCII_MASK)) {
			file_type_1 = TYPE_NAME_1_BASIC;
			file_type_2 = TYPE_NAME_2_ASCII;
		} else if ((ftype & FILE_TYPE_MACHINE_MASK) == FILE_TYPE_MACHINE_MASK) {
			file_type_1 = TYPE_NAME_1_MACHINE;
			file_type_2 = TYPE_NAME_2_BINARY;
		} else if ((ftype & FILE_TYPE_RANDOM_MASK) == FILE_TYPE_RANDOM_MASK) {
			file_type_1 = TYPE_NAME_1_DATA;
			file_type_2 = TYPE_NAME_2_RANDOM;
		} else {
			file_type_1 = TYPE_NAME_1_DATA;
			file_type_2 = TYPE_NAME_2_ASCII;
		}
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemFAT8::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();
	wxRadioBox *radType1;
	wxRadioBox *radType2;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString types1;
	for(size_t i=0; i<=TYPE_NAME_1_MACHINE; i++) {
		types1.Add(wxGetTranslation(gTypeName1[i]));
	}
	radType1 = new wxRadioBox(parent, ATTR_DIALOG_IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 0, wxRA_SPECIFY_COLS);
	radType1->SetSelection(file_type_1);
	sizer->Add(radType1, flags);

	wxArrayString types2;
	for(size_t i=0; i<=TYPE_NAME_2_RANDOM; i++) {
		types2.Add(wxGetTranslation(gTypeName2[i]));
	}

	radType2 = new wxRadioBox(parent, ATTR_DIALOG_IDC_RADIO_TYPE2, _("Data Type"), wxDefaultPosition, wxDefaultSize, types2, 0, wxRA_SPECIFY_ROWS);
	radType2->SetSelection(file_type_2);
	sizer->Add(radType2, flags);

	// event handler
	parent->Bind(wxEVT_RADIOBOX, &IntNameBox::OnChangeType1, parent, ATTR_DIALOG_IDC_RADIO_TYPE1);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemFAT8::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxTextCtrl *txtIntName = (wxTextCtrl *)parent->FindWindow(IntNameBox::IDC_TEXT_INTNAME);
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(ATTR_DIALOG_IDC_RADIO_TYPE1);
	wxRadioBox *radType2 = (wxRadioBox *)parent->FindWindow(ATTR_DIALOG_IDC_RADIO_TYPE2);

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
		txtIntName->SetValue(AddExtension(selected_idx, txtIntName->GetValue()));
	}
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemFAT8::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(ATTR_DIALOG_IDC_RADIO_TYPE1);
	wxRadioBox *radType2 = (wxRadioBox *)parent->FindWindow(ATTR_DIALOG_IDC_RADIO_TYPE2);

	int pos1 = radType1->GetSelection();
	int pos2 = radType2->GetSelection();
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
	attr.SetFileAttr(FORMAT_TYPE_UNKNOWN, val);

	return true;
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemFAT8F::DiskBasicDirItemFAT8F(DiskBasic *basic)
	: DiskBasicDirItemFAT8(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemFAT8F::DiskBasicDirItemFAT8F(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemFAT8(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
}
DiskBasicDirItemFAT8F::DiskBasicDirItemFAT8F(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemFAT8(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);

	Used(CheckUsed(n_unuse));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemFAT8F::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemFAT8::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemFAT8F::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;
	directory_fat8f_t *p = m_data.Data();
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

/// 使用しているアイテムか
bool DiskBasicDirItemFAT8F::CheckUsed(bool unuse)
{
	return (m_data.Data()->name[0] != 0 && m_data.Data()->name[0] != 0xff);
}

/// 削除
/// @return true:OK
bool DiskBasicDirItemFAT8F::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data.Fill(basic->InvertUint8(basic->GetDeleteCode()), 1);
	Used(false);
	return true;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemFAT8F::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	// 8chars
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// 属性１を返す
int	DiskBasicDirItemFAT8F::GetFileType1() const
{
	return m_data.Data()->type;
}

/// 属性２を返す
int	DiskBasicDirItemFAT8F::GetFileType2() const
{
	return m_data.Data()->type2;
}

/// 属性３を返す
int DiskBasicDirItemFAT8F::GetFileType3() const
{
	return m_data.Data()->type3;
}

/// 属性１を設定
void DiskBasicDirItemFAT8F::SetFileType1(int val)
{
	m_data.Data()->type = val & 0xff;
}

/// 属性２を設定
void DiskBasicDirItemFAT8F::SetFileType2(int val)
{
	m_data.Data()->type2 = val & 0xff;
}

/// 属性３を設定
void DiskBasicDirItemFAT8F::SetFileType3(int val)
{
	m_data.Data()->type3 = val & 0xff;
}

/// 最初のグループ番号を設定
void DiskBasicDirItemFAT8F::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	m_data.Data()->start_group = (val & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemFAT8F::GetStartGroup(int fileunit_num) const
{
	return m_data.Data()->start_group;
}

/// ファイルのサイズ
void DiskBasicDirItemFAT8F::SetFileSize(int val)
{
//	// ファイルサイズはセクタサイズ境界で丸める
//	int sector_size = basic->GetSectorSize();
//	m_file_size = val;
	m_groups.SetSize(val);
}

/// ディレクトリのサイズ
size_t DiskBasicDirItemFAT8F::GetDataSize() const
{
	return sizeof(directory_fat8f_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemFAT8F::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemFAT8F::CopyData(const directory_t *val)
{
	return m_data.Copy(val);
}

/// ディレクトリをクリア
void DiskBasicDirItemFAT8F::ClearData()
{
	m_data.Fill(0);
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemFAT8F::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimExtensionByExtensionAttr(filename);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
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

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemFAT8F::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("(EXT)"), m_data.Data()->ext, sizeof(m_data.Data()->ext));
	vals.Add(wxT("TYPE"), m_data.Data()->type);
	vals.Add(wxT("TYPE2"), m_data.Data()->type2);
	vals.Add(wxT("TYPE3"), m_data.Data()->type3);
	vals.Add(wxT("START_GROUP"), m_data.Data()->start_group);
	vals.Add(wxT("RESERVED"), m_data.Data()->reserved, sizeof(m_data.Data()->reserved));
}
