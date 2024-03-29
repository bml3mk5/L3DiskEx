/// @file basicdiritem_xdos.cpp
///
/// @brief disk basic directory item for X-DOS for X1
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_xdos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basictype.h"
#include "../charcodes.h"
#include "../config.h"
#include "../utils.h"
#include <wx/msgdlg.h>


//////////////////////////////////////////////////////////////////////

const name_value_t gTypeNameXDOS1[] = {
	{ "NUL", FILETYPE_XDOS_NUL }, // 0x00
	{ "BIN", FILETYPE_XDOS_BIN }, // 0x01
	{ "BAS", FILETYPE_XDOS_BAS }, // 0x02
	{ "CMD", FILETYPE_XDOS_CMD }, // 0x03
	{ "ASC", FILETYPE_XDOS_ASC }, // 0x04
	{ "SUB", FILETYPE_XDOS_SUB }, // 0x05
	{ "BAT", FILETYPE_XDOS_BAT }, // 0x06
	{ "SYS", FILETYPE_XDOS_SYS }, // 0x07
	{ "DIC", FILETYPE_XDOS_DIC }, // 0x08
	{ "DIR", FILETYPE_XDOS_DIR }, // 0x80
	{ NULL, -1 }
};
enum en_type_name_xdos_1 {
	TYPE_NAME_XDOS_NUL = 0,
	TYPE_NAME_XDOS_BIN,
	TYPE_NAME_XDOS_BAS,
	TYPE_NAME_XDOS_CMD,
	TYPE_NAME_XDOS_ASC,
	TYPE_NAME_XDOS_SUB,
	TYPE_NAME_XDOS_BAT,
	TYPE_NAME_XDOS_SYS,
	TYPE_NAME_XDOS_DIC,
	TYPE_NAME_XDOS_DIR,
};

//////////////////////////////////////////////////////////////////////

/// X-DOS サブ属性名マップ
typedef struct st_xdos_sub_type {
	int start;
	int end;
	const char *desc;
} xdos_sub_type_t;

const xdos_sub_type_t xdos_sub_types_3cmd[] = {
	{ 0x00, 0x00, "Default" },
	{ 0x10, 0x10, "SX-BASIC" },
	{ 0x11, 0x11, "XASM" },
	{ 0x12, 0x12, "XEDIT" },
	{ 0x13, 0x13, "SLANG" },
	{ -1, -1, NULL }
};
const xdos_sub_type_t xdos_sub_types_5sub[] = {
	{ 0x00, 0x00, "Default" },
	{ 0x01, 0x01, "Printer" },
	{ 0x10, 0x17, "overley module (turbo/MZ)" },
	{ 0x18, 0x1f, "overley module (nomal X1)" },
	{ 0x20, 0x2f, "access module" },
	{ -1, -1, NULL }
};
const xdos_sub_type_t xdos_sub_types_7sys[] = {
	{ 0x00, 0x00, "X-DOS System (turbo)" },
	{ 0x01, 0x01, "X-DOS System (nomal X1)" },
	{ 0x02, 0x02, "X-DOS System (MZ-2500)" },
	{ -1, -1, NULL }
};
const xdos_sub_type_t *xdos_sub_types[] = {
	NULL,	// NUL
	NULL,	// OBJ
	NULL,	// BAS
	xdos_sub_types_3cmd,	// CMD
	NULL,	// ASC
	xdos_sub_types_5sub,	// SUB
	NULL,	// BAT
	xdos_sub_types_7sys,	// SYS
	NULL,	// DIC
	NULL,	// DIR
};

const char *gTypeNameXDOS2[] = {
	wxTRANSLATE("Hidden"),				// 0x80
	wxTRANSLATE("Write Protected"),		// 0x40
	wxTRANSLATE("System"),				// 0x20 
	wxTRANSLATE("Kanji"),				// 0x10
	NULL
};
enum en_type_name_xdos_2 {
	TYPE_NAME_XDOS2_HIDDEN = 0,
	TYPE_NAME_XDOS2_READONLY,
	TYPE_NAME_XDOS2_SYSTEM,
	TYPE_NAME_XDOS2_KANJI,
};
enum en_file_type_mask_xdos2 {
	FILETYPE_XDOS2_HIDDEN = 0x80,
	FILETYPE_XDOS2_READONLY = 0x40,
	FILETYPE_XDOS2_SYSTEM = 0x20,
	FILETYPE_XDOS2_KANJI = 0x10,
};

