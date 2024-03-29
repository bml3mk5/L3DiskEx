/// @file basictype_magical.cpp
///
/// @brief disk basic type for Magical DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_magical.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_magical.h"


#define MAGICAL_FAT_START	0xa8
//
//
//
DiskBasicTypeMAGICAL::DiskBasicTypeMAGICAL(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeXDOS(basic, fat, dir)
{
}

/// 未使用が連続している位置をさがす
wxUint32 DiskBasicTypeMAGICAL::GetContinuousArea(wxUint32 group_size)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return new_num;
	}
	wxUint32 cnt = 0;
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup() && cnt < group_size; gnum++) {
		if (!IsUsedGroupNumber(gnum)) {
			if (cnt == 0) {
				new_num = gnum;
			}
			cnt++;
		} else {
			new_num = INVALID_GROUP_NUMBER;
			cnt = 0;
		}
	}
	return new_num;
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMAGICAL::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	wxUint8 hed[2];
	hed[0] = 0;
	hed[1] = (wxUint8)basic->GetSectorsPerTrackOnBasic();
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

/// サブディレクトリを作成する前の準備を行う
bool DiskBasicTypeMAGICAL::PrepareToMakeDirectory(DiskBasicDirItem *item)
{
	return true;
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeMAGICAL::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
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

	wxUint32 parent_group = INVALID_GROUP_NUMBER;
	if (parent_item) {
		// 親がサブディレクトリ
		parent_group = parent_item->GetStartGroup(0);
	}
	if (parent_group == INVALID_GROUP_NUMBER) {
		// ルート
		parent_group = basic->GetDirStartSector() - 1;
	}

	// サイズ
	int trk, sid, sec;
	basic->CalcNumFromSectorPosForGroup(parent_group, trk, sid, sec);
	xdos_seg_t s;
	s.track = trk * basic->GetSidesPerDiskOnBasic() + sid;
	s.sector = sec;
	s.size = 1 + basic->GetDirEndSector() - basic->GetDirStartSector();
	sector->Copy(&s, 3, 0x71);
	s.track = group->track * basic->GetSidesPerDiskOnBasic() + group->side;
	s.sector = group->sector_start;
	sector->Copy(&s, 3, 0x74);
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeMAGICAL::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// IPL
	DiskImageSector *sector = NULL;
	sector = basic->GetSectorFromSectorPos(0);
	if (sector) {
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));	// invert
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
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));	// invert
		// セクタ数
		wxUint8 val = 0x40 | (basic->GetSectorsPerTrackOnBasic());
		int trks = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic();
		sector->Fill(val, trks, 0);
		// トラック0は予約
		sector->Fill(basic->GetParamDensity() >> 4, 1, 0);
		sector->Fill(0, 1, 1);
		// 使用状況
		wxUint32 mapi = (1 << basic->GetSectorsPerTrackOnBasic()) - 1;
		mapi <<= (16 - basic->GetSectorsPerTrackOnBasic());
		wxUint8 map[2];
		map[0] = ((mapi >> 8) & 0xff);
		map[1] = (mapi & 0xff);

		sector->Fill(0, 4, MAGICAL_FAT_START);	// トラック0
		for(int i=2; i<trks; i++) {
			sector->Copy(map, 2, i * 2 + MAGICAL_FAT_START);
		}
	}

	// DIR
	for(int pos=basic->GetDirStartSector(); pos<=basic->GetDirEndSector(); pos++) {
		sector = basic->GetSectorFromSectorPos(pos - 1);
		if (sector) {
			sector->Fill(basic->InvertUint8(basic->GetFillCodeOnDir()));
			if (pos == basic->GetDirStartSector()) {
				sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()), basic->GetDirStartPosOnRoot());
				// サイズ
				int trk, sid, sec;
				basic->CalcNumFromSectorPosForGroup(pos - 1, trk, sid, sec);
				xdos_seg_t s;
				s.track = trk * basic->GetSidesPerDiskOnBasic() + sid;
				s.sector = sec;
				s.size = 1 + basic->GetDirEndSector() - basic->GetDirStartSector();
				sector->Copy(&s, 3, 0x74);
			}
		}
	}

	// タイトルラベル
	SetIdentifiedData(data);

	return true;
}
