/// @file basicdiritem_sdos.cpp
///
/// @brief disk basic directory item for S-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_sdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../config.h"
#include "../charcodes.h"
#include <wx/msgdlg.h>


//////////////////////////////////////////////////////////////////////
//
//
//

/// S-DOS 属性名
const name_value_t gTypeNameSDOS_1[] = {
	{ wxTRANSLATE("BASIC (N)"), FILETYPE_SDOS_BAS1 },
	{ wxTRANSLATE("BASIC (n88)"), FILETYPE_SDOS_BAS2 },
	{ wxTRANSLATE("Binary"), FILETYPE_SDOS_DAT },
	{ wxTRANSLATE("Machine"),FILETYPE_SDOS_OBJ },
	{ "???", FILETYPE_SDOS_UNKNOWN },
	{ NULL, -1 }
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemSDOS::DiskBasicDirItemSDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
	AllocateItem(NULL);
}
DiskBasicDirItemSDOS::DiskBasicDirItemSDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	AllocateItem(NULL);
}
DiskBasicDirItemSDOS::DiskBasicDirItemSDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);
	AllocateItem(n_next);

	Used(CheckUsed(n_unuse));
//	unuse = (unuse || (m_data.Data()->name[0] == 0xff));

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
void DiskBasicDirItemSDOS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
	AllocateItem(n_next);
}

/// ディレクトリエントリを確保
/// data  は内部で確保したメモリ
/// sdata がセクタ内部へのポインタとなる
bool DiskBasicDirItemSDOS::AllocateItem(const SectorParam *next)
{
	m_sdata.Clear();
	bool bound = m_sdata.Set(basic, m_sector, m_position, (directory_t *)m_data.Data(), GetDataSize(), next);
	
	if (!m_data.IsSelf() && bound) {
		// セクタをまたぐ場合、dataは内部で確保する
		m_data.Alloc();
		m_data.Fill(0, sizeof(directory_sdos_t));
	}

	// コピー
	if (m_data.IsSelf()) {
		m_sdata.CopyTo((directory_t *)m_data.Data());
	}

	return true;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemSDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 属性１を返す
int	DiskBasicDirItemSDOS::GetFileType1() const
{
	return m_data.Data()->type;
}

/// 属性１を設定
void DiskBasicDirItemSDOS::SetFileType1(int val)
{
	m_data.Data()->type = val & 0xff;
}

/// 使用しているアイテムか
bool DiskBasicDirItemSDOS::CheckUsed(bool unuse)
{
	return (!unuse && this->m_data.Data()->name[0] != 0 && this->m_data.Data()->name[0] != 0xff);
}

/// ファイル名を設定
void DiskBasicDirItemSDOS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	DiskBasicDirItem::SetNativeName(filename, size, length);
}

/// ファイル名と拡張子を得る
void DiskBasicDirItemSDOS::GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	DiskBasicDirItem::GetNativeFileName(name, nlen, ext, elen);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemSDOS::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;
	if (m_data.Data()->name[0] == 0xff) {
		last = true;
		return valid;
	}
	return valid;
}

/// 削除
bool DiskBasicDirItemSDOS::Delete()
{
	m_data.Data()->name[0] = basic->GetDeleteCode();
	Used(false);
	return true;
}

/// 属性を設定
/// @param [in] file_type
void DiskBasicDirItemSDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		t1 = file_type.GetOrigin();
	} else {
		if (ftype & FILE_TYPE_BASIC_MASK) {
			if (basic->GetFormatSubTypeNumber() != 0) {
				t1 = FILETYPE_SDOS_BAS2;
			} else {
				t1 = FILETYPE_SDOS_BAS1;
			}
		} else if (ftype & FILE_TYPE_MACHINE_MASK) {
			t1 = FILETYPE_SDOS_OBJ;
		} else {
			t1 = FILETYPE_SDOS_DAT;
		}
	}
	SetFileType1(t1);

	SetUnknownData();
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemSDOS::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	switch (t1) {
	case FILETYPE_SDOS_DAT:
		val = FILE_TYPE_DATA_MASK;			// data
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	case FILETYPE_SDOS_OBJ:
		val = FILE_TYPE_MACHINE_MASK;		// machine
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	case FILETYPE_SDOS_BAS1:
	case FILETYPE_SDOS_BAS2:
		val = FILE_TYPE_BASIC_MASK;			// basic
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	default:
		val = FILE_TYPE_DATA_MASK;			// data
		val |= FILE_TYPE_BINARY_MASK;		// binary
		break;
	}
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemSDOS::GetFileAttrStr() const
{
	wxString attr = wxGetTranslation(gTypeNameSDOS_1[GetFileType1Pos()].name);
	return attr;
}

