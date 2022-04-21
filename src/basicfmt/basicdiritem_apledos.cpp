/// @file basicdiritem_apledos.cpp
///
/// @brief disk basic directory item for Apple DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_apledos.h"
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

/// Apple DOS属性名
const name_value_t gTypeNameAppleDOS[] = {
	{ wxTRANSLATE("Text"), FILETYPE_MASK_APLEDOS_TEXT },
	{ wxTRANSLATE("Integer BASIC"), FILETYPE_MASK_APLEDOS_IBASIC },
	{ wxTRANSLATE("Applesoft BASIC"), FILETYPE_MASK_APLEDOS_ABASIC },
	{ wxTRANSLATE("Binary"), FILETYPE_MASK_APLEDOS_BINARY },
	{ wxTRANSLATE("Read Only"), FILETYPE_MASK_APLEDOS_READ_ONLY },
	{ NULL, -1 }
};

//////////////////////////////////////////////////////////////////////
//
// Apple DOS トラックセクタリストの各セクタ
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

	apledos_chain_t *newitem = new apledos_chain_t;
	memset(newitem, 0, sizeof(apledos_chain_t));

	ArrayOfAppleDOSChain::Add(newitem);
	chain_ownmake = true;
}

//////////////////////////////////////////////////////////////////////
//
// Apple DOS トラックセクタリスト
//
DiskBasicDirItemAppleDOSChain::DiskBasicDirItemAppleDOSChain()
{
	basic = NULL;
}
DiskBasicDirItemAppleDOSChain::~DiskBasicDirItemAppleDOSChain()
{
}
#ifdef COPYABLE_DIRITEM
/// 代入
DiskBasicDirItemAppleDOSChain &DiskBasicDirItemAppleDOSChain::operator=(const DiskBasicDirItemAppleDOSChain &src)
{
	this->Dup(src);
	return *this;
}
/// 複製
void DiskBasicDirItemAppleDOSChain::Dup(const DiskBasicDirItemAppleDOSChain &src)
{
	sector = src.sector;
	if (src.chain_ownmake) {
		chain = new apledos_chain_t;
		memcpy(&chain, src.chain, sizeof(apledos_chain_t));
	} else {
		chain = src.chain;
	}
	chain_ownmake = src.chain_ownmake;
}
#endif
/// BASICをセット
void DiskBasicDirItemAppleDOSChain::SetBasic(DiskBasic *n_basic)
{
	basic = n_basic;
}
/// ポインタをセット
void DiskBasicDirItemAppleDOSChain::Add(apledos_chain_t *n_chain)
{
	chains.Add(n_chain);
}
/// メモリ確保
void DiskBasicDirItemAppleDOSChain::Alloc()
{
	chains.Alloc();
}
/// クリア
void DiskBasicDirItemAppleDOSChain::Clear()
{
	chains.Clear();
}
/// セクタ数を返す
int DiskBasicDirItemAppleDOSChain::Count() const
{
	return (int)chains.Count();
}
/// トラック＆セクタを返す
void DiskBasicDirItemAppleDOSChain::GetTrackAndSector(int idx, int &track, int &sector) const
{
	int max_idx = APLEDOS_TRACK_LIST_MAX;
	for(size_t i=0; i<chains.Count(); i++) {
		if (idx < max_idx) {
			apledos_chain_t *item = chains.Item(i);
			track = item->list[idx].track;
			sector = item->list[idx].sector;
			track += basic->GetTrackNumberBase();
			sector += basic->GetSectorNumberBase();
			break;
		}
		idx -= max_idx;
	}
}
/// トラック＆セクタを設定
void DiskBasicDirItemAppleDOSChain::SetTrackAndSector(int idx, int track, int sector)
{
	int max_idx = APLEDOS_TRACK_LIST_MAX;
	for(size_t i=0; i<chains.Count(); i++) {
		if (idx < max_idx) {
			apledos_chain_t *item = chains.Item(i);
			track -= basic->GetTrackNumberBase();
			sector -= basic->GetSectorNumberBase();
			item->list[idx].track = (wxUint8)track;
			item->list[idx].sector = (wxUint8)sector;
			break;
		}
		idx -= max_idx;
	}
}
/// 次のセクタのあるセクタ番号を得る
int DiskBasicDirItemAppleDOSChain::GetNext(int idx) const
{
	apledos_chain_t *item = chains.Item(idx);
	return item->next.next_track * basic->GetSectorsPerTrackOnBasic() + item->next.next_sector;
}
/// 次のセクタのあるセクタ番号を設定
void DiskBasicDirItemAppleDOSChain::SetNext(int idx, int val)
{
	apledos_chain_t *item = chains.Item(idx);
	item->next.next_track = (wxUint8)(val / basic->GetSectorsPerTrackOnBasic());
	item->next.next_sector = (wxUint8)(val % basic->GetSectorsPerTrackOnBasic());
}

