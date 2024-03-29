/// @file basicdiritem_prodos.cpp
///
/// @brief disk basic directory item for Apple ProDOS 8 / 16
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_prodos.h"
//#include <wx/regex.h>
#include "basicfmt.h"
#include "basictype.h"
#include "../config.h"
#include "../charcodes.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// Apple ProDOS属性名 STORAGE_TYPE
const char *gTypeNameProDOS1[] = {
	wxTRANSLATE("File"),
	wxTRANSLATE("<DIR>"),
	wxTRANSLATE("<VOL>"),
	NULL
};

/// Apple ProDOS属性名 FILE_TYPE
const name_value_t gTypeNameProDOS2[] = {
	{ "<no type>", FILETYPE_MASK_PRODOS_NOT },
	{ "BAD", FILETYPE_MASK_PRODOS_BAD },
	{ "TXT", FILETYPE_MASK_PRODOS_TXT },
	{ "BIN", FILETYPE_MASK_PRODOS_BIN },
	{ "DIR", FILETYPE_MASK_PRODOS_DIR },
	{ "ADB", FILETYPE_MASK_PRODOS_ADB },
	{ "AWB", FILETYPE_MASK_PRODOS_AWP },
	{ "ASP", FILETYPE_MASK_PRODOS_ASP },
	{ "PAS", FILETYPE_MASK_PRODOS_PAS },
	{ "CMD", FILETYPE_MASK_PRODOS_CMD },
	{ "INT", FILETYPE_MASK_PRODOS_INT },
	{ "IVR", FILETYPE_MASK_PRODOS_IVR },
	{ "BAS", FILETYPE_MASK_PRODOS_BAS },
	{ "VAR", FILETYPE_MASK_PRODOS_VAR },
	{ "REL", FILETYPE_MASK_PRODOS_REL },
	{ "SYS", FILETYPE_MASK_PRODOS_SYS },
	{ NULL, -1 }
};

/// Apple ProDOS属性名 ACCESS
const name_value_t gTypeNameProDOS3[] = {
	{ wxTRANSLATE("Readable"), FILETYPE_MASK_PRODOS_READ },
	{ wxTRANSLATE("Writable"), FILETYPE_MASK_PRODOS_WRITE },
	{ wxTRANSLATE("Changed"), FILETYPE_MASK_PRODOS_CHANGE },
	{ wxTRANSLATE("Can Rename"), FILETYPE_MASK_PRODOS_RENAME },
	{ wxTRANSLATE("Can Destroy"), FILETYPE_MASK_PRODOS_DESTROY },
	{ NULL, -1 }
};

/// Apple ProDOS属性名 ACCESS 一覧表示用
const char gTypeNameProDOS3S[] = "rwcnd";

//////////////////////////////////////////////////////////////////////
//
// Apple ProDOS インデックス１つ
//
ProDOSOneIndex::ProDOSOneIndex()
{
	for(int i=0; i<(int)(sizeof(m_buf)/sizeof(m_buf[0])); i++) {
		m_buf[i] = NULL;
	}
	m_size = 0;
	m_group_num = (wxUint32)-1;
}

/// セクタバッファを割当て
void ProDOSOneIndex::AttachBuffer(DiskBasic *basic, wxUint32 group_num, int st_pos)
{
	DiskImageSector *sector = NULL;
	int buf_idx = 0;
	for(int i=0; i < basic->GetSectorsPerGroup() && buf_idx < 2; i++) {
		sector = basic->GetSectorFromSectorPos(st_pos + i);
		if (!sector) break;
		m_size = sector->GetSectorSize();
		for(int buf_pos = 0; buf_pos < m_size && buf_idx < 2; buf_pos += 256) {
			SetBuffer(buf_idx, sector->GetSectorBuffer(buf_pos));
			buf_idx++;
		}
	}
}

/// 指定位置のブロック番号を得る
/// @param[in] pos 位置
/// @return ブロック番号
wxUint16 ProDOSOneIndex::GetGroupNumber(int pos) const
{
	return ((wxUint16)m_buf[1][pos] << 8) | m_buf[0][pos];
}

/// 指定位置のブロック番号をセット
/// @param[in] pos 位置 インデックスすべての通し番号
/// @param[in] val ブロック番号
void ProDOSOneIndex::SetGroupNumber(int pos, wxUint16 val)
{
	m_buf[1][pos] = (val >> 8);
	m_buf[0][pos] = (val & 0xff);
}

//////////////////////////////////////////////////////////////////////

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfProDOSIndex);

//////////////////////////////////////////////////////////////////////
//
// Apple ProDOS インデックス ProDOSOneIndex の配列
//
ProDOSIndex::ProDOSIndex()
	: ArrayOfProDOSIndex()
{
	m_basic = NULL;
}

ProDOSIndex::~ProDOSIndex()
{
}

#ifdef COPYABLE_DIRITEM
/// 代入
ProDOSIndex &ProDOSIndex::operator=(const ProDOSIndex &src)
{
	this->Dup(src);
	return *this;
}
/// 複製
void ProDOSIndex::Dup(const ProDOSIndex &src)
{
	sector = src.sector;
	if (src.chain_ownmake) {
		chain = new prodos_chain_t;
		memcpy(&chain, src.chain, sizeof(prodos_chain_t));
	} else {
		chain = src.chain;
	}
	chain_ownmake = src.chain_ownmake;
}
#endif

/// BASICをセット
void ProDOSIndex::SetBasic(DiskBasic *basic)
{
	m_basic = basic;
}

/// 指定位置のブロック番号を得る
/// @param[in] pos 位置 インデックスすべての通し番号
/// @return ブロック番号
wxUint16 ProDOSIndex::GetGroupNumber(int pos) const
{
	wxUint16 group_num = 0xffff;

	for(size_t idx = 0; idx < Count(); idx++) {
		ProDOSOneIndex *item = &Item(idx);
		int size = item->GetSize();
		if (pos < size) {
			group_num = item->GetGroupNumber(pos);
			break;
		}
		pos -= size;
	}
	return group_num;
}

