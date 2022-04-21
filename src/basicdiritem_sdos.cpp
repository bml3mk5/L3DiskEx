/// @file basicdiritem_sdos.cpp
///
/// @brief disk basic directory item for S-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_sdos.h"
#include "basicfmt.h"
#include "basictype.h"
#include "config.h"
#include "charcodes.h"
#include <wx/msgdlg.h>


//
//
//

/// S-DOS 属性名
const char *gTypeNameSDOS_1[] = {
	wxTRANSLATE("BASIC (N)"),
	wxTRANSLATE("BASIC (n88)"),
	wxTRANSLATE("Binary"),
	wxTRANSLATE("Machine"),
	"???",
	NULL
};

const wxUint8 gTypeNameSDOSMap[] = {
	FILETYPE_SDOS_BAS1,
	FILETYPE_SDOS_BAS2,
	FILETYPE_SDOS_DAT,
	FILETYPE_SDOS_OBJ,
	FILETYPE_SDOS_UNKNOWN,
};

//
//
//
DiskBasicDirItemSDOS::DiskBasicDirItemSDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	AllocateItem();
}
DiskBasicDirItemSDOS::DiskBasicDirItemSDOS(DiskBasic *basic, DiskD88Sector *sector, int secpos, wxUint8 *data)
	: DiskBasicDirItem(basic, sector, secpos, data)
{
	AllocateItem();
}
DiskBasicDirItemSDOS::DiskBasicDirItemSDOS(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItem(basic, num, track, side, sector, secpos, data, unuse)
{
	AllocateItem();

	Used(CheckUsed(unuse));
//	unuse = (unuse || (m_data->sdos.name[0] == 0xff));

	// ファイルサイズとグループ数を計算
	CalcFileSize();
}

/// アイテムへのポインタを設定
void DiskBasicDirItemSDOS::SetDataPtr(int n_num, int n_track, int n_side, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_track, n_side, n_sector, n_secpos, n_data);

	AllocateItem();
}

/// ディレクトリエントリを確保
/// data  は内部で確保したメモリ
/// sdata がセクタ内部へのポインタとなる
bool DiskBasicDirItemSDOS::AllocateItem()
{
	for(int i=0; i<2; i++) {
		sdata[i].data = NULL;
		sdata[i].size = 0;
		sdata[i].pos  = 0xffff;
	}

	if (m_sector) {
		int spos = m_position;
		int ssize = m_sector->GetSectorSize();
		wxUint8 *sptr = (wxUint8 *)m_data;
		if (m_data) {
			sdata[0].data = (directory_sdos_t *)sptr;
			sdata[0].size = (int)sizeof(directory_sdos_t);
			sdata[0].pos  = 0;
		}
		if (spos + 32 >= ssize) {
			// セクタまたぎ
			sdata[1].pos = ssize - spos;
			sdata[1].size = sdata[0].size - sdata[1].pos;
			sdata[0].size = sdata[1].pos;

			// 次のセクタ
			int sec_pos = type->GetSectorPosFromNum(m_sector->GetIDC(), m_sector->GetIDH(), m_sector->GetIDR());
			sec_pos++;
			DiskD88Sector *nsector = basic->GetSectorFromSectorPos(sec_pos);
			if (nsector) {
				sptr = nsector->GetSectorBuffer();
				ssize = nsector->GetSectorSize();
				spos = 0;
				sdata[1].data = (directory_sdos_t *)(sptr - sdata[1].pos);
			}
		}
	}

	// dataは常に内部で確保する
	if (!m_ownmake_data) {
		m_data = new directory_t;
		memset(m_data, 0, sizeof(directory_t));
		m_ownmake_data = true;
	}

	// コピー
	for(int i=0; i<2; i++) {
		if (sdata[i].data) {
			memcpy(&m_data->sdos.p[sdata[i].pos], &sdata[i].data->p[sdata[i].pos], sdata[i].size);
		}
	}

	return true;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemSDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data->sdos.s.name);
		return m_data->sdos.s.name;
	} else {
		size = len = 0;
		return NULL;
	}
}