//////////////////////////////////////////////////////////////////////
//
// ディレクトリ１アイテム Apple DOS 3.x
//
DiskBasicDirItemAppleDOS::DiskBasicDirItemAppleDOS(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_start_address = -1;
	m_data_length = -1;

	m_data.Alloc();
	chain.Alloc();
}
DiskBasicDirItemAppleDOS::DiskBasicDirItemAppleDOS(DiskBasic *basic, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_start_address = -1;
	m_data_length = -1;

	m_data.Attach(n_data);
}
DiskBasicDirItemAppleDOS::DiskBasicDirItemAppleDOS(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_start_address = -1;
	m_data_length = -1;

	m_data.Attach(n_data);

	Used(CheckUsed(n_unuse));

	// チェインセクタへのポインタをセット
	if (IsUsed()) {
		chain.Clear();
		chain.SetBasic(basic);
		wxUint32 grp = GetStartGroup(0);
		while (grp != 0) {
			DiskD88Sector *sector = basic->GetSectorFromGroup(grp);
			if (!sector) break;

			wxUint8 *buf = sector->GetSectorBuffer();
			chain.Add((apledos_chain_t *)buf);
			apledos_ptr_t *p = (apledos_ptr_t *)buf;
			grp = type->GetSectorPosFromNumS(p->next_track + basic->GetTrackNumberBase(), p->next_sector + basic->GetSectorNumberBase());
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
void DiskBasicDirItemAppleDOS::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskD88Sector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemAppleDOS::GetFileNamePos(int num, size_t &size, size_t &len) const
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
/// @note filename はデータビットが反転している場合あり
void DiskBasicDirItemAppleDOS::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		if (ns > size) ns = size;
		// ファイル名はMSBをセット
		for(size_t i=0; i<ns; i++) {
			n[i] = filename[i] | 0x80;
		}
	}
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemAppleDOS::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t s = 0;
	size_t l = 0;

	n = GetFileNamePos(0, s, l);
	if (n != NULL && s > 0) {
		if (s > size) s = size;
		// ファイル名はMSBをはずす
		for(size_t i=0; i<s; i++) {
			filename[i] = n[i] & 0x7f;
		}
	}

	length = l;
}

/// 属性１を返す
int	DiskBasicDirItemAppleDOS::GetFileType1() const
{
	return m_data.Data()->type;
}

/// 属性１を設定
void DiskBasicDirItemAppleDOS::SetFileType1(int val)
{
	m_data.Data()->type = (val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemAppleDOS::CheckUsed(bool unuse)
{
	return !(m_data.Data()->track == 0xff || (m_data.Data()->track == 0 && m_data.Data()->sector == 0));
}

/// 削除
bool DiskBasicDirItemAppleDOS::Delete()
{
	// 削除
	Used(false);
	// ここで属性は更新しない
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemAppleDOS::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;

	if (m_data.Data()->track == 0 && m_data.Data()->sector == 0) {
		last = true;
		return valid;
	}
	// 属性 3-6bitはゼロ
	if (m_data.Data()->type & 0x78) {
		valid = false;
	}
	return valid;
}

/// 共通属性を個別属性に変換
int DiskBasicDirItemAppleDOS::ConvToFileType1(int ftype)
{
	int type1 = 0;
	if ((ftype & (FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK | FILE_TYPE_INTEGER_MASK)) == (FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK)) {
		type1 = FILETYPE_MASK_APLEDOS_ABASIC;
	} else if ((ftype & (FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK | FILE_TYPE_INTEGER_MASK)) == (FILE_TYPE_BASIC_MASK | FILE_TYPE_INTEGER_MASK)) {
		type1 = FILETYPE_MASK_APLEDOS_IBASIC;
	} else if (ftype & FILE_TYPE_BINARY_MASK) {
		type1 = FILETYPE_MASK_APLEDOS_BINARY;
	} else {
		type1 = FILETYPE_MASK_APLEDOS_TEXT;
	}
	if (ftype & FILE_TYPE_READONLY_MASK) {
		type1 |= FILETYPE_MASK_APLEDOS_READ_ONLY;
	}
	return type1;
}

/// 個別属性を共通属性に変換
int DiskBasicDirItemAppleDOS::ConvFromFileType1(int type1)
{
	int val = 0;
	if (type1 & FILETYPE_MASK_APLEDOS_ABASIC) {
		val = (FILE_TYPE_BINARY_MASK | FILE_TYPE_BASIC_MASK);
	} else if (type1 & FILETYPE_MASK_APLEDOS_IBASIC) {
		val = (FILE_TYPE_INTEGER_MASK | FILE_TYPE_BASIC_MASK);
	} else if (type1 & FILETYPE_MASK_APLEDOS_BINARY) {
		val = (FILE_TYPE_BINARY_MASK | FILE_TYPE_MACHINE_MASK);
	} else {
		val = (FILE_TYPE_ASCII_MASK | FILE_TYPE_DATA_MASK);
	}
	if (type1 & FILETYPE_MASK_APLEDOS_READ_ONLY) {
		val |= FILE_TYPE_READONLY_MASK;
	}
	return val;
}

/// 属性を設定
void DiskBasicDirItemAppleDOS::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int type1 = ConvToFileType1(ftype);

	SetFileType1(type1);
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemAppleDOS::GetFileAttr() const
{
	int type1 = GetFileType1();
	int val = ConvFromFileType1(type1);
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, type1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemAppleDOS::GetFileAttrStr() const
{
	wxString str;
	int oval = GetFileType1();
	if (oval & FILETYPE_MASK_APLEDOS_IBASIC) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameAppleDOS[TYPE_NAME_APLEDOS_IBASIC].name);
	} else if (oval & FILETYPE_MASK_APLEDOS_ABASIC) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameAppleDOS[TYPE_NAME_APLEDOS_ABASIC].name);
	} else if (oval & FILETYPE_MASK_APLEDOS_BINARY) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameAppleDOS[TYPE_NAME_APLEDOS_BINARY].name);
	} else {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameAppleDOS[TYPE_NAME_APLEDOS_TEXT].name);
	}
	if (oval & FILETYPE_MASK_APLEDOS_READ_ONLY) {
		if (!str.IsEmpty()) str += wxT(", ");
		str += wxGetTranslation(gTypeNameAppleDOS[TYPE_NAME_APLEDOS_READ_ONLY].name);
	}
	return str;
}