enum en_file_type_mask_xdos {
	FILE_TYPE_KANJI_MASK = 0x1000000,
};

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemXDOSChain::DiskBasicDirItemXDOSChain()
{
	basic = NULL;
	sector = NULL;
	chain = NULL;
	chain_ownmake = false;
}
DiskBasicDirItemXDOSChain::~DiskBasicDirItemXDOSChain()
{
	if (chain_ownmake) {
		delete chain;
	}
}
#ifdef COPYABLE_DIRITEM
/// 代入
DiskBasicDirItemXDOSChain &DiskBasicDirItemXDOSChain::operator=(const DiskBasicDirItemXDOSChain &src)
{
	this->Dup(src);
	return *this;
}
/// 複製
void DiskBasicDirItemXDOSChain::Dup(const DiskBasicDirItemXDOSChain &src)
{
	sector = src.sector;
	if (src.chain_ownmake) {
		chain = new xdos_chain_t;
		memcpy(&chain, src.chain, sizeof(xdos_chain_t));
	} else {
		chain = src.chain;
	}
	chain_ownmake = src.chain_ownmake;
}
#endif
/// ポインタをセット
void DiskBasicDirItemXDOSChain::Set(DiskBasic *n_basic, DiskImageSector *n_sector, xdos_chain_t *n_chain)
{
	basic = n_basic;
	sector = n_sector;
	if (chain_ownmake) delete chain;
	chain = n_chain;
	chain_ownmake = false;
}
/// メモリ確保
void DiskBasicDirItemXDOSChain::Alloc()
{
	if (chain_ownmake) delete chain;
	chain = new xdos_chain_t;
	chain_ownmake = true;
	memset(chain, 0, sizeof(xdos_chain_t));
}
/// クリア
void DiskBasicDirItemXDOSChain::Clear()
{
	if (sector) sector->Fill(0);
	else if (chain) memset(chain, 0, sizeof(xdos_chain_t));
}
/// セグメントのセクタ番号を返す
wxUint32 DiskBasicDirItemXDOSChain::GetSectorPos(int idx) const
{
	if (!chain || idx >= XDOS_CHAIN_SEGMENTS) return 0;

	return chain->seg[idx].track * basic->GetSectorsPerTrackOnBasic() + chain->seg[idx].sector - 1;
}
/// セクタ数を返す
int DiskBasicDirItemXDOSChain::GetSectors() const
{
	if (!chain) return 0;

	int cnt = 0;
	for(int i=0; i<XDOS_CHAIN_SEGMENTS; i++) {
		if (chain->seg[i].track == 0) {
			break;
		}
		cnt += chain->seg[i].size;
	}
	return cnt;
}
/// セグメントのセクタ数を返す
wxUint8 DiskBasicDirItemXDOSChain::GetSectors(int idx) const
{
	if (!chain || idx >= XDOS_CHAIN_SEGMENTS) return 0;

	return chain->seg[idx].size;
}
/// セグメントにLSNを設定 すでにあればサイズを増やす
void DiskBasicDirItemXDOSChain::AddSectorPos(int idx, wxUint32 val)
{
	if (!chain || idx >= XDOS_CHAIN_SEGMENTS) return;

	if (chain->seg[idx].size == 0) {
		val++;
		chain->seg[idx].track = val / basic->GetSectorsPerTrackOnBasic();
		chain->seg[idx].sector = val % basic->GetSectorsPerTrackOnBasic();
	}
	chain->seg[idx].size++;
}
/// セグメントにセクタ数を設定
void DiskBasicDirItemXDOSChain::SetSectors(int idx, wxUint8 val)
{
	if (!chain || idx >= XDOS_CHAIN_SEGMENTS) return;

	chain->seg[idx].size = val;
}
/// セグメントを得る
bool DiskBasicDirItemXDOSChain::GetSegment(int idx, wxUint32 &group_num, int &size) const
{
	if (!chain) return false;

	group_num = chain->seg[idx].track * basic->GetSectorsPerTrackOnBasic() + chain->seg[idx].sector - 1;
	size = chain->seg[idx].size;

	return (chain->seg[idx].track != 0);
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemXDOSBase::DiskBasicDirItemXDOSBase(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	chain.SetBasic(basic);
	chain.Alloc();
}
DiskBasicDirItemXDOSBase::DiskBasicDirItemXDOSBase(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	chain.SetBasic(basic);
	chain.Alloc();
}
DiskBasicDirItemXDOSBase::DiskBasicDirItemXDOSBase(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse, bool n_inherit)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
}
DiskBasicDirItemXDOSBase::DiskBasicDirItemXDOSBase(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
}

