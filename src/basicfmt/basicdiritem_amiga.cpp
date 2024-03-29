/// @file basicdiritem_amiga.cpp
///
/// @brief disk basic directory item for Amiga DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_amiga.h"
//#include <wx/regex.h>
#include <wx/datetime.h>
#include "basicfmt.h"
#include "basictype.h"
#include "../config.h"
#include "../charcodes.h"
#include "../utils.h"


//////////////////////////////////////////////////////////////////////
//
//
//

/// Amiga属性名１
const name_value_t gTypeNameAmiga1[] = {
	{ "File", FILETYPE_MASK_AMIGA_FILE },
	{ "Dir", FILETYPE_MASK_AMIGA_USERDIR },
	{ "L.F.", FILETYPE_MASK_AMIGA_LINKFILE },
	{ "L.D.", FILETYPE_MASK_AMIGA_LINKDIR },
	{ "S.L.", FILETYPE_MASK_AMIGA_SOFTLINK },
	{ NULL, -1 }
};

/// Amiga属性名１ プロパティ用
const char *gTypeNameAmiga1l[] = {
	wxTRANSLATE("File"),
	wxTRANSLATE("Directory"),
	wxTRANSLATE("Link File"),
	wxTRANSLATE("Link Directory"),
	wxTRANSLATE("Soft Link"),
	NULL
};

/// Amiga属性１変換テーブル
const value_value_t gTypeConvAmiga1[] = {
	{ FILE_TYPE_DIRECTORY_MASK, FILETYPE_MASK_AMIGA_ROOT },
	{ FILE_TYPE_DATA_MASK, FILETYPE_MASK_AMIGA_FILE },
	{ FILE_TYPE_DIRECTORY_MASK, FILETYPE_MASK_AMIGA_USERDIR },
	{ FILE_TYPE_HARDLINK_MASK | FILE_TYPE_DATA_MASK, FILETYPE_MASK_AMIGA_LINKFILE },
	{ FILE_TYPE_HARDLINK_MASK | FILE_TYPE_DIRECTORY_MASK, FILETYPE_MASK_AMIGA_LINKDIR },
	{ FILE_TYPE_SOFTLINK_MASK, FILETYPE_MASK_AMIGA_SOFTLINK },
	{ -1, -1 }
};

/// Amiga属性名２ リスト用
const char gTypeNameAmiga2[] = "dewrapsh";

/// Amiga属性名２ プロパティ用
const char *gTypeNameAmiga2p[] = {
	wxTRANSLATE("Deletable"),
	wxTRANSLATE("Executable"),
	wxTRANSLATE("Writable"),
	wxTRANSLATE("Readable"),
	wxTRANSLATE("Archive"),
	wxTRANSLATE("Pure"),
	wxTRANSLATE("Script"),
	wxTRANSLATE("Hold"),
	wxTRANSLATE("Set UID"),
	NULL
};

/// Amiga属性変換
const wxUint32 gTypeConvAmiga2[] = {
	FILETYPE_MASK_AMIGA_U_NDEL,
	FILETYPE_MASK_AMIGA_U_NEXEC,
	FILETYPE_MASK_AMIGA_U_NWRITE,
	FILETYPE_MASK_AMIGA_U_NREAD,
	FILETYPE_MASK_AMIGA_ARCHIVE,
	FILETYPE_MASK_AMIGA_PURE,
	FILETYPE_MASK_AMIGA_SCRIPT,
	FILETYPE_MASK_AMIGA_HOLD,
	FILETYPE_MASK_AMIGA_G_NDEL,
	FILETYPE_MASK_AMIGA_G_NEXEC,
	FILETYPE_MASK_AMIGA_G_NWRITE,
	FILETYPE_MASK_AMIGA_G_NREAD,
	FILETYPE_MASK_AMIGA_O_NDEL,
	FILETYPE_MASK_AMIGA_O_NEXEC,
	FILETYPE_MASK_AMIGA_O_NWRITE,
	FILETYPE_MASK_AMIGA_O_NREAD,
	FILETYPE_MASK_AMIGA_SETUID,
	(const wxUint32)-1
};

//////////////////////////////////////////////////////////////////////
//
// ユーザデータに渡すチェイン情報
//
AmigaChain::AmigaChain()
	: DiskBasicGroupUserData()
{
	m_idx = 0;
	p_prev_chain = NULL;
	p_next_chain = NULL;
}
AmigaChain::AmigaChain(int idx, wxUint32 *prev_chain, wxUint32 *next_chain)
{
	m_idx = idx;
	p_prev_chain = prev_chain;
	p_next_chain = next_chain;
}
AmigaChain &AmigaChain::operator=(const DiskBasicGroupUserData &src)
{
	m_idx = ((const AmigaChain &)src).m_idx;
	p_prev_chain = ((const AmigaChain &)src).p_prev_chain;
	p_next_chain = ((const AmigaChain &)src).p_next_chain;
	return *this;
}

//////////////////////////////////////////////////////////////////////
//
// ディレクトリ１アイテム Amiga DOS
//
DiskBasicDirItemAmiga::DiskBasicDirItemAmiga(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_temp_pre = NULL;
	m_temp_post = NULL;

	AllocData(NULL, NULL);
	AllocTemp();
}
DiskBasicDirItemAmiga::DiskBasicDirItemAmiga(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_temp_pre = NULL;
	m_temp_post = NULL;

	AllocData(n_sector, n_data);
}
/// @param [in]  basic      DISK BASIC パラメータ
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [in]  n_next     次のセクタ
/// @param [out] n_unuse    未使用か
DiskBasicDirItemAmiga::DiskBasicDirItemAmiga(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_temp_pre = NULL;
	m_temp_post = NULL;
	if (n_gitem) m_chain = *n_gitem->user_data;

	AllocData(n_sector, n_data);

	Used(CheckUsed(n_unuse));

	CalcFileSize();
}
DiskBasicDirItemAmiga::~DiskBasicDirItemAmiga()
{
	DeleteTemp();
}

/// バッファをアロケート
void DiskBasicDirItemAmiga::AllocTemp()
{
	m_temp_pre = new amiga_block_pre_t;
	memset(m_temp_pre, 0, sizeof(amiga_block_pre_t));
	m_temp_post = new amiga_block_post_t;
	memset(m_temp_post, 0, sizeof(amiga_block_post_t));

	m_data.Data()->pre = m_temp_pre;
	m_data.Data()->post = m_temp_post;
}

/// バッファを削除
void DiskBasicDirItemAmiga::DeleteTemp()
{
	delete m_temp_pre;
	delete m_temp_post;
	m_temp_pre = NULL;
	m_temp_post = NULL;
}

/// ディレクトリ情報をアロケート
void DiskBasicDirItemAmiga::AllocData(DiskImageSector *n_sector, wxUint8 *n_data)
{
	m_data.Alloc();

	if (n_sector && n_data) {
		int num = type->GetSectorPosFromNum(n_sector->GetIDC(), n_sector->GetIDH(), n_sector->GetIDR());
		m_data.Data()->block_num = (wxUint32)num;
		m_data.Data()->pre = (amiga_block_pre_t *)n_data;
		m_data.Data()->post = (amiga_block_post_t *)(n_data + n_sector->GetSectorSize() - sizeof(amiga_block_post_t));
	} else {
		m_data.Data()->block_num = 0;
		m_data.Data()->pre = NULL;
		m_data.Data()->post = NULL;
	}
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemAmiga::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	if (n_gitem) m_chain = *n_gitem->user_data;
	AllocData(n_sector, n_data);
}

