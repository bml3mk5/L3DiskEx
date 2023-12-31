﻿/// @file basicdiritem_losa.cpp
///
/// @brief disk basic directory item for L-os Angeles
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_losa.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include "../utils.h"


#define FILE_TYPE_LOSA_BINARY	0xa0
#define TYPE_NAME_LOSA_BINARY	wxTRANSLATE("LA binary")

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemLOSA::DiskBasicDirItemLOSA(DiskBasic *basic)
	: DiskBasicDirItemMSDOS(basic)
{
}
DiskBasicDirItemLOSA::DiskBasicDirItemLOSA(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemMSDOS(basic, n_sector, n_secpos, n_data)
{
}
DiskBasicDirItemLOSA::DiskBasicDirItemLOSA(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemMSDOS(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemLOSA::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	switch(num) {
	case 0:
		size = len = sizeof(m_data.Data()->losa.name);
		return m_data.Data()->losa.name;
	default:
		size = len = 0;
		return NULL;
	}
}

/// 拡張子を格納する位置を返す
wxUint8 *DiskBasicDirItemLOSA::GetFileExtPos(size_t &len) const
{
	len = sizeof(m_data.Data()->losa.ext);
	return m_data.Data()->losa.ext;
}

/// 属性２を返す
int	DiskBasicDirItemLOSA::GetFileType2() const
{
	return m_data.Data()->losa.binary_type;
}

/// 属性２を設定
void DiskBasicDirItemLOSA::SetFileType2(int val)
{
	m_data.Data()->losa.binary_type = val & 0xff;
}

/// 属性を設定
void DiskBasicDirItemLOSA::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	// MS-DOS
	SetFileType1((ftype & 0xff00) >> 8);

//	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		int t2 = file_type.GetOrigin() >> 16;
		SetFileType2(t2);
//	}
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemLOSA::GetFileAttr() const
{
	int t1 = GetFileType1();
	int t2 = GetFileType2();
	return DiskBasicFileType(basic->GetFormatTypeNumber(), t1 << 8, t2 << 8 | t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemLOSA::GetFileAttrStr() const
{
	wxString attr;
	int ftype = GetFileAttr().GetType();
	int t2 = GetFileAttr().GetOrigin() >> 8;
	if (t2 == FILE_TYPE_LOSA_BINARY) {
		attr = wxGetTranslation(TYPE_NAME_LOSA_BINARY);
	}
	GetFileAttrStrSub(ftype, attr);
	if (attr.IsEmpty()) {
		attr = wxT("---");
	}
	return attr;
}

/// 開始アドレスを返す
int DiskBasicDirItemLOSA::GetStartAddress() const
{
	wxString addr = wxString((const char *)m_data.Data()->losa.start_addr, 4);
	long lval = 0;
	addr.ToLong(&lval, 16);
	return (int)lval;
}

/// 実行アドレスを返す
int DiskBasicDirItemLOSA::GetExecuteAddress() const
{
	wxString addr = wxString((const char *)m_data.Data()->losa.exec_addr, 4);
	long lval = 0;
	addr.ToLong(&lval, 16);
	return (int)lval;
}

/// 開始アドレスをセット
void DiskBasicDirItemLOSA::SetStartAddress(int val)
{
	char addr[6];
	sprintf(addr, "%04X", val);
	memcpy(m_data.Data()->losa.start_addr, addr, 4);
}

/// 実行アドレスをセット
void DiskBasicDirItemLOSA::SetExecuteAddress(int val)
{
	char addr[6];
	sprintf(addr, "%04X", val);
	memcpy(m_data.Data()->losa.exec_addr, addr, 4);
}

size_t DiskBasicDirItemLOSA::GetDataSize() const
{
	return sizeof(directory_losa_t);
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"


#define IDC_CHK_LABIN		61

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemLOSA::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileAttr().GetType();
	int file_type_2 = GetFileType2();

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	// 属性チェックボックスを作成
	wxStaticBoxSizer *box = CreateControlsSubForAttrDialog(parent, show_flags, sizer, flags, file_type_1);

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox *chkLAbin = new wxCheckBox(parent, IDC_CHK_LABIN, wxGetTranslation(TYPE_NAME_LOSA_BINARY));
	chkLAbin->SetValue(file_type_2 == FILE_TYPE_LOSA_BINARY);
	hbox->Add(chkLAbin, flags);
	box->Add(hbox);
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemLOSA::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	// 属性
	SetAttrSubInAttrDialog(parent, attr);

	wxCheckBox *chkLAbin = (wxCheckBox *)parent->FindWindow(IDC_CHK_LABIN);
//	SetFileType2(chkLAbin->IsChecked() ? FILE_TYPE_LOSA_BINARY : 0);
	attr.SetFileOriginAttr(chkLAbin->IsChecked() ? (FILE_TYPE_LOSA_BINARY << 16) : 0);
	return true;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemLOSA::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("NAME"), m_data.Data()->losa.name, sizeof(m_data.Data()->losa.name));
	vals.Add(wxT("EXT"), m_data.Data()->losa.ext, sizeof(m_data.Data()->losa.ext));
	vals.Add(wxT("TYPE"), m_data.Data()->losa.type);
	vals.Add(wxT("START_ADDR"), m_data.Data()->losa.start_addr, sizeof(m_data.Data()->losa.start_addr));
	vals.Add(wxT("BINARY_TYPE"), m_data.Data()->losa.binary_type);
	vals.Add(wxT("EXEC_ADDR"), m_data.Data()->losa.exec_addr, sizeof(m_data.Data()->losa.exec_addr));
	vals.Add(wxT("RESERVED"), m_data.Data()->losa.reserved);
	vals.Add(wxT("WTIME"), m_data.Data()->losa.wtime);
	vals.Add(wxT("WDATE"), m_data.Data()->losa.wdate);
	vals.Add(wxT("START_GROUP"), m_data.Data()->losa.start_group);
	vals.Add(wxT("FILE_SIZE"), m_data.Data()->losa.file_size);
}
