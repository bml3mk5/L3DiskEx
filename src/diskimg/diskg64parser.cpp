/// @file diskg64parser.cpp
///
/// @brief Commodore G64 ディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskg64parser.h"
#include <wx/mstream.h>
#include <wx/dynarray.h>
#include "../diskd88.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"
#include "../logging.h"


#pragma pack(1)
/// G64 セクタ
typedef struct st_g64_sector_header {
	wxUint8  block_id;
	wxUint8  chksum;
	wxUint8  sector_number;
	wxUint8  track_number;
	wxUint8  format_id0;
	wxUint8  format_id1;
	wxUint8  reserved[2];
} g64_sector_header_t;

/// G64 データ
typedef struct st_g64_sector_data {
	wxUint8  block_id;
	wxUint8  data[256];
	wxUint8  chksum;
	wxUint8  reserved[2];
} g64_sector_data_t;
#pragma pack()

//
//
//
DiskG64Parser::DiskG64Parser(DiskD88File *file, short mod_flags, DiskResult *result)
{
	this->file = file;
	this->mod_flags = mod_flags;
	this->result = result;
	memset(&this->header, 0, sizeof(this->header));
}

DiskG64Parser::~DiskG64Parser()
{
}

/// セクタデータの作成
/// @param [in] indata          セクタデータ
/// @param [in] disk_number     ディスク番号
/// @param [in] track_number    トラック番号
/// @param [in] side_number     サイド番号
/// @param [in] sector_nums     セクタ数
/// @param [in] sector_number   セクタ番号
/// @param [in] sector_size     セクタサイズ
/// @param [in] single_density  単密度か
/// @param [in,out] track       トラック
/// @return ヘッダ込みのセクタサイズ
wxUint32 DiskG64Parser::ParseSector(wxUint8 *indata, int disk_number, int track_number, int side_number, int sector_nums, int sector_number, int sector_size, bool single_density, DiskD88Track *track)
{
	// セクタ作成
	DiskD88Sector *sector = new DiskD88Sector(track_number, side_number, sector_number, sector_size, sector_nums, false);
	track->Add(sector);

	// コピー plain data
	sector->Copy(indata, sector_size);

	sector->SetSingleDensity(single_density);
	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sizeof(d88_sector_header_t) + sector_size;
}

/// Commodore GCR 5-bit -> HEX 4-bit map
static wxUint8 gcr_bin_map[] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff ,
	0xff, 0x8  ,0x0  ,0x1  ,0xff, 0xc  ,0x4  ,0x5  ,
	0xff, 0xff, 0x2  ,0x3  ,0xff, 0xf  ,0x6  ,0x7  ,
	0xff, 0x9  ,0xa  ,0xb  ,0xff, 0xd  ,0xe  ,0xff ,
};

/// GCRデータをデコード
/// @param [in]  indata		入力データ
/// @param [in]  inbitlen	入力データ長さ(bit単位)
/// @param [out] outdata	出力データ
/// @param [in]  outlen		出力データバッファサイズ
size_t DiskG64Parser::DecodeGCR(const wxUint8 *indata, int inbitlen, wxUint8 *outdata, size_t outlen)
{
	int bitpos = 0;
	size_t outpos = 0;
	while(bitpos < inbitlen && outpos < outlen) {
		wxUint8 adat = 0;
		for(int i=0; i<2; i++) {
			int pos = (bitpos >> 3);
			int bit = (bitpos & 7);

			wxUint32 dat = (wxUint32)indata[pos] << 8 | indata[pos+1];

			wxUint16 idat = (wxUint16)(dat >> (11 - bit));
			idat &= 0x1f;

			// 5ビットを4ビットに変換
			wxUint8 ndat = gcr_bin_map[idat];

			adat <<= 4;
			adat |= ndat;

			bitpos += 5;
		}
		outdata[outpos] = adat;
		outpos++;
	}
	return outpos;
}