#if 0
/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemXDOSBase::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);
}
#endif

/// チェイン情報を設定
void DiskBasicDirItemXDOSBase::AttachChain(wxUint32 group_num)
{
	if (group_num != 0) {
		DiskImageSector *sector = basic->GetSectorFromGroup(group_num);
		if (sector) {
			chain.Set(basic, sector, (xdos_chain_t *)sector->GetSectorBuffer());
		}
	}
}

/// ファイル名を編集できるか
bool DiskBasicDirItemXDOSBase::IsFileNameEditable() const
{
	// "!"は不可
	return IsDeletable();
}

/// アイテムをロード・エクスポートできるか
/// @return true ロードできる
bool DiskBasicDirItemXDOSBase::IsLoadable() const
{
	// "!"は不可
	return IsDeletable();
}

/// アイテムをコピーできるか
/// @return true コピーできる
bool DiskBasicDirItemXDOSBase::IsCopyable() const
{
	// "!"は不可
	return IsDeletable();
}

/// アイテムを上書きできるか
/// @return true 上書きできる
bool DiskBasicDirItemXDOSBase::IsOverWritable() const
{
	// ディレクトリは不可
	bool valid = !IsDirectory();
	// "!"は不可
	valid &= IsDeletable();
	return valid;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemXDOSBase::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemXDOSBase::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	int calc_file_size = 0;
	int calc_groups = 0;

	if (GetFileAttr().IsDirectory()) {
		// ディレクトリの場合
		wxUint32 group_num = GetStartGroup(fileunit_num);
		for(int idx=0; idx < basic->GetSubDirGroupSize(); idx++) {
			AddGroups(group_num, idx + 1 != basic->GetSubDirGroupSize() ? group_num + 1 : 0, group_items);
			group_num++;
			calc_groups++;
			calc_file_size += basic->GetSectorsPerGroup() * basic->GetSectorSize();
		}
	} else {
		// ファイルの場合
		// ファイルサイズ
		calc_file_size += GetFileSize();

		if (!chain.IsValid()) return;

		for(int idx=0; idx<XDOS_CHAIN_SEGMENTS; idx++) {
			wxUint32 group_num;
			int size;
			if (!chain.GetSegment(idx, group_num, size)) {
//				calc_file_size = RecalcFileSize(group_items, calc_file_size);
				break;
			}
			if (idx != 0) {
				if (group_items.Count() > 0) {
					DiskBasicGroupItem *gitem = &group_items.Last();
					gitem->next = group_num;
				}
			}
			for(int siz = 0; siz < size; siz++) {
				wxUint32 next_grp = (siz + 1 != size ? group_num + 1 : 0);
				AddGroups(group_num, next_grp, group_items);
				group_num++;
				calc_groups++;
			}
		}
	}
	group_items.SetNums(calc_groups);
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize() * basic->GetSectorsPerGroup());
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有しているファイルサイズ
/// @return 計算後のファイルサイズ
int	DiskBasicDirItemXDOSBase::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
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

/// グループを追加する
void DiskBasicDirItemXDOSBase::AddGroups(wxUint32 group_num, wxUint32 next_group, DiskBasicGroups &group_items)
{
	int trk, sid, sec, div, divs;
	trk = sid = sec = -1;
	basic->CalcNumFromSectorPosForGroup(group_num, trk, sid, sec, &div, &divs);
	group_items.Add(group_num, next_group, trk, sid, sec, sec, div, divs);
}

wxString DiskBasicDirItemXDOSBase::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

wxString DiskBasicDirItemXDOSBase::GetFileCreateTimeStr() const
{
	TM tm;
	GetFileCreateTime(tm);
	return Utils::FormatHMSStr(tm);
}

/// チェイン用のセクタをセット
/// @param [in] sector セクタ
/// @param [in] data   セクタ内のバッファ
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemXDOSBase::SetChainSector(DiskImageSector *sector, wxUint8 *data, const DiskBasicDirItem *pitem)
{
	chain.Set(basic, sector, (xdos_chain_t *)data);
}

