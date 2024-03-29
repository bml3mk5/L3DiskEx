/// @file basicdiritem_c1541.cpp
///
/// @brief disk basic directory item for Commodore 1541
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_c1541.h"
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

/// C1541属性名
const name_value_t gTypeNameC1541[] = {
	{ "DEL", FILETYPE_MASK_C1541_DEL },
	{ "SEQ", FILETYPE_MASK_C1541_SEQ },
	{ "PRG", FILETYPE_MASK_C1541_PRG },
	{ "USR", FILETYPE_MASK_C1541_USR },
	{ "REL", FILETYPE_MASK_C1541_REL },
	{ NULL, -1 }
};

/// C1541属性変換テーブル
const value_value_t gTypeConvC1541[] = {
	{ FILE_TYPE_DATA_MASK | FILE_TYPE_ASCII_MASK, FILETYPE_MASK_C1541_SEQ },
	{ FILE_TYPE_BASIC_MASK | FILE_TYPE_BINARY_MASK, FILETYPE_MASK_C1541_PRG },
	{ FILE_TYPE_MACHINE_MASK | FILE_TYPE_BINARY_MASK, FILETYPE_MASK_C1541_USR },
	{ FILE_TYPE_DATA_MASK | FILE_TYPE_RANDOM_MASK, FILETYPE_MASK_C1541_REL },
	{ -1, -1 }
};


//////////////////////////////////////////////////////////////////////
//
// ディレクトリ１アイテム Commodore 1541
//
DiskBasicDirItemC1541::DiskBasicDirItemC1541(DiskBasic *basic)
	: DiskBasicDirItem(basic)
{
	m_data.Alloc();
}
DiskBasicDirItemC1541::DiskBasicDirItemC1541(DiskBasic *basic, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data)
	: DiskBasicDirItem(basic, n_sector, n_secpos, n_data)
{
	m_data.Attach(n_data);
}
DiskBasicDirItemC1541::DiskBasicDirItemC1541(DiskBasic *basic, int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next, bool &n_unuse)
	: DiskBasicDirItem(basic, n_num, n_gitem, n_sector, n_secpos, n_data, n_next, n_unuse)
{
	m_data.Attach(n_data);

	Used(CheckUsed(n_unuse));

	CalcFileSize();
}

