/// @file disk2mgparser.cpp
///
/// @brief 2MGディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "disk2mgparser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"


#pragma pack(1)
/// 2MGヘッダ
typedef struct st_twomg_header {
	wxUint8  ident[4];
	wxUint8  creator[4];
	wxUint16 header_size;	// LE
	wxUint16 version;
	wxUint32 format_type;

	wxUint32 flags;			// for DOS3.3
	wxUint32 blocks;		// for ProDOS
	wxUint32 offset_data;	// start position of data
	wxUint32 data_size;

	wxUint32 offset_comm;	// start position of comment
	wxUint32 comm_size;
	wxUint32 offset_creat;	// start position of creator data
	wxUint32 creat_size;

	wxUint8  reserved[16];
} twomg_header_t;
#pragma pack()

#define DISK_2MG_HEADER "2IMG"

//
//
//
Disk2MGParser::Disk2MGParser(DiskImageFile *file, short mod_flags, DiskResult *result)
	: DiskPlainParser(file, mod_flags, result)
{
}

Disk2MGParser::~Disk2MGParser()
{
}

/// ファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param ディスクパラメータ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int Disk2MGParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	if (!disk_param) {
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return p_result->GetValid();
	}

	istream.SeekI(0);

	twomg_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return p_result->GetValid();
	}

//	// フォーマットタイプ
//	wxUint32 format_type = wxUINT32_SWAP_ON_BE(header.format_type);

	wxUint32 offset_data = wxUINT32_SWAP_ON_BE(header.offset_data);

	istream.SeekI(offset_data);

	int rc = DiskPlainParser::Parse(istream, disk_param);
	if (rc >= 0) {
		DiskImageDisk *disk = p_file->GetDisk(0);
		if (disk) {
			wxUint32 flags = wxUINT32_SWAP_ON_BE(header.flags);

			// write protected ?
			if (flags & 0x80000000) {
				disk->SetWriteProtect(true);
			}
		}
	}
	return rc;
}

/// チェック
/// @param [in] istream       解析対象データ
/// @param [in] disk_hints    ディスクパラメータヒント("2D"など)
/// @param [in] disk_param    ディスクパラメータ disk_hints指定時はNullable
/// @param [out] disk_params  ディスクパラメータの候補
/// @param [out] manual_param 候補がないときのパラメータヒント
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int Disk2MGParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	istream.SeekI(0);

	twomg_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		p_result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return p_result->GetValid();
	}
	// ヘッダ文字列チェック
	if (memcmp(header.ident, DISK_2MG_HEADER, sizeof(header.ident)) != 0) {
		// not disk
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return p_result->GetValid();
	}
	// フォーマットタイプ
	wxUint32 format_type = wxUINT32_SWAP_ON_BE(header.format_type);
	if (format_type == 2) {
		// unsupported format
		p_result->SetError(DiskResult::ERRV_UNSUPPORTED_TYPE, 0, wxT("NIB"));
		return p_result->GetValid();
	} else if (format_type > 2) {
		// invalid
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return p_result->GetValid();
	}

//	wxUint32 flags = wxUINT32_SWAP_ON_BE(header.flags);
	wxUint32 data_size = wxUINT32_SWAP_ON_BE(header.data_size);

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
		, 1, 0, 0, 0, 0
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
