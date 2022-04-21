/// @file basicdiritem_mdos.cpp
///
/// @brief disk basic directory item for MDOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_mdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include <wx/msgdlg.h>


//
//
//
DiskBasicDirItemMDOS::DiskBasicDirItemMDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemMDOS::DiskBasicDirItemMDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, secpos, data)
{
}
DiskBasicDirItemMDOS::DiskBasicDirItemMDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	Used(CheckUsed(unuse));
	unuse = (unuse || (m_data->mdos.name[0] == 0));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemMDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data->mdos.name);
		return m_data->mdos.name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemMDOS::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data->mdos.ext);
	return m_data->mdos.ext;
}

/// 属性１を設定
void DiskBasicDirItemMDOS::SetFileType1(int val)
{
}

/// 使用しているアイテムか
bool DiskBasicDirItemMDOS::CheckUsed(bool unuse)
{
	return (!unuse && this->m_data->mdos.name[0] != 0);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemMDOS::GetFileAttr() const
{
	int val = FILE_TYPE_BINARY_MASK;

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, 0);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMDOS::GetFileAttrStr() const
{
	return wxT("");
}

/// ファイルサイズをセット
void DiskBasicDirItemMDOS::SetFileSize(int val)
{
	m_groups.SetSize(val);
	m_data->mdos.file_size = wxUINT16_SWAP_ON_LE((wxUint16)val);
}

/// ファイルサイズを返す
int DiskBasicDirItemMDOS::GetFileSize() const
{
	wxUint16 val = m_data->mdos.file_size;
	return (int)wxUINT16_SWAP_ON_LE(val);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemMDOS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemMDOS::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	int calc_file_size = 0;
	int calc_groups = 0; 

	// 16bit FAT
	bool rc = true;
	wxUint32 group_num = GetStartGroup(fileunit_num);
	bool working = true;
	int limit = basic->GetFatEndGroup() + 1;
	while(working) {
		wxUint32 next_group = type->GetGroupNumber(group_num);
		if (next_group == group_num) {
			// 同じポジションならエラー
			rc = false;
		} else if (next_group == basic->GetGroupFinalCode()) {
			// 最終グループ(0xffff)
			working = false;
		} else if (next_group > basic->GetFatEndGroup()) {
			// グループ番号がおかしい
			rc = false;
		} else if (next_group >= basic->GetGroupSystemCode()) {
			// システム領域はエラー(0xeeee)
			rc = false;
		}
		if (rc) {
			basic->GetNumsFromGroup(group_num, next_group, basic->GetSectorSize(), 0, group_items);
			calc_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			calc_groups++;
			group_num = next_group;
			limit--;
		}
		working = working && rc && (limit >= 0);
	}

	group_items.AddNums(calc_groups);
	group_items.AddSize(calc_file_size >= GetFileSize() ? GetFileSize() : calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());

	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemMDOS::GetDataSize() const
{
	return sizeof(directory_mdos_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemMDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	m_data->mdos.start_group = wxUINT16_SWAP_ON_BE(val & 0xffff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemMDOS::GetStartGroup(int fileunit_num) const
{
	return wxUINT16_SWAP_ON_BE(m_data->mdos.start_group);
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemMDOS::NeedCheckEofCode()
{
	return false;
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemMDOS::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	return file_size;
}

/// ディレクトリをクリア
void DiskBasicDirItemMDOS::ClearData()
{
	if (!m_data) return;
	int c = basic->GetFillCodeOnDir();
	size_t l;
	l = GetDataSize();
	memset(m_data, c, l);
}

/// ファイル名から属性を決定する
int DiskBasicDirItemMDOS::ConvFileTypeFromFileName(const wxString &filename) const
{
	int ftype = FILE_TYPE_BINARY_MASK;
	return ftype;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemMDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;
	return t1;
}