/// header用にセクタを初期化
void DiskBasicDirItemAmiga::InitForHeaderBlock(wxUint32 parent_num)
{
	amiga_block_pre_t *pre = (amiga_block_pre_t *)m_data.Data()->pre;
	if (!pre) return;

	pre->type = wxUINT32_SWAP_ON_LE(FILETYPE_MASK_AMIGA_HEADER);
	pre->header_key = wxUINT32_SWAP_ON_LE(m_data.Data()->block_num);

	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (!post) return;

	post->parent_dir = wxUINT32_SWAP_ON_LE(parent_num);
//	wxUint32 sec_type = FILETYPE_MASK_AMIGA_FILE;
//	post->sec_type = wxUINT32_SWAP_ON_LE(sec_type);
}

/// extension用にセクタを初期化
void DiskBasicDirItemAmiga::InitForExtensionBlock(wxUint32 parent_num)
{
	amiga_block_pre_t *pre = (amiga_block_pre_t *)m_data.Data()->pre;
	if (!pre) return;

	pre->type = wxUINT32_SWAP_ON_LE(FILETYPE_MASK_AMIGA_LIST);
	pre->header_key = wxUINT32_SWAP_ON_LE(m_data.Data()->block_num);

	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (!post) return;

	post->parent_dir = wxUINT32_SWAP_ON_LE(parent_num);
	post->sec_type = wxUINT32_SWAP_ON_LE(FILETYPE_MASK_AMIGA_FILE);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemAmiga::GetFileNamePos(int num, size_t &size, size_t &len) const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post && num == 0) {
		size = len = sizeof(post->name);
		len--;
		return post->name;
	}
	size = len = 0;
	return NULL; 
}

/// ファイル名を設定
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [in]     length   長さ
void DiskBasicDirItemAmiga::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		memcpy(n, filename, ns);
		amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
		if (post) post->name_len = ((length - 1) & 0xff);
	}
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemAmiga::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
{
	wxUint8 *n = NULL;
	size_t s = 0;
	size_t l = 0;

	n = GetFileNamePos(0, s, l);
	if (n != NULL && s > 0) {
		if (s > size) s = size;
		memcpy(filename, n, s);
	}

	length = l;
}

/// ファイル名＋拡張子のサイズ
int DiskBasicDirItemAmiga::GetFileNameStrSize() const
{
	size_t s = 0;
	size_t l = 0;
	GetFileNamePos(0, s, l);

	return (int)l;
}

/// 小文字を大文字にする
/// @param [in,out] str  文字列
/// @param [in]     size 長さ
void DiskBasicDirItemAmiga::ToUpper(wxUint8 *str, size_t size) const
{
	bool is_intr = basic->GetVariousBoolParam(wxT(KEY_INTERNATIONAL));
	for(size_t i=0; i<size; i++) {
		str[i] = Upper(str[i], is_intr);
	}
}

/// 大文字にする
/// @param [in] ch      文字
/// @param [in] is_intr インターナショナルか
/// @return 変換後文字
wxUint8 DiskBasicDirItemAmiga::Upper(wxUint8 ch, bool is_intr)
{
	// a -> A
	if (ch >= 0x61 && ch <= 0x7a) {
		ch -= 0x20;
	}
	// i18n (iso-8859-1)
	if (is_intr) {
		if (ch >= 0xe0 && ch <= 0xfe && ch != 0xf7) {
			ch -= 0x20;
		}
	}
	return ch;
}

/// 属性１を返す sec_type
int	DiskBasicDirItemAmiga::GetFileType1() const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) return wxUINT32_SWAP_ON_LE(post->sec_type);
	return 0;
}

/// 属性１を設定 sec_type
void DiskBasicDirItemAmiga::SetFileType1(int val)
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) post->sec_type = wxUINT32_SWAP_ON_LE(val);
}

/// 属性２を返す protect
int	DiskBasicDirItemAmiga::GetFileType2() const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) return wxUINT32_SWAP_ON_LE(post->protect);
	return 0;
}

/// 属性２を設定 protect
void DiskBasicDirItemAmiga::SetFileType2(int val)
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) post->protect = wxUINT32_SWAP_ON_LE(val);
}

/// 属性３を返す gid uid
int DiskBasicDirItemAmiga::GetFileType3() const
{
	return (int)(GetGroupID() << 16 | GetUserID());
}

/// 属性３のセット gid uid
void DiskBasicDirItemAmiga::SetFileType3(int val)
{
	SetGroupID((wxUint16)(val >> 16));
	SetUserID((wxUint16)(val & 0xffff));
}

/// @brief ファイルサイズを返す
wxUint32 DiskBasicDirItemAmiga::GetByteSize() const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) return wxUINT32_SWAP_ON_LE(post->byte_size);
	return 0;
}

#if 0
/// ブロック数をセット
void DiskBasicDirItemAmiga::SetBlocks(wxUint16 val)
{
	m_data.Data()->num_of_blocks = wxUINT16_SWAP_ON_BE(val);
}

/// ブロック数を返す
wxUint16 DiskBasicDirItemAmiga::GetBlocks() const
{
	return wxUINT16_SWAP_ON_BE(m_data.Data()->num_of_blocks);
}
#endif

/// ストアしたブロック数をセット
void DiskBasicDirItemAmiga::SetHighSeq(wxUint32 val)
{
	amiga_block_pre_t *pre = (amiga_block_pre_t *)m_data.Data()->pre;
	if (pre) pre->high_seq = wxUINT32_SWAP_ON_LE(val);
}

/// ブロックテーブルを返す
wxUint32 *DiskBasicDirItemAmiga::GetBlockTable() const
{
	amiga_block_pre_t *pre = (amiga_block_pre_t *)m_data.Data()->pre;
	return pre ? pre->u.table : NULL;
}

/// ブロック番号を返す
wxUint32 DiskBasicDirItemAmiga::GetDataBlock(int idx) const
{
	amiga_block_pre_t *pre = (amiga_block_pre_t *)m_data.Data()->pre;
	return pre ? wxUINT32_SWAP_ON_LE(pre->u.table[idx]) : 0;
}

/// ブロック番号をセット
void DiskBasicDirItemAmiga::SetDataBlock(int idx, wxUint32 val)
{
	amiga_block_pre_t *pre = (amiga_block_pre_t *)m_data.Data()->pre;
	if (pre) pre->u.table[idx] = wxUINT32_SWAP_ON_LE(val);
}

/// 1ヘッダで格納できるブロック数を返す
int DiskBasicDirItemAmiga::GetDataBlockNums() const
{
	return (basic->GetSectorSize() - sizeof(amiga_block_pre_t) - sizeof(amiga_header_post_t)) / 4 + 1;
}