/// 指定位置のブロック番号をセット
/// @param[in] pos 位置 インデックスすべての通し番号
/// @param[in] val ブロック番号
void ProDOSIndex::SetGroupNumber(int pos, wxUint16 val)
{
	for(size_t idx = 0; idx < Count(); idx++) {
		ProDOSOneIndex *item = &Item(idx);
		int size = item->GetSize();
		if (pos < size) {
			item->SetGroupNumber(pos, val);
			break;
		}
		pos -= size;
	}
}

//////////////////////////////////////////////////////////////////////
//
// ディレクトリ１アイテム Apple ProDOS 8 / 16
//
DiskBasicDirItemProDOS::DiskBasicDirItemProDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
	AllocateItem(NULL);

	m_dir_group_num = 0;
}
DiskBasicDirItemProDOS::DiskBasicDirItemProDOS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
	AllocateItem(NULL);

	m_dir_group_num = 0;
}
DiskBasicDirItemProDOS::DiskBasicDirItemProDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);
	AllocateItem(n_next);

	m_dir_group_num = type->GetSectorPosFromNum(n_gitem->track, n_gitem->side, n_sector->GetSectorNumber());
	m_dir_group_num /= basic->GetSectorsPerGroup();

	Used(CheckUsed(n_unuse));

	// チェインセクタへのポインタをセット
	if (IsUsed()) {
		m_index.Clear();
		m_index.SetBasic(basic);
		int stype = GetFileType1();
		switch(stype) {
		case FILETYPE_MASK_PRODOS_SAPLING:
		case FILETYPE_MASK_PRODOS_TREE:
			wxUint32 grp_num = GetStartGroup(0);

//			int sector_size = basic->GetSectorSize();
			int st_pos = type->GetStartSectorFromGroup(grp_num);

			ProDOSOneIndex item;
			item.AttachBuffer(basic, grp_num, st_pos);
			m_index.Add(item);

			if (stype == FILETYPE_MASK_PRODOS_TREE) {
				// tree
				int size = 256;
				for(int i = 0; i < size; i++) {
					grp_num = m_index.GetGroupNumber(i);
					if (grp_num == 0) {
						break;
					}
					st_pos = type->GetStartSectorFromGroup(grp_num);

					ProDOSOneIndex item;
					item.AttachBuffer(basic, grp_num, st_pos);
					m_index.Add(item);
				}
			}
			break;
		}
	}

	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemProDOS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
	AllocateItem(n_next);
}

/// ディレクトリエントリを確保
/// data  は内部で確保したメモリ
/// sdata がセクタ内部へのポインタとなる
bool DiskBasicDirItemProDOS::AllocateItem(const SectorParam *next)
{
	m_sdata.Clear();
	bool bound = m_sdata.Set(basic, m_sector, m_position, (directory_t *)m_data.Data(), GetDataSize(), next);

	if (!m_data.IsSelf() && bound) {
		// セクタをまたぐ場合、dataは内部で確保する
		m_data.Alloc();
		m_data.Fill(0);
	}

	// コピー
	if (m_data.IsSelf()) {
		m_sdata.CopyTo((directory_t *)m_data.Data());
	}

	return true;
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemProDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = sizeof(m_data.Data()->name);
		return m_data.Data()->name;
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// ファイル名を設定
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
void DiskBasicDirItemProDOS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		memcpy(n, filename, ns);
	}

	nl = rtrim(n, length, basic->GetDirTerminateCode());

	// ファイル名長さ
	m_data.Data()->stype_and_nlen = (wxUint8)((nl & 0xf) | (m_data.Data()->stype_and_nlen & 0xf0));
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemProDOS::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t s = 0;
	size_t l = 0;

	n = GetFileNamePos(0, s, l);
	if (n != NULL && s > 0) {
		if (s > size) s = size;
		memcpy(filename, n, s);
		size_t nlen = m_data.Data()->stype_and_nlen & 0xf;
		if (nlen < size) filename[nlen] = 0;
	}

	length = l;
}

/// ファイル名＋拡張子のサイズ
int DiskBasicDirItemProDOS::GetFileNameStrSize() const
{
	size_t s = 0;
	size_t l = 0;
	GetFileNamePos(0, s, l);

	return (int)s;
}

/// 属性１を返す STORAGE_TYPE
int	DiskBasicDirItemProDOS::GetFileType1() const
{
	return (m_data.Data()->stype_and_nlen >> 4);
}

/// 属性１を設定 STORAGE_TYPE
void DiskBasicDirItemProDOS::SetFileType1(int val)
{
	m_data.Data()->stype_and_nlen = (val << 4) | (m_data.Data()->stype_and_nlen & 0x0f);
}

/// 属性２を返す FILE_TYPE
int	DiskBasicDirItemProDOS::GetFileType2() const
{
	return m_data.Data()->file_type;
}

/// 属性２を設定 FILE_TYPE
void DiskBasicDirItemProDOS::SetFileType2(int val)
{
	m_data.Data()->file_type = (val & 0xff);
}

/// 属性３を返す ACCESS
int DiskBasicDirItemProDOS::GetFileType3() const
{
	return m_data.Data()->access;
}

/// 属性３のセット ACCESS
void DiskBasicDirItemProDOS::SetFileType3(int val)
{
	m_data.Data()->access = (val & 0xff);
}

/// AUX_TYPEを返す
int DiskBasicDirItemProDOS::GetAuxType() const
{
	return wxUINT16_SWAP_ON_BE(m_data.Data()->f.aux_type);
}

