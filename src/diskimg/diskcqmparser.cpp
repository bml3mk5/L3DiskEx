/// @file diskcqmparser.cpp
///
/// @brief CopyQM imgディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskcqmparser.h"
#include <wx/mstream.h>
#include "../diskd88.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"


#pragma pack(1)
/// Copy QM形式ヘッダ
typedef struct st_cqm_dsk_header {
	wxUint8  ident[3];
	wxUint16 sector_size;
	wxUint8  sec_per_cluster;
	wxUint16 reserved_secs;
	wxUint8  num_of_fat;
	wxUint16 root_ent_cnt;
	wxUint16 total_sec_16;
	wxUint8  media_id;
	wxUint16 fat_size_16;
	wxUint16 sectors_per_track;
	wxUint16 num_of_heads;
	wxUint32 hidden_sectors;
	wxUint32 total_sec_32;
	wxUint8  desc[60];
	wxUint8  blind;
	wxUint8  density;
	wxUint8  used_tracks;
	wxUint8  total_tracks;
	wxUint32 data_crc;
	wxUint8  volume_label[11];
	wxUint16 time;
	wxUint16 date;
	wxUint16 comment_length;
	wxUint8  sector_base;
	wxUint8  unknown1[2];
	wxUint8  interleave;
	wxUint8  skew;
	wxUint8  drive;
	wxUint8  reserved2[13];
	wxUint8  head_crc;
//	wxUint8  comment[1];	// flexible bytes comment_length
} cqm_dsk_header_t;
#pragma pack()

//
//
//
DiskCQMParser::DiskCQMParser(DiskD88File *file, short mod_flags, DiskResult *result)
	: DiskPlainParser(file, mod_flags, result)
{
}

DiskCQMParser::~DiskCQMParser()
{
}

/// CQMファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param ディスクパラメータ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskCQMParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	if (!disk_param) {
		result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return result->GetValid();
	}

	istream.SeekI(0);

	cqm_dsk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len != sizeof(header)) {
		result->SetError(DiskResult::ERRV_DISK_TOO_SMALL, 0);
		return result->GetValid();
	}
	int comment_length = (int)wxUINT16_SWAP_ON_BE(header.comment_length);

	istream.SeekI(comment_length, wxFromCurrent);

	//
	int sector_size = (int)wxUINT16_SWAP_ON_BE(header.sector_size);
	int sectors_per_track = (int)wxUINT16_SWAP_ON_BE(header.sectors_per_track);
	int sides_per_disk = (int)wxUINT16_SWAP_ON_BE(header.num_of_heads);
	int tracks_per_side = (int)(header.total_tracks);

	int disk_size_hint = sides_per_disk * (tracks_per_side + 2) * sectors_per_track * sector_size;

	// expand datas
	wxUint8 *buffer = new wxUint8[disk_size_hint];

	wxMemoryOutputStream otemp(buffer, disk_size_hint);
	ExpandData(istream, otemp);

	wxMemoryInputStream itemp(buffer, disk_size_hint);

	int sts = DiskPlainParser::Parse(itemp, disk_param);

	delete [] buffer;

	return sts;
}

/// データを展開
size_t DiskCQMParser::ExpandData(wxInputStream &istream, wxOutputStream &ostream)
{
	size_t size = 0;

	wxUint8 buf[2];

	size_t len = 1;
	while(len > 0) {
		len = istream.Read(buf, 2).LastRead();
		if (len < 2) {
			break;
		}
		size += len;

		wxInt16 n = buf[0] | ((wxInt16)buf[1] << 8);
		if (n < 0) {
			// negative
			// １バイトデータをn回繰り返す
			len = istream.Read(buf, 1).LastRead();
			if (len < 1) {
				break;
			}
			for(int i=0; i<(-n); i++) {
				ostream.PutC((char)buf[0]);
				size++;
			}
		} else if (n > 0) {
			// positive
			// nバイトのベタデータ
			wxCharBuffer c(n);
			len = istream.Read(c.data(), c.length()).LastRead();
			if (len == 0) {
				break;
			}
			len = ostream.Write(c.data(), len).LastWrite();

			size += len;
		} else {
			// invalid size 0
			break;
		}
	}
	return size;
}

/// チェック
/// @param [in] dp            ディスクパーサ
/// @param [in] istream       解析対象データ
/// @param [in] disk_hints    ディスクパラメータヒント("2D"など)
/// @param [in] disk_param    ディスクパラメータ disk_hints指定時はNullable
/// @param [out] disk_params  ディスクパラメータの候補
/// @param [out] manual_param 候補がないときのパラメータヒント
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskCQMParser::Check(DiskParser &dp, wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	istream.SeekI(0);

	cqm_dsk_header_t header;
	size_t len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return -1;
	}
	if (memcmp(header.ident, "CQ\x14", 3) != 0) {
		// not CopyCQ image
		return -1;
	}
	int sector_size = (int)wxUINT16_SWAP_ON_BE(header.sector_size);
	if (sector_size <= 0 || sector_size > 4096) {
		// invalid
		result->SetError(DiskResult::ERRV_SECTOR_SIZE_HEADER, 0, sector_size);
		return result->GetValid();
	}
	int sectors_per_track = (int)wxUINT16_SWAP_ON_BE(header.sectors_per_track);
	if (sectors_per_track <= 0) {
		// invalid
		result->SetError(DiskResult::ERRV_SECTORS_HEADER, 0, sectors_per_track);
		return result->GetValid();
	}
	int sides_per_disk = (int)wxUINT16_SWAP_ON_BE(header.num_of_heads);
	if (sides_per_disk <= 0 || sides_per_disk > 2) {
		// invalid
		result->SetError(DiskResult::ERRV_SIDES_HEADER, 0, sides_per_disk);
		return result->GetValid();
	}
	int tracks_per_side = (int)(header.total_tracks);
	if (tracks_per_side < 1 || tracks_per_side > 164) {
		// invalid
		result->SetError(DiskResult::ERRV_TRACKS_HEADER, 0, tracks_per_side);
		return result->GetValid();
	}
//	tracks_per_side /= sides_per_disk;

	int interleave = (int)(header.interleave);
	if (interleave < 1 || interleave > sectors_per_track) interleave = 1;

	// ディスクテンプレートから探す
	DiskParam dummy;
	const DiskParam *param = gDiskTemplates.FindStrict(sides_per_disk, tracks_per_side, sectors_per_track, sector_size
		, interleave, dummy.GetTrackNumberBaseOnDisk(), dummy.GetSectorNumberBaseOnDisk(), 0
		, dummy.GetSingles(), dummy.GetParticularTracks());
	if (param) {
		disk_params.Add(param);
	}

	// 候補がないとき手動設定をセット
	if (disk_params.Count() == 0) {
		manual_param.SetDiskParam(
			sides_per_disk,
			tracks_per_side,
			sectors_per_track,
			sector_size,
			0,
			interleave,
			dummy.GetSingles(),
			dummy.GetParticularTracks()
		);
		return 1;
	}

	return 0;
}
