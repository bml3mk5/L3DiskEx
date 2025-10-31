/// @file basicdiritem_hfs.cpp
///
/// @brief disk basic directory item for HFS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_hfs.h"
#include <wx/regex.h>
#include "basicfmt.h"
#include "basictype.h"
#include "../config.h"
#include "../charcodes.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// HFS属性名
const name_value_t gTypeNameHFS[] = {
	{ wxTRANSLATE("unknown"), FILETYPE_HFS_INVALID },
	{ wxTRANSLATE("Directory"), FILETYPE_HFS_DIR },
	{ wxTRANSLATE("File - Data"), (FILETYPE_HFS_FILE | FILETYPE_HFS_DATA_FORK << 8) },
	{ wxTRANSLATE("File - Resource"), (FILETYPE_HFS_FILE | FILETYPE_HFS_RES_FORK << 8) },
	{ wxTRANSLATE("Dir Th"), FILETYPE_HFS_DIR_THREAD },
	{ wxTRANSLATE("File Th"), FILETYPE_HFS_FILE_THREAD },
	{ NULL, -1 }
};

#if 0
//////////////////////////////////////////////////////////////////////
//
// HFS トラックセクタリストの各セクタ
//
AppleDOSChains::AppleDOSChains()
	: ArrayOfAppleDOSChain()
{
	chain_ownmake = false;
}

AppleDOSChains::~AppleDOSChains()
{
	if (chain_ownmake) {
		for(size_t i=0; i<Count(); i++) {
			delete Item(i);
		}
	}
}
void AppleDOSChains::Clear()
{
	if (chain_ownmake) {
		for(size_t i=0; i<Count(); i++) {
			delete Item(i);
		}
	}
	ArrayOfAppleDOSChain::Clear();
	chain_ownmake = false;
}
void AppleDOSChains::Alloc()
{
	if (chain_ownmake) {
		for(size_t i=0; i<Count(); i++) {
			delete Item(i);
		}
	}
	ArrayOfAppleDOSChain::Clear();

	hfs_chain_t *newitem = new hfs_chain_t;
	memset(newitem, 0, sizeof(hfs_chain_t));

	ArrayOfAppleDOSChain::Add(newitem);
	chain_ownmake = true;
}
#endif

//////////////////////////////////////////////////////////////////////
//
#if 0
DiskBasicDirDataHFS::DiskBasicDirDataHFS()
{
	m_data.key = NULL;
	m_data.data = NULL;
	m_type = 0;
}

DiskBasicDirDataHFS::~DiskBasicDirDataHFS()
{
}

/// レコードのキー部分のポインタをセット
void DiskBasicDirDataHFS::SetKeyPtr(void *key)
{
	m_data.key = (hfs_cat_key_rec_t *)key;
}

/// レコードのデータ部分のポインタをセット
void DiskBasicDirDataHFS::SetDataPtr(void *data)
{
	m_data.data = (hfs_cat_data_rec_t *)data;
	if (!data) {
		m_type = 0;
		return;
	}

	m_type = m_data.data->recType;
}

/// データを返す
const hfs_catalog_t &DiskBasicDirDataHFS::Data() const
{
	return m_data;
}

/// 有効か
bool DiskBasicDirDataHFS::IsValid() const
{
	return (m_data.key != NULL && m_data.data != NULL);
}

/// 属性１を返す
int DiskBasicDirDataHFS::GetType1() const
{
	return m_type;
}

/// 属性１のセット
void DiskBasicDirDataHFS::SetType1(int val)
{
	m_type = val;
}
#endif

//////////////////////////////////////////////////////////////////////
//
// ディレクトリ１アイテム HFS
//
DiskBasicDirItemHFS::DiskBasicDirItemHFS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_key.Alloc();
	m_data.Alloc();
}
DiskBasicDirItemHFS::DiskBasicDirItemHFS(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
#if 0
	if (!n_data) return;

	m_key.Attach(n_data);
	int rpos = (int)m_key.Data()->keyLength;
	if (rpos <= 37) {
		rpos++;
		if (rpos & 1) {
			// ワード境界に合わせる
			rpos++;
		}
		m_data.Attach(&n_data[rpos]);
	}
#endif
}
DiskBasicDirItemHFS::DiskBasicDirItemHFS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_key.Attach(n_data);
	int rpos = (int)m_key.Data()->keyLength;
	if (rpos <= 37) {
		rpos++;
		if (rpos & 1) {
			// ワード境界に合わせる
			rpos++;
		}
		m_data.Attach(&n_data[rpos]);
	}

	Used(CheckUsed(n_unuse));

	// 上位で行う
//	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemHFS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_key.Attach(n_data);
	int rpos = (int)m_key.Data()->keyLength;
	if (rpos <= 37) {
		rpos++;
		if (rpos & 1) {
			// ワード境界に合わせる
			rpos++;
		}
		m_data.Attach(&n_data[rpos]);
	}
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemHFS::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	if (num == 0) {
		size = len = m_key.Data()->nodeName[0];
		return &m_key.Data()->nodeName[1];
	} else {
		size = len = 0;
		return NULL; 
	}
}