/// AUX_TYPEのセット
void DiskBasicDirItemProDOS::SetAuxType(int val)
{
	m_data.Data()->f.aux_type = wxUINT16_SWAP_ON_BE(val & 0xffff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemProDOS::CheckUsed(bool unuse)
{
	return GetFileType1() != 0;
}

/// バージョンを返す(VERSION,MIN_VERSION)
int DiskBasicDirItemProDOS::GetVersion() const
{
	return (int)m_data.Data()->version << 8 | m_data.Data()->min_version;
}

/// バージョンをセット(VERSION,MIN_VERSION)
void DiskBasicDirItemProDOS::SetVersion(int val)
{
	m_data.Data()->min_version = val & 0xff;
	val >>= 8;
	m_data.Data()->version = val & 0xff;
}

/// 使用ブロック数を返す
int DiskBasicDirItemProDOS::GetBlocksUsed() const
{
	return wxUINT16_SWAP_ON_BE(m_data.Data()->blocks_used);
}

/// 使用ブロック数をセット
void DiskBasicDirItemProDOS::SetBlocksUsed(int val)
{
	m_data.Data()->blocks_used = wxUINT16_SWAP_ON_BE(val & 0xffff);
}

/// 削除
bool DiskBasicDirItemProDOS::Delete()
{
	// 削除
	Used(false);
	m_data.Data()->stype_and_nlen = 0;
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemProDOS::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;

	wxUint8 stype = GetFileType1();
	if (0x3 < stype && stype < 0xd) {
		valid = false;
	}
	return valid;
}

/// 属性を設定
/// @note 固有属性の意味: STORAGE_TYPE,FILE_TYPE,ACCESS
void DiskBasicDirItemProDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	if (file_type.GetFormat()== basic->GetFormatTypeNumber()) {
		// 同じOSから
		int t3 = file_type.GetOrigin();
		int t2 = t3 >> 8;
		int t1 = t2 >> 8;
		t2 &= 0xff;
		t1 &= 0xff;

		SetFileType1(t1);
		if (t1 != FILETYPE_MASK_PRODOS_VOLUME) {
			SetFileType2(t2);
		}
		SetFileType3(t3);

		int type4 = file_type.GetOrigin(1);
		int version = file_type.GetOrigin(2);
		SetAuxType(type4);
		SetVersion(version);

	} else {
		// 違うOSから
		int t1 = 0;
		int t2 = FILETYPE_MASK_PRODOS_NOT;
		int t3 = FILETYPE_MASK_PRODOS_ACCESS_ALL;
		if (ftype & FILE_TYPE_BASIC_MASK) {
			t2 = FILETYPE_MASK_PRODOS_BAS;
		} else if (ftype & FILE_TYPE_DIRECTORY_MASK) {
			t1 = FILETYPE_MASK_PRODOS_SUBDIR;
			t2 = FILETYPE_MASK_PRODOS_DIR;
		}
		if (ftype & FILE_TYPE_READONLY_MASK) {
			t3 &= ~FILETYPE_MASK_PRODOS_WRITE;
		}
		if (ftype & FILE_TYPE_UNDELETE_MASK) {
			t3 &= ~FILETYPE_MASK_PRODOS_DESTROY;
		}
		if (ftype & FILE_TYPE_WRITEONLY_MASK) {
			t3 &= ~FILETYPE_MASK_PRODOS_READ;
		}
		if (ftype & (FILE_TYPE_SYSTEM_MASK | FILE_TYPE_HIDDEN_MASK)) {
			t3 &= ~(FILETYPE_MASK_PRODOS_WRITE | FILETYPE_MASK_PRODOS_RENAME | FILETYPE_MASK_PRODOS_DESTROY);
		}

		if (t1 > 0) SetFileType1(t1);
		SetFileType2(t2);
		SetFileType3(t3);
	}
}

/// 属性を返す
/// @note 固有属性の意味: STORAGE_TYPE,FILE_TYPE,ACCESS
DiskBasicFileType DiskBasicDirItemProDOS::GetFileAttr() const
{
	int val = 0;
	int stype = GetFileType1();
	int ftype = GetFileType2();
	int access = GetFileType3();

	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBDIR:
		val |= FILE_TYPE_DIRECTORY_MASK;
		break;
	case FILETYPE_MASK_PRODOS_VOLUME:
	case FILETYPE_MASK_PRODOS_SUBVOL:
		val |= FILE_TYPE_VOLUME_MASK;
		ftype = 0;
		break;
	default:
		break;
	}

	int type4 = GetAuxType();
	int version = GetVersion();

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, (stype << 16) | (ftype << 8) | access, type4, version);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemProDOS::GetFileAttrStr() const
{
	wxString str;
	int spos = ConvFileType1Pos(GetFileType1());
	if (spos != TYPE_NAME_PRODOS_FILE) {
		// DIR or VOL
		str = gTypeNameProDOS1[spos];
	} else {
		// FILE TYPE
		int type2 = GetFileType2();
		int fpos = ConvFileType2Pos(type2);
		if (fpos >= 0) {
			str = gTypeNameProDOS2[fpos].name;
		} else {
			str = wxString::Format(wxT("0x%02x"), type2);
		}
	}

	// ACCESS
	str += wxT(" ,");

	int access = GetFileType3();
	for(int i=0; gTypeNameProDOS3[i].name != NULL; i++) {
		if (access & gTypeNameProDOS3[i].value) {
			str += gTypeNameProDOS3S[i];
		} else {
			str += "-";
		}
	}

	return str;
}

/// 属性からリストの位置を返す
int DiskBasicDirItemProDOS::ConvFileType1Pos(int type1) const
{
	int pos;
	switch(type1) {
	case FILETYPE_MASK_PRODOS_SUBDIR:
		pos = TYPE_NAME_PRODOS_SUBDIR;
		break;
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		pos = TYPE_NAME_PRODOS_VOLUME;
		break;
	default:
		pos = TYPE_NAME_PRODOS_FILE;
		break;
	}

	return pos;
}

/// 属性からリストの位置を返す
int DiskBasicDirItemProDOS::ConvFileType2Pos(int type2) const
{
	return gTypeNameProDOS2[0].IndexOf(gTypeNameProDOS2, type2 & 0xff);
}

