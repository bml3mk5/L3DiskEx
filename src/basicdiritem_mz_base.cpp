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
#include "utils.h"


///
///
///
DiskBasicDirItemMZBase::DiskBasicDirItemMZBase(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemMZBase::DiskBasicDirItemMZBase(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, data)
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
	type->SetGroupNumber(GetStartGroup(), 0);
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
	file_size = val;
	SetFileSizeBase(val);
}

/// ファイルサイズとグループ数を計算する前処理
void DiskBasicDirItemMZBase::PreCalcFileSize()
{
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemMZBase::CalcFileSize()
{
	if (!IsUsed()) return;

	// ファイルサイズ
	file_size = GetFileSizeBase();
	PreCalcFileSize();

	int calc_groups = 0;

	bool rc = true;
	wxUint32 group_num = GetStartGroup();
	int remain = file_size;
	int sec_size = basic->GetSectorSize();
	int calc_flags = 0;
	void *user_data = NULL;

	PreCalcAllGroups(calc_flags, group_num, remain, sec_size, &user_data);

	int limit = basic->GetFatEndGroup() + 1;
	while(remain > 0 && limit >= 0) {
		// 使用しているか
		bool used_group = type->IsUsedGroupNumber(group_num);
		if (used_group) {
			int start_sec = type->GetStartSectorFromGroup(group_num);
			int end_sec = type->GetEndSectorFromGroup(group_num, 0, start_sec, sec_size, remain);
			CalcAllGroups(calc_flags, group_num, remain, sec_size, end_sec, user_data);
			calc_groups++;
			remain -= (sec_size * basic->GetSectorsPerGroup());
		} else {
			limit = 0;
			rc = false;
		}
		limit--;
	}
	if (limit < 0) {
		rc = false;
	}

	// グループ数を計算
	if (rc) {
		groups = calc_groups;
	}

	PostCalcAllGroups(user_data);
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
void DiskBasicDirItemMZBase::GetAllGroups(DiskBasicGroups &group_items)
{
	wxUint32 group_num = GetStartGroup();
	int remain = file_size;
	int sec_size = basic->GetSectorSize();
	int calc_flags = 0;
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
			remain -= (sec_size * basic->GetSectorsPerGroup());
		} else {
			limit = 0;
		}
		limit--;
	}

	group_items.SetSize(file_size);

	PostCalcAllGroups(user_data);
}


/// ファイルパスから内部ファイル名を生成する
wxString DiskBasicDirItemMZBase::RemakeFileNameStr(const wxString &filepath) const
{
	wxString newname;
	wxFileName fn(filepath);
	newname = fn.GetFullName().Left(GetFileNameSize());
	return newname;
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemMZBase::InvalidateChars() const
{
	return wxT("\"\\/:*?");
}

/// 同じファイル名か
bool DiskBasicDirItemMZBase::IsSameFileName(const DiskBasicFileName &filename) const
{
	// 属性が0以外
	if (GetFileType1() == 0) return false;

	return DiskBasicDirItem::IsSameFileName(filename);
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
