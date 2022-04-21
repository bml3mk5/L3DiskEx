/// @file basicdiritem_dos80.cpp
///
/// @brief disk basic directory item for PC-8001 DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_dos80.h"
#include "basicfmt.h"
#include "basictype.h"
#include "../config.h"
#include "../charcodes.h"
#include <wx/msgdlg.h>


//////////////////////////////////////////////////////////////////////

//#define DOS80_BASIC_LOAD_ADDR	0x8d20
//#define DOS80_BASIC_EXEC_ADDR	0x3df4

// PC-8001 DOS 属性名
const char *gTypeNameDOS80[] = {
	wxTRANSLATE("BASIC"),
	wxTRANSLATE("Machine"),
	wxTRANSLATE("BASIC + Machine"),
	NULL
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemDOS80::DiskBasicDirItemDOS80(DiskBasic *basic)
	: DiskBasicDirItemFAT8(basic)
{
	m_cached_type = 0;
	m_data.Alloc();
	m_data2.Alloc();
	m_data2.Fill(0);
}
DiskBasicDirItemDOS80::DiskBasicDirItemDOS80(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemFAT8(basic, n_sector, n_secpos, n_data)
{
	m_cached_type = 0;
	m_data.Attach(n_data);
	m_data2.Alloc();
	m_data2.Fill(0);
}
DiskBasicDirItemDOS80::DiskBasicDirItemDOS80(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemFAT8(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_cached_type = 0;
	m_data.Attach(n_data);

	// 2セクタ後に属性などがある
	DiskD88Sector *sector_2 = basic->GetSector(n_gitem->track, n_gitem->side, n_sector->GetSectorNumber() + 2);
	if (sector_2) {
		wxUint8 *buffer2 = sector_2->GetSectorBuffer();
		m_data2.Attach(&buffer2[n_secpos]);
	}

	Used(CheckUsed(n_unuse));
//	unuse = (unuse || (this->m_data.Data()->name[0] == 0));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

DiskBasicDirItemDOS80::~DiskBasicDirItemDOS80()
{
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemDOS80::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);

	m_data2.Delete();

	// 2セクタ後に属性などがある
	DiskD88Sector *sector_2 = basic->GetSector(n_gitem->track, n_gitem->side, m_sector->GetSectorNumber() + 2);
	if (sector_2) {
		wxUint8 *buffer2 = sector_2->GetSectorBuffer();
		m_data2.Attach(&buffer2[n_secpos]);
	}
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemDOS80::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = sizeof(m_data.Data()->name);
		len = size - 1;
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// 属性１を返す
int	DiskBasicDirItemDOS80::GetFileType1() const
{
	int val = 0;
	if (m_data2.IsValid()) {
		// 属性は開始アドレスで判断する
		if (basic->OrderUint16(m_data2.Data()->grps[0].a) == basic->GetVariousIntegerParam(wxT("DefaultStartAddress"))) {
			if (m_data2.Data()->grps[1].g == 0x01) {
				val = TYPE_NAME_DOS80_BASIC;
			} else {
				val = TYPE_NAME_DOS80_BASIC_MACHINE;
			}
		} else {
			val = TYPE_NAME_DOS80_MACHINE;
		}
	}
	return val;
}

/// 属性１を設定
void DiskBasicDirItemDOS80::SetFileType1(int val)
{
	if (!m_data2.IsValid()) return;
	if ((val & 0xff00) == 0) return;

	// BASICの固定アドレス設定
	//
	// マシン語のアドレスはSetStartAddress(),SetEndAddress(),SetExecuteAddress()で
	// 設定する
	//
	switch(val & 0xff) {
	case TYPE_NAME_DOS80_BASIC:
		// BASICの場合、ロードアドレス、終了アドレス、実行アドレスを固定で設定
		m_data2.Data()->grps[0].a = basic->OrderUint16(basic->GetVariousIntegerParam(wxT("DefaultStartAddress")));
		m_data2.Data()->grps[1].g = 1;
		m_data2.Data()->grps[1].a = basic->OrderUint16(0);
		m_data2.Data()->grps[2].g = 0;
		m_data2.Data()->grps[2].a = basic->OrderUint16(basic->GetVariousIntegerParam(wxT("DefaultExecuteAddress")));
		break;
	case TYPE_NAME_DOS80_BASIC_MACHINE:
		// BASIC + マシン語の場合、ロードアドレス、実行アドレスを固定で設定
		m_data2.Data()->grps[0].a = basic->OrderUint16(basic->GetVariousIntegerParam(wxT("DefaultStartAddress")));
		m_data2.Data()->grps[3].g = 0;
		m_data2.Data()->grps[3].a = basic->OrderUint16(basic->GetVariousIntegerParam(wxT("DefaultExecuteAddress")));
		break;
	}
}

/// 使用しているアイテムか
bool DiskBasicDirItemDOS80::CheckUsed(bool unuse)
{
	return (!unuse && this->m_data.Data()->name[0] != 0 && this->m_data.Data()->name[0] != 0xff && this->GetStartGroup(0) != 0);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemDOS80::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;
	if (m_data.Data()->name[0] == 0) {
//		last = true;
		return valid;
	}
	return valid;
}

/// 削除
/// @return true:OK
bool DiskBasicDirItemDOS80::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data.Fill(basic->InvertUint8(basic->GetDeleteCode()), 1);
	Used(false);
	return true;
}

/// 属性を設定
/// @param [in] file_type
void DiskBasicDirItemDOS80::SetFileAttr(const DiskBasicFileType &file_type)
{
	int val = file_type.GetType();
	int t1 = ConvFileAttrToTypePos(val);

	m_cached_type = (1 << 8 | t1);

	SetFileType1(m_cached_type);
}

/// 属性を変換
int DiskBasicDirItemDOS80::ConvFileAttrToTypePos(int file_type) const
{
	int t1 = TYPE_NAME_DOS80_BASIC;
	switch(file_type & (FILE_TYPE_BASIC_MASK | FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK)) {
	case (FILE_TYPE_BASIC_MASK | FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK):
		t1 = TYPE_NAME_DOS80_BASIC_MACHINE;
		break;
	case (FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK):
		// Machine
		t1 = TYPE_NAME_DOS80_MACHINE;
		break;
	}
	return t1;
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemDOS80::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = ConvFileAttrFromTypePos(t1);
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1);
}

/// 属性を変換
int DiskBasicDirItemDOS80::ConvFileAttrFromTypePos(int t1) const
{
	int val = 0;
	switch(t1) {
	case TYPE_NAME_DOS80_MACHINE:
		// Machine
		val = FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK;
		break;
	case TYPE_NAME_DOS80_BASIC_MACHINE:
		// BASIC + Machine
		val = FILE_TYPE_BASIC_MASK | FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK;
		break;
	default:
		// BASIC
		val = FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK;
		break;
	}
	return val;
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemDOS80::GetFileAttrStr() const
{
	int file_type_1 = GetFileType1();
	return wxGetTranslation(gTypeNameDOS80[file_type_1]);
}

/// ファイルサイズの計算
void DiskBasicDirItemDOS80::CalcFileSize()
{
	m_groups.Empty();
	for(int fileunit_num = 0; fileunit_num < 4; fileunit_num++) {
		if (!IsValidFileUnit(fileunit_num)) {
			break;
		}
		if (!IsUsed()) {
			break;
		}
		DiskBasicGroups groups;
		GetUnitGroups(fileunit_num, groups);
		m_groups.Add(groups);
		if (fileunit_num < 2) m_file_unit[fileunit_num] = groups;
	}
}

/// ファイルサイズをセット
void DiskBasicDirItemDOS80::SetFileSize(int val)
{
	// ファイルサイズはセクタサイズ境界で丸める
	int sector_size = basic->GetSectorSize();
	m_groups.SetSize((((val - 1) / sector_size) + 1) * sector_size);
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasicDirItemDOS80::GetAllGroups(DiskBasicGroups &group_items)
{
	group_items.Empty();
	for(int fileunit_num = 0; fileunit_num < 4; fileunit_num++) {
		if (!IsValidFileUnit(fileunit_num)) {
			break;
		}
		GetUnitGroups(fileunit_num, group_items);
	}
}

/// 最初のグループ番号を設定
void DiskBasicDirItemDOS80::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	if (m_data2.IsValid()) m_data2.Data()->grps[fileunit_num].g = (val & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemDOS80::GetStartGroup(int fileunit_num) const
{
	return (m_data2.IsValid() ? m_data2.Data()->grps[fileunit_num].g : 0);
}

/// アイテムがアドレスを持っているか
bool DiskBasicDirItemDOS80::HasAddress() const
{
	return (m_data2.IsValid());
}

/// アドレスを編集できるか
bool DiskBasicDirItemDOS80::IsAddressEditable() const
{
	int t1 = GetFileType1();
	return (t1 != TYPE_NAME_DOS80_BASIC);
}

/// 開始アドレスを返す
int DiskBasicDirItemDOS80::GetStartAddress() const
{
	int t1 = GetFileType1();
	int val = 0;
	if (m_data2.IsValid()) {
		switch(t1) {
		case TYPE_NAME_DOS80_BASIC_MACHINE:
			val = basic->OrderUint16(m_data2.Data()->grps[1].a);
			break;
		default:
			val = basic->OrderUint16(m_data2.Data()->grps[0].a);
			break;
		}
	}
	return val;
}

/// 終了アドレスを返す
int DiskBasicDirItemDOS80::GetEndAddress() const
{
	int t1 = GetFileType1();
	int val = 0;
	if (m_data2.IsValid()) {
		switch(t1) {
		case TYPE_NAME_DOS80_BASIC_MACHINE:
			val = basic->OrderUint16(m_data2.Data()->grps[2].a);
			break;
		default:
			val = basic->OrderUint16(m_data2.Data()->grps[1].a);
			break;
		}
	}
	return val;
}

/// 実行アドレスを返す
int DiskBasicDirItemDOS80::GetExecuteAddress() const
{
	int t1 = GetFileType1();
	int val = 0;
	if (m_data2.IsValid()) {
		switch(t1) {
		case TYPE_NAME_DOS80_BASIC_MACHINE:
			val = basic->OrderUint16(m_data2.Data()->grps[3].a);
			break;
		default:
			val = basic->OrderUint16(m_data2.Data()->grps[2].a);
			break;
		}
	}
	return val;
}

/// 開始アドレスをセット
void DiskBasicDirItemDOS80::SetStartAddress(int val)
{
	if (!m_data2.IsValid()) return;
	if ((m_cached_type & 0xff00) == 0) return;

	switch(m_cached_type & 0xff) {
	case TYPE_NAME_DOS80_MACHINE:
		m_data2.Data()->grps[0].a = basic->OrderUint16(val);
		break;
	case TYPE_NAME_DOS80_BASIC_MACHINE:
		m_data2.Data()->grps[1].a = basic->OrderUint16(val);
		break;
	}
}

/// 終了アドレスをセット
void DiskBasicDirItemDOS80::SetEndAddress(int val)
{
	if (!m_data2.IsValid()) return;
	if ((m_cached_type & 0xff00) == 0) return;

	switch(m_cached_type & 0xff) {
	case TYPE_NAME_DOS80_MACHINE:
		m_data2.Data()->grps[1].g = 0;
		m_data2.Data()->grps[1].a = basic->OrderUint16(val);
		break;
	case TYPE_NAME_DOS80_BASIC_MACHINE:
		m_data2.Data()->grps[2].g = 0;
		m_data2.Data()->grps[2].a = basic->OrderUint16(val);
		break;
	}
}

/// 実行アドレスをセット
void DiskBasicDirItemDOS80::SetExecuteAddress(int val)
{
	if (!m_data2.IsValid()) return;
	if ((m_cached_type & 0xff00) == 0) return;

	switch(m_cached_type & 0xff) {
	case TYPE_NAME_DOS80_MACHINE:
		m_data2.Data()->grps[2].g = 0;
		m_data2.Data()->grps[2].a = basic->OrderUint16(val);
		break;
	}
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemDOS80::NeedCheckEofCode()
{
	return false;
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemDOS80::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	return file_size;
}

/// ファイル番号のファイルサイズを得る
int DiskBasicDirItemDOS80::GetFileUnitSize(int fileunit_num, wxInputStream &istream, int file_offset)
{
	int file_type_1 = GetFileType1();
	int basic_size = (int)istream.GetLength();
	int machine_size = -1;
	if (file_type_1 == TYPE_NAME_DOS80_BASIC_MACHINE) {
		// BASIC + マシン語の場合
		machine_size = GetEndAddress() - GetStartAddress();
		if (machine_size >= 0) {
			machine_size++;
			machine_size = ((machine_size + 255) & ~0xff);
		}

		basic_size -= machine_size;
	}

	if (fileunit_num == 0) {
		return basic_size;
	}
	if (fileunit_num == 1) {
		return machine_size;
	}
	return -1;
}

/// ファイル番号のファイルへアクセスできるか
bool DiskBasicDirItemDOS80::IsValidFileUnit(int fileunit_num)
{
	if (fileunit_num == 0) {
		return true;
	}
	if (fileunit_num == 1) {
		int file_type_1 = GetFileType1();

		if (file_type_1 == TYPE_NAME_DOS80_BASIC_MACHINE) {
			// BASIC + マシン語の場合
			return true;
		}
	}
	return false;
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemDOS80::GetDataSize() const
{
	return sizeof(directory_dos80_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemDOS80::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemDOS80::CopyData(const directory_t *val)
{
	return m_data.Copy(val);
}

/// ディレクトリをクリア
void DiskBasicDirItemDOS80::ClearData()
{
	m_data.Fill(basic->GetFillCodeOnDir());
}

/// ファイル名、属性をコピー
void DiskBasicDirItemDOS80::CopyItem(const DiskBasicDirItem &src)
{
	DiskBasicDirItemFAT8::CopyItem(src);

	const DiskBasicDirItemDOS80 *psrc = (const DiskBasicDirItemDOS80 *)&src;

	if (m_data2.IsValid() && psrc->m_data2.IsValid()) {
		m_data2.Copy(psrc->m_data2.Data());
	}
}

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemDOS80::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimExtensionByExtensionAttr(filename);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

//
// ダイアログ用
//

#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/numformatter.h>
#include "../ui/intnamebox.h"


#define IDC_COMBO_TYPE1	51

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemDOS80::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1();
//	int file_type_2 = 0;

	if (show_flags & INTNAME_NEW_FILE) {
		// 外部から
		file_type_1 = ConvFileTypeFromFileName(file_path);
		file_type_1 = ConvFileAttrToTypePos(file_type_1);
	}

	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);

	wxArrayString types1;
	for(size_t i=0; gTypeNameDOS80[i] != NULL; i++) {
		types1.Add(wxGetTranslation(gTypeNameDOS80[i]));
	}

	wxChoice *comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	comType1->SetSelection(file_type_1);
	staType1->Add(comType1, flags);
	sizer->Add(staType1, flags);

	wxString str;

	if ((show_flags & INTNAME_SHOW_PROPERTY) && file_type_1 == TYPE_NAME_DOS80_BASIC_MACHINE) {
		wxGridSizer *szrG = new wxGridSizer(2, 2, 2);
		for(size_t i=0; i<2; i++) {
			str = (i == 0 ? _("BASIC") : _("Machine"));
			str += _(" Part");
			wxStaticText *staUnit1 = new wxStaticText(parent, wxID_ANY, str);
			szrG->Add(staUnit1, flags);

			str = wxNumberFormatter::ToString((long)m_file_unit[i].GetSize());
			str += wxString::Format(wxT(" (0x%x) "), m_file_unit[i].GetSize());
			str += _("bytes");
			wxStaticText *staUnit2 = new wxStaticText(parent, wxID_ANY, str, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			szrG->Add(staUnit2, flags);
		}
		sizer->Add(szrG, flags);
	}

	// 選択肢を変更した時のイベント
	parent->Bind(wxEVT_CHOICE, &IntNameBox::OnChangeType1, parent);
}

/// ダイアログ内の値を設定
void DiskBasicDirItemDOS80::InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data)
{
}

/// controls 0:txtIntName 1...: dependent
void DiskBasicDirItemDOS80::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	if (!comType1) return;
	int file_type_1 = comType1->GetSelection();
	if (file_type_1 < 0) return;

	parent->SetEditableStartAddress(file_type_1 != TYPE_NAME_DOS80_BASIC); 
	parent->SetEditableEndAddress(file_type_1 != TYPE_NAME_DOS80_BASIC); 
	parent->SetEditableExecuteAddress(file_type_1 == TYPE_NAME_DOS80_MACHINE); 
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemDOS80::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	if (!m_data2.IsValid()) return true;

	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	if (!comType1) return true;
	int file_type_1 = comType1->GetSelection();

	bool valid = true;

	int stval = parent->GetStartAddress();
	int edval = parent->GetEndAddress();

	if (file_type_1 == TYPE_NAME_DOS80_MACHINE
	 || file_type_1 == TYPE_NAME_DOS80_BASIC_MACHINE
	) {
		// マシン語の場合
		if (stval > edval) {
			errinfo.SetError(DiskBasicError::ERR_END_ADDR_TOO_SMALL);
			valid = false;
		}
	}

	if (valid) {
		attr.SetFileAttr(FORMAT_TYPE_UNKNOWN, ConvFileAttrFromTypePos(file_type_1), 2 << 8 | file_type_1);
	}

	return valid;
}

/// ダイアログの内容が反映された後の処理 
/// @param [in] parent         プロパティダイアログ
/// @param [in] status         データが反映されたか
void DiskBasicDirItemDOS80::ComittedAttrInAttrDialog(const IntNameBox *parent, bool status)
{
}

/// ダイアログの終了アドレスを編集できるか
bool DiskBasicDirItemDOS80::IsEndAddressEditableInAttrDialog(IntNameBox *parent)
{
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	if (!comType1) return true;
	int file_type_1 = comType1->GetSelection();

	return (file_type_1 != TYPE_NAME_DOS80_BASIC);
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemDOS80::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));

	if (!m_data2.IsValid()) return;

	vals.Add(wxT("GRPS"), m_data2.Data()->grps, sizeof(m_data2.Data()->grps));
	vals.Add(wxT("RESERVED"), m_data2.Data()->reserved, sizeof(m_data2.Data()->reserved));
}
