/// @file basictype_smc.cpp
///
/// @brief disk basic type for SMC-777 Sony Filer
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_smc.h"
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem_cpm.h"


//
//
//
DiskBasicTypeSMC::DiskBasicTypeSMC(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeCPM(basic, fat, dir)
{
	// システムは仮想的にインターリーブを持っている
	int vinter = 3;
	int secs = basic->GetSectorsPerTrackOnBasic();

	vtop_map = new wxUint8[secs];
	ptov_map = new wxUint8[secs];

	memset(vtop_map, 0, secs);
	memset(ptov_map, 0, secs);

	int p = 1;
	for(int i = 0; i < secs; i++) {
		vtop_map[i]   = (wxUint8)p;
		ptov_map[p-1] = (wxUint8)(i + 1);
		p += vinter;
		if (p > secs) {
			p -= secs;
			for(int limit = secs + 1; ptov_map[p-1] != 0 && !limit; limit--) {
				p++;
			}
		}
	}
}

DiskBasicTypeSMC::~DiskBasicTypeSMC()
{
	delete [] vtop_map;
	delete [] ptov_map;
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos    セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num    トラック番号
/// @param [out] side_num     サイド番号
/// @param [out] sector_num   セクタ番号
/// @param [out] div_num      分割番号
/// @param [out] div_nums     分割数
void DiskBasicTypeSMC::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num, int *div_num, int *div_nums)
{
	int selected_side = basic->GetSelectedSide();
//	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();

	if (selected_side >= 0) {
		// 1S
		track_num = sector_pos / sectors_per_track;
		side_num = selected_side;
	} else {
		// 2D, 2HD
		track_num = sector_pos / sectors_per_track / sides_per_disk;
		side_num = (sector_pos / sectors_per_track) % sides_per_disk;
	}
	sector_num = (sector_pos % sectors_per_track) + 1;

	// マッピング
	sector_num = vtop_map[sector_num - 1];

	if (div_num)  *div_num = 0;
	if (div_nums) *div_nums = 1;
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num   トラック番号
/// @param [in] side_num    サイド番号
/// @param [in] sector_num  セクタ番号
/// @param [in] div_num     分割番号
/// @param [in] div_nums    分割数
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasicTypeSMC::GetSectorPosFromNum(int track_num, int side_num, int sector_num, int div_num, int div_nums)
{
	int selected_side = basic->GetSelectedSide();
	int numbering_sector = basic->GetNumberingSector();
	int sectors_per_track = basic->GetSectorsPerTrackOnBasic();
	int sides_per_disk = basic->GetSidesPerDiskOnBasic();
	int sector_pos;

	// マッピング
	sector_num = ptov_map[sector_num - 1];

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * sectors_per_track + sector_num - 1;
	} else {
		// 2D, 2HD
		sector_pos = track_num * sectors_per_track * sides_per_disk;
		sector_pos += (side_num % sides_per_disk) * sectors_per_track;
		if (numbering_sector == 1) {
			sector_pos += ((sector_num - 1) % sectors_per_track);
		} else {
			sector_pos += (sector_num - 1);
		}
	}
	return sector_pos;
}