/// アイテムへのポインタを設定
/// @param [in]  n_num      通し番号
/// @param [in]  n_gitem    トラック番号などのデータ
/// @param [in]  n_sector   セクタ
/// @param [in]  n_secpos   セクタ内のディレクトリエントリの位置
/// @param [in]  n_data     ディレクトリアイテム
/// @param [out] n_next     次のセクタ
void DiskBasicDirItemC1541::SetDataPtr(int n_num, const DiskBasicGroupItem *n_gitem, DiskImageSector *n_sector, int n_secpos, wxUint8 *n_data, const SectorParam *n_next)
{
	DiskBasicDirItem::SetDataPtr(n_num, n_gitem, n_sector, n_secpos, n_data, n_next);

	m_data.Attach(n_data);
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemC1541::GetFileNamePos(int num, size_t &size, size_t &len) const
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
void DiskBasicDirItemC1541::SetNativeName(wxUint8 *filename, size_t size, size_t length)
{
	wxUint8 *n;
	size_t nl = 0;
	size_t ns = 0;
	n = GetFileNamePos(0, ns, nl);
	if (n != NULL && ns > 0) {
		memcpy(n, filename, ns);
	}
}

/// ファイル名を得る
/// @param [in,out] filename ファイル名
/// @param [in]     size     バッファサイズ
/// @param [out]    length   長さ
void DiskBasicDirItemC1541::GetNativeName(wxUint8 *filename, size_t size, size_t &length) const
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
int DiskBasicDirItemC1541::GetFileNameStrSize() const
{
	size_t s = 0;
	size_t l = 0;
	GetFileNamePos(0, s, l);

	return (int)s;
}

/// 属性１を返す
int	DiskBasicDirItemC1541::GetFileType1() const
{
	return m_data.Data()->type;
}

/// 属性１を設定
void DiskBasicDirItemC1541::SetFileType1(int val)
{
	m_data.Data()->type = (val & 0xff);
}

/// 使用しているアイテムか
bool DiskBasicDirItemC1541::CheckUsed(bool unuse)
{
	return (GetFileType1() & 0x80) != 0;
}

/// 削除
bool DiskBasicDirItemC1541::Delete()
{
	// 削除
	Used(false);
	SetFileType1(basic->GetDeleteCode());
	return true;
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemC1541::Check(bool &last)
{
	if (!m_data.IsValid()) return false;

	bool valid = true;

	wxUint8 type = GetFileType1();
	if (FILETYPE_MASK_C1541_REL < type) {
		valid = false;
	}
	return valid;
}

/// 属性を設定
void DiskBasicDirItemC1541::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	if (file_type.GetFormat()== basic->GetFormatTypeNumber()) {
		// 同じOS
		int t1 = file_type.GetOrigin();
		int rl = t1 >> 8;
		t1 &= 0xff;
		SetFileType1(t1);
		SetRecordLength(rl);
	} else {
		// 違うOSから
		int t1 = ConvToFileType1(ftype);
		if (t1 > 0) SetFileType1(t1);
	}
}

/// 属性を返す
DiskBasicFileType DiskBasicDirItemC1541::GetFileAttr() const
{
	int rl = GetRecordLength();
	int t1 = GetFileType1();
	int val = ConvFromFileType1(t1);
	return DiskBasicFileType(basic->GetFormatTypeNumber(), val, rl << 8 | t1);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemC1541::GetFileAttrStr() const
{
	wxString str;
	int spos = ConvFileType1Pos(GetFileType1());
	if (spos >= 0) {
		str = gTypeNameC1541[spos].name;
	} else {
		str = wxT("???");
	}
	return str;
}

/// 共通属性を個別属性に変換
int DiskBasicDirItemC1541::ConvToFileType1(int ftype)
{
	int t1 = 0;
	for(int i=0; gTypeConvC1541[i].ori_value != -1; i++) {
		if ((ftype & FILE_TYPE_EXTENSION_MASK) == gTypeConvC1541[i].com_value) {
			t1 = gTypeConvC1541[i].ori_value;
			break;
		}
	}
	return t1;
}

/// 個別属性を共通属性に変換
int DiskBasicDirItemC1541::ConvFromFileType1(int type1)
{
	int val = 0;
	for(int i=0; gTypeConvC1541[i].ori_value != -1; i++) {
		if (type1 == gTypeConvC1541[i].ori_value) {
			val = gTypeConvC1541[i].com_value;
			break;
		}
	}
	return val;
}

/// 属性からリストの位置を返す
int DiskBasicDirItemC1541::ConvFileType1Pos(int type1) const
{
	return gTypeNameC1541[0].IndexOf(gTypeNameC1541, type1);
}

/// ファイルサイズをセット
void DiskBasicDirItemC1541::SetFileSize(int val)
{
	m_groups.SetSize(val);
	int blk = basic->GetSectorSize() - 2;
	int grps = (val + blk - 1) / blk;

	SetBlocks(grps);
}

/// ファイルサイズを返す
int DiskBasicDirItemC1541::GetFileSize() const
{
	int val = (int)m_groups.GetSize(); 
	if (val == 0) {
		val = (int)GetBlocks();
		val *= (basic->GetSectorSize() - 2);
	}
	return val;
}

/// ブロック数をセット
void DiskBasicDirItemC1541::SetBlocks(wxUint16 val)
{
	m_data.Data()->num_of_blocks = wxUINT16_SWAP_ON_BE(val);
}

/// ブロック数を返す
wxUint16 DiskBasicDirItemC1541::GetBlocks() const
{
	return wxUINT16_SWAP_ON_BE(m_data.Data()->num_of_blocks);
}

/// ファイルサイズとグループ数を計算する
void DiskBasicDirItemC1541::CalcFileUnitSize(int fileunit_num)
{
	if (!IsUsed()) return;

	GetUnitGroups(fileunit_num, m_groups);
}

/// 指定ディレクトリのすべてのグループを取得
/// @param [in]  fileunit_num ファイル番号
/// @param [out] group_items  グループリスト
void DiskBasicDirItemC1541::GetUnitGroups(int fileunit_num, DiskBasicGroups &group_items)
{
//	if (!chain.IsValid()) return;

	int track_num = 0;
	int side_num = 0;
	int sector_num = 0;

	int sector_size = basic->GetSectorSize();
	// 1セクタ当たり2バイトはチェイン用のリンクポインタになるので減算
	int bytes_per_group = basic->GetSectorSize() - 2;
	int limit = bytes_per_group * (int)GetBlocks();

	int blks = 1;
	int type1 = GetFileType1();
	// RELative fileの場合はサイドセクタ分を加算する
	if (type1 == FILETYPE_MASK_C1541_REL) {
		m_ss_groups.Empty();
		blks = 2;		
	}

	// データ部分とサイドセクタ分のサイズ
	int calc_groups = 0;	// 合計分
	for(int i=0; i<blks; i++) {
		int calc_file_size = 0;
		DiskBasicGroups tmp_grp_items;
		wxUint32 group_num = (i == 0 ? GetStartGroup(fileunit_num) : GetExtraGroup());

		while(limit > 0) {
			int sector_pos = (int)group_num;
			DiskImageSector *sector = basic->GetSectorFromSectorPos(sector_pos, track_num, side_num);
			if (!sector) {
				break;
			}
			wxUint8 *buffer = sector->GetSectorBuffer();
			if (!buffer) {
				break;
			}
			sector_num = sector->GetSectorNumber();

			tmp_grp_items.Add(group_num, 0, track_num, side_num, sector_num, sector_num);

			calc_groups++;
			calc_file_size += bytes_per_group;
			limit -= bytes_per_group;

			// 次のセクタなし
			c1541_ptr_t *next = (c1541_ptr_t *)buffer;
			if (next->track == 0 || (int)next->track > basic->GetTracksPerSideOnBasic()) {
				break;
			}

			group_num = type->GetSectorPosFromNumS(next->track - C1541_START_TRACK_OFFSET, next->sector - C1541_START_SECTOR_OFFSET);
		}

		calc_file_size = RecalcFileSize(tmp_grp_items, calc_file_size);

		if (i == 0) {
			// 最終セクタの再計算
			group_items.Add(tmp_grp_items);
			group_items.SetSize(calc_file_size);
		} else {
			// サイドセクタ分
			m_ss_groups.Add(tmp_grp_items);
			m_ss_groups.SetSize(calc_file_size);
		}
	}

	group_items.SetNums(calc_groups);
	group_items.SetSizePerGroup(sector_size);

//	// ファイル内部のアドレスを得る
//	TakeAddressesInFile(group_items);
}

/// 最終セクタのサイズを計算してファイルサイズを返す
/// @param [in] group_items   グループリスト
/// @param [in] occupied_size 占有サイズ
/// @return 計算後のファイルサイズ
int DiskBasicDirItemC1541::RecalcFileSize(DiskBasicGroups &group_items, int occupied_size)
{
	if (group_items.Count() == 0) return occupied_size;

	// 現在のセクタ
	DiskImageSector *sector = basic->GetSectorFromSectorPos(group_items.Last().group);
	if (!sector) {
		return occupied_size;
	}
	c1541_ptr_t *p = (c1541_ptr_t *)sector->GetSectorBuffer();
	if (!p) {
		return occupied_size;
	}
	if (p->track != 0) {
		// really last sector ?
		return occupied_size;
	}
	occupied_size = occupied_size + 1 + (int)p->sector - basic->GetSectorSize();

	return occupied_size;
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemC1541::GetDataSize() const
{
	return sizeof(directory_c1541_t);
}

/// アイテムを返す
directory_t	*DiskBasicDirItemC1541::GetData() const
{
	return (directory_t *)m_data.Data();
}

/// アイテムをコピー
bool DiskBasicDirItemC1541::CopyData(const directory_t *val)
{
	// エントリの最初2バイトは使用禁止
	return m_data.Copy(val, GetDataSize(), basic->IsDataInverted(), sizeof(m_data.Data()->do_not_write));
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemC1541::ClearData()
{
	// エントリの最初2バイトは使用禁止
	m_data.Fill(0, GetDataSize(), basic->IsDataInverted(), sizeof(m_data.Data()->do_not_write));
}

/// 最初のグループ番号を設定
void DiskBasicDirItemC1541::SetStartGroup(int fileunit_num, wxUint32 val, int size)
{
	int trk_num = 0;
	int sec_num = 0;
	type->GetNumFromSectorPosS((int)val, trk_num, sec_num);
	trk_num += C1541_START_TRACK_OFFSET;
	sec_num += C1541_START_SECTOR_OFFSET;
	m_data.Data()->first_data.track = (trk_num & 0xff);
	m_data.Data()->first_data.sector = (sec_num & 0xff);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemC1541::GetStartGroup(int fileunit_num) const
{
	int trk_num = m_data.Data()->first_data.track - C1541_START_TRACK_OFFSET;
	int sec_num = m_data.Data()->first_data.sector - C1541_START_SECTOR_OFFSET;
	return (wxUint32)type->GetSectorPosFromNumS(trk_num, sec_num);
}

/// レコード長をセット(REL file)
void DiskBasicDirItemC1541::SetRecordLength(int val)
{
	m_data.Data()->record_size = (val & 0xff);
}

/// レコード長を返す(REL file)
int DiskBasicDirItemC1541::GetRecordLength() const
{
	return m_data.Data()->record_size;
}

/// サイドセクタのあるグループ番号をセット(機種依存)(REL file)
/// @param [in] val 番号
void DiskBasicDirItemC1541::SetExtraGroup(wxUint32 val)
{
	int trk_num = 0;
	int sec_num = 0;
	type->GetNumFromSectorPosS((int)val, trk_num, sec_num);
	trk_num += C1541_START_TRACK_OFFSET;
	sec_num += C1541_START_SECTOR_OFFSET;
	m_data.Data()->first_side.track = (trk_num & 0xff);
	m_data.Data()->first_side.sector = (sec_num & 0xff);
}

/// サイドセクタのあるグループ番号を返す(機種依存)(REL file)
/// @return 番号
wxUint32 DiskBasicDirItemC1541::GetExtraGroup() const
{
	int trk_num = m_data.Data()->first_side.track - C1541_START_TRACK_OFFSET;
	int sec_num = m_data.Data()->first_side.sector - C1541_START_SECTOR_OFFSET;
	return (wxUint32)type->GetSectorPosFromNumS(trk_num, sec_num);
}

/// サイドセクタのグループリストをセット(機種依存)
void DiskBasicDirItemC1541::SetExtraGroups(const DiskBasicGroups &grps)
{
	m_ss_groups = grps;
	// ブロック数を合計する
	SetBlocks(GetBlocks() + grps.GetNums());
	// グループ数も加算
	m_groups.AddNums(grps.GetNums());
}

/// サイドセクタのグループ番号を得る(機種依存)
void DiskBasicDirItemC1541::GetExtraGroups(wxArrayInt &arr) const
{
	// RELファイルの時はサイドセクタのグループ
	int type1 = GetFileType1();
	if (type1 != FILETYPE_MASK_C1541_REL) return;

	for(size_t i=0; i<m_ss_groups.Count(); i++) {
		arr.Add((int)m_ss_groups.Item(i).group);
	}
}

/// サイドセクタのグループリストを返す(機種依存)
void DiskBasicDirItemC1541::GetExtraGroups(DiskBasicGroups &grps) const
{
	grps = m_ss_groups;
}

/// ファイルの終端コードをチェックする必要があるか
bool DiskBasicDirItemC1541::NeedCheckEofCode()
{
	return false;
}

/// セーブ時にファイルサイズを再計算する
int DiskBasicDirItemC1541::RecalcFileSizeOnSave(wxInputStream *istream, int file_size)
{
	return file_size;
}

/// データをエクスポートする前に必要な処理
/// 属性に応じて拡張子を付加する
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemC1541::PreExportDataFile(wxString &filename)
{
	if (!gConfig.IsAddExtensionExport()) return true;

	/// 属性から拡張子を付加する
	wxString ext;
	if (GetFileAttrName(ConvFileType1Pos(GetFileType1()), gTypeNameC1541, ext)) {
		filename += wxT(".");
		if (Utils::IsUpperString(filename)) {
			filename += ext.Upper();
		} else {
			filename += ext.Lower();
		}
	}
	return true;
}

/// データをインポートする前に必要な処理
/// @param [in,out] filename ファイル名
/// @return false このファイルは対象外とする
bool DiskBasicDirItemC1541::PreImportDataFile(wxString &filename)
{
	if (gConfig.IsDecideAttrImport()) {
		TrimLastExtensionByExtensionAttr(filename, gTypeNameC1541, TYPE_NAME_C1541_DEL, TYPE_NAME_C1541_REL, &filename, NULL, NULL);
	}
	// 拡張子を消す
	filename = RemakeFileNameAndExtStr(filename);
	return true;
}

/// ファイル名から属性を決定する
int DiskBasicDirItemC1541::ConvOriginalTypeFromFileName(const wxString &filename) const
{
	int t1 = 0;
	int p1 = 0;
	// 拡張子で属性を設定する
	if (TrimLastExtensionByExtensionAttr(filename, gTypeNameC1541, TYPE_NAME_C1541_DEL, TYPE_NAME_C1541_REL, NULL, &t1, &p1)) {
		// 外部パラメータで設定したものは共通属性なので変換
		if (p1 < 0) {
			t1 = ConvToFileType1(t1);
		}
	} else {
		// default
		t1 = FILETYPE_MASK_C1541_SEQ;
	}

	return t1;
}

/// アイテムの属するセクタを変更済みにする
void DiskBasicDirItemC1541::SetModify()
{
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
#define IDC_TEXT_RECSIZE	52
#define IDC_TEXT_SIDESEC	53


/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemC1541::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
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
/// @param [in,out] sizer      レイアウト
/// @param [in] flags          レイアウトフラグ
void DiskBasicDirItemC1541::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1();
	int file_type_2 = 0;
	wxChoice   *comType1;
	wxTextCtrl *txtRecSize;
	wxTextCtrl *txtSideSec;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString types1;
	for(int i=TYPE_NAME_C1541_DEL; i<=TYPE_NAME_C1541_REL; i++) {
		types1.Add(wxGetTranslation(gTypeNameC1541[i].name));
	}
	wxStaticBoxSizer *staType1 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Type")), wxVERTICAL);
	comType1 = new wxChoice(parent, IDC_COMBO_TYPE1, wxDefaultPosition, wxDefaultSize, types1);
	int type1pos = ConvFileType1Pos(file_type_1);
	comType1->SetSelection(type1pos);
	staType1->Add(comType1, flags);

	sizer->Add(staType1, flags);

	wxSize tsize(INTNAME_COLUMN_WIDTH, -1);
	wxSizerFlags atitle = wxSizerFlags().Align(wxALIGN_CENTER_VERTICAL);


	wxStaticBoxSizer *staType2 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("Attribute of REL file")), wxVERTICAL);

	wxFlexGridSizer *szrG = new wxFlexGridSizer(3, 4, 4);
	IntNameBox::CreateFileSize(parent, IDC_TEXT_RECSIZE, _("Record Length"), 12, true, tsize, atitle, szrG, txtRecSize);
	txtRecSize->SetMaxLength(3);
	int rec_len = GetRecordLength();
	txtRecSize->SetValue(wxString::Format(wxT("%d"), rec_len));
	txtRecSize->SetEditable((show_flags & INTNAME_NEW_FILE) != 0 && file_type_1 == FILETYPE_MASK_C1541_REL);
	staType2->Add(szrG, flags);

	if ((show_flags & INTNAME_NEW_FILE) == 0) {
		wxFlexGridSizer *szrG = new wxFlexGridSizer(3, 4, 4);
		IntNameBox::CreateFileSize(parent, IDC_TEXT_SIDESEC, _("Size of Side Sector"), 20, true, tsize, atitle, szrG, txtSideSec);
		int sid_size = (int)m_ss_groups.GetSize();
		wxString sid_size_str;
		IntNameBox::ConvFileSize(sid_size, sid_size_str);
		txtSideSec->SetValue(sid_size_str);
		txtSideSec->SetEditable(false);
		staType2->Add(szrG, flags);
	}

	sizer->Add(staType2, flags);
}

/// ダイアログ内の値を設定
void DiskBasicDirItemC1541::InitializeForAttrDialog(IntNameBox *parent, int show_flags, int *user_data)
{
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemC1541::ChangeTypeInAttrDialog(IntNameBox *parent)
{
}

/// 機種依存の属性を設定する
/// @param [in,out] parent  プロパティダイアログ
/// @param [in,out] attr    プロパティの属性値
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemC1541::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicDirItemAttr &attr, DiskBasicError &errinfo) const
{
	wxChoice   *comType1 = (wxChoice *)parent->FindWindow(IDC_COMBO_TYPE1);
	wxTextCtrl *txtRecSize = (wxTextCtrl *)parent->FindWindow(IDC_TEXT_RECSIZE);
	bool valid = true;
	int ori = 0;

	int type1pos = comType1->GetSelection();

	ori = gTypeNameC1541[type1pos].value;

	long rec_siz = 0;
	valid = txtRecSize->GetValue().ToLong(&rec_siz);

	if (type1pos == TYPE_NAME_C1541_REL) {
		// 新規ファイルでREL形式の場合はレコードサイズをチェック
		valid &= (0 < rec_siz && rec_siz < 256);
		if (!valid) {
			errinfo.SetError(DiskBasicError::ERRV_INVALID_VALUE_IN, _("Record Length").t_str());
		}
	} else {
		valid = true;
	}
	ori |= (rec_siz << 8);

	attr.SetFileAttr(basic->GetFormatTypeNumber(), 0, ori);

	return valid;
}

#if 0
/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemC1541::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
//	wxFileName fn(filename);
//	if (fn.GetExt().IsEmpty()) {
//		errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILEEXT_EMPTY]);
//		return false;
//	}
	return true;
}

/// その他の属性値を設定する
void DiskBasicDirItemC1541::SetOptionalAttr(DiskBasicDirItemAttr &attr)
{
}
#endif

/// プロパティで表示する内部データを設定
void DiskBasicDirItemC1541::SetInternalDataInAttrDialog(KeyValArray &vals)
{
	vals.Add(wxT("self"), m_data.IsSelf());
	vals.Add(wxT("(DO_NOT_WRITE)"), m_data.Data()->do_not_write, sizeof(m_data.Data()->do_not_write));
	vals.Add(wxT("TYPE"), m_data.Data()->type);
	vals.Add(wxT("FIRST_DATA.TRACK"), m_data.Data()->first_data.track);
	vals.Add(wxT("FIRST_DATA.SECTOR"), m_data.Data()->first_data.sector);
	vals.Add(wxT("NAME"), m_data.Data()->name, sizeof(m_data.Data()->name));
	vals.Add(wxT("FIRST_SIDE.TRACK"), m_data.Data()->first_side.track);
	vals.Add(wxT("FIRST_SIDE.SECTOR"), m_data.Data()->first_side.sector);
	vals.Add(wxT("RECORD_SIZE"), m_data.Data()->record_size);
	vals.Add(wxT("UNUSED"), m_data.Data()->unused, sizeof(m_data.Data()->unused));
	vals.Add(wxT("REPLACE.TRACK"), m_data.Data()->replace.track);
	vals.Add(wxT("REPLACE.SECTOR"), m_data.Data()->replace.sector);
	vals.Add(wxT("NUM_OF_BLOCKS"), m_data.Data()->num_of_blocks);
}
