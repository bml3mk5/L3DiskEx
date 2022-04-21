/// @file basicdiritem_falcom.cpp
///
/// @brief disk basic directory item for Falcom DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_falcom.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include <wx/msgdlg.h>


//
//
//
DiskBasicDirItemFalcom::DiskBasicDirItemFalcom(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
}
DiskBasicDirItemFalcom::DiskBasicDirItemFalcom(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, secpos, data)
{
}
DiskBasicDirItemFalcom::DiskBasicDirItemFalcom(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	Used(CheckUsed(unuse));
	unuse = (unuse || (m_data->falcom.name[0] == 0xff));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// ディレクトリアイテムのチェック
bool DiskBasicDirItemFalcom::Check(bool &last)
{
	return (m_data->falcom.name[0] == 0
		|| m_data->falcom.name[0] == 0xff
		|| (m_data->falcom.name[0] >= 0x20 && m_data->falcom.name[0] < 0x7f));
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemFalcom::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data->falcom.name);
		return m_data->falcom.name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemFalcom::GetFileExtPos(size_t &len) const
{
	len = 0;
	return NULL;
}

/// 属性１を設定
void DiskBasicDirItemFalcom::SetFileType1(int val)
{
}

/// 使用しているアイテムか
bool DiskBasicDirItemFalcom::CheckUsed(bool unuse)
{
	return (!unuse && this->m_data->falcom.name[0] != 0xff);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemFalcom::GetFileAttr() const
{
	int val = FILE_TYPE_BINARY_MASK;

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, 0);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemFalcom::GetFileAttrStr() const
{
	return wxT("");
}

/// ファイルサイズを返す
int DiskBasicDirItemFalcom::GetFileSize() const
{
	int val = GetEndAddress() - GetStartAddress() + 1;
	if (val < 0) val = 0;
	return val;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemFalcom::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemFalcom::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	int calc_file_size = 0;
	int calc_groups = 0; 

	bool rc = true;
	wxUint32 start_group = GetStartGroup(fileunit_num);
	wxUint32 last_group = GetLastGroup();
	for(wxUint32 grp = start_group; grp <= last_group; grp++) {
		wxUint32 next_grp = grp < last_group ? grp + 1 : 0xffff;
		basic->GetNumsFromGroup(grp, next_grp, basic->GetSectorSize(), 0, group_items);
		calc_file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
		calc_groups++;
	}

	group_items.AddNums(calc_groups);
	int file_size = GetFileSize();
	group_items.AddSize(calc_file_size <= file_size && file_size > 0 ? file_size : calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemFalcom::GetDataSize() const
{
	return sizeof(directory_mdos_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemFalcom::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	int sub_type = basic->GetFormatSubTypeNumber();
	switch(sub_type) {
	case 1:
		m_data->falcom.start_group.track = (val % 256);
		m_data->falcom.start_group.sector = (val / 256);
		break;
	default:
		m_data->falcom.start_group.track = (val / basic->GetSectorsPerTrackOnBasic());
		m_data->falcom.start_group.sector = (val %  basic->GetSectorsPerTrackOnBasic()) + 1;
		break;
	}
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemFalcom::GetStartGroup(int fileunit_num) const
{
	int sub_type = basic->GetFormatSubTypeNumber();
	int val = 0;
	switch(sub_type) {
	case 1:
		val = m_data->falcom.start_group.sector * 256 + m_data->falcom.start_group.track;
		break;
	default:
		val = m_data->falcom.start_group.track;
		if (val >= basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDisk()) {
			val = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDisk();
		}
		val *= basic->GetSectorsPerTrackOnBasic();
		val += (m_data->falcom.start_group.sector - 1);
		break;
	}
	if (val < 0) {
		val = 0;
	}
	return (wxUint32)val;
}

/// 最後のグループ番号を設定
void DiskBasicDirItemFalcom::SetLastGroup(wxUint32 val)
{
	int sub_type = basic->GetFormatSubTypeNumber();
	switch(sub_type) {
	case 1:
		m_data->falcom.end_group.track = (val % 256);
		m_data->falcom.end_group.sector = (val / 256);
		break;
	default:
		m_data->falcom.end_group.track = (val / basic->GetSectorsPerTrackOnBasic());
		m_data->falcom.end_group.sector = (val %  basic->GetSectorsPerTrackOnBasic()) + 1;
		break;
	}
}

/// 最後のグループ番号を返す
wxUint32 DiskBasicDirItemFalcom::GetLastGroup() const
{
	int sub_type = basic->GetFormatSubTypeNumber();
	int val = 0;
	switch(sub_type) {
	case 1:
		val = m_data->falcom.end_group.sector * 256 + m_data->falcom.end_group.track;
		break;
	default:
		val = m_data->falcom.end_group.track;
		if (val >= basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDisk()) {
			val = 0;
		}
		val *= basic->GetSectorsPerTrackOnBasic();
		val += (m_data->falcom.end_group.sector - 1);
		break;
	}
	if (val < 0) {
		val = 0;
	}
	return (wxUint32)val;
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemFalcom::NeedCheckEofCode()
{
	return false;
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemFalcom::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	return file_size;
}

/// 開始アドレスを返す
int DiskBasicDirItemFalcom::GetStartAddress() const
{
	return wxUINT16_SWAP_ON_BE(m_data->falcom.start_addr);
}

/// 終了アドレスを返す
int DiskBasicDirItemFalcom::GetEndAddress() const
{
	return wxUINT16_SWAP_ON_BE(m_data->falcom.end_addr);
}

/// 実行アドレスを返す
int DiskBasicDirItemFalcom::GetExecuteAddress() const
{
	return wxUINT16_SWAP_ON_BE(m_data->falcom.exec_addr);
}

/// ディレクトリをクリア
void DiskBasicDirItemFalcom::ClearData()
{
	if (!m_data) return;
	int c = basic->GetFillCodeOnDir();
	size_t l;
	l = GetDataSize();
	memset(m_data, c, l);
}

/// ファイル名から属性を決定する
int DiskBasicDirItemFalcom::ConvFileTypeFromFileName(const wxString &filename) const
{
	return FILE_TYPE_BINARY_MASK;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemFalcom::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	return 0;
}