/// トラックデータの作成
/// @param [in] istream         ディスクイメージ
/// @param [in] disk_number     ディスク番号
/// @param [in] side_number     サイド番号
/// @param [in] offset_pos      オフセット番号
/// @param [in] offset          オフセット位置
/// @param [in,out] disk        ディスク
/// @return -1:エラー or 終り >0:トラックサイズ
int DiskG64Parser::ParseTrack(wxInputStream &istream, int disk_number, int side_number, int offset_pos, wxUint32 offset, DiskD88Disk *disk)
{
	wxUint16 track_size = 0;
	size_t len = 0;

	// トラックサイズ
	len = istream.Read(&track_size, 2).LastRead();
	if (len < 2) {
		return -1;
	}

	track_size = wxUINT16_SWAP_ON_BE(track_size);
	if (track_size == 0) {
		return -1;
	}

	Utils::TempData intempdata((size_t)track_size + 32);
	wxUint8 *indata = intempdata.GetData();
	size_t insize = intempdata.GetBufferSize();
	Utils::TempData outtempdata((size_t)track_size);
	wxUint8 *outdata = outtempdata.GetData();
	size_t outsize = outtempdata.GetBufferSize();

	len = istream.Read(indata, insize).LastRead();

	wxArrayPtrVoid sector_headers;
	wxArrayPtrVoid sector_datas;

	size_t inpos = 0;
	size_t outpos = 0;
	// トラックデータの解析
	while(inpos < (size_t)track_size) {
		// skip header sync($ff) usually 4-5bytes
		while(indata[inpos] == 0xff && inpos < (size_t)track_size) {
			inpos++;
		}
		if (inpos >= (size_t)track_size) break;

		// header info CGR usually 10bytes
		len = DecodeGCR(&indata[inpos], 80, &outdata[outpos], outsize - outpos);
		sector_headers.Add(&outdata[outpos]);
		inpos += 10;
		outpos += len;

		// skip header gap
		while(indata[inpos] != 0xff && inpos < (size_t)track_size) {
			inpos++;
		}
		if (inpos >= (size_t)track_size) break;

		// skip data sync($ff) usually 4-5bytes
		while(indata[inpos] == 0xff && inpos < (size_t)track_size) {
			inpos++;
		}
		if (inpos >= (size_t)track_size) break;

		// data CGR usually 325bytes
		len = DecodeGCR(&indata[inpos], 2600, &outdata[outpos], outsize - outpos);
		sector_datas.Add(&outdata[outpos]);
		inpos += 325;
		outpos += len;

		// skip data gap
		while(indata[inpos] != 0xff && inpos < (size_t)track_size) {
			inpos++;
		}
		if (inpos >= (size_t)track_size) break;
	}

	// トラック番号を計算
	g64_sector_header_t *sh = (g64_sector_header_t *)sector_headers.Item(0);
	int track_number = sh->track_number;

//	myLog.SetDebug("G64: C:%d H:%d POS:%d", track_number, side_number, offset_pos);

	// トラックの作成
	wxUint32 d88_track_size = 0;
	DiskD88Track *track = new DiskD88Track(disk, track_number, side_number, offset_pos, 1);
	disk->SetMaxTrackNumber(track_number);

	for(size_t i=0; i<sector_datas.Count(); i++) {
		// セクタの作成
		g64_sector_header_t *sh = (g64_sector_header_t *)sector_headers.Item(i);
		g64_sector_data_t *sd = (g64_sector_data_t *)sector_datas.Item(i); 

		d88_track_size = ParseSector(sd->data,
			disk_number,
			sh->track_number,
			side_number,
			(int)sector_headers.Count(),
			sh->sector_number,
			256,
			false,
			track
		);
	}

	if (result->GetValid() >= 0) {
		// インターリーブの計算
		track->CalcInterleave();
	}

	if (result->GetValid() >= 0) {
		// トラックサイズ設定
		track->SetSize(d88_track_size);
		// サイド番号は各セクタのID Hに合わせる
		track->SetSideNumber(track->GetMajorIDH());

		// ディスクに追加
		disk->Add(track);
		// オフセット設定
		disk->SetOffset(offset_pos, offset);
	} else {
		delete track;
	}

	return (int)d88_track_size;
}