/// 属性１を返す
int	DiskBasicDirItemSDOS::GetFileType1() const
{
	if (DIR_SDOS_POS_TYPE >= sdata[1].pos) {
		return sdata[1].data->s.type;
	} else if (DIR_SDOS_POS_TYPE >= sdata[0].pos) {
		return sdata[0].data->s.type;
	} else {
		return m_data->sdos.s.type;
	}
}

/// 属性１を設定
void DiskBasicDirItemSDOS::SetFileType1(int val)
{
	m_data->sdos.s.type = val & 0xff;
	if (DIR_SDOS_POS_TYPE >= sdata[1].pos) {
		sdata[1].data->s.type = val & 0xff;
	} else if (DIR_SDOS_POS_TYPE >= sdata[0].pos) {
		sdata[0].data->s.type = val & 0xff;
	}
}

/// 使用しているアイテムか
bool DiskBasicDirItemSDOS::CheckUsed(bool unuse)
{
	return (!unuse && this->m_data->sdos.s.name[0] != 0 && this->m_data->sdos.s.name[0] != 0xff);
}

/// ファイル名を設定
void DiskBasicDirItemSDOS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	DiskBasicDirItem::SetNativeName(filename, size, length);
	if (DIR_SDOS_POS_NAME >= sdata[0].pos) {
		int len = (int)sizeof(m_data->sdos.s.name) > sdata[0].size ? sdata[0].size : (int)sizeof(m_data->sdos.s.name);
		memcpy(sdata[0].data->s.name, m_data->sdos.s.name, len);
	}
	if (sdata[1].pos < (DIR_SDOS_POS_NAME + (int)sizeof(m_data->sdos.s.name))) {
		int pos = sdata[1].pos;
		int len = (int)sizeof(m_data->sdos.s.name) - pos;
		memcpy(&sdata[1].data->s.name[pos], &m_data->sdos.s.name[pos], len);
	}
}