/// ファイルサイズをセット
void DiskBasicDirItemSDOS::SetFileSize(int val)
{
	wxUint16 size = val + basic->GetSectorSize() - 1;

	m_data.Data()->size = (size / basic->GetSectorSize());
	m_data.Data()->rest_size = ((val - 1) % basic->GetSectorSize());
}

/// ファイルサイズを返す
int DiskBasicDirItemSDOS::GetFileSize() const
{
	int size;
	size = m_data.Data()->size;

	if (size > 0) size--;
	size *= basic->GetSectorSize();

	size += m_data.Data()->rest_size;

	size++;
	return size;
}

/// グループ数を返す
wxUint32 DiskBasicDirItemSDOS::GetGroupSize() const
{
	wxUint32 size;
	size = m_data.Data()->size;
	return size;
}

/// ファイルサイズを計算
void DiskBasicDirItemSDOS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemSDOS::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	int calc_groups = 0;
	int calc_file_size = GetFileSize();

	wxUint32 group_num = GetStartGroup(fileunit_num);
	int gsize = (int)GetGroupSize();
	int limit = basic->GetFatEndGroup() + 1;
	while(gsize > 0 && limit >= 0) {
		AddGroups(group_num, 0, group_items);
		group_num++;
		calc_groups++;
		gsize--;
		limit--;
	}

	group_items.SetNums(calc_groups);
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize());
}