/// ハッシュテーブルにセット（必要ならチェインをたどる）ディレクトリ用
bool DiskBasicDirItemAmiga::ChainHashNumber(int idx, wxUint32 val, DiskBasicDirItem *item)
{
	DiskBasicDirItemAmiga *aitem = (DiskBasicDirItemAmiga *)item;
	aitem->m_chain.m_idx = idx;
	amiga_header_post_t *post = &aitem->m_data.Data()->post->h;
	if (post) {
		// 次のチェインが入ったポインタ
		aitem->m_chain.p_next_chain = &post->hash_chain;
	}

	wxUint32 num = GetDataBlock(idx);
	if (num < 2) {
		// ディレクトリのhash_tableにセット
		SetDataBlock(idx, val);
		amiga_block_pre_t *pre = m_data.Data()->pre;
		if (pre) {
			aitem->m_chain.p_prev_chain = &pre->u.table[idx];
		}
		return true;
	}
	// チェインをたどる
	int limit = basic->GetFatEndGroup() + 1;
	amiga_hash_chain_t *chain = NULL;
	while(num >= 2 && limit >= 0) {
		// セクタを得る
		DiskImageSector *sector = basic->GetSectorFromGroup(num);
		if (!sector) return false;
		// 次のヘッダブロックがあるか
		int offset = sector->GetSectorSize() - (int)sizeof(amiga_hash_chain_t);
		chain = (amiga_hash_chain_t *)sector->GetSectorBuffer(offset);
		if (!chain) return false;
		num = wxUINT32_SWAP_ON_LE(chain->hash_chain);
		limit--;
	}
	if (limit >= 0) {
		chain->hash_chain = wxUINT32_SWAP_ON_LE(val);
		aitem->m_chain.p_prev_chain = &chain->hash_chain;
		return true;
	}
	return false;
}

/// ハッシュ番号を返す
int DiskBasicDirItemAmiga::GetHashNumber() const
{
	return m_chain.m_idx;
}

/// エクステンションをセット
void DiskBasicDirItemAmiga::SetExtension(wxUint32 val)
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) post->extension = wxUINT32_SWAP_ON_LE(val);
}

/// エクステンションを返す
wxUint32 DiskBasicDirItemAmiga::GetExtension() const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) return wxUINT32_SWAP_ON_LE(post->extension);
	return 0;
}

/// ユーザＩＤを返す
wxUint16 DiskBasicDirItemAmiga::GetUserID() const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) return wxUINT32_SWAP_ON_LE(post->uid);
	return 0;
}

/// ユーザＩＤをセット
void DiskBasicDirItemAmiga::SetUserID(wxUint16 val)
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) post->uid = wxUINT16_SWAP_ON_LE(val);
}

/// グループＩＤを返す
wxUint16 DiskBasicDirItemAmiga::GetGroupID() const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) return wxUINT32_SWAP_ON_LE(post->gid);
	return 0;
}

/// グループＩＤをセット
void DiskBasicDirItemAmiga::SetGroupID(wxUint16 val)
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) post->gid = wxUINT16_SWAP_ON_LE(val);
}

/// 使用しているアイテムか
bool DiskBasicDirItemAmiga::CheckUsed(bool unuse)
{
	// エントリはリンク形式なので常に有効
	return true;
}

/// アイテムを削除できるか
bool DiskBasicDirItemAmiga::IsDeletable() const
{
//	// ハードリンクは削除不可とする
//	int t1 = GetFileType1();
//	if (t1 == FILETYPE_MASK_AMIGA_LINKFILE || t1 == FILETYPE_MASK_AMIGA_LINKDIR) {
//		return false;
//	}
	// ハードリンクがあるファイルやディレクトリは削除不可とする
	amiga_header_post_t *post = &m_data.Data()->post->h;
	wxUint32 val = (post ? wxUINT32_SWAP_ON_LE(post->next_link) : 0);
	if (val > 0) {
		return false;
	}
	return true;
}

/// 削除
bool DiskBasicDirItemAmiga::Delete()
{
	// extensionブロックを未使用にする
	// ディレクトリの場合、directory cacheを未使用にする
	for(size_t i=0; i<m_extension_list.Count(); i++) {
		wxUint32 num = m_extension_list.Item(i);
		type->DeleteGroupNumber(num);
	}
	// ハードリンクの場合、リンクのつなぎ替えを行う
	DeleteHardLink();

	// 自分を未使用にする
	type->DeleteGroupNumber(m_data.Data()->block_num);

	Used(false);
	return true;
}

/// ハードリンクを削除
bool DiskBasicDirItemAmiga::DeleteHardLink()
{
	// ハードリンクの場合、リンクのつなぎ替えを行う
	int t1 = GetFileType1();
	if (t1 != FILETYPE_MASK_AMIGA_LINKFILE && t1 != FILETYPE_MASK_AMIGA_LINKDIR) {
		return true;
	}
	// real
	wxUint32 my_block_num = GetStartGroup(0);
	amiga_header_post_t *prev_post = NULL;
	amiga_header_post_t *post = &m_data.Data()->post->h;
	wxUint32 block_num = wxUINT32_SWAP_ON_LE(post->real_entry);
	while (block_num >= 2) {
		// next link
		DiskImageSector *sector = basic->GetSectorFromGroup(block_num);
		int npos = sector->GetSectorSize() - (int)sizeof(amiga_header_post_t);
		post = (amiga_header_post_t *)sector->GetSectorBuffer(npos);

		if (prev_post && block_num == my_block_num) {
			prev_post->next_link = post->next_link;
			break;
		}
		block_num = wxUINT32_SWAP_ON_LE(post->next_link);
		prev_post = post;
	}
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemAmiga::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;

	return valid;
}

/// 属性を設定
void DiskBasicDirItemAmiga::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	if (file_type.GetFormat()== basic->GetFormatTypeNumber()) {
		// 同じOS
		SetFileType1(file_type.GetOrigin(0));
		SetFileType2(file_type.GetOrigin(1));
		SetFileType3(file_type.GetOrigin(2));
	} else {
		// 違うOSから
		int t1 = ConvToFileType1(ftype);
		SetFileType1(t1);
	}
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemAmiga::GetFileAttr() const
{
	int t1 = GetFileType1();
	int t2 = GetFileType2();
	int t3 = GetFileType3();
	int val = ConvFromFileType1(t1);
	val |= ConvFromFileType2(t2);
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, t1, t2, t3);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemAmiga::GetFileAttrStr() const
{
	wxString str;
	int spos = ConvFileType1Pos(GetFileType1());
	if (spos >= 0) {
		str = gTypeNameAmiga1[spos].name;
	} else {
		str = wxT("???");
	}
	str += wxT(" ,");
	// パーミッション
	int type2 = GetFileType2();
	for(int i = 7; i >= 0; i--) {
		bool high = (i >= 4);
		bool bset = (((wxUint32)type2 & gTypeConvAmiga2[i]) != 0);
		if (bset == high) {
			str += gTypeNameAmiga2[i];
		} else {
			str += wxT("-");
		}
	}
	return str;
}

/// 共通属性を個別属性に変換
int DiskBasicDirItemAmiga::ConvToFileType1(int ftype)
{
	int t1 = 0;
	if (ftype & FILE_TYPE_DIRECTORY_MASK) {
		t1 = FILETYPE_MASK_AMIGA_USERDIR;
	} else {
		t1 = FILETYPE_MASK_AMIGA_FILE;
	}
	return t1;
}

/// 個別属性を共通属性に変換
int DiskBasicDirItemAmiga::ConvFromFileType1(int type1)
{
	int val = 0;
	for(int i=0; gTypeConvAmiga1[i].ori_value != -1; i++) {
		if (type1 == gTypeConvAmiga1[i].ori_value) {
			val = gTypeConvAmiga1[i].com_value;
			break;
		}
	}
	return val;
}