/// チェイン用のセクタにグループ番号をセット(機種依存)
void DiskBasicDirItemXDOSBase::AddChainGroupNumber(int idx, wxUint32 val)
{
	chain.AddSectorPos(idx, val);
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemXDOSBase::NeedCheckEofCode()
{
	return (GetFileAttr().IsAscii()); 
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemXDOSBase::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	// ファイルの最終が終端記号で終わっているかを調べる
	// ただし、ファイルサイズがクラスタサイズと合うなら終端記号は不要
	if ((file_size % (basic->GetSectorSize() * basic->GetSectorsPerGroup())) != 0) {
		file_size = CheckEofCode(istream, file_size);
	}
	return file_size;
}

/// ファイルサイズが適正か
bool DiskBasicDirItemXDOSBase::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	int limit_size = 0xffff;
	if (limit) *limit = limit_size;

	return (size <= limit_size);
}

//////////////////////////////////////////////////////////////////////
//
//
//
DiskBasicDirItemXDOS::DiskBasicDirItemXDOS(DiskBasic *basic)
	: DiskBasicDirItemXDOSBase(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemXDOS::DiskBasicDirItemXDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItemXDOSBase(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
}
DiskBasicDirItemXDOS::DiskBasicDirItemXDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse, bool n_inherit)
	: DiskBasicDirItemXDOSBase(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
}
DiskBasicDirItemXDOS::DiskBasicDirItemXDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItemXDOSBase(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);

	Used(CheckUsed(n_unuse));

	// チェインセクタへのポインタをセット
	if (IsUsed()) {
		AttachChain(GetStartGroup(0));
	}

	// ファイルサイズとグループ数を計算
	CalcFileSize();

	// 親ディレクトリはツリーに表示しない
	wxString name = GetFileNamePlainStr();
	VisibleOnTree(!(IsDirectory() && name == wxT("!")));
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemXDOS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItemXDOSBase::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemXDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
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
int	DiskBasicDirItemXDOS::GetFileType1() const
{
	return wxUINT16_SWAP_ON_LE(m_data.Data()->ftype);
}

/// 属性１を設定
void DiskBasicDirItemXDOS::SetFileType1(int val)
{
	m_data.Data()->ftype = wxUINT16_SWAP_ON_LE(val);
}

/// 属性１の文字列
wxString DiskBasicDirItemXDOS::ConvFileType1Str(int t1) const
{
	wxString str;
	if (t1 <= 0x0800) {
		str = wxGetTranslation(gTypeNameXDOS1[t1 >> 8].name);
	} else if (t1 == 0x8000) {
		str = wxGetTranslation(gTypeNameXDOS1[TYPE_NAME_XDOS_DIR].name);
	} else if (t1 & 0x8000) {
		// ユーザファイルタイプ
		ConvUserFileTypeToStr(t1, str);
	}
	return str;
}

/// 属性２を返す
int	DiskBasicDirItemXDOS::GetFileType2() const
{
	return m_data.Data()->attr;
}

