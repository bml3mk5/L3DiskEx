/// @file basictype_msdos.cpp
///
/// @brief disk basic type for MS-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_msdos.h"
#include "basicfmt.h"
#include "basicdir.h"


//
//
//
DiskBasicTypeMSDOS::DiskBasicTypeMSDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT12(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMSDOS::CheckFat(bool is_formatting)
{
	// 重複チェック
	double valid_ratio = DiskBasicTypeFAT12::CheckFat(is_formatting);

	if (valid_ratio < 0.0) return valid_ratio;

	// FATエリアの先頭がメディアIDであること
	int match = 0;
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	match = bufs->MatchData8(0, basic->GetMediaId());
	if (match != (basic->GetValidNumberOfFats() > 0 ? basic->GetValidNumberOfFats() : basic->GetNumberOfFats())) {
		valid_ratio -= 0.8;
	}

	// 最終グループ番号を計算
	int max_grp_on_fat = basic->GetSectorsPerFat() * basic->GetSectorSize() * 2 / 3;
	int max_grp_on_prm = (basic->GetSidesPerDiskOnBasic() * basic->GetTracksPerSide() * basic->GetSectorsPerTrackOnBasic() - basic->GetDirEndSector()) / basic->GetSectorsPerGroup() + 1;

	basic->SetFatEndGroup(max_grp_on_fat > max_grp_on_prm ? max_grp_on_prm : max_grp_on_fat);

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMSDOS::ParseParamOnDisk(bool is_formatting)
{
	double valid_ratio = 1.0;

	if (!basic->GetVariousBoolParam(wxT("IgnoreParameter"))) {
		valid_ratio = ParseMSDOSParamOnDisk(basic->GetDisk(), is_formatting);
	} else {
		if (basic->GetFatEndGroup() == 0) {
			int max_grp_on_prm = (basic->GetSidesPerDiskOnBasic() * basic->GetTracksPerSide() * basic->GetSectorsPerTrackOnBasic() - basic->GetDirEndSector()) / basic->GetSectorsPerGroup() + 1;
			basic->SetFatEndGroup(max_grp_on_prm);
		}
	}

	wxCharBuffer ipl = basic->GetVariousStringParam(wxT("IPLCompareString")).To8BitData();
	if (ipl.length() > 0) {
		DiskD88Sector *sector = basic->GetSector(0, 0, 1);
		if (!sector) return -1.0;
		if (sector->Find(ipl.data(), ipl.length()) < 0) {
			valid_ratio = 0.0;
		}
	}

	return valid_ratio;
}

/// ディスクからMS-DOSパラメータを取得＆必要なパラメータを計算
/// @param [in] disk          ディスク
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0>      正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMSDOS::ParseMSDOSParamOnDisk(DiskD88Disk *disk, bool is_formatting)
{
	if (is_formatting) return 1.0;

	int nums = 0;
	int valids = 0;

	// MS-DOS ディスク上のパラメータを読む
	DiskD88Sector *sector = disk->GetSector(0, 0, 1);
	if (!sector) return -1.0;
	wxUint8 *datas = sector->GetSectorBuffer();
	if (!datas) return -1.0;
	fat_bpb_t *bpb = (fat_bpb_t *)datas;

	nums++;
	if (bpb->BPB_SecPerClus != 0) {
		// クラスタサイズ
		valids++;
	}
	nums++;
	if (disk->GetSectorSize() == wxUINT16_SWAP_ON_BE(bpb->BPB_BytsPerSec)) {
		// セクタサイズ
		valids++;
	}
	nums++;
	if (disk->GetSidesPerDisk() >= wxUINT16_SWAP_ON_BE(bpb->BPB_NumHeads)) {
		// サイド数
		valids++;
	}
	nums++;
	if (basic->GetSidesPerDiskOnBasic() == wxUINT16_SWAP_ON_BE(bpb->BPB_NumHeads)) {
		// BASICテンプレートのサイド数
		valids++;
	}
	nums++;
	if (basic->GetSectorsPerTrackOnBasic() == wxUINT16_SWAP_ON_BE(bpb->BPB_SecPerTrk)) {
		// BASICテンプレートのセクタ数
		valids++;
	}

	if (nums == valids) {
		basic->SetSidesPerDiskOnBasic(wxUINT16_SWAP_ON_BE(bpb->BPB_NumHeads));
		basic->SetSectorsPerGroup(bpb->BPB_SecPerClus);
		basic->SetReservedSectors(wxUINT16_SWAP_ON_BE(bpb->BPB_RsvdSecCnt));
		basic->SetNumberOfFats(bpb->BPB_NumFATs);
		basic->SetSectorsPerFat(wxUINT16_SWAP_ON_BE(bpb->BPB_FATSz16));
		basic->SetDirEntryCount(wxUINT16_SWAP_ON_BE(bpb->BPB_RootEntCnt));

		basic->SetDirStartSector(-1);
		basic->SetDirEndSector(-1);
		basic->CalcDirStartEndSector(wxUINT16_SWAP_ON_BE(bpb->BPB_BytsPerSec));

		basic->SetSectorsPerTrackOnBasic(wxUINT16_SWAP_ON_BE(bpb->BPB_SecPerTrk));

		basic->SetMediaId(bpb->BPB_Media);

		// トラック数
		int tracks_per_side = wxUINT16_SWAP_ON_BE(bpb->BPB_TotSec16);
		if (tracks_per_side > 0) {
			tracks_per_side = tracks_per_side / basic->GetSidesPerDiskOnBasic() / basic->GetSectorsPerTrackOnBasic();
			basic->SetTracksPerSideOnBasic(tracks_per_side);
		}
	}

	// 最終グループ番号を計算
	int max_grp_on_fat = basic->GetSectorsPerFat() * basic->GetSectorSize() * 2 / 3;
//	int max_grp_on_prm = ((basic->GetSidesPerDiskOnBasic() * disk->GetTracksPerSide() * basic->GetSectorsPerTrackOnBasic() - basic->GetDirEndSector()) / basic->GetSectorsPerGroup());
	int max_grp_on_prm = (basic->GetSidesPerDiskOnBasic() * basic->GetTracksPerSide() * basic->GetSectorsPerTrackOnBasic() - basic->GetDirEndSector()) / basic->GetSectorsPerGroup() + 1;

	basic->SetFatEndGroup(max_grp_on_fat > max_grp_on_prm ? max_grp_on_prm : max_grp_on_fat);

	// テンプレートに一致するものがあるか
	const DiskBasicParam *param = gDiskBasicTemplates.FindType(basic->GetBasicCategoryName(), basic->GetBasicTypeName(), basic->GetSidesPerDiskOnBasic(), basic->GetSectorsPerTrackOnBasic());
	if (param) {
		basic->SetBasicDescription(param->GetBasicDescription());
	}

	double valid_ratio = 0.0;
	if (nums > 0) {
		valid_ratio = (double)valids/nums;
	}
	return valid_ratio;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット IPLの書き込み
bool DiskBasicTypeMSDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	if (!CreateBiosParameterBlock("\xeb\x3c\x90", "FAT12")) {
		return false;
	}

	// ボリュームラベルを設定
	const DiskBasicFormat *fmt = basic->GetFormatType();
	if (fmt->HasVolumeName()) {
		int dir_start = basic->GetReservedSectors() + basic->GetNumberOfFats() * basic->GetSectorsPerFat();
		DiskD88Sector *sec = basic->GetSectorFromSectorPos(dir_start);
		DiskBasicDirItem *ditem = dir->NewItem(sec, 0, sec->GetSectorBuffer());

		ditem->SetFileNamePlain(data.GetVolumeName());
		ditem->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_VOLUME_MASK);
		TM tm;
		wxDateTime::GetTmNow(tm);
		ditem->SetFileCreateDateTime(tm);
		delete ditem;
	}

	return true;
}

/// BIOS Parameter Block を作成
bool DiskBasicTypeMSDOS::CreateBiosParameterBlock(const char *jmp, const char *name, wxUint8 **sec_buf)
{
	DiskD88Sector *sec = basic->GetSector(0, 0, 1);
	if (!sec) return false;
	wxUint8 *buf = sec->GetSectorBuffer();
	if (!buf) return false;

	if(sec_buf) *sec_buf = buf;

	sec->Fill(0);

	fat_bpb_t *hed = (fat_bpb_t *)buf;

	size_t len;

	wxCharBuffer s_jmp = basic->GetVariousStringParam(wxT("JumpBoot")).To8BitData();
	if (s_jmp.length() > 0) {
		jmp = s_jmp.data();
	}
	len = strlen(jmp) < sizeof(hed->BS_JmpBoot) ? strlen(jmp) : sizeof(hed->BS_JmpBoot);
	memcpy(hed->BS_JmpBoot, jmp, len);

	hed->BPB_BytsPerSec = wxUINT16_SWAP_ON_BE(basic->GetSectorSize());
	hed->BPB_SecPerClus = basic->GetSectorsPerGroup();
	hed->BPB_RsvdSecCnt = wxUINT16_SWAP_ON_BE(basic->GetReservedSectors());
	hed->BPB_NumFATs = basic->GetNumberOfFats();
	hed->BPB_RootEntCnt = wxUINT16_SWAP_ON_BE(basic->GetDirEntryCount());

	wxCharBuffer s_name = basic->GetVariousStringParam(wxT("OEMName")).To8BitData();
	if (s_name.length() > 0) {
		name = s_name.data();
	}
	// 上記パラメータ領域をまたがって設定可能にする
	len = strlen(name) < 16 ? strlen(name) : 16;
	memset(hed->BS_OEMName, 0x20, sizeof(hed->BS_OEMName));
	memcpy(hed->BS_OEMName, name, len);

	len = basic->GetTracksPerSide() * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic();
	hed->BPB_TotSec16 = wxUINT16_SWAP_ON_BE(len);
	hed->BPB_Media = basic->GetMediaId();
	hed->BPB_FATSz16 = wxUINT16_SWAP_ON_BE(basic->GetSectorsPerFat());
	hed->BPB_SecPerTrk =  wxUINT16_SWAP_ON_BE(basic->GetSectorsPerTrackOnBasic());
	hed->BPB_NumHeads = wxUINT16_SWAP_ON_BE(basic->GetSidesPerDiskOnBasic());

	// FATの先頭にメディアIDをセット
	SetGroupNumber(0, 0xffffff00 | basic->GetMediaId());
	SetGroupNumber(1, 0xffffffff);

	return true;
}

/// ルートディレクトリか
bool DiskBasicTypeMSDOS::IsRootDirectory(wxUint32 group_num)
{
	// オフセット未満だったらルート
	return (group_num <= 1);
}

/// サブディレクトリを作成する前にディレクトリ名を編集する
bool DiskBasicTypeMSDOS::RenameOnMakingDirectory(wxString &dir_name)
{
	// 空や"."で始まるディレクトリは作成不可
	if (dir_name.IsEmpty() || dir_name.Left(1) == wxT(".")) {
		return false;
	}
	return true;
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeMSDOS::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	if (group_items.Count() <= 0) return;

	// ファイルサイズをクリア
	item->SetFileSize(0);

	// カレントと親ディレクトリのエントリを作成する
	DiskBasicGroupItem *gitem = &group_items.Item(0);

	DiskD88Sector *sector = basic->GetDisk()->GetSector(gitem->track, gitem->side, gitem->sector_start);

	wxUint8 *buf = sector->GetSectorBuffer();
	DiskBasicDirItem *newitem = basic->CreateDirItem(sector, 0, buf);

	// カレント
	newitem->CopyData(item->GetData());
	newitem->SetFileNamePlain(wxT("."));
	newitem->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_DIRECTORY_MASK);

	// 親
	buf += newitem->GetDataSize();
	newitem->SetDataPtr(0, NULL, sector, 0, buf);
	if (parent_item) {
		// 親がサブディレクトリ
		newitem->CopyData(parent_item->GetData());
	} else {
		// 親がルート
		newitem->CopyData(item->GetData());
		newitem->SetStartGroup(0, 0);
	}
	newitem->SetFileCreateDateTime(item->GetFileCreateDateTime());
	newitem->SetFileNamePlain(wxT(".."));
	newitem->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_DIRECTORY_MASK);

	delete newitem;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeMSDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// volume label
	DiskBasicDirItem *ditem = dir->FindFileByAttrOnRoot(FILE_TYPE_VOLUME_MASK, FILE_TYPE_VOLUME_MASK | FILE_TYPE_DIRECTORY_MASK);
	if (ditem && ditem->IsUsed()) {
		data.SetVolumeName(ditem->GetFileNameStr());
		data.SetVolumeNameMaxLength(ditem->GetFileNameStrSize());
	}
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeMSDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	const DiskBasicFormat *fmt = basic->GetFormatType();

	// volume label
	if (fmt->HasVolumeName()) {
		ModifyOrMakeVolumeLabel(data.GetVolumeName());
	}
}

/// ボリュームラベルを更新 なければ作成
bool DiskBasicTypeMSDOS::ModifyOrMakeVolumeLabel(const wxString &filename)
{
	DiskBasicDirItem *next_item;
	// ボリュームラベルがあるか
	DiskBasicDirItem *item = dir->FindFileByAttrOnRoot(FILE_TYPE_VOLUME_MASK, FILE_TYPE_VOLUME_MASK | FILE_TYPE_DIRECTORY_MASK);
	if (!item) {
		// 新しいディレクトリアイテムを確保
		if ((item = dir->GetEmptyItemOnRoot(NULL, &next_item)) == NULL) {
			// 確保できない時
			return false;
		} else {
			item->SetEndMark(next_item);
		}
		item->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_VOLUME_MASK, 0);
	}
	item->SetFileNameStr(filename);
	item->Used(true);

	return true;
}