/// 個別属性を共通属性に変換
int DiskBasicDirItemAmiga::ConvFromFileType2(int type2)
{
	int val = 0;
	if (type2 & FILETYPE_MASK_AMIGA_U_NDEL) {
		val |= FILE_TYPE_UNDELETE_MASK;
	} else if (type2 & FILETYPE_MASK_AMIGA_U_NWRITE) {
		val |= FILE_TYPE_READONLY_MASK;
	} else if (type2 & FILETYPE_MASK_AMIGA_U_NREAD) {
		val |= FILE_TYPE_WRITEONLY_MASK;
	}
	return val;
}

/// 属性２からリストの位置を返す
int DiskBasicDirItemAmiga::ConvFileType1Pos(int type1) const
{
	return gTypeNameAmiga1[0].IndexOf(gTypeNameAmiga1, type1);
}

/// ファイルサイズをセット
void DiskBasicDirItemAmiga::SetFileSize(int val)
{
	m_groups.SetSize(val);
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) post->byte_size = wxUINT32_SWAP_ON_LE(val);
}

/// ファイルサイズを返す
int DiskBasicDirItemAmiga::GetFileSize() const
{
	int val = 0;
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (post) val = wxUINT32_SWAP_ON_LE(post->byte_size);
	if (val == 0) val = (int)m_groups.GetSize();
	return val;
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemAmiga::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemAmiga::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
//	if (!chain.IsValid()) return;
	m_extension_list.Empty();

	int limit = basic->GetFatEndGroup() + 1;
	wxUint32 *tables =GetBlockTable();
	int max_blks = GetDataBlockNums();

	int t1 = GetFileType1();
	switch(t1) {
	case FILETYPE_MASK_AMIGA_FILE:
		// ファイルの時、data_block をたどる
		GetFileGroups(tables, max_blks, limit, &group_items, &m_extension_list);
		break;
	case FILETYPE_MASK_AMIGA_USERDIR:
		// ディレクトリの時、hash_table をたどる
		GetDirectoryGroups(basic, tables, max_blks, limit, &group_items);
		break;
	}
}

/// ファイルのすべてのグループを取得
bool DiskBasicDirItemAmiga::GetFileGroups(const wxUint32 *tables, int table_size, int limit, DiskBasicGroups *group_items, wxArrayInt *extension_list)
{
	int track_num = 0;
	int side_num = 0;
	int sector_num = 0;
	wxUint32 prev_num = 0;
	int prev_track_num = 0;
	int prev_side_num = 0;
	int prev_sector_num = 0;

	int calc_groups = 0;
	int calc_file_size = 0;
//	int real_file_size = 0;

	amiga_block_pre_t *expre = NULL;
	amiga_header_post_t *expost = NULL;

	const wxUint32 *table = tables;

	// データ部のブロックサイズ(OFSは24バイト減)
	int data_block_size = basic->GetSectorSize();
	if(!basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM))) {
		data_block_size -= 24;
	}

	// ファイルの時、data_block をたどる
	for(int extension = -1; ; extension++) {
		// 後ろから
		for(int i=table_size-1; i>=0 && limit >= 0; i--) {
			wxUint32 num = wxUINT32_SWAP_ON_LE(table[i]);
			if (num == 0) {
				break;
			}
			DiskImageSector *sector = basic->GetSectorFromGroup(num, track_num, side_num);
			if (!sector) {
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				break;
			}

			sector_num = sector->GetSectorNumber();
			if (group_items && prev_num > 0) {
				group_items->Add(prev_num, num, prev_track_num, prev_side_num, prev_sector_num, prev_sector_num);
			}
			prev_num = num;
			prev_track_num = track_num;
			prev_side_num = side_num;
			prev_sector_num = sector_num;

			calc_groups++;
			calc_file_size += data_block_size;
			limit--;
		}
		// グループ数はヘッダブロック分も足す
		calc_groups++;
		// エクステンションがあるか
		wxUint32 exnum = extension < 0 ? GetExtension() : wxUINT32_SWAP_ON_LE(expost->extension);
		if (exnum < 2) {
			break;
		}
		DiskImageSector *sector = basic->GetSectorFromGroup(exnum, track_num, side_num);
		if (!sector) {
			break;
		}
		expre = (amiga_block_pre_t *)sector->GetSectorBuffer();
		if (!expre) {
			break;
		}
		if (wxUINT32_SWAP_ON_LE(expre->type) != FILETYPE_MASK_AMIGA_LIST) {
			// エクステンションじゃない？
			break;
		}
		table = expre->u.table;
		expost = (amiga_header_post_t *)sector->GetSectorBuffer(sector->GetSectorSize() - (int)sizeof(amiga_header_post_t));
		if (extension_list) extension_list->Add(exnum);
	}
	if (group_items && prev_num > 0) {
		group_items->Add(prev_num, 0, prev_track_num, prev_side_num, prev_sector_num, prev_sector_num);
	}

	// ファイルサイズ
	if (group_items) {
		int real_file_size = (int)GetByteSize();
		if (calc_file_size < real_file_size) {
			group_items->SetSize(calc_file_size);
		} else {
			group_items->SetSize(real_file_size);
		}
		group_items->SetNums(calc_groups);
		group_items->SetSizePerGroup(basic->GetSectorSize());
	}

	return (limit < 0);
}

/// 指定ディレクトリのすべてのグループを取得
bool DiskBasicDirItemAmiga::GetDirectoryGroups(DiskBasic *basic, wxUint32 *tables, int table_size, int limit, DiskBasicGroups *group_items)
{
	bool valid = true;

	int track_num = 0;
	int side_num = 0;
	int sector_num = 0;

	// ディレクトリの時、hash_table をたどる
	for(int i=0; i<table_size && limit >= 0; i++) {
		wxUint32 num = wxUINT32_SWAP_ON_LE(tables[i]);
		if (num < 2) {
			continue;
		}
		wxUint32 *p_prev_chain = &tables[i];
		while (num >= 2 && limit >= 0) {
			limit--;
			DiskImageSector *sector = basic->GetSectorFromGroup(num, track_num, side_num);
			if (!sector) {
				// Why?
				valid = false;
				break;
			}
			// ヘッダ種類が2なら有効
			wxUint32 type = *(wxUint32 *)sector->GetSectorBuffer();
			type = wxUINT32_SWAP_ON_LE(type);
			if (type != FILETYPE_MASK_AMIGA_HEADER) {
				valid = false;
				break;
			}

			// 次のヘッダブロックがあるか
			int offset = sector->GetSectorSize() - (int)sizeof(amiga_hash_chain_t);
			amiga_hash_chain_t *chain = (amiga_hash_chain_t *)sector->GetSectorBuffer(offset);

			if (group_items) {
				sector_num = sector->GetSectorNumber();
				group_items->Add(num, 0, track_num, side_num, sector_num,
					new AmigaChain(i, p_prev_chain, &chain->hash_chain)
				);
			}

			p_prev_chain = &chain->hash_chain;
			num = wxUINT32_SWAP_ON_LE(chain->hash_chain);
		}
	}
	// 常に１ブロック
	if (group_items) {
		group_items->SetNums(1);
		group_items->SetSize(basic->GetSectorSize());
	}
	return (limit >= 0) && valid;
}