/// ファイルサイズをセット
void DiskBasicDirItemProDOS::SetFileSize(int val)
{
	m_groups.SetSize(val);
	int blk = basic->GetSectorSize() * basic->GetSectorsPerGroup();
	int grps = (val + blk - 1) / blk;
	grps += (int)m_index.Count();

	m_data.Data()->eof[0] = (val & 0xff);
	val >>= 8;
	m_data.Data()->eof[1] = (val & 0xff);
	val >>= 8;
	m_data.Data()->eof[2] = (val & 0xff);

	SetBlocksUsed(grps);
}

/// ファイルサイズを返す
int DiskBasicDirItemProDOS::GetFileSize() const
{
	int val = (int)m_data.Data()->eof[0]
		+ ((int)m_data.Data()->eof[1] << 8)
		+ ((int)m_data.Data()->eof[2] << 16);

	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		val = 0;
		break;
	}
	return val;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemProDOS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemProDOS::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
//	if (!chain.IsValid()) return;

	int calc_groups = 0;
	int calc_file_size = 0;

	int track_num = 0;
	int side_num = 0;
	int sector_num = 0;

	int sector_size = basic->GetSectorSize();
	int block_size = sector_size * basic->GetSectorsPerGroup();

	int remain_size = GetFileSize();
	wxUint32 group_num = GetStartGroup(fileunit_num);
	int sector_pos = group_num * basic->GetSectorsPerGroup();

	int stype = GetFileType1();
	if (stype == FILETYPE_MASK_PRODOS_SEEDING) {
		// 1ブロックで収まるファイル
		basic->GetNumsFromGroup(group_num, 0, sector_size, remain_size, group_items);
		calc_groups++;
		calc_file_size += GetFileSize();
	} else if (stype == FILETYPE_MASK_PRODOS_SAPLING) {
		// インデックスを参照するファイル
		for(int i=0; i < GetBlocksUsed(); i++) {
			group_num = m_index.GetGroupNumber(i);
			if (group_num == 0) {
				break;
			}
			basic->GetNumsFromGroup(group_num, 0, sector_size, remain_size, group_items);
			calc_groups++;
		}
		calc_file_size += GetFileSize();
	} else if (stype == FILETYPE_MASK_PRODOS_TREE) {
		// インデックスを参照するファイル ツリー形式
		for(int i=0; i < GetBlocksUsed(); i++) {
			group_num = m_index.GetGroupNumber(i + sector_size);
			if (group_num == 0) {
				break;
			}
			basic->GetNumsFromGroup(group_num, 0, sector_size, remain_size, group_items);
			calc_groups++;
		}
		calc_file_size += GetFileSize();
	} else if (stype == FILETYPE_MASK_PRODOS_SUBDIR) {
		// サブディレクトリ
		for(int i=0; i < GetBlocksUsed(); i++) {
			prodos_dir_ptr_t next;
			next.next_block = 0;

			for(int ss = 0; ss < basic->GetSectorsPerGroup(); ss++) {
				DiskImageSector *sector = basic->GetSectorFromSectorPos(sector_pos, track_num, side_num);
				if (!sector) {
					break;
				}
				wxUint8 *buffer = sector->GetSectorBuffer();
				if (!buffer) {
					break;
				}
				if (ss == 0) {
					// 次のブロックへのポインタを保持
					memcpy(&next, buffer, sizeof(prodos_dir_ptr_t));
				}

				sector_num = sector->GetSectorNumber();

				group_items.Add(group_num, 0, track_num, side_num, sector_num, sector_num);

				sector_pos++;
			}

			calc_groups++;

			// 次のセクタなし
			if (next.next_block == 0) {
				break;
			}

			group_num = wxUINT16_SWAP_ON_BE(next.next_block);
			sector_pos = (int)group_num * basic->GetSectorsPerGroup();
		}
		calc_file_size += GetFileSize();
	} else {

	}
	group_items.SetNums(calc_groups);
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(block_size);

//	// 最終セクタの再計算
//	group_items.SetSize(RecalcFileSize(group_items, (int)group_items.GetSize()));

	// ファイル内部のアドレスを得る
	TakeAddressesInFile(group_items);
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有サイズ
/// @return 計算後のファイルサイズ
int DiskBasicDirItemProDOS::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
#if 0
	if (group_items.Count() == 0) return occupied_size;

	DiskBasicGroupItem *litem = &group_items.Last();
	DiskImageSector *sector = basic->GetSector(litem->track, litem->side, litem->sector_end);
	if (!sector) return occupied_size;

	int sector_size = sector->GetSectorSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	int remain_size = type->CalcDataSizeOnLastSector(this, NULL, NULL, buf, sector_size, sector_size);

	occupied_size = occupied_size - sector_size + remain_size;
#endif
	return occupied_size;
}

