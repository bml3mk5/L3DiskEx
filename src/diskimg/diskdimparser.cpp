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
	wxUint8  ident[15];
	wxUint8  date[4];
	wxUint8  time[4];
	wxUint8  comments[0x3d];
	wxUint8  overtrack;
} dim_dsk_header_t;
#pragma pack()

#define DISK_DIM_HEADER "DIFC HEADER  \0\0"

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

/// ディスクデータの解析
/// @param[in] istream           入力ディスクイメージ
/// @param[in] disk_number       ディスク番号
/// @param[in] disk_param        ディスクパラメータ
/// @return オフセット
wxUint32 DiskDIMParser::ParseDisk(wxInputStream &istream, int disk_number, const DiskParam *disk_param)
{
	dim_dsk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len != sizeof(header)) {
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return 0;
	}

	istream.SeekI(0x100);

	DiskImageDisk *disk = p_file->NewImageDisk(disk_number);

	// パラメータの計算値がディスクサイズの２倍なら
	// 表面にのみデータをセットする
	int dummy_side = -1;
	if ((int)istream.GetLength() * 2 <= disk_param->CalcDiskSize()) {
		dummy_side = disk_param->GetSideNumberBaseOnDisk() + 1;
	}

	wxUint32 offset = (int)disk->GetOffsetStart();
	int offset_pos = 0;
	int track_num = disk_param->GetTrackNumberBaseOnDisk();
	int tracks_per_side = disk_param->GetTracksPerSide() + track_num;
	int side_num_st = disk_param->GetSideNumberBaseOnDisk();
	int side_num_ed = disk_param->GetSidesPerDisk() + side_num_st;
	for(; track_num < tracks_per_side && p_result->GetValid() >= 0; track_num++) {
		for(int side_num = side_num_st; side_num < side_num_ed && p_result->GetValid() >= 0; side_num++) {
			bool is_dummy_track = (side_num == dummy_side);
			if (header.tracks[offset_pos] == 0) {
				// トラック情報がないので、ダミーのトラックを作成する
				is_dummy_track = true;
			}
			offset += ParseTrack(istream, offset_pos, offset, disk_number, disk_param, track_num, side_num, is_dummy_track, disk); 
			offset_pos++;
		}
	}
	disk->SetSize(offset);

	if (p_result->GetValid() >= 0) {
		// ディスクを追加
		const DiskParam *disk_param = disk->CalcMajorNumber();
		if (disk_param) {
			disk->SetDensity(disk_param->GetParamDensity());
		}
		p_file->Add(disk, m_mod_flags);
	} else {
		delete disk;
	}

	return offset;
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


	ParseDisk(istream
		, 0
		, disk_param);

	return p_result->GetValid();
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
	if (memcmp(header.ident, DISK_DIM_HEADER, sizeof(header.ident)) != 0) {
		// not disk
		return -1;
	}

	int stream_size = (int)istream.GetLength() - 0x100;
	int sides_per_disk = 2;
	int sector_size = 1024;
	int sectors_per_track = 8;

	// トラック数を数える
	int max_tracks = 0;
	for(size_t n=0; n<sizeof(header.tracks); n++) {
		if (header.tracks[n] != 0) {
			max_tracks = (int)(n+1);
		}
	}
	if (max_tracks == 0) {
		max_tracks = 144;
	}

	// セクタ数
	int secs_per_trk_mod = (stream_size / sector_size) % max_tracks;
	if (secs_per_trk_mod == 0) {
		// decide
		sectors_per_track = (stream_size / sector_size) / max_tracks;
	} else {
		for(int i=8; i<10; i++) {
			if (((stream_size / sector_size) % i) == 0) {
				sectors_per_track = i;
				break;
			}
		}
	}

	int tracks_per_side = (max_tracks / sides_per_disk);

	// ディスクテンプレートから探す
	DiskParam dummy;

	if (disk_hints != NULL) {
		// パラメータヒントあり
		for(int retry = 0; retry < 2 &&  disk_params.Count() == 0; retry++) {
			// 優先順位の高い候補を追加
			for(size_t i=0; i<disk_hints->Count(); i++) {
				int kind = disk_hints->Item(i).GetKind();
				if (kind != header.type) {
					continue;
				}
				wxString hint = disk_hints->Item(i).GetHint();
				const DiskParam *param = gDiskTemplates.Find(hint);
				if (param) {
					// ファイルサイズが一致
					// or トラック数が一致
					// or リトライ時
					if (stream_size == param->CalcDiskSize()
					 || tracks_per_side == param->GetTracksPerSide()
					 || retry > 0) {
						 disk_params.Add(param);
					}
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
