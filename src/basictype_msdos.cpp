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

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] disk          ディスク
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0  正常
/// @retval <1.0 警告あり
/// @retval <0.0 エラーあり
double DiskBasicTypeMSDOS::ParseParamOnDisk(DiskD88Disk *disk, bool is_formatting)
{
	double valid_ratio = 1.0;

	if (!basic->GetVariousBoolParam(wxT("IgnoreParameter"))) {
		valid_ratio = DiskBasicTypeFAT12::ParseParamOnDisk(disk, is_formatting);
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

/// セクタデータを埋めた後の個別処理
/// フォーマット IPLの書き込み
bool DiskBasicTypeMSDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	if (!DiskBasicTypeFAT12::AdditionalProcessOnFormatted(data)) {
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
		struct tm tm;
		wxDateTime::GetTmNow(&tm);
		ditem->SetFileDateTime(&tm);
		delete ditem;
	}

	return true;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeMSDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// volume label
	DiskBasicDirItem *ditem = dir->FindFileByAttrOnRoot(FILE_TYPE_VOLUME_MASK, FILE_TYPE_VOLUME_MASK | FILE_TYPE_DIRECTORY_MASK);
	if (ditem && ditem->IsUsed()) {
		data.SetVolumeName(ditem->GetFileNameStr());
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
		if ((item = dir->GetEmptyItemOnRoot(&next_item)) == NULL) {
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
