/// @file diskdimparser.cpp
///
/// @brief DIFC.X DIMディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskdimparser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"


#pragma pack(1)
/// DIFC.X DIMヘッダ
typedef struct st_dim_dsk_header {
	wxUint8  type;
	wxUint8  tracks[0xaa];
	wxUint8  ident[13];
	wxUint8  unknown[0x48];
	wxUint8  checkh;
	wxUint8  checkl;
} dim_dsk_header_t;
#pragma pack()

#define DISK_DIM_HEADER "DIFC HEADER  "

//
//
//
DiskDIMParser::DiskDIMParser(DiskImageFile *file, short mod_flags, DiskResult *result)
	: DiskPlainParser(file, mod_flags, result)
{
}

DiskDIMParser::~DiskDIMParser()
{
}

/// DIMファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param ディスクパラメータ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskDIMParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	if (!disk_param) {
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return p_result->GetValid();
	}

	istream.SeekI(0);

	dim_dsk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len != sizeof(header)) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return p_result->GetValid();
	}

	istream.SeekI(0x100);

	return DiskPlainParser::Parse(istream, disk_param);
}

/// チェック
/// @param [in] istream       解析対象データ
/// @param [in] disk_hints    ディスクパラメータヒント("2D"など)
/// @param [in] disk_param    ディスクパラメータ disk_hints指定時はNullable
/// @param [out] disk_params  ディスクパラメータの候補
/// @param [out] manual_param 候補がないときのパラメータヒント
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskDIMParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	istream.SeekI(0);

	dim_dsk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return p_result->GetValid();
	}
	// ヘッダ文字列チェック
	if (memcmp(header.ident, DISK_DIM_HEADER, sizeof(DISK_DIM_HEADER)) != 0) {
		// not disk
		return -1;
	}

	int stream_size = (int)istream.GetLength() - 0x100;
	int sides_per_disk = 2;
	int sector_size = 1024;
	int sectors_per_track = 8;

	// トラック数を数える
	int tracks_per_side = 0;
	for(size_t n=0; n<sizeof(header.tracks); n++) {
		if (header.tracks[n] == 0) break;
		tracks_per_side++;
	}
	if (tracks_per_side == 0) {
		tracks_per_side = 144;
	}

	// セクタ数
	int secs_per_trk_mod = (stream_size / sector_size) % tracks_per_side;
	if (secs_per_trk_mod == 0) {
		// decide
		sectors_per_track = (stream_size / sector_size) / tracks_per_side;
	} else {
		for(int i=8; i<10; i++) {
			if (((stream_size / sector_size) % i) == 0) {
				sectors_per_track = i;
				break;
			}
		}
	}

	tracks_per_side /= sides_per_disk;

	// ディスクテンプレートから探す
	DiskParam dummy;

	if (disk_hints != NULL) {
		// パラメータヒントあり

		// 優先順位の高い候補
		for(size_t i=0; i<disk_hints->Count(); i++) {
			int kind = disk_hints->Item(i).GetKind();
			if (kind != header.type) {
				continue;
			}
			wxString hint = disk_hints->Item(i).GetHint();
			const DiskParam *param = gDiskTemplates.Find(hint);
			if (param) {
				int disk_size_hint = param->CalcDiskSize();
				if (stream_size == disk_size_hint) {
					// ファイルサイズが一致
					disk_params.Add(param);
				}
			}
		}
	}

	// その他に同じパラメータの候補を追加
	gDiskTemplates.Find(sides_per_disk, tracks_per_side, sectors_per_track, sector_size, disk_params, disk_params.Count() > 0);

	// 候補がないとき手動設定
	if (disk_params.Count() == 0) {
		manual_param.SetDiskParam(
			sides_per_disk,
			tracks_per_side,
			sectors_per_track,
			sector_size,
			0,
			1,
			dummy.GetSingles(),
			dummy.GetParticularTracks()
		);
		return 1;
	}

	return 0;
}