/// ファイル内部のアドレスを取り出す
void DiskBasicDirItemProDOS::TakeAddressesInFile(DiskBasicGroups &group_items)
{
//	m_start_address = -1;
//	m_data_length = -1;

	if (group_items.Count() == 0) {
		return;
	}

	DiskBasicGroupItem *item = &group_items.Item(0);
	DiskImageSector *sector = basic->GetSector(item->track, item->side, item->sector_start);
	if (!sector) return;
#if 0
	int t1 = GetFileType1();

	if (t1 & FILETYPE_MASK_PRODOS_BINARY) {
		// バイナリ
		// 開始アドレス
		m_start_address = (int)sector->Get16(0);
		// データサイズ
		m_data_length = (int)sector->Get16(2);
		// 実際のサイズを設定
		if (m_data_length + 5 <= (int)group_items.GetSize()) group_items.SetSize(m_data_length + 5);
	}
	else if (t1 & (FILETYPE_MASK_PRODOS_IBASIC | FILETYPE_MASK_PRODOS_ABASIC)) {
		// BASICファイルサイズ
		// データサイズ => 最終データ位置みたい
		m_data_length = (int)sector->Get16(0);
		// 実際のサイズを設定
		if (m_data_length + 3 <= (int)group_items.GetSize()) group_items.SetSize(m_data_length + 3);
	}
#endif
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemProDOS::GetDataSize() const
{
	return sizeof(directory_prodos_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemProDOS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemProDOS::CopyData(const directory_t *val)
{
	return m_data.Copy(val);
}

/// ディレクトリをクリア
void DiskBasicDirItemProDOS::ClearData()
{
	m_data.Fill(0);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemProDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	m_data.Data()->key_pointer = wxUINT16_SWAP_ON_BE((wxUint16)val);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemProDOS::GetStartGroup(int fileunit_num) const
{
	wxUint32 val = wxUINT16_SWAP_ON_BE(m_data.Data()->key_pointer);

	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		val = m_dir_group_num;
		break;
	}
	return val;
}

/// ディレクトリヘッダのあるグループ番号をセット(機種依存)(HEADER_POINTER)
/// @param [in] val 番号
void DiskBasicDirItemProDOS::SetParentGroup(wxUint32 val)
{
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		break;
	default:
		m_data.Data()->f.header_pointer = wxUINT16_SWAP_ON_BE(val);
		break;
	}
}

/// ディレクトリヘッダのあるグループ番号を返す(機種依存)(HEADER_POINTER)
/// @return 番号
wxUint32 DiskBasicDirItemProDOS::GetParentGroup() const
{
	wxUint32 val;
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		val = INVALID_GROUP_NUMBER;
		break;
	default:
		val = wxUINT16_SWAP_ON_BE(m_data.Data()->f.header_pointer);
		break;
	}
	return val;
}

/// 追加のグループ番号を返す(機種依存)
wxUint32 DiskBasicDirItemProDOS::GetExtraGroup() const
{
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SAPLING:
	case FILETYPE_MASK_PRODOS_TREE:
		return GetStartGroup(0);
		break;
	}
	return INVALID_GROUP_NUMBER;
}

/// 追加のグループ番号を得る(機種依存)
void DiskBasicDirItemProDOS::GetExtraGroups(wxArrayInt &arr) const
{
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SAPLING:
	case FILETYPE_MASK_PRODOS_TREE:
		arr.Add((int)GetStartGroup(0));
		break;
	}
}

/// チェイン用のセクタをクリア(機種依存)
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemProDOS::ClearChainSector(const DiskBasicDirItem *pitem)
{
	for(size_t i=0; i<m_index.Count(); i++) {
		ProDOSOneIndex *item = &m_index.Item(i);
		wxUint32 gnum = item->GetMyGroupNumber();
		type->DeleteGroupNumber(gnum);
	}
	m_index.Clear();
	m_index.SetBasic(basic);
}

/// チェイン用のセクタをセット
/// @param [in] num    グループ番号
/// @param [in] pos    セクタ位置
/// @param [in] data   未使用
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemProDOS::SetChainSector(wxUint32 num, int pos, wxUint8 *WXUNUSED(data), const DiskBasicDirItem *pitem)
{
	ProDOSOneIndex item;
	item.AttachBuffer(basic, num, pos);
	m_index.Add(item);
}

/// チェイン用のセクタにグループ番号をセット(機種依存)
void DiskBasicDirItemProDOS::AddChainGroupNumber(int idx, wxUint32 val)
{
	m_index.SetGroupNumber(idx, val);
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemProDOS::NeedCheckEofCode()
{
	return false;
}

/// セーブ時にファイルサイズを再計算する
int DiskBasicDirItemProDOS::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	int stype;
	if (file_size < 0x200) {
		// 512バイト未満はインデックスなし
		stype = FILETYPE_MASK_PRODOS_SEEDING;
	} else if (file_size < 0x20000) {
		// 131Kバイト未満はインデックス１つ
		stype = FILETYPE_MASK_PRODOS_SAPLING;
	} else {
		// 131Kバイト以上 ツリー
		stype = FILETYPE_MASK_PRODOS_TREE;
	}
	SetFileType1(stype);

	return file_size;
}

/// データをエクスポートする前に必要な処理
/// 属性に応じて拡張子を付加する
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemProDOS::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	/// 属性から拡張子を付加する
	if (!IsDirectory()) {
		wxString ext;
		if (GetFileAttrName(ConvFileType2Pos(GetFileType2()), gTypeNameProDOS2, ext)) {
			filename += wxT(".");
			if (Utils::IsUpperString(filename)) {
				filename += ext.Upper();
			} else {
				filename += ext.Lower();
			}
		}
	}
	return true;
}

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemProDOS::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		IsContainAttrByExtension(filename, gTypeNameProDOS2, TYPE_NAME_PRODOS_NOT, TYPE_NAME_PRODOS_SYS, &filename, NULL, NULL);
	}
	// 拡張子を消す
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemProDOS::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t2 = 0;
	// 拡張子で属性を設定する
	if (!IsContainAttrByExtension(filename, gTypeNameProDOS2, TYPE_NAME_PRODOS_NOT, TYPE_NAME_PRODOS_SYS, NULL, &t2, NULL)) {
		t2 = FILETYPE_MASK_PRODOS_TXT;
	}

	return t2;
}

/// アイテムを削除できるか
bool DiskBasicDirItemProDOS::IsDeletable() const
{
	int stype = GetFileType1();
	return (stype != FILETYPE_MASK_PRODOS_SUBVOL && stype != FILETYPE_MASK_PRODOS_VOLUME);
}

/// アイテムが更新日時を持っているか
bool DiskBasicDirItemProDOS::HasModifyDateTime() const
{
	int stype = GetFileType1();
	return (stype != FILETYPE_MASK_PRODOS_SUBVOL && stype != FILETYPE_MASK_PRODOS_VOLUME);
}

/// 日付を得る
void DiskBasicDirItemProDOS::GetFileCreateDate(TM &tm) const
{
	ConvDateToTm(m_data.Data()->cdate, tm);
}

/// 時間を得る
void DiskBasicDirItemProDOS::GetFileCreateTime(TM &tm) const
{
	ConvTimeToTm(m_data.Data()->ctime, tm);
}

/// 日付を文字列で返す
wxString DiskBasicDirItemProDOS::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 時間を文字列で返す
wxString DiskBasicDirItemProDOS::GetFileCreateTimeStr() const
{
	TM tm;
	GetFileCreateTime(tm);
	return Utils::FormatHMStr(tm);
}

