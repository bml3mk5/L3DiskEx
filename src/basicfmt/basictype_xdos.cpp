/// @file basictype_xdos.cpp
///
/// @brief disk basic type for X-DOS for X1
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_xdos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_xdos.h"
#include "../charcodes.h"


#pragma pack(1)
/// 使用状況セクタ
struct st_fat_xdos {
	wxUint8  use[0x0a8];
	wxUint8  map[0x158];
};
#pragma pack()

#define XDOS_FAT_START	0xa8
//
//
//
DiskBasicTypeXDOS::DiskBasicTypeXDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeXDOS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	if (num > basic->GetFatEndGroup()) {
		return;
	}

	wxUint32 pos = num / basic->GetSectorsPerTrackOnBasic();	// 丸める
	pos *= 2;
	wxUint32 mask = (0x8000 >> (num % basic->GetSectorsPerTrackOnBasic()));
	if (mask >= 0x100) {
		mask >>= 8;
	} else {
		pos++;
	}

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return;
	}
	// FATには使用状況テーブルがある
	pos += XDOS_FAT_START;
	fatbuf->Bit(pos, (wxUint8)mask, val == 0, basic->IsDataInverted());
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeXDOS::IsUsedGroupNumber(wxUint32 num)
{
	bool exist = false;

	if (num > basic->GetFatEndGroup()) {
		return false;
	}

	wxUint32 pos = num / basic->GetSectorsPerTrackOnBasic();	// 丸める
	pos *= 2;
	wxUint32 mask = (0x8000 >> (num % basic->GetSectorsPerTrackOnBasic()));
	if (mask >= 0x100) {
		mask >>= 8;
	} else {
		pos++;
	}

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return true;
	}
	// FATには使用状況テーブルがある
	pos += XDOS_FAT_START;
	exist = !fatbuf->BitTest(pos, (wxUint8)mask, basic->IsDataInverted());
	return exist;
}

/// 空き位置を返す
/// @return 0xff:空きなし
wxUint32 DiskBasicTypeXDOS::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return new_num;
	}
	// 空き位置をさがす
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup(); gnum++) {
		if (!IsUsedGroupNumber(gnum)) {
			new_num = gnum;
			break;
		}
	}
	return new_num;
}

/// 未使用が連続している位置をさがす
wxUint32 DiskBasicTypeXDOS::GetContinuousArea(wxUint32 group_size)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return new_num;
	}
	wxUint32 step = basic->GetSectorsPerTrackOnBasic();
	wxUint32 cnt = 0;
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup() && cnt < group_size;) {
		if (!IsUsedGroupNumber(gnum)) {
			if (cnt == 0) {
				new_num = gnum;
			}
			cnt++;
			gnum++;
		} else {
			new_num = INVALID_GROUP_NUMBER;
			cnt = 0;
			// 各トラックの先頭までスキップ
			gnum = ((gnum + step) / step) * step;
		}
	}
	return new_num;
}

/// 次の空き位置を返す 未使用
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeXDOS::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	return GetEmptyGroupNumber();
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeXDOS::ParseParamOnDisk(bool is_formatting)
{
	// グループ数
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		basic->SetFatEndGroup(end_group - 1);
	}
	return 1.0;
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeXDOS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	wxUint8 hed[2];
	hed[0] = 1;
	hed[1] = 0x0a; // (wxUint8)basic->GetSectorsPerTrackOnBasic();
	hed[1] |= 0x40;
	DiskImageSector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
	if (sector) {
		if (sector->Find(hed, 2) < 0) {
			return -1.0;
		}

		// セクタ数チェック
		for(int i=2; i<basic->GetTracksPerSide() * basic->GetSidesPerDiskOnBasic(); i++) {
			if (sector->Get(i) != hed[1]) {
				valid_ratio -= 0.5;
				if (valid_ratio < 0.0) break;
			}
		}
	}
	return valid_ratio;
}