/// ファイルを解析
/// @param [in] istream     解析対象データ
/// @param [in] disk_number ディスク番号
/// @retval -1: finish parsing
/// @retval  0: parse next disk
int DiskG64Parser::ParseDisk(wxInputStream &istream, int disk_number)
{
	// read header
	if (ParseHeader(istream, disk_number) < 0) {
		return -1;
	}

	// offset to track data
	size_t len = 0;
	wxArrayInt offsets;
	bool has_halftrack = false;
	for(int pos = 0; pos < header.num_of_tracks; pos++) {
		wxUint32 offset = 0;
		len = istream.Read(&offset, 4).LastRead();
		if (len < 4) {
			return -1;
		}
		offset = wxUINT32_SWAP_ON_BE(offset);

		offsets.Add((int)offset);
		// ハーフトラックを持っているか
		has_halftrack |= (offset != 0 && (pos & 1) != 0); 
	}

	// skip spped zone data
	size_t size = header.num_of_tracks * 4;
	len = istream.SeekI(size, wxFromCurrent);
	if (len == (size_t)wxInvalidOffset) {
		return -1;
	}

	// ディスク作成
	DiskD88Disk *disk = new DiskD88Disk(file, disk_number);

	// トラック解析
	wxUint32 d88_offset = (int)sizeof(d88_header_t);
	int d88_offset_pos = 0;
	for(int pos = 0; pos < header.num_of_tracks; pos++) {
		size = offsets[pos];
		if (size == 0) {
			// skip empty track
			continue;
		}

		len = istream.SeekI(size, wxFromStart);
		if (len == (size_t)wxInvalidOffset) {
			break;
		}
		int offset = ParseTrack(istream, disk_number, has_halftrack ? pos & 1 : 0, d88_offset_pos, d88_offset, disk);
		if (offset == -1) {
			break;
		}
		d88_offset += offset;
		d88_offset_pos++;

//		if (d88_offset_pos >= DISKD88_MAX_TRACKS) {
//			result->SetError(DiskResult::ERRV_OVERFLOW_SIZE, disk_number, d88_offset);
//		}
	}
	disk->SetSize(d88_offset);

	if (result->GetValid() >= 0) {
		// ディスクを追加
		const DiskParam *disk_param = disk->CalcMajorNumber();
		if (disk_param) {
			disk->SetDensity(disk_param->GetParamDensity());
		}
		file->Add(disk, mod_flags);
	} else {
		delete disk;
	}

	return 0;
}

/// ヘッダ解析
/// @param [in] istream     解析対象データ
/// @param [in] disk_number ディスク番号
/// @retval -1: エラー
/// @retval  0:
int DiskG64Parser::ParseHeader(wxInputStream &istream, int disk_number)
{
	size_t len = 0;

	len = istream.Read(&header, sizeof(header)).LastRead();
	if (len < sizeof(header)) {
		// too short
		return -1;
	}
	if (memcmp(header.sig, "GCR-1541", sizeof(header.sig)) != 0) {
		// not a image
		return -1;
	}
	if (header.num_of_tracks == 0) {
		return -1;
	}

	return 0;
}

/// チェック
/// @param [in] dp            ディスクパーサ
/// @param [in] istream       解析対象データ
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskG64Parser::Check(DiskParser &dp, wxInputStream &istream)
{
	istream.SeekI(0);

	if (ParseHeader(istream, 0) < 0) {
		return -1;
	}

	return 0;
}

/// ファイルを解析
/// @param [in] istream    解析対象データ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskG64Parser::Parse(wxInputStream &istream)
{
	istream.SeekI(0);

	for(int disk_number = 0; disk_number < 1; disk_number++) {
		if (ParseDisk(istream, disk_number) < 0) {
			break;
		}
	}
	return result->GetValid();
}