/// インデックス番号をリナンバ
void DiskBasicDirItemAmiga::RenumberInDirectory(DiskBasic *basic, DiskBasicDirItems *items)
{
	if (!items) return;

	int prev_idx = -1;
	int chains = 0;
	for(size_t i=0; i<items->Count(); i++) {
		DiskBasicDirItemAmiga *aitem = (DiskBasicDirItemAmiga *)items->Item(i);
		int idx = aitem->m_chain.m_idx;
		if (prev_idx != idx) {
			chains = 0;
		}
		aitem->SetNumber(chains * 1000 + idx);
		prev_idx = idx;
		chains++;
	}
}

/// アイテムリストにアイテムを挿入
/// @param[in] basic       DISK BASIC
/// @param[in] tables      ディレクトリヘッダ内のハッシュテーブル
/// @param[in] table_size  ハッシュテーブルのサイズ数
/// @param[in] limit       ループ制限値
/// @param[in,out] items   ディレクトリの子供アイテムリスト
/// @param[in,out] item    新たに追加するアイテム
bool DiskBasicDirItemAmiga::InsertItemInDirectory(DiskBasic *basic, const wxUint32 *tables, int table_size, int limit, DiskBasicDirItems *items, DiskBasicDirItem *item)
{
	bool valid = true;

	if (!(items && item)) {
		return false;
	}
#if 0
	wxUint32 block_num = item->GetStartGroup(0);
	int seq_num = 0;

	// ディレクトリの時、hash_table をたどる
	for(int i=0; i<table_size && limit >= 0; i++) {
		wxUint32 num = wxUINT32_SWAP_ON_LE(tables[i]);
		if (num < 2) {
			continue;
		}

		while (num >= 2 && limit >= 0) {
			limit--;
			DiskImageSector *sector = basic->GetSectorFromGroup(num);
			if (!sector) {
				// Why?
				valid = false;
				break;
			}
			// ヘッダ種類が2なら有効
			wxUint32 type = *(wxUint32 *)sector->GetSectorBuffer();
			type = wxUINT32_SWAP_ON_LE(type);
			if (type != FILETYPE_MASK_AMIGA_HEADER) {
				valid = false;
				break;
			}

			if (block_num == num) {
				items->Insert(item, seq_num); 
			} else {
				items->Item(seq_num)->SetNumber(seq_num);
			}
			seq_num++;

			// 次のヘッダブロックがあるか
			int offset = sector->GetSectorSize() - (int)sizeof(amiga_hash_chain_t);
			amiga_hash_chain_t *chain = (amiga_hash_chain_t *)sector->GetSectorBuffer(offset);
			num = wxUINT32_SWAP_ON_LE(chain->hash_chain);
		}
	}
#else
	valid = false;
	DiskBasicDirItemAmiga *aitem = (DiskBasicDirItemAmiga *)item;
	for(size_t i=0; i<items->Count(); i++) {
		DiskBasicDirItemAmiga *ad = (DiskBasicDirItemAmiga *)items->Item(i);
		// hash_tableのインデックス番号に沿ってインサート
		if (ad->m_chain.m_idx > aitem->m_chain.m_idx) {
			items->Insert(item, i);
			valid = true;
			break;
		}
	}
	if (!valid) {
		items->Add(item);
		valid = true;
	}
	// リナンバー
	RenumberInDirectory(basic, items);
#endif
	return (limit >= 0) && valid;
}

/// アイテムリストからアイテムを削除
bool DiskBasicDirItemAmiga::DeleteItemInDirectory(DiskBasic *basic, wxUint32 *tables, int table_size, int limit, DiskBasicDirItems *items, DiskBasicDirItem *item)
{
//	bool valid = true;

	if (!(items && item)) {
		return false;
	}
#if 0
	wxUint32 block_num = item->GetStartGroup(0);
	int seq_num = 0;

	// ディレクトリの時、hash_table をたどる
	for(int i=0; i<table_size && limit >= 0; i++) {
		wxUint32 num = wxUINT32_SWAP_ON_LE(tables[i]);
		if (num < 2) {
			continue;
		}
		DiskImageSector *prev_sector = NULL;
		while (num >= 2 && limit >= 0) {
			limit--;
			DiskImageSector *sector = basic->GetSectorFromGroup(num);
			if (!sector) {
				// Why?
				valid = false;
				break;
			}
			// ヘッダ種類が2なら有効
			wxUint32 type = *(wxUint32 *)sector->GetSectorBuffer();
			type = wxUINT32_SWAP_ON_LE(type);
			if (type != FILETYPE_MASK_AMIGA_HEADER) {
				valid = false;
				break;
			}

			// 次のヘッダブロックがあるか
			int offset = sector->GetSectorSize() - (int)sizeof(amiga_hash_chain_t);
			amiga_hash_chain_t *chain = (amiga_hash_chain_t *)sector->GetSectorBuffer(offset);
			wxUint32 next_num = wxUINT32_SWAP_ON_LE(chain->hash_chain);

			if (block_num == num) {
				// チェインの張り直し
				if (prev_sector) {
					int offset = prev_sector->GetSectorSize() - (int)sizeof(amiga_hash_chain_t);
					amiga_hash_chain_t *chain = (amiga_hash_chain_t *)prev_sector->GetSectorBuffer(offset);
					chain->hash_chain = wxUINT32_SWAP_ON_LE(next_num);
				} else {
					tables[i] = wxUINT32_SWAP_ON_LE(next_num);
				}
				// 削除
				items->Remove(item);
			} else {
				items->Item(seq_num)->SetNumber(seq_num);
				seq_num++;
			}

			prev_sector = sector;
			num = next_num;
		}
	}
	return (limit >= 0) && valid;
#else
	DiskBasicDirItemAmiga *aitem = (DiskBasicDirItemAmiga *)item;
	*aitem->m_chain.p_prev_chain = *aitem->m_chain.p_next_chain;

	items->Remove(item);

	// リナンバー
	RenumberInDirectory(basic, items);
	return true;
#endif
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有サイズ
/// @return 計算後のファイルサイズ
int DiskBasicDirItemAmiga::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
#if 0
	if (group_items.Count() == 0) return occupied_size;

	// 現在のセクタ
	DiskImageSector *sector = basic->GetSectorFromSectorPos(group_items.Last().group);
	if (!sector) {
		return occupied_size;
	}
	amiga_ptr_t *p = (amiga_ptr_t *)sector->GetSectorBuffer();
	if (!p) {
		return occupied_size;
	}
	if (p->track != 0) {
		// really last sector ?
		return occupied_size;
	}
	occupied_size = occupied_size + 1 + (int)p->sector - basic->GetSectorSize();
#endif
	return occupied_size;
}