/// 使用可能なディスクサイズを得る
/// @param [out] disk_size  ディスクサイズ
/// @param [out] group_size グループ数
void DiskBasicTypeXDOS::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1 - basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
void DiskBasicTypeXDOS::CalcDiskFreeSize(bool wrote)
{
	fat_availability.Empty();
	fat_availability.SetCount(basic->GetFatEndGroup() + 1, FAT_AVAIL_USED);

	// Allocation Mapを調べる
	int grps = 0;
	wxUint32 gnum = 0;
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) return;
	for(size_t pos = XDOS_FAT_START; pos < fatbuf->GetSize(); pos+=2) {
		wxUint16 dat = (wxUint16)((fatbuf->Get(pos) << 8) | fatbuf->Get(pos+1));
		for(int bit = 0; bit < basic->GetSectorsPerTrackOnBasic() && gnum <= basic->GetFatEndGroup(); bit++) {
			bool used = ((dat & (0x8000 >> bit)) == 0);
			if (!used) {
				fat_availability.Set(gnum, FAT_AVAIL_FREE);
				grps++;
			}
			gnum++;
		}
	}

	// ディレクトリエントリのグループ
	const DiskBasicDirItems *items = dir->GetCurrentItems();
	if (!items) return;
	for(size_t idx = 0; idx < items->Count(); idx++) {
		DiskBasicDirItem *item = items->Item(idx);
		if (!item || !item->IsUsed()) continue;
		size_t gcnt = item->GetGroupCount();
		if (gcnt > 0) {
			const DiskBasicGroupItem *gitem = item->GetGroup(gcnt - 1);
			gnum = gitem->group;
			if (gnum <= basic->GetFatEndGroup()) {
				fat_availability.Set(gnum, FAT_AVAIL_USED_LAST);
			}
		}
	}

	// 空きをチェック
	wxUint32 gend = basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	for(wxUint32 pos = 0; pos < gend; pos++) {
		// ディレクトリエリアは使用済み
		fat_availability.Set(pos, FAT_AVAIL_SYSTEM);
	}

	int fsize = grps * basic->GetSectorSize() * basic->GetSectorsPerGroup();

	fat_availability.SetFreeSize(fsize);
	fat_availability.SetFreeGroups(grps);
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeXDOS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	// Chain用のセクタを確保する
	wxUint32 gnum = GetEmptyGroupNumber();
	if (gnum == INVALID_GROUP_NUMBER) {
		return false;
	}
	// セクタ
	DiskImageSector *sector = basic->GetSectorFromGroup(gnum);
	if (!sector) {
		return false;
	}
	wxUint8 *buf = sector->GetSectorBuffer();
	if (!buf) {
		return false;
	}
	sector->Fill(basic->InvertUint8(0));

	// チェイン情報にセクタをセット
	nitem->SetChainSector(sector, buf);

	// 開始グループを設定
	nitem->SetStartGroup(0, gnum, 1);

	// セクタを予約
	SetGroupNumber(gnum, 1);

	return true;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    確保するデータサイズ（バイト）
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeXDOS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
//	int file_size = 0;
//	int groups = 0;

	int rc = 0;
	int sec_size = basic->GetSectorSize();
	int remain = data_size;
	int limit = basic->GetFatEndGroup() + 1;
	int chain_idx = -1;
	int prev_trk = -1;
	wxUint32 group_num = INVALID_GROUP_NUMBER;

	if (item->GetFileAttr().IsDirectory()) {
		// ディレクトリ作成の場合、連続した空き領域をさがす
		group_num = GetContinuousArea(basic->GetSubDirGroupSize());
	} else {
		group_num = GetEmptyGroupNumber();
	}
	if (group_num == INVALID_GROUP_NUMBER) {
		// 空きなし
		rc = -1;
		return rc;
	}
	while(remain > 0 && limit >= 0) {
		// 使用しているか
		bool used = IsUsedGroupNumber(group_num);
		if (!used) {
			// 使用済みにする
			basic->GetNumsFromGroup(group_num, 0, sec_size, remain, group_items);
			SetGroupNumber(group_num, 1);
			// チェインセクタも更新
			int trk = group_num / basic->GetSectorsPerTrackOnBasic();
			if (trk != prev_trk) {
				chain_idx++;
			}
			prev_trk = trk;
			item->AddChainGroupNumber(chain_idx, group_num);

//			file_size += (sec_size * basic->GetSectorsPerGroup());
//			groups++;

			remain -= (sec_size * basic->GetSectorsPerGroup());
		}
		// グループ番号はなるべく連続するように設定
		group_num++;
		limit--;
	}
	if (limit < 0) {
		// 無限ループ？
		rc = -2;
	}
	return rc;
}

/// ルートディレクトリか
bool DiskBasicTypeXDOS::IsRootDirectory(wxUint32 group_num)
{
	return ((int)group_num < basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic());
}

/// サブディレクトリを作成する前にディレクトリ名を編集する
bool DiskBasicTypeXDOS::RenameOnMakingDirectory(wxString &dir_name)
{
	// "!"は作成不可
	if (dir_name == wxT("!")) {
		return false;
	}
	return true;
}