/// ファイル名を設定
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
/// @note filename はデータビットが反転している場合あり
void DiskBasicDirItemHFS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		if (ns > size) ns = size;
		// ファイル名
		for(size_t i=0; i<ns; i++) {
			n[i] = filename[i];
		}
	}
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemHFS::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t s = 0;
	size_t l = 0;

	n = GetFileNamePos(0, s, l);
	if (n != NULL && s > 0) {
		if (s > size) s = size;
		// ファイル名
		for(size_t i=0; i<s; i++) {
			filename[i] = n[i];
		}
	}

	length = l;
}

/// 属性１を返す
int	DiskBasicDirItemHFS::GetFileType1() const
{
	return m_type;
}

/// 属性１を設定
void DiskBasicDirItemHFS::SetFileType1(int val)
{
	m_type = val;
}

/// 使用しているアイテムか
bool DiskBasicDirItemHFS::CheckUsed(bool unuse)
{
	return true;
	//	return !(m_data.Data()->track == 0xff || (m_data.Data()->track == 0 && m_data.Data()->sector == 0));
}

/// アイテムを削除できるか
bool DiskBasicDirItemHFS::IsDeletable() const
{
	// 削除不可
	return false;
}

/// 削除
bool DiskBasicDirItemHFS::Delete()
{
//	// 削除
//	Used(false);
//	// ここで属性は更新しない
	return false;
}

/// アイテムをロード・エクスポートできるか
bool DiskBasicDirItemHFS::IsLoadable() const
{
	return true;
}

/// アイテムをコピー(内部でDnD)できるか
bool DiskBasicDirItemHFS::IsCopyable() const
{
	return false;
}