/// 日付を設定
void DiskBasicDirItemProDOS::SetFileCreateDate(const TM &tm)
{
	ConvDateFromTm(tm, m_data.Data()->cdate);
}

/// 時間を設定
void DiskBasicDirItemProDOS::SetFileCreateTime(const TM &tm)
{
	ConvTimeFromTm(tm, m_data.Data()->ctime);
}

/// 日付を得る
void DiskBasicDirItemProDOS::GetFileModifyDate(TM &tm) const
{
	int stype = GetFileType1();
	switch (stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		tm.ClearDate();
		break;
	default:
		ConvDateToTm(m_data.Data()->f.mdate, tm);
		break;
	}
}

/// 時間を得る
void DiskBasicDirItemProDOS::GetFileModifyTime(TM &tm) const
{
	int stype = GetFileType1();
	switch (stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		tm.ClearTime();
		break;
	default:
		ConvTimeToTm(m_data.Data()->f.mtime, tm);
		break;
	}
}

/// 日付を文字列で返す
wxString DiskBasicDirItemProDOS::GetFileModifyDateStr() const
{
	TM tm;
	GetFileModifyDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 時間を文字列で返す
wxString DiskBasicDirItemProDOS::GetFileModifyTimeStr() const
{
	TM tm;
	GetFileModifyTime(tm);
	return Utils::FormatHMStr(tm);
}

/// 日付を設定
void DiskBasicDirItemProDOS::SetFileModifyDate(const TM &tm)
{
	ConvDateFromTm(tm, m_data.Data()->f.mdate);
}

/// 時間を設定
void DiskBasicDirItemProDOS::SetFileModifyTime(const TM &tm)
{
	ConvTimeFromTm(tm, m_data.Data()->f.mtime);
}

/// 日付を変換
void DiskBasicDirItemProDOS::ConvDateToTm(const wxUint8 *data, TM &tm)
{
	int ymd = 0;

	ymd = (int)data[1] << 8 | data[0]; 

	tm.SetYear(ymd >> 9);
	if (tm.GetYear() < 70) tm.AddYear(100);
	tm.SetMonth(((ymd >> 5) & 0xf) - 1);
	tm.SetDay(ymd & 0x1f);
}

/// 時間を変換
void DiskBasicDirItemProDOS::ConvTimeToTm(const wxUint8 *data, TM &tm)
{
	tm.SetHour(data[1] & 0x1f);
	tm.SetMinute(data[0] & 0x3f);
	tm.SetSecond(0);
}

/// 日付を変換
void DiskBasicDirItemProDOS::ConvDateFromTm(const TM &tm, wxUint8 *data)
{
	if (tm.GetYear() >= 0 && tm.GetMonth() >= -1) {
		data[1] = (((tm.GetYear() & 0x7f) % 100) << 1) | (((tm.GetMonth() + 1) & 0x8) >> 3);
		data[0] = (((tm.GetMonth() + 1) & 0x7) << 5) | (tm.GetDay() & 0x1f); 
	}
}

/// 時間を変換
void DiskBasicDirItemProDOS::ConvTimeFromTm(const TM &tm, wxUint8 *data)
{
	if (tm.GetHour() >= 0 && tm.GetMinute() >= 0) {
		data[1] = tm.GetHour() & 0x1f;
		data[0] = tm.GetMinute() & 0x3f;
	}
}

/// 開始アドレスを返す
int DiskBasicDirItemProDOS::GetStartAddress() const
{
	int ftype = GetFileType2();
	switch(ftype) {
	case FILETYPE_MASK_PRODOS_BIN:
	case FILETYPE_MASK_PRODOS_BAS:
	case FILETYPE_MASK_PRODOS_SYS:
		return GetAuxType();
		break;
	}

	return -1;
}

/// 終了アドレスを返す
int DiskBasicDirItemProDOS::GetEndAddress() const
{
	int val = -1;
	int ftype = GetFileType2();
	switch(ftype) {
	case FILETYPE_MASK_PRODOS_BIN:
	case FILETYPE_MASK_PRODOS_BAS:
	case FILETYPE_MASK_PRODOS_SYS:
		val = GetAuxType() + GetFileSize() - 1;
		break;
	}

	return val;
}

/// 開始アドレスをセット
void DiskBasicDirItemProDOS::SetStartAddress(int val)
{
	int ftype = GetFileType2();
	switch(ftype) {
	case FILETYPE_MASK_PRODOS_BIN:
	case FILETYPE_MASK_PRODOS_BAS:
	case FILETYPE_MASK_PRODOS_SYS:
		SetAuxType(val);
		break;
	}
}

/// ファイル数を＋１
void DiskBasicDirItemProDOS::IncreaseFileCount()
{
	wxUint16 val = 0;
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		val = wxUINT16_SWAP_ON_BE(m_data.Data()->v.file_count);
		val++;
		m_data.Data()->v.file_count = wxUINT16_SWAP_ON_BE(val);
	default:
		break;
	}
}

/// ファイル数を－１
void DiskBasicDirItemProDOS::DecreaseFileCount()
{
	wxUint16 val = 0;
	int stype = GetFileType1();
	switch(stype) {
	case FILETYPE_MASK_PRODOS_SUBVOL:
	case FILETYPE_MASK_PRODOS_VOLUME:
		val = wxUINT16_SWAP_ON_BE(m_data.Data()->v.file_count);
		val--;
		m_data.Data()->v.file_count = wxUINT16_SWAP_ON_BE(val);
		break;
	default:
		break;
	}
}

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemProDOS::SetModify()
{
	if (m_data.IsSelf()) {
		m_sdata.CopyFrom((const directory_t *)m_data.Data());
	}
}

//
// ダイアログ用
//

#include <wx/radiobox.h>
#include <wx/combobox.h>
#include <wx/checkbox.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"
#include "../ui/intnamevalid.h"