/// サブディレクトリを作成する前の準備を行う
bool DiskBasicTypeXDOS::PrepareToMakeDirectory(DiskBasicDirItem *item)
{
	return true;
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeXDOS::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	if (group_items.Count() == 0) return;

	const DiskBasicGroupItem *group = group_items.ItemPtr(0);
	item->SetStartGroup(0, group->group, basic->GetSubDirGroupSize());

	DiskImageSector *sector = basic->GetSector(group->track, group->side, group->sector_start);
	if (!sector) return;

	wxUint8 *buf = sector->GetSectorBuffer();
	if (!buf) return;

	// セクタの先頭をクリア
	sector->Fill(0, basic->GetDirStartPos());
	// セクタ名を設定
	wxUint8 name[32];
	int nlen = (int)sizeof(name);
	memset(name, 0, nlen);
	item->GetFileName(name, nlen);
	sector->Copy(name, nlen);
	// サイズはクリア
	item->SetFileSize(0);

	// 親をつくる
	buf += basic->GetDirStartPos();
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, basic->GetDirStartPos(), buf);
	newitem->ClearData();
	newitem->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_DIRECTORY_MASK);

	wxUint32 parent_group = INVALID_GROUP_NUMBER;
	if (parent_item) {
		// 親がサブディレクトリ
		parent_group = parent_item->GetStartGroup(0);
	}
	if (parent_group == INVALID_GROUP_NUMBER) {
		// ルート
		parent_group = basic->GetDirStartSector() - 1;
	}
	newitem->SetStartGroup(0, parent_group);
	newitem->SetFileNamePlain(wxT("!"));

	delete newitem;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeXDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// IPL
	DiskImageSector *sector = NULL;
	sector = basic->GetSectorFromSectorPos(0);
	if (sector) {
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnDir()));
		wxCharBuffer ipl = basic->GetVariousStringParam(wxT("IPLString")).To8BitData();
		int len = (int)ipl.length();
		if (len > 0) {
			if (len > 32) len = 32;
			basic->InvertMem(ipl.data(), len);
			sector->Copy(ipl.data(), len);
		}
	}

	// FAT
	sector = basic->GetSectorFromSectorPos(basic->GetFatStartSector() - 1);
	if (sector) {
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));
		// セクタ数
		wxUint8 val = 0x4a; // | (basic->GetSectorsPerTrackOnBasic());
		int trks = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic();
		sector->Fill(val, trks, 0);
		// トラック0は予約
		sector->Fill(basic->GetParamDensity() >> 4, 1, 0);
		sector->Fill(1, 1, 1);
		// 使用状況
		wxUint32 mapi = (1 << basic->GetSectorsPerTrackOnBasic()) - 1;
		mapi <<= (16 - basic->GetSectorsPerTrackOnBasic());
		wxUint8 map[2];
		map[0] = ((mapi >> 8) & 0xff);
		map[1] = (mapi & 0xff);

		sector->Fill(0, 4, XDOS_FAT_START);	// トラック0
		for(int i=2; i<trks; i++) {
			sector->Copy(map, 2, i * 2 + XDOS_FAT_START);
		}
	}

	// DIR
	for(int pos=basic->GetDirStartSector(); pos<=basic->GetDirEndSector(); pos++) {
		sector = basic->GetSectorFromSectorPos(pos - 1);
		if (sector) {
			sector->Fill(basic->InvertUint8(basic->GetFillCodeOnDir()));
			if (pos == basic->GetDirStartSector()) {
				sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()), basic->GetDirStartPosOnRoot());
			}
		}
	}

	// タイトルラベル
	SetIdentifiedData(data);

	return true;
}

//
// for data access
//

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeXDOS::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	if (item->NeedCheckEofCode()) {
		// 終端コード($00)の1つ前までを出力
		wxUint8 eof_code = basic->InvertUint8(basic->GetTextTerminateCode());
		for(int len = 0; len < remain_size; len++) {
			if (sector_buffer[len] == eof_code) {
				remain_size = len;
				break;
			}
		}
	}
	return remain_size;
}

//
// for delete
//

/// 指定したグループ番号のFAT領域を削除する
/// @param [in] group_num グループ番号
void DiskBasicTypeXDOS::DeleteGroupNumber(wxUint32 group_num)
{
	// 未使用にする
	SetGroupNumber(group_num, 0);
}

/// ファイル削除後の処理
bool DiskBasicTypeXDOS::AdditionalProcessOnDeletedFile(DiskBasicDirItem *item)
{
	// チェインセクタを未使用にする
	SetGroupNumber(item->GetStartGroup(0), 0);
	return true;
}

#define VOLUME_NAME_LENGTH	80

/// IPLや管理エリアの属性を得る
void DiskBasicTypeXDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// タイトル名 DIRエリアの最初
	DiskImageSector *sector = basic->GetSectorFromSectorPos(basic->GetDirStartSector() - 1);
	if (sector) {
		wxString dst;
		basic->GetCharCodes().ConvToString(sector->GetSectorBuffer(), VOLUME_NAME_LENGTH, dst, 0);
		data.SetVolumeName(dst);
		data.SetVolumeNameMaxLength(VOLUME_NAME_LENGTH);
	}
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeXDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	// タイトル名 DIRエリアの最初
	if (basic->GetFormatType()->HasVolumeName()) {
		DiskImageSector *sector = basic->GetSectorFromSectorPos(basic->GetDirStartSector() - 1);
		if (sector) {
			wxUint8 dst[VOLUME_NAME_LENGTH + 1];
			memset(dst, 0, sizeof(dst));
			int l = basic->GetCharCodes().ConvToChars(data.GetVolumeName(), dst, sizeof(dst));
			if (l > 0) {
				if (l > VOLUME_NAME_LENGTH) {
					l = VOLUME_NAME_LENGTH;
				}
				sector->Copy(dst, VOLUME_NAME_LENGTH);
			}
		}
	}
}