#if 0
/// ファイル名、属性をコピー
/// @param [in] src ディレクトリアイテム
void DiskBasicDirItemAmiga::CopyItem(const DiskBasicDirItem &src)
{
	// データはコピーしない
//	CopyData(src.GetData());
	// グループ
	m_groups = src.m_groups;
	// サイズ
//	m_file_size = src.m_file_size;
	// フラグ
	m_flags = src.m_flags;
	// その他の属性
	m_external_attr = src.m_external_attr;
}
#endif

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemAmiga::GetDataSize() const
{
	return sizeof(directory_amiga_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemAmiga::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemAmiga::CopyData(const directory_t *val)
{
	if (!m_data.IsValid()) return false;

	m_data.Data()->block_num = val->amiga.block_num;

	// ポインタではなく内容をコピー
	amiga_block_pre_t *src_pre = val->amiga.pre;
	amiga_block_pre_t *dst_pre = m_data.Data()->pre;
	dst_pre->type = src_pre->type;
	dst_pre->header_key = src_pre->header_key;

	amiga_header_post_t *src_post = &val->amiga.post->h;
	amiga_header_post_t *dst_post = &m_data.Data()->post->h;

	dst_post->name_len = src_post->name_len;
	memcpy(dst_post->name, src_post->name, sizeof(dst_post->name));

	dst_post->protect = src_post->protect;

	dst_post->uid = src_post->uid;
	dst_post->gid = src_post->gid;

	dst_post->days = src_post->days;
	dst_post->mins = src_post->mins;
	dst_post->ticks = src_post->ticks;

	dst_post->parent_dir = src_post->parent_dir;
	dst_post->sec_type = src_post->sec_type;

	return true;
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemAmiga::ClearData()
{
	m_data.Data()->block_num = 0;

	// ポインタはクリアしない
}

/// 最初のグループ番号を設定
void DiskBasicDirItemAmiga::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	m_data.Data()->block_num = val;
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemAmiga::GetStartGroup(int fileunit_num) const
{
	return m_data.Data()->block_num;
}

#if 0
/// レコード長をセット(REL file)
void DiskBasicDirItemAmiga::SetRecordLength(int val)
{
	m_data.Data()->record_size = (val & 0xff);
}

/// レコード長を返す(REL file)
int DiskBasicDirItemAmiga::GetRecordLength() const
{
	return m_data.Data()->record_size;
}
#endif

/// ヘッダのあるグループ番号をセット(機種依存)
/// @param [in] val 番号
void DiskBasicDirItemAmiga::SetExtraGroup(wxUint32 val)
{
}

/// ヘッダのあるグループ番号を返す(機種依存)
/// @return 番号
wxUint32 DiskBasicDirItemAmiga::GetExtraGroup() const
{
	return GetStartGroup(0);
}

/// ヘッダのグループリストをセット(機種依存)
void DiskBasicDirItemAmiga::SetExtraGroups(const DiskBasicGroups &grps)
{
}

/// ヘッダのグループ番号を得る(機種依存)
void DiskBasicDirItemAmiga::GetExtraGroups(wxArrayInt &arr) const
{
	arr.Add(GetStartGroup(0));
	for(int i=0; i<(int)m_extension_list.Count(); i++) {
		arr.Add(m_extension_list.Item(i));
	}
}

/// ヘッダのグループリストを返す(機種依存)
void DiskBasicDirItemAmiga::GetExtraGroups(DiskBasicGroups &grps) const
{
	grps.Add(GetStartGroup(0), 0, 0, 0, 0, 0);
	for(int i=0; i<(int)m_extension_list.Count(); i++) {
		grps.Add(m_extension_list.Item(i), 0, 0, 0, 0, 0);
	}
}

/// @brief 次のグループ番号をセット(機種依存)
/// @param [in] val 番号
void DiskBasicDirItemAmiga::SetNextGroup(wxUint32 val)
{
	amiga_header_post_t *post = &m_data.Data()->post->h;
	if (post) post->hash_chain = wxUINT32_SWAP_ON_LE(val);
}

/// @brief 次のグループ番号を返す(機種依存)
wxUint32 DiskBasicDirItemAmiga::GetNextGroup() const
{
	amiga_header_post_t *post = &m_data.Data()->post->h;
	if (post) return wxUINT32_SWAP_ON_LE(post->hash_chain);
	return 0;
}

/// 日時を返す（ファイルリスト用）
wxString DiskBasicDirItemAmiga::GetFileDateTimeStr() const
{
	return GetFileModifyDateTimeStr();
}

/// 更新日付を返す
void DiskBasicDirItemAmiga::GetFileModifyDate(TM &tm) const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (!post) return;

	wxUint32 days = wxUINT32_SWAP_ON_LE(post->days);
	ConvDateToTm(days, tm);
}

/// 更新時間を返す
void DiskBasicDirItemAmiga::GetFileModifyTime(TM &tm) const
{
	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (!post) return;

	wxUint32 mins = wxUINT32_SWAP_ON_LE(post->mins);
	wxUint32 ticks = wxUINT32_SWAP_ON_LE(post->ticks);
	ConvTimeToTm(mins, ticks, tm);
}

/// 更新日付を返す
wxString DiskBasicDirItemAmiga::GetFileModifyDateStr() const
{
	TM tm;
	GetFileModifyDate(tm);
	return Utils::FormatYMDStr(tm);
}

/// 更新時間を返す
wxString DiskBasicDirItemAmiga::GetFileModifyTimeStr() const
{
	TM tm;
	GetFileModifyTime(tm);
	return Utils::FormatHMSStr(tm);
}

/// 更新日付をセット
void DiskBasicDirItemAmiga::SetFileModifyDate(const TM &tm)
{
	amiga_header_post_t *post = &m_data.Data()->post->h;
	if (!post) return;

	wxUint32 days;
	ConvDateFromTm(tm, days);
	post->days = wxUINT32_SWAP_ON_LE(days);
}

/// 更新時間をセット
void DiskBasicDirItemAmiga::SetFileModifyTime(const TM &tm)
{
	amiga_header_post_t *post = &m_data.Data()->post->h;
	if (!post) return;

	wxUint32 mins;
	wxUint32 ticks;
	ConvTimeFromTm(tm, mins, ticks);
	post->mins = wxUINT32_SWAP_ON_LE(mins);
	post->ticks = wxUINT32_SWAP_ON_LE(ticks);
}

/// 日付を変換
void DiskBasicDirItemAmiga::ConvDateToTm(wxUint32 days, TM &tm)
{
	wxUint32 year = 0;
	wxUint32 month = 0;
	wxUint32 day = 0;
//	bool leap_year = false;
	// 1978-01-01から
	for(wxUint32 i=0; i<200 ;i++) {
		year = 1978 + i;
//		leap_year = (((year % 400) == 0) || ((year % 100) != 0 && (year % 4) == 0));
//		leap_year = wxDateTime::IsLeapYear((int)year);
//		wxUint32 days_per_year;
//		if (leap_year) {
//			days_per_year = 366;
//		} else {
//			days_per_year = 365;
//		}
		wxUint32 days_per_year = wxDateTime::GetNumberOfDays((int)year);
		if (days < days_per_year) {
			month = days % days_per_year;
			break;
		}
		days -= days_per_year;
	}
	// month and day
//	const wxUint32 day_of_months[] = {
//		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
//	};
	for(wxUint32 i=0; i<12 ;i++) {
//		wxUint32 day_of_month = day_of_months[i];
		wxUint32 day_of_month = wxDateTime::GetNumberOfDays((wxDateTime::Month)i, (int)year);
//		if (i == 1 && leap_year) {
//			day_of_month++;
//		}
		if (month < day_of_month) {
			day = (month % day_of_month) + 1;
			month = i;
			break;
		}
		month -= day_of_month;
	}

	tm.SetYear((int)year - 1900);
	tm.SetMonth((int)month);
	tm.SetDay((int)day);
}

/// 時間を変換
 void DiskBasicDirItemAmiga::ConvTimeToTm(wxUint32 mins, wxUint32 ticks, TM &tm)
{
	// hour minute
	wxUint32 hour = mins / 60;
	wxUint32 minute = mins % 60;
	// second
	wxUint32 second = ticks / 50;

	tm.SetHour((int)hour);
	tm.SetMinute((int)minute);
	tm.SetSecond((int)second);
}

/// 日付を変換
void DiskBasicDirItemAmiga::ConvDateFromTm(const TM &tm, wxUint32 &days)
{
	days = 0;

	int year = tm.GetYear() + 1900;
	if (year < 1978) year = 1978;
	if (year > 2178) year = 2178;
	for(int i=1978; i<year; i++) {
		days += (wxUint32)wxDateTime::GetNumberOfDays(i);
	}

	int month = tm.GetMonth();
	for(int i=0; i<month; i++) {
		days += (wxUint32)wxDateTime::GetNumberOfDays((wxDateTime::Month)i, year);
	}

	days += tm.GetDay() - 1;
}

/// 時間を変換
void DiskBasicDirItemAmiga::ConvTimeFromTm(const TM &tm, wxUint32 &mins, wxUint32 &ticks)
{
	ticks = (wxUint32)tm.GetSecond();
	ticks *= 50;

	mins = (wxUint32)tm.GetHour();
	mins *= 60;
	mins += (wxUint32)tm.GetMinute();
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemAmiga::NeedCheckEofCode()
{
	return false;
}

/// セーブ時にファイルサイズを再計算する
int DiskBasicDirItemAmiga::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	return file_size;
}

/// データをエクスポートする前に必要な処理
/// 属性に応じて拡張子を付加する
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemAmiga::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	return true;
}

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemAmiga::PreImportDataFile(wxString &filename)
{
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemAmiga::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	return 0;
}

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemAmiga::SetModify()
{
	// 関連するセクタのチェックサムを更新する
	UpdateCheckSum();
}

/// チェックサムを更新する
void DiskBasicDirItemAmiga::UpdateCheckSumAll()
{
	if (GetFileType1() != FILETYPE_MASK_AMIGA_FILE) {
		// ファイルでない
		return;
	}

	// ファイルの場合
	DiskBasicDirItemAmiga *aitem = new DiskBasicDirItemAmiga(basic, NULL, 0, NULL);
	int trk = 0;
	int sid = 0;
	wxUint32 ext_block_num = GetExtension();
	if (ext_block_num > 0) {
		DiskImageSector *sector = basic->GetSectorFromGroup(ext_block_num, trk, sid);
		aitem->SetDataPtr(0, NULL, sector, 0, sector->GetSectorBuffer());
		aitem->UpdateCheckSum();
		aitem->UpdateCheckSumAll();
	}
	// OFSの場合は、各データブロックのチェックサムも更新する
	if (!basic->GetVariousBoolParam(wxT(KEY_FAST_FILE_SYSTEM))) {
		int max_blks = GetDataBlockNums();
		for(int i=max_blks-1; i>=0; i--) {
			wxUint32 num = GetDataBlock(i);
			if (num == 0) {
				break;
			}
			DiskImageSector *sector = basic->GetSectorFromGroup(num, trk, sid);
			aitem->SetDataPtr(0, NULL, sector, 0, sector->GetSectorBuffer());
			aitem->UpdateCheckSum();
		}
	}
	delete aitem;
}

/// チェックサムを更新する
void DiskBasicDirItemAmiga::UpdateCheckSum()
{
	amiga_block_pre_t *pre = m_data.Data()->pre;
	if (!pre) return;

	CalcCheckSum(pre, basic->GetSectorSize(), pre->check_sum);
}

/// チェックサムを計算
/// @param[in]     data     ブロックデータ
/// @param[in]     size     ブロックサイズ
wxUint32 DiskBasicDirItemAmiga::CalcCheckSum(const void *data, int size)
{
	const wxUint32 *dp = (const wxUint32 *)data;
	wxUint32 newsum = 0;
	size >>= 2;
	for(int i=0; i<size; i++) {
		newsum += wxUINT32_SWAP_ON_LE(dp[i]);
	}
	newsum = ~newsum;
	newsum++;

	return newsum;
}

/// チェックサムを計算
/// @param[in]     data     ブロックデータ
/// @param[in]     size     ブロックサイズ
/// @param[in,out] sum_data チェックサムを入れるポインタ
void DiskBasicDirItemAmiga::CalcCheckSum(const void *data, int size, wxUint32 &sum_data)
{
	sum_data = 0;	// require
	wxUint32 newsum = CalcCheckSum(data, size);
	sum_data = wxUINT32_SWAP_ON_LE(newsum);
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

#define IDC_COMBO_TYPE1		51
#define IDC_TEXT_USERID		52
#define IDC_TEXT_GROUPID	53
#define IDC_CHECK_TYPE2		54


/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemAmiga::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_NEW_FILE) {
		// 外部からインポート時
//		file_type_1 = ConvOriginalTypeFromFileName(name);
		file_type_1 = FILETYPE_MASK_AMIGA_FILE;
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in,out] sizer      レイアウト
/// @param [in] flags          レイアウトフラグ
void DiskBasicDirItemAmiga::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1();
	int file_type_2 = GetFileType2();

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString types1;
	for(int i=TYPE_NAME_AMIGA_FILE; i<=TYPE_NAME_AMIGA_SOFTLINK; i++) {
		types1.Add(wxGetTranslation(gTypeNameAmiga1l[i]));
	}
	wxFlexGridSizer *gbox = new wxFlexGridSizer(2, 0, 0);
	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	wxBoxSizer *vbox;
	vbox = new wxBoxSizer(wxVERTICAL);
	wxChoice *comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	int type1pos = ConvFileType1Pos(file_type_1);
	comType1->SetSelection(type1pos);
	if (show_flags & INTNAME_NEW_FILE) {
		comType1->Enable(false);
	}
	vbox->Add(comType1, flags);
	gbox->Add(vbox);

	wxBoxSizer *hbox = NULL;
	vbox = new wxBoxSizer(wxVERTICAL);
	for(int i=3; i>=0; i--) {
		if ((i % 2) == 1) hbox = new wxBoxSizer(wxHORIZONTAL);
		wxCheckBox *chk = new wxCheckBox(parent, IDC_CHECK_TYPE2 + i, wxGetTranslation(gTypeNameAmiga2p[i]));
		chk->SetValue(((wxUint32)file_type_2 & gTypeConvAmiga2[i]) == 0);
		hbox->Add(chk, flags);
		if ((i % 2) == 0) vbox->Add(hbox);
	}
	gbox->Add(vbox);

	for(int i=4; i<8; i++) {
		if ((i % 2) == 0) hbox = new wxBoxSizer(wxHORIZONTAL);
		wxCheckBox *chk = new wxCheckBox(parent, IDC_CHECK_TYPE2 + i, wxGetTranslation(gTypeNameAmiga2p[i]));
		chk->SetValue(((wxUint32)file_type_2 & gTypeConvAmiga2[i]) != 0);
		hbox->Add(chk, flags);
		if ((i % 2) == 1) gbox->Add(hbox);
	}

	staType1->Add(gbox);

	sizer->Add(staType1, flags);

	wxSize size;

	wxStaticBoxSizer *staType2b = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("Attributes on Multi User System")), wxVERTICAL);
	hbox = new wxBoxSizer(wxHORIZONTAL);

	hbox->Add(new wxStaticText(parent, wxID_ANY, _("User ID")), flags);
	wxTextCtrl *txtUID = new wxTextCtrl(parent, IDC_TEXT_USERID, wxString::Format(wxT("%d"), GetUserID()));
	txtUID->SetMaxLength(6);
	hbox->Add(txtUID);

	hbox->Add(new wxStaticText(parent, wxID_ANY, _("Group ID")), flags);
	wxTextCtrl *txtGID = new wxTextCtrl(parent, IDC_TEXT_GROUPID, wxString::Format(wxT("%d"), GetGroupID()));
	txtGID->SetMaxLength(6);
	hbox->Add(txtGID);

	{
		int i=16;
		wxCheckBox *chk = new wxCheckBox(parent, IDC_CHECK_TYPE2 + i, wxGetTranslation(gTypeNameAmiga2p[i-8]));
		chk->SetValue(((wxUint32)file_type_2 & gTypeConvAmiga2[i]) != 0);
		hbox->Add(chk, flags);
	}
	staType2b->Add(hbox);

	size = txtGID->GetTextExtent(wxT("00000000"));
	if (size.x < 0) size.x = 80;
	size.y = -1;
	txtUID->SetMinSize(size);
	txtGID->SetMinSize(size);

	gbox = new wxFlexGridSizer(2, 0, 0);
	gbox->Add(new wxStaticText(parent, wxID_ANY, _("User in Group")), flags);
	vbox = new wxBoxSizer(wxVERTICAL);
	for(int i=11; i>=8; i--) {
		if ((i % 2) == 1) hbox = new wxBoxSizer(wxHORIZONTAL);
		wxCheckBox *chk = new wxCheckBox(parent, IDC_CHECK_TYPE2 + i, wxGetTranslation(gTypeNameAmiga2p[i-8]));
		chk->SetValue(((wxUint32)file_type_2 & gTypeConvAmiga2[i]) == 0);
		hbox->Add(chk, flags);
		if ((i % 2) == 0) vbox->Add(hbox);
	}
	gbox->Add(vbox);

	gbox->Add(new wxStaticText(parent, wxID_ANY, _("Other User")), flags);
	vbox = new wxBoxSizer(wxVERTICAL);
	for(int i=15; i>=12; i--) {
		if ((i % 2) == 1) hbox = new wxBoxSizer(wxHORIZONTAL);
		wxCheckBox *chk = new wxCheckBox(parent, IDC_CHECK_TYPE2 + i, wxGetTranslation(gTypeNameAmiga2p[i-12]));
		chk->SetValue(((wxUint32)file_type_2 & gTypeConvAmiga2[i]) == 0);
		hbox->Add(chk, flags);
		if ((i % 2) == 0) vbox->Add(hbox);
	}
	gbox->Add(vbox);

	staType2b->Add(gbox);

	sizer->Add(staType2b, flags);
}

