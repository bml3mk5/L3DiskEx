/// @file diskadcparser.cpp
///
/// @brief Apple Disk Copyディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskadcparser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"


#pragma pack(1)
/// Apple Disk Copyヘッダ 84bytes
typedef struct st_adc_header {
	wxUint8  label_length;
	wxUint8  label[63];
	wxUint32 data_size;		// BE
	wxUint32 post_size;		// BE

	wxUint8  unknown[12];
} adc_header_t;
#pragma pack()

//
//
//
DiskADCParser::DiskADCParser(DiskImageFile *file, short mod_flags, DiskResult *result)
	: DiskPlainParser(file, mod_flags, result)
{
}

DiskADCParser::~DiskADCParser()
{
}

/// ファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param ディスクパラメータ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskADCParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	if (!disk_param) {
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return p_result->GetValid();
	}

	istream.SeekI(0);

	adc_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return p_result->GetValid();
	}

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
int DiskADCParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	istream.SeekI(0);

	adc_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return p_result->GetValid();
	}
	// ラベル長の末尾が0かどうか
	if ((size_t)header.label_length > sizeof(header.label) || header.label[header.label_length] != 0) {
		// not disk
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return p_result->GetValid();
	}
	// ファイルサイズが一致するか
	wxUint32 data_size = wxUINT32_SWAP_ON_LE(header.data_size);
	wxUint32 file_size = (wxUint32)sizeof(header) + data_size + wxUINT32_SWAP_ON_LE(header.post_size);
	if (file_size != (wxUint32)istream.GetLength()) {
		// not disk
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return p_result->GetValid();
	}

	// データサイズからディスクのパラメータを算出
	int sides_per_disk = 1;
	int tracks_per_side = 1;
	int sectors_per_track = 1;
	int sector_size = 256;
	DiskParticulars sd;
	DiskParticulars pt;

	if (data_size <= 143360) {
		sides_per_disk = 1;
		tracks_per_side = 35;
		sectors_per_track = 16;
		sector_size = 256;
	} else if (data_size <= 819200) {
		sides_per_disk = 2;
		tracks_per_side = 80;
		sectors_per_track = 12;
		sector_size = 512;
		for(int i=16, n=sectors_per_track-1; i<tracks_per_side; i+=16, n--) {
			pt.Add(DiskParticular(i, -1, -1, 16, n, 512));
		}
	}

	// ディスクテンプレートから探す
	const DiskParam *param = gDiskTemplates.FindStrict(sides_per_disk, tracks_per_side, sectors_per_track, sector_size
		, 1, 0, 0, 0
		, sd, pt);
	if (param) {
		disk_params.Add(param);
	}

	// 候補がないとき手動設定
	if (disk_params.Count() == 0) {
		manual_param.SetDiskParam(
			sides_per_disk,
			tracks_per_side,
			sectors_per_track,
			sector_size,
			0,
			1,
			sd,
			pt
		);
		return 1;
	}

	return 0;
}