/// 属性２を設定
void DiskBasicDirItemXDOS::SetFileType2(int val)
{
	m_data.Data()->attr = (val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemXDOS::CheckUsed(bool unuse)
{
	int t1 = (GetFileType1() >> 8);
	return (!unuse && t1 != 0 && t1 != 0xff);
}

/// ユーザーファイルタイプ名を得る
void DiskBasicDirItemXDOS::ConvUserFileTypeToStr(int type1, wxString &str)
{
	wxUint8 ext[4];
	ext[0] = ((type1 >> 10) & 0x1f) + 0x40;
	ext[1] = ((type1 >> 5) & 0x1f) + 0x40;
	ext[2] = (type1 & 0x1f) + 0x40;
	ext[3] = 0;
	str = ext;
}

/// ユーザーファイルタイプに変換
int DiskBasicDirItemXDOS::ConvStrToUserFileType(const wxString &str)
{
	int type1 = 0x8000;
	char c;
	for(int i=0; i<(int)str.Length() && i<3; i++) {
		c = 0;
		if (str.GetChar(i).GetAsChar(&c) && c >= 'A' && c <= 'Z') {
			type1 |= (((c - 0x40) & 0x1f) << ((2-i) * 5));
		}
	}
	return type1;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemXDOS::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;
	if (m_data.Data()->ftype == 0xffff && m_data.Data()->start.track == 0xff) {
		last = true;
		return valid;
	}
	return valid;
}

/// 削除できるか
bool DiskBasicDirItemXDOS::IsDeletable() const
{
	// "!"は不可
	bool valid = true;
	wxString name =	GetFileNamePlainStr();
	if (name == wxT("!")) {
		valid = false;
	}
	return valid;
}

/// 削除
bool DiskBasicDirItemXDOS::Delete()
{
	// 削除はエントリの先頭にコードを入れるだけ
	m_data.Fill(basic->InvertUint8(basic->GetDeleteCode()), 1);
	Used(false);
	return true;
}

/// 属性を設定
/// @param [in] file_type
void DiskBasicDirItemXDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int t1 = 0;
	int t2 = 0;
	if (file_type.IsDirectory()) {
		// ディレクトリの場合
		t1 = FILETYPE_XDOS_DIR << 8;
	} else if (file_type.GetFormat() == basic->GetFormatTypeNumber()) {
		// 同じOSの場合
		t1 = file_type.GetOrigin();
		t2 = t1 >> 16;
		t1 &= 0xffff;
	} else {
		// 違うOSの場合
		if (ftype & FILE_TYPE_DIRECTORY_MASK) {
			t1 = FILETYPE_XDOS_DIR;
		} else if (ftype & FILE_TYPE_BASIC_MASK) {
			t1 = FILETYPE_XDOS_BAS;
		} else if (ftype & FILE_TYPE_MACHINE_MASK) {
			t1 = FILETYPE_XDOS_BIN;
		} else if (ftype & FILE_TYPE_SYSTEM_MASK) {
			t1 = FILETYPE_XDOS_SYS;
		} else if (ftype & FILE_TYPE_BINARY_MASK) {
			t1 = FILETYPE_XDOS_CMD;
		}
		t1 <<= 8;
	}
	SetFileType1(t1);
	SetFileType2(t2);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemXDOS::GetFileAttr() const
{
	int val = 0;
	int typ1 = GetFileType1();
	switch(typ1 >> 8) {
	case 1:	// OBJ
		val |= FILE_TYPE_MACHINE_MASK;
		break;
	case 2:	// BAS
		val |= FILE_TYPE_BASIC_MASK;
		break;
	case 3:	// CMD
		val |= FILE_TYPE_BINARY_MASK;
		break;
	case 4:	// ASC
		val |= FILE_TYPE_ASCII_MASK;
		break;
	case 5:	// SUB
		val |= FILE_TYPE_BASIC_MASK;
		break;
	case 6:	// BAT
		val |= FILE_TYPE_BASIC_MASK;
		break;
	case 7:	// SYS
		val |= FILE_TYPE_SYSTEM_MASK;
		break;
	case 8:	// DIC
		val |= FILE_TYPE_DATA_MASK;
		break;
	case 0x80:	// DIR
		val |= FILE_TYPE_DIRECTORY_MASK;
		break;
	default:
		val |= FILE_TYPE_DATA_MASK;
		break;
	}

	int typ2 = GetFileType2();
	if (typ2 & FILETYPE_XDOS2_HIDDEN) {
		val |= FILE_TYPE_HIDDEN_MASK;
	}
	if (typ2 & FILETYPE_XDOS2_READONLY) {
		val |= FILE_TYPE_READONLY_MASK;
	}
	if (typ2 & FILETYPE_XDOS2_SYSTEM) {
		val |= FILE_TYPE_SYSTEM_MASK;
	}
	if (typ2 & FILETYPE_XDOS2_KANJI) {
		val |= FILE_TYPE_KANJI_MASK;
	}

	// 独自属性にはファイル種類そのまま入れる
	int extended = (typ1 | (typ2 << 16));

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, extended);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemXDOS::GetFileAttrStr() const
{
	wxString str = ConvFileType1Str(GetFileType1());

	int typ2 = GetFileType2();
	if (typ2 & FILETYPE_XDOS2_HIDDEN) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameXDOS2[TYPE_NAME_XDOS2_HIDDEN]);
	}
	if (typ2 & FILETYPE_XDOS2_READONLY) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameXDOS2[TYPE_NAME_XDOS2_READONLY]);
	}
	if (typ2 & FILETYPE_XDOS2_SYSTEM) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameXDOS2[TYPE_NAME_XDOS2_SYSTEM]);
	}
	if (typ2 & FILETYPE_XDOS2_KANJI) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameXDOS2[TYPE_NAME_XDOS2_KANJI]);
	}
	return str;
}

/// ファイルサイズをセット
void DiskBasicDirItemXDOS::SetFileSize(int val)
{
	m_data.Data()->file_size = wxUINT16_SWAP_ON_BE(val);
	m_groups.SetSize(val);
}