/// ダイアログ内の値を設定
void DiskBasicDirItemAmiga::InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data)
{
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemAmiga::ChangeTypeInAttrDialog(IntNameBox *parent)
{
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemAmiga::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	bool valid = true;

	// ファイル, ディレクトリ
	int t1 = 0;
	wxChoice *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	if (comType1) {
		int pos1 = comType1->GetSelection();
		if (0 <= pos1 && pos1 <= TYPE_NAME_AMIGA_SOFTLINK) {
			t1 = gTypeNameAmiga1[pos1].value;
		}
	}

	// プロテクション
	int t2 = 0;
	for(int i=0; i<=16; i++) {
		wxCheckBox *chk = (wxCheckBox *)parent->FindWindow(IDC_CHECK_TYPE2 + i);
		bool checked = !((0 <= i && i <= 3) || (8 <= i && i <= 15));
		if (chk) {
			checked = (chk->GetValue() == checked);
		}
		if (checked) {
			t2 |= (int)gTypeConvAmiga2[i];
		}
	}

	// ユーザID
	int t3 = 0;
	wxTextCtrl *txtUID = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_USERID);
	wxTextCtrl *txtGID = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_GROUPID);
	if (txtUID && txtGID) {
		t3 = Utils::ToInt(txtGID->GetValue());
		t3 <<= 16;
		t3 |= Utils::ToInt(txtUID->GetValue());
	}

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, t1, t2, t3);

	return valid;
}