/// アイテムを上書きできるか
bool DiskBasicDirItemHFS::IsOverWritable() const
{
	return false;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemHFS::Check(bool &last)
{
	return m_key.IsValid() && m_data.IsValid();
}

/// 共通属性を個別属性に変換
int DiskBasicDirItemHFS::ConvToFileType1(int ftype)
{
	int type1 = FILETYPE_HFS_FILE;
	if (ftype & FILE_TYPE_DIRECTORY_MASK) {
		type1 = FILETYPE_HFS_DIR;
	} else if (ftype & FILE_TYPE_RANDOM_MASK) {
		type1 |= (FILETYPE_HFS_RES_FORK << 8);
	}
	return type1;
}

/// 個別属性を共通属性に変換
int DiskBasicDirItemHFS::ConvFromFileType1(int type1)
{
	int val = 0;
	if (type1 == FILETYPE_HFS_DIR) {
		val |= FILE_TYPE_DIRECTORY_MASK;
	} else if (type1 == (FILETYPE_HFS_FILE | (FILETYPE_HFS_RES_FORK << 8))) {
		val |= FILE_TYPE_RANDOM_MASK;
	}
	return val;
}

/// 属性を設定
void DiskBasicDirItemHFS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int type1 = ConvToFileType1(ftype);

	SetFileType1(type1);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemHFS::GetFileAttr() const
{
	int type1 = GetFileType1();
	int val = ConvFromFileType1(type1);
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, type1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemHFS::GetFileAttrStr() const
{
	wxString str;
	int oval = GetFileType1();
	oval = st_name_value::IndexOf(gTypeNameHFS, oval);
	if (oval < 0) oval = 0;
	str += wxGetTranslation(gTypeNameHFS[oval].name);
	return str;
}

/// ファイルサイズをセット
void DiskBasicDirItemHFS::SetFileSize(int val)
{
//	m_groups.SetSize(val);
//	int sec_size = basic->GetSectorSize();
//	val = (val + sec_size - 1) / sec_size;
//	SetSectorCount(val + chain.Count());
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemHFS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	const hfs_cat_data_rec_t *data = m_data.Data();
	if (!data) return;

	switch(m_type) {
	case (FILETYPE_HFS_FILE | (FILETYPE_HFS_DATA_FORK << 8)):
		GetUnitGroupsFile(fileunit_num, FILETYPE_HFS_DATA_FORK, &data->file, &data->file.datF, &data->file.datExts, m_groups);
		break;
	case (FILETYPE_HFS_FILE | (FILETYPE_HFS_RES_FORK << 8)):
		GetUnitGroupsFile(fileunit_num, FILETYPE_HFS_RES_FORK, &data->file, &data->file.resF, &data->file.resExts, m_groups);
		break;
	default:
		break;
	}
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemHFS::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	const hfs_cat_data_rec_t *data = m_data.Data();
	if (!data) return;

	switch(m_type) {
	case FILETYPE_HFS_DIR:
		GetUnitGroupsDir(fileunit_num, &data->dir, group_items);
		break;
	case (FILETYPE_HFS_FILE | (FILETYPE_HFS_DATA_FORK << 8)):
		GetUnitGroupsFile(fileunit_num, FILETYPE_HFS_DATA_FORK, &data->file, &data->file.datF, &data->file.datExts, group_items);
		break;
	case (FILETYPE_HFS_FILE | (FILETYPE_HFS_RES_FORK << 8)):
		GetUnitGroupsFile(fileunit_num, FILETYPE_HFS_RES_FORK, &data->file, &data->file.resF, &data->file.resExts, group_items);
		break;
	default:
		break;
	}
}

/// ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [in]  dir ディレクトリ
/// @param [out] group_items  グループリスト
void DiskBasicDirItemHFS::GetUnitGroupsDir(int fileunit_num, const hfs_cat_dir_rec_t *dir, DiskBasicGroups &group_items)
{
	// ディレクトリの時は常にカタログファイル内
	size_t size = 0;
	int groups = 0;

	int start_sector = basic->GetDirStartSector();
	int end_sector = basic->GetDirEndSector();

	// FixMe: ツリー構造なのでそれをたどったほうが速いと思う
	int trk_num = 0;
	int sid_num = 0;
	int sec_num = 1;
	for(int sec = start_sector; sec < end_sector; sec++) {
		DiskImageSector *sector = basic->GetSectorFromSectorPos(sec, &trk_num, &sid_num);
		if (!sector) {
			break;
		}

		sec_num = sector->GetSectorNumber();

		wxUint8 *buffer = sector->GetSectorBuffer(); 
		if (!buffer) {
			break;
		}
		group_items.Add(0, 0, trk_num, sid_num, sec_num, sec_num);

		size += sector->GetSectorSize();
		groups++;
	}

	group_items.SetSize(size);
	group_items.SetNums(groups);
	group_items.SetSizePerGroup(basic->GetSectorsPerGroup() * basic->GetSectorSize());
}

/// データフォークのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [in]  fork_type フォーク種類 0:データ 0xff:リソース
/// @param [in]  file ファイルデータ
/// @param [in]  fsize フォークサイズ
/// @param [in]  exts フォークセグメントのリスト
/// @param [out] group_items  グループリスト
void DiskBasicDirItemHFS::GetUnitGroupsFile(int fileunit_num, int fork_type, const hfs_cat_file_rec_t *file, const hfs_cat_size_t *fsize, const hfs_ext_data_rec_t *exts, DiskBasicGroups &group_items)
{
	// 拡張データレコードからグループを取得
	GetGroupsFromExtDataRec(exts, group_items);

	if (group_items.GetSize() < (size_t)wxUINT32_SWAP_ON_LE(fsize->PhysicalSize)) {
		// extended overflow file内に続きがある
		GetUnitGroupsFileExt(fileunit_num, fork_type, file, fsize, exts, group_items);
	}

	// ファイルサイズは実際のサイズ
	group_items.SetSize(wxUINT32_SWAP_ON_LE(fsize->LogicalSize));
	group_items.SetSizePerGroup(basic->GetSectorsPerGroup() * basic->GetSectorSize());
}

/// ファイルの拡張オーバフローグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [in]  fork_type フォーク種類 0:データ 0xff:リソース
/// @param [in]  file ファイルデータ
/// @param [in]  fsize フォークサイズ
/// @param [in]  exts フォークセグメントのリスト
/// @param [out] group_items  グループリスト
void DiskBasicDirItemHFS::GetUnitGroupsFileExt(int fileunit_num, int fork_type, const hfs_cat_file_rec_t *file, const hfs_cat_size_t *fsize, const hfs_ext_data_rec_t *exts, DiskBasicGroups &group_items)
{
	// ファイルID
	int file_id = wxUINT32_SWAP_ON_LE(file->id);

	int sta_sec = basic->GetVariousIntegerParam(wxT("ExtStartSector"));
	int end_sec = basic->GetVariousIntegerParam(wxT("ExtEndSector"));
	int end_idx = end_sec + 1 - sta_sec;
	for(int idx = 0; idx < end_idx; idx++) {
		int sec = idx + sta_sec;
		DiskImageSector *sector = basic->GetSectorFromSectorPos(sec);
		if (!sector) {
			break;
		}
		wxUint8 *buffer = sector->GetSectorBuffer();
		if (!buffer) {
			break;
		}

		hfs_node_descriptor_t *node = (hfs_node_descriptor_t *)buffer;
		if (node->type == ndHdrNode) {
			// ヘッダノードは常に最初
			if (idx != 0) {
				break;
			}
			hfs_bt_hdr_rec_t *header = (hfs_bt_hdr_rec_t *)&buffer[0xe];

			end_idx = (int)(wxUINT32_SWAP_ON_LE(header->totalNodes) - wxUINT32_SWAP_ON_LE(header->freeNodes));
			continue;

		} else if (node->type != ndLeafNode) {
			// リーフノード以外は無視する
			continue;
		}

		// リーフノード

		// ノード内のレコード数
		int num_recs = wxUINT16_SWAP_ON_LE(node->numRecs);

		for(int rec = 0; rec < num_recs; rec++) {
			// セクタ末尾のレコードの位置を取得
			wxUint32 rpos = (wxUint32)buffer[510 - 2 * rec] * 256 + buffer[511 - 2 * rec]; 
			if (rpos < 0xe || rpos >= (wxUint32)sector->GetSectorSize()) {
				break;
			}
//			int pos = (int)rpos;

			// レコードのキー部分
			hfs_ext_key_rec_t* rec_key = (hfs_ext_key_rec_t *)&buffer[rpos];
			if ((fork_type & 0xff) != rec_key->forkType || (wxUint32)file_id != wxUINT32_SWAP_ON_LE(rec_key->id)) {
				continue;
			}
			if (rec_key->keyLength > 7) {
				// キー長すぎる
				continue;
			}

			rpos += rec_key->keyLength;
			rpos++;
			if (rpos & 1) {
				// ワード境界に合わせる
				rpos++;
			}
			// レコードのデータ部分
			hfs_ext_data_rec_t *rec_dat = (hfs_ext_data_rec_t *)&buffer[rpos];

			// 拡張レコードからグループを取得
			GetGroupsFromExtDataRec(rec_dat, group_items);
		}
	}
}

/// 拡張レコードからグループを取得
void DiskBasicDirItemHFS::GetGroupsFromExtDataRec(const hfs_ext_data_rec_t *exts, DiskBasicGroups &group_items)
{
	size_t size = group_items.GetSize();
	int groups = group_items.GetNums();
	int sec_per_grp = basic->GetSectorsPerGroup();

	for(int i=0; i<3; i++) {
		wxUint32 start_group = wxUINT16_SWAP_ON_LE(exts->d[i].start);
		wxUint32 count = wxUINT16_SWAP_ON_LE(exts->d[i].count);
		if (!count) break;
		wxUint32 end_group = start_group + count;

		int trk_num = 0;
		int sid_num = 0;
		int sec_num = 1;
		for(wxUint32 grp = start_group; grp < end_group; grp++) {
			DiskImageSector *sector = basic->GetSectorFromGroup(grp, trk_num, sid_num);
			if (!sector) {
				break;
			}
			sec_num = sector->GetSectorNumber();

			wxUint8 *buffer = sector->GetSectorBuffer(); 
			if (!buffer) {
				break;
			}
			group_items.Add(grp, 0, trk_num, sid_num, sec_num, sec_num + sec_per_grp - 1);

			size += sector->GetSectorSize() * sec_per_grp;
			groups++;
		}
	}

	group_items.SetSize(size);
	group_items.SetNums(groups);
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有サイズ
/// @return 計算後のファイルサイズ
int DiskBasicDirItemHFS::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
	if (group_items.Count() == 0) return occupied_size;

	DiskBasicGroupItem *litem = &group_items.Last();
	DiskImageSector *sector = basic->GetSector(litem->track, litem->side, litem->sector_end);
	if (!sector) return occupied_size;

	int sector_size = sector->GetSectorSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	int remain_size = type->CalcDataSizeOnLastSector(this, NULL, NULL, buf, sector_size, sector_size);

	occupied_size = occupied_size - sector_size + remain_size;

	return occupied_size;
}

#if 0
/// ファイル内部のアドレスを取り出す
void DiskBasicDirItemHFS::TakeAddressesInFile(DiskBasicGroups &group_items)
{
	m_start_address = -1;
	m_data_length = -1;

	if (group_items.Count() == 0) {
		return;
	}

	DiskBasicGroupItem *item = &group_items.Item(0);
	DiskImageSector *sector = basic->GetSector(item->track, item->side, item->sector_start);
	if (!sector) return;

	int t1 = GetFileType1();

	if (t1 & FILETYPE_MASK_HFS_BINARY) {
		// バイナリ
		// 開始アドレス
		m_start_address = (int)sector->Get16(0);
		// データサイズ
		m_data_length = (int)sector->Get16(2);
		// 実際のサイズを設定
		if (m_data_length + 5 <= (int)group_items.GetSize()) group_items.SetSize(m_data_length + 5);
	}
	else if (t1 & (FILETYPE_MASK_HFS_IBASIC | FILETYPE_MASK_HFS_ABASIC)) {
		// BASICファイルサイズ
		// データサイズ => 最終データ位置みたい
		m_data_length = (int)sector->Get16(0);
		// 実際のサイズを設定
		if (m_data_length + 3 <= (int)group_items.GetSize()) group_items.SetSize(m_data_length + 3);
	}
}
#endif

/// 最初のグループ番号を設定
void DiskBasicDirItemHFS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
//	int track_num = 0;
//	int sector_num = 0;
//	type->GetNumFromSectorPosS(val, track_num, sector_num);
//	m_data.Data()->track = track_num - basic->GetTrackNumberBaseOnDisk();
	//m_data.Data()->sector = sector_num - basic->GetSectorNumberBase();
}

/// 最初のグループ番号を返す IDを返す
wxUint32 DiskBasicDirItemHFS::GetStartGroup(int fileunit_num) const
{
	wxUint32 val = 0;

	const hfs_cat_data_rec_t *data = m_data.Data();
	if (!data) {
		return val;
	}

	switch(m_type) {
	case FILETYPE_HFS_DIR:
		val = (0x80000000 | (wxUINT32_SWAP_ON_LE(data->dir.id)));
		break;
	case (FILETYPE_HFS_FILE | (FILETYPE_HFS_DATA_FORK << 8)):
		val = wxUINT16_SWAP_ON_LE(data->file.datExts.d[0].start);
		break;
	case (FILETYPE_HFS_FILE | (FILETYPE_HFS_RES_FORK << 8)):
		val = wxUINT16_SWAP_ON_LE(data->file.resExts.d[0].start);
		break;
	default:
		break;
	}

	return val;
}

#if 0
/// 最後のグループ番号をセット
void DiskBasicDirItemHFS::SetLastGroup(wxUint32 val)
{
	int trk_num = 0;
	int sec_num = 0;
	type->GetNumFromSectorPosS(val, trk_num, sec_num);
}

/// 最後のグループ番号を返す
wxUint32 DiskBasicDirItemHFS::GetLastGroup() const
{
	wxUint32 val = (wxUint32)type->GetSectorPosFromNumS(0, 0);
	return val;
}
#endif

/// 追加のグループ番号を返す(機種依存)
wxUint32 DiskBasicDirItemHFS::GetExtraGroup() const
{
	return GetStartGroup(0);
}

/// 追加のグループ番号を得る(機種依存)
void DiskBasicDirItemHFS::GetExtraGroups(wxArrayInt &arr) const
{
//	int gnum = (int)GetExtraGroup();
//	for(int i=0; i<chain.Count(); i++) {
//		arr.Add(gnum);
//		gnum = chain.GetNext(i);
//		if (gnum == 0) break;
//	}
}

/// チェイン用のセクタをクリア(機種依存)
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemHFS::ClearChainSector(const DiskBasicDirItem *pitem)
{
//	chain.Clear();
//	chain.SetBasic(basic);
}

/// チェイン用のセクタをセット
/// @param [in] sector セクタ
/// @param [in] gnum   グループ番号
/// @param [in] data   セクタ内のバッファ
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemHFS::SetChainSector(DiskImageSector *sector, wxUint32 gnum, wxUint8 *data, const DiskBasicDirItem *pitem)
{
//	chain.Add((hfs_chain_t *)data);
//	if (chain.Count() > 1) {
//		int i = (int)chain.Count() - 2;
//		chain.SetNext(i, gnum);
//	}
}

/// チェイン用のセクタにグループ番号をセット(機種依存)
void DiskBasicDirItemHFS::AddChainGroupNumber(int idx, wxUint32 val)
{
//	int track_num = 0;
//	int sector_num = 0;
//	type->GetNumFromSectorPosS(val, track_num, sector_num);
//	chain.SetTrackAndSector(idx, track_num, sector_num);
}

/// セクタカウントを返す(機種依存)
void DiskBasicDirItemHFS::SetSectorCount(wxUint16 val)
{
//	m_data.Data()->sector_count = wxUINT16_SWAP_ON_BE(val);
}

/// セクタカウントを返す(機種依存)
/// @note セクタカウントはトラックセクタリストで占有しているセクタ数も含んでいる
wxUint16 DiskBasicDirItemHFS::GetSectorCount() const
{
	return 0;
//	return wxUINT16_SWAP_ON_BE(m_data.Data()->sector_count);
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemHFS::NeedCheckEofCode()
{
	return ((GetFileType1() & 0x7f) == 0);
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemHFS::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	if (NeedCheckEofCode()) {
		// ファイルの最終が終端記号で終わっているかを調べる
		// ただし、ファイルサイズがセクタサイズで割り切れるなら終端記号は不要
		if ((file_size % basic->GetSectorSize()) != 0) {
			file_size = CheckEofCode(istream, file_size);
			file_size--;
		}
	}
	return file_size;
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemHFS::GetDataSize() const
{
	return sizeof(directory_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemHFS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemHFS::CopyData(const directory_t *val)
{
	return false;
//	return m_data.Copy(val, GetDataSize());
}

/// ディレクトリをクリア
void DiskBasicDirItemHFS::ClearData()
{
//	m_data.Fill(basic->GetDeleteCode(), GetDataSize());
}

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemHFS::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimExtensionByExtensionAttr(filename);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// 最初のトラック番号をセット
void DiskBasicDirItemHFS::SetStartTrack(wxUint8 val)
{
//	m_data.Data()->track = val;
}
/// 最初のセクタ番号をセット
void DiskBasicDirItemHFS::SetStartSector(wxUint8 val)
{
//	m_data.Data()->sector = val;
}
/// 最初のトラック番号を返す
wxUint8 DiskBasicDirItemHFS::GetStartTrack() const
{
//	return m_data.Data()->track;
	return 0;
}
/// 最初のセクタ番号を返す
wxUint8 DiskBasicDirItemHFS::GetStartSector() const
{
//	return m_data.Data()->sector;
	return 0;
}
#if 0
/// 最後のトラック番号をセット
void DiskBasicDirItemHFS::SetLastTrack(wxUint8 val)
{
}
/// 最後のセクタ番号をセット
void DiskBasicDirItemHFS::SetLastSector(wxUint8 val)
{
}
/// 最後のトラック番号を返す
wxUint8 DiskBasicDirItemHFS::GetLastTrack() const
{
	return 0;
}
/// 最後のセクタ番号を返す
wxUint8 DiskBasicDirItemHFS::GetLastSector() const
{
	return 0;
}

/// 開始アドレスを返す
int DiskBasicDirItemHFS::GetStartAddress() const
{
	return m_start_address;
}

/// 終了アドレスを返す
int DiskBasicDirItemHFS::GetEndAddress() const
{
	return (m_start_address >= 0 && m_data_length > 0) ? m_start_address + m_data_length - 1 : -1;
}
#endif

/// 作成日付を得る
/// @param [out] tm 日付
void DiskBasicDirItemHFS::GetFileCreateDate(TM &tm) const
{
	switch(m_type & 0xff) {
	case FILETYPE_HFS_DIR:
		ConvHFSDateToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->dir.createDate), tm);
		break;
	case FILETYPE_HFS_FILE:
		ConvHFSDateToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->file.createDate), tm);
		break;
	default:
		break;
	}
}

/// 作成時間を得る
/// @param [out] tm 時間
void DiskBasicDirItemHFS::GetFileCreateTime(TM &tm) const
{
	switch(m_type & 0xff) {
	case FILETYPE_HFS_DIR:
		ConvHFSTimeToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->dir.createDate), tm);
		break;
	case FILETYPE_HFS_FILE:
		ConvHFSTimeToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->file.createDate), tm);
		break;
	default:
		break;
	}
}