/// ファイル名と拡張子を得る
void DiskBasicDirItemSDOS::GetNativeFileName(wxUint8 *name, size_t &nlen, wxUint8 *ext, size_t &elen) const
{
	if (DIR_SDOS_POS_NAME >= sdata[0].pos) {
		int len = (int)sizeof(m_data->sdos.s.name) > sdata[0].size ? sdata[0].size : (int)sizeof(m_data->sdos.s.name);
		memcpy(m_data->sdos.s.name, sdata[0].data->s.name, len);
	}
	if (sdata[1].pos < (DIR_SDOS_POS_NAME + (int)sizeof(m_data->sdos.s.name))) {
		int pos = sdata[1].pos;
		int len = (int)sizeof(m_data->sdos.s.name) - pos;
		memcpy(&m_data->sdos.s.name[pos], &sdata[1].data->s.name[pos], len);
	}
	DiskBasicDirItem::GetNativeFileName(name, nlen, ext, elen);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemSDOS::Check(bool &last)
{
	if (!m_data) return false;

	bool valid = true;
	if (m_data->sdos.s.name[0] == 0xff) {
		last = true;
		return valid;
	}
	return valid;
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
	wxString attr = wxGetTranslation(gTypeNameSDOS_1[GetFileType1Pos()]);
	return attr;
}

/// ファイルサイズをセット
void DiskBasicDirItemSDOS::SetFileSize(int val)
{
	wxUint16 size = val + basic->GetSectorSize() - 1;

	m_data->sdos.s.size = (size / basic->GetSectorSize());
	m_data->sdos.s.rest_size = ((val - 1) % basic->GetSectorSize());
	if (DIR_SDOS_POS_SIZE >= sdata[1].pos) {
		sdata[1].data->s.size = m_data->sdos.s.size;
	} else if (DIR_SDOS_POS_SIZE >= sdata[0].pos) {
		sdata[0].data->s.size = m_data->sdos.s.size;
	}
	if (DIR_SDOS_POS_REST_SIZE >= sdata[1].pos) {
		sdata[1].data->s.rest_size = m_data->sdos.s.rest_size;
	} else if (DIR_SDOS_POS_REST_SIZE >= sdata[0].pos) {
		sdata[0].data->s.rest_size = m_data->sdos.s.rest_size;
	}
}

/// ファイルサイズを返す
int DiskBasicDirItemSDOS::GetFileSize() const
{
	int size;
	if (DIR_SDOS_POS_SIZE >= sdata[1].pos) {
		size = sdata[1].data->s.size;
	} else if (DIR_SDOS_POS_SIZE >= sdata[0].pos) {
		size = sdata[0].data->s.size;
	} else {
		size = m_data->sdos.s.size;
	}
	if (size > 0) size--;
	size *= basic->GetSectorSize();

	if (DIR_SDOS_POS_REST_SIZE >= sdata[1].pos) {
		size += sdata[1].data->s.rest_size;
	} else if (DIR_SDOS_POS_REST_SIZE >= sdata[0].pos) {
		size += sdata[0].data->s.rest_size;
	} else {
		size += m_data->sdos.s.rest_size;
	}
	size++;
	return size;
}

/// グループ数を返す
wxUint32 DiskBasicDirItemSDOS::GetGroupSize() const
{
	wxUint32 size;
	if (DIR_SDOS_POS_SIZE >= sdata[1].pos) {
		size = sdata[1].data->s.size;
	} else if (DIR_SDOS_POS_SIZE >= sdata[0].pos) {
		size = sdata[0].data->s.size;
	} else {
		size = m_data->sdos.s.size;
	}
	return size;
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemSDOS::GetDataSize() const
{
	return sizeof(directory_sdos_t);
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
	m_data->sdos.s.track = track;
	m_data->sdos.s.sector = sector;
	if (DIR_SDOS_POS_TRACK >= sdata[1].pos) {
		sdata[1].data->s.track = track;
	} else if (DIR_SDOS_POS_TRACK >= sdata[0].pos) {
		sdata[0].data->s.track = track;
	}
	if (DIR_SDOS_POS_SECTOR >= sdata[1].pos) {
		sdata[1].data->s.sector = sector;
	} else if (DIR_SDOS_POS_SECTOR >= sdata[0].pos) {
		sdata[0].data->s.sector = sector;
	}
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemSDOS::GetStartGroup(int fileunit_num) const
{
	wxUint32 track = 0;
	wxUint32 sector = 0;
	if (DIR_SDOS_POS_TRACK >= sdata[1].pos) {
		track = sdata[1].data->s.track;
	} else if (DIR_SDOS_POS_TRACK >= sdata[0].pos) {
		track = sdata[0].data->s.track;
	} else {
		track = m_data->sdos.s.track;
	}
	if (DIR_SDOS_POS_SECTOR >= sdata[1].pos) {
		sector = sdata[1].data->s.sector;
	} else if (DIR_SDOS_POS_SECTOR >= sdata[0].pos) {
		sector = sdata[0].data->s.sector;
	} else {
		sector = m_data->sdos.s.sector;
	}
	return track * basic->GetSectorsPerTrackOnBasic() + sector - 1;
}

// 開始アドレスを返す
int DiskBasicDirItemSDOS::GetStartAddress() const
{
	int addr;
	if (DIR_SDOS_POS_LOAD_ADDR >= sdata[1].pos) {
		addr = sdata[1].data->s.load_addr;
	} else if (DIR_SDOS_POS_LOAD_ADDR >= sdata[0].pos) {
		addr = sdata[0].data->s.load_addr;
	} else {
		addr = m_data->sdos.s.load_addr;
	}
	return basic->OrderUint16(addr);
}

// 実行アドレスを返す
int DiskBasicDirItemSDOS::GetExecuteAddress() const
{
	int addr;
	if (DIR_SDOS_POS_EXEC_ADDR >= sdata[1].pos) {
		addr = sdata[1].data->s.exec_addr;
	} else if (DIR_SDOS_POS_EXEC_ADDR >= sdata[0].pos) {
		addr = sdata[0].data->s.exec_addr;
	} else {
		addr = m_data->sdos.s.exec_addr;
	}
	return basic->OrderUint16(addr);
}

/// 開始アドレスをセット
void DiskBasicDirItemSDOS::SetStartAddress(int val)
{
	wxUint16 addr = basic->OrderUint16(val);
	m_data->sdos.s.load_addr = addr;
	if (DIR_SDOS_POS_LOAD_ADDR >= sdata[1].pos) {
		sdata[1].data->s.load_addr = addr;
	} else if (DIR_SDOS_POS_LOAD_ADDR >= sdata[0].pos) {
		sdata[0].data->s.load_addr = addr;
	}
}

/// 実行アドレスをセット
void DiskBasicDirItemSDOS::SetExecuteAddress(int val)
{
	wxUint16 addr = basic->OrderUint16(val);
	m_data->sdos.s.exec_addr = addr;
	if (DIR_SDOS_POS_EXEC_ADDR >= sdata[1].pos) {
		sdata[1].data->s.exec_addr = addr;
	} else if (DIR_SDOS_POS_EXEC_ADDR >= sdata[0].pos) {
		sdata[0].data->s.exec_addr = addr;
	}
}

/// 削除
bool DiskBasicDirItemSDOS::Delete(wxUint8 code)
{
	m_data->sdos.s.name[0] = code;
	if (DIR_SDOS_POS_NAME >= sdata[1].pos) {
		sdata[1].data->s.name[0] = code;
	} else if (DIR_SDOS_POS_NAME >= sdata[0].pos) {
		sdata[0].data->s.name[0] = code;
	}
	Used(false);
	return true;
}

#if 0
/// ENDマークがあるか(一度も使用していないか)
bool DiskBasicDirItemSDOS::HasEndMark()
{
	bool val = false;
//	val = ((wxUint32)m_data->sdos.s.name[0] == basic->GetGroupUnusedCode());
	return val;
}
#endif

/// 次のアイテムにENDマークを入れる
void DiskBasicDirItemSDOS::SetEndMark(DiskBasicDirItem *next_item)
{
	if (!next_item) return;

	next_item->Delete(basic->GetDeleteCode());
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemSDOS::NeedCheckEofCode()
{
	return false;
}

/// アイテムをコピー
bool DiskBasicDirItemSDOS::CopyData(const directory_t *val)
{
	if (!m_data) return false;
	memcpy(m_data, val, GetDataSize());
	for(int i=0; i<2; i++) {
		if (sdata[i].size > 0) {
			int pos = sdata[i].pos;
			memcpy(&sdata[i].data->p[pos], &val->sdos.p[pos], sdata[i].size);
		}
	}
	return true;
}

/// ディレクトリをクリア
void DiskBasicDirItemSDOS::ClearData()
{
	if (!m_data) return;
	int c = basic->GetFillCodeOnDir();
	size_t l;
	l = GetDataSize();
	memset(m_data, c, l);
}

/// 未使用領域の設定
void DiskBasicDirItemSDOS::SetUnknownData()
{
	wxUint8 val = 0xff;
	m_data->sdos.s.reserved = val;
	if (DIR_SDOS_POS_RESERVED >= sdata[1].pos) {
		sdata[1].data->s.reserved = val;
	} else if (DIR_SDOS_POS_RESERVED >= sdata[0].pos) {
		sdata[0].data->s.reserved = val;
	}
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

//
// ダイアログ用
//

#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_COMBO_TYPE1 51

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemSDOS::GetFileType1Pos() const
{
	int t1 = GetFileType1();
	int pos = TYPE_NAME_SDOS_UNKNOWN;
	for(int i = 0; ; i++) {
		int val = gTypeNameSDOSMap[i];
		if (val == FILETYPE_SDOS_UNKNOWN) {
			break;
		}
		if (t1 == val) {
			pos = i;
			break;
		}
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
	for(size_t i=0; gTypeNameSDOS_1[i] != NULL; i++) {
		types1.Add(wxGetTranslation(gTypeNameSDOS_1[i]));
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

#if 0
/// リストの位置から属性を返す(プロパティダイアログ用)
int	DiskBasicDirItemSDOS::CalcFileTypeFromPos(int pos)
{
	return 0;
}
#endif

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