#if 0
/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemAmiga::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	return true;
}

/// その他の属性値を設定する
void DiskBasicDirItemAmiga::SetOptionalAttr(DiskBasicDirItemAttr &attr)
{
}
#endif

/// プロパティで表示する内部データを設定
void DiskBasicDirItemAmiga::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("ht_idx"), m_chain.m_idx);
	vals.Add(wxT("prev_blk"), (wxUint32)(m_chain.p_prev_chain ? *m_chain.p_prev_chain : -1), true);
	vals.Add(wxT("next_blk"), (wxUint32)(m_chain.p_next_chain ? *m_chain.p_next_chain : -1), true);

	amiga_block_pre_t *pre = (amiga_block_pre_t *)m_data.Data()->pre;
	if (!pre) return;

	vals.Add(wxT("TYPE"), pre->type, true);
	vals.Add(wxT("HEADER_KEY"), pre->header_key, true);
	vals.Add(wxT("HIGH_SEQ"), pre->high_seq, true);
	vals.Add(wxT("TABLE_SIZE"), pre->table_size, true);
	vals.Add(wxT("FIRST_DATA"), pre->first_data, true);
	vals.Add(wxT("CHECK_SUM"), pre->check_sum, true);

	int blks = GetDataBlockNums();
	for(int i=0; i<blks; i++) {
		vals.Add(wxString::Format(wxT("TABLE %d"), i), pre->u.table[i], true);
	}

	amiga_header_post_t *post = (amiga_header_post_t *)m_data.Data()->post;
	if (!post) return;

	vals.Add(wxT("UNUSED0"), post->unused0, sizeof(post->unused0));
	vals.Add(wxT("UID"), post->uid, true);
	vals.Add(wxT("GID"), post->gid, true);
	vals.Add(wxT("PROTECT"), post->protect, true);
	vals.Add(wxT("BYTE_SIZE"), post->byte_size, true);
	vals.Add(wxT("COMMENT_LEN"), post->comment_len);
	vals.Add(wxT("COMMENT"), post->comment, sizeof(post->comment));
	vals.Add(wxT("UNUSED1"), post->unused1, sizeof(post->unused1));
	vals.Add(wxT("DAYS"), post->days, true);
	vals.Add(wxT("MINS"), post->mins, true);
	vals.Add(wxT("TICKS"), post->ticks, true);
	vals.Add(wxT("NAME_LEN"), post->name_len);
	vals.Add(wxT("NAME"), post->name, sizeof(post->name));
	vals.Add(wxT("UNUSED2"), post->unused2, sizeof(post->unused2));
	vals.Add(wxT("REAL_ENTRY"), post->real_entry, true);
	vals.Add(wxT("NEXT_LINK"), post->next_link, true);
	vals.Add(wxT("UNUSED3"), post->unused3, sizeof(post->unused3));
	vals.Add(wxT("HASH_CHAIN"), post->hash_chain, true);
	vals.Add(wxT("PARENT_DIR"), post->parent_dir, true);
	vals.Add(wxT("EXTENSION"), post->extension, true);
	vals.Add(wxT("SEC_TYPE"), post->sec_type, true);
}