/// 作成日時を得る
/// @param [out] tm 日時
void DiskBasicDirItemHFS::GetFileCreateDateTime(TM &tm) const
{
	GetFileCreateDate(tm);
	GetFileCreateTime(tm);
}

/// 作成日時を返す
TM DiskBasicDirItemHFS::GetFileCreateDateTime() const
{
	TM tm;
	GetFileCreateDateTime(tm);
	return tm;
}

#if 0
/// 作成日時をセット
/// @param [in] tm 日時
void DiskBasicDirItemHFS::SetFileCreateDateTime(const TM &tm)
{
	SetFileCreateDate(tm);
	SetFileCreateTime(tm);
}

/// 作成日付のタイトル名（ダイアログ用）
/// @return タイトル文字列
wxString DiskBasicDirItemHFS::GetFileCreateDateTimeTitle() const
{
	return _("Created Date");
}
#endif

/// ファイルの作成日付を文字列にして返す
/// @return 日付文字列
wxString DiskBasicDirItemHFS::GetFileCreateDateStr() const
{
	TM tm;
	GetFileCreateDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// ファイルの作成時間を文字列にして返す
/// @return 時間文字列
wxString DiskBasicDirItemHFS::GetFileCreateTimeStr() const
{
	TM tm;
	GetFileCreateTime(tm);
	return Utils::FormatHMSStr(tm);
}

#if 0
/// ファイルの作成日時を文字列にして返す
/// @return 日時文字列 ない場合"---"
wxString DiskBasicDirItemHFS::GetFileCreateDateTimeStr() const
{
	wxString str = GetFileCreateDateStr();
	if (!str.IsEmpty()) str += wxT(" ");
	str += GetFileCreateTimeStr();
	if (str.IsEmpty()) str += wxT("---");
	return str;
}
#endif

/// 変更日付を得る
/// @param [out] tm 日付
void DiskBasicDirItemHFS::GetFileModifyDate(TM &tm) const
{
	switch(m_type & 0xff) {
	case FILETYPE_HFS_DIR:
		ConvHFSDateToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->dir.modifyDate), tm);
		break;
	case FILETYPE_HFS_FILE:
		ConvHFSDateToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->file.modifyDate), tm);
		break;
	default:
		break;
	}
}