#define IDC_COMBO_TYPE1	51
#define IDC_COMBO_TYPE2 52
#define IDC_TEXT_TYPE4 53
#define IDC_TEXT_VERSION 54
#define IDC_CHECK_TYPE3 55


/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemProDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		file_type_1 = FILETYPE_MASK_PRODOS_SEEDING << 8	| FILETYPE_MASK_PRODOS_ACCESS_ALL;
		file_type_2 = ConvOriginalTypeFromFileName(name);
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in,out] sizer      レイアウト
/// @param [in] flags          レイアウトフラグ
void DiskBasicDirItemProDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1() << 8 | GetFileType3();
	int file_type_2 = GetFileType2();
	wxChoice   *comType1;
	wxComboBox *comType2;
	wxTextCtrl *txtType4;
	wxTextCtrl *txtType5;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	int file_type_3 = file_type_1 & 0xff;
	file_type_1 >>= 8;

	wxSizerFlags expand = wxSizerFlags().Expand();
	wxGridSizer *gbox = new wxGridSizer(1, 2, 4, 4);

	// STORAGE_TYPE

	wxArrayString types1;
	for(size_t i=0; i<=TYPE_NAME_PRODOS_VOLUME; i++) {
		types1.Add(wxGetTranslation(gTypeNameProDOS1[i]));
	}
	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("Storage Type")), wxVERTICAL);
	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	int type1pos = ConvFileType1Pos(file_type_1);
	comType1->SetSelection(type1pos);
	// all disable
	comType1->Enable(false);
	staType1->Add(comType1, expand);
	gbox->Add(staType1, expand);

	// FILE_TYPE

	wxStaticBoxSizer *staType2 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);

	wxArrayString types2;
	CreateChoiceForAttrDialog(basic, gTypeNameProDOS2, TYPE_NAME_PRODOS_END, types2, file_type_2);

	comType2 = new wxComboBox(parent, IDC_COMBO_TYPE2, wxEmptyString, wxDefaultPosition, wxDefaultSize, types2);
	int type2pos = 0;
	if (type1pos == TYPE_NAME_PRODOS_VOLUME) {
		// all disable
		type2pos = TYPE_NAME_PRODOS_NOT;
		comType2->Enable(false);
		comType2->SetSelection(type2pos);
	} else {
		type2pos = ConvFileType2Pos(file_type_2);
		type2pos = SelectChoiceForAttrDialog(basic, type2pos, TYPE_NAME_PRODOS_END, (int)types2.Count() - 1);
		comType2->SetSelection(type2pos);
	}

	staType2->Add(comType2, expand);
	gbox->Add(staType2, expand);

	sizer->Add(gbox, flags);

	// ACCESS

	wxStaticBoxSizer *staType3 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("Access")), wxVERTICAL);

	wxBoxSizer *hbox = NULL;
	for(int i=0; gTypeNameProDOS3[i].name != NULL; i++) {
		if ((i % 3) == 0) {
			hbox = new wxBoxSizer(wxHORIZONTAL);
			staType3->Add(hbox);
		}
		wxCheckBox *chk = new wxCheckBox(parent, IDC_CHECK_TYPE3 + i, wxGetTranslation(gTypeNameProDOS3[i].name));
		chk->SetValue((file_type_3 & gTypeNameProDOS3[i].value) != 0);
		hbox->Add(chk, flags);
	}
	sizer->Add(staType3, flags);

	// VERSION

	wxSize size;

	AddressValidator validate;
	wxSizerFlags atitle = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);

	gbox = new wxGridSizer(2, 4, 4);

	gbox->Add(new wxStaticText(parent, wxID_ANY, _("Version") + _("(Hex)")), atitle);

	txtType5 = new wxTextCtrl(parent, IDC_TEXT_VERSION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, validate);
	size = txtType5->GetTextExtent(wxString((char)'0', 8));
	size.y = -1;
	txtType5->SetMinSize(size);
	txtType5->SetMaxLength(4);
	txtType5->SetValue(wxString::Format(wxT("%x"), GetVersion()));

	gbox->Add(txtType5);

	// AUX_TYPE

	gbox->Add(new wxStaticText(parent, wxID_ANY, _("Auxiliary Type") + _("(Hex)")), atitle);

	txtType4 = new wxTextCtrl(parent, IDC_TEXT_TYPE4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, validate);
//	size = txtType4->GetTextExtent(wxString((char)'0', 8));
//	size.y = -1;
//	txtType4->SetMinSize(size);
	txtType4->SetMaxLength(4);
	if (type1pos == TYPE_NAME_PRODOS_VOLUME) {
		// all disable
		txtType4->Enable(false);
	} else {
		txtType4->SetValue(wxString::Format(wxT("%x"), GetAuxType()));
	}

	gbox->Add(txtType4);
	sizer->Add(gbox, flags);

	// event handler
	parent->Bind(wxEVT_COMBOBOX, &IntNameBox::OnChangeType1, parent, IDC_COMBO_TYPE2);
}