/// グループを追加する
void DiskBasicDirItemSDOS::AddGroups(wxUint32 group_num, wxUint32 next_group, DiskBasicGroups &group_items)
{
	int trk, sid, sec, div, divs;
	trk = sid = sec = -1;
	basic->CalcNumFromSectorPosForGroup(group_num, trk, sid, sec, &div, &divs);
	group_items.Add(group_num, next_group, trk, sid, sec, sec, div, divs);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemSDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	wxUint8 track = (val /  basic->GetSectorsPerTrackOnBasic()) & 0xff;
	wxUint8 sector = ((val %  basic->GetSectorsPerTrackOnBasic()) + 1) & 0xff;
	m_data.Data()->track = track;
	m_data.Data()->sector = sector;
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemSDOS::GetStartGroup(int fileunit_num) const
{
	wxUint32 track = 0;
	wxUint32 sector = 0;
	track = m_data.Data()->track;
	sector = m_data.Data()->sector;

	return track * basic->GetSectorsPerTrackOnBasic() + sector - 1;
}

// 開始アドレスを返す
int DiskBasicDirItemSDOS::GetStartAddress() const
{
	int addr;
	addr = m_data.Data()->load_addr;

	return basic->OrderUint16(addr);
}

// 実行アドレスを返す
int DiskBasicDirItemSDOS::GetExecuteAddress() const
{
	int addr;
	addr = m_data.Data()->exec_addr;

	return basic->OrderUint16(addr);
}

/// 開始アドレスをセット
void DiskBasicDirItemSDOS::SetStartAddress(int val)
{
	wxUint16 addr = basic->OrderUint16(val);
	m_data.Data()->load_addr = addr;
}

/// 実行アドレスをセット
void DiskBasicDirItemSDOS::SetExecuteAddress(int val)
{
	wxUint16 addr = basic->OrderUint16(val);
	m_data.Data()->exec_addr = addr;
}

/// 次のアイテムにENDマークを入れる
void DiskBasicDirItemSDOS::SetEndMark(DiskBasicDirItem *next_item)
{
	if (!next_item) return;

	next_item->Delete();
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemSDOS::NeedCheckEofCode()
{
	return false;
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemSDOS::GetDataSize() const
{
	return sizeof(directory_sdos_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemSDOS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemSDOS::CopyData(const directory_t *val)
{
	m_data.Copy(val, GetDataSize());
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
	return true;
}

/// ディレクトリをクリア
void DiskBasicDirItemSDOS::ClearData()
{
	m_data.Fill(basic->GetFillCodeOnDir(), GetDataSize());
}

/// 未使用領域の設定
void DiskBasicDirItemSDOS::SetUnknownData()
{
	wxUint8 val = 0xff;
	m_data.Data()->reserved = val;
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemSDOS::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimExtensionByExtensionAttr(filename);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemSDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = TYPE_NAME_SDOS_DAT;
	// 拡張子で属性を設定する
	wxFileName fn(filename);
	const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
	if (sa) {
		if ((sa->GetType() & (FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK)) == (FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK)) {
			if (basic->GetFormatSubTypeNumber() != 0) {
				t1 = TYPE_NAME_SDOS_BAS2;
			} else {
				t1 = TYPE_NAME_SDOS_BAS1;
			}
		} else if ((sa->GetType() & (FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK)) == (FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK)) {
			t1 = TYPE_NAME_SDOS_OBJ;
		}
	}
	return t1;
}

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemSDOS::SetModify()
{
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
}

//
// ダイアログ用
//

#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_COMBO_TYPE1 51

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemSDOS::GetFileType1Pos() const
{
	int t1 = GetFileType1();
	int pos =gTypeNameSDOS_1[0].IndexOf(gTypeNameSDOS_1, t1);
	if (pos < 0) {
		pos = TYPE_NAME_SDOS_UNKNOWN;
	}
	return pos;
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemSDOS::GetFileType2Pos() const
{
	return GetFileAttr().GetType();
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemSDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = ConvOriginalTypeFromFileName(name);
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemSDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();
	wxChoice *comType1;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString types1;
	for(size_t i=0; gTypeNameSDOS_1[i].name != NULL; i++) {
		types1.Add(wxGetTranslation(gTypeNameSDOS_1[i].name));
	}
	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);

	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	if (file_type_1 >= 0) {
		comType1->SetSelection(file_type_1);
	}
	staType1->Add(comType1, flags);
	sizer->Add(staType1, flags);

	parent->Bind(wxEVT_CHOICE, &IntNameBox::OnChangeType1, parent, IDC_COMBO_TYPE1);
}

/// controls 0:txtIntName 1...: dependent
void DiskBasicDirItemSDOS::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);

	int sel = comType1->GetSelection();
	bool editable = (sel >= TYPE_NAME_SDOS_DAT && sel <= TYPE_NAME_SDOS_OBJ);

	parent->SetEditableStartAddress(editable);
	parent->SetEditableExecuteAddress(editable);
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemSDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);

	int t1 = comType1->GetSelection();
	if (t1 < 0) t1 = FILETYPE_SDOS_BAS1;

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t1);

	return true;
}

/// 属性値を加工する
bool DiskBasicDirItemSDOS::ProcessAttr(DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int t1 = attr.GetFileOriginAttr();

	// BASICの固定アドレス設定
	switch(t1) {
	case FILETYPE_SDOS_BAS1:
	case FILETYPE_SDOS_BAS2:
		// BASICの場合、ロードアドレス、実行アドレスを固定で設定
		attr.SetStartAddress(basic->GetVariousIntegerParam(wxT("DefaultStartAddress")));
		attr.SetExecuteAddress(basic->GetVariousIntegerParam(wxT("DefaultExecuteAddress")));
		break;
	}
	return true;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemSDOS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("TYPE"), m_data.Data()->type);
	vals.Add(wxT("TRACK"), m_data.Data()->track);
	vals.Add(wxT("SECTOR"), m_data.Data()->sector);
	vals.Add(wxT("SIZE"), m_data.Data()->size);
	vals.Add(wxT("REST SIZE"), m_data.Data()->rest_size);
	vals.Add(wxT("LOAD ADDR"), m_data.Data()->load_addr, basic->IsBigEndian());
	vals.Add(wxT("EXEC ADDR"), m_data.Data()->exec_addr, basic->IsBigEndian());
	vals.Add(wxT("RESERVED"), m_data.Data()->reserved);
}