/// 変更時間を得る
/// @param [out] tm 時間
void DiskBasicDirItemHFS::GetFileModifyTime(TM &tm) const
{
	switch(m_type & 0xff) {
	case FILETYPE_HFS_DIR:
		ConvHFSTimeToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->dir.modifyDate), tm);
		break;
	case FILETYPE_HFS_FILE:
		ConvHFSTimeToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->file.modifyDate), tm);
		break;
	default:
		break;
	}
}

/// 変更日時を得る
/// @param [out] tm 日時
void DiskBasicDirItemHFS::GetFileModifyDateTime(TM &tm) const
{
	GetFileModifyDate(tm);
	GetFileModifyTime(tm);
}

/// 変更日時を返す
TM DiskBasicDirItemHFS::GetFileModifyDateTime() const
{
	TM tm;
	GetFileModifyDateTime(tm);
	return tm;
}

#if 0
/// 変更日時をセット
/// @param [in] tm 日時
void DiskBasicDirItemHFS::SetFileModifyDateTime(const TM &tm)
{
	SetFileModifyDate(tm);
	SetFileModifyTime(tm);
}

/// 変更日付のタイトル名（ダイアログ用）
/// @return タイトル文字列
wxString DiskBasicDirItemHFS::GetFileModifyDateTimeTitle() const
{
	return _("Modified Date");
}
#endif