/// ダイアログ内の値を設定
void DiskBasicDirItemProDOS::InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時 開始アドレスを設定する
		wxComboBox *comType2 = (wxComboBox *)parent->FindWindow(IDC_COMBO_TYPE2);
		if (!comType2) return;

		int type2pos = comType2->GetSelection();
		switch(type2pos) {
		case TYPE_NAME_PRODOS_BAS:
			parent->SetStartAddress(0x801);
			break;
		}
	}
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemProDOS::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxChoice   *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	if (!comType1) return;
	wxComboBox *comType2 = (wxComboBox *)parent->FindWindow(IDC_COMBO_TYPE2);
	if (!comType2) return;

	int type1pos = comType1->GetSelection();
	int type2pos = comType2->GetSelection();

	if (type1pos != TYPE_NAME_PRODOS_VOLUME) {
		if (type2pos == TYPE_NAME_PRODOS_DIR) {
			comType1->SetSelection(TYPE_NAME_PRODOS_SUBDIR);
		} else {
			comType1->SetSelection(TYPE_NAME_PRODOS_FILE);
		}
	}
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemProDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxChoice   *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	wxComboBox *comType2 = (wxComboBox *)parent->FindWindow(IDC_COMBO_TYPE2);
	wxTextCtrl *txtType4 = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_TYPE4);
	wxTextCtrl *txtType5 = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_VERSION);

	int val = 0;
	int ori = 0;

	int type1pos = comType1->GetSelection();
	int type2pos = comType2->GetSelection();

	int type1 = 0;
	int type2 = 0;
	int type3 = 0;
	int type4 = 0;
	int version = 0;

	switch(type1pos) {
	case TYPE_NAME_PRODOS_VOLUME:
		val = FILE_TYPE_VOLUME_MASK;
		if (m_dir_group_num <= 2) {
			type1 = FILETYPE_MASK_PRODOS_VOLUME;
		} else {
			type1 = FILETYPE_MASK_PRODOS_SUBVOL;
		}
		break;
	case TYPE_NAME_PRODOS_SUBDIR:
		val = FILE_TYPE_DIRECTORY_MASK;
		type1 = FILETYPE_MASK_PRODOS_SUBDIR;
		type2 = FILETYPE_MASK_PRODOS_DIR;
		break;
	default:
		type1 = GetFileType1();
		if (type2pos < 0) {
			type2 = Utils::ToInt(comType2->GetValue());
			if (type2 < 0) {
				type2 = GetFileType2();
			}
		} else if (type2pos < TYPE_NAME_PRODOS_END) {
			type2 = gTypeNameProDOS2[type2pos].value;
		} else {
			type2 = CalcSpecialOriginalTypeFromPos(basic, type2pos, TYPE_NAME_PRODOS_END);
			if (type2 < 0) {
				type2 = GetFileType2();
			}
		}
		break;
	}

	// ACCESS
	for(int i=0; gTypeNameProDOS3[i].name != NULL; i++) {
		wxCheckBox *chk = (wxCheckBox *)parent->FindWindow(IDC_CHECK_TYPE3 + i);
		if (chk && chk->GetValue()) {
			type3 |= gTypeNameProDOS3[i].value;
		}
	}

	// AUX_TYPE
	type4 = Utils::ConvFromHexa(txtType4->GetValue());

	// VERSION
	version = Utils::ConvFromHexa(txtType5->GetValue());

	ori = (type1 << 16) | (type2 << 8) | type3;

	attr.SetFileAttr(basic->GetFormatTypeNumber(), val, ori, type4, version);
//	attr.SetUserData(type4, version);

	return true;
}

#if 0
/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemProDOS::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
//	wxFileName fn(filename);
//	if (fn.GetExt().IsEmpty()) {
//		errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILEEXT_EMPTY]);
//		return false;
//	}
	return true;
}

/// その他の属性値を設定する
void DiskBasicDirItemProDOS::SetOptionalAttr(DiskBasicDirItemAttr &attr)
{
//	int type4 = attr.GetUserData(0);
//	int version = attr.GetUserData(1);
//	if (type4 >= 0) {
//		SetAuxType(type4);
//	}
//	if (version >= 0) {
//		SetVersion(version);
//	}
}
#endif

/// プロパティで表示する内部データを設定
void DiskBasicDirItemProDOS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	int stype = GetFileType1();
	vals.Add(wxT("STORAGE_TYPE"), m_data.Data()->stype_and_nlen);
	vals.Add(wxT("FILE_NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("FILE_TYPE"), m_data.Data()->file_type);
	vals.Add(wxT("KEY_POINTER"), m_data.Data()->key_pointer);
	vals.Add(wxT("BLOCKS_USED"), m_data.Data()->blocks_used);
	vals.Add(wxT("EOF"), m_data.Data()->eof, sizeof(m_data.Data()->eof));
	vals.Add(wxT("CREATION_DATE"), m_data.Data()->cdate, sizeof(m_data.Data()->cdate));
	vals.Add(wxT("CREATION_TIME"), m_data.Data()->ctime, sizeof(m_data.Data()->ctime));
	vals.Add(wxT("VERSION"), m_data.Data()->version);
	vals.Add(wxT("MIN_VERSION"), m_data.Data()->min_version);
	vals.Add(wxT("ACCESS"), m_data.Data()->access);
	switch(stype) {
	case FILETYPE_MASK_PRODOS_VOLUME:
		vals.Add(wxT("ENTRY_LENGTH"), m_data.Data()->v.entry_len);
		vals.Add(wxT("ENTRIES_PER_BLOCK"), m_data.Data()->v.entries_per_block);
		vals.Add(wxT("FILE_COUNT"), m_data.Data()->v.file_count);
		vals.Add(wxT("BITMAP_POINTER"), m_data.Data()->v.bitmap_pointer);
		vals.Add(wxT("TOTAL_BLOCKS"), m_data.Data()->v.total_blocks);
		break;
	case FILETYPE_MASK_PRODOS_SUBVOL:
		vals.Add(wxT("ENTRY_LENGTH"), m_data.Data()->sv.entry_len);
		vals.Add(wxT("ENTRIES_PER_BLOCK"), m_data.Data()->sv.entries_per_block);
		vals.Add(wxT("FILE_COUNT"), m_data.Data()->sv.file_count);
		vals.Add(wxT("PARENT_POINTER"), m_data.Data()->sv.parent_pointer);
		vals.Add(wxT("PARENT_ENTRY"), m_data.Data()->sv.parent_entry);
		vals.Add(wxT("PARENT_ENTRY_LENGTH"), m_data.Data()->sv.parent_entry_len);
		break;
	default:
		vals.Add(wxT("AUX_TYPE"), m_data.Data()->f.aux_type);
		vals.Add(wxT("LASTMOD_DATE"), m_data.Data()->f.mdate, sizeof(m_data.Data()->f.mdate));
		vals.Add(wxT("LASTMOD_TIME"), m_data.Data()->f.mtime, sizeof(m_data.Data()->f.mtime));
		vals.Add(wxT("HEADER_POINTER"), m_data.Data()->f.header_pointer);
		break;
	}
}