/// ファイルサイズをセット
void DiskBasicDirItemAppleDOS::SetFileSize(int val)
{
	m_groups.SetSize(val);
	int sec_size = basic->GetSectorSize();
	val = (val + sec_size - 1) / sec_size;
	SetSectorCount(val + chain.Count());
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemAppleDOS::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemAppleDOS::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
	if (!chain.IsValid()) return;

	int calc_groups = 0;
	int calc_file_size = 0;

	for(int i=0; ; i++) {
		int track_num = 0;
		int sector_num = 0;
		chain.GetTrackAndSector(i, track_num, sector_num);
		if (track_num == 0 && sector_num == 0) {
			break;
		}
		int group_num = type->GetSectorPosFromNumS(track_num, sector_num);
		int side_num = 0;
		type->GetNumFromSectorPos(group_num, track_num, side_num, sector_num);
		group_items.Add(group_num, 0, track_num, side_num, sector_num, sector_num);
		calc_groups++;
		calc_file_size += basic->GetSectorSize();
		if (calc_groups >= (int)basic->GetFatEndGroup()) {
			// too large block size
			break;
		}
	}
	calc_groups += (int)chain.Count();
	if (GetSectorCount() != calc_groups) {
		calc_groups = GetSectorCount();
		calc_file_size = calc_groups * basic->GetSectorSize();
	}
	group_items.SetNums(calc_groups);
	group_items.SetSize(calc_file_size);
	group_items.SetSizePerGroup(basic->GetSectorSize());

	// 最終セクタの再計算
	group_items.SetSize(RecalcFileSize(group_items, (int)group_items.GetSize()));

	// ファイル内部のアドレスを得る
	TakeAddressesInFile(group_items);
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有サイズ
/// @return 計算後のファイルサイズ
int DiskBasicDirItemAppleDOS::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
	if (group_items.Count() == 0) return occupied_size;

	DiskBasicGroupItem *litem = &group_items.Last();
	DiskD88Sector *sector = basic->GetSector(litem->track, litem->side, litem->sector_end);
	if (!sector) return occupied_size;

	int sector_size = sector->GetSectorSize();
	wxUint8 *buf = sector->GetSectorBuffer();
	int remain_size = type->CalcDataSizeOnLastSector(this, NULL, NULL, buf, sector_size, sector_size);

	occupied_size = occupied_size - sector_size + remain_size;

	return occupied_size;
}

