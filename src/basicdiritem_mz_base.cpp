/// @file basicdiritem_mz_base.cpp
///
/// @brief disk basic directory item for MZ Base
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_mz_base.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "config.h"
#include "utils.h"


///
///
///
DiskBasicDirItemMZBase::DiskBasicDirItemMZBase(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemMZBase::DiskBasicDirItemMZBase(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, secpos, data)
{
}
DiskBasicDirItemMZBase::DiskBasicDirItemMZBase(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
}

/// 削除
/// @param [in] code : 削除コード(デフォルト0)
bool DiskBasicDirItemMZBase::Delete(wxUint8 code)
{
	// エントリの先頭にコードを入れる
	SetFileType1(code);
	Used(false);
	// 開始グループを未使用にする
	type->SetGroupNumber(GetStartGroup(0), 0);
	return true;
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemMZBase::InitialData()
{
	ClearData();
}

/// ファイルサイズをセット
void DiskBasicDirItemMZBase::SetFileSize(int val)
{
	m_groups.SetSize(val);
	SetFileSizeBase(val);
}

/// ファイルサイズとグループ数を計算する前処理
void DiskBasicDirItemMZBase::PreCalcFileSize()
{
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemMZBase::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// グループ取得計算前処理
void DiskBasicDirItemMZBase::PreCalcAllGroups(int &calc_flags, wxUint32 &group_num, int &remain, int &sec_size, void **user_data)
{
}

/// グループ取得計算中処理
void DiskBasicDirItemMZBase::CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data)
{
	group_num++;
}
/// グループ取得計算後処理
void DiskBasicDirItemMZBase::PostCalcAllGroups(void *user_data)
{
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemMZBase::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	// ファイルサイズ
	int calc_file_size = GetFileSizeBase();
	PreCalcFileSize();

	wxUint32 group_num = GetStartGroup(fileunit_num);
	int remain = calc_file_size;
	int sec_size = basic->GetSectorSize();
	int calc_flags = 0;
	int calc_groups = 0;
	void *user_data = NULL;

	PreCalcAllGroups(calc_flags, group_num, remain, sec_size, &user_data);

	int limit = basic->GetFatEndGroup() + 1;
	while(remain > 0 && limit >= 0) {
		// 使用しているか
		bool used_group = type->IsUsedGroupNumber(group_num);
		if (used_group) {
			int end_sec = -1;
			basic->GetNumsFromGroup(group_num, 0, sec_size, remain, group_items, &end_sec);
			CalcAllGroups(calc_flags, group_num, remain, sec_size, end_sec, user_data);
			calc_groups++;
			remain -= (sec_size * basic->GetSectorsPerGroup());
		} else {
			limit = 0;
		}
		limit--;
	}

	group_items.SetNums(calc_groups);
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());

	PostCalcAllGroups(user_data);
}

bool DiskBasicDirItemMZBase::IsDeletable() const
{
	bool valid = true;
	DiskBasicFileType attr = GetFileAttr();
	if (attr.IsVolume()) {
		// ボリューム番号は削除できない
		valid = false;
	} else if (attr.IsDirectory()) {
		wxString name =	GetFileNamePlainStr();
		if (name == wxT(".") || name == wxT("..")) {
			// ディレクトリ ".", ".."は削除不可
			valid = false;
		}
	}
	return valid;
}
bool DiskBasicDirItemMZBase::IsFileNameEditable() const
{
	bool valid = true;
	DiskBasicFileType attr = GetFileAttr();
	if (attr.IsVolume()) {
		// ボリューム番号は編集できない
		valid = false;
	} else if (attr.IsDirectory()) {
		wxString name =	GetFileNamePlainStr();
		if (name == wxT(".") || name == wxT("..")) {
			// ディレクトリ ".", ".."は編集不可
			valid = false;
		}
	}
	return valid;
}

#include <wx/choice.h>

/// 属性の選択肢を作成する（プロパティダイアログ用）
void DiskBasicDirItemMZBase::CreateChoiceForAttrDialog(DiskBasic *basic, const char *list[], int end_pos, wxArrayString &types1)
{
	for(size_t i=0; i<(size_t)end_pos; i++) {
		types1.Add(wxGetTranslation(list[i]));
	}
	const L3Attributes *attrs = &basic->GetSpecialAttributes();
	for(size_t i=0; i<attrs->Count(); i++) {
		const L3Attribute *attr = &attrs->Item(i);
		wxString str;
		str += attr->GetName();
		str += wxString::Format(wxT(" 0x%02x"), attr->GetValue());
		if (!attr->GetDescription().IsEmpty()) {
			str += wxT(" (");
			str += attr->GetDescription();
			str += wxT(")");
		}
		types1.Add(str);
	}
}

/// 属性の選択肢を選ぶ（プロパティダイアログ用）
int  DiskBasicDirItemMZBase::SelectChoiceForAttrDialog(DiskBasic *basic, wxChoice *choice, int file_type_1, int end_pos, int unknown_pos)
{
	if (file_type_1 < 0) {
		const L3Attributes *attrs = &basic->GetSpecialAttributes();
		int n_type = attrs->GetIndexByValue(-file_type_1);
		if (n_type >= 0) {
			file_type_1 = n_type + end_pos;
		}
	}
	choice->SetSelection(file_type_1 >= 0 ? file_type_1 : unknown_pos);

	return file_type_1;
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int DiskBasicDirItemMZBase::CalcSpecialOriginalTypeFromPos(DiskBasic *basic, int pos, int end_pos)
{
	int val = -1;
	if (pos >= end_pos) {
		pos -= end_pos;
		const L3Attributes *attrs = &basic->GetSpecialAttributes();
		int count = (int)attrs->Count();
		if (pos < count) {
			val = attrs->GetValueByIndex(pos);
		}
	}
	return val;
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int DiskBasicDirItemMZBase::CalcSpecialFileTypeFromPos(DiskBasic *basic, int pos, int end_pos)
{
	int t = 0;
	if (pos >= end_pos) {
		pos -= end_pos;
		const L3Attributes *attrs = &basic->GetSpecialAttributes();
		int count = (int)attrs->Count();
		if (pos < count) {
			t = attrs->GetTypeByIndex(pos);
		}
	}
	return t;
}