/// ファイルサイズを返す
int DiskBasicDirItemXDOS::GetFileSize() const
{
	return wxUINT16_SWAP_ON_BE(m_data.Data()->file_size);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemXDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	m_data.Data()->start.track = val / basic->GetSectorsPerTrackOnBasic();
	m_data.Data()->start.sector = (val % basic->GetSectorsPerTrackOnBasic()) + 1;
	m_data.Data()->start.size = (wxUint8)size;
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemXDOS::GetStartGroup(int fileunit_num) const
{
	return m_data.Data()->start.track * basic->GetSectorsPerTrackOnBasic() + m_data.Data()->start.sector - 1;
}

/// 追加のグループ番号をセット チェインセクタへセット
void DiskBasicDirItemXDOS::SetExtraGroup(wxUint32 val)
{
	m_data.Data()->start.track = val / basic->GetSectorsPerTrackOnBasic();
	m_data.Data()->start.sector = (val % basic->GetSectorsPerTrackOnBasic()) + 1;
}

/// 追加のグループ番号を返す チェインセクタを返す
wxUint32 DiskBasicDirItemXDOS::GetExtraGroup() const
{
	return m_data.Data()->start.track * basic->GetSectorsPerTrackOnBasic() + m_data.Data()->start.sector - 1;
}

/// 追加のグループ番号を得る
void DiskBasicDirItemXDOS::GetExtraGroups(wxArrayInt &arr) const
{
	arr.Add((int)GetExtraGroup());
}

/// 日付を得る
void DiskBasicDirItemXDOS::GetFileCreateDate(TM &tm) const
{
	wxUint16 date = wxUINT16_SWAP_ON_LE(m_data.Data()->date);
	ConvDateToTm(date, tm);
}

/// 時間を得る
void DiskBasicDirItemXDOS::GetFileCreateTime(TM &tm) const
{
	wxUint16 time = wxUINT16_SWAP_ON_LE(m_data.Data()->time);
	ConvTimeToTm(time, tm);
}

void DiskBasicDirItemXDOS::SetFileCreateDate(const TM &tm)
{
	if (tm.GetYear() >= 0 && tm.GetMonth() >= 0) {
		wxUint16 date = ConvTmToDate(tm);
		m_data.Data()->date = wxUINT16_SWAP_ON_LE(date);
	}
}

void DiskBasicDirItemXDOS::SetFileCreateTime(const TM &tm)
{
	if (tm.GetHour() >= 0 && tm.GetMinute() >= 0) {
		wxUint16 time = ConvTmToTime(tm);
		m_data.Data()->time = wxUINT16_SWAP_ON_LE(time);
	}
}

/// 日付のタイトル名（ダイアログ用）
wxString DiskBasicDirItemXDOS::GetFileCreateDateTimeTitle() const
{
	return _("Created Date");
}

/// 日付を変換
void DiskBasicDirItemXDOS::ConvDateToTm(wxUint16 date, TM &tm)
{
	tm.SetYear(((date & 0xfe00) >> 9) + 80);
	tm.SetMonth(((date & 0x01e0) >> 5) - 1);
	tm.SetDay(date & 0x001f);
}

/// 時間を変換
void DiskBasicDirItemXDOS::ConvTimeToTm(wxUint16 time, TM &tm)
{
	tm.SetHour((time & 0xf800) >> 11);
	tm.SetMinute((time & 0x07e0) >> 5);
	tm.SetSecond((time & 0x001f) << 1);
}

/// 日付に変換
wxUint16 DiskBasicDirItemXDOS::ConvTmToDate(const TM &tm)
{
	return (wxUint16)
		(((tm.GetYear() - 80) & 0x7f) << 9)
		| (((tm.GetMonth() + 1) & 0xf) << 5)
		| (tm.GetDay() & 0x1f);
}

/// 時間に変換
wxUint16 DiskBasicDirItemXDOS::ConvTmToTime(const TM &tm)
{
	return (wxUint16)
		((tm.GetHour() & 0x1f) << 11)
		| ((tm.GetMinute() & 0x3f) << 5)
		| ((tm.GetSecond() & 0x3f) >> 1);
}

/// 開始アドレスを返す
int DiskBasicDirItemXDOS::GetStartAddress() const
{
	int addr;
	addr = m_data.Data()->load_addr;
	return basic->OrderUint16(addr);
}

// 実行アドレスを返す
int DiskBasicDirItemXDOS::GetExecuteAddress() const
{
	int addr;
	addr = m_data.Data()->exec_addr;
	return basic->OrderUint16(addr);
}

/// 開始アドレスをセット
void DiskBasicDirItemXDOS::SetStartAddress(int val)
{
	m_data.Data()->load_addr = basic->OrderUint16(val);
}

/// 実行アドレスをセット
void DiskBasicDirItemXDOS::SetExecuteAddress(int val)
{
	m_data.Data()->exec_addr = basic->OrderUint16(val);
}

/// ディレクトリサイズを返す
size_t DiskBasicDirItemXDOS::GetDataSize() const
{
	return sizeof(directory_xdos_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemXDOS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemXDOS::CopyData(const directory_t *val)
{
	return m_data.Copy(val, GetDataSize());
}

/// ディレクトリをクリア
void DiskBasicDirItemXDOS::ClearData()
{
	m_data.Fill(basic->GetDeleteCode(), GetDataSize());
}

/// ディレクトリを初期化 未使用にする
void DiskBasicDirItemXDOS::InitialData()
{
	m_data.Fill(basic->GetFillCodeOnDir(), GetDataSize());
}

/// データをエクスポートする前に必要な処理
/// 拡張子を付加する
bool DiskBasicDirItemXDOS::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	// 拡張子を付加する
	if (!IsDirectory()) {
		wxString ext = ConvFileType1Str(GetFileType1());
		filename += wxT(".");
		if (Utils::IsUpperString(filename)) {
			ext.MakeUpper();
		} else {
			ext.MakeLower();
		}
		filename += ext;
	}
	return true;
}

/// インポート時のダイアログを出す前にファイルパスから内部ファイル名を生成する
/// @param [in,out] filename ファイル名
/// @return ファイル名
bool DiskBasicDirItemXDOS::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		int p1 = 0;
		IsContainAttrByExtension(filename, gTypeNameXDOS1, TYPE_NAME_XDOS_BIN, TYPE_NAME_XDOS_DIC, &filename, NULL, &p1);
		if (!(TYPE_NAME_XDOS_BIN <= p1 && p1 <= TYPE_NAME_XDOS_DIC)) {
			wxFileName fn(filename);
			// 拡張子は除く
			filename = fn.GetName();
		}
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemXDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;
	// 拡張子で属性を設定する
	if (IsContainAttrByExtension(filename, gTypeNameXDOS1, TYPE_NAME_XDOS_BIN, TYPE_NAME_XDOS_DIC, NULL, &t1, NULL)) {
//	if (TYPE_NAME_XDOS_BIN <= t1 && t1 <= TYPE_NAME_XDOS_DIC) {
		t1 = (t1 << 8);
	} else {
		wxFileName fn(filename);
		t1 = ConvStrToUserFileType(fn.GetExt().Upper());
	}
	return t1;
}

//
// ダイアログ用
//

#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include "../ui/intnamebox.h"

#define IDC_COMBO_FTYPE 51
#define IDC_COMBO_STYPE 52
#define IDC_CHECK_ATTR0 53


/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemXDOS::GetFileType1Pos() const
{
	return GetFileType1();
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemXDOS::GetFileType2Pos() const
{
	return GetFileType2();
}

/// ファイルタイプのサブタイプを設定
static void SetSubTypeInAttrDialog(IntNameBox *parent, const xdos_sub_type_t *stypes, int stype, wxChoice *comSType)
{
	if (stypes) {
		for(int idx = 0; stypes[idx].desc != NULL; idx++) {
			for(int i = stypes[idx].start; i <= stypes[idx].end; i++) {
				wxString str = wxString::Format(wxT("0x%02x "), i);
				str += stypes[idx].desc;
				int row = comSType->Append(str);
				if (stype == i) {
					comSType->Select(row);
				}
			}
		}
	} else {
		comSType->Append(wxT("0x00"));
		comSType->Select(0);
	}
}

/// ファイルタイプのサブタイプを得る
static int GetSubTypeInAttrDialog(const IntNameBox *parent, const xdos_sub_type_t *stypes, int sel_idx, const wxChoice *comSType)
{
	int stype = 0;
	if (stypes) {
		int row = 0;
		for(int idx = 0; stypes[idx].desc != NULL; idx++) {
			for(int i = stypes[idx].start; i <= stypes[idx].end; i++) {
				if (sel_idx == row) {
					stype = i;
					break;
				}
				row++;
			}
		}
	}
	return stype;
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemXDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int t1 = GetFileType1();
	int t2 = GetFileType2();

	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		t1 = ConvOriginalTypeFromFileName(file_path);
	}

	// ファイルタイプ
	wxStaticBoxSizer *staFType = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);
	wxComboBox *comFType = new wxComboBox(parent, IDC_COMBO_FTYPE);
	for(int idx = 1; gTypeNameXDOS1[idx].name != NULL; idx++) {
		comFType->Append(gTypeNameXDOS1[idx].name);
	}
	int ftype = (t1 >> 8);
	if (TYPE_NAME_XDOS_BIN <= ftype && ftype <= TYPE_NAME_XDOS_DIC) {
		comFType->Select(ftype - 1);
	} else {
		comFType->SetValue(ConvFileType1Str(t1));
	}
	staFType->Add(comFType, flags);

	// サブタイプ
	wxChoice *comSType = new wxChoice(parent, IDC_COMBO_STYPE);

	staFType->Add(comSType, flags);

	sizer->Add(staFType, flags);

	// 属性
	wxStaticBoxSizer *staFAttr = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	for(int idx = 0; gTypeNameXDOS2[idx] != NULL; idx++) {
		wxCheckBox *chk = new wxCheckBox(parent, IDC_CHECK_ATTR0 + idx, wxGetTranslation(gTypeNameXDOS2[idx]));
		staFAttr->Add(chk, flags);

		chk->SetValue((t2 & (0x80 >> idx)) != 0);
	}
	sizer->Add(staFAttr, flags);

	// event handler
	parent->Bind(wxEVT_COMBOBOX, &IntNameBox::OnChangeType1, parent, IDC_COMBO_FTYPE);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemXDOS::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxComboBox	*comFType = (wxComboBox *)parent->FindWindow(IDC_COMBO_FTYPE);
	wxChoice	*comSType = (wxChoice *)parent->FindWindow(IDC_COMBO_STYPE);

	if (!comFType || !comSType) return;

	int typ1 = GetFileType1();
	int ftype = (typ1 >> 8);
	int stype = (typ1 & 0xff);
	int fidx = comFType->GetSelection();
	const xdos_sub_type_t *stypes = NULL;
	if (fidx >= 0) {
		stypes = xdos_sub_types[fidx + 1];
		if (fidx + 1 != ftype) {
			stype = -1;
		}
	}
	comSType->Clear();

	SetSubTypeInAttrDialog(parent, stypes, stype, comSType);
}