/// ファイルの変更日付を文字列にして返す
/// @return 日付文字列
wxString DiskBasicDirItemHFS::GetFileModifyDateStr() const
{
	TM tm;
	GetFileModifyDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// ファイルの変更時間を文字列にして返す
/// @return 時間文字列
wxString DiskBasicDirItemHFS::GetFileModifyTimeStr() const
{
	TM tm;
	GetFileModifyTime(tm);
	return Utils::FormatHMSStr(tm);
}

#if 0
/// ファイルの変更日時を文字列にして返す
/// @return 日時文字列 ない場合"---"
wxString DiskBasicDirItemHFS::GetFileModifyDateTimeStr() const
{
	wxString str = GetFileModifyDateStr();
	if (!str.IsEmpty()) str += wxT(" ");
	str += GetFileModifyTimeStr();
	if (str.IsEmpty()) str += wxT("---");
	return str;
}
#endif

/// アクセス日付を得る
/// @param [out] tm 日付
void DiskBasicDirItemHFS::GetFileAccessDate(TM &tm) const
{
	switch(m_type & 0xff) {
	case FILETYPE_HFS_DIR:
		ConvHFSDateToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->dir.backupDate), tm);
		break;
	case FILETYPE_HFS_FILE:
		ConvHFSDateToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->file.backupDate), tm);
		break;
	default:
		break;
	}
}