/// ファイル内部のアドレスを取り出す
void DiskBasicDirItemAppleDOS::TakeAddressesInFile(DiskBasicGroups &group_items)
{
	m_start_address = -1;
	m_data_length = -1;

	if (group_items.Count() == 0) {
		return;
	}

	DiskBasicGroupItem *item = &group_items.Item(0);
	DiskD88Sector *sector = basic->GetSector(item->track, item->side, item->sector_start);
	if (!sector) return;

	int t1 = GetFileType1();

	if (t1 & FILETYPE_MASK_APLEDOS_BINARY) {
		// バイナリ
		// 開始アドレス
		m_start_address = (int)sector->Get16(0);
		// データサイズ
		m_data_length = (int)sector->Get16(2);
		// 実際のサイズを設定
		if (m_data_length + 5 <= (int)group_items.GetSize()) group_items.SetSize(m_data_length + 5);
	}
	else if (t1 & (FILETYPE_MASK_APLEDOS_IBASIC | FILETYPE_MASK_APLEDOS_ABASIC)) {
		// BASICファイルサイズ
		// データサイズ => 最終データ位置みたい
		m_data_length = (int)sector->Get16(0);
		// 実際のサイズを設定
		if (m_data_length + 3 <= (int)group_items.GetSize()) group_items.SetSize(m_data_length + 3);
	}
}

/// 最初のグループ番号を設定
void DiskBasicDirItemAppleDOS::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	int track_num = 0;
	int sector_num = 0;
	type->GetNumFromSectorPosS(val, track_num, sector_num);
	m_data.Data()->track = track_num - basic->GetTrackNumberBase();
	m_data.Data()->sector = sector_num - basic->GetSectorNumberBase();
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemAppleDOS::GetStartGroup(int fileunit_num) const
{
	wxUint32 val = (wxUint32)type->GetSectorPosFromNumS(m_data.Data()->track + basic->GetTrackNumberBase(), m_data.Data()->sector + basic->GetSectorNumberBase());
	return val;
}

#if 0
/// 最後のグループ番号をセット
void DiskBasicDirItemAppleDOS::SetLastGroup(wxUint32 val)
{
	int trk_num = 0;
	int sec_num = 0;
	type->GetNumFromSectorPosS(val, trk_num, sec_num);
}

/// 最後のグループ番号を返す
wxUint32 DiskBasicDirItemAppleDOS::GetLastGroup() const
{
	wxUint32 val = (wxUint32)type->GetSectorPosFromNumS(0, 0);
	return val;
}
#endif

/// 追加のグループ番号を返す(機種依存)
wxUint32 DiskBasicDirItemAppleDOS::GetExtraGroup() const
{
	return GetStartGroup(0);
}

/// 追加のグループ番号を得る(機種依存)
void DiskBasicDirItemAppleDOS::GetExtraGroups(wxArrayInt &arr) const
{
	int gnum = (int)GetExtraGroup();
	for(int i=0; i<chain.Count(); i++) {
		arr.Add(gnum);
		gnum = chain.GetNext(i);
		if (gnum == 0) break;
	}
}

/// チェイン用のセクタをクリア(機種依存)
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemAppleDOS::ClearChainSector(const DiskBasicDirItem *pitem)
{
	chain.Clear();
	chain.SetBasic(basic);
}