/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemXDOS::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxComboBox	*comFType = (wxComboBox *)parent->FindWindow(IDC_COMBO_FTYPE);
	wxChoice	*comSType = (wxChoice *)parent->FindWindow(IDC_COMBO_STYPE);

	int typ1 = 0;

	int fidx = comFType->GetSelection();
	if (fidx >= 0) {
		if (fidx < (TYPE_NAME_XDOS_DIR - 1)) {
			int sidx = comSType->GetSelection();
			const xdos_sub_type_t *stypes = NULL;
			stypes = xdos_sub_types[fidx + 1];

			typ1 = ((fidx + 1) << 8);
			typ1 |= GetSubTypeInAttrDialog(parent, stypes, sidx, comSType);
		} else {
			typ1 = 0x8000;	// DIR
		}
	} else {
		typ1 = ConvStrToUserFileType(comFType->GetValue());
	}
	return typ1;
}

/// @return CalcFileTypeFromPos()のpos2に渡す値
int DiskBasicDirItemXDOS::GetFileType2InAttrDialog(const IntNameBox *parent) const
{
	int typ2 = 0;
	for(int idx = 0; gTypeNameXDOS2[idx] != NULL; idx++) {
		wxCheckBox *chk = (wxCheckBox *)parent->FindWindow(IDC_CHECK_ATTR0 + idx);
		typ2 |= (chk->GetValue() ? (0x80 >> idx) : 0);
	}
	return typ2;
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemXDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	int val = GetFileType1InAttrDialog(parent) | (GetFileType2InAttrDialog(parent) << 16);

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, val);

	return true;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemXDOS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("FTYPE"), m_data.Data()->ftype, true);
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("LOAD_ADDR"), m_data.Data()->load_addr, basic->IsBigEndian());
	vals.Add(wxT("FILE_SIZE"), m_data.Data()->file_size, basic->IsBigEndian());
	vals.Add(wxT("EXEC_ADDR"), m_data.Data()->exec_addr, basic->IsBigEndian());
	vals.Add(wxT("DATE"), m_data.Data()->date, true);
	vals.Add(wxT("TIME"), m_data.Data()->time, true);
	vals.Add(wxT("ATTR"), m_data.Data()->attr);
	vals.Add(wxT("START"), &m_data.Data()->start, sizeof(m_data.Data()->start));
}