/// アクセス時間を得る
/// @param [out] tm 時間
void DiskBasicDirItemHFS::GetFileAccessTime(TM &tm) const
{
	switch(m_type & 0xff) {
	case FILETYPE_HFS_DIR:
		ConvHFSTimeToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->dir.backupDate), tm);
		break;
	case FILETYPE_HFS_FILE:
		ConvHFSTimeToTm(wxUINT32_SWAP_ON_LE(m_data.Data()->file.backupDate), tm);
		break;
	default:
		break;
	}
}

/// アクセス日時を得る
/// @param [out] tm 日時
void DiskBasicDirItemHFS::GetFileAccessDateTime(TM &tm) const
{
	GetFileAccessDate(tm);
	GetFileAccessTime(tm);
}

/// アクセス日時を返す
TM DiskBasicDirItemHFS::GetFileAccessDateTime() const
{
	TM tm;
	GetFileAccessDateTime(tm);
	return tm;
}

/// アクセス日付のタイトル名（ダイアログ用）
/// @return タイトル文字列
wxString DiskBasicDirItemHFS::GetFileAccessDateTimeTitle() const
{
	return _("Backup Date");
}

/// ファイルのアクセス日付を文字列にして返す
/// @return 日付文字列
wxString DiskBasicDirItemHFS::GetFileAccessDateStr() const
{
	TM tm;
	GetFileAccessDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// ファイルのアクセス時間を文字列にして返す
/// @return 時間文字列
wxString DiskBasicDirItemHFS::GetFileAccessTimeStr() const
{
	TM tm;
	GetFileAccessTime(tm);
	return Utils::FormatHMSStr(tm);
}

/// HFS日付をTMに変換
bool DiskBasicDirItemHFS::ConvHFSDateToTm(wxUint32 src, TM &dst)
{
	// 1904/01/01 00:00:00からの秒数
	int dday = (src / 60 / 60 / 24);
	int dyear;
	int dmonth = 0;
	bool decided = false;
	for(dyear = 1904; dyear <= 9999; dyear++) {
		for(dmonth = wxDateTime::Jan; dmonth <= wxDateTime::Dec; dmonth++) {
			int days_of_month = (int)wxDateTime::GetNumberOfDays((wxDateTime::Month)dmonth, dyear);
			if (dday < days_of_month) {
				decided = true;
				break;
			}
			dday -= days_of_month;
		}
		if (decided) {
			break;
		}
	}
	dst.SetYear(dyear - 1900);
	dst.SetMonth(dmonth);
	dst.SetDay(dday + 1);
	return true;
}

/// HFS時間をTMに変換
bool DiskBasicDirItemHFS::ConvHFSTimeToTm(wxUint32 src, TM &dst)
{
	// 1904/01/01 00:00:00からの秒数
	int ss = src % 60;
	int mi = (src / 60) % 60;
	int hh = (src / 60 / 60) % 24;
	dst.SetHour(hh);
	dst.SetMinute(mi);
	dst.SetSecond(ss);
	return true;
}

/// TMをHFS日時に変換
bool DiskBasicDirItemHFS::ConvTmToHFSDateTime(const TM &src, wxUint32 &dst)
{
	// 1904/01/01 00:00:00からの秒数
	int ddays = 0;
	int syear = src.GetYear() + 1900;
	int smonth = src.GetMonth() - 1;
	for(int dyear = 1904; dyear <= syear; dyear++) {
		int emonth = wxDateTime::Dec;
		if (dyear == syear) {
			emonth = smonth;
		}
		for(int dmonth = wxDateTime::Jan; dmonth <= emonth; dmonth++) {
			int days_of_month = (int)wxDateTime::GetNumberOfDays((wxDateTime::Month)dmonth, dyear);
			ddays += days_of_month;
		}
	}
	ddays += src.GetDay() - 1;

	dst = ddays * 60 * 60 * 24 + src.GetHour() * 60 * 60 + src.GetMonth() * 60 + src.GetSecond();

	return true;
}

//
// ダイアログ用
//

#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "../ui/intnamebox.h"

#define IDC_RADIO_TYPE1	51
#define IDC_CHECK_READONLY 52

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemHFS::ConvFileType1Pos(int type1) const
{
	int p1 = TYPE_NAME_HFS_FILE_DATA;
	if (type1 == FILETYPE_HFS_DIR) {
		p1 = TYPE_NAME_HFS_DIR;
	} else if (type1 == (FILETYPE_HFS_FILE | (FILETYPE_HFS_RES_FORK << 8))) {
		p1 = TYPE_NAME_HFS_FILE_RES;
	}
	return p1;
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemHFS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		wxFileName fn(name);
		const MyAttribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
		if (sa) {
			int ftype = sa->GetType();
			file_type_1 = ConvToFileType1(ftype);
		} else {
			file_type_1 = 0;
		}
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemHFS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1();
	wxRadioBox *radType1;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1);

	wxArrayString types1;
	for(size_t i=1; i<=3; i++) {
		types1.Add(wxGetTranslation(gTypeNameHFS[i].name));
	}
	radType1 = new wxRadioBox(parent, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 2, wxRA_SPECIFY_COLS);
	radType1->SetSelection(ConvFileType1Pos(file_type_1));
	sizer->Add(radType1, flags);

	radType1->Enable(false);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemHFS::ChangeTypeInAttrDialog(IntNameBox *parent)
{
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemHFS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	return false;
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemHFS::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	return true;
}

/// ファイルサイズが適正か
/// @param [in]  parent     ダイアログ
/// @param [in]  size       ファイルサイズ
/// @param [out] limit      制限サイズ
/// @return true 適正
bool DiskBasicDirItemHFS::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	int limit_size = 1 * basic->GetSectorSize() - 1;
	if (limit) *limit = limit_size;
	return limit_size >= size;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemHFS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	const hfs_cat_key_rec_t *key = m_key.Data();
	if (!key) return;

	vals.Add(wxT("ckrKeyLen"), key->keyLength);
	vals.Add(wxT("ckrParID"), wxUINT32_SWAP_ON_LE(key->parentID));
	vals.Add(wxT("ckrCName"), key->nodeName, key->keyLength - 5);

	const hfs_cat_data_rec_t *data = m_data.Data();
	if (!data) return;

	vals.Add(wxT("cdrType"), data->recType);

	switch(data->recType) {
	case FILETYPE_HFS_DIR:
		vals.Add(wxT("dirFlags"), wxUINT16_SWAP_ON_LE(data->dir.flags));
		vals.Add(wxT("dirVal"), wxUINT16_SWAP_ON_LE(data->dir.valence));
		vals.Add(wxT("dirDirID"), wxUINT32_SWAP_ON_LE(data->dir.id));
		vals.Add(wxT("dirCrDat"), wxUINT32_SWAP_ON_LE(data->dir.createDate));
		vals.Add(wxT("dirMdDat"), wxUINT32_SWAP_ON_LE(data->dir.modifyDate));
		vals.Add(wxT("dirBkDat"), wxUINT32_SWAP_ON_LE(data->dir.backupDate));
		vals.Add(wxT("dirUsrInfo"), &data->dir.UsrInfo, sizeof(data->dir.UsrInfo));
		vals.Add(wxT("dirFndrInfo"), &data->dir.finderInfo, sizeof(data->dir.finderInfo));
		vals.Add(wxT("dirResrv"), &data->dir.reserved, sizeof(data->dir.reserved));
	case FILETYPE_HFS_FILE:
		vals.Add(wxT("filFlags"), data->file.flags);
		vals.Add(wxT("filTyp"), data->file.fileType);
		vals.Add(wxT("filUsrWds"), &data->file.userInfo, sizeof(data->file.userInfo));
		vals.Add(wxT("filFlNum"), wxUINT32_SWAP_ON_LE(data->file.id));
		vals.Add(wxT("filStBlk"), wxUINT16_SWAP_ON_LE(data->file.datF.StartBlock));
		vals.Add(wxT("filLgLen"), wxUINT32_SWAP_ON_LE(data->file.datF.LogicalSize));
		vals.Add(wxT("filPyLen"), wxUINT32_SWAP_ON_LE(data->file.datF.PhysicalSize));
		vals.Add(wxT("filRStBlk"), wxUINT16_SWAP_ON_LE(data->file.resF.StartBlock));
		vals.Add(wxT("filRLgLen"), wxUINT32_SWAP_ON_LE(data->file.resF.LogicalSize));
		vals.Add(wxT("filRPyLen"), wxUINT32_SWAP_ON_LE(data->file.resF.PhysicalSize));
		vals.Add(wxT("filCrDat"), wxUINT32_SWAP_ON_LE(data->file.createDate));
		vals.Add(wxT("filMdDat"), wxUINT32_SWAP_ON_LE(data->file.modifyDate));
		vals.Add(wxT("filBkDat"), wxUINT32_SWAP_ON_LE(data->file.backupDate));
		vals.Add(wxT("filFndrInfo"), &data->file.finderInfo, sizeof(data->file.finderInfo));
		vals.Add(wxT("filClpSize"), wxUINT16_SWAP_ON_LE(data->file.clumpSize));
		vals.Add(wxT("filExtRec"), &data->file.datExts, sizeof(data->file.datExts));
		vals.Add(wxT("filRExtRec"), &data->file.resExts, sizeof(data->file.resExts));
		vals.Add(wxT("filResrv"), &data->file.reserved, sizeof(data->file.reserved));
		break;
	case FILETYPE_HFS_DIR_THREAD:
		vals.Add(wxT("thdParID"), wxUINT32_SWAP_ON_LE(data->thread.parentID));
		vals.Add(wxT("thdCName"), data->thread.nodeName, sizeof(data->thread.nodeName));
		break;
	case FILETYPE_HFS_FILE_THREAD:
		vals.Add(wxT("fthdParID"), wxUINT32_SWAP_ON_LE(data->thread.parentID));
		vals.Add(wxT("fthdCName"), data->thread.nodeName, sizeof(data->thread.nodeName));
		break;
	default:
		break;
	}
}