/// チェイン用のセクタをセット
/// @param [in] sector セクタ
/// @param [in] gnum   グループ番号
/// @param [in] data   セクタ内のバッファ
/// @param [in] pitem  コピー元のアイテム
void DiskBasicDirItemAppleDOS::SetChainSector(DiskD88Sector *sector, wxUint32 gnum, wxUint8 *data, const DiskBasicDirItem *pitem)
{
	chain.Add((apledos_chain_t *)data);
	if (chain.Count() > 1) {
		int i = (int)chain.Count() - 2;
		chain.SetNext(i, gnum);
	}
}

/// チェイン用のセクタにグループ番号をセット(機種依存)
void DiskBasicDirItemAppleDOS::AddChainGroupNumber(int idx, wxUint32 val)
{
	int track_num = 0;
	int sector_num = 0;
	type->GetNumFromSectorPosS(val, track_num, sector_num);
	chain.SetTrackAndSector(idx, track_num, sector_num);
}

/// セクタカウントを返す(機種依存)
void DiskBasicDirItemAppleDOS::SetSectorCount(wxUint16 val)
{
	m_data.Data()->sector_count = wxUINT16_SWAP_ON_BE(val);
}

/// セクタカウントを返す(機種依存)
/// @note セクタカウントはトラックセクタリストで占有しているセクタ数も含んでいる
wxUint16 DiskBasicDirItemAppleDOS::GetSectorCount() const
{
	return wxUINT16_SWAP_ON_BE(m_data.Data()->sector_count);
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemAppleDOS::NeedCheckEofCode()
{
	return ((GetFileType1() & 0x7f) == 0);
}

/// セーブ時にファイルサイズを再計算する ファイルの終端コードが必要な場合
int DiskBasicDirItemAppleDOS::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
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
size_t DiskBasicDirItemAppleDOS::GetDataSize() const
{
	return sizeof(directory_apledos_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemAppleDOS::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemAppleDOS::CopyData(const directory_t *val)
{
	return m_data.Copy(val, GetDataSize());
}

/// ディレクトリをクリア
void DiskBasicDirItemAppleDOS::ClearData()
{
	m_data.Fill(basic->GetDeleteCode(), GetDataSize());
}

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemAppleDOS::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimExtensionByExtensionAttr(filename);
	}
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// アイテムを削除できるか
bool DiskBasicDirItemAppleDOS::IsDeletable() const
{
	return true;
}

/// 最初のトラック番号をセット
void DiskBasicDirItemAppleDOS::SetStartTrack(wxUint8 val)
{
	m_data.Data()->track = val;
}
/// 最初のセクタ番号をセット
void DiskBasicDirItemAppleDOS::SetStartSector(wxUint8 val)
{
	m_data.Data()->sector = val;
}
/// 最初のトラック番号を返す
wxUint8 DiskBasicDirItemAppleDOS::GetStartTrack() const
{
	return m_data.Data()->track;
}
/// 最初のセクタ番号を返す
wxUint8 DiskBasicDirItemAppleDOS::GetStartSector() const
{
	return m_data.Data()->sector;
}
#if 0
/// 最後のトラック番号をセット
void DiskBasicDirItemAppleDOS::SetLastTrack(wxUint8 val)
{
}
/// 最後のセクタ番号をセット
void DiskBasicDirItemAppleDOS::SetLastSector(wxUint8 val)
{
}
/// 最後のトラック番号を返す
wxUint8 DiskBasicDirItemAppleDOS::GetLastTrack() const
{
	return 0;
}
/// 最後のセクタ番号を返す
wxUint8 DiskBasicDirItemAppleDOS::GetLastSector() const
{
	return 0;
}
#endif

/// 開始アドレスを返す
int DiskBasicDirItemAppleDOS::GetStartAddress() const
{
	return m_start_address;
}

/// 終了アドレスを返す
int DiskBasicDirItemAppleDOS::GetEndAddress() const
{
	return (m_start_address >= 0 && m_data_length > 0) ? m_start_address + m_data_length - 1 : -1;
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
int DiskBasicDirItemAppleDOS::ConvFileType1Pos(int type1) const
{
	int p1 = 0;
	if ((type1 & 0x7f) == FILETYPE_MASK_APLEDOS_IBASIC) {
		p1 = TYPE_NAME_APLEDOS_IBASIC;
	} else if ((type1 & 0x7f) == FILETYPE_MASK_APLEDOS_ABASIC) {
		p1 = TYPE_NAME_APLEDOS_ABASIC;
	} else if ((type1 & 0x7f) == FILETYPE_MASK_APLEDOS_BINARY) {
		p1 = TYPE_NAME_APLEDOS_BINARY;
	} else {
		p1 = TYPE_NAME_APLEDOS_TEXT;
	}
	return p1;
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemAppleDOS::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		wxFileName fn(name);
		const L3Attribute *sa = basic->GetAttributesByExtension().FindUpperCase(fn.GetExt());
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
void DiskBasicDirItemAppleDOS::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1();
	wxRadioBox *radType1;
	wxCheckBox *chkReadOnly;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1);

	wxArrayString types1;
	for(size_t i=0; i<TYPE_NAME_APLEDOS_READ_ONLY; i++) {
		types1.Add(wxGetTranslation(gTypeNameAppleDOS[i].name));
	}
	radType1 = new wxRadioBox(parent, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 2, wxRA_SPECIFY_COLS);
	radType1->SetSelection(ConvFileType1Pos(file_type_1));
	sizer->Add(radType1, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);

	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, wxGetTranslation(gTypeNameAppleDOS[TYPE_NAME_APLEDOS_READ_ONLY].name));
	chkReadOnly->SetValue((file_type_1 & FILETYPE_MASK_APLEDOS_READ_ONLY) != 0);
	staType4->Add(chkReadOnly, flags);

	sizer->Add(staType4, flags);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemAppleDOS::ChangeTypeInAttrDialog(IntNameBox *parent)
{
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemAppleDOS::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);

	int val = 0;
	int ori = 0;

	switch(radType1->GetSelection()) {
	case TYPE_NAME_APLEDOS_IBASIC:
		val = (FILE_TYPE_INTEGER_MASK | FILE_TYPE_BASIC_MASK);
		ori = FILETYPE_MASK_APLEDOS_IBASIC;
		break;
	case TYPE_NAME_APLEDOS_ABASIC:
		val = (FILE_TYPE_BINARY_MASK | FILE_TYPE_BASIC_MASK);
		ori = FILETYPE_MASK_APLEDOS_ABASIC;
		break;
	case TYPE_NAME_APLEDOS_BINARY:
		val = (FILE_TYPE_BINARY_MASK | FILE_TYPE_MACHINE_MASK);
		ori = FILETYPE_MASK_APLEDOS_BINARY;
		break;
	default:
		val = (FILE_TYPE_ASCII_MASK | FILE_TYPE_DATA_MASK);
		ori = FILETYPE_MASK_APLEDOS_TEXT;
		break;
	}
	if (chkReadOnly->GetValue()) {
		val |= FILE_TYPE_READONLY_MASK;
		ori |= FILETYPE_MASK_APLEDOS_READ_ONLY;
	}

	attr.SetFileAttr(basic->GetFormatTypeNumber(), val, ori);

	return true;
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemAppleDOS::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
//	wxFileName fn(filename);
//	if (fn.GetExt().IsEmpty()) {
//		errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILEEXT_EMPTY]);
//		return false;
//	}
	return true;
}

/// ファイルサイズが適正か
/// @param [in]  parent     ダイアログ
/// @param [in]  size       ファイルサイズ
/// @param [out] limit      制限サイズ
/// @return true 適正
bool DiskBasicDirItemAppleDOS::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	int limit_size = APLEDOS_TRACK_LIST_MAX * basic->GetSectorSize() - 1;
	if (limit) *limit = limit_size;
	return limit_size >= size;
}

/// プロパティで表示する内部データを設定
/// @param[in,out] vals 名前＆値のリスト
void DiskBasicDirItemAppleDOS::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("TRACK"), m_data.Data()->track);
	vals.Add(wxT("SECTOR"), m_data.Data()->sector);
	vals.Add(wxT("TYPE"), m_data.Data()->type);
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("SECTOR_COUNT"), m_data.Data()->sector_count);
}
