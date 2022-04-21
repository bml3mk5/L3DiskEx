/// @file basictype_msdos.cpp
///
/// @brief disk basic fat type for MS-DOS
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

/// セクタデータを埋めた後の個別処理
/// フォーマット IPLの書き込み
bool DiskBasicTypeMSDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	if (!DiskBasicTypeFAT12::AdditionalProcessOnFormatted(data)) {
		return false;
	}

	// ボリュームラベルを設定
	int dir_start = basic->GetReservedSectors() + basic->GetNumberOfFats() * basic->GetSectorsPerFat();
	DiskD88Sector *sec = basic->GetSectorFromSectorPos(dir_start);
	DiskBasicDirItem *ditem = dir->NewItem(sec, sec->GetSectorBuffer());

	ditem->SetFileNamePlain(data.GetVolumeName());
	ditem->SetFileAttr(FILE_TYPE_VOLUME_MASK);
	struct tm tm;
	wxDateTime::GetTmNow(&tm);
	ditem->SetFileDateTime(&tm);

	delete ditem;

	return true;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeMSDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeMSDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
}
